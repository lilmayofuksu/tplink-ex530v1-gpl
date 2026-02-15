/*
 *  ebt_ieee1905
 *
 *	Authors:
 *	Chihwei <tp-link>
 *
 *  Oct, 2019
 *
 */
#include <linux/module.h>
#include <net/sock.h>
#include <linux/netfilter.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_ieee1905.h>

static unsigned int
ebt_ieee1905_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct ebt_ieee1905_info *info = par->targinfo;
	unsigned char *ie = skb_network_header(skb);

	if(info->len == 0 || info->len > PAYLOAD_MAX_LEN)
		return EBT_CONTINUE;

	if(memcmp(ie, info->payload , info->len))
		return EBT_CONTINUE;

	return info->target;
}

static int ebt_ieee1905_tg_check(const struct xt_tgchk_param *par)
{
//	const struct ebt_ieee1905_info *info = par->targinfo;

	return 0;
}

static struct xt_target ebt_ieee1905_tg_reg __read_mostly = {
	.name		= "ieee1905",
	.revision	= 0,
	.family		= NFPROTO_BRIDGE,
	.hooks		= (1 << NF_BR_NUMHOOKS) | (1 << NF_BR_PRE_ROUTING) | (1 << NF_BR_LOCAL_IN) |
			  (1 << NF_BR_LOCAL_OUT) | (1 << NF_BR_POST_ROUTING) |(1 << NF_BR_BROUTING),
	.target		= ebt_ieee1905_tg,
	.checkentry	= ebt_ieee1905_tg_check,
	.targetsize	= sizeof(struct ebt_ieee1905_info),
	.me		= THIS_MODULE,
};

static int __init ebt_ieee1905_init(void)
{
	return xt_register_target(&ebt_ieee1905_tg_reg);
}

static void __exit ebt_ieee1905_fini(void)
{
	xt_unregister_target(&ebt_ieee1905_tg_reg);
}

module_init(ebt_ieee1905_init);
module_exit(ebt_ieee1905_fini);
MODULE_DESCRIPTION("Ebtables: 1905 Vendor Specific Content Filter");
MODULE_LICENSE("GPL");
