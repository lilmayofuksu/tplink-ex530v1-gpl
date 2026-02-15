/* ebt_mark
 *
 * Authors:renjun <renjun@tp-link.com.cn>
 * 
 *
 * 2017.10.30
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "../include/ebtables_u.h"
#include <linux/netfilter_bridge/ebt_mark_t.h>

static int mark_supplied;

#define MARK_TARGET  '1'
#define MARK_SETMARK '2'
#define MARK_ORMARK  '3'
#define MARK_ANDMARK '4'
#define MARK_XORMARK '5'
static struct option opts[] =
{
	/*add for LAN/WAN dinding,using tpmark instead of mark*/
	{ "tpmark-target" , required_argument, 0, MARK_TARGET },
	/* an oldtime messup, we should have always used the scheme
	 * <extension-name>-<option> */
	{ "tpset-mark"    , required_argument, 0, MARK_SETMARK },
	{ "tpmark-set"    , required_argument, 0, MARK_SETMARK },
	{ "tpmark-or"     , required_argument, 0, MARK_ORMARK  },
	{ "tpmark-and"    , required_argument, 0, MARK_ANDMARK },
	{ "tpmark-xor"    , required_argument, 0, MARK_XORMARK },
	{ 0 }
};

static void print_help()
{
	/*add for LAN/WAN dinding,using tpmark instead of mark*/
	printf(
	"mark target options:\n"
	" --tpmark-set value     : Set nfmark value\n"
	" --tpmark-or  value     : Or nfmark with value (nfmark |= value)\n"
	" --tpmark-and value     : And nfmark with value (nfmark &= value)\n"
	" --tpmark-xor value     : Xor nfmark with value (nfmark ^= value)\n"
	" --tpmark-target target : ACCEPT, DROP, RETURN or CONTINUE\n");
}

static void init(struct ebt_entry_target *target)
{
	struct ebt_mark_t_info *markinfo =
	   (struct ebt_mark_t_info *)target->data;

	markinfo->target = EBT_ACCEPT;
	markinfo->mark = 0;
	mark_supplied = 0;
}

#define OPT_MARK_TARGET   0x01
#define OPT_MARK_SETMARK  0x02
#define OPT_MARK_ORMARK   0x04
#define OPT_MARK_ANDMARK  0x08
#define OPT_MARK_XORMARK  0x10
static int parse(int c, char **argv, int argc,
   const struct ebt_u_entry *entry, unsigned int *flags,
   struct ebt_entry_target **target)
{
	struct ebt_mark_t_info *markinfo =
	   (struct ebt_mark_t_info *)(*target)->data;
	char *end;
	switch (c) {
	case MARK_TARGET:
		{ int tmp;
		ebt_check_option2(flags, OPT_MARK_TARGET);
		if (FILL_TARGET(optarg, tmp))
			ebt_print_error2("Illegal --mark-target target");
		/* the 4 lsb are left to designate the target */
		markinfo->target = (markinfo->target & ~EBT_VERDICT_BITS) | (tmp & EBT_VERDICT_BITS);
		}
		return 1;
	case MARK_SETMARK:
		ebt_check_option2(flags, OPT_MARK_SETMARK);
		if (*flags & (OPT_MARK_ORMARK|OPT_MARK_ANDMARK|OPT_MARK_XORMARK))
			ebt_print_error2("--mark-set cannot be used together with specific --mark option");
                break;
	case MARK_ORMARK:
		ebt_check_option2(flags, OPT_MARK_ORMARK);
		if (*flags & (OPT_MARK_SETMARK|OPT_MARK_ANDMARK|OPT_MARK_XORMARK))
			ebt_print_error2("--mark-or cannot be used together with specific --mark option");
		markinfo->target = (markinfo->target & EBT_VERDICT_BITS) | MARK_OR_VALUE;
                break;
	case MARK_ANDMARK:
		ebt_check_option2(flags, OPT_MARK_ANDMARK);
		if (*flags & (OPT_MARK_SETMARK|OPT_MARK_ORMARK|OPT_MARK_XORMARK))
			ebt_print_error2("--mark-and cannot be used together with specific --mark option");
		markinfo->target = (markinfo->target & EBT_VERDICT_BITS) | MARK_AND_VALUE;
                break;
	case MARK_XORMARK:
		ebt_check_option2(flags, OPT_MARK_XORMARK);
		if (*flags & (OPT_MARK_SETMARK|OPT_MARK_ANDMARK|OPT_MARK_ORMARK))
			ebt_print_error2("--mark-xor cannot be used together with specific --mark option");
		markinfo->target = (markinfo->target & EBT_VERDICT_BITS) | MARK_XOR_VALUE;
                break;
	 default:
		return 0;
	}
	/* mutual code */
	markinfo->mark = strtoul(optarg, &end, 0);
	if (*end != '\0' || end == optarg)
		ebt_print_error2("Bad MARK value '%s'", optarg);
	mark_supplied = 1;
	return 1;
}

static void final_check(const struct ebt_u_entry *entry,
   const struct ebt_entry_target *target, const char *name,
   unsigned int hookmask, unsigned int time)
{
	struct ebt_mark_t_info *markinfo =
	   (struct ebt_mark_t_info *)target->data;

	if (time == 0 && mark_supplied == 0) {
		ebt_print_error("No mark value supplied");
	} else if (BASE_CHAIN && (markinfo->target|~EBT_VERDICT_BITS) == EBT_RETURN)
		ebt_print_error("--tpmark-target RETURN not allowed on base chain");
}

static void print(const struct ebt_u_entry *entry,
   const struct ebt_entry_target *target)
{
	struct ebt_mark_t_info *markinfo =
	   (struct ebt_mark_t_info *)target->data;
	int tmp;

	tmp = markinfo->target & ~EBT_VERDICT_BITS;
	if (tmp == MARK_SET_VALUE)
		printf("--mark-set");
	else if (tmp == MARK_OR_VALUE)
		printf("--mark-or");
	else if (tmp == MARK_XOR_VALUE)
		printf("--mark-xor");
	else if (tmp == MARK_AND_VALUE)
		printf("--mark-and");
	else
		ebt_print_error("oops, unknown mark action, try a later version of ebtables");
	printf(" 0x%lx", markinfo->mark);
	tmp = markinfo->target | ~EBT_VERDICT_BITS;
	printf(" --mark-target %s", TARGET_NAME(tmp));
}

static int compare(const struct ebt_entry_target *t1,
   const struct ebt_entry_target *t2)
{
	struct ebt_mark_t_info *markinfo1 =
	   (struct ebt_mark_t_info *)t1->data;
	struct ebt_mark_t_info *markinfo2 =
	   (struct ebt_mark_t_info *)t2->data;

	return markinfo1->target == markinfo2->target &&
	   markinfo1->mark == markinfo2->mark;
}

static struct ebt_u_target mark_target =
{
	.name		= "tpmark",
	.size		= sizeof(struct ebt_mark_t_info),
	.help		= print_help,
	.init		= init,
	.parse		= parse,
	.final_check	= final_check,
	.print		= print,
	.compare	= compare,
	.extra_ops	= opts,
};

void _init(void)
{
	ebt_register_target(&mark_target);
}
