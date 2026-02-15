/*
 *  ebt_ip6
 *
 *	Authors:
 * Kuo-Lang Tseng <kuo-lang.tseng@intel.com>
 * Manohar Castelino <manohar.r.castelino@intel.com>
 *
 *  Jan 11, 2008
 *
 */

#ifndef __LINUX_BRIDGE_EBT_IP6_H
#define __LINUX_BRIDGE_EBT_IP6_H

#include <linux/types.h>

#define EBT_IP6_SOURCE 0x01
#define EBT_IP6_DEST 0x02
#define EBT_IP6_TCLASS 0x04
#define EBT_IP6_PROTO 0x08
#define EBT_IP6_SPORT 0x10
#define EBT_IP6_DPORT 0x20
#define EBT_IP6_ICMP6 0x40
#if defined(TCSUPPORT_CT_PORTSLIMIT)
#define EBT_IP6_ICMPV6TYPE  0x80
#else
#define EBT_IP6_ICMPV6TYPE  0
#endif
#if defined(TCSUPPORT_CT_JOYME4)
#define EBT_IP6_FLABEL 0x100
#else
#define EBT_IP6_FLABEL 0
#endif

#define EBT_IP6_MASK (EBT_IP6_SOURCE | EBT_IP6_DEST | EBT_IP6_TCLASS |\
		      EBT_IP6_PROTO | EBT_IP6_SPORT | EBT_IP6_DPORT | \
		      EBT_IP6_ICMP6 | EBT_IP6_ICMPV6TYPE | EBT_IP6_FLABEL)

#define EBT_IP6_MATCH "ip6"

/* the same values are used for the invflags */
struct ebt_ip6_info {
	struct in6_addr saddr;
	struct in6_addr daddr;
	struct in6_addr smsk;
	struct in6_addr dmsk;
#if defined(TCSUPPORT_ORN_EBTABLES)	
	__u8  tclass;
#else
	__u8  tclass[2];
#endif
#if defined(TCSUPPORT_ORN_EBTABLES)	
	__u8  protocol;
#else
	__u8  protocol[2];
#endif
#if defined(TCSUPPORT_CT_JOYME4)
	__u16  bitmask;
#else
	__u8  bitmask;
#endif
	__u8  invflags;
	union {
		__u16 sport[2];
		__u8 icmpv6_type[2];
	};
	union {
		__u16 dport[2];
		__u8 icmpv6_code[2];
	};
#if defined(TCSUPPORT_CT_PORTSLIMIT)
	__u8  icmpv6type;
#endif
#if defined(TCSUPPORT_CT_JOYME4)
	__u32 flowlabel;
#endif
};

#endif
