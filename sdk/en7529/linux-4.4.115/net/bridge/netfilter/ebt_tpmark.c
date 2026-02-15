/*
 *  ebt_tpmark
 *
 *  Authors: renjun 
 *
 *  2017.10.30
 *
 */

/* The tpmark target can be used in any chain
 */

 #ifdef CONFIG_TP_IMAGE

#include <linux/module.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_mark_t.h>

static unsigned int
ebt_tpmark_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct ebt_mark_t_info *info = par->targinfo;
	int action = info->target & -16;

/*
 * brief	16 bit lan_vlan_tci to 32bit mark2
 * By	wangwenhao, 2Nov22
 */
#if 0
	unsigned short real_mark;
	unsigned short tci;
#define TCI_BITS 0xE000
	if (info->mark >= 0x10000)
	{
		real_mark = (unsigned short)(info->mark >> 16);
	}
	else
	{
		real_mark = (unsigned short)(info->mark & 0xffff);
	}
	tci = skb->lan_vlan_tci & TCI_BITS;
    
	if (action == MARK_SET_VALUE)
		skb->lan_vlan_tci = real_mark;
	else if (action == MARK_OR_VALUE)
		skb->lan_vlan_tci |= real_mark;
	else if (action == MARK_AND_VALUE)
		skb->lan_vlan_tci &= real_mark;
	else
		skb->lan_vlan_tci ^= real_mark;
	skb->lan_vlan_tci = (skb->lan_vlan_tci & ~TCI_BITS) | tci;
    
#undef TCI_BITS
#else
	if (action == MARK_SET_VALUE)
	{
		skb->mark2 = info->mark;
	}
	else if (action == MARK_OR_VALUE)
	{
		skb->mark2 |= info->mark;
	}
	else if (action == MARK_AND_VALUE)
	{
		skb->mark2 &= info->mark;
	}
	else
	{
		skb->mark2 ^= info->mark;
	}
#endif

	return info->target | ~EBT_VERDICT_BITS;
}

static int ebt_tpmark_tg_check(const struct xt_tgchk_param *par)
{
	const struct ebt_mark_t_info *info = par->targinfo;
	int tmp;

	tmp = info->target | ~EBT_VERDICT_BITS;
	if (BASE_CHAIN && tmp == EBT_RETURN)
		return -EINVAL;
	if (tmp < -NUM_STANDARD_TARGETS || tmp >= 0)
		return -EINVAL;
	tmp = info->target & ~EBT_VERDICT_BITS;
	if (tmp != MARK_SET_VALUE && tmp != MARK_OR_VALUE &&
	    tmp != MARK_AND_VALUE && tmp != MARK_XOR_VALUE)
		return -EINVAL;
	return 0;
}
#ifdef CONFIG_COMPAT
struct compat_ebt_mark_t_info {
	compat_ulong_t mark;
	compat_uint_t target;
};

static void tpmark_tg_compat_from_user(void *dst, const void *src)
{
	const struct compat_ebt_mark_t_info *user = src;
	struct ebt_mark_t_info *kern = dst;

	kern->mark = user->mark;
	kern->target = user->target;
}

static int tpmark_tg_compat_to_user(void __user *dst, const void *src)
{
	struct compat_ebt_mark_t_info __user *user = dst;
	const struct ebt_mark_t_info *kern = src;

	if (put_user(kern->mark, &user->mark) ||
	    put_user(kern->target, &user->target))
		return -EFAULT;
	return 0;
}
#endif

static struct xt_target ebt_tpmark_tg_reg __read_mostly = {
	.name		= "tpmark",
	.revision	= 0,
	.family		= NFPROTO_BRIDGE,
	.target		= ebt_tpmark_tg,
	.checkentry	= ebt_tpmark_tg_check,
	.targetsize	= sizeof(struct ebt_mark_t_info),
#ifdef CONFIG_COMPAT
	.compatsize	= sizeof(struct compat_ebt_mark_t_info),
	.compat_from_user = tpmark_tg_compat_from_user,
	.compat_to_user	= tpmark_tg_compat_to_user,
#endif
	.me		= THIS_MODULE,
};

static int __init ebt_tpmark_init(void)
{
	return xt_register_target(&ebt_tpmark_tg_reg);
}

static void __exit ebt_tpmark_fini(void)
{
	xt_unregister_target(&ebt_tpmark_tg_reg);
}

module_init(ebt_tpmark_init);
module_exit(ebt_tpmark_fini);
MODULE_DESCRIPTION("Ebtables: Packet tpmark modification");
MODULE_LICENSE("GPL");

#endif /* CONFIG_TP_IMAGE */
