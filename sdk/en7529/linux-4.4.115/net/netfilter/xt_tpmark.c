/*
 *	xt_tpmark - Netfilter module to match tplink private NFMARK value
 *
 *	Ren Jun <renjun@tp-link.com.cn>
 *
 */

#ifdef CONFIG_TP_IMAGE

#include <linux/module.h>
#include <linux/skbuff.h>

#include <linux/netfilter/xt_mark.h>
#include <linux/netfilter/x_tables.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ren Jun <renjun@tp-link.com.cn>");
MODULE_DESCRIPTION("Xtables: packet tpmark operations");
MODULE_ALIAS("ipt_tpmark");
MODULE_ALIAS("ip6t_tpmark");
MODULE_ALIAS("ipt_TPMARK");
MODULE_ALIAS("ip6t_TPMARK");

static unsigned int
mark_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
/* #define TCI_BITS 0xE000 */
	const struct xt_mark_tginfo2 *info = par->targinfo;
/*
 * brief	16 bit lan_vlan_tci to 32bit mark2
 * By	wangwenhao, 2Nov22
 */
#if 0
	unsigned short real_mark;
	unsigned short real_mask;
	unsigned short tci;

	if (info->mark >= 0x10000)
	{
		real_mark = (unsigned short)(info->mark >> 16);
		real_mask = (unsigned short)(info->mask >> 16);
	}
	else
	{
		real_mark = (unsigned short)(info->mark & 0xffff);
		real_mask = (unsigned short)(info->mask & 0xffff);
	}
	tci = skb->lan_vlan_tci & TCI_BITS;
	skb->lan_vlan_tci = (skb->lan_vlan_tci & ~real_mask) ^ real_mark;
	skb->lan_vlan_tci = (skb->lan_vlan_tci & ~TCI_BITS) | tci;

#undef TCI_BITS
#else
	skb->mark2 = (skb->mark2 & ~info->mask) ^ info->mark;
#endif

	return XT_CONTINUE;
}

static bool
mark_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct xt_mark_mtinfo1 *info = par->matchinfo;
/*
 * brief	16 bit lan_vlan_tci to 32bit mark2
 * By	wangwenhao, 2Nov22
 */
#if 0
	unsigned short real_mark;
	unsigned short real_mask;

	if (info->mark >= 0x10000)
	{
		real_mark = (unsigned short)(info->mark >> 16);
		real_mask = (unsigned short)(info->mask >> 16);
	}
	else
	{
		real_mark = (unsigned short)(info->mark & 0xffff);
		real_mask = (unsigned short)(info->mask & 0xffff);
	}
	
	return ((skb->lan_vlan_tci & real_mask) == real_mark) ^ info->invert;
#else
	return ((skb->mark2 & info->mask) == info->mark) ^ info->invert;
#endif

}

static bool mark_tg_check_v1(const struct xt_tgchk_param *par)
{
    const struct xt_mark_target_info_v1 *markinfo = par->targinfo;

    if (markinfo->mode != XT_MARK_SET
        && markinfo->mode != XT_MARK_AND
        && markinfo->mode != XT_MARK_OR) {
            printk(KERN_WARNING "MARK: unknown mode %u\n",
                   markinfo->mode);
            return false;
    }
    if (markinfo->mark > 0xffffffff) {
            printk(KERN_WARNING "MARK: Only supports 32bit wide mark\n");
            return false;
    }
    return true;
}

static unsigned int
mark_tg_v1(struct sk_buff *skb, const struct xt_action_param *par)
{
    const struct xt_mark_target_info_v1 *markinfo = par->targinfo;

/*
 * brief	16 bit lan_vlan_tci to 32bit mark2
 * By	wangwenhao, 2Nov22
 */
#if 0
	unsigned short real_mark;
#define TCI_BITS 0xE000

	if (markinfo->mark >= 0x10000)
	{
		real_mark = (unsigned short)(markinfo->mark >> 16);
	}
	else
	{
		real_mark = (unsigned short)(markinfo->mark & 0xffff);
	}

    switch (markinfo->mode) {
    case XT_MARK_SET:
            real_mark = real_mark;
            break;

    case XT_MARK_AND:
            real_mark = skb->lan_vlan_tci & real_mark;
            break;

    case XT_MARK_OR:
            real_mark = skb->lan_vlan_tci | real_mark;
            break;
    }
	skb->lan_vlan_tci = (skb->lan_vlan_tci & TCI_BITS) | real_mark;

#undef TCI_BITS
#else
	int mark = 0;

	switch (markinfo->mode)
	{
	case XT_MARK_SET:
		mark = markinfo->mark;
		break;

	case XT_MARK_AND:
		mark = skb->mark2 & markinfo->mark;
		break;

	case XT_MARK_OR:
		mark = skb->mark2 | markinfo->mark;
		break;
	}

	skb->mark2 = mark;
#endif

    return XT_CONTINUE;
}

static struct xt_target mark_tg_reg[] __read_mostly = {
	{
        .name           = "TPMARK",
        .family         = NFPROTO_UNSPEC,
        .revision       = 1,
        .checkentry     = mark_tg_check_v1,
        .target         = mark_tg_v1,
        .targetsize     = sizeof(struct xt_mark_target_info_v1),
        .me             = THIS_MODULE,
    },

    {
    	.name           = "TPMARK",
    	.revision       = 2,
    	.family         = NFPROTO_UNSPEC,
    	.target         = mark_tg,
    	.targetsize     = sizeof(struct xt_mark_tginfo2),
    	.me             = THIS_MODULE,
	}
};

static struct xt_match mark_mt_reg __read_mostly = {
	.name           = "tpmark",
	.revision       = 1,
	.family         = NFPROTO_UNSPEC,
	.match          = mark_mt,
	.matchsize      = sizeof(struct xt_mark_mtinfo1),
	.me             = THIS_MODULE,
};

static int __init mark_tpmt_init(void)
{
	int ret;
	
	ret = xt_register_targets(mark_tg_reg, ARRAY_SIZE(mark_tg_reg));

	if (ret < 0)
		return ret;
	ret = xt_register_match(&mark_mt_reg);
	if (ret < 0) {

        xt_unregister_targets(mark_tg_reg, ARRAY_SIZE(mark_tg_reg));

		return ret;
	}
	return 0;
}

static void __exit mark_tpmt_exit(void)
{
	xt_unregister_match(&mark_mt_reg);
	xt_unregister_targets(mark_tg_reg, ARRAY_SIZE(mark_tg_reg));

}

module_init(mark_tpmt_init);
module_exit(mark_tpmt_exit);

#endif /* CONFIG_TP_IMAGE */
