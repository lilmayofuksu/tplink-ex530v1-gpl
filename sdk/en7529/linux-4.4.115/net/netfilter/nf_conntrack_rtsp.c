/*
 * RTSP extension for IP connection tracking
 * (C) 2003 by Tom Marshall <tmarshall at real.com>
 *
 * 2005-02-13: Harald Welte <laforge at netfilter.org>
 * 	- port to 2.6
 * 	- update to recent post-2.6.11 api changes
 * 2006-09-14: Steven Van Acker <deepstar at singularity.be>
 *	- removed calls to NAT code from conntrack helper: NAT no longer needed to use rtsp-conntrack
 * 2007-04-18: Michael Guntsche <mike at it-loops.com>
 * 			- Port to new NF API
 * 2013-03-04: Il'inykh Sergey <sergeyi at inango-sw.com>. Inango Systems Ltd
 *	- fixed rtcp nat mapping and other port mapping fixes
 *	- simple TEARDOWN request handling
 *	- codestyle fixes and other less significant bug fixes 
 *
 * based on ip_conntrack_irc.c
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 * Module load syntax:
 *   insmod nf_conntrack_rtsp.o ports=port1,port2,...port<MAX_PORTS>
 *                              max_outstanding=n setup_timeout=secs
 *
 * If no ports are specified, the default will be port 554.
 *
 * With max_outstanding you can define the maximum number of not yet
 * answered SETUP requests per RTSP session (default 8).
 * With setup_timeout you can specify how long the system waits for
 * an expected data channel (default 300 seconds).
 *
 */

#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <linux/inet.h>
#include <net/tcp.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <linux/netfilter/nf_conntrack_rtsp.h>
#include <linux/proc_fs.h>

#define NF_NEED_STRNCASECMP
#define NF_NEED_STRTOU16
#define NF_NEED_STRTOU32
#define NF_NEED_NEXTLINE
#include <linux/netfilter_helpers.h>
#define NF_NEED_MIME_NEXTLINE
#include <linux/netfilter_mime.h>

#include <linux/ctype.h>
#include "ecnt_netfilter.h"

#define MAX_PORTS 8
static int ports[MAX_PORTS];
static int num_ports = 0;
static int max_outstanding = 8;
static unsigned int setup_timeout = 300;

#define pr_debug(fmt, ...) 	printk(KERN_DEBUG pr_fmt(fmt), ##__VA_ARGS__)

#define RTSP_ALG_PORT_PATH "tc3162/rtsp_alg_port"

MODULE_AUTHOR("Tom Marshall <tmarshall at real.com>");
MODULE_DESCRIPTION("RTSP connection tracking module");
MODULE_LICENSE("GPL");
module_param_array(ports, int, &num_ports, 0400);
MODULE_PARM_DESC(ports, "port numbers of RTSP servers");
module_param(max_outstanding, int, 0400);
MODULE_PARM_DESC(max_outstanding, "max number of outstanding SETUP requests per RTSP session");
module_param(setup_timeout, int, 0400);
MODULE_PARM_DESC(setup_timeout, "timeout on for unestablished data channels");

static char *rtsp_buffer;
static DEFINE_SPINLOCK(rtsp_buffer_lock);

static struct nf_conntrack_expect_policy rtsp_exp_policy;

unsigned int (*nf_nat_rtsp_hook)(struct sk_buff *skb,
				 enum ip_conntrack_info ctinfo,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
				 unsigned int protoff,
#endif
				 unsigned int matchoff, unsigned int matchlen,
				 struct ip_ct_rtsp_expect* prtspexp,
				 struct nf_conntrack_expect *rtp_exp,
				 struct nf_conntrack_expect *rtcp_exp);

unsigned int (*nf_nat_rtp_hook)(struct sk_buff *skb,
				struct nf_conn *ct,
				 enum ip_conntrack_info ctinfo,
				 __be32 rtpip,
				 u_int16_t rtp_srcport) = NULL;


EXPORT_SYMBOL_GPL(nf_nat_rtsp_hook);
EXPORT_SYMBOL_GPL(nf_nat_rtp_hook);

/*
 * Max mappings we will allow for one RTSP connection (for RTP, the number
 * of allocated ports is twice this value).  Note that SMIL burns a lot of
 * ports so keep this reasonably high.  If this is too low, you will see a
 * lot of "no free client map entries" messages.
 */
#define MAX_PORT_MAPS 16

/*** default port list was here in the masq code: 554, 3030, 4040 ***/

#define SKIP_WSPACE(ptr,len,off) while(off < len && isspace(*(ptr+off))) { off++; }

/*
 * Parse an RTSP packet.
 *
 * Returns zero if parsing failed.
 *
 * Parameters:
 *  IN      ptcp        tcp data pointer
 *  IN      tcplen      tcp data len
 *  IN/OUT  ptcpoff     points to current tcp offset
 *  OUT     phdrsoff    set to offset of rtsp headers
 *  OUT     phdrslen    set to length of rtsp headers
 *  OUT     pcseqoff    set to offset of CSeq header
 *  OUT     pcseqlen    set to length of CSeq header
 */
static int
rtsp_parse_message(char* ptcp, uint tcplen, uint* ptcpoff,
		   uint* phdrsoff, uint* phdrslen,
		   uint* pcseqoff, uint* pcseqlen,
		   uint* transoff, uint* translen,
		   uint* rtpinfoff, uint* rtpinflen)
{
	uint	entitylen = 0;
	uint	lineoff;
	uint	linelen;
	
	if (!nf_nextline(ptcp, tcplen, ptcpoff, &lineoff, &linelen))
		return 0;
	
	*phdrsoff = *ptcpoff;
	while (nf_mime_nextline(ptcp, tcplen, ptcpoff, &lineoff, &linelen)) {
		if (linelen == 0) {
			if (entitylen > 0)
				*ptcpoff += min(entitylen, tcplen - *ptcpoff);
			break;
		}
		if (lineoff+linelen > tcplen) {
			pr_info("!! overrun !!\n");
			break;
		}

		if (nf_strncasecmp(ptcp+lineoff, "CSeq:", 5) == 0) {
			*pcseqoff = lineoff;
			*pcseqlen = linelen;
		} 

		if (nf_strncasecmp(ptcp+lineoff, "Transport:", 10) == 0) {
			*transoff = lineoff;
			*translen = linelen;
		}
		
		if (nf_strncasecmp(ptcp+lineoff, "RTP-Info:", 9) == 0) {
			*rtpinfoff = lineoff;
			*rtpinflen = linelen;
		}
		
		if (nf_strncasecmp(ptcp+lineoff, "Content-Length:", 15) == 0) {
			uint off = lineoff+15;
			SKIP_WSPACE(ptcp+lineoff, linelen, off);
			nf_strtou32(ptcp+off, &entitylen);
		}
	}
	*phdrslen = (*ptcpoff) - (*phdrsoff);
	
	return 1;
}

/*
 * Find lo/hi client ports (if any) in transport header
 * In:
 *   ptcp, tcplen = packet
 *   tranoff, tranlen = buffer to search
 *
 * Out:
 *   pport_lo, pport_hi = lo/hi ports (host endian)
 *
 * Returns nonzero if any client ports found
 *
 * Note: it is valid (and expected) for the client to request multiple
 * transports, so we need to parse the entire line.
 */
static int
rtsp_parse_transport(char* ptran, uint tranlen,
		     struct ip_ct_rtsp_expect* prtspexp)
{
	int  rc = 0;
	uint off = 0;
	
	if (tranlen < 10 || !iseol(ptran[tranlen-1]) ||
	    nf_strncasecmp(ptran, "Transport:", 10) != 0) {
		pr_info("sanity check failed\n");
		return 0;
	}
	
	pr_debug("tran='%.*s'\n", (int)tranlen, ptran);
	off += 10;
	SKIP_WSPACE(ptran, tranlen, off);
	
	/* Transport: tran;field;field=val,tran;field;field=val,... */
	while (off < tranlen) {
		const char* pparamend;
		uint        nextparamoff;
		
		pparamend = memchr(ptran+off, ',', tranlen-off);
		pparamend = (pparamend == NULL) ? ptran+tranlen : pparamend+1;
		nextparamoff = pparamend-ptran;
		
		while (off < nextparamoff) {
			const char* pfieldend;
			uint        nextfieldoff;
			
			pfieldend = memchr(ptran+off, ';', nextparamoff-off);
			nextfieldoff = (pfieldend == NULL) ? nextparamoff : pfieldend-ptran+1;
		   
			if (strncmp(ptran+off, "client_port=", 12) == 0) {
				u_int16_t   port;
				uint        numlen;

				off += 12;
				numlen = nf_strtou16(ptran+off, &port);
				off += numlen;
				if (prtspexp->loport != 0 && prtspexp->loport != port)
					pr_debug("multiple ports found, port %hu ignored\n", port);
				else {
					pr_debug("lo port found : %hu\n", port);
					prtspexp->loport = prtspexp->hiport = port;
					if (ptran[off] == '-') {
						off++;
						numlen = nf_strtou16(ptran+off, &port);
						off += numlen;
						prtspexp->pbtype = pb_range;
						prtspexp->hiport = port;
						
						// If we have a range, assume rtp:
						// loport must be even, hiport must be loport+1
						if ((prtspexp->loport & 0x0001) != 0 ||
						    prtspexp->hiport != prtspexp->loport+1) {
							pr_debug("incorrect range: %hu-%hu, correcting\n",
							       prtspexp->loport, prtspexp->hiport);
							prtspexp->loport &= 0xfffe;
							prtspexp->hiport = prtspexp->loport+1;
						}
					} else if (ptran[off] == '/') {
						off++;
						numlen = nf_strtou16(ptran+off, &port);
						off += numlen;
						prtspexp->pbtype = pb_discon;
						prtspexp->hiport = port;
					}
					rc = 1;
				}
			}
			
			/*
			 * Note we don't look for the destination parameter here.
			 * If we are using NAT, the NAT module will handle it.  If not,
			 * and the client is sending packets elsewhere, the expectation
			 * will quietly time out.
			 */
			
			off = nextfieldoff;
		}
		
		off = nextparamoff;
	}
	
	return rc;
}

static int
rtsp_reply_rtpinfo(struct sk_buff *skb,
			 char* prtpinf, uint rtpinflen,
		     struct nf_conn *ct,  enum ip_conntrack_info ctinfo)
{
	int  rc = 0;
	uint off = 0;
	char ipbuf[40];
	int ip_buf_len = 0;
	__be32 newip = 0;
	u_int16_t src_port = 0;
	typeof(nf_nat_rtp_hook) nf_nat_rtp;
	int dir = CTINFO2DIR(ctinfo);   /* = IP_CT_DIR_REPLY */

	if (rtpinflen < 9 || !iseol(prtpinf[rtpinflen-1]) ||
	    nf_strncasecmp(prtpinf, "RTP-Info:", 9) != 0) {
		pr_info("rtsp_process_rtpinfo check failed\n");
		return 0;
	}

	pr_debug("rtpinfo='%.*s'\n", (int)rtpinflen, prtpinf);
	off += 10;
	SKIP_WSPACE(prtpinf, rtpinflen, off);
	
	/* RTP-Info: tran;field;field=val,tran;field;field=val,... */
	while (off < rtpinflen) {
		const char* pparamend;
		uint        nextparamoff;

		pparamend = memchr(prtpinf+off, ',', rtpinflen-off);
		pparamend = (pparamend == NULL) ? prtpinf+rtpinflen : pparamend+1;
		nextparamoff = pparamend-prtpinf;

		while (off < nextparamoff) {
			const char* pfieldend;
			uint        nextfieldoff;
			
			pfieldend = memchr(prtpinf+off, ';', nextparamoff-off);
			nextfieldoff = (pfieldend == NULL) ? nextparamoff : pfieldend-prtpinf+1;
		   
			if (strncmp(prtpinf+off, "url=", 4) == 0) {
				off += 4;

				/* rtsp:// */
				off += 7;
				const char* pipend;
				pipend = memchr(prtpinf+off, ':', nextparamoff-off);
				if ( pipend )
				{
					nf_strtou16(pipend+1, &src_port);
					ip_buf_len = pipend - (prtpinf+off);
					if ( ip_buf_len < sizeof(ipbuf) )
					{
						memset(ipbuf, 0, sizeof(ipbuf));
						memcpy(ipbuf, prtpinf+off, ip_buf_len);
						newip = in_aton(ipbuf);

						/*
						for vlc debug
						newip = in_aton("192.168.40.250");
						ct->loport = ct->hiport = htons(5566);
						printk("\n==>org src_port=[%d]<===\n", src_port);
						src_port = htons(1234);*/

						nf_nat_rtp = rcu_dereference(nf_nat_rtp_hook);
						if (nf_nat_rtp && ct->status & IPS_NAT_MASK)
						{
							pr_debug("\n=>nf_nat_rtp ip=%pI4,"
								" loport=[%d], hiport=[%d], src_port=[%d]\n"
								, &newip, ntohs(ct->loport), ntohs(ct->hiport), src_port);
							/* pass the request off to the rtp helper */
							rc = nf_nat_rtp(skb, ct, ctinfo, newip, src_port);
						}
					}
				}

				/* now only process one ip*/
				break;
			}

			off = nextfieldoff;
		}
		
		off = nextparamoff;
	}
	
	return rc;
}

/*** conntrack functions ***/

/* outbound packet: client->server */

static inline int
help_out(struct sk_buff *skb, unsigned char *rb_ptr, unsigned int datalen,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
	 struct nf_conn *ct, enum ip_conntrack_info ctinfo,
	 unsigned int protoff)
#else
	 struct nf_conn *ct, enum ip_conntrack_info ctinfo)
#endif
{
	struct ip_ct_rtsp_expect expinfo;
	
	int dir = CTINFO2DIR(ctinfo);   /* = IP_CT_DIR_ORIGINAL */
	//struct  tcphdr* tcph = (void*)iph + iph->ihl * 4;
	//uint    tcplen = pktlen - iph->ihl * 4;
	char*   pdata = rb_ptr;
	//uint    datalen = tcplen - tcph->doff * 4;
	uint    dataoff = 0;
	int ret = NF_ACCEPT;
	
	struct nf_conntrack_expect *rtp_exp;
	struct nf_conntrack_expect *rtcp_exp = NULL;
	
	__be16 be_loport;
	__be16 be_hiport;
	
	typeof(nf_nat_rtsp_hook) nf_nat_rtsp;

	memset(&expinfo, 0, sizeof(expinfo));
	
	while (dataoff < datalen) {
		uint cmdoff = dataoff;
		uint hdrsoff = 0;
		uint hdrslen = 0;
		uint cseqoff = 0;
		uint cseqlen = 0;
		uint transoff = 0;
		uint translen = 0;
		uint rtpinfoff = 0;
		uint rtpinflen = 0;
		uint off;
		
		if (!rtsp_parse_message(pdata, datalen, &dataoff,
					&hdrsoff, &hdrslen,
					&cseqoff, &cseqlen,
					&transoff, &translen,
					&rtpinfoff, &rtpinflen))
			break;      /* not a valid message */

		if (strncmp(pdata+cmdoff, "TEARDOWN ", 9) == 0) {
			pr_debug("teardown handled\n");
			nf_ct_remove_expectations(ct); /* FIXME must be session id aware */
			break;
		}

		if (strncmp(pdata+cmdoff, "SETUP ", 6) != 0)
			continue;   /* not a SETUP message */

		pr_debug("found a setup message\n");

		off = 0;
		if(translen)
			rtsp_parse_transport(pdata+transoff, translen, &expinfo);

		if (expinfo.loport == 0) {
			pr_debug("no udp transports found\n");
			continue;   /* no udp transports found */
		}

		pr_debug("udp transport found, ports=(%d,%hu,%hu)\n",
			 (int)expinfo.pbtype, expinfo.loport, expinfo.hiport);


		be_loport = htons(expinfo.loport);
		ct->loport = htons(expinfo.loport);
		ct->hiport = htons(expinfo.hiport);

		rtp_exp = nf_ct_expect_alloc(ct);
		if (rtp_exp == NULL) {
			ret = NF_DROP;
			goto out;
		}

		nf_ct_expect_init(rtp_exp, NF_CT_EXPECT_CLASS_DEFAULT,
				  nf_ct_l3num(ct),
				  &ct->tuplehash[!dir].tuple.src.u3,
				  &ct->tuplehash[!dir].tuple.dst.u3,
				  IPPROTO_UDP, NULL, &be_loport);

		rtp_exp->flags = 0;

		/* 
		 * CHENMING 20240401 for RTSP ALG, 
		 * when the server udp arrived, we will delete the exp if the flag is not NF_CT_EXPECT_PERMANENT, 
		 * if the rtsp PAUSE message to server to pause the stream, and then PLAY to continue,
		 * when server udp stream arrived again, as the exp not exist, we will not forward the UDP stream then,
		 * which may cause the STB not playing.
		 * so here we keep the exp rule permanent to avoid it.
		 */
		if (1)
		{
			rtp_exp->flags = NF_CT_EXPECT_PERMANENT;
			pr_debug("setup expectation(%p) for rtp with flag NF_CT_EXPECT_PERMANENT!\n", rtp_exp);
		}

		ecnt_nf_conntrack_rtsp_help_out_inline_hook(rtp_exp, ct);

		if (expinfo.pbtype == pb_range) {
			pr_debug("setup expectation for rtcp\n");

			be_hiport = htons(expinfo.hiport);
			rtcp_exp = nf_ct_expect_alloc(ct);
			if (rtcp_exp == NULL) {
				ret = NF_DROP;
				goto out1;
			}

			nf_ct_expect_init(rtcp_exp, NF_CT_EXPECT_CLASS_DEFAULT,
					  nf_ct_l3num(ct),
					  &ct->tuplehash[!dir].tuple.src.u3,
					  &ct->tuplehash[!dir].tuple.dst.u3,
					  IPPROTO_UDP, NULL, &be_hiport);

			rtcp_exp->flags = 0;
			/* 
			 * CHENMING 20240401 for RTSP ALG, 
			 * when the server udp arrived, we will delete the exp if the flag is not NF_CT_EXPECT_PERMANENT, 
			 * if the rtsp PAUSE message to server to pause the stream, and then PLAY to continue,
			 * when server udp stream arrived again, as the exp not exist, we will not forward the UDP stream then,
			 * which may cause the STB not playing.
			 * so here we keep the exp rule permanent to avoid it.
			 */
			if (1)
			{
				rtcp_exp->flags = NF_CT_EXPECT_PERMANENT;
				pr_debug("setup expectation(%p) for rtcp with flag NF_CT_EXPECT_PERMANENT!\n", rtcp_exp);
			}

			ecnt_nf_conntrack_rtsp_help_out_inline_hook(rtcp_exp, ct);

			pr_debug("expect_related %pI4:%u-%u-%pI4:%u-%u\n",
				   &rtp_exp->tuple.src.u3.ip,
				   ntohs(rtp_exp->tuple.src.u.udp.port),
				   ntohs(rtcp_exp->tuple.src.u.udp.port),
				   &rtp_exp->tuple.dst.u3.ip,
				   ntohs(rtp_exp->tuple.dst.u.udp.port),
				   ntohs(rtcp_exp->tuple.dst.u.udp.port));
		} else {
			pr_debug("expect_related %pI4:%u-%pI4:%u\n",
					&rtp_exp->tuple.src.u3.ip,
					ntohs(rtp_exp->tuple.src.u.udp.port),
					&rtp_exp->tuple.dst.u3.ip,
					ntohs(rtp_exp->tuple.dst.u.udp.port));
		}

		nf_nat_rtsp = rcu_dereference(nf_nat_rtsp_hook);
		if (nf_nat_rtsp && ct->status & IPS_NAT_MASK)
			/* pass the request off to the nat helper */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
			ret = nf_nat_rtsp(skb, ctinfo, protoff, hdrsoff, hdrslen,
					  &expinfo, rtp_exp, rtcp_exp);
#else
			ret = nf_nat_rtsp(skb, ctinfo, hdrsoff, hdrslen,
					  &expinfo, rtp_exp, rtcp_exp);
#endif
		else {
			if (nf_ct_expect_related(rtp_exp) == 0) {
				if (rtcp_exp && nf_ct_expect_related(rtcp_exp) != 0) {
					nf_ct_unexpect_related(rtp_exp);
					pr_info("nf_conntrack_expect_related failed for rtcp\n");
					ret = NF_DROP;
				}
			} else {
				pr_info("nf_conntrack_expect_related failed for rtp\n");
				ret = NF_DROP;
			}
		}
		if (rtcp_exp) {
			nf_ct_expect_put(rtcp_exp);
		}
out1:
		nf_ct_expect_put(rtp_exp);
		goto out;
	}
out:

	return ret;
}


static inline int
help_in(struct sk_buff *skb, unsigned char *rb_ptr, unsigned int datalen,
	 struct nf_conn *ct, enum ip_conntrack_info ctinfo,
	 unsigned int protoff)
{
	char*   pdata = rb_ptr;
	uint    dataoff = 0;
	uint hdrsoff = 0;
	uint hdrslen = 0;
	uint cseqoff = 0;
	uint cseqlen = 0;
	uint transoff = 0;
	uint translen = 0;
	uint rtpinfoff = 0;
	uint rtpinflen = 0;
	uint off;

	if ( !rtsp_parse_message(pdata, datalen, &dataoff,
				&hdrsoff, &hdrslen,
				&cseqoff, &cseqlen,
				&transoff, &translen,
				&rtpinfoff, &rtpinflen) )
		return NF_ACCEPT;	/* not a valid message */

	if ( 0 != rtpinflen )
		rtsp_reply_rtpinfo(skb, pdata+rtpinfoff, rtpinflen, ct, ctinfo);

	return NF_ACCEPT;
}

static int help(struct sk_buff *skb, unsigned int protoff,
		struct nf_conn *ct, enum ip_conntrack_info ctinfo) 
{
	struct tcphdr _tcph, *th;
	unsigned int dataoff, datalen;
	char *rb_ptr;
	int ret = NF_DROP;

	/* Until there's been traffic both ways, don't look in packets. */
	if (ctinfo != IP_CT_ESTABLISHED && 
	    ctinfo != IP_CT_ESTABLISHED + IP_CT_IS_REPLY) {
		pr_debug("conntrackinfo = %u\n", ctinfo);
		return NF_ACCEPT;
	} 

	if ( ECNT_RETURN == ecnt_nf_conntrack_rtsp_help_inline_hook(skb, ct) )
		return NF_ACCEPT;

	/* Not whole TCP header? */
	th = skb_header_pointer(skb, protoff, sizeof(_tcph), &_tcph);

	if (!th)
		return NF_ACCEPT;
   
	/* No data ? */
	dataoff = protoff + th->doff*4;
	datalen = skb->len - dataoff;
	if (dataoff >= skb->len)
		return NF_ACCEPT;

	spin_lock_bh(&rtsp_buffer_lock);
	rb_ptr = skb_header_pointer(skb, dataoff,
				    skb->len - dataoff, rtsp_buffer);
	BUG_ON(rb_ptr == NULL);

#if 0
	/* Checksum invalid?  Ignore. */
	/* FIXME: Source route IP option packets --RR */
	if (tcp_v4_check(tcph, tcplen, iph->saddr, iph->daddr,
			 csum_partial((char*)tcph, tcplen, 0)))
	{
		DEBUGP("bad csum: %p %u %u.%u.%u.%u %u.%u.%u.%u\n",
		       tcph, tcplen, NIPQUAD(iph->saddr), NIPQUAD(iph->daddr));
		return NF_ACCEPT;
	}
#endif

	switch (CTINFO2DIR(ctinfo)) {
	case IP_CT_DIR_ORIGINAL:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
		ret = help_out(skb, rb_ptr, datalen, ct, ctinfo, protoff);
#else
		ret = help_out(skb, rb_ptr, datalen, ct, ctinfo);
#endif
		break;
	case IP_CT_DIR_REPLY:
		pr_debug("IP_CT_DIR_REPLY\n");
		ecnt_nf_conntrack_rtsp_help_reply_inline_hook(skb, rb_ptr, datalen);
		/* inbound packet: server->client */
		/*ret = NF_ACCEPT;*/
		ret = help_in(skb, rb_ptr, datalen, ct, ctinfo, protoff);
		break;
	}

	spin_unlock_bh(&rtsp_buffer_lock);

	return ret;
}

static struct nf_conntrack_helper rtsp_helpers[MAX_PORTS*2];
static char rtsp_names[MAX_PORTS*2][10];

static int rtsp_alg_port_read_proc(char *buf, char **start, off_t off, int count,int *eof, void *data)
{
	int len = 0, i = 0;
	char port_buf[12] = {0};

	len += snprintf(buf, 15, "%s", "rtsp alg port:");
	for ( i = 0;  i < MAX_PORTS; i++ )
	{
		if ( 0 != ports[i] )
		{
			len += snprintf(port_buf, sizeof(port_buf), "%d ", ports[i]);
			strncat(buf, port_buf, 12);
		}
	}
	buf[len - 1] = '\n';

	*start = buf + off;
	if (len < off + count)
		*eof = 1;
	len -= off;
	if (len > count)
		len = count ;
	if (len <0)
		len = 0;
		
	return len;

}

static int add_rtsp_alg_port(__be16 port)
{
	int i = 0;
	int empty_port = -1;
	struct nf_conntrack_helper *hlpr = NULL;
	char *tmpname = NULL;

	if ( port < 1 || port > 65535 )
		return -1;

	if (( RTSP_PORT == port ) || ( RTSP_PORT1 == port ))/* no need to add default port.*/
		return 1;

	for ( i = 1;  i < MAX_PORTS; i++ )
	{
		if ( -1 == empty_port && 0 == ports[i] )
			empty_port = i;
		else
		{
			if ( port == ports[i] ) /* port already exist*/
				return 2;
		}
	}

	if ( -1 != empty_port )
	{
		hlpr = &rtsp_helpers[empty_port];
		memset(hlpr, 0, sizeof(struct nf_conntrack_helper));
		hlpr->tuple.src.l3num = AF_INET;
		hlpr->tuple.src.u.tcp.port = htons(port);
		hlpr->tuple.dst.protonum = IPPROTO_TCP;
		hlpr->expect_policy = &rtsp_exp_policy;
		hlpr->me = THIS_MODULE;
		hlpr->help = help;

		tmpname = &rtsp_names[empty_port][0];
		snprintf(tmpname, 10, "rtsp-%d", i);
		snprintf(hlpr->name, 16, "%s", tmpname);
		/*hlpr->name = tmpname;*/

		if ( 0 != nf_conntrack_helper_register(hlpr) ) /* register succeed*/
		{
			pr_err("register new rtsp port #%d: %d failed!\n", empty_port, port);
			return -1;
		}
		hlpr = &rtsp_helpers[empty_port + MAX_PORTS];
		memset(hlpr, 0, sizeof(struct nf_conntrack_helper));
		hlpr->tuple.src.l3num = AF_INET6;
		hlpr->tuple.src.u.tcp.port = htons(port);
		hlpr->tuple.dst.protonum = IPPROTO_TCP;
		hlpr->expect_policy = &rtsp_exp_policy;
		hlpr->me = THIS_MODULE;
		hlpr->help = help;

		tmpname = &rtsp_names[empty_port + MAX_PORTS][0];
		snprintf(tmpname, 10, "rtspv6-%d", i);
		snprintf(hlpr->name, 16, "%s", tmpname);
		if ( 0 == nf_conntrack_helper_register(hlpr) ) /* register succeed*/
		{
			ports[empty_port] = port;
			pr_debug("register new rtsp port #%d: %d\n", empty_port, port);
			num_ports++;
		}
		else
		{
			pr_err("register new rtspv6 port #%d: %d failed!\n", empty_port, port);
			nf_conntrack_helper_unregister(&rtsp_helpers[empty_port]);
			return -1;
		}
	}

	return 0;
}
static int del_rtsp_alg_port(__be16 port)
{
	int i = 0;

	if ( port < 1 || port > 65535 )
		return -1;

	if (( RTSP_PORT == port ) || ( RTSP_PORT1 == port )) /* can't del default port.*/
		return 1;

	for ( i = 2;  i < MAX_PORTS; i++ )
	{
		if ( port == ports[i] )
		{
			nf_conntrack_helper_unregister(&rtsp_helpers[i]);
			nf_conntrack_helper_unregister(&rtsp_helpers[i + MAX_PORTS]);
			ports[i] = 0;
			num_ports--;
			pr_debug("unregistering rtsp port #%d: %d\n", i, port);
			break;
		}
	}

	return 0;
}

static int rtsp_alg_port_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	char buff[64] = {0}, cmd[8] = {0};
	__be32 len = 0;
	__be32 value = 0;

	if (count > 64)
		len = 64;
	else
		len = count;

	memset(buff, 0, 64);
	memset(cmd, 0, 8);
	if (copy_from_user(buff, buffer, len - 1))
		return -EFAULT;

	sscanf(buff,"%s %u", cmd, &value);

	if ( 0 == memcmp(cmd, "ADD", 3) )
		add_rtsp_alg_port(value);
	if ( 0 == memcmp(cmd, "DEL", 3) )
		del_rtsp_alg_port(value);
	
	return len;
}	


/* This function is intentionally _NOT_ defined as __exit */
static void
fini(void)
{
	int i;
	for (i = 0; i < MAX_PORTS; i++) {
		if (ports[i] != 0)
		{
			pr_debug("unregistering port %d\n", ports[i]);
			nf_conntrack_helper_unregister(&rtsp_helpers[i]);
			nf_conntrack_helper_unregister(&rtsp_helpers[i + MAX_PORTS]);
		}
	}
	kfree(rtsp_buffer);
	remove_proc_entry(RTSP_ALG_PORT_PATH,NULL);
}

static int __init
init(void)
{
	int i, ret;
	struct nf_conntrack_helper *hlpr;
	char *tmpname;
	struct proc_dir_entry *rtsp_port_proc = NULL;

	printk("nf_conntrack_rtsp v" IP_NF_RTSP_VERSION " loading\n");

	if (max_outstanding < 1) {
		printk("nf_conntrack_rtsp: max_outstanding must be a positive integer\n");
		return -EBUSY;
	}
	if (setup_timeout < 1) {
		printk("nf_conntrack_rtsp: setup_timeout must be a positive integer\n");
		return -EBUSY;
	}

	rtsp_exp_policy.max_expected = max_outstanding;
	rtsp_exp_policy.timeout = setup_timeout;
	
	rtsp_buffer = kmalloc(65536, GFP_KERNEL);
	if (!rtsp_buffer) 
		return -ENOMEM;

	/* If no port given, default to standard rtsp port */
	if (ports[0] == 0) {
		ports[0] = RTSP_PORT;
		num_ports = 1;
	}

	if (ports[1] == 0) {
		ports[1] = RTSP_PORT1;
		num_ports = num_ports + 1;
	}
	
	for (i = 0; (i < MAX_PORTS) && ports[i]; i++) {
		hlpr = &rtsp_helpers[i];
		memset(hlpr, 0, sizeof(struct nf_conntrack_helper));
		hlpr->tuple.src.l3num = AF_INET;
		hlpr->tuple.src.u.tcp.port = htons(ports[i]);
		hlpr->tuple.dst.protonum = IPPROTO_TCP;
		hlpr->expect_policy = &rtsp_exp_policy;
		hlpr->me = THIS_MODULE;
		hlpr->help = help;

		tmpname = &rtsp_names[i][0];
		if (ports[i] == RTSP_PORT) {
			sprintf(tmpname, "rtsp");
		} else {
			sprintf(tmpname, "rtsp-%d", i);
		}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0)
		strlcpy(hlpr->name, tmpname, sizeof(hlpr->name));
#else
		hlpr->name = tmpname;
#endif
		pr_debug("port #%d: %d\n", i, ports[i]);

		ret = nf_conntrack_helper_register(hlpr);

		if (ret) {
			printk("nf_conntrack_rtsp: ERROR registering port %d\n", ports[i]);
			fini();
			return -EBUSY;
		}

		hlpr = &rtsp_helpers[i+MAX_PORTS];
		memset(hlpr, 0, sizeof(struct nf_conntrack_helper));
		hlpr->tuple.src.l3num = AF_INET6;
		hlpr->tuple.src.u.tcp.port = htons(ports[i]);
		hlpr->tuple.dst.protonum = IPPROTO_TCP;
		hlpr->expect_policy = &rtsp_exp_policy;
		hlpr->me = THIS_MODULE;
		hlpr->help = help;

		tmpname = &rtsp_names[i+MAX_PORTS][0];
		if (ports[i] == RTSP_PORT) {
			sprintf(tmpname, "rtspv6");
		} else {
			sprintf(tmpname, "rtspv6-%d", i);
		}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0)
		strlcpy(hlpr->name, tmpname, sizeof(hlpr->name));
#else
		hlpr->name = tmpname;
#endif
		pr_debug("port #%d: %d\n", i, ports[i]);

		ret = nf_conntrack_helper_register(hlpr);

		if (ret) {
			printk("nf_conntrack_rtsp: ERROR registering port %d for v6\n", ports[i]);
			fini();
			return -EBUSY;
		}
	}

	rtsp_port_proc = create_proc_entry(RTSP_ALG_PORT_PATH, 0, NULL);
	rtsp_port_proc->read_proc = rtsp_alg_port_read_proc;
	rtsp_port_proc->write_proc = rtsp_alg_port_write_proc; 

	return 0;
}

module_init(init);
module_exit(fini);
