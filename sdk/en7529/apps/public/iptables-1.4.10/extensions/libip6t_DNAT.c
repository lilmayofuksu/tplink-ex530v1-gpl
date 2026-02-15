/*
 * Copyright (c) 2011 Patrick McHardy <kaber@trash.net>
 *
 * Based on Rusty Russell's IPv4 DNAT target. Development of IPv6 NAT
 * funded by Astaro.
 */

#include <stdbool.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <xtables.h>
#include <iptables.h> /* get_kernel_version */
#include <limits.h> /* INT_MAX in ip_tables.h */
#include <linux/netfilter_ipv6/ip6_tables.h>
#include <net/netfilter/nf_nat.h>

struct nf_nat_ipv6_range {
	unsigned int			flags;
	union nf_inet_addr		min_addr;
	union nf_inet_addr		max_addr;
	union nf_conntrack_man_proto	min_proto;
	union nf_conntrack_man_proto	max_proto;
};

#define NF_NAT_RANGE_MAP_IPS			(1 << 0)
#define NF_NAT_RANGE_PROTO_SPECIFIED		(1 << 1)
#define NF_NAT_RANGE_PROTO_RANDOM		(1 << 2)
#define NF_NAT_RANGE_PERSISTENT			(1 << 3)
#define NF_NAT_RANGE_PROTO_RANDOM_FULLY		(1 << 4)

#define IPT_DNAT_OPT_DEST 0x1
#define IPT_DNAT_OPT_RANDOM 0x2

/* Dest NAT data consists of a multi-range, indicating where to map
   to. */
struct ip6t_natinfo
{
	struct xt_entry_target t;
	struct nf_nat_ipv6_range range;
};

enum {
	O_TO_DEST = 0,
	O_RANDOM,
	O_PERSISTENT,
	O_X_TO_DEST,
	F_TO_DEST   = 1 << O_TO_DEST,
	F_RANDOM   = 1 << O_RANDOM,
	F_X_TO_DEST = 1 << O_X_TO_DEST,
};

static void DNAT_help(void)
{
	printf(
"DNAT target options:\n"
" --to-destination [<ipaddr>[-<ipaddr>]][:port[-port]]\n"
"				Address to map destination to.\n"
"[--random] [--persistent]\n");
}

static const struct option DNAT_opts[] = {
	{.name = "to-destination", .has_arg = true,  .val = '1'},
	{.name = "random",         .has_arg = false, .val = '2'},
	{.name = "persistent",     .has_arg = false, .val = '3'},
	XT_GETOPT_TABLEEND,
};


static struct ip6t_natinfo *
append_range(struct ip6t_natinfo *info, const struct nf_nat_ipv6_range *range)
{
	unsigned int size;

	/* One rangesize already in struct ip6t_natinfo */
	size = XT_ALIGN(sizeof(*info));
	
	info = realloc(info, size);
	if (!info)
		xtables_error(OTHER_PROBLEM, "Out of memory\n");

	info->t.u.target_size = size;
	info->range = *range;

	return info;
}


/* Ranges expected in network order. */
static struct xt_entry_target *
parse_to(char *arg, int portok, struct ip6t_natinfo *info)
{
	struct nf_nat_ipv6_range range;
	char *start, *end = NULL, *colon = NULL, *dash, *error;
	const struct in6_addr *ip;

	memset(&range, 0, sizeof(range));

	start = strchr(arg, '[');
	if (start == NULL) {
		start = arg;
		/* Lets assume one colon is port information. Otherwise its an IPv6 address */
		colon = strchr(arg, ':');
		if (colon && strchr(colon+1, ':'))
			colon = NULL;
	}
	else 
	{
		start++;
		end = strchr(start, ']');
		if (end == NULL)
			xtables_error(PARAMETER_PROBLEM,
				      "Invalid address format");

		*end = '\0';
		colon = strchr(end + 1, ':');
	}

	if (colon) {
		int port;

		if (!portok)
			xtables_error(PARAMETER_PROBLEM,
				   "Need TCP, UDP, SCTP or DCCP with port specification");

		range.flags |= NF_NAT_RANGE_PROTO_SPECIFIED;

		port = atoi(colon+1);
		if (port <= 0 || port > 65535)
			xtables_error(PARAMETER_PROBLEM,
				   "Port `%s' not valid\n", colon+1);

		error = strchr(colon+1, ':');
		if (error)
			xtables_error(PARAMETER_PROBLEM,
				   "Invalid port:port syntax - use dash\n");

		dash = strchr(colon, '-');
		if (!dash) {
			range.min_proto.tcp.port
				= range.max_proto.tcp.port
				= htons(port);
		} else {
			int maxport;

			maxport = atoi(dash + 1);
			if (maxport <= 0 || maxport > 65535)
				xtables_error(PARAMETER_PROBLEM,
					   "Port `%s' not valid\n", dash+1);
			if (maxport < port)
				/* People are stupid. */
				xtables_error(PARAMETER_PROBLEM,
					   "Port range `%s' funky\n", colon+1);
			range.min_proto.tcp.port = htons(port);
			range.max_proto.tcp.port = htons(maxport);
		}
		/* Starts with colon or [] colon? No IP info...*/
		if (colon == arg || colon == arg+2) {
			return &(append_range(info, &range)->t);
		}
		*colon = '\0';
	}

	range.flags |= NF_NAT_RANGE_MAP_IPS;
	dash = strchr(start, '-');
	if (colon && dash && dash > colon)
		dash = NULL;

	if (dash)
		*dash = '\0';

	ip = xtables_numeric_to_ip6addr(start);
	if (!ip)
		xtables_error(PARAMETER_PROBLEM, "Bad IP address \"%s\"\n",
			      start);
	range.min_addr.in6 = *ip;
	if (dash) {
		ip = xtables_numeric_to_ip6addr(dash + 1);
		if (!ip)
			xtables_error(PARAMETER_PROBLEM, "Bad IP address \"%s\"\n",
				      dash+1);
		range.max_addr.in6 = *ip;
	} else
		range.max_addr = range.min_addr;
	
	return &(append_range(info, &range)->t);
}

static int DNAT_parse(int c, char **argv, int invert, unsigned int *flags,
                      const void *e, struct xt_entry_target **target)
{
	const struct ip6t_entry *entry = e;
	struct ip6t_natinfo *info = (void *)*target;
	struct nf_nat_ipv6_range range = info->range;
	
	int portok;
		
	if (entry->ipv6.proto == IPPROTO_TCP ||
	    entry->ipv6.proto == IPPROTO_UDP ||
	    entry->ipv6.proto == IPPROTO_SCTP ||
	    entry->ipv6.proto == IPPROTO_DCCP ||
	    entry->ipv6.proto == IPPROTO_ICMP)
		portok = 1;
	else
		portok = 0;
	
	switch (c) {
	case '1':
		if (xtables_check_inverse(optarg, &invert, NULL, 0, argv))
			xtables_error(PARAMETER_PROBLEM,
				   "Unexpected `!' after --to-destination");

		if (*flags & IPT_DNAT_OPT_DEST) {
			//if (!kernel_version)
			//	get_kernel_version();
			//if (kernel_version > LINUX_VERSION(2, 6, 10))
				xtables_error(PARAMETER_PROBLEM,
					   "Multiple --to-destination not supported");
		}
		*target = parse_to(optarg, portok, info);
		/* WTF do we need this for?? */
		if (*flags & IPT_DNAT_OPT_RANDOM)
			range.flags |= IP_NAT_RANGE_PROTO_RANDOM;
		*flags |= IPT_DNAT_OPT_DEST;
		return 1;

	case '2':
		if (*flags & IPT_DNAT_OPT_DEST) {
			range.flags |= IP_NAT_RANGE_PROTO_RANDOM;
			*flags |= IPT_DNAT_OPT_RANDOM;
		} else
			*flags |= IPT_DNAT_OPT_RANDOM;
		return 1;

	case '3':
		range.flags |= IP_NAT_RANGE_PERSISTENT;
		return 1;

	default:
		return 0;
	}
}

static void DNAT_fcheck(unsigned int flags)
{
	if (!flags)
		xtables_error(PARAMETER_PROBLEM,
			   "You must specify --to-destination");
}

static void print_range(const struct nf_nat_ipv6_range *range)
{
	if (range->flags & NF_NAT_RANGE_MAP_IPS) {
		if (range->flags & NF_NAT_RANGE_PROTO_SPECIFIED)
			printf("[");
		printf("%s", xtables_ip6addr_to_numeric(&range->min_addr.in6));
		if (memcmp(&range->min_addr, &range->max_addr,
			   sizeof(range->min_addr)))
			printf("-%s", xtables_ip6addr_to_numeric(&range->max_addr.in6));
		if (range->flags & NF_NAT_RANGE_PROTO_SPECIFIED)
			printf("]");
	}
	if (range->flags & NF_NAT_RANGE_PROTO_SPECIFIED) {
		printf(":");
		printf("%hu", ntohs(range->min_proto.tcp.port));
		if (range->max_proto.tcp.port != range->min_proto.tcp.port)
			printf("-%hu", ntohs(range->max_proto.tcp.port));
	}
}

static void DNAT_print(const void *ip, const struct xt_entry_target *target,
                       int numeric)
{
	const struct nf_nat_ipv6_range *range = (const void *)target->data;
	
	printf(" to:");
	print_range(range);
	if (range->flags & NF_NAT_RANGE_PROTO_RANDOM)
		printf(" random");
	if (range->flags & NF_NAT_RANGE_PERSISTENT)
		printf(" persistent");
}


static void DNAT_save(const void *ip, const struct xt_entry_target *target)
{
	const struct nf_nat_ipv6_range *range = (const void *)target->data;
	
	printf(" --to-destination ");
	print_range(range);
	if (range->flags & NF_NAT_RANGE_PROTO_RANDOM)
		printf(" --random");
	if (range->flags & NF_NAT_RANGE_PERSISTENT)
		printf(" --persistent");
}

static struct xtables_target dnat_tg_reg = {
	.name		= "DNAT",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_IPV6,
	.revision	= 1,
	.size		= XT_ALIGN(sizeof(struct nf_nat_ipv6_range)),
	.userspacesize	= XT_ALIGN(sizeof(struct nf_nat_ipv6_range)),
	.help		= DNAT_help,
	.parse	= DNAT_parse,
	.final_check	= DNAT_fcheck,
	.print		= DNAT_print,
	.save		= DNAT_save,
	.extra_opts	= DNAT_opts,
};

void _init(void)
{
	xtables_register_target(&dnat_tg_reg);
}
