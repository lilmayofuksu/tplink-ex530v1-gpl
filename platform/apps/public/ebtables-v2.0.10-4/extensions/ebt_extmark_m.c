/* ebt_extmark_m
 *
 * Authors:
 * Bart De Schuymer <bdschuym@pandora.be>
 *
 * July, 2002
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "../include/ebtables_u.h"
#include <linux/netfilter_bridge/ebt_extmark_m.h>

#define EXTMARK '1'

static struct option opts[] =
{
	{ "extmark", required_argument, 0, EXTMARK },
	{ 0 }
};

static void print_help()
{
	printf(
"extmark option:\n"
"--extmark    [!] [value][/mask]: Match nfmask value (see man page)\n");
}

static void init(struct ebt_entry_match *match)
{
	struct ebt_extmark_m_info *extmarkinfo = (struct ebt_extmark_m_info *)match->data;

	extmarkinfo->extmark    = 0;
	extmarkinfo->mask    = 0;
	extmarkinfo->invert  = 0;
	extmarkinfo->bitmask = 0;
}

#define OPT_EXTMARK 0x01
static int parse(int c, char **argv, int argc, const struct ebt_u_entry *entry,
   unsigned int *flags, struct ebt_entry_match **match)
{
	struct ebt_extmark_m_info *extmarkinfo = (struct ebt_extmark_m_info *)
	   (*match)->data;
	char *end;

	switch (c) {
	case EXTMARK:
		ebt_check_option2(flags, EXTMARK);
		if (ebt_check_inverse2(optarg))
			extmarkinfo->invert = 1;
		extmarkinfo->extmark = strtoul(optarg, &end, 0);
		extmarkinfo->bitmask = EBT_EXTMARK_AND;
		if (*end == '/') {
			if (end == optarg)
				extmarkinfo->bitmask = EBT_EXTMARK_OR;
			extmarkinfo->mask = strtoul(end+1, &end, 0);
		} else
			extmarkinfo->mask = 0xffffffff;
		if ( *end != '\0' || end == optarg)
			ebt_print_error2("Bad extmark value '%s'", optarg);
		break;
	default:
		return 0;
	}
	return 1;
}

static void final_check(const struct ebt_u_entry *entry,
   const struct ebt_entry_match *match, const char *name,
   unsigned int hookmask, unsigned int time)
{
}

static void print(const struct ebt_u_entry *entry,
   const struct ebt_entry_match *match)
{
	struct ebt_extmark_m_info *extmarkinfo =
	   (struct ebt_extmark_m_info *)match->data;

	printf("--extmark ");
	if (extmarkinfo->invert)
		printf("! ");
	if (extmarkinfo->bitmask == EBT_EXTMARK_OR)
		printf("/0x%lx ", extmarkinfo->mask);
	else if(extmarkinfo->mask != 0xffffffff)
		printf("0x%lx/0x%lx ", extmarkinfo->extmark, extmarkinfo->mask);
	else
		printf("0x%lx ", extmarkinfo->extmark);
}

static int compare(const struct ebt_entry_match *m1,
   const struct ebt_entry_match *m2)
{
	struct ebt_extmark_m_info *extmarkinfo1 = (struct ebt_extmark_m_info *)m1->data;
	struct ebt_extmark_m_info *extmarkinfo2 = (struct ebt_extmark_m_info *)m2->data;

	if (extmarkinfo1->invert != extmarkinfo2->invert)
		return 0;
	if (extmarkinfo1->extmark != extmarkinfo2->extmark)
		return 0;
	if (extmarkinfo1->mask != extmarkinfo2->mask)
		return 0;
	if (extmarkinfo1->bitmask != extmarkinfo2->bitmask)
		return 0;
	return 1;
}

static struct ebt_u_match extmark_match =
{
	.name		= "extmark_m",
	.size		= sizeof(struct ebt_extmark_m_info),
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
	ebt_register_match(&extmark_match);
}
