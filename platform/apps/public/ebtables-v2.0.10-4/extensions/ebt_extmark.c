/* ebt_extmark
 *
 * Authors:
 * Bart De Schuymer <bdschuym@pandora.be>
 *
 * July, 2002, September 2006
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "../include/ebtables_u.h"
#include <linux/netfilter_bridge/ebt_extmark_t.h>

static int mark_supplied;

#define EXTMARK_TARGET  '1'
#define EXTMARK_SETEXTMARK '2'
#define EXTMARK_OREXTMARK  '3'
#define EXTMARK_ANDEXTMARK '4'
#define EXTMARK_XOREXTMARK '5'
static struct option opts[] =
{
	{ "extmark-target" , required_argument, 0, EXTMARK_TARGET },
	/* an oldtime messup, we should have always used the scheme
	 * <extension-name>-<option> */
	{ "set-extmark"    , required_argument, 0, EXTMARK_SETEXTMARK },
	{ "extmark-set"    , required_argument, 0, EXTMARK_SETEXTMARK },
	{ "extmark-or"     , required_argument, 0, EXTMARK_OREXTMARK  },
	{ "extmark-and"    , required_argument, 0, EXTMARK_ANDEXTMARK },
	{ "extmark-xor"    , required_argument, 0, EXTMARK_XOREXTMARK },
	{ 0 }
};

static void print_help()
{
	printf(
	"extmark target options:\n"
	" --extmark-set value     : Set nfextmark value\n"
	" --extmark-or  value     : Or nfextmark with value (nfextmark |= value)\n"
	" --extmark-and value     : And nfextmark with value (nfextmark &= value)\n"
	" --extmark-xor value     : Xor nfextmark with value (nfextmark ^= value)\n"
	" --extmark-target target : ACCEPT, DROP, RETURN or CONTINUE\n");
}

static void init(struct ebt_entry_target *target)
{
	struct ebt_extmark_t_info *extmarkinfo =
	   (struct ebt_extmark_t_info *)target->data;

	extmarkinfo->target = EBT_ACCEPT;
	extmarkinfo->extmark = 0;
	mark_supplied = 0;
}

#define OPT_EXTMARK_TARGET   0x01
#define OPT_EXTMARK_SETEXTMARK  0x02
#define OPT_EXTMARK_OREXTMARK   0x04
#define OPT_EXTMARK_ANDEXTMARK  0x08
#define OPT_EXTMARK_XOREXTMARK  0x10
static int parse(int c, char **argv, int argc,
   const struct ebt_u_entry *entry, unsigned int *flags,
   struct ebt_entry_target **target)
{
	struct ebt_extmark_t_info *extmarkinfo =
	   (struct ebt_extmark_t_info *)(*target)->data;
	char *end;

	switch (c) {
	case EXTMARK_TARGET:
		{ int tmp;
		ebt_check_option2(flags, OPT_EXTMARK_TARGET);
		if (FILL_TARGET(optarg, tmp))
			ebt_print_error2("Illegal --extmark-target target");
		/* the 4 lsb are left to designate the target */
		extmarkinfo->target = (extmarkinfo->target & ~EBT_VERDICT_BITS) | (tmp & EBT_VERDICT_BITS);
		}
		return 1;
	case EXTMARK_SETEXTMARK:
		ebt_check_option2(flags, OPT_EXTMARK_SETEXTMARK);
		if (*flags & (OPT_EXTMARK_OREXTMARK|OPT_EXTMARK_ANDEXTMARK|OPT_EXTMARK_XOREXTMARK))
			ebt_print_error2("--extmark-set cannot be used together with specific --extmark option");
                break;
	case EXTMARK_OREXTMARK:
		ebt_check_option2(flags, OPT_EXTMARK_OREXTMARK);
		if (*flags & (OPT_EXTMARK_SETEXTMARK|OPT_EXTMARK_ANDEXTMARK|OPT_EXTMARK_XOREXTMARK))
			ebt_print_error2("--extmark-or cannot be used together with specific --extmark option");
		extmarkinfo->target = (extmarkinfo->target & EBT_VERDICT_BITS) | EXTMARK_OR_VALUE;
                break;
	case EXTMARK_ANDEXTMARK:
		ebt_check_option2(flags, OPT_EXTMARK_ANDEXTMARK);
		if (*flags & (OPT_EXTMARK_SETEXTMARK|OPT_EXTMARK_OREXTMARK|OPT_EXTMARK_XOREXTMARK))
			ebt_print_error2("--extmark-and cannot be used together with specific --extmark option");
		extmarkinfo->target = (extmarkinfo->target & EBT_VERDICT_BITS) | EXTMARK_AND_VALUE;
                break;
	case EXTMARK_XOREXTMARK:
		ebt_check_option2(flags, OPT_EXTMARK_XOREXTMARK);
		if (*flags & (OPT_EXTMARK_SETEXTMARK|OPT_EXTMARK_ANDEXTMARK|OPT_EXTMARK_OREXTMARK))
			ebt_print_error2("--extmark-xor cannot be used together with specific --extmark option");
		extmarkinfo->target = (extmarkinfo->target & EBT_VERDICT_BITS) | EXTMARK_XOR_VALUE;
                break;
	 default:
		return 0;
	}
	/* mutual code */
	extmarkinfo->extmark = strtoul(optarg, &end, 0);
	if (*end != '\0' || end == optarg)
		ebt_print_error2("Bad EXTMARK value '%s'", optarg);
	mark_supplied = 1;
	return 1;
}

static void final_check(const struct ebt_u_entry *entry,
   const struct ebt_entry_target *target, const char *name,
   unsigned int hookmask, unsigned int time)
{
	struct ebt_extmark_t_info *extmarkinfo =
	   (struct ebt_extmark_t_info *)target->data;

	if (time == 0 && mark_supplied == 0) {
		ebt_print_error("No extmark value supplied");
	} else if (BASE_CHAIN && (extmarkinfo->target|~EBT_VERDICT_BITS) == EBT_RETURN)
		ebt_print_error("--extmark-target RETURN not allowed on base chain");
}

static void print(const struct ebt_u_entry *entry,
   const struct ebt_entry_target *target)
{
	struct ebt_extmark_t_info *extmarkinfo =
	   (struct ebt_extmark_t_info *)target->data;
	int tmp;

	tmp = extmarkinfo->target & ~EBT_VERDICT_BITS;
	if (tmp == EXTMARK_SET_VALUE)
		printf("--extmark-set");
	else if (tmp == EXTMARK_OR_VALUE)
		printf("--extmark-or");
	else if (tmp == EXTMARK_XOR_VALUE)
		printf("--extmark-xor");
	else if (tmp == EXTMARK_AND_VALUE)
		printf("--extmark-and");
	else
		ebt_print_error("oops, unknown extmark action, try a later version of ebtables");
	printf(" 0x%lx", extmarkinfo->extmark);
	tmp = extmarkinfo->target | ~EBT_VERDICT_BITS;
	printf(" --extmark-target %s", TARGET_NAME(tmp));
}

static int compare(const struct ebt_entry_target *t1,
   const struct ebt_entry_target *t2)
{
	struct ebt_extmark_t_info *extmarkinfo1 =
	   (struct ebt_extmark_t_info *)t1->data;
	struct ebt_extmark_t_info *extmarkinfo2 =
	   (struct ebt_extmark_t_info *)t2->data;

	return extmarkinfo1->target == extmarkinfo2->target &&
	   extmarkinfo1->extmark == extmarkinfo2->extmark;
}

static struct ebt_u_target extmark_target =
{
	.name		= "extmark",
	.size		= sizeof(struct ebt_extmark_t_info),
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
	ebt_register_target(&extmark_target);
}
