/* $Id: addr_resolv_sock.c 3553 2011-05-05 06:14:19Z nanang $ */
/* 
 * Copyright (C) 2008-2011 Teluu Inc. (http://www.teluu.com)
 * Copyright (C) 2003-2008 Benny Prijono <benny@prijono.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */
#include <pj/addr_resolv.h>
#include <pj/assert.h>
#include <pj/string.h>
#include <pj/errno.h>
#include <pj/ip_helper.h>
#include <pj/compat/socket.h>

#if defined(PJ_GETADDRINFO_USE_CFHOST) && PJ_GETADDRINFO_USE_CFHOST!=0
#   include <CoreFoundation/CFString.h>
#   include <CFNetwork/CFHost.h>
#endif

/*<< BosaZhong@20Jun2014, add, dns socket with mark. */
#define PJ_USE_TP_DNS_RESOLVER				1
#define PJ_USE_TP_DNS_RESOLVER_IPV4_ONLY	1

#if defined(PJ_USE_TP_DNS_RESOLVER) && (0 == PJ_USE_TP_DNS_RESOLVER)
pj_status_t pj_gethostbyname(const pj_str_t *hostname, pj_hostent *phe)
{
    struct hostent *he;
    char copy[PJ_MAX_HOSTNAME];

    pj_assert(hostname && hostname ->slen < PJ_MAX_HOSTNAME);
    
    if (hostname->slen >= PJ_MAX_HOSTNAME)
	return PJ_ENAMETOOLONG;

    pj_memcpy(copy, hostname->ptr, hostname->slen);
    copy[ hostname->slen ] = '\0';

    he = gethostbyname(copy);
    if (!he) {
	return PJ_ERESOLVE;
	/* DO NOT use pj_get_netos_error() since host resolution error
	 * is reported in h_errno instead of errno!
	return pj_get_netos_error();
	 */
    }

    phe->h_name = he->h_name;
    phe->h_aliases = he->h_aliases;
    phe->h_addrtype = he->h_addrtype;
    phe->h_length = he->h_length;
    phe->h_addr_list = he->h_addr_list;

    return PJ_SUCCESS;
}
#else  /* 1 == PJ_USE_TP_DNS_RESOLVER */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <os_lib.h>

#ifndef UINT16
#define UINT16	unsigned short
#endif /* UINT16 */

#ifndef UINT8
#define UINT8		unsigned char
#endif 

#ifndef UINT32
#define UINT32	unsigned int
#endif 

#ifndef PJ_MAX_NAMESERVER
#define PJ_MAX_NAMESERVER		4
#endif 

#ifndef PJ_MAX_IPADDR_PER_NAME		
#define PJ_MAX_IPADDR_PER_NAME		4
#endif

#define		DNS_QUERY_FLAG				(0x0100)	/* query flag, standard query */
#define		DNS_QUERY_TRANSACTION_ID	(0x2)			//	(0x6400)	/* transact id	*/
#define		DNS_QUERY_TYPE				(0x0001)	/* type A */
#define		DNS_QUERY_CLASS				(0x0001)	/* class inet */
#define		DNS_HEADER_SIZE				12		/* dns header size in bytes (no include queries)*/
#define		DNS_SERVER_PORT				53		/* dns server port */
#define		DNS_DATA_SIZE				1500	/* buf size */
#define		DNS_LOCAL_PORT				531		/* dns local port */

#define		DNS_PKT_FLAG_RESPONSE		0x8000
#define		DNS_PKT_FLAG_REPLY			0x000f


#ifndef CDBG_NOTICE
//#define CDBG_NOTICE(format, args...)		printf("%s:%d " format "\n", __FILE__, __LINE__, ##args)
#define CDBG_NOTICE(format, args...)
#endif 

#ifndef CDBG_ERROR
#define CDBG_ERROR(format, args...)			printf(format"\n", ##args)
#endif 

#ifndef TT_DBG
//#define TT_DBG(format, args...)  printf("%s:%d [%s] " format "\n", __FILE__, __LINE__, __FUNCTION__, ##args)
#define TT_DBG(format, args...)
#endif


/* DNS query format */
typedef struct
{
	char			q_name[256];
	unsigned short	q_type;
	char			q_class;
}__attribute__ ((__packed__))DNS_QUERY_FIELD;

/* DNS packet */
typedef struct 
{
	UINT16 transaction_id;
	UINT16 flag;
	UINT16 questions;
	UINT16 answers_rrs;
	UINT16 authority_rrs;
	UINT16 additional_rrs;
	UINT8  data[1460];
}__attribute__ ((__packed__))INSIDE_DNS_PACKET;

typedef struct insideDnsStackStruct
{
	INSIDE_DNS_PACKET *packet;

	UINT16 localSrcPort;
	UINT16 transaction_id;
	UINT16 dns_query_len;			/* liaodaiguo add */

}INSIDE_DNS_STACK_STRUCT;


unsigned int l_routeMark = 0;
char l_dnsServers[PJ_MAX_NAMESERVER][PJ_MAX_HOSTNAME];
int  l_dnsCount = 0;

static UINT32 getIpFromDnsReplyPkt(INSIDE_DNS_PACKET* pkt, int dns_query_len)
{
	short	i;
	char	*p = (char *)pkt + 12 + dns_query_len; /*point to the ip field*/
	short	n_type, n_datalen;
	long	ret_ip = 0;

	for (i = 0; i < pkt->answers_rrs; i++)
	{
		if (*p != (char)0xc0 )	/* answer rrs is not type A or CNAME */
		{
			/* ignore domain name (several bytes) */
			while (*p != 0x00)		
				p++;
			p++;
			/* ignore type ,class, timelive  */
			/*			   type + class	+ timelive */
			p	=	p	+	2	+	2	+	4	;
			memcpy(&n_datalen, p, sizeof(n_datalen));
			p	=	p	+	sizeof(n_datalen);
			p	=	p	+	ntohs(n_datalen);
		}
		else	 
		{
			/* ignore domain name (2 bytes)*/
			p	+= sizeof(short);	
			/* get the type */
			memcpy(&n_type, p, sizeof(n_type));
			n_type	=	ntohs(n_type);
			/* ignore type ,class, timelive  */
			/*			   type + class	+ timelive */
			p	=	p	+	2	+	2	+	4	;
			memcpy(&n_datalen, p, sizeof(n_datalen));
			p	=	p	+	sizeof(n_datalen);
			if(n_type == 0x0001)	/* answer rrs type is A */
			{
				memcpy(&ret_ip, p, sizeof(ret_ip));
				break;
			}
			p	=	p	+	ntohs(n_datalen);
		}
	}
	return ret_ip;

}


static short generateQueryField(char *url, char *query)
{
	char * des	= query + 1;		/* point destion string for copy data */
	char * src	= url;				/* point source string for copy */
	char	n	= 0;				/* number */
	char * pn	= query;			/* point the number position of destion string */
	short	nCounter = des - query;	/* counter */
	unsigned short	ntype, nclass;
	
	if(!url || strlen(url) <= 0)
	{
		return 0;
	}
	
	while(*src !='\0'  && *src != '/')	
	{
		if(*src =='.')
		{
			*pn = n;
			pn = pn+n+1;
			n=0;
		}
		else
		{
			*des = *src;
			n++;
		}
		des++;
		src++;
		nCounter++;
	}
	
	if (n != 0)
	{
		*pn = n;
	}
	
	*des++='\0';
	nCounter++;
	ntype	= htons(DNS_QUERY_TYPE);		/* type */
	memcpy(des, &ntype, sizeof(ntype));
	des		+= sizeof(ntype);
	nclass	= htons(DNS_QUERY_CLASS);		/* class */
	memcpy(des, &nclass, sizeof(nclass));    
	nCounter = nCounter + sizeof(ntype) + sizeof(nclass);
	
	return nCounter;
}


/* 
 * note		the return value is network byte	
 */
UINT32 cnet_getHostByNameWithMark(char *pUrl, UINT32 dns, int timeOut, UINT32 mark)
{
	int lRet;							/*	ret value */

	UINT8 socketBuf[2048];

	INSIDE_DNS_STACK_STRUCT insideDnsStack;
	INSIDE_DNS_PACKET *pDnsResponse;	/*	dns pakcet point */
	DNS_QUERY_FIELD	dns_query;			/*	dns query to be constructed */
	UINT16 dns_query_len;				/*	dns query size */
	struct sockaddr_in addr_server;		/*	addr/port for sendto */

	UINT16 buf_len =0;					/*	buf len for sendto */
	fd_set rset;						/*	set for select */
	struct timeval tv;					/*	time for select */

	/* add by chengang ,2003/07/30 */
	/* Need fix "static"?? Yangxv */
	static UINT16 ltrans_id = DNS_QUERY_TRANSACTION_ID;

	int sockDns;
	struct sockaddr_in addr_local;
	struct sockaddr_in serverAddr;
	int    servAddrLen;

	struct in_addr in;
	UINT32 ulDns = dns;

#if 0
	UINT16 bindPort;
#endif /* 0 */

	/* First check if this is already an address */
    if (0 == os_inet_aton(pUrl, &in)) 
	{ 
        return in.s_addr;
    }

	if (0 == ulDns)
	{
		CDBG_NOTICE("Invalid DNS - %x\n", ulDns);
		return 0;
	}

	memset(&insideDnsStack, 0, sizeof(INSIDE_DNS_STACK_STRUCT));
	insideDnsStack.packet = (INSIDE_DNS_PACKET *)socketBuf;

	if (0 == ulDns ||NULL == pUrl || 0 == pUrl[0])	
	{
		return 0;
	}

	/* Socket init ... */
	sockDns = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockDns <= 0) 
	{
		return 0;
	}

    if (setsockopt(sockDns, SOL_SOCKET, SO_MARK, (void *)&mark, sizeof(mark)) < 0)
	{
		close(sockDns);
		CDBG_ERROR("setsockopt SO_MARK error.");
		perror("setsockopt SO_MARK error.");
		return -1;
	}

	/* Alloc managed port from pool. */
	insideDnsStack.localSrcPort = 0;

	/* Bind the DNS port to insideDnsStack->localSrcPort */
	addr_local.sin_family	= AF_INET;
	addr_local.sin_port		=  htons(insideDnsStack.localSrcPort);
	addr_local.sin_addr.s_addr = htonl(INADDR_ANY);
	lRet = bind(sockDns,( struct sockaddr *)&addr_local, sizeof(addr_local));
	
	/* if bind failed, try next port; if failed also, return 0 */
	if (lRet < 0)
	{
		close(sockDns);
		return 0;
	}
		
	/* init dns packet */
	memset(insideDnsStack.packet, 0, sizeof(INSIDE_DNS_PACKET));
	memset(&dns_query, 0, sizeof(dns_query));

	/* construct dns question packet */
	insideDnsStack.packet->flag = htons(DNS_QUERY_FLAG);			//standard query
	
	/* add by chengang ,2003/07/30 */
	ltrans_id  = (ltrans_id + 1) % 60000;
	insideDnsStack.packet->transaction_id = htons(ltrans_id);
	insideDnsStack.packet->questions = htons(1);
	
	dns_query_len = generateQueryField( pUrl, dns_query.q_name);	//construct queries
	
	if(dns_query_len == 0 )	//failure
	{
		close(sockDns);
		return 0 ;
	}
	
	memcpy(insideDnsStack.packet->data, &dns_query.q_name, dns_query_len);
	buf_len  = dns_query_len + DNS_HEADER_SIZE;	

	/* construct dns server addr/port */
	addr_server.sin_family		= AF_INET;
	addr_server.sin_port		= htons(DNS_SERVER_PORT);
	addr_server.sin_addr.s_addr = ulDns;


	/* Tx DNS query */
	lRet = sendto(sockDns,(char *)insideDnsStack.packet, 
					buf_len, 0, 
					(struct sockaddr*)&addr_server, sizeof(addr_server));

	if (-1 == lRet)
	{
		close(sockDns);
		return 0;
	}

	/**** Prepare to Rx DNS response ****/
	tv.tv_sec	= timeOut;
	tv.tv_usec	= 0;
	FD_ZERO(&rset);
	FD_SET(sockDns,&rset);
	lRet = select(sockDns + 1, &rset, NULL, NULL, &tv);
	
	if ((lRet != -1) && FD_ISSET(sockDns, &rset))
	{
		servAddrLen = sizeof(serverAddr);

		memset(insideDnsStack.packet, 0, sizeof(INSIDE_DNS_PACKET));

		
		lRet = recvfrom(sockDns, (UINT8 *)insideDnsStack.packet, 
						DNS_DATA_SIZE, 0, 
						(struct sockaddr *)&serverAddr, (socklen_t *)&servAddrLen);
		if (lRet < 0)
		{			
			close(sockDns);
			return 0;
		}
		
		pDnsResponse = insideDnsStack.packet;

#if _BYTE_ORDER == _LITTLE_ENDIAN
		CDBG_NOTICE("Little Endian\n");
		pDnsResponse->flag	= ntohs(pDnsResponse->flag);
#endif


		if(	pDnsResponse->transaction_id == insideDnsStack.packet->transaction_id && 	/* is response for pkt sended */
			(pDnsResponse->flag & DNS_PKT_FLAG_RESPONSE) > 0  &&						/* is response pkt */
			(pDnsResponse->flag & DNS_PKT_FLAG_REPLY) == 0  )		 					/* is correct response */
		{
			/* return the ip address (in network address )*/
			close(sockDns);
			return getIpFromDnsReplyPkt(pDnsResponse, dns_query_len);
		}
		else
		{
			close(sockDns);
			return 0;
		}
	}
	else
	{
		close(sockDns);
		return 0;
	}

	close(sockDns);

	return 0;
}

int cnet_addrStrToNum(const char *pStrAddr, UINT32 *pNumAddr)
{
	struct in_addr in;

	/* 因为Linux与vxWorks中inet_aton的返回值定义不同，所以封装了os_inet_aton()函数 */
	if (-1 == os_inet_aton(pStrAddr, &in))
	{
		if (pStrAddr[0] != '\0')
		{
			CDBG_ERROR("Error ip is %s\n", pStrAddr);
		}
		return -1;
	}

    *pNumAddr = in.s_addr;

	return 0;
}


static int getDnsServers(UINT32 * pDnsServers)
{
	int ret = 0;
	int number = 0;
	int idx = 0;
	UINT32 dns = 0;

	for (idx = 0, number = 0; idx < l_dnsCount&& idx < PJ_MAX_NAMESERVER; idx++)
	{
		ret = cnet_addrStrToNum(l_dnsServers[idx], &dns);

		if (ret < 0)
		{
			continue;
		}

		pDnsServers[number++] = dns;
	}	

	return number;
}

/*<< BosaZhong@22Jun2014, add, dns server. */
pj_status_t pj_nameserver_set_routemark(unsigned int routemark)
{
	l_routeMark = routemark;

	return PJ_SUCCESS;
}

pj_status_t pj_nameserver_set_nameserver(char * nameserver)
{
	if (strcmp(nameserver, ZERO_IPADDR))
	{
		strncpy(l_dnsServers[l_dnsCount++], nameserver, PJ_MAX_HOSTNAME);
	}

	return PJ_SUCCESS;
}

pj_status_t pj_nameserver_clear_nameservers(void)
{
	l_dnsCount = 0;

	return PJ_SUCCESS;
}

/*>> endof BosaZhong@22Jun2014, add, dns server. */
static char  l_name[PJ_MAX_HOSTNAME];

static struct hostent l_hoste;

#if 0 
Note: introduce
 	struct in_addr{
		in_addr_t s_addr;		/* 32-bit IPv4 address, network byte ordered. */
	}
#endif

static char l_buf[PJ_MAX_IPADDR_PER_NAME][sizeof(struct in_addr)];
static char * l_addrList[PJ_MAX_IPADDR_PER_NAME];
#if 0
			/*sizeof(char *)*ALIAS_DIM +*/ 384/*namebuffer*/ + 32/* margin */];
#endif


pj_status_t pj_gethostbyname(const pj_str_t *hostname, pj_hostent *phe)
{
    struct hostent *he;
    char copy[PJ_MAX_HOSTNAME];
	UINT32 ipTarget = 0;
	int idx = 0;
	int number = 0;
	UINT32 dnsServers[PJ_MAX_NAMESERVER] = { 0 };

    pj_assert(hostname && hostname ->slen < PJ_MAX_HOSTNAME);
    
    if (hostname->slen >= PJ_MAX_HOSTNAME)
	return PJ_ENAMETOOLONG;

    pj_memcpy(copy, hostname->ptr, hostname->slen);
    copy[ hostname->slen ] = '\0';

#if 0
    he = gethostbyname(copy);
#else
	number = getDnsServers(&dnsServers);
	
	for (idx = 0; idx < number; idx++)
	{
		ipTarget = cnet_getHostByNameWithMark(copy, dnsServers[idx], 3, l_routeMark);
		if (ipTarget != 0)
		{
			TT_DBG("own dns resolver(%s -> %u)", copy, ipTarget);
			break;
		}
	}

	if (0 == ipTarget)
	{
		/* failed. use default interface. */
		he = gethostbyname(copy);
	}
	else
	{
		/* create the he, refer to uclibc gethostbyname_r */
		he = &l_hoste;
		snprintf(l_name, PJ_MAX_HOSTNAME, "%s", copy);
		he->h_name = l_name;
		he->h_aliases = NULL;
		he->h_addrtype = AF_INET;
		he->h_length = sizeof(struct in_addr);
		*((unsigned int *)l_buf[0]) = ipTarget;
		l_addrList[0] = (char *) l_buf[0];
		for (idx = 1; idx < PJ_MAX_IPADDR_PER_NAME; idx++)
		{
			*((unsigned int *)l_buf[idx]) = (UINT32)0;
			l_addrList[idx] = NULL;
		}
		he->h_addr_list = l_addrList;
	}	
#endif 

    if (!he) {
	return PJ_ERESOLVE;
	/* DO NOT use pj_get_netos_error() since host resolution error
	 * is reported in h_errno instead of errno!
	return pj_get_netos_error();
	 */
    }

    phe->h_name = he->h_name;
    phe->h_aliases = he->h_aliases;
    phe->h_addrtype = he->h_addrtype;
    phe->h_length = he->h_length;
    phe->h_addr_list = he->h_addr_list;

    return PJ_SUCCESS;
}
#endif /* PJ_USE_TP_DNS_RESOLVER */
/*<< BosaZhong@20Jun2014, add, dns socket with mark. */


/* Resolve IPv4/IPv6 address */

/* Note & TODO: BosaZhong@22Jun2014, add.
**   we must use our dns resolver which is just supported IPv4, so there is IPv6 bug. 
** NOT use getaddrinfo.
*/
pj_status_t pj_getaddrinfo(int af, const pj_str_t *nodename,
				   unsigned *count, pj_addrinfo ai[])
{
#if defined(PJ_SOCK_HAS_GETADDRINFO) && (PJ_SOCK_HAS_GETADDRINFO!=0) \
 && defined(PJ_USE_TP_DNS_RESOLVER) && (PJ_USE_TP_DNS_RESOLVER == 0) \
 && defined(PJ_USE_TP_DNS_RESOLVER_IPV4_ONLY) && (PJ_USE_TP_DNS_RESOLVER_IPV4_ONLY == 0)
 
    char nodecopy[PJ_MAX_HOSTNAME];
    pj_bool_t has_addr = PJ_FALSE;
    unsigned i;
#if defined(PJ_GETADDRINFO_USE_CFHOST) && PJ_GETADDRINFO_USE_CFHOST!=0
    CFStringRef hostname;
    CFHostRef hostRef;
    pj_status_t status = PJ_SUCCESS;
#else
    int rc;
    struct addrinfo hint, *res, *orig_res;
#endif

    PJ_ASSERT_RETURN(nodename && count && *count && ai, PJ_EINVAL);
    PJ_ASSERT_RETURN(nodename->ptr && nodename->slen, PJ_EINVAL);
    PJ_ASSERT_RETURN(af==PJ_AF_INET || af==PJ_AF_INET6 || af==PJ_AF_UNSPEC, PJ_EINVAL);

    /* Check if nodename is IP address */
    pj_bzero(&ai[0], sizeof(ai[0]));
    if ((af==PJ_AF_INET || af==PJ_AF_UNSPEC) &&
	pj_inet_pton(PJ_AF_INET, nodename, &ai[0].ai_addr.ipv4.sin_addr) == PJ_SUCCESS)
    {
		af = PJ_AF_INET;
		has_addr = PJ_TRUE;
    }
	 else if ((af==PJ_AF_INET6 || af==PJ_AF_UNSPEC) &&
	       pj_inet_pton(PJ_AF_INET6, nodename, &ai[0].ai_addr.ipv6.sin6_addr) == PJ_SUCCESS)
    {
		af = PJ_AF_INET6;
		has_addr = PJ_TRUE;
    }

    if (has_addr)
	 {
		pj_str_t tmp;

		tmp.ptr = ai[0].ai_canonname;
		pj_strncpy_with_null(&tmp, nodename, PJ_MAX_HOSTNAME);
		ai[0].ai_addr.addr.sa_family = (pj_uint16_t)af;
		*count = 1;

		return PJ_SUCCESS;
    }


    /* Copy node name to null terminated string. */
    if (nodename->slen >= PJ_MAX_HOSTNAME)
	return PJ_ENAMETOOLONG;
	 
    pj_memcpy(nodecopy, nodename->ptr, nodename->slen);
    nodecopy[nodename->slen] = '\0';


#if defined(PJ_GETADDRINFO_USE_CFHOST) && PJ_GETADDRINFO_USE_CFHOST!=0
    hostname =  CFStringCreateWithCStringNoCopy(kCFAllocatorDefault, nodecopy,
						kCFStringEncodingASCII,
						kCFAllocatorNull);
    hostRef = CFHostCreateWithName(kCFAllocatorDefault, hostname);
    if (CFHostStartInfoResolution(hostRef, kCFHostAddresses, nil)) {
	CFArrayRef addrRef = CFHostGetAddressing(hostRef, nil);
	i = 0;
	if (addrRef != nil) {
	    CFIndex idx, naddr;
	    
	    naddr = CFArrayGetCount(addrRef);
	    for (idx = 0; idx < naddr && i < *count; idx++) {
		struct sockaddr *addr;
		
		addr = (struct sockaddr *)
		       CFDataGetBytePtr(CFArrayGetValueAtIndex(addrRef, idx));
		/* This should not happen. */
		pj_assert(addr);
		
		/* Ignore unwanted address families */
		if (af!=PJ_AF_UNSPEC && addr->sa_family != af)
		    continue;

		/* Store canonical name */
		pj_ansi_strcpy(ai[i].ai_canonname, nodecopy);
		
		/* Store address */
		PJ_ASSERT_ON_FAIL(sizeof(*addr) <= sizeof(pj_sockaddr),
				  continue);
		pj_memcpy(&ai[i].ai_addr, addr, sizeof(*addr));
		PJ_SOCKADDR_RESET_LEN(&ai[i].ai_addr);
		
		i++;
	    }
	}
	
	*count = i;
    } else {
	status = PJ_ERESOLVE;
    }
    
    CFRelease(hostRef);
    CFRelease(hostname);
    
    return status;
#else
    /* Call getaddrinfo() */
    pj_bzero(&hint, sizeof(hint));
    hint.ai_family = af;

    rc = getaddrinfo(nodecopy, NULL, &hint, &res);
    if (rc != 0)
	return PJ_ERESOLVE;

    orig_res = res;

    /* Enumerate each item in the result */
    for (i=0; i<*count && res; res=res->ai_next)
	 {
		/* Ignore unwanted address families */
		if (af!=PJ_AF_UNSPEC && res->ai_family != af)
	   	 continue;

		/* Store canonical name (possibly truncating the name) */
		if (res->ai_canonname)
		{
	   	pj_ansi_strncpy(ai[i].ai_canonname, res->ai_canonname, sizeof(ai[i].ai_canonname));
	    	ai[i].ai_canonname[sizeof(ai[i].ai_canonname)-1] = '\0';
		}
		else
		{
	   	pj_ansi_strcpy(ai[i].ai_canonname, nodecopy);
		}

		/* Store address */
		PJ_ASSERT_ON_FAIL(res->ai_addrlen <= sizeof(pj_sockaddr), continue);
		pj_memcpy(&ai[i].ai_addr, res->ai_addr, res->ai_addrlen);
		PJ_SOCKADDR_RESET_LEN(&ai[i].ai_addr);

		/* Next slot */
		++i;
    }

    *count = i;

    freeaddrinfo(orig_res);

    /* Done */
    return PJ_SUCCESS;
#endif

#else	/* PJ_SOCK_HAS_GETADDRINFO && 0 != PJ_USE_TP_DNS_RESOLVER && 0 != PJ_USE_TP_DNS_RESOLVER_IPV4_ONLY*/
    pj_bool_t has_addr = PJ_FALSE;

    PJ_ASSERT_RETURN(count && *count, PJ_EINVAL);

    /* Check if nodename is IP address */
    pj_bzero(&ai[0], sizeof(ai[0]));
    if ((af==PJ_AF_INET || af==PJ_AF_UNSPEC) &&
	pj_inet_pton(PJ_AF_INET, nodename,
		     &ai[0].ai_addr.ipv4.sin_addr) == PJ_SUCCESS)
    {
	af = PJ_AF_INET;
	has_addr = PJ_TRUE;
    }
    else if ((af==PJ_AF_INET6 || af==PJ_AF_UNSPEC) &&
	     pj_inet_pton(PJ_AF_INET6, nodename,
			  &ai[0].ai_addr.ipv6.sin6_addr) == PJ_SUCCESS)
    {
	af = PJ_AF_INET6;
	has_addr = PJ_TRUE;
    }

    if (has_addr) {
	pj_str_t tmp;

	tmp.ptr = ai[0].ai_canonname;
	pj_strncpy_with_null(&tmp, nodename, PJ_MAX_HOSTNAME);
	ai[0].ai_addr.addr.sa_family = (pj_uint16_t)af;
	*count = 1;

	return PJ_SUCCESS;
    }

    if (af == PJ_AF_INET || af == PJ_AF_UNSPEC) {
	pj_hostent he;
	unsigned i, max_count;
	pj_status_t status;
	
	/* VC6 complains that "he" is uninitialized */
	#ifdef _MSC_VER
	pj_bzero(&he, sizeof(he));
	#endif

	status = pj_gethostbyname(nodename, &he);
	if (status != PJ_SUCCESS)
	    return status;

	max_count = *count;
	*count = 0;

	pj_bzero(ai, max_count * sizeof(pj_addrinfo));

	for (i=0; he.h_addr_list[i] && *count<max_count; ++i) {
	    pj_ansi_strncpy(ai[*count].ai_canonname, he.h_name,
			    sizeof(ai[*count].ai_canonname));
	    ai[*count].ai_canonname[sizeof(ai[*count].ai_canonname)-1] = '\0';

	    ai[*count].ai_addr.ipv4.sin_family = PJ_AF_INET;
	    pj_memcpy(&ai[*count].ai_addr.ipv4.sin_addr,
		      he.h_addr_list[i], he.h_length);
	    PJ_SOCKADDR_RESET_LEN(&ai[*count].ai_addr);

	    (*count)++;
	}

	return PJ_SUCCESS;

    } else {
	/* IPv6 is not supported */
	*count = 0;

	return PJ_EIPV6NOTSUP;
    }
#endif	/* PJ_SOCK_HAS_GETADDRINFO */
}

