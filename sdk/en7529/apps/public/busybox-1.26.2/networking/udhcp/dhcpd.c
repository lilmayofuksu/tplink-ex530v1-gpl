/* vi: set sw=4 ts=4: */
/*
 * udhcp server
 * Copyright (C) 1999 Matthew Ramsay <matthewr@moreton.com.au>
 *			Chris Trew <ctrew@moreton.com.au>
 *
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
//applet:IF_UDHCPD(APPLET(udhcpd, BB_DIR_USR_SBIN, BB_SUID_DROP))

//kbuild:lib-$(CONFIG_UDHCPD) += common.o packet.o signalpipe.o socket.o
//kbuild:lib-$(CONFIG_UDHCPD) += dhcpd.o arpping.o
//kbuild:lib-$(CONFIG_FEATURE_UDHCP_RFC3397) += domain_codec.o

//usage:#define udhcpd_trivial_usage
//usage:       "[-fS] [-I ADDR]" IF_FEATURE_UDHCP_PORT(" [-P N]") " [CONFFILE]"
//usage:#define udhcpd_full_usage "\n\n"
//usage:       "DHCP server\n"
//usage:     "\n	-f	Run in foreground"
//usage:     "\n	-S	Log to syslog too"
//usage:     "\n	-I ADDR	Local address"
//usage:     "\n	-a MSEC	Timeout for ARP ping (default 2000)"
//usage:	IF_FEATURE_UDHCP_PORT(
//usage:     "\n	-P N	Use port N (default 67)"
//usage:	)

#include <netinet/ether.h>
#include <syslog.h>
#include "common.h"
#include "dhcpc.h"
#include "dhcpd.h"
#ifdef BBU_SOC
#include "libtcapi.h"

int dhcp_arp_timeout_ms = 50;
#endif

#ifdef DHCP_PROFILE
struct server_config_t option60_config;//option 60 pool dhcp server
struct Option_Param_t gOption_Param;
uint8_t gOption60IsMatched = 0;
#endif

struct server_config_t dhcp_config;

#if defined(DHCP_PROFILE)||(defined(CWMP) && defined(TR111))
unsigned long gTotalMaxLeases = 0;//total max leases of all pools
#endif

#if defined(CWMP) && defined(TR111)
Tr111_Param_t gTR111_Parm;
#endif

/* globals */
struct dyn_lease *g_leases;
/* struct server_config_t server_config is in bb_common_bufsiz1 */

/* Takes the address of the pointer to the static_leases linked list,
 * address to a 6 byte mac address,
 * 4 byte IP address */
static void add_static_lease(struct static_lease **st_lease_pp,
		uint8_t *mac,
		uint32_t nip)
{
	struct static_lease *st_lease;

	/* Find the tail of the list */
	while ((st_lease = *st_lease_pp) != NULL) {
		st_lease_pp = &st_lease->next;
	}

	/* Add new node */
	*st_lease_pp = st_lease = xzalloc(sizeof(*st_lease));
	memcpy(st_lease->mac, mac, 6);
	st_lease->nip = nip;
	/*st_lease->next = NULL;*/
}

/* Find static lease IP by mac */
static uint32_t get_static_nip_by_mac(struct static_lease *st_lease, void *mac)
{
	while (st_lease) {
		if (memcmp(st_lease->mac, mac, 6) == 0)
			return st_lease->nip;
		st_lease = st_lease->next;
	}

	return 0;
}

static int is_nip_reserved(struct static_lease *st_lease, uint32_t nip)
{
	while (st_lease) {
		if (st_lease->nip == nip)
			return 1;
		st_lease = st_lease->next;
	}

	return 0;
}

#if defined CONFIG_UDHCP_DEBUG && CONFIG_UDHCP_DEBUG >= 2
/* Print out static leases just to check what's going on */
/* Takes the address of the pointer to the static_leases linked list */
static void log_static_leases(struct static_lease **st_lease_pp)
{
	struct static_lease *cur;

	if (dhcp_verbose < 2)
		return;

	cur = *st_lease_pp;
	while (cur) {
		bb_error_msg("static lease: mac:%02x:%02x:%02x:%02x:%02x:%02x nip:%x",
			cur->mac[0], cur->mac[1], cur->mac[2],
			cur->mac[3], cur->mac[4], cur->mac[5],
			cur->nip
		);
		cur = cur->next;
	}
}
#else
# define log_static_leases(st_lease_pp) ((void)0)
#endif

/* Find the oldest expired lease, NULL if there are no expired leases */
static struct dyn_lease *oldest_expired_lease(void)
{
	struct dyn_lease *oldest_lease = NULL;
	leasetime_t oldest_time = time(NULL);
	unsigned i;

	/* Unexpired leases have g_leases[i].expires >= current time
	 * and therefore can't ever match */
#ifdef DHCP_PROFILE
	for (i = 0; i < gTotalMaxLeases; i++) 
#else
	for (i = 0; i < server_config.max_leases; i++) 
#endif
	{
		if (g_leases[i].expires == 0 /* empty entry */
		 || g_leases[i].expires < oldest_time
		) {
			oldest_time = g_leases[i].expires;
			oldest_lease = &g_leases[i];
		}
	}
	return oldest_lease;
}

/* Clear out all leases with matching nonzero chaddr OR yiaddr.
 * If chaddr == NULL, this is a conflict lease.
 */
static void clear_leases(const uint8_t *chaddr, uint32_t yiaddr)
{
	unsigned i;
#ifdef DHCP_PROFILE
	for (i = 0; i < gTotalMaxLeases; i++) 
#else
	for (i = 0; i < server_config.max_leases; i++)
#endif
	{
		if ((chaddr && memcmp(g_leases[i].lease_mac, chaddr, 6) == 0)
		 || (yiaddr && g_leases[i].lease_nip == yiaddr)
		) {
			memset(&g_leases[i], 0, sizeof(g_leases[i]));
		}
	}
}

/* Add a lease into the table, clearing out any old ones.
 * If chaddr == NULL, this is a conflict lease.
 */
static struct dyn_lease *add_lease(
		const uint8_t *chaddr, uint32_t yiaddr,
		leasetime_t leasetime,
		const char *hostname, int hostname_len)
{
	struct dyn_lease *oldest;

	/* clean out any old ones */
	clear_leases(chaddr, yiaddr);

	oldest = oldest_expired_lease();

	if (oldest) {
		memset(oldest, 0, sizeof(*oldest));
		if (hostname) {
			char *p;

			hostname_len++; /* include NUL */
			if (hostname_len > sizeof(oldest->hostname))
				hostname_len = sizeof(oldest->hostname);
			p = safe_strncpy(oldest->hostname, hostname, hostname_len);
			/*
			 * Sanitization (s/bad_char/./g).
			 * The intent is not to allow only "DNS-valid" hostnames,
			 * but merely make dumpleases output safe for shells to use.
			 * We accept "0-9A-Za-z._-", all other chars turn to dots.
			 */
			while (*p) {
				if (!isalnum(*p) && *p != '-' && *p != '_')
					*p = '.';
				p++;
			}
		}
		if (chaddr)
			memcpy(oldest->lease_mac, chaddr, 6);
		oldest->lease_nip = yiaddr;
		oldest->expires = time(NULL) + leasetime;
	}

	return oldest;
}

/* True if a lease has expired */
static int is_expired_lease(struct dyn_lease *lease)
{
	return (lease->expires < (leasetime_t) time(NULL));
}

/* Find the first lease that matches MAC, NULL if no match */
static struct dyn_lease *find_lease_by_mac(const uint8_t *mac)
{
	unsigned i;
#ifdef DHCP_PROFILE
	for (i = 0; i < gTotalMaxLeases; i++) 
#else
	for (i = 0; i < server_config.max_leases; i++)
#endif
		if (memcmp(g_leases[i].lease_mac, mac, 6) == 0)
			return &g_leases[i];

	return NULL;
}

/* Find the first lease that matches IP, NULL is no match */
static struct dyn_lease *find_lease_by_nip(uint32_t nip)
{
	unsigned i;
#ifdef DHCP_PROFILE
	for (i = 0; i < gTotalMaxLeases; i++) 
#else
	for (i = 0; i < server_config.max_leases; i++)
#endif
		if (g_leases[i].lease_nip == nip)
			return &g_leases[i];

	return NULL;
}

/* Check if the IP is taken; if it is, add it to the lease table */
static int nobody_responds_to_arp(uint32_t nip, const uint8_t *safe_mac, unsigned arpping_ms)
{
	struct in_addr temp;
	int r;

	r = arpping(nip, safe_mac,
			server_config.server_nip,
			server_config.server_mac,
			server_config.interface,
			arpping_ms);
	if (r)
		return r;

	temp.s_addr = nip;
	bb_error_msg("%s belongs to someone, reserving it for %u seconds",
		inet_ntoa(temp), (unsigned)server_config.conflict_time);
	add_lease(NULL, nip, server_config.conflict_time, NULL, 0);
	return 0;
}

/* Find a new usable (we think) address */
static uint32_t find_free_or_expired_nip(const uint8_t *safe_mac, unsigned arpping_ms)
{
	uint32_t addr;
	struct dyn_lease *oldest_lease = NULL;

#if ENABLE_FEATURE_UDHCPD_BASE_IP_ON_MAC
	uint32_t stop;
	unsigned i, hash;

	/* hash hwaddr: use the SDBM hashing algorithm.  Seems to give good
	 * dispersal even with similarly-valued "strings".
	 */
	hash = 0;
	for (i = 0; i < 6; i++)
		hash += safe_mac[i] + (hash << 6) + (hash << 16) - hash;

	/* pick a seed based on hwaddr then iterate until we find a free address. */
	addr = server_config.start_ip
		+ (hash % (1 + server_config.end_ip - server_config.start_ip));
	stop = addr;
#else
	addr = server_config.start_ip;
#define stop (server_config.end_ip + 1)
#endif
	do {
		uint32_t nip;
		struct dyn_lease *lease;

		/* ie, 192.168.55.0 */
		if ((addr & 0xff) == 0)
			goto next_addr;
		/* ie, 192.168.55.255 */
		if ((addr & 0xff) == 0xff)
			goto next_addr;
		nip = htonl(addr);
		/* skip our own address */
		if (nip == server_config.server_nip)
			goto next_addr;
		/* is this a static lease addr? */
		if (is_nip_reserved(server_config.static_leases, nip))
			goto next_addr;

		lease = find_lease_by_nip(nip);
		if (!lease) {
//TODO: DHCP servers do not always sit on the same subnet as clients: should *ping*, not arp-ping!
			if (nobody_responds_to_arp(nip, safe_mac, arpping_ms))
				return nip;
		} else {
			if (!oldest_lease || lease->expires < oldest_lease->expires)
				oldest_lease = lease;
		}

 next_addr:
		addr++;
#if ENABLE_FEATURE_UDHCPD_BASE_IP_ON_MAC
		if (addr > server_config.end_ip)
			addr = server_config.start_ip;
#endif
	} while (addr != stop);

	if (oldest_lease
	 && is_expired_lease(oldest_lease)
	 && nobody_responds_to_arp(oldest_lease->lease_nip, safe_mac, arpping_ms)
	) {
		return oldest_lease->lease_nip;
	}

	return 0;
}

/* On these functions, make sure your datatype matches */

#ifdef DHCP_PROFILE
static int FAST_FUNC read_yn(const char *line, void *arg)
{
	char *dest = arg;
	int retval = 1;

	if (!strcasecmp("yes", line))
		*dest = 1;
	else if (!strcasecmp("no", line))
		*dest = 0;
	else retval = 0;

	return retval;
}
#endif
static int FAST_FUNC read_str(const char *line, void *arg)
{
	char **dest = arg;

	free(*dest);
	*dest = xstrdup(line);
	return 1;
}

static int FAST_FUNC read_u32(const char *line, void *arg)
{
	*(uint32_t*)arg = bb_strtou32(line, NULL, 10);
	return errno == 0;
}

static int FAST_FUNC read_staticlease(const char *const_line, void *arg)
{
	char *line;
	char *mac_string;
	char *ip_string;
	struct ether_addr mac_bytes; /* it's "struct { uint8_t mac[6]; }" */
	uint32_t nip;

	/* Read mac */
	line = (char *) const_line;
	mac_string = strtok_r(line, " \t", &line);
	if (!mac_string || !ether_aton_r(mac_string, &mac_bytes))
		return 0;

	/* Read ip */
	ip_string = strtok_r(NULL, " \t", &line);
	if (!ip_string || !udhcp_str2nip(ip_string, &nip))
		return 0;

	add_static_lease(arg, (uint8_t*) &mac_bytes, nip);

	log_static_leases(arg);

	return 1;
}

struct config_keyword {
	const char *keyword;
	int (*handler)(const char *line, void *var) FAST_FUNC;
	unsigned ofs;
	const char *def;
};

#define OFS(field) offsetof(struct server_config_t, field)

static const struct config_keyword keywords[] = {
	/* keyword        handler           variable address               default */
	{"start"        , udhcp_str2nip   , OFS(start_ip     ), "192.168.0.20"},
	{"end"          , udhcp_str2nip   , OFS(end_ip       ), "192.168.0.254"},
	{"interface"    , read_str        , OFS(interface    ), "eth0"},
	/* Avoid "max_leases value not sane" warning by setting default
	 * to default_end_ip - default_start_ip + 1: */
	{"max_leases"   , read_u32        , OFS(max_leases   ), "235"},
#ifdef DHCP_PROFILE
	{"remaining"	, read_yn		  , OFS(remaining)	  , "yes"},
#endif
	{"auto_time"    , read_u32        , OFS(auto_time    ), "7200"},
	{"decline_time" , read_u32        , OFS(decline_time ), "3600"},
	{"conflict_time", read_u32        , OFS(conflict_time), "3600"},
	{"offer_time"   , read_u32        , OFS(offer_time   ), "60"},
	{"min_lease"    , read_u32        , OFS(min_lease_sec), "60"},
	{"lease_file"   , read_str        , OFS(lease_file   ), LEASES_FILE},
	{"pidfile"      , read_str        , OFS(pidfile      ), "/var/run/udhcpd.pid"},
	{"siaddr"       , udhcp_str2nip   , OFS(siaddr_nip   ), "0.0.0.0"},
	/* keywords with no defaults must be last! */
	{"option"       , udhcp_str2optset, OFS(options      ), ""},
	{"opt"          , udhcp_str2optset, OFS(options      ), ""},
	{"notify_file"  , read_str        , OFS(notify_file  ), NULL},
	{"sname"        , read_str        , OFS(sname        ), NULL},
	{"boot_file"    , read_str        , OFS(boot_file    ), NULL},
	{"static_lease" , read_staticlease, OFS(static_leases), ""},
};
enum { KWS_WITH_DEFAULTS = ARRAY_SIZE(keywords) - 6 };

#ifdef DHCP_PROFILE

#define OFS_Option(field) offsetof(struct Option_Param_t, field)

static const struct config_keyword option_keywords[] = {
	{"enable",		read_yn, 		OFS_Option(opt60_enable),		"1"},
	{"vendorClassID",	read_str, 		OFS_Option(opt60_vid), 			"MSFT 5.0"},
	{"opt240Enable",	read_yn, 		OFS_Option(opt240_enable),		"1"},
	{"opt240Value",		read_str, 		OFS_Option(opt240_value), 		"This is option 240"},
};

enum { KWS_OPTION_WITH_DEFAULTS = ARRAY_SIZE(option_keywords)};

static NOINLINE void read_config(struct server_config_t *conf_start, const char *file)
{
	parser_t *parser;
	const struct config_keyword *k;
	unsigned i;
	char *token[2];
	int opt_flag = 0;
	if(strcmp(file, DHCPD_OPTION_CONF_FILE) == 0)
	{
		opt_flag = 1;
	}
	
	for (i = 0; i < KWS_WITH_DEFAULTS; i++){
		keywords[i].handler(keywords[i].def, (char *)conf_start + keywords[i].ofs);
	}

	parser = config_open(file);
	while (config_read(parser, token, 2, 2, "# \t", PARSE_NORMAL)) {
		for (k = keywords, i = 0; i < ARRAY_SIZE(keywords); k++, i++) {
			if (strcasecmp(token[0], k->keyword) == 0) {
				if (!k->handler(token[1], (char *)conf_start + k->ofs)) {
					bb_error_msg("can't parse line %u in %s",parser->lineno, file);
					/* reset back to the default value */
					k->handler(k->def, (char *)conf_start + k->ofs);
				}
				break;
			}
		}
		if(opt_flag == 1)
		{
			for (k = option_keywords, i = 0; i < ARRAY_SIZE(option_keywords); k++, i++) {
				if (strcasecmp(token[0], k->keyword) == 0) {
					if (!k->handler(token[1], (char*)&gOption_Param + k->ofs)) {
						bb_error_msg("can't parse line %u in %s",parser->lineno, file);
						/* reset back to the default value */
						k->handler(k->def, (char*)&gOption_Param + k->ofs);
					}
					break;
				}
			}
		}	
	}
	
	config_close(parser);
	conf_start->start_ip = ntohl(conf_start->start_ip);	
	conf_start->end_ip = ntohl(conf_start->end_ip);
}

#else
static NOINLINE void read_config(const char *file)
{
	parser_t *parser;
	const struct config_keyword *k;
	unsigned i;
	char *token[2];

	for (i = 0; i < KWS_WITH_DEFAULTS; i++)
		keywords[i].handler(keywords[i].def, (char*)&server_config + keywords[i].ofs);

	parser = config_open(file);
	while (config_read(parser, token, 2, 2, "# \t", PARSE_NORMAL)) {
		for (k = keywords, i = 0; i < ARRAY_SIZE(keywords); k++, i++) {
			if (strcasecmp(token[0], k->keyword) == 0) {
				if (!k->handler(token[1], (char*)&server_config + k->ofs)) {
					bb_error_msg("can't parse line %u in %s",
							parser->lineno, file);
					/* reset back to the default value */
					k->handler(k->def, (char*)&server_config + k->ofs);
				}
				break;
			}
		}
	}
	config_close(parser);

	server_config.start_ip = ntohl(server_config.start_ip);
	server_config.end_ip = ntohl(server_config.end_ip);
}

#endif

static void write_leases(void)
{
	int fd;
	unsigned i;
	leasetime_t curr;
	int64_t written_at;

	fd = open_or_warn(server_config.lease_file, O_WRONLY|O_CREAT|O_TRUNC);
	if (fd < 0)
		return;

	curr = written_at = time(NULL);

	written_at = SWAP_BE64(written_at);
	full_write(fd, &written_at, sizeof(written_at));
#ifdef DHCP_PROFILE
	for (i = 0; i < gTotalMaxLeases; i++) 
#else
	for (i = 0; i < server_config.max_leases; i++)
#endif
	{
		leasetime_t tmp_time;

		if (g_leases[i].lease_nip == 0)
			continue;

		/* Screw with the time in the struct, for easier writing */
		tmp_time = g_leases[i].expires;

		g_leases[i].expires -= curr;
		if ((signed_leasetime_t) g_leases[i].expires < 0)
			g_leases[i].expires = 0;
		g_leases[i].expires = htonl(g_leases[i].expires);

		/* No error check. If the file gets truncated,
		 * we lose some leases on restart. Oh well. */
		full_write(fd, &g_leases[i], sizeof(g_leases[i]));

		/* Then restore it when done */
		g_leases[i].expires = tmp_time;
	}
	close(fd);

	if (server_config.notify_file) {
		char *argv[3];
		argv[0] = server_config.notify_file;
		argv[1] = server_config.lease_file;
		argv[2] = NULL;
		spawn_and_wait(argv);
	}
}

#if defined(DHCP_PROFILE) || (defined(CWMP) && defined(TR111))
int lease_expired(struct dyn_lease *lease)
{
	struct  timespec  curtime;
	clock_gettime(CLOCK_MONOTONIC, &curtime);
	/* see from code, the DHCP LEASE has been done before the following code,
         * but the curtime maybe same as the lease time because CPU is very fast
         * so "<" must be instead of "<="
         * Shelven.Lu @ 20110715 */
	return (lease->expires <= (unsigned long) curtime.tv_sec);
	/*return (lease->expires < (unsigned long) time(0));*/
}
#endif

#ifdef DHCP_PROFILE
void write_leases_enhance(struct dyn_lease *lease)
{
	FILE *fp;
	unsigned int i;
	char buf[255] = {0};
	unsigned long tmp_time, expires;
	char mac_addr[17];
	char ip_addr[16];
	struct in_addr addr;
	int currentLeaseNumer = 0;

	if (!(fp = fopen(LEASE_PATH, "w"))) {
		log1("error: Unable to open %s for writing", LEASE_PATH);
		return;
	}

	for (i = 0; i < gTotalMaxLeases; i++) {		
		if((lease!=NULL) && !memcmp(g_leases[i].lease_mac,lease->lease_mac,16)){
			g_leases[i].expires = lease->expires;
		}

		if (g_leases[i].lease_nip != 0) {
			/* screw with the time in the struct, for easier writing */
			tmp_time = g_leases[i].expires;
			
			if (server_config.remaining) {
				if (lease_expired(&(g_leases[i]))){
					g_leases[i].expires = 0;
					continue;
				}
				//else leases[i].expires -= curr;
			} /* else stick with the time we got */
			
			sprintf(mac_addr,"%02X:%02X:%02X:%02X:%02X:%02X",g_leases[i].lease_mac[0],g_leases[i].lease_mac[1],g_leases[i].lease_mac[2],g_leases[i].lease_mac[3],g_leases[i].lease_mac[4],g_leases[i].lease_mac[5]);
			
			addr.s_addr = g_leases[i].lease_nip;
			sprintf(ip_addr,"%s", inet_ntoa(addr));
			expires = g_leases[i].expires;			
					
			sprintf(buf, "%s %s %ld %s\n", mac_addr, ip_addr, expires, g_leases[i].hostname);
			
			fwrite(buf, sizeof(char), strlen(buf), fp);
			currentLeaseNumer++;

			/* Then restore it when done. */
			g_leases[i].expires = tmp_time;
		}
	}
	fclose(fp);
/*
	if(leaseNumber != currentLeaseNumer){
		log1("info: i will reinit host.\n");
		leaseNumber = currentLeaseNumer;
	#ifdef CWMP	
		sendCwmpMsg(3); //HOST_REINIT
	#endif	
*/
}
#endif

#ifdef DHCP_PROFILE
static NOINLINE void read_leases(const char *file)
{
	FILE *fp;
	unsigned int i = 0, j = 0;
	struct dyn_lease lease;
	struct  timespec  curr;
	char buf[80];
	char ip_buf[20];
	int mac[6];
	
	memset(&lease,0,sizeof(struct dyn_lease));
	/* suppose that dhcp server restarted, it need read leases from a file that record
	 * the leases. In dhcpd config file, the file is /etc/udhcpd.lease;
	 * but dhcp server actually write the record to /etc/udhcp_lease;
	 * so we need copy /etc/udhcp_lease to udhcpd.lease
	 * Why don't we read /etc/udhcp_lease directly? because someone may write udhcp_lease when 
	 * we are reading this file, please refer to function add_lease()
	 * Leto.liu@20110316
	 * */
	if((fp = fopen(LEASE_PATH, "r")) != NULL){
		fclose(fp);
		snprintf(buf,sizeof(buf), "/bin/cp %s %s", LEASE_PATH, file);
		system(buf);
	}
	
	if (!(fp = fopen(file, "r"))) {
		log1("error: Unable to open %s for reading\n", file);
		return;
	}

	while (i < gTotalMaxLeases && fgets(buf, sizeof(buf)-1, fp)) {
		sscanf(buf,"%02X:%02X:%02X:%02X:%02X:%02X %s %u %s", &(mac[0]),&(mac[1]),&(mac[2]),&(mac[3]),&(mac[4]),&(mac[5]), ip_buf, &(lease.expires), lease.hostname);
		for(j=0;j<6;j++){
			lease.lease_mac[j] = (uint8_t)mac[j];
		}
		inet_aton(ip_buf, &(lease.lease_nip));
		/* ADDME: is it a static lease */

		if ((lease.lease_nip >= dhcp_config.start_ip && lease.lease_nip <= dhcp_config.end_ip)
		|| (lease.lease_nip >= option60_config.start_ip && lease.lease_nip <= option60_config.end_ip))


 		{
			lease.expires = ntohl(lease.expires);
			
			/* match witch write_leases_enhance
			 * it directly plus current time no matter how dhcp_config.remaining is*/
			//if (!dhcp_config.remaining){
				clock_gettime(CLOCK_MONOTONIC, &curr);
				lease.expires -= curr.tv_sec;
				//lease.expires -= time(0);
			//}
			if (add_lease(lease.lease_mac, lease.lease_nip,
					lease.expires ,
					lease.hostname, sizeof(lease.hostname)==0)) {
				log1("warning: Too many leases while loading %s\n", file);
				break;
			}
			i++;
		}
	}
	log1("error:Read %d leases\n", i);
	fclose(fp);
}

#else
static NOINLINE void read_leases(const char *file)
{
	struct dyn_lease lease;
	int64_t written_at, time_passed;
	int fd;
#if defined CONFIG_UDHCP_DEBUG && CONFIG_UDHCP_DEBUG >= 1
	unsigned i = 0;
#endif

	fd = open_or_warn(file, O_RDONLY);
	if (fd < 0)
		return;

	if (full_read(fd, &written_at, sizeof(written_at)) != sizeof(written_at))
		goto ret;
	written_at = SWAP_BE64(written_at);

	time_passed = time(NULL) - written_at;
	/* Strange written_at, or lease file from old version of udhcpd
	 * which had no "written_at" field? */
	if ((uint64_t)time_passed > 12 * 60 * 60)
		goto ret;

	while (full_read(fd, &lease, sizeof(lease)) == sizeof(lease)) {
		uint32_t y = ntohl(lease.lease_nip);
	#if defined(DHCP_PROFILE)
		if ((y >= server_config.start_ip && y <= server_config.end_ip)||(y >= option60_config.start_ip && y <= option60_config.end_ip))
	#else
		if (y >= server_config.start_ip && y <= server_config.end_ip) 
	#endif
		{
			signed_leasetime_t expires = ntohl(lease.expires) - (signed_leasetime_t)time_passed;
			uint32_t static_nip;

			if (expires <= 0)
				/* We keep expired leases: add_lease() will add
				 * a lease with 0 seconds remaining.
				 * Fewer IP address changes this way for mass reboot scenario.
				 */
				expires = 0;

			/* Check if there is a different static lease for this IP or MAC */
			static_nip = get_static_nip_by_mac(server_config.static_leases, lease.lease_mac);
			if (static_nip) {
				/* NB: we do not add lease even if static_nip == lease.lease_nip.
				 */
				continue;
			}
			if (is_nip_reserved(server_config.static_leases, lease.lease_nip))
				continue;

			/* NB: add_lease takes "relative time", IOW,
			 * lease duration, not lease deadline. */
			if (add_lease(lease.lease_mac, lease.lease_nip,
					expires,
					lease.hostname, sizeof(lease.hostname)
				) == 0
			) {
				bb_error_msg("too many leases while loading %s", file);
				break;
			}
#if defined CONFIG_UDHCP_DEBUG && CONFIG_UDHCP_DEBUG >= 1
			i++;
#endif
		}
	}
	log1("read %d leases", i);
 ret:
	close(fd);
}

#endif
/* Send a packet to a specific mac address and ip address by creating our own ip packet */
static void send_packet_to_client(struct dhcp_packet *dhcp_pkt, int force_broadcast)
{
	const uint8_t *chaddr;
	uint32_t ciaddr;

	// Was:
	//if (force_broadcast) { /* broadcast */ }
	//else if (dhcp_pkt->ciaddr) { /* unicast to dhcp_pkt->ciaddr */ }
	//else if (dhcp_pkt->flags & htons(BROADCAST_FLAG)) { /* broadcast */ }
	//else { /* unicast to dhcp_pkt->yiaddr */ }
	// But this is wrong: yiaddr is _our_ idea what client's IP is
	// (for example, from lease file). Client may not know that,
	// and may not have UDP socket listening on that IP!
	// We should never unicast to dhcp_pkt->yiaddr!
	// dhcp_pkt->ciaddr, OTOH, comes from client's request packet,
	// and can be used.

	if (force_broadcast
	 || (dhcp_pkt->flags & htons(BROADCAST_FLAG))
	 || dhcp_pkt->ciaddr == 0
	) {
		log1("broadcasting packet to client");
		ciaddr = INADDR_BROADCAST;
		chaddr = MAC_BCAST_ADDR;
	} else {
		log1("unicasting packet to client ciaddr");
		ciaddr = dhcp_pkt->ciaddr;
		chaddr = dhcp_pkt->chaddr;
	}

	udhcp_send_raw_packet(dhcp_pkt,
		/*src*/ server_config.server_nip, SERVER_PORT,
		/*dst*/ ciaddr, CLIENT_PORT, chaddr,
		server_config.ifindex);
}

/* Send a packet to gateway_nip using the kernel ip stack */
static void send_packet_to_relay(struct dhcp_packet *dhcp_pkt)
{
	log1("forwarding packet to relay");

	udhcp_send_kernel_packet(dhcp_pkt,
			server_config.server_nip, SERVER_PORT,
			dhcp_pkt->gateway_nip, SERVER_PORT);
}

static void send_packet(struct dhcp_packet *dhcp_pkt, int force_broadcast)
{
	if (dhcp_pkt->gateway_nip)
		send_packet_to_relay(dhcp_pkt);
	else
		send_packet_to_client(dhcp_pkt, force_broadcast);
}

static void init_packet(struct dhcp_packet *packet, struct dhcp_packet *oldpacket, char type)
{
	/* Sets op, htype, hlen, cookie fields
	 * and adds DHCP_MESSAGE_TYPE option */
	udhcp_init_header(packet, type);

	packet->xid = oldpacket->xid;
	memcpy(packet->chaddr, oldpacket->chaddr, sizeof(oldpacket->chaddr));
	packet->flags = oldpacket->flags;
	packet->gateway_nip = oldpacket->gateway_nip;
	packet->ciaddr = oldpacket->ciaddr;
	udhcp_add_simple_option(packet, DHCP_SERVER_ID, server_config.server_nip);
}

/* Fill options field, siaddr_nip, and sname and boot_file fields.
 * TODO: teach this code to use overload option.
 */
static void add_server_options(struct dhcp_packet *packet)
{
	struct option_set *curr = server_config.options;

	while (curr) {
		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
			udhcp_add_binary_option(packet, curr->data);
		curr = curr->next;
	}

	packet->siaddr_nip = server_config.siaddr_nip;

	if (server_config.sname)
		strncpy((char*)packet->sname, server_config.sname, sizeof(packet->sname) - 1);
	if (server_config.boot_file)
		strncpy((char*)packet->file, server_config.boot_file, sizeof(packet->file) - 1);
}

static uint32_t select_lease_time(struct dhcp_packet *packet)
{
	uint32_t lease_time_sec = server_config.max_lease_sec;
	uint8_t *lease_time_opt = udhcp_get_option(packet, DHCP_LEASE_TIME);
	if (lease_time_opt) {
		move_from_unaligned32(lease_time_sec, lease_time_opt);
		lease_time_sec = ntohl(lease_time_sec);
		if (lease_time_sec > server_config.max_lease_sec)
			lease_time_sec = server_config.max_lease_sec;
		if (lease_time_sec < server_config.min_lease_sec)
			lease_time_sec = server_config.min_lease_sec;
	}
	return lease_time_sec;
}

/* We got a DHCP DISCOVER. Send an OFFER. */
/* NOINLINE: limit stack usage in caller */
static NOINLINE void send_offer(struct dhcp_packet *oldpacket,
		uint32_t static_lease_nip,
		struct dyn_lease *lease,
		uint8_t *requested_ip_opt,
		unsigned arpping_ms)
{
	struct dhcp_packet packet;
	uint32_t lease_time_sec;
	struct in_addr addr;

	init_packet(&packet, oldpacket, DHCPOFFER);

	/* If it is a static lease, use its IP */
	packet.yiaddr = static_lease_nip;
	/* Else: */
	if (!static_lease_nip) {
		/* We have no static lease for client's chaddr */
		uint32_t req_nip;
		const char *p_host_name;

		if (lease) {
			/* We have a dynamic lease for client's chaddr.
			 * Reuse its IP (even if lease is expired).
			 * Note that we ignore requested IP in this case.
			 */
			packet.yiaddr = lease->lease_nip;
		}
		/* Or: if client has requested an IP */
		else if (requested_ip_opt != NULL
		 /* (read IP) */
		 && (move_from_unaligned32(req_nip, requested_ip_opt), 1)
		 /* and the IP is in the lease range */
		 && ntohl(req_nip) >= server_config.start_ip
		 && ntohl(req_nip) <= server_config.end_ip
		 /* and */
		 && (  !(lease = find_lease_by_nip(req_nip)) /* is not already taken */
		    || is_expired_lease(lease) /* or is taken, but expired */
		    )
		) {
			packet.yiaddr = req_nip;
		}
		else {
			/* Otherwise, find a free IP */
			packet.yiaddr = find_free_or_expired_nip(oldpacket->chaddr, arpping_ms);
		}

		if (!packet.yiaddr) {
			bb_error_msg("no free IP addresses. OFFER abandoned");
			return;
		}
		/* Reserve the IP for a short time hoping to get DHCPREQUEST soon */
		p_host_name = (const char*) udhcp_get_option(oldpacket, DHCP_HOST_NAME);
		lease = add_lease(packet.chaddr, packet.yiaddr,
				server_config.offer_time,
				p_host_name,
				p_host_name ? (unsigned char)p_host_name[OPT_LEN - OPT_DATA] : 0
		);
		if (!lease) {
			bb_error_msg("no free IP addresses. OFFER abandoned");
			return;
		}
	}

#if defined(CWMP) && defined(TR111)
	if (check_vi(oldpacket))
		addVIOption(packet.options);
#endif

#ifdef DHCP_PROFILE
	if(gOption_Param.opt240_enable)
		addOPT240Option(packet.options);
#endif

	lease_time_sec = select_lease_time(oldpacket);
	udhcp_add_simple_option(&packet, DHCP_LEASE_TIME, htonl(lease_time_sec));
	add_server_options(&packet);

	addr.s_addr = packet.yiaddr;
	bb_error_msg("sending OFFER of %s", inet_ntoa(addr));
	/* send_packet emits error message itself if it detects failure */
	send_packet(&packet, /*force_bcast:*/ 0);
}

/* NOINLINE: limit stack usage in caller */
static NOINLINE void send_NAK(struct dhcp_packet *oldpacket)
{
	struct dhcp_packet packet;

	init_packet(&packet, oldpacket, DHCPNAK);

#if defined(CWMP) && defined(TR111)
	if(check_vi(oldpacket))
		addVIOption(packet.options);
#endif

#ifdef DHCP_PROFILE
	if(gOption_Param.opt240_enable)
		addOPT240Option(packet.options);
#endif

	log1("sending %s", "NAK");
	send_packet(&packet, /*force_bcast:*/ 1);
}

/* NOINLINE: limit stack usage in caller */
static NOINLINE void send_ACK(struct dhcp_packet *oldpacket, uint32_t yiaddr)
{
	struct dhcp_packet packet;
	uint32_t lease_time_sec;
	struct in_addr addr;
	const char *p_host_name;

#if defined(CWMP) && defined(TR111)
	if (check_vi(oldpacket))
	{
		oldpacket->yiaddr = yiaddr;
		insertDevice(oldpacket);
	}
#endif

	init_packet(&packet, oldpacket, DHCPACK);
	packet.yiaddr = yiaddr;

	lease_time_sec = select_lease_time(oldpacket);
	udhcp_add_simple_option(&packet, DHCP_LEASE_TIME, htonl(lease_time_sec));

#if defined(CWMP) && defined(TR111)
	if(check_vi(oldpacket))
		addVIOption(packet.options);
#endif

#ifdef DHCP_PROFILE
	if(gOption_Param.opt240_enable)
		addOPT240Option(packet.options);
#endif

	add_server_options(&packet);

	addr.s_addr = yiaddr;
	/*bb_error_msg("sending ACK to %s", inet_ntoa(addr));*/
	tcdbg_printf("sending ACK to %s\n", inet_ntoa(addr));
	send_packet(&packet, /*force_bcast:*/ 0);

	p_host_name = (const char*) udhcp_get_option(oldpacket, DHCP_HOST_NAME);
	add_lease(packet.chaddr, packet.yiaddr,
		lease_time_sec,
		p_host_name,
		p_host_name ? (unsigned char)p_host_name[OPT_LEN - OPT_DATA] : 0
	);
	if (ENABLE_FEATURE_UDHCPD_WRITE_LEASES_EARLY) {
		/* rewrite the file with leases at every new acceptance */
		
		#ifdef DHCP_PROFILE
			write_leases_enhance(NULL);
		#else
		write_leases();
		#endif
	}
}

/* NOINLINE: limit stack usage in caller */
static NOINLINE void send_inform(struct dhcp_packet *oldpacket)
{
	struct dhcp_packet packet;

	/* "If a client has obtained a network address through some other means
	 * (e.g., manual configuration), it may use a DHCPINFORM request message
	 * to obtain other local configuration parameters.  Servers receiving a
	 * DHCPINFORM message construct a DHCPACK message with any local
	 * configuration parameters appropriate for the client without:
	 * allocating a new address, checking for an existing binding, filling
	 * in 'yiaddr' or including lease time parameters.  The servers SHOULD
	 * unicast the DHCPACK reply to the address given in the 'ciaddr' field
	 * of the DHCPINFORM message.
	 * ...
	 * The server responds to a DHCPINFORM message by sending a DHCPACK
	 * message directly to the address given in the 'ciaddr' field
	 * of the DHCPINFORM message.  The server MUST NOT send a lease
	 * expiration time to the client and SHOULD NOT fill in 'yiaddr'."
	 */
//TODO: do a few sanity checks: is ciaddr set?
//Better yet: is ciaddr == IP source addr?
	init_packet(&packet, oldpacket, DHCPACK);
	add_server_options(&packet);

	send_packet(&packet, /*force_bcast:*/ 0);
}

int udhcpd_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int udhcpd_main(int argc UNUSED_PARAM, char **argv)
{
	int server_socket = -1, retval, max_sock;
	uint8_t *state = NULL;
	unsigned timeout_end;
	unsigned num_ips;
	unsigned opt;
	struct option_set *option;
	char *str_I = str_I;
	const char *str_a = "2000";
	unsigned arpping_ms;
#ifdef BBU_SOC
	FILE *fp;
	char buf[32];
#endif

#ifdef DHCP_PROFILE
	uint8_t *vendorID;
	uint8_t config_file_exist = 0;
	FILE *pconfig_file = NULL;
#endif

#if defined(CWMP) && defined(TR111)
	gTR111_Parm.VIHeader= (mDev_t*)malloc(sizeof(mDev_t));
	if(!gTR111_Parm.VIHeader)
	{		
		log1("error:malloc device node(head) failed.\n");
		return -1;
	}
	memset(gTR111_Parm.VIHeader, 0, sizeof(mDev_t));
	gTR111_Parm.VITail= gTR111_Parm.VIHeader;
	gTR111_Parm.VIHeader->next = NULL;
	gTR111_Parm.iNotifyFlag = 1;
	gTR111_Parm.iTotalNumber = 0;
	gTR111_Parm.iNotifyTimer = 0;
#endif

#ifdef TR111
	struct	timespec  curtime;
#endif
	IF_FEATURE_UDHCP_PORT(char *str_P;)

	setup_common_bufsiz();

	IF_FEATURE_UDHCP_PORT(SERVER_PORT = 67;)
	IF_FEATURE_UDHCP_PORT(CLIENT_PORT = 68;)

#if defined CONFIG_UDHCP_DEBUG && CONFIG_UDHCP_DEBUG >= 1
	opt_complementary = "vv";
#endif
	opt = getopt32(argv, "fSI:va:"
		IF_FEATURE_UDHCP_PORT("P:")
		, &str_I
		, &str_a
		IF_FEATURE_UDHCP_PORT(, &str_P)
		IF_UDHCP_VERBOSE(, &dhcp_verbose)
		);
	if (!(opt & 1)) { /* no -f */
		bb_daemonize_or_rexec(0, argv);
		logmode = LOGMODE_NONE;
	}
	/* update argv after the possible vfork+exec in daemonize */
	argv += optind;
	if (opt & 2) { /* -S */
		openlog(applet_name, LOG_PID, LOG_DAEMON);
		logmode |= LOGMODE_SYSLOG;
	}
	if (opt & 4) { /* -I */
		len_and_sockaddr *lsa = xhost_and_af2sockaddr(str_I, 0, AF_INET);
		server_config.server_nip = lsa->u.sin.sin_addr.s_addr;
		free(lsa);
	}
#if ENABLE_FEATURE_UDHCP_PORT
	if (opt & 32) { /* -P */
		SERVER_PORT = xatou16(str_P);
		CLIENT_PORT = SERVER_PORT + 1;
	}
#endif
	arpping_ms = xatou(str_a);

	/* Would rather not do read_config before daemonization -
	 * otherwise NOMMU machines will parse config twice */
	 	
#ifdef DHCP_PROFILE
	read_config(&dhcp_config, argv[0] ? argv[0] : DHCPD_CONF_FILE);
#else
	read_config(argv[0] ? argv[0] : DHCPD_CONF_FILE);
#endif

	/* Make sure fd 0,1,2 are open */
	bb_sanitize_stdio();
	/* Equivalent of doing a fflush after every \n */
	setlinebuf(stdout);

	/* Create pidfile */
	write_pidfile(dhcp_config.pidfile);
	/* if (!..) bb_perror_msg("can't create pidfile %s", pidfile); */

	bb_error_msg("started, v"BB_VER);

	option = udhcp_find_option(dhcp_config.options, DHCP_LEASE_TIME);
	dhcp_config.max_lease_sec = DEFAULT_LEASE_TIME;
	if (option) {
		move_from_unaligned32(dhcp_config.max_lease_sec, option->data + OPT_DATA);
		dhcp_config.max_lease_sec = ntohl(dhcp_config.max_lease_sec);
	}
#ifdef BBU_SOC
	fp = fopen("/etc/dhcpdarpwaittime","r");
	if(fp != NULL)
	{
		fread(buf, sizeof(buf), 1, fp);
		dhcp_arp_timeout_ms = atoi(buf);
		tcdbg_printf("User set Arp Wait time %dms",dhcp_arp_timeout_ms);
		fclose(fp);
	}
	else
	{
		dhcp_arp_timeout_ms = 50;	
	}
	arpping_ms = dhcp_arp_timeout_ms;
#endif	

	/* Sanity check */
	num_ips = dhcp_config.end_ip - dhcp_config.start_ip + 1;
	if (dhcp_config.max_leases > num_ips) {
		bb_error_msg("max_leases=%u is too big, setting to %u",
			(unsigned)dhcp_config.max_leases, num_ips);
		dhcp_config.max_leases = num_ips;
	}
	/*g_leases = xzalloc(dhcp_config.max_leases * sizeof(g_leases[0]));*/
	read_leases(dhcp_config.lease_file);

	if (udhcp_read_interface(dhcp_config.interface,
			&dhcp_config.ifindex,
			(dhcp_config.server_nip == 0 ? &dhcp_config.server_nip : NULL),
			dhcp_config.server_mac)
	) {
		retval = 1;
		goto ret;
	}

	
#ifdef DHCP_PROFILE
		/*init gOption60_Param*/
		gOption_Param.opt60_enable = 0;
		gOption_Param.opt60_vid = NULL; 
		gOption_Param.opt240_enable = 0;
		gOption_Param.opt240_value = NULL;
		memset(&gOption_Param, 0, sizeof(struct Option_Param_t));
			
		/*check conditional pool config file is exist or not*/
		if ((pconfig_file = fopen(DHCPD_OPTION_CONF_FILE, "r")) != NULL)
			config_file_exist = 1;
				
		if(config_file_exist) {
			/*read dhcp option60 pool config file*/
		memset(&option60_config, 0, sizeof(struct server_config_t));		
		read_config(&option60_config, DHCPD_OPTION_CONF_FILE);
			
			/*get option60 pool lease time*/
			if ((option = udhcp_find_option(option60_config.options, DHCP_LEASE_TIME))) {
				memcpy(&option60_config.max_lease_sec, option->data + 2, 4);
				option60_config.max_lease_sec = ntohl(option60_config.max_lease_sec);
			}
			else option60_config.max_lease_sec = DEFAULT_LEASE_TIME;
			
			/* Sanity check */
			num_ips = ntohl(option60_config.end_ip) - ntohl(option60_config.start_ip) + 1;
			if (option60_config.max_leases > num_ips) {
				log1("error: max_leases value (%lu) not sane, setting to %lu instead\n",option60_config.max_leases, num_ips);
				option60_config.max_leases = num_ips;
			}
			
			if (udhcp_read_interface(option60_config.interface, &option60_config.ifindex,
				&option60_config.server_nip, option60_config.server_mac) < 0)
			{
				retval = 1;
				goto ret;
			}
		}
		 
		if(config_file_exist) 
		{
			gTotalMaxLeases = dhcp_config.max_leases + option60_config.max_leases ;
		}
		else
			gTotalMaxLeases = dhcp_config.max_leases;	
	g_leases = xzalloc(gTotalMaxLeases * sizeof(g_leases[0]));
	memcpy(&server_config, &dhcp_config, sizeof(struct server_config_t));
#else
	g_leases = xzalloc(dhcp_config.max_leases * sizeof(g_leases[0]));
#endif

	/* Setup the signal pipe */
	udhcp_sp_setup();

 continue_with_autotime:
	timeout_end = monotonic_sec() + dhcp_config.auto_time;
	while (1) { /* loop until universe collapses */
		fd_set rfds;
		struct dhcp_packet packet;
		int bytes;
		struct timeval tv;
		uint8_t *server_id_opt;
		uint8_t *requested_ip_opt;
		uint32_t requested_nip = requested_nip; /* for compiler */
		uint32_t static_lease_nip;
		struct dyn_lease *lease, fake_lease;

		if (server_socket < 0) {
			server_socket = udhcp_listen_socket(/*INADDR_ANY,*/ SERVER_PORT,
					server_config.interface);
		}

		max_sock = udhcp_sp_fd_set(&rfds, server_socket);
		if (server_config.auto_time) {
			/* cast to signed is essential if tv_sec is wider than int */
			tv.tv_sec = (int)(timeout_end - monotonic_sec());
			tv.tv_usec = 0;
		}
		
	#if defined(TR111)
		tv.tv_sec = 1;
	#endif
		retval = 0;
	
		if (!server_config.auto_time || tv.tv_sec > 0)
		{
		#if defined(TR111) 
			retval = select(max_sock + 1, &rfds, NULL, NULL, &tv);
		#else
			retval = select(max_sock + 1, &rfds, NULL, NULL,server_config.auto_time ? &tv : NULL);
		#endif
		}
		if (retval == 0) 
		{
		#if defined(TR111)
			clock_gettime(CLOCK_MONOTONIC, &curtime);
			if(timeout_end <curtime.tv_sec)
			{
		#endif
		#ifdef DHCP_PROFILE
			write_leases_enhance(NULL);
		#else
			write_leases();
		#endif
		#if defined(TR111)
			}
			else
			{
				dhcpRefresh();
			}
		#endif
			goto continue_with_autotime;
		}
		if (retval < 0 && errno != EINTR) {
			log1("error on select");
			continue;
		}

		switch (udhcp_sp_read(&rfds)) {
		case SIGUSR1:
			bb_error_msg("received %s", "SIGUSR1");
		#ifdef DHCP_PROFILE
			write_leases_enhance(NULL);
		#else
			write_leases();
		#endif
			/* why not just reset the timeout, eh */
			goto continue_with_autotime;
		case SIGTERM:
			bb_error_msg("received %s", "SIGTERM");
		#ifdef DHCP_PROFILE
			write_leases_enhance(NULL);
		#else
			write_leases();
		#endif
			goto ret0;
		case 0: /* no signal: read a packet */
			break;
		default: /* signal or error (probably EINTR): back to select */
			continue;
		}

		bytes = udhcp_recv_kernel_packet(&packet, server_socket);
		if (bytes < 0) {
			/* bytes can also be -2 ("bad packet data") */
			if (bytes == -1 && errno != EINTR) {
				log1("read error: %s, reopening socket", strerror(errno));
				close(server_socket);
				server_socket = -1;
			}
			continue;
		}
		if (packet.hlen != 6) {
			bb_error_msg("MAC length != 6, ignoring packet");
			continue;
		}
		if (packet.op != BOOTREQUEST) {
			bb_error_msg("not a REQUEST, ignoring packet");
			continue;
		}

#ifdef DHCP_PROFILE
	if ((vendorID = udhcp_get_option(&packet, DHCP_VENDOR)) != NULL)
	{	
		if (config_file_exist && gOption_Param.opt60_enable)
		{
				if ((vendorID[OPT_LEN - 2] >= VENDOR_CLASS_ID_LEN_MAX)||(vendorID[OPT_LEN - 2] != strlen(gOption_Param.opt60_vid))||(strncmp(vendorID, gOption_Param.opt60_vid, vendorID[OPT_LEN - 2]) != 0))
				{		
					gOption60IsMatched = 0;//not match		
				}
				else
				{
					gOption60IsMatched = 1;//matc
				}
		}
		else
		{
			gOption60IsMatched = 0;
		}	
	}
		
	if(gOption60IsMatched)
	{		
		memcpy(&server_config, &option60_config, sizeof(struct server_config_t));//using option 60 pool
		gOption60IsMatched = 0;
		tcdbg_printf("goptionasdfasd = %d\n", gOption60IsMatched);	
	}	
	else
		memcpy(&server_config, &dhcp_config, sizeof(struct server_config_t));//using main pool
#endif
		state = udhcp_get_option(&packet, DHCP_MESSAGE_TYPE);
		if (state == NULL || state[0] < DHCP_MINTYPE || state[0] > DHCP_MAXTYPE) {
			bb_error_msg("no or bad message type option, ignoring packet");
			continue;
		}

		/* Get SERVER_ID if present */
		server_id_opt = udhcp_get_option(&packet, DHCP_SERVER_ID);
		if (server_id_opt) {
			uint32_t server_id_network_order;
			move_from_unaligned32(server_id_network_order, server_id_opt);
			if (server_id_network_order != server_config.server_nip) {
				/* client talks to somebody else */
				log1("server ID doesn't match, ignoring");
				continue;
			}
		}

		/* Look for a static/dynamic lease */
		static_lease_nip = get_static_nip_by_mac(server_config.static_leases, &packet.chaddr);
		if (static_lease_nip) {
			bb_error_msg("found static lease: %x", static_lease_nip);
			memcpy(&fake_lease.lease_mac, &packet.chaddr, 6);
			fake_lease.lease_nip = static_lease_nip;
			fake_lease.expires = 0;
			lease = &fake_lease;
		} else {
			lease = find_lease_by_mac(packet.chaddr);
		}

		/* Get REQUESTED_IP if present */
		requested_ip_opt = udhcp_get_option(&packet, DHCP_REQUESTED_IP);
		if (requested_ip_opt) {
			move_from_unaligned32(requested_nip, requested_ip_opt);
		}

		switch (state[0]) {

		case DHCPDISCOVER:
			log1("received %s", "DISCOVER");

			send_offer(&packet, static_lease_nip, lease, requested_ip_opt, arpping_ms);
			break;

		case DHCPREQUEST:
			log1("received %s", "REQUEST");
/* RFC 2131:

o DHCPREQUEST generated during SELECTING state:

   Client inserts the address of the selected server in 'server
   identifier', 'ciaddr' MUST be zero, 'requested IP address' MUST be
   filled in with the yiaddr value from the chosen DHCPOFFER.

   Note that the client may choose to collect several DHCPOFFER
   messages and select the "best" offer.  The client indicates its
   selection by identifying the offering server in the DHCPREQUEST
   message.  If the client receives no acceptable offers, the client
   may choose to try another DHCPDISCOVER message.  Therefore, the
   servers may not receive a specific DHCPREQUEST from which they can
   decide whether or not the client has accepted the offer.

o DHCPREQUEST generated during INIT-REBOOT state:

   'server identifier' MUST NOT be filled in, 'requested IP address'
   option MUST be filled in with client's notion of its previously
   assigned address. 'ciaddr' MUST be zero. The client is seeking to
   verify a previously allocated, cached configuration. Server SHOULD
   send a DHCPNAK message to the client if the 'requested IP address'
   is incorrect, or is on the wrong network.

   Determining whether a client in the INIT-REBOOT state is on the
   correct network is done by examining the contents of 'giaddr', the
   'requested IP address' option, and a database lookup. If the DHCP
   server detects that the client is on the wrong net (i.e., the
   result of applying the local subnet mask or remote subnet mask (if
   'giaddr' is not zero) to 'requested IP address' option value
   doesn't match reality), then the server SHOULD send a DHCPNAK
   message to the client.

   If the network is correct, then the DHCP server should check if
   the client's notion of its IP address is correct. If not, then the
   server SHOULD send a DHCPNAK message to the client. If the DHCP
   server has no record of this client, then it MUST remain silent,
   and MAY output a warning to the network administrator. This
   behavior is necessary for peaceful coexistence of non-
   communicating DHCP servers on the same wire.

   If 'giaddr' is 0x0 in the DHCPREQUEST message, the client is on
   the same subnet as the server.  The server MUST broadcast the
   DHCPNAK message to the 0xffffffff broadcast address because the
   client may not have a correct network address or subnet mask, and
   the client may not be answering ARP requests.

   If 'giaddr' is set in the DHCPREQUEST message, the client is on a
   different subnet.  The server MUST set the broadcast bit in the
   DHCPNAK, so that the relay agent will broadcast the DHCPNAK to the
   client, because the client may not have a correct network address
   or subnet mask, and the client may not be answering ARP requests.

o DHCPREQUEST generated during RENEWING state:

   'server identifier' MUST NOT be filled in, 'requested IP address'
   option MUST NOT be filled in, 'ciaddr' MUST be filled in with
   client's IP address. In this situation, the client is completely
   configured, and is trying to extend its lease. This message will
   be unicast, so no relay agents will be involved in its
   transmission.  Because 'giaddr' is therefore not filled in, the
   DHCP server will trust the value in 'ciaddr', and use it when
   replying to the client.

   A client MAY choose to renew or extend its lease prior to T1.  The
   server may choose not to extend the lease (as a policy decision by
   the network administrator), but should return a DHCPACK message
   regardless.

o DHCPREQUEST generated during REBINDING state:

   'server identifier' MUST NOT be filled in, 'requested IP address'
   option MUST NOT be filled in, 'ciaddr' MUST be filled in with
   client's IP address. In this situation, the client is completely
   configured, and is trying to extend its lease. This message MUST
   be broadcast to the 0xffffffff IP broadcast address.  The DHCP
   server SHOULD check 'ciaddr' for correctness before replying to
   the DHCPREQUEST.

   The DHCPREQUEST from a REBINDING client is intended to accommodate
   sites that have multiple DHCP servers and a mechanism for
   maintaining consistency among leases managed by multiple servers.
   A DHCP server MAY extend a client's lease only if it has local
   administrative authority to do so.
*/
			if (!requested_ip_opt) {
				requested_nip = packet.ciaddr;
				if (requested_nip == 0) {
					log1("no requested IP and no ciaddr, ignoring");
					break;
				}
			}
			if (lease && requested_nip == lease->lease_nip) {
				/* client requested or configured IP matches the lease.
				 * ACK it, and bump lease expiration time. */
				send_ACK(&packet, lease->lease_nip);
				break;
			}
			/* No lease for this MAC, or lease IP != requested IP */

			if (server_id_opt    /* client is in SELECTING state */
			 || requested_ip_opt /* client is in INIT-REBOOT state */
			) {
				/* "No, we don't have this IP for you" */
				send_NAK(&packet);
			} /* else: client is in RENEWING or REBINDING, do not answer */

			break;

		case DHCPDECLINE:
			/* RFC 2131:
			 * "If the server receives a DHCPDECLINE message,
			 * the client has discovered through some other means
			 * that the suggested network address is already
			 * in use. The server MUST mark the network address
			 * as not available and SHOULD notify the local
			 * sysadmin of a possible configuration problem."
			 *
			 * SERVER_ID must be present,
			 * REQUESTED_IP must be present,
			 * chaddr must be filled in,
			 * ciaddr must be 0 (we do not check this)
			 */
			log1("received %s", "DECLINE");
			if (server_id_opt
			 && requested_ip_opt
			 && lease  /* chaddr matches this lease */
			 && requested_nip == lease->lease_nip
			) {
				memset(lease->lease_mac, 0, sizeof(lease->lease_mac));
				lease->expires = time(NULL) + server_config.decline_time;
			}
			break;

		case DHCPRELEASE:
			/* "Upon receipt of a DHCPRELEASE message, the server
			 * marks the network address as not allocated."
			 *
			 * SERVER_ID must be present,
			 * REQUESTED_IP must not be present (we do not check this),
			 * chaddr must be filled in,
			 * ciaddr must be filled in
			 */
			log1("received %s", "RELEASE");
			
		#if defined(CWMP) && defined(TR111)
			if (check_vi(&packet))
				deleteDevice(&packet);
		#endif
			if (server_id_opt
			 && lease  /* chaddr matches this lease */
			 && packet.ciaddr == lease->lease_nip
			) {
				lease->expires = time(NULL);
			}
			break;

		case DHCPINFORM:
			log1("received %s", "INFORM");
		#if defined(CWMP) && defined(TR111)
			if (check_vi(&packet))
				insertDevice(&packet);
		#endif
			send_inform(&packet);
			break;
		}
	}
 ret0:
	retval = 0;
 ret:
	/*if (server_config.pidfile) - server_config.pidfile is never NULL */
		remove_pidfile(server_config.pidfile);
	#ifdef DHCP_PROFILE
		fclose(pconfig_file);
	#endif
	return retval;
}

#if defined(CWMP) && defined(TR111)
/* compare two mDev_t . return 0 means equal*/
int devCompare(mDev_t *dev1, mDev_t *dev2)
{
	int result = 1;
	if((dev1 != NULL) &&
		(dev2 != NULL) &&
		/*(memcmp(dev1->clientID, dev2->clientID, CLINETID_LENGTH) == 0) &&*/
		(strcmp(dev1->oui, dev2->oui) == 0) &&
		(strcmp(dev1->sn, dev2->sn) == 0) &&
		(strcmp(dev1->pclass, dev2->pclass) == 0))
		result = 0;
	return result;
}

/* check the vi option */
int check_vi(struct dhcp_packet *packet)
{
	uint8_t *vipos = NULL;
#ifdef TR111_TEST
	uint16_t entNum;
#else
	uint32_t entNum;
#endif
	int viflag = 0;

	if ((vipos = udhcp_get_option(packet, OPT_VENDOR_INFOR)) != NULL){
	#ifdef TR111_TEST
		memcpy(&entNum, vipos, DSL_FORUM_IANA_NUMBER_LENGTH_TEST);
		log1("info: Enterprise Number is : %0x\n",entNum);
		if(entNum != DSL_FORUM_IANA_NUMBER_TEST)
	#else
		memcpy(&entNum, vipos, DSL_FORUM_IANA_NUMBER_LENGTH);
		if(entNum != DSL_FORUM_IANA_NUMBER)
	#endif
		{
			log1("info: Enterprise Number is error: %0x\n",entNum);
			return 0;
		}
		viflag = 1;
	}
	return viflag;
}

/* parse mDev_t struct from dhcp_packet */
void parseDev(struct dhcp_packet *packet, mDev_t *dev)
{
	uint8_t* IDpos = udhcp_get_option(packet, DHCP_CLIENT_ID);
	uint8_t IDlength = CLINETID_LENGTH;
	uint8_t* vipos = udhcp_get_option(packet, OPT_VENDOR_INFOR);
	uint8_t ouilength = (uint8_t)(*(vipos+6));
	uint8_t snlength = (uint8_t)(*(vipos+8+ouilength));
	uint8_t pclasslength = *(vipos+10+ouilength+snlength);
	if(IDpos)
	{
		IDlength = (uint8_t)(*(IDpos+1));
	}

	if(IDlength > CLINETID_LENGTH)
	{
		IDlength = CLINETID_LENGTH;
	}

	if(IDpos)	
	{
		memcpy(dev->clientID, IDpos+2, sizeof(uint8_t)*IDlength);
	}
	else
	{
		memcpy(&dev->clientID[1], packet->chaddr, CLINETID_LENGTH -1);
	}
	
	memcpy(dev->oui, vipos+7, sizeof(uint8_t)*ouilength);
	memcpy(dev->sn, vipos+9+ouilength, sizeof(uint8_t)*snlength);
	memcpy(dev->pclass, vipos+11+ouilength+snlength, sizeof(uint8_t)*pclasslength);
	dev->next = NULL;
}
void hostindexinit(mDev_t *dev)
{
	int i;
	dev->hostnum =0;
	for(i = 0; i < 8; i++)
	{
		dev->hostindex[i] = 0;
	}
	return;
}
	
void gethostindex(struct dhcp_packet *packet, mDev_t *dev)
{
	unsigned int i;
	int currentLeaseNumer = 0;
	uint32_t tmp_time;

	//hostindexinit(dev);
	for (i = 0; i < gTotalMaxLeases; i++) {
		if (g_leases[i].lease_nip != 0) {

			/* screw with the time in the struct, for easier writing */
			tmp_time = g_leases[i].expires;
			
			if (server_config.remaining) {
				if (lease_expired(&(g_leases[i]))){
					g_leases[i].expires = 0;
					continue;
				}
			} 
			
			if(packet->yiaddr == g_leases[i].lease_nip)
			{
				dev->hostnum += 1;
				if((currentLeaseNumer >> 5) >= 8)
					continue;
				dev->hostindex[currentLeaseNumer >> 5] |= (1 << (currentLeaseNumer % 32));
			}
			
			currentLeaseNumer++;		
		}
	}
	return;
}

/* insert device to list */
void insertDevice(struct dhcp_packet *packet)
{
	int exist = 0;

	mDev_t* devp = (mDev_t*)malloc(sizeof(mDev_t));
	if(!devp)
	{		
		log1("error: malloc device node failed.\n");
		return;
	}
	memset(devp, 0, sizeof(mDev_t));
	parseDev(packet, devp);
	#if 0
	exist = findDevice(devp->clientID);
	#else
	//exist = findDevice(devp);
	exist = findDevice(packet, devp);
	#endif
	if (!exist && getDeviceNum() < server_config.max_leases) //prevent attacking
	{
		gTR111_Parm.VITail->next = devp;
		gTR111_Parm.VITail = devp;
		//LOG(LOG_INFO, "Add a device into list.");
		sendCwmpMsg(4); //message type:DEVICE_REINIT
		writeDevicesFile();
		/* for test */
		//cwmpShowmDev();
	}
	else
	{
		free(devp);
	}
	
	writeHostIndexFile();
	return;
}

int findDevice(struct dhcp_packet *packet,mDev_t* pdev)
{
	mDev_t* tmpnode = gTR111_Parm.VIHeader->next;
	while(tmpnode != NULL){
		if (devCompare(tmpnode, pdev) == 0)
		{
			gethostindex(packet, tmpnode);
			return 1;
		}
		tmpnode = tmpnode->next;
	}
	
	hostindexinit(pdev);
	gethostindex(packet, pdev);
	return 0;
}

/* delete device by id */
void deleteDevice(struct dhcp_packet *packet)
{
	//uint8_t* IDpos = get_option(packet, DHCP_SERVER_ID);
	mDev_t* prevnode = gTR111_Parm.VIHeader;
	mDev_t* tmpnode = gTR111_Parm.VIHeader->next;
	while (tmpnode != NULL){
		#if 0
		if (memcmp(tmpnode->clientID, IDpos, CLINETID_LENGTH) == 0)
		#endif
		{
			if(devCompare(tmpnode, gTR111_Parm.VITail) == 0)
			{
				gTR111_Parm.VITail = prevnode;
			}		
			prevnode->next = tmpnode->next;
			free(tmpnode);
			tmpnode = NULL;
			//LOG(LOG_INFO, "Delete a device from list.");
			sendCwmpMsg(4); //message type:DEVICE_REINIT
			writeDevicesFile();
			/* for test */
			//cwmpShowmDev();
			break;
		}
		prevnode = tmpnode;
		tmpnode = tmpnode->next;
	}
	writeHostIndexFile();
}

/* delete device by mac address */
void deleteDeviceByMac(uint8_t *macAddr)
{
	mDev_t* prevnode = gTR111_Parm.VIHeader;
	mDev_t* tmpnode = gTR111_Parm.VIHeader->next;
	while (tmpnode != NULL){
		if (memcmp(&(tmpnode->clientID[1]), macAddr, (CLINETID_LENGTH-1)) == 0){
			if(devCompare(tmpnode, gTR111_Parm.VITail) == 0)
			{
				gTR111_Parm.VITail = prevnode;
			}		
			prevnode->next = tmpnode->next;
			free(tmpnode);
			tmpnode = NULL;
			//LOG(LOG_INFO, "Delete a device byMac from list.");
			writeDevicesFile();
			/* for test */
			//cwmpShowmDev();
			break;
		}
		prevnode = tmpnode;
		tmpnode = tmpnode->next;
	}
	writeHostIndexFile();
}

/* get length of device list */
unsigned long getDeviceNum(void)
{
	mDev_t* tmpnode = gTR111_Parm.VIHeader->next;
	unsigned long num = 0;
	while (tmpnode != NULL)
	{
		num++;
		tmpnode = tmpnode->next;
	}
	return num;
}

void writeHostIndexFile(void)
{
	FILE *fp;
	char bufNumber[100] = {0};
	char bufIndex[120] = {0};
	int i = 0;//must be initilaze to 0
	mDev_t* tmpnode = gTR111_Parm.VIHeader->next;

	if (!(fp = fopen(DEVICE_HOSTINDEX_INFO_FILE, "w")))
	{
		log1("error: Unable to open %s for writing\n",DEVICE_HOSTINDEX_INFO_FILE);
		return;
	}

	sprintf(bufNumber, "totalCount=%lu\n", getDeviceNum());
	fwrite(bufNumber, strlen(bufNumber), 1, fp);
	while (tmpnode != NULL)
	{
		memset(bufNumber,0,sizeof(bufNumber));
		sprintf(bufNumber, "HostCount%d=%lu\n",i, tmpnode->hostnum);
		fwrite(bufNumber, strlen(bufNumber), 1, fp);
		sprintf(bufIndex, "HostIndex%d=%lu %lu %lu %lu %lu %lu %lu %lu\n",i, tmpnode->hostindex[0],tmpnode->hostindex[1], \
			tmpnode->hostindex[2], tmpnode->hostindex[3], tmpnode->hostindex[4], tmpnode->hostindex[5], tmpnode->hostindex[6], tmpnode->hostindex[7]);
		fwrite(bufIndex, strlen(bufIndex), 1, fp);
		i++;
		tmpnode = tmpnode->next;
	}
	fclose(fp);
	return;
}

/* write devices list to file */
void writeDevicesFile(void)
{
	FILE *fp;
	char bufNumber[100] = {0};
	char bufoui[100] = {0};
	char bufsn[100] = {0};
	char bufpclass[100] = {0};
	int i = 0;
	mDev_t* tmpnode = gTR111_Parm.VIHeader->next;
	if (!(fp = fopen(DEVICE_INFO_FILE, "w"))) {
		log1("error: Unable to open %s for writing\n",DEVICE_INFO_FILE);
		return;
	}
	sprintf(bufNumber, "totalCount = %-8lu\n", getDeviceNum());
	fwrite(bufNumber, strlen(bufNumber), 1, fp);
	while (tmpnode != NULL){
		sprintf(bufoui, "oui%d    = %-6s\n", i, tmpnode->oui);
		sprintf(bufsn, "sn%d     = %-64s\n", i, tmpnode->sn);
		sprintf(bufpclass, "pclass%d = %-64s\n", i, tmpnode->pclass);
		
		fwrite(bufoui, strlen(bufoui), 1, fp);
		fwrite(bufsn, strlen(bufsn), 1, fp);
		fwrite(bufpclass, strlen(bufpclass), 1, fp);
		i++;
		tmpnode = tmpnode->next;
	}
	fclose(fp);

}

/*show device list info*/
void cwmpShowmDev(void)
{
	mDev_t* tmpnode = gTR111_Parm.VIHeader->next;
	uint32_t num = 0;
	int i;
	num = getDeviceNum();
	log1("info: **********show device list start************\n");
	log1("info: num of mdev = %d\n",num);
	while (tmpnode != NULL)
	{		
		for(i = 0; i<CLINETID_LENGTH; i++)
		{
			log1("info:clientID value is 0X%02X.\n",tmpnode->clientID[i]);
		}
		log1("info: oui = %s\n", tmpnode->oui);
		log1("info: sn = %s\n", tmpnode->sn);
		log1("info: pclass = %s\n", tmpnode->pclass);
		tmpnode = tmpnode->next;
	}
	log1("info: **********show device list end************\n");
}

void dhcpRefresh(void)
{
	char value[10];
	int r_val;
	uint32_t limitTime;
	int attrLimitTime;
	
	unsigned int i;
	unsigned long tmp_time;
	uint8_t macAddr[CLINETID_LENGTH-1] = {0};

	for (i = 0; i < gTotalMaxLeases; i++) {
		if (g_leases[i].lease_nip != 0) {
			/* screw with the time in the struct, for easier writing */
			tmp_time = g_leases[i].expires;
			if (server_config.remaining) {
				if (lease_expired(&(g_leases[i])))
				{
					//LOG(LOG_INFO, "entry[%d] expired", i);
					/* remove dhcp client from ManageableDev_S*/
					memcpy(macAddr, g_leases[i].lease_mac, sizeof(uint8_t)*(CLINETID_LENGTH-1));
					deleteDeviceByMac(macAddr);					
				}
			}
		}
	}

	memset(value, 0, sizeof(value));
	
	r_val = tcapi_get("DhcpClientLimit_Entry", "time", value);
	limitTime = strtoul(value, NULL, 10);
	
	r_val = tcapi_get("TR069Attr_SpeAttr", "dhcpClientCount", value);
	attrLimitTime = atoi(value);

	if(attrLimitTime & ATTRIBUTE_ACTIVE)
{
		if(1 == gTR111_Parm.iNotifyFlag)
		{
			if(gTR111_Parm.iTotalNumber != getDeviceNum())
			{
				sendCwmpMsg(2); //messaye type:VALUE_CHANGED
				gTR111_Parm.iTotalNumber = getDeviceNum();
				gTR111_Parm.iNotifyFlag = 0;
			}
		}
		else
		{
			if(gTR111_Parm.iNotifyTimer >= limitTime)
			{
				gTR111_Parm.iNotifyTimer = 0; //reset timer
				if(gTR111_Parm.iTotalNumber != getDeviceNum())
				{
					sendCwmpMsg(2); //messaye type:VALUE_CHANGED
					gTR111_Parm.iTotalNumber = getDeviceNum();
				}
				else
				{
					gTR111_Parm.iNotifyFlag = 1;
				}
			}
			else
			{
				gTR111_Parm.iNotifyTimer++;
			}
		}
	}
}
	
void addVIOption(uint8_t *optionptr)
{
	uint8_t oui_len;
	uint8_t sn_len;
	uint8_t pclass_len;
	uint8_t subopt_len;
	uint8_t total_len;
	char oui[8] = "";
	char sn[68] = "";
	char pclass[68] = "";
	uint8_t data[150];
	int i = 0;
	int r_val;
#if/*TCSUPPORT_TTNET*/ defined(TCSUPPORT_TTNET)	
	int nm_MAC[7] ;
#endif/*TCSUPPORT_TTNET*/
	memset(oui, 0, sizeof(oui));
	memset(sn, 0, sizeof(sn));
	memset(pclass, 0, sizeof(pclass));
	memset(data, 0, sizeof(data));
	
	r_val = tcapi_get("Cwmp_Entry", "ManufacturerOUI", oui);
	if(r_val < 0)
		strcpy(oui, "N/A");
	r_val = tcapi_get("SysInfo_Entry", "SerialNum", sn);
	if(r_val < 0)
		strcpy(sn, "N/A");
#ifdef TCSUPPORT_TTNET
	r_val= tcapi_get("Info_Ether", "mac", sn);

	if(r_val < 0)
		strcpy(sn, "N/A");
	else
       {
       	sscanf(sn, "%02x:%02x:%02x:%02x:%02x:%02x", &nm_MAC[0],&nm_MAC[1],&nm_MAC[2],&nm_MAC[3],&nm_MAC[4],&nm_MAC[5]);
		memset(sn, 0, sizeof(sn));
		sprintf(sn, "%02X%02X%02X%02X%02X%02X",  nm_MAC[0] , nm_MAC[1] , nm_MAC[2] , nm_MAC[3] , nm_MAC[4] , nm_MAC[5]);
	}
#endif
	r_val = tcapi_get("Cwmp_Entry", "ProductClass", pclass);

	if(r_val < 0)
		strcpy(pclass, "N/A");
		
	oui_len = strlen(oui);
	sn_len = strlen(sn);
	pclass_len = strlen(pclass);
	
	/* subopt length = (oui code+oui length+oui lenght) + (sn code+sn length+sn length) + (pclass code+pclass length+pclass length) */
	subopt_len = 6 + oui_len + sn_len + pclass_len;
	total_len = DSL_FORUM_IANA_NUMBER_LENGTH + 1 + subopt_len;
	
	/* option code + length(total length)*/
	data[i++] = OPT_VENDOR_INFOR;
	data[i++] = total_len;

	/*put DSL_FORUM_IANA_NUMBER*/
	data[i++] = DSL_FORUM_IANA_NUMBER>>24;
	data[i++] = DSL_FORUM_IANA_NUMBER>>16;
	data[i++] = DSL_FORUM_IANA_NUMBER>>8;
	data[i++] = DSL_FORUM_IANA_NUMBER;
	
	/* option code + length(suboption length) */
	data[i++] = subopt_len;
	
	/* For oui: option code + length + data */
	data[i++] = 0x4;
	data[i++] = oui_len;
	memcpy(&data[i], oui, oui_len);
	i += oui_len;
	
	/* For sn: option code + length + data */
	data[i++] = 0x5;
	data[i++] = sn_len;
	memcpy(&data[i], sn, sn_len);
	i += sn_len;

	/* For pclass: option code + length + data */
	data[i++] = 0x6;
	data[i++] = pclass_len;
	memcpy(&data[i], pclass, pclass_len);
	 
	add_option_string(optionptr, data);
}

#endif
	
#if defined(CWMP)
void sendCwmpMsg(int msgType)
{
/* we will send 3 kinds of msg
  * 2: VALUE_CHANGED
  * 3: HOST_REINIT
  * 4: DEVICE_REINIT
  */
	int ret;
	long type = 1;
	int flag = 0;
	
	cwmp_msg_t message;
	memset(&message,0,sizeof(cwmp_msg_t));
	message.cwmptype = msgType;
	ret = sendmegq(type,&message,flag);
	
	if(ret < 0)
	{
		tcdbg_printf("\r\nsend message error!!");
		return;
}
	return;
}
#endif

#ifdef DHCP_PROFILE
void addOPT240Option(uint8_t *optionptr)
{
	uint8_t data[150];
	char opt240Str[64];

	memset(opt240Str, 0, sizeof(opt240Str));

	sprintf(opt240Str, "%s", gOption_Param.opt240_value);
	/* For option 240: option code + length + data */
	data[OPT_CODE] = 240;
	data[OPT_LEN] = strlen(opt240Str);
	memcpy(&data[OPT_DATA], opt240Str, strlen(opt240Str));
	add_option_string(optionptr, data);
}
#endif

#if defined(DHCP_PROFILE)||(defined(CWMP) && defined(TR111))
int add_option_string(uint8_t *optionptr, uint8_t *string)
{
	int end = udhcp_end_option(optionptr);

	/* end position + string length + option code/length + end option */
	if (end + string[OPT_LEN] + 2 + 1 >= 308) {
		log1("error: Option 0x%02x did not fit into the packet!", string[OPT_CODE]);
		return 0;
	}
	log1("info: adding option 0x%02x", string[OPT_CODE]); 
	memcpy(optionptr + end, string, string[OPT_LEN] + 2);
	optionptr[end + string[OPT_LEN] + 2] = DHCP_END;
	return string[OPT_LEN] + 2;
}
#endif

