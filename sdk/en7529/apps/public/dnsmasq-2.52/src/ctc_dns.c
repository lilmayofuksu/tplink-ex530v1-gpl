/* dnsmasq is Copyright (c) 2000-2010 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991, or
   (at your option) version 3 dated 29 June, 2007.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
      
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* The SURF random number generator was taken from djbdns-1.05, by 
   Daniel J Bernstein, which is public domain. */


#include "dnsmasq.h"
#if defined(TCSUPPORT_CT_JOYME4) || defined(TCSUPPORT_CT_UBUS) || defined(RA_PARENTALCONTROL)
#include "dns_url_filter_db.h"
#endif

#define MACADDR	18
#define IPTABLES_DROP_ALL		"%s -A url_%d_https_filter -p tcp --dport 443 -m mac --mac-source %s -j DROP"
#define IPTABLES_DROP_ALL_DEL	"%s -D url_%d_https_filter -p tcp --dport 443 -m mac --mac-source %s -j DROP"
#define IPTABLES_DROP_ONE		"%s -A url_%d_https_filter -p tcp --dport 443 -d %s -m mac --mac-source %s -j DROP"
#define IPTABLES_ACCEPT_ALL		"%s -A url_%d_https_filter -m mac --mac-source %s -j ACCEPT"
#define IPTABLES_ACCEPT_ONE		"%s -A url_%d_https_filter -d %s -m mac --mac-source %s -j ACCEPT"

#define MAC_STR_LEN				12
#define MAC_STR_DOT_LEN			17
#define MAX_PARENTAL_NUM	8
#define MAX_MAC_PER_ENTRY	4
#define MAX_URL_PER_ENTRY	10

#if defined(TCSUPPORT_CT_JOYME4) || defined(TCSUPPORT_CT_UBUS)
#define MAX_DNS_FILTER_NUM	32
#define MAX_URL_FILTER_NUM	33
#else
#define MAX_DNS_FILTER_NUM	100
#define MAX_URL_FILTER_NUM	100
hashmap_map *URLFilterMap = NULL;
hashmap_map *DNSFilterMap = NULL;
#endif

char *string_toUper(char *string)
{
	int i =0;

	while(string[i]!='\0'){
		if (string[i]>=97 && string[i]<=122){
			string[i]=string[i]-32;
		}
		i++;
	}
	return string;
}

int mac_rm_dot(char *old_mac, char *new_mac)
{
	int i = 0, j = 0, mac_len = MAC_STR_DOT_LEN;

	if (old_mac == NULL || new_mac == NULL)
		return -1;
	
	for (i = 0; i < mac_len; i++) {
		if (old_mac[i] != ':')
			new_mac[j++] = old_mac[i];
	}
	new_mac[MAC_STR_LEN] = '\0';
	return 0;
}

int parseMac(char *mac)
{
	char *p = NULL;
	int i = 0, j = 0;
	char tmpVal[20] = {0}, MacAddr[512] = {0};

	p = strtok(mac, ",");
	while ( p )
	{
		j = 0;
		memset(tmpVal, 0, sizeof(tmpVal));
		if ( strlen(p) > 12 )
			continue;

		for ( i = 0; i < 12; i++)
		{
			tmpVal[j] = p[i];
			j++;i++;
			tmpVal[j] = p[i];
			j++;
			tmpVal[j] = ':';
			j++;
		}
		tmpVal[17] = '\0';
		strncat(MacAddr, tmpVal, sizeof(MacAddr) - strlen(MacAddr) - 1);
		strncat(MacAddr, ",", sizeof(MacAddr) - strlen(MacAddr) - 1);
		p = strtok(NULL, ",");
	}

	strncpy(mac, MacAddr, MAX_MAC_LEN - 1);
	return 0;
}

int setIptablesBWRules(struct url_filter_entry **match, int family, int policy, char *addr)
{
	char *pValue = NULL;
	char cmdBuf[256] = {0};
	char iptabletype[20] = {0};
	char tmpBuf[256] = {0};

	if (AF_INET == family)
	{
		snprintf(iptabletype, sizeof(iptabletype), "iptables");
		(*match)->v4_add_flag = 1;
	}
	else if (AF_INET6 == family)
	{
		snprintf(iptabletype, sizeof(iptabletype), "ip6tables");
		(*match)->v6_add_flag = 1;
	}
	if(!strcmp((*match)->Mac, ""))
	{
		if(0 == policy)
		{
			snprintf(cmdBuf, sizeof(cmdBuf), "%s -A url_https_filter -p tcp --dport 443 -d %s -j DROP", iptabletype, addr);
			system(cmdBuf);
		}		
	} 
	else 
	{
		strncpy(tmpBuf, (*match)->Mac, sizeof(tmpBuf) - 1);
		pValue = strtok(tmpBuf, ",");
		while(pValue)
		{
			if(0 == policy)
			{
				snprintf(cmdBuf, sizeof(cmdBuf), "%s -A url_https_filter -p tcp --dport 443 -d %s -m mac --mac-source %s -j DROP", iptabletype, addr, pValue);
				system(cmdBuf);
			}
			else
			{
				snprintf(cmdBuf, sizeof(cmdBuf), "%s -A url_https_filter -d %s -m mac --mac-source %s -j ACCEPT", iptabletype, addr, pValue);
			}
			pValue = strtok(NULL, ",");
		}
		if (0 != policy)
		{
			snprintf(cmdBuf, sizeof(cmdBuf), "%s -A url_https_filter -p tcp --dport 443 -d %s -j DROP", iptabletype, addr);
			system(cmdBuf);
		}
	}
	return 0;
}

int setURLFilterRule(struct url_filter_entry **match, char *addr, int family)
{
	char cmdBuf[256] = {0};
	char nodeName[32] = {0};
	char tmpBuf[20] = {0};
	int policy = 0;
	int ret = 0;

	memset(cmdBuf, 0, sizeof(cmdBuf));
	memset(nodeName, 0, sizeof(nodeName));
	snprintf(nodeName, sizeof(nodeName), "UrlFilter_Common");
	if (0 != tcapi_get(nodeName, "Filter_Policy", tmpBuf) || '\0' == tmpBuf[0])
		policy = 0;
	else
		policy = atoi(tmpBuf);

	ret = setIptablesBWRules(match, family, policy, addr);
	return ret;
}

int doSetRule(struct url_filter_entry **match, struct all_addr *addr, int flags)
{
	int family = 0;
	char ipaddr[64] = {0};
	
	if (flags & F_IPV4)
	{
		family = AF_INET;
		inet_ntop(AF_INET, addr
				, ipaddr, sizeof(ipaddr));
		if((1 == (*match)->chain_rule_add_flag) && (*match)->v4_add_flag )
			return 0;
			
	}
	else if (flags & F_IPV6)
	{
		family = AF_INET6;
		inet_ntop(AF_INET6, addr
				, ipaddr, sizeof(ipaddr));
		if((1 == (*match)->chain_rule_add_flag) && (*match)->v6_add_flag )
			return 0;
	}

	setURLFilterRule(match, ipaddr, family);

	return 0;
}

#if defined(TCSUPPORT_CT_JOYME4) || defined(TCSUPPORT_CT_UBUS)
/* init node dnsfilter in hashmap */
int initDNSFilter(void)
{
	dns_urlfilter_data data = {0};
	int i = 0;
	char nodeName[32] = {0};
	char hostName[256] = {0};
	char hostName0[256] = {0};
	char tmphostName[256] = {0};
	char attrName[32] = {0};
	char Enable[4] = {0}; 
	char MacAddr[18] = {0};
	char Mode[8] = {0};
	char induration[8] = {0};
	char blockTimes[64] = {0};
	int blocktimes = 0;	
	char hostnamenum_s[8] = {0};
	int hostnamenum_i = 0;
	char action_s[8] = {0};
	int action_i = 0;
	int mode = 0;
	int j = 0; 

	/* max 32 entry */
	for (i = 0;i < MAX_DNS_FILTER_NUM; i++)
	{
		memset(nodeName, 0, sizeof(nodeName));
		snprintf(nodeName, sizeof(nodeName), "DnsFilter_Entry%d", i);
		if (0 != tcapi_get(nodeName, "Enable", Enable) || '\0' == Enable[0] || !strcmp(Enable, "0"))
			continue;

		memset(Mode, 0, sizeof(Mode));
		mode = 0;
		if (0 == tcapi_get(nodeName, "Filter_Policy", Mode) && '\0' != Mode[0])
			mode = atoi(Mode);


		memset(action_s, 0, sizeof(action_s));
		action_i = 0;
		if (0 == tcapi_get(nodeName, "Action", action_s) && '\0' != action_s[0])
			action_i = atoi(action_s);
		else
			continue;

		memset(blockTimes, 0, sizeof(blockTimes));
		blocktimes = 0;
		if (0 == tcapi_get(nodeName, "dnsBlockedTimes", blockTimes) && '\0' != blockTimes[0])
			blocktimes = atoi(blockTimes);

		memset(hostnamenum_s, 0, sizeof(hostnamenum_s));
		hostnamenum_i = 0;
		if (0 == tcapi_get(nodeName, "hostname_num", hostnamenum_s) && '\0' != hostnamenum_s[0])
			hostnamenum_i = atoi(hostnamenum_s);

		/* no need to init other url when URL0 is '*' */
		memset(hostName0, 0, sizeof(hostName0));
		tcapi_get(nodeName, "URL0", hostName0);
		if(!strcmp(hostName0, "*"))
			hostnamenum_i = 0;

		memset(induration, 0, sizeof(induration));
		if (0 != tcapi_get(nodeName, "induration", induration) || '\0' == induration[0] || !strcmp(induration, "0"))
			continue;

		memset(MacAddr, 0, sizeof(MacAddr));
		tcapi_get(nodeName, "MAC", MacAddr);

		/* each entry's has 99 hostname */
		for(j = 0; j < (hostnamenum_i + 1); j++)
		{	
			bzero(&data, sizeof(data));

			memset(attrName, 0, sizeof(attrName));
			snprintf(attrName, sizeof(attrName), "HostName_new%d", j);
			memset(hostName, 0, sizeof(hostName));
			if (0 != tcapi_get(nodeName, attrName, hostName) || '\0' == hostName[0])
				continue;

			//memset(tmphostName, 0, sizeof(tmphostName));
			if (NULL != strstr(hostName, "https://"))
				strncpy(data.domain, hostName + strlen("https://"), sizeof(hostName) - 1);
			else if (NULL != strstr(hostName, "http://"))
				strncpy(data.domain, hostName + strlen("http://"), sizeof(hostName) - 1);
			else
				strncpy(data.domain, hostName, sizeof(hostName) - 1);
 
			data.cmd_type = E_URL_ADD;
			data.mode = mode;
			data.action = action_i;
			data.times = blocktimes;
			if(!strcmp(data.domain, "*"))
				data.match_all = 1;
			else
				data.match_all = 0;
			data.index = i;
	
			/*set into dns hashmap    1:dnsfilter*/
			urlfiter_setup(&data, MacAddr, DNS_FILTER_TYPE);
		}
	}
	return 0;
}

/* init urlfilter */
int initURLFilterInfo(void)
{
	dns_urlfilter_data data = {0};
	int i = 0;
	char nodeName[32] = {0};
	char hostName[256] = {0};
	char tmphostName[256] = {0};
	char attrName[32] = {0};
	char portName[32] = {0};
	char Enable[4] = {0};
	char MacAddr[18] = {0};
	char Mode[8] = {0};
	char induration[8] = {0};
	char hostnamenum_s[8] = {0};
	char blockTimes[64] = {0};
	char hostName0[256] = {0};
	int blocktimes = 0;
	char Port[20] = {0};
	int port = 80;
	char *pValue = NULL;
	int hostnamenum_i = 0;
	int mode = 0;
	int j = 0;
	void *value = NULL;
	int dns_filter_action = 0;

	for (i = 0; i < MAX_URL_FILTER_NUM; i++)
	{		
		memset(nodeName, 0, sizeof(nodeName));
		snprintf(nodeName, sizeof(nodeName), "UrlFilter_Entry%d", i);
		if (0 != tcapi_get(nodeName, "Activate", Enable) || '\0' == Enable[0] || !strcmp(Enable, "0"))
			continue;
		
		memset(Mode, 0, sizeof(Mode));
		mode = 0;
		if (0 == tcapi_get(nodeName, "Filter_Policy", Mode) && '\0' != Mode[0])
			mode = atoi(Mode);

		memset(blockTimes, 0, sizeof(blockTimes));
		blocktimes = 0;
		if (0 == tcapi_get(nodeName, "dnsBlockedTimes", blockTimes) && '\0' != blockTimes[0])
			blocktimes = atoi(blockTimes);
		
		memset(hostnamenum_s, 0, sizeof(hostnamenum_s));
		hostnamenum_i = 0;
		if (0 == tcapi_get(nodeName, "url_num", hostnamenum_s) && '\0' != hostnamenum_s[0])
			hostnamenum_i = atoi(hostnamenum_s);

		/* no need to init other url when URL0 is '*' */
		memset(hostName0, 0, sizeof(hostName0));
		tcapi_get(nodeName, "URL0", hostName0);
		if(hostName0[0] != 0)
			hostnamenum_i++;
		if(!strcmp(hostName0, "*"))
			hostnamenum_i = 1;

		memset(induration, 0, sizeof(induration));
		if (0 != tcapi_get(nodeName, "induration", induration) || '\0' == induration[0] || !strcmp(induration, "0"))
			continue;

		memset(MacAddr, 0, sizeof(MacAddr));
		tcapi_get(nodeName, "MAC", MacAddr);
		for(j = 0; j < 100; j++)
		{	
			bzero(&data, sizeof(data));
			memset(attrName, 0, sizeof(attrName));
			memset(hostName, 0, sizeof(hostName));
			snprintf(attrName, sizeof(attrName), "URL%d", j);
			memset(hostName, 0, sizeof(hostName));
			if (0 != tcapi_get(nodeName, attrName, hostName) || '\0' == hostName[0])
				continue;

			/*cut https:// and http://*/
			//memset(tmphostName, 0, sizeof(tmphostName));
			if (NULL != strstr(hostName, "https://"))
				strncpy(data.domain, hostName + strlen("https://"), sizeof(hostName) - 1);
			else if (NULL != strstr(hostName, "http://"))
				strncpy(data.domain, hostName + strlen("http://"), sizeof(hostName) - 1);
			else
				strncpy(data.domain, hostName, sizeof(hostName) - 1);
	
			data.cmd_type = E_URL_ADD;
			data.mode = mode;
			if(!strcmp(data.domain, "*"))
				data.match_all = 1;
			else
				data.match_all = 0;
			data.index = i;
			data.times = blocktimes;

			urlfiter_setup(&data, MacAddr, URL_FILTER_TYPE);/*urlfilter hash :2*/
			if(hostnamenum_i == j + 1)
				break;
		}
	}

	return 0;
}

int doURLMatch(union mysockaddr *source_addr, int family)
{
	unsigned char hw_addr[ETH_ALEN];
	char nodeName[32] = {0};
	char blockTimes[20] = {0};
	int index = 0;
	int ret = 0;
	int mode = 0, ismatchall = 0;
	dns_urlfilter_info blockdata = {0};

	getHWAddrByIP(hw_addr, family, source_addr);

	ret = url_list_match_hash(daemon->namebuff, hw_addr, &index, URL_FILTER_TYPE, &mode, &ismatchall, NULL);
	
	/*0 drop, 1 continue*/
	if(0 == ret)
	{
		memset(nodeName, 0, sizeof(nodeName));
		snprintf(nodeName, sizeof(nodeName), "UrlFilter_Entry%d", index);
		/* get blocktimes */
		bzero(&blockdata, sizeof(blockdata));
		blockdata.index = index;
		urlfiter_get_blocked_times(&blockdata, URL_FILTER_TYPE);
		snprintf(blockTimes, sizeof(blockTimes), "%lu", blockdata.blocked_times);
		tcapi_set(nodeName, "dnsBlockedTimes", blockTimes);
		
		return 1;
	}
	
	return 0;
}

int DNSFilterMatch(HEADER *header, ssize_t n, union mysockaddr *source_addr, struct all_addr *dst_addr, int if_index, int family, int fd)
{
	unsigned char hw_addr[ETH_ALEN];
	char MacAddr[18] = {0};
	char nodeName[32] = {0};
	char blocktimes[20] = {0};
	char lan_ip[20] = {0};
	char Action[8] = {0};
	char blockTimes[64] = {0};
	int action = 0;
	struct all_addr rep_addr;
	size_t t_plen = 0;
	int index = 0;
	int ret = 0;
	int mode = 0, ismatchall = 0;
	dns_urlfilter_info blockdata = {0};
	
	getHWAddrByIP(hw_addr, family, source_addr);

	ret = url_list_match_hash(daemon->namebuff, hw_addr, &index, DNS_FILTER_TYPE, &mode, &ismatchall, &action);

	if(0 == ret)
	{	
		memset(nodeName, 0, sizeof(nodeName));
		snprintf(nodeName, sizeof(nodeName), "DnsFilter_Entry%d", index);
		
		/* get blocktimes */
		bzero(&blockdata, sizeof(blockdata));
		blockdata.index = index;
		urlfiter_get_blocked_times(&blockdata, DNS_FILTER_TYPE);
		
		snprintf(blockTimes, sizeof(blockTimes), "%lu", blockdata.blocked_times);
		tcapi_set(nodeName, "BlockedTimes", blockTimes);
		
		if(action == 0)
		{
			return 1;
		}
		else if(action == 1)
		{
			memset(nodeName, 0, sizeof(nodeName));
			strncpy(nodeName, "Lan_Entry0", sizeof(nodeName) - 1);
			if ( T_A == daemon->qtype )
			{
				tcapi_get(nodeName, "IP", lan_ip);
				rep_addr.addr.addr4.s_addr = inet_addr(lan_ip);
				t_plen = setup_reply(header, (size_t)n, &rep_addr, F_IPV4, daemon->local_ttl);

			}
#ifdef HAVE_IPV6
			else if ( T_AAAA == daemon->qtype )
			{
				tcapi_get(nodeName, "IP6", lan_ip);
				inet_pton(AF_INET6, lan_ip, &rep_addr);
				t_plen = setup_reply(header, (size_t)n, &rep_addr, F_IPV6, daemon->local_ttl);
			}
#endif

			send_from(fd, daemon->options & OPT_NOWILD, (char *)header, t_plen, source_addr, dst_addr, if_index);
			return 1;
		}
		else if(action == 2)
		{
			t_plen = setup_reply(header, (size_t)n, NULL, F_NXDOMAIN, daemon->local_ttl);
			send_from(fd, daemon->options & OPT_NOWILD, (char *)header, t_plen, source_addr, dst_addr, if_index);
			return 1;
		}
		else
		{
			tcdbg_printf("wrong dns filter action!\r\n");
		}
	}

	return 0;
}

/* 1:setted */
int URLFilterMatch(union mysockaddr *source_addr, struct all_addr *addr, int flags)
{
	unsigned char hw_addr[ETH_ALEN];
	char MacAddr[MACADDR] = {0};
	char tmp_key1[MAX_HOSTNAME_LEN + MACADDR] = {0};
	char nodeName[32] = {0};
	char blockTimes[20] = {0};	
	int family = 0;
	char ipaddr[64] = {0};
	char cmdBuf[256] = {0};
	char iptabletype[20] = {0};
	char tmpBuf[256] = {0};
	dns_urlfilter_info blockdata = {0};	
	struct iptables_entry *iptabinfo = NULL;	
	void *value = NULL;
	int index = 0;
	int ret = 0, is_setted = 0;
	int mode = 0, ismatchall = 0;
	static int dropall_flag = 0;

	if (flags & F_IPV4)
	{
		family = AF_INET;
		inet_ntop(AF_INET, addr
				, ipaddr, sizeof(ipaddr));
		snprintf(iptabletype, sizeof(iptabletype), "iptables");
			
	}
	else if (flags & F_IPV6)
	{
		family = AF_INET6;
		inet_ntop(AF_INET6, addr
				, ipaddr, sizeof(ipaddr));
		snprintf(iptabletype, sizeof(iptabletype), "ip6tables");
	}
	
	if(!source_addr)
		return 0;

	getHWAddrByIP(hw_addr, family, source_addr);
	
	snprintf(MacAddr, sizeof(MacAddr),"%02X:%02X:%02X:%02X:%02X:%02X"
				, hw_addr[0], hw_addr[1], hw_addr[2]
				, hw_addr[3], hw_addr[4], hw_addr[5]);

	ret = url_list_match_hash(daemon->namebuff, hw_addr, &index, URL_FILTER_TYPE, &mode, &ismatchall, NULL);

	if(ret == 0)
	{
		memset(nodeName, 0, sizeof(nodeName));
		snprintf(nodeName, sizeof(nodeName), "UrlFilter_Entry%d", index);
		/* get blocktimes */
		bzero(&blockdata, sizeof(blockdata));
		blockdata.index = index;
		urlfiter_get_blocked_times(&blockdata, URL_FILTER_TYPE);
		
		snprintf(blockTimes, sizeof(blockTimes), "%lu", blockdata.blocked_times);
		tcapi_set(nodeName, "dnsBlockedTimes", blockTimes);

	}

	/* 1:iptables setted ,-1/-2: fail to malloc*/
	if(mode == 0 && ret == 0)/* mode black, ret drop */
	{	
		is_setted = check_iptables_hash(ipaddr, MacAddr, ismatchall);
		if(1 == is_setted)
			return 0;

		if(ismatchall == 0)
			snprintf(cmdBuf, sizeof(cmdBuf), IPTABLES_DROP_ONE, iptabletype, index, ipaddr, MacAddr);
		else
			snprintf(cmdBuf, sizeof(cmdBuf), IPTABLES_DROP_ALL, iptabletype, index, MacAddr);

		system(cmdBuf);

	}
	else if(mode == 1)/* mode white, ret continue */
	{
		if(ret == 1)
		{
			is_setted = check_iptables_hash(ipaddr, MacAddr, ismatchall);
			if(1 == is_setted)
				return 0;

			if(ismatchall == 0)
				snprintf(cmdBuf, sizeof(cmdBuf), IPTABLES_ACCEPT_ONE, iptabletype, index, ipaddr, MacAddr);
			else
				snprintf(cmdBuf, sizeof(cmdBuf), IPTABLES_ACCEPT_ALL, iptabletype, index, MacAddr);

			system(cmdBuf);
			dropall_flag = 0;
		}

		if(dropall_flag == 0)
		{
			snprintf(cmdBuf, sizeof(cmdBuf), IPTABLES_DROP_ALL_DEL, iptabletype, index, MacAddr);
			system(cmdBuf);		

			snprintf(cmdBuf, sizeof(cmdBuf), IPTABLES_DROP_ALL, iptabletype, index, MacAddr);
			system(cmdBuf);	

			dropall_flag = 1;
		}

	}

	return 0;
}


#else

int initDNSFilter(void)
{
	struct dns_filter_entry *DNSInfo = NULL;
	int i = 0;
	char nodeName[32] = {0};
	char hostName[256] = {0};
	char Enable[4] = {0};
	char Action[4] = {0};
	char MacAddr[18] = {0};
	void *value = NULL;
	struct dns_filter_entry *match = NULL;
	int dns_filter_action = 0;
	
	DNSFilterMap = (hashmap_map*)hashmap_new();
	if (NULL == DNSFilterMap)
		return -1;
	
	for (i = 0;i < MAX_DNS_FILTER_NUM; i++)
	{
		memset(nodeName, 0, sizeof(nodeName));
		snprintf(nodeName, sizeof(nodeName), "DnsFilter_Common");
		if(0 != tcapi_get(nodeName, "Enable", Enable) || '\0' == Enable[0] || !strcmp(Enable, "0"))
			break;

		memset(nodeName, 0, sizeof(nodeName));
		snprintf(nodeName, sizeof(nodeName), "DnsFilter_Entry%d", i);
		if (0 != tcapi_get(nodeName, "Enable", Enable) || '\0' == Enable[0] || !strcmp(Enable, "0"))
			continue;

		if (0 != tcapi_get(nodeName, "HostName_new", hostName) && '\0' == hostName[0])
			continue;

		DNSInfo = safe_malloc(sizeof(struct dns_filter_entry));
		memset(DNSInfo, 0, sizeof(struct dns_filter_entry));
		DNSInfo->key = safe_malloc(MAX_HOSTNAME_LEN + MAX_MAC_LEN);
		memset(DNSInfo->key, 0, MAX_HOSTNAME_LEN + MAX_MAC_LEN);

		if( NULL == strstr(hostName, "www") )
		{
			snprintf(DNSInfo->hostname, sizeof(DNSInfo->hostname), "www.%s", hostName);
		}
		else
		{
			strncpy(DNSInfo->hostname, hostName, sizeof(DNSInfo->hostname) - 1);
		}
		
		if (0 == tcapi_get(nodeName, "Action", Action) && '\0' != Action[0])
		{
			dns_filter_action = atoi(Action);
			if (dns_filter_action >= 0 && dns_filter_action <= 2)
				DNSInfo->action = dns_filter_action;
		}

		if (0 == tcapi_get(nodeName, "MAC", MacAddr) && '\0' != MacAddr[0])
			strncpy(DNSInfo->Mac, MacAddr, sizeof(DNSInfo->Mac) - 1);
		else
			strncpy(DNSInfo->Mac, "ALL", sizeof(DNSInfo->Mac) - 1);
		

		DNSInfo->entry_idx = i;

		snprintf(DNSInfo->key, MAX_HOSTNAME_LEN + MAX_MAC_LEN, "%s_%s", DNSInfo->hostname, DNSInfo->Mac);
		if (MAP_OMEM == hashmap_put((map_t)DNSFilterMap, DNSInfo->key, (map_t)DNSInfo))
		{
			tcdbg_printf("put into hash map fail!\n");
			free(DNSInfo->key);
			free(DNSInfo);
			return -1;
		}
	}
	return 0;
}


int initURLFilterInfo(void)
{
	struct url_filter_entry *URLInfo = NULL;
	int i = 0;
	char nodeName[32] = {0};
	char url[MAX_HOSTNAME_LEN] = {0};
	char Enable[4] = {0};
	char MacAddr[MAX_MAC_LEN] = {0};
	char *pValue = NULL;
	char tmpBuf[20] = {0};
	char *tmp_key = NULL;
	
	URLFilterMap = (hashmap_map*)hashmap_new();
	if(NULL == URLFilterMap)
		return -1;

	for(i = 0; i < MAX_URL_FILTER_NUM; i++)
	{
		memset(nodeName, 0, sizeof(nodeName));
		snprintf(nodeName, sizeof(nodeName), "UrlFilter_Common");
		if (0 != tcapi_get(nodeName, "Activate", Enable) || '\0' == Enable[0] || !strcmp(Enable, "0"))
			break;

		memset(nodeName, 0, sizeof(nodeName));
		snprintf(nodeName, sizeof(nodeName), "UrlFilter_Entry%d", i);
		if (0 != tcapi_get(nodeName, "Activate", Enable) || '\0' == Enable[0] || !strcmp(Enable, "0"))
			continue;

		if (0 != tcapi_get(nodeName, "URL", url) || '\0' == url[0])
			continue;
		
		URLInfo = safe_malloc(sizeof(struct url_filter_entry));
		memset(URLInfo, 0, sizeof(struct url_filter_entry));
		URLInfo->key = safe_malloc(MAX_HOSTNAME_LEN + MAX_MAC_LEN);
		memset(URLInfo->key, 0, MAX_HOSTNAME_LEN + MAX_MAC_LEN);

		URLInfo->entry_idx = i;

		if (NULL != strstr(url, "https://"))
			strncpy(URLInfo->url, url + strlen("https://"), sizeof(URLInfo->url) - 1);
		else if (NULL != strstr(url, "http://"))
			strncpy(URLInfo->url, url + strlen("http://"), sizeof(URLInfo->url) - 1);
		else
			strncpy(URLInfo->url, url, sizeof(URLInfo->url) - 1);

		tcapi_get(nodeName, "MAC", MacAddr);
		parseMac(MacAddr);
		snprintf(URLInfo->Mac, MAX_MAC_LEN, "%s", MacAddr);
		snprintf(URLInfo->key, MAX_HOSTNAME_LEN, "%s", URLInfo->url);
		if (MAP_OMEM == hashmap_put((map_t)URLFilterMap, URLInfo->key, (map_t)URLInfo))
		{
			free(URLInfo->key);
			free(URLInfo);
			return -1;
		}
	}
	return 0;
}

struct url_filter_entry * doURLMatch(void)
{
	void *value = NULL;
	char tmp_key1[MAX_HOSTNAME_LEN + MAX_MAC_LEN] = {0};
	char tmp_key2[MAX_HOSTNAME_LEN + MAX_MAC_LEN] = {0};
	struct url_filter_entry *match = NULL;
	
	memset(tmp_key1, 0, sizeof(tmp_key1));
	snprintf(tmp_key1, sizeof(tmp_key1), "%s", daemon->namebuff);
	if (MAP_OK == hashmap_get((map_t)URLFilterMap, tmp_key1, &value))
	{
		match = (struct url_filter_entry*)value;
	}

	if(!match)
	{
		if(!strncmp(tmp_key1,"www.",4))
		{
			memset(tmp_key2, 0, sizeof(tmp_key2));
			strncpy(tmp_key2, tmp_key1 + 4, sizeof(tmp_key2) - 1);
			if (MAP_OK == hashmap_get((map_t)URLFilterMap, tmp_key2, &value))
			{
				match = (struct url_filter_entry*)value;
			}
		}
	}
	return match;
}

int DNSFilterMatch(HEADER *header, ssize_t n, union mysockaddr *source_addr, struct all_addr *dst_addr, int if_index, int family, int fd)
{
	unsigned char hw_addr[ETH_ALEN];
	struct dns_filter_entry *match = NULL;
	void *value = NULL;
	char MacAddr[18] = {0};
	char tmp_key[MAX_HOSTNAME_LEN + MAX_MAC_LEN] = {0};
	char nodeName[32] = {0}, tmpHost[256] = {0};
	char blocktimes[20] = {0};
	char lan_ip[20] = {0};
	struct all_addr rep_addr;
	size_t t_plen = 0;

	if( NULL == strstr(daemon->namebuff, "www.") )
	{
		snprintf(tmpHost, sizeof(tmpHost), "www.%s", daemon->namebuff);
	}
	else
	{
		strncpy(tmpHost, daemon->namebuff, sizeof(tmpHost) - 1);
	}
	snprintf(tmp_key, sizeof(tmp_key), "%s_ALL", tmpHost);
	if(MAP_OK == hashmap_get((map_t)DNSFilterMap, tmp_key, &value))
	{
		match = (struct dns_filter_entry*)value;
	}
	else
	{
		getHWAddrByIP(hw_addr, family, source_addr);

		snprintf(MacAddr, sizeof(MacAddr),"%02X%02X%02X%02X%02X%02X"
					, hw_addr[0], hw_addr[1], hw_addr[2]
					, hw_addr[3], hw_addr[4], hw_addr[5]);

		memset(tmp_key, 0, sizeof(tmp_key));
		snprintf(tmp_key, sizeof(tmp_key), "%s_%s", tmpHost, MacAddr);
		if(MAP_OK == hashmap_get((map_t)DNSFilterMap, tmp_key, &value)){
			match = (struct dns_filter_entry*)value;
		}
	}
	
	if(match)
	{
		tcdbg_printf("[%s %d],daemon->namebuff:%s\r\n",__FUNCTION__,__LINE__,daemon->namebuff);
		match->blockedtimes += 1;
		tcdbg_printf("[%s %d]SERVFAIL:%d, NOERROR:%d, NXDOMAIN:%d, REFUSED:%d \r\n",
			__FUNCTION__,__LINE__,SERVFAIL,NOERROR,NXDOMAIN,REFUSED);
		snprintf(nodeName, sizeof(nodeName), "DnsFilter_Entry%d", match->entry_idx);
		snprintf(blocktimes, sizeof(blocktimes), "%d", match->blockedtimes);
		tcapi_set(nodeName, "BlockedTimes", blocktimes);
		if(match->action == 0)
		{
			return 1;
		}
		else if(match->action == 1)
		{
			memset(nodeName, 0, sizeof(nodeName));
			strncpy(nodeName, "Lan_Entry0", sizeof(nodeName) - 1);
			tcapi_get(nodeName, "IP", lan_ip);
			rep_addr.addr.addr4.s_addr = inet_addr(lan_ip);
			t_plen = setup_reply(header, (size_t)n, &rep_addr, F_IPV4, daemon->local_ttl);
			send_from(fd, daemon->options & OPT_NOWILD, (char *)header, t_plen, source_addr, dst_addr, if_index);
			return 1;
		}
		else if(match->action == 2)
		{
			t_plen = setup_reply(header, (size_t)n, NULL, F_NXDOMAIN, daemon->local_ttl);
			send_from(fd, daemon->options & OPT_NOWILD, (char *)header, t_plen, source_addr, dst_addr, if_index);
			return 1;
		}
		else
		{
			tcdbg_printf("wrong dns filter action!\r\n");
		}
	}
	return 0;
}

#endif

#if defined(RA_PARENTALCONTROL)
int initParentalFilterInfo(void)
{
	char nodeName[32] = {0};
	char Enable[4] = {0};
	char hostName[256] = {0};
	char MacAddr[32] = {0};
	char Mode[8] = {0};
	char induration[8] = {0};
	char durationActive[8] = {0};
	char durationPolicy[16] = {0};
	char buf[32] = {0};
	char macAttr[16] = {0};
	char urlAttr[16] = {0};
	int urlNum = 0;
	int macNum= 0;
	int mode = 0;
	int i = 0, m=0, n=0;
	dns_urlfilter_data data = {0};
	
	for (i = 0; i < MAX_PARENTAL_NUM; i++)
	{		
		memset(nodeName, 0, sizeof(nodeName));
		snprintf(nodeName, sizeof(nodeName), "Parental_Entry%d", i);
		if (0 != tcapi_get(nodeName, "Active", Enable) || '\0' == Enable[0] || !strcmp(Enable, "No"))
		{	
			continue;
		}

		if (0 != tcapi_get(nodeName, "macNum", buf) || '\0' == buf[0]  || 0 == (macNum = atoi(buf)) )
		{
			continue;
		}

		memset(buf, 0, sizeof(buf));
		if (0 != tcapi_get(nodeName, "urlNum", buf) || '\0' == buf[0]  || 0 == (urlNum = atoi(buf)) )
		{
			continue;
		}


		memset(durationActive, 0, sizeof(durationActive));
		if (0 == tcapi_get(nodeName, "DurationActive", durationActive)  && !strcmp(durationActive, "Yes"))
		{
			memset(induration, 0, sizeof(induration));
			memset(durationPolicy, 0, sizeof(durationPolicy));
			tcapi_get(nodeName, "induration", induration);
			tcapi_get(nodeName, "DurationPolicy", durationPolicy);

			if((!strcmp(induration, "1") && !strcmp(durationPolicy, "Black")) || (!strcmp(induration, "0") && !strcmp(durationPolicy, "White")))
			{
				/*
				*If parental is in duration and duration policy is Black or parental is not in duration and duration policy is White
				*pareantal is Mac filter actually, CFG_NG will set iptables rule
				*/
				continue;
			}
			else
			{
				/*
				*pareantal is url filter actually, dns will follow the logic of URL filter.
				*/
			}
		}
				
		mode = 0;
		memset(Mode, 0, sizeof(Mode));
		if (0 == tcapi_get(nodeName, "Policy", Mode) && '\0' != Mode[0])
		{
			if(!strcmp(Mode, "White"))
			{
				mode = 1;
			}
			else
			{
				mode = 0;
			}	
		}

		for(m = 0; m < MAX_MAC_PER_ENTRY; m++)
		{
			memset(macAttr, 0, sizeof(macAttr));
			memset(MacAddr, 0, sizeof(MacAddr));
			snprintf(macAttr, sizeof(macAttr), "MAC%d", m);
			if (0 != tcapi_get(nodeName, macAttr, buf) || '\0' == buf[0] )
			{
				continue;
			}
			mac_rm_dot(string_toUper(buf), MacAddr);
			for(n= 0; n < MAX_URL_PER_ENTRY; n++)
			{
				bzero(&data, sizeof(data));
				memset(urlAttr, 0, sizeof(urlAttr));
				memset(hostName, 0, sizeof(hostName));
				snprintf(urlAttr, sizeof(urlAttr), "URL%d", n);
				if (0 != tcapi_get(nodeName, urlAttr, hostName) || '\0' == hostName[0] )
				{
					continue;
				}
				
				if (NULL != strstr(hostName, "https://"))
				{
					strncpy(data.domain, hostName + strlen("https://"), sizeof(hostName) - 1);
				}
				else if (NULL != strstr(hostName, "http://"))
				{
					strncpy(data.domain, hostName + strlen("http://"), sizeof(hostName) - 1);
				}
				else
				{
					strncpy(data.domain, hostName, sizeof(hostName) - 1);
				}
		
				data.cmd_type = E_URL_ADD;
				data.mode = mode;
				data.match_all = 0;
				data.index = i;
				data.times = 0;

				urlfiter_setup(&data, MacAddr, PARENTAL_FILTER_TYPE);
			}
		}		
	}

	return 0;
}

int doParentalMatch(union mysockaddr *source_addr, int family)
{
	unsigned char hw_addr[ETH_ALEN];
	char nodeName[32] = {0};
	char blockTimes[20] = {0};
	int index = 0;
	int ret = 0;
	int mode = 0, ismatchall = 0;
	dns_urlfilter_info blockdata = {0};

	getHWAddrByIP(hw_addr, family, source_addr);

	ret = url_list_match_hash(daemon->namebuff, hw_addr, &index, PARENTAL_FILTER_TYPE, &mode, &ismatchall, NULL);
	
	/*0 drop, 1 continue*/
	if(0 == ret)
	{
		return 1;
	}
	
	return 0;
}

#endif


