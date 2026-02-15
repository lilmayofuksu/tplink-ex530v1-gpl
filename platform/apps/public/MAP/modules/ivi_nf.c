/*************************************************************************
 *
 * ivi_nf.c :
 *
 * MAP-T/MAP-E Packet Processing based on Netfilter 
 *
 * Copyright (C) 2013 CERNET Network Center
 * All rights reserved.
 * 
 * Design and coding: 
 *   Xing Li <xing@cernet.edu.cn> 
 *	 Congxiao Bao <congxiao@cernet.edu.cn>
 * 	 Yuncheng Zhu <haoyu@cernet.edu.cn>
 * 	 Wentao Shang <wentaoshang@gmail.com>
 * 	 Guoliang Han <bupthgl@gmail.com>
 * 
 * Contributions:
 *
 * This file is part of MAP-T/MAP-E Kernel Module.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * You should have received a copy of the GNU General Public License 
 * along with MAP-T/MAP-E Kernel Module. If not, see 
 * <http://www.gnu.org/licenses/>.
 *
 * For more versions, please send an email to <bupthgl@gmail.com> to
 * obtain an password to access the svn server.
 *
 * LIC: GPLv2
 *
 ************************************************************************/

#include "ivi_nf.h"

struct net_device *v4_dev, *v6_dev;

static int running;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,3,0)
unsigned int nf_hook4(unsigned int hooknum, struct sk_buff *skb,
		const struct net_device *in, const struct net_device *out,
		int (*okfn)(struct sk_buff *))
#else
unsigned int nf_hook4(void *priv, struct sk_buff *skb,
				      const struct nf_hook_state *state)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,3,0)
	if ((!running) || (in != v4_dev)) {
#else
	if ((!running) || (state->in != v4_dev)) {
#endif
		return NF_ACCEPT;
	}

	if (ivi_v4v6_xmit(skb) == 0) {
		return NF_DROP;
	}
	else {
		return NF_ACCEPT;
	}
}
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,3,0)
unsigned int nf_hook6(unsigned int hooknum, struct sk_buff *skb,
		const struct net_device *in, const struct net_device *out,
		int (*okfn)(struct sk_buff *))
#else
unsigned int nf_hook6(void *priv, struct sk_buff *skb,
				      const struct nf_hook_state *state)
#endif
{

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,3,0)
	if ((!running) || (in != v6_dev)) {
#else
	if ((!running) || (state->in != v6_dev)) {
#endif
		return NF_ACCEPT;
	}

	if (ivi_v6v4_xmit(skb) == 0) {
		return NF_DROP;
	}
	else {
		return NF_ACCEPT;
	}
}

struct nf_hook_ops v4_ops = {
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4,12,14)
	list	:	{ NULL, NULL },
#endif
	hook	:	nf_hook4,
#if ( LINUX_VERSION_CODE < KERNEL_VERSION(4,4,1) )
	owner	:	THIS_MODULE,
#endif
	pf	:	PF_INET,
	hooknum	:	NF_INET_PRE_ROUTING,
	priority:	NF_IP_PRI_FIRST,
};

struct nf_hook_ops v6_ops = {
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4,12,14)
	list	:	{ NULL, NULL },
#endif
	hook	:	nf_hook6,
#if ( LINUX_VERSION_CODE < KERNEL_VERSION(4,4,1) )
	owner	:	THIS_MODULE,
#endif
	pf	:	PF_INET6,
	hooknum	:	NF_INET_PRE_ROUTING,
	priority:	NF_IP6_PRI_FIRST,
};

int nf_running(const int run) {
	running = run;
#ifdef IVI_DEBUG
	printk(KERN_DEBUG "nf_running: set running state to %d.\n", running);
#endif
	return running;
}

int nf_getv4dev(struct net_device *dev) {
	v4_dev = dev;
	return 0;
}

int nf_getv6dev(struct net_device *dev) {
	v6_dev = dev;
	return 0;
}

int ivi_nf_init(void) {
	running = 0;
	v4_dev = NULL;
	v6_dev = NULL;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,3,0)
	nf_register_hook(&v4_ops);
	nf_register_hook(&v6_ops);
#else
	nf_register_net_hook(&init_net,&v4_ops);
	nf_register_net_hook(&init_net,&v6_ops);
#endif

#ifdef IVI_DEBUG
	printk(KERN_DEBUG "IVI: ivi_nf loaded.\n");
#endif
	return 0;
}

void ivi_nf_exit(void) {
	running = 0;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,3,0)
	nf_unregister_hook(&v4_ops);
	nf_unregister_hook(&v6_ops);
#else
	nf_unregister_net_hook(&init_net,&v4_ops);
	nf_unregister_net_hook(&init_net,&v6_ops);
#endif
	
	if (v4_dev)
		dev_put(v4_dev);

	if (v6_dev)
		dev_put(v6_dev);

#ifdef IVI_DEBUG
	printk(KERN_DEBUG "IVI: ivi_nf unloaded.\n");
#endif
}
