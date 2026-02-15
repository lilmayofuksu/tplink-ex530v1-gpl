/***************************************************************
Copyright Statement:

This software/firmware and related documentation (EcoNet Software) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (EcoNet) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (ECONET SOFTWARE) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN AS IS 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVERS SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVERS SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/

/************************************************************************
*                  I N C L U D E S
*************************************************************************/
#include "dns_url_filter_db.h"
#include "hashmap.h"

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
************************************************************************/

/************************************************************************
*                  M A C R O S
************************************************************************/

/************************************************************************
*                  D A T A   T Y P E S
************************************************************************/

/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
************************************************************************/

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
************************************************************************/


/************************************************************************
*                  P U B L I C   D A T A
************************************************************************/
global_domain_filter_node_info gDomainDB;
global_mac_filter_node_info gMACDB;

global_domain_filter_node_info gDomainDB_url;
global_mac_filter_node_info gMACDB_url;


global_domain_filter_node_info gDomainDB_parental;
global_mac_filter_node_info gMACDB_parental;

/************************************************************************
*                  P R I V A T E   D A T A
*************************************************************************/

/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
*************************************************************************/
int check_iptables_hash(char *domain, char *MacAddr, int ismatchall)
{
	char key_val[128 + 18] = {0}, *p_new_key = NULL;;
	iptables_node_info *p_iptables_data = NULL;

	if ( !domain || !MacAddr )
		return -3;

	if(ismatchall)
		snprintf(key_val, sizeof(key_val),"_%s", MacAddr);
	else
		snprintf(key_val, sizeof(key_val),"%s_%s", domain, MacAddr);
	if ( MAP_OK == hashmap_get(gDomainDB_url.iptables_map, key_val, (any_t*)&p_iptables_data) )
	{
		/* found */
		return 1;
	}
	else
	{
		/* create new entry */
		p_iptables_data = malloc(sizeof(iptables_node_info));
		if ( !p_iptables_data )
			return -1;

		memset(p_iptables_data, 0, sizeof(iptables_node_info));
		p_iptables_data->inserted_flag = 1;

		asprintf(&p_new_key, "%s", key_val);
		if ( !p_new_key )
		{
			free(p_iptables_data);
			return -2;
		}
		
		hashmap_put(gDomainDB_url.iptables_map, p_new_key, p_iptables_data);
	}

	return 0;
}


int init_blocked_dbmap_count(map_t dbmap, int entryidx, int times)
{
	blocked_times_node_info *p_blocked_data = NULL;
	char entry_buf[24] = {0}, *p_new_idx = NULL;

	snprintf(entry_buf, sizeof(entry_buf), "%d", entryidx);
	if ( MAP_OK == hashmap_get(dbmap, entry_buf, (any_t*)&p_blocked_data) )
	{
		/* found and increase it. */
		p_blocked_data->times = times;
	}
	else
	{
		/* create new entry */
		p_blocked_data = malloc(sizeof(blocked_times_node_info));
		if ( !p_blocked_data )
			return -1;

		memset(p_blocked_data, 0, sizeof(blocked_times_node_info));
		p_blocked_data->times = times;
		asprintf(&p_new_idx, "%d", entryidx);
		if ( !p_new_idx )
		{
			free(p_blocked_data);
			return -2;
		}
		hashmap_put(dbmap, p_new_idx, p_blocked_data);
	}
	return 0;
}

int blocked_dbmap_count_plus(map_t dbmap, int entryidx)
{
	blocked_times_node_info *p_blocked_data = NULL;
	char entry_buf[24] = {0}, *p_new_idx = NULL;

	snprintf(entry_buf, sizeof(entry_buf), "%d", entryidx);
	if ( MAP_OK == hashmap_get(dbmap, entry_buf, (any_t*)&p_blocked_data) )
	{
		/* found and increase it. */
		p_blocked_data->times ++;
	}
	else
	{
		/* create new entry */
		p_blocked_data = malloc(sizeof(blocked_times_node_info));
		if ( !p_blocked_data )
			return -1;

		memset(p_blocked_data, 0, sizeof(blocked_times_node_info));
		asprintf(&p_new_idx, "%d", entryidx);
		if ( !p_new_idx )
		{
			free(p_blocked_data);
			return -2;
		}

		hashmap_put(dbmap, p_new_idx, p_blocked_data);
	}

	return 0;
}

int urlfiter_get_blocked_times(dns_urlfilter_info *data, int type)
{
	blocked_times_node_info *p_blocked_data = NULL;
	char entry_buf[24] = {0}, *p_new_idx = NULL;
	global_domain_filter_node_info *gDomainDB_p = NULL;

	if ( !data )
		return -1;

	if(type == DNS_FILTER_TYPE)
	{
		gDomainDB_p = &gDomainDB;
	}
	else if(type == URL_FILTER_TYPE)

	{
		gDomainDB_p = &gDomainDB_url;
	}
	else
	{
		return -1;
	}

	snprintf(entry_buf, sizeof(entry_buf), "%d", data->index);
	if ( MAP_OK == hashmap_get(gDomainDB_p->blocked_map, entry_buf, (any_t*)&p_blocked_data) )
	{
		data->blocked_times = p_blocked_data->times;
	}

	return 0;
}

int get_db_length(void)
{
	return hashmap_length(gMACDB.mac_map) + hashmap_length(gDomainDB.url_map);
}

int count_mac_hashdb(int *p_count, mac_filter_node_info *data)
{
	if ( NULL == data )
		return -1;

	hashmap_free(data->mac_url_map);
	return MAP_OK;
}

int clear_mac_hashdb(map_t dbmap)
{
	int tmp_idx = 0;

	if ( MAP_OK == hashmap_iterate(dbmap
					, count_mac_hashdb, &tmp_idx))
		return 0;

	return -1;
}

int urlfiter_setup(dns_urlfilter_data *data, char *mac_addr, int type)
{
	int entry_idx = 0;
	char key_val[256] = {0};
	domain_filter_node_info *node_domain = NULL, *p_domain_data = NULL;
	mac_filter_node_info *node_mac = NULL, *p_mac_data = NULL;
	global_domain_filter_node_info *gDomainDB_p = NULL;
	global_mac_filter_node_info *gMACDB_p = NULL;


	if (NULL == mac_addr
		|| NULL == data
		|| data->cmd_type < E_URL_ADD
		|| data->cmd_type > E_URL_CMD_MAX )
		return -1;

	if(type == DNS_FILTER_TYPE)
	{
		gDomainDB_p = &gDomainDB;
		gMACDB_p = &gMACDB;
	}
	else if(type == URL_FILTER_TYPE)
	{
		gDomainDB_p = &gDomainDB_url;
		gMACDB_p = &gMACDB_url;
	}
	else if(type == PARENTAL_FILTER_TYPE)
	{
		gDomainDB_p = &gDomainDB_parental;
		gMACDB_p = &gMACDB_parental;
	}
	else
	{
		return -1;
	}


	if ( E_URL_ADD == data->cmd_type )
	{
		/* 1. check has MAC ? */
		if(0 == mac_addr[0])
		{
			/* global setting. */
			gDomainDB_p->mode = data->mode;
			gDomainDB_p->action = data->action;
			gDomainDB_p->match_all = data->match_all;
			if ( gDomainDB_p->match_all )
				gDomainDB_p->index = data->index;

			snprintf(key_val, sizeof(key_val), "%s", data->domain);
			if ( MAP_OK != hashmap_get(gDomainDB_p->url_map, key_val, (any_t*)&p_domain_data) )
			{
				node_domain = malloc(sizeof(domain_filter_node_info));
				if ( !node_domain )
					return -2;
			
				asprintf(&node_domain->domain, "%s", data->domain);
				if ( node_domain->domain )
				{
					node_domain->index = data->index;
					/* add it */
					hashmap_put(gDomainDB_p->url_map, node_domain->domain, node_domain);
				}
				else
					free(node_domain);
			}
		}
		else
		{
			/* 2. MAC hash map. */
			if ( MAP_OK != hashmap_get(gMACDB_p->mac_map, mac_addr, (any_t*)&p_mac_data) )
			{
				/* add MAC hash first. */
				node_mac = malloc(sizeof(mac_filter_node_info));
				if ( !node_mac )
					return -3;

				asprintf(&node_mac->mac, "%s", mac_addr);
				if ( !node_mac->mac )
				{
					free(node_mac);
					return -4;
				}
				node_mac->mac_url_map = hashmap_new();
				/* update mode */
				node_mac->mode = data->mode;
				node_mac->action = data->action;
				node_mac->index = data->index;
				node_mac->match_all = data->match_all;
				/* add it */
				hashmap_put(gMACDB_p->mac_map, node_mac->mac, node_mac);

				/* add domain hash */
				node_domain = malloc(sizeof(domain_filter_node_info));
				if ( !node_domain )
				{
					free(node_mac->mac);
					free(node_mac);
					return -5;
				}
				asprintf(&node_domain->domain, "%s", data->domain);
				if ( node_domain->domain )
				{
					node_domain->index = data->index;
					/* add it */
					hashmap_put(node_mac->mac_url_map, node_domain->domain, node_domain);
				}
				else
				{
					free(node_mac->mac);
					free(node_mac);

					free(node_domain);
				}
			}
			else
			{
				/* update mode */
				p_mac_data->mode = data->mode;
				p_mac_data->action = data->action;
				p_mac_data->index = data->index;
				p_mac_data->match_all = data->match_all;
				/* check domain */
				snprintf(key_val, sizeof(key_val), "%s", data->domain);
				if ( MAP_OK != hashmap_get(p_mac_data->mac_url_map, key_val, (any_t*)&p_domain_data) )
				{
					node_domain = malloc(sizeof(domain_filter_node_info));
					if ( !node_domain )
						return -6;

					asprintf(&node_domain->domain, "%s", data->domain);
					if ( node_domain->domain )
					{
						node_domain->index = data->index;
						/* add it */
						hashmap_put(p_mac_data->mac_url_map, node_domain->domain, node_domain);
					}
					else
						free(node_domain);
				}
			}

		}
		init_blocked_dbmap_count(gDomainDB_p->blocked_map, data->index, data->times);
	}
	else if ( E_URL_REM_ALL == data->cmd_type )
	{
		/* clear */
		hashmap_free(gDomainDB_p->url_map);
		hashmap_free(gDomainDB_p->blocked_map);
		hashmap_free(gDomainDB_p->iptables_map);
		
		clear_mac_hashdb(gMACDB_p->mac_map);
		hashmap_free(gMACDB_p->mac_map);

		/* reinit */
		memset(gDomainDB_p, 0, sizeof(global_domain_filter_node_info));
		memset(gMACDB_p, 0, sizeof(global_mac_filter_node_info));

		gDomainDB_p->url_map = hashmap_new();
		gDomainDB_p->blocked_map = hashmap_new();
		gDomainDB_p->iptables_map = hashmap_new();
		gMACDB_p->mac_map = hashmap_new();
	}

	return 0;
}

int UrlFilterDB_init(void)
{
	memset(&gDomainDB, 0, sizeof(gDomainDB));
	memset(&gMACDB, 0, sizeof(gMACDB));

	memset(&gDomainDB_url, 0, sizeof(gDomainDB_url));
	memset(&gMACDB_url, 0, sizeof(gMACDB_url));

	gDomainDB.url_map = hashmap_new();
	gDomainDB.blocked_map = hashmap_new();
	gDomainDB.iptables_map = hashmap_new();
	gMACDB.mac_map = hashmap_new();

	gDomainDB_url.url_map = hashmap_new();
	gDomainDB_url.blocked_map = hashmap_new();
	gDomainDB_url.iptables_map = hashmap_new();
	gMACDB_url.mac_map = hashmap_new();

	return 0;
}

void UrlFilterDB_exit(void)
{
	hashmap_free(gDomainDB.url_map);

	clear_mac_hashdb(gMACDB.mac_map);
	hashmap_free(gMACDB.mac_map);

	hashmap_free(gDomainDB_url.url_map);

	clear_mac_hashdb(gMACDB_url.mac_map);
	hashmap_free(gMACDB_url.mac_map);
}

int ParentalFilterDB_init(void)
{
	memset(&gDomainDB_parental, 0, sizeof(gDomainDB_parental));
	memset(&gMACDB_parental, 0, sizeof(gMACDB_parental));

	gDomainDB_parental.url_map = hashmap_new();
	gDomainDB_parental.blocked_map = hashmap_new();
	gDomainDB_parental.iptables_map = hashmap_new();
	gMACDB_parental.mac_map = hashmap_new();

	return 0;
}

void ParentalFilterDB_exit(void)
{
	hashmap_free(gDomainDB_parental.url_map);

	clear_mac_hashdb(gMACDB_parental.mac_map);
	hashmap_free(gMACDB_parental.mac_map);
}
