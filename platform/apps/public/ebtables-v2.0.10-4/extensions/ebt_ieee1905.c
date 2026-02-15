/*
 *  ebt_ieee1905
 *
 *      Authors:
 *      Chihwei <tp-link>
 *
 *  Oct, 2019
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "../include/ebtables_u.h"
#include <netinet/ether.h>
#include <linux/netfilter_bridge/ebt_ieee1905.h>

static int hex2byte(char *data_str, unsigned char *data_output, int len)
{
        char *src; 
	unsigned char *dest;
        unsigned char val;
        int idx;
        char hexstr[3];

        src = data_str;
        dest = data_output;

        for (idx = 0; idx < len; idx++) {
                hexstr[0] = src[0];
                hexstr[1] = src[1];
                hexstr[2] = '\0';

                val = strtoul(hexstr, NULL, 16);
                *dest++ = val;
                src += 2;
        }

        return 0;
}


#define IEEE1905_DATA '1'
#define IEEE1905_TARGET '2'
static struct option opts[] =
{
	{ "1905-data" ,    required_argument, 0, IEEE1905_DATA    },
	{ "1905-target" , required_argument, 0, IEEE1905_TARGET },
	{ 0 }
};

static void print_help()
{
	printf(
	"ieee1905 target options:\n"
	" --1905-data payload           : set 1905 data to filter packet\n"
	" --1905-target target         : ACCEPT, DROP, RETURN or CONTINUE\n"
	"                                    (standard target is ACCEPT)\n");
}

static void init(struct ebt_entry_target *target)
{
	struct ebt_ieee1905_info *ieee1905info =
	   (struct ebt_ieee1905_info *)target->data;

	memset(ieee1905info->payload, 0, PAYLOAD_MAX_LEN);
	ieee1905info->len = 0;
	ieee1905info->target = EBT_ACCEPT;
}

#define OPT_IEEE1905_DATA     0x01
#define OPT_IEEE1905_TARGET  0x02
static int parse(int c, char **argv, int argc,
   const struct ebt_u_entry *entry, unsigned int *flags,
   struct ebt_entry_target **target)
{
	struct ebt_ieee1905_info *ieee1905info =
	   (struct ebt_ieee1905_info *)(*target)->data;

	switch (c) {
	case IEEE1905_DATA:
		ebt_check_option2(flags, OPT_IEEE1905_DATA);
                if ((strlen(optarg)/2) > sizeof(ieee1905info->payload))
                        ebt_print_error2("payload too long, MAX is %d", PAYLOAD_MAX_LEN);

                if ((strlen(optarg)%2))
                        ebt_print_error2("payload should be even, not odd");

		if (hex2byte(optarg, ieee1905info->payload, strlen(optarg)/2))
                        ebt_print_error2("payload should be hex string");

		ieee1905info->len = strlen(optarg)/2;
		break;

	case IEEE1905_TARGET:
		ebt_check_option2(flags, OPT_IEEE1905_TARGET);
		if (FILL_TARGET(optarg, ieee1905info->target))
			ebt_print_error2("Illegal --1905-target target");
		break;

	default:
		return 0;
	}
	return 1;
}

static void final_check(const struct ebt_u_entry *entry,
   const struct ebt_entry_target *target, const char *name,
   unsigned int hookmask, unsigned int time)
{
//	struct ebt_ieee1905_info *ieee1905info =
//	   (struct ebt_ieee1905_info *)target->data;

        if (entry->ethproto != 0x893a)
                ebt_print_error("Only support proto 1905 , please add -p 0x893a");
}

static void print(const struct ebt_u_entry *entry,
   const struct ebt_entry_target *target)
{
	struct ebt_ieee1905_info *ieee1905info =
	   (struct ebt_ieee1905_info *)target->data;
	int idx = 0;

	printf("--1905-data ");
	for (idx = 0 ; idx < ieee1905info->len ; idx++)
		printf("%02x", ieee1905info->payload[idx]);
	printf(" --1905-target %s", TARGET_NAME(ieee1905info->target));
}

static int compare(const struct ebt_entry_target *t1,
   const struct ebt_entry_target *t2)
{
	struct ebt_ieee1905_info *ieee1905info1 =
	   (struct ebt_ieee1905_info *)t1->data;
	struct ebt_ieee1905_info *ieee1905info2 =
	   (struct ebt_ieee1905_info *)t2->data;

	return memcmp(ieee1905info1->payload, ieee1905info2->payload, ETH_ALEN) == 0
		&& ieee1905info1->target == ieee1905info2->target;
}

static struct ebt_u_target ieee1905_target =
{
	.name		= "ieee1905",
	.size		= sizeof(struct ebt_ieee1905_info),
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
	ebt_register_target(&ieee1905_target);
}
