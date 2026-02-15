#ifndef _LINUX_ECNT_NETFILTER_IPV6_H
#define _LINUX_ECNT_NETFILTER_IPV6_H
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <ecnt_hook/ecnt_hook.h>
#include "../ecnt_net_ipv6.h"
#include <linux/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,4,0)
#include <net/netfilter/nf_nat_core.h>
#endif
static inline int ecnt_ip6t_mangle_out_inline_hook
(struct sk_buff *skb, const struct net_device *out)
{
#if defined(TCSUPPORT_CT)
	char if_buf[10];
	int if_index = 0;
#endif

	if ( !skb || !out )
		return ECNT_CONTINUE;

#if defined(TCSUPPORT_CT)
	memset(if_buf, 0, sizeof(if_buf));
	
		if ( strlen(out->name) > 3 )
		strlcpy(if_buf, out->name + 3,sizeof(if_buf));

		if ( out->name[0] == 'n' && strlen(if_buf) > 2 )
		{
			if_index = (if_buf[0] - '0') * MAX_PVC_NUM + (if_buf[2] - '0');
			skb->mark |= (if_index + 1) << 16;
		}
		else if (out->name[0] == 'p')
		{
			if_index = skip_atoi(if_buf);
			skb->mark |= (if_index + 1) << 16;
		}
	
#endif

	return ECNT_CONTINUE;
}

static inline int enct_ip6t_snpt_tg_inline_hook(struct sk_buff* skb)
{
	struct nf_nat_range range = {0};
	enum ip_conntrack_info ctinfo;
	struct nf_conn *ct;

	ct = nf_ct_get(skb, &ctinfo);
	if((ct && ctinfo == IP_CT_NEW) 
        &&(!nf_nat_initialized(ct, NF_NAT_MANIP_SRC)))
	{
		memcpy(&range.min_addr, &ipv6_hdr(skb)->saddr, sizeof(struct in6_addr));
		memcpy(&range.max_addr, &ipv6_hdr(skb)->saddr, sizeof(struct in6_addr));
		range.flags = NF_NAT_RANGE_MAP_IPS;

		nf_nat_setup_info(ct, &range, NF_NAT_MANIP_SRC);
	}

	return 0;
}

#endif

