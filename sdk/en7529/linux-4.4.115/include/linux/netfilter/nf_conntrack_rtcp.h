/*
 * RTCP extension for IP connection tracking.
 * (C) 2003 by Tom Marshall <tmarshall at real.com>
 * based on ip_conntrack_irc.h
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 * 2013-03-04: Il'inykh Sergey <sergeyi at inango-sw.com>. Inango Systems Ltd
 *	- conditional compilation for kernel 3.7
 *	- port mapping improvements
 */
#ifndef _IP_CONNTRACK_RTCP_H
#define _IP_CONNTRACK_RTCP_H

#include <linux/version.h>

#define IP_NF_RTCP_VERSION "0.7"

#ifdef __KERNEL__

extern unsigned int (*nf_nat_rtp_hook)(struct sk_buff *skb,
				struct nf_conn *ct,
				 enum ip_conntrack_info ctinfo,
				 __be32 rtpip,
				 u_int16_t rtp_srcport);

#define RTCP_PORT   8027

#endif /* __KERNEL__ */

#endif /* _IP_CONNTRACK_RTCP_H */
