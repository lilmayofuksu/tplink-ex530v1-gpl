/*
 * RTCP extension for IP connection tracking
 * (C) 2003 by Tom Marshall <tmarshall at real.com>
 *
 * 2005-02-13: Harald Welte <laforge at netfilter.org>
 * 	- port to 2.6
 * 	- update to recent post-2.6.11 api changes
 * 2006-09-14: Steven Van Acker <deepstar at singularity.be>
 *	- removed calls to NAT code from conntrack helper: NAT no longer needed to use rtcp-conntrack
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
 *   insmod nf_conntrack_rtcp.o ports=port1,port2,...port<MAX_PORTS>
 *                              max_outstanding=n setup_timeout=secs
 *
 * If no ports are specified, the default will be port 554.
 *
 * With max_outstanding you can define the maximum number of not yet
 * answered SETUP requests per RTCP session (default 8).
 * With setup_timeout you can specify how long the system waits for
 * an expected data channel (default 300 seconds).
 *
 */

#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <linux/inet.h>
#include <net/udp.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_zones.h>
#include <linux/netfilter/nf_conntrack_rtcp.h>
#include <linux/proc_fs.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
#include <net/netfilter/nf_nat.h>
#else
#include <net/netfilter/nf_nat_rule.h>
#endif

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

#define RTCP_ALG_PORT_PATH "tc3162/rtcp_alg_port"

MODULE_AUTHOR("Tom Marshall <tmarshall at real.com>");
MODULE_DESCRIPTION("RTCP connection tracking module");
MODULE_LICENSE("GPL");
module_param_array(ports, int, &num_ports, 0400);
MODULE_PARM_DESC(ports, "port numbers of RTCP servers");
module_param(max_outstanding, int, 0400);
MODULE_PARM_DESC(max_outstanding, "max number of outstanding SETUP requests per RTCP session");
module_param(setup_timeout, int, 0400);
MODULE_PARM_DESC(setup_timeout, "timeout on for unestablished data channels");

static char *rtcp_buffer;
static DEFINE_SPINLOCK(rtcp_buffer_lock);

static struct nf_conntrack_expect_policy rtcp_exp_policy;
static struct nf_conntrack_helper rtcp_helpers[MAX_PORTS];
static char rtcp_names[MAX_PORTS][10];


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
	struct udphdr _udph, *uh=NULL;	
	int dir = CTINFO2DIR(ctinfo);   /* = IP_CT_DIR_ORIGINAL */
	char* pdata = rb_ptr;
	int ret = NF_ACCEPT;
    unsigned char fcc_fmt=0, fcc_fmt_offset=0;    
	struct nf_conntrack_expect *rtp_exp = NULL;
    
	/* Not whole UDP header? */
	uh = skb_header_pointer(skb, protoff, sizeof(_udph), &_udph);

	if (!uh)
		return NF_ACCEPT;
    
    fcc_fmt = *(pdata + fcc_fmt_offset) & 0x1f;
    printk("fcc_fmt=%x\n", fcc_fmt);
    
    /* fcc_fmt: 2 means client request, 3 means server reply, 4 means server synchronize, 5 means client stop*/
    if(fcc_fmt != 2)
        goto out;

    ct->loport = uh->source;
    ct->hiport = uh->source;

    rtp_exp = nf_ct_expect_alloc(ct);
    if (rtp_exp == NULL) {
        ret = NF_DROP;
        goto out;
    }

    nf_ct_expect_init(rtp_exp, NF_CT_EXPECT_CLASS_DEFAULT,
              nf_ct_l3num(ct),
              &ct->tuplehash[!dir].tuple.src.u3,
              &ct->tuplehash[!dir].tuple.dst.u3,
              IPPROTO_UDP, NULL, &uh->source);

    rtp_exp->flags = 0;
    printk("expect_related %pI4:%u-%pI4:%u\n",
            &rtp_exp->tuple.src.u3.ip,
            ntohs(rtp_exp->tuple.src.u.udp.port),
            &rtp_exp->tuple.dst.u3.ip,
            ntohs(rtp_exp->tuple.dst.u.udp.port));

    if (nf_ct_expect_related(rtp_exp) != 0) {
        pr_info("nf_conntrack_expect_related failed for rtp\n");
        ret = NF_DROP;
    }
    
    nf_ct_expect_put(rtp_exp);

out:
	return ret;
}


static inline int
help_in(struct sk_buff *skb, unsigned char *rb_ptr, unsigned int datalen,
	 struct nf_conn *ct, enum ip_conntrack_info ctinfo,
	 unsigned int protoff)
{
	struct udphdr _udph, *uh = NULL;
	int dir = CTINFO2DIR(ctinfo);
	char* pdata = rb_ptr;
	int ret = NF_ACCEPT;
	__be16 be_srcport = 0;
	__be32 newip = 0;
    unsigned char fcc_fmt = 0, fcc_fmt_offset = 0;
    unsigned char fcc_result = 0, fcc_result_offset = 12;
    unsigned char fcc_type = 0, fcc_type_offset = 13;
    unsigned short fcc_data_sport = 0, fcc_data_sport_offset = 16;
	typeof(nf_nat_rtp_hook) nf_nat_rtp;

	/* Not whole UDP header? */
	uh = skb_header_pointer(skb, protoff, sizeof(_udph), &_udph);

	if (!uh)
		return NF_ACCEPT;

    fcc_fmt = *(pdata + fcc_fmt_offset) & 0x1f;
    fcc_result = *(pdata + fcc_result_offset);
    fcc_type = *(pdata + fcc_type_offset);
    fcc_data_sport = *(unsigned short *)(pdata + fcc_data_sport_offset);
    
    /* fcc_fmt: 2 means client request, 3 means server reply, 4 means server synchronize, 5 means client stop*/
    if(fcc_fmt != 3)
		goto out;
    
    /* fcc_result: 0 means succeed, other means fail */
    if(fcc_result != 0)
		goto out;
    
    /* fcc_type: 1 means fcc refuse, 2 means fcc normal path, 3 means fcc redirect path */
    if(fcc_type != 2)
		goto out;
    
    fcc_data_sport = *(unsigned short *)(pdata + fcc_data_sport_offset);

    be_srcport = htons(fcc_data_sport);
    ct->loport = uh->dest;
    ct->hiport = uh->dest;
    newip = ct->tuplehash[dir].tuple.src.u3.ip;
    
    //init_rtp_expect(ct, ctinfo, ct->loport, be_srcport);
    
	nf_nat_rtp = rcu_dereference(nf_nat_rtp_hook);
	if (nf_nat_rtp && ct->status & IPS_NAT_MASK)
	{
		printk("\n=>nf_nat_rtp ip=%pI4,"
			" loport=[%d], hiport=[%d], src_port=[%d]\n"
			, &newip, ct->loport, ct->hiport, be_srcport);
		/* pass the request off to the rtp helper */
		ret = nf_nat_rtp(skb, ct, ctinfo, newip, be_srcport);
	}

out:
    return ret;

}

static int help(struct sk_buff *skb, unsigned int protoff,
		struct nf_conn *ct, enum ip_conntrack_info ctinfo) 
{
	struct udphdr _udph, *uh = NULL;
	unsigned int dataoff = 0, datalen = 0;
	char *rb_ptr = NULL;
	int ret = NF_DROP;

	/* Until there's been traffic both ways, don't look in packets. */
	if (ctinfo != IP_CT_ESTABLISHED && ctinfo != IP_CT_NEW &&
	    ctinfo != IP_CT_ESTABLISHED + IP_CT_IS_REPLY) {
		printk("conntrackinfo = %u\n", ctinfo);
		return NF_ACCEPT;
	}

	if ( ECNT_RETURN == ecnt_nf_conntrack_rtcp_help_inline_hook(skb, ct) )
		return NF_ACCEPT;

	/* Not whole UDP header? */
	uh = skb_header_pointer(skb, protoff, sizeof(_udph), &_udph);

	if (!uh)
		return NF_ACCEPT;
   
	/* No data ? */
	dataoff = protoff + sizeof(_udph);
	datalen = skb->len - dataoff;
	if (dataoff >= skb->len)
		return NF_ACCEPT;

	spin_lock_bh(&rtcp_buffer_lock);
	rb_ptr = skb_header_pointer(skb, dataoff,
				    skb->len - dataoff, rtcp_buffer);
	BUG_ON(rb_ptr == NULL);

	switch (CTINFO2DIR(ctinfo)) {
	case IP_CT_DIR_ORIGINAL:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
		ret = help_out(skb, rb_ptr, datalen, ct, ctinfo, protoff);
#else
		ret = help_out(skb, rb_ptr, datalen, ct, ctinfo);
#endif
		break;
	case IP_CT_DIR_REPLY:
		printk("IP_CT_DIR_REPLY\n");
		/* inbound packet: server->client */
		/*ret = NF_ACCEPT;*/
		ret = help_in(skb, rb_ptr, datalen, ct, ctinfo, protoff);
		break;
	}

	spin_unlock_bh(&rtcp_buffer_lock);

	return ret;
}

static int rtcp_alg_port_read_proc(char *buf, char **start, off_t off, int count,int *eof, void *data)
{
	int len = 0, i = 0;
	char port_buf[12] = {0};

	len += snprintf(buf, 15, "%s", "rtcp alg port:");
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

static int add_rtcp_alg_port(__be16 port)
{
	int i = 0;
	int empty_port = -1;
	struct nf_conntrack_helper *hlpr = NULL;
	char *tmpname = NULL;

	if ( port < 1 || port > 65535 )
		return -1;

	if ( RTCP_PORT == port )/* no need to add default port.*/
		return 1;

	for ( i = 1;  i < MAX_PORTS; i++ )
	{
		if ((-1 == empty_port) && (0 == ports[i]))
			empty_port = i;
		else
		{
			if ( port == ports[i] ) /* port already exist*/
				return 2;
		}
	}

	if ( -1 != empty_port )
	{
		hlpr = &rtcp_helpers[empty_port];
		memset(hlpr, 0, sizeof(struct nf_conntrack_helper));
		hlpr->tuple.src.l3num = AF_INET;
		hlpr->tuple.src.u.udp.port = htons(port);
		hlpr->tuple.dst.protonum = IPPROTO_UDP;
		hlpr->expect_policy = &rtcp_exp_policy;
		hlpr->me = THIS_MODULE;
		hlpr->help = help;

		tmpname = &rtcp_names[empty_port][0];
		snprintf(tmpname, 10, "rtcp-%d", empty_port);
		snprintf(hlpr->name, 16, "%s", tmpname);
		/*hlpr->name = tmpname;*/

		if ( 0 == nf_conntrack_helper_register(hlpr) ) /* register succeed*/
		{
			ports[empty_port] = port;
			printk("register new rtcp port #%d: %d\n", empty_port, port);
		}
	}

	return 0;
}
static int del_rtcp_alg_port(__be16 port)
{
	int i = 0;

	if ( port < 1 || port > 65535 )
		return -1;

	if ( RTCP_PORT == port ) /* can't del default port.*/
		return 1;

	for ( i = 1;  i < MAX_PORTS; i++ )
	{
		if ( port == ports[i] )
		{
			nf_conntrack_helper_unregister(&rtcp_helpers[i]);
			ports[i] = 0;
			printk("unregistering rtcp port #%d: %d\n", i, port);
			break;
		}
	}

	return 0;
}

static int rtcp_alg_port_write_proc(struct file *file, const char *buffer,
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
		add_rtcp_alg_port(value);
	if ( 0 == memcmp(cmd, "DEL", 3) )
		del_rtcp_alg_port(value);
	
	return len;
}	


/* This function is intentionally _NOT_ defined as __exit */
static void
fini(void)
{
	int i = 0;
	for (i = 0; i < num_ports; i++) {
		printk("unregistering port %d\n", ports[i]);
		nf_conntrack_helper_unregister(&rtcp_helpers[i]);
	}
	kfree(rtcp_buffer);
	remove_proc_entry(RTCP_ALG_PORT_PATH, NULL);
}

static int __init
init(void)
{
	int i = 0, ret = 0;
	struct nf_conntrack_helper *hlpr = NULL;
	char *tmpname = NULL;
	struct proc_dir_entry *rtcp_port_proc = NULL;

	printk("nf_conntrack_rtcp v" IP_NF_RTCP_VERSION " loading\n");

	if (max_outstanding < 1) {
		printk("nf_conntrack_rtcp: max_outstanding must be a positive integer\n");
		return -EBUSY;
	}
	if (setup_timeout < 0) {
		printk("nf_conntrack_rtcp: setup_timeout must be a positive integer\n");
		return -EBUSY;
	}

	rtcp_exp_policy.max_expected = max_outstanding;
	rtcp_exp_policy.timeout = setup_timeout;
	
	rtcp_buffer = kmalloc(65536, GFP_KERNEL);
	if (!rtcp_buffer) 
		return -ENOMEM;

	/* If no port given, default to standard rtcp port */
	if (ports[0] == 0) {
		ports[0] = RTCP_PORT;
		num_ports = 1;
	}
	
	for (i = 0; (i < MAX_PORTS) && ports[i]; i++) {
		hlpr = &rtcp_helpers[i];
		memset(hlpr, 0, sizeof(struct nf_conntrack_helper));
		hlpr->tuple.src.l3num = AF_INET;
		hlpr->tuple.src.u.udp.port = htons(ports[i]);
		hlpr->tuple.dst.protonum = IPPROTO_UDP;
		hlpr->expect_policy = &rtcp_exp_policy;
		hlpr->me = THIS_MODULE;
		hlpr->help = help;

		tmpname = &rtcp_names[i][0];
		if (ports[i] == RTCP_PORT) {
			sprintf(tmpname, "rtcp");
		} else {
			sprintf(tmpname, "rtcp-%d", i);
		}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0)
		strlcpy(hlpr->name, tmpname, sizeof(hlpr->name));
#else
		hlpr->name = tmpname;
#endif
		printk("port #%d: %d\n", i, ports[i]);

		ret = nf_conntrack_helper_register(hlpr);

		if (ret) {
			printk("nf_conntrack_rtcp: ERROR registering port %d\n", ports[i]);
			fini();
			return -EBUSY;
		}
	}

	rtcp_port_proc = create_proc_entry(RTCP_ALG_PORT_PATH, 0, NULL);
	rtcp_port_proc->read_proc = rtcp_alg_port_read_proc;
	rtcp_port_proc->write_proc = rtcp_alg_port_write_proc; 

	return 0;
}

module_init(init);
module_exit(fini);
