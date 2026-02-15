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
#include "dns_url_filter_hash_if.h"
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

/************************************************************************
*                  P R I V A T E   D A T A
*************************************************************************/


/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
*************************************************************************/
int domain_split(char *domain, char buf[][64], int buf_level, int *domain_depth, const char *delim)
{
	char *token = NULL, *pStr = NULL;
	int idx = 0;

	if ( !domain || !delim || !domain_depth )
		return -1;

	*domain_depth = 0;
	pStr = domain;
	token = strsep(&pStr, delim);
	strncpy(buf[0], token, 63);
	(*domain_depth) ++;

	for ( idx = 1; idx < buf_level; idx ++ )
	{
		token = strsep(&pStr, delim);
		if ( token )
		{
			strncpy(buf[idx], token, 63);
			(*domain_depth) ++;
		}
		else
			break;
	}

	return 0;
}

/*
0: not MATCH
1: MATCH
*/
int domain_hash_check(map_t dbmap, char *domain, int *entry_idx)
{
	char domain_url[256] = {0}, bak_domain_url[256] = {0};
	char check_path[256] = {0}, domain_path[4][64] = {0};
	int domain_depth = 0;
	char *pos1 = NULL;
	domain_filter_node_info *p_data = NULL;

	if ( !domain || !entry_idx )
		return 0;

	memset(domain_url, 0, sizeof(domain_url));
	snprintf(domain_url, sizeof(domain_url), "%s", domain);

	/* 2. find url */
	/* step 1:	whole path. */
	snprintf(check_path, sizeof(check_path), "%s", domain_url);

	if ( MAP_OK == hashmap_get(dbmap, check_path, (any_t*)&p_data) )
	{
		*entry_idx = p_data->index;
		return 1;
	}

	strcpy(bak_domain_url, domain_url);
	domain_split(bak_domain_url, domain_path, 4, &domain_depth, ".");

	/* step 2:	check xxx.com type */
	if ( 2 == domain_depth )
	{
		/* search as xxx */
		snprintf(check_path, sizeof(check_path), "%s", domain_path[0]);
		if ( MAP_OK == hashmap_get(dbmap, check_path, (any_t*)&p_data) )
		{
			*entry_idx = p_data->index;
			return 1;
		}
	}
	/* step 3:	check www.xxx.com or m.xxx.com or www.yyy.com.cn or news.sina.com.cn */
	else if ( domain_depth >= 3 )
	{
		if ( 'w' == domain_url[0]
			&& 'w' == domain_url[1]
			&& 'w' == domain_url[2]
			&& '.' == domain_url[3] )
		{
			snprintf(check_path, sizeof(check_path), "%s", &domain_url[4]);
			if ( MAP_OK == hashmap_get(dbmap, check_path, (any_t*)&p_data) )
			{
				*entry_idx = p_data->index;
				return 1;
			}

			/* check yyy */
			snprintf(check_path, sizeof(check_path), "%s", domain_path[1]);
			if ( MAP_OK == hashmap_get(dbmap, check_path, (any_t*)&p_data) )
			{
				*entry_idx = p_data->index;
				return 1;
			}

			/* check yyy.com */
			snprintf(check_path, sizeof(check_path), "%s.%s", domain_path[1], domain_path[2]);
			if ( MAP_OK == hashmap_get(dbmap, check_path, (any_t*)&p_data) )
			{
				*entry_idx = p_data->index;
				return 1;
			}

			if ( domain_depth >= 4 )
			{
				/* check yyy.com.cn */
				snprintf(check_path, sizeof(check_path), "%s.%s.%s", domain_path[1], domain_path[2], domain_path[3]);
				if ( MAP_OK == hashmap_get(dbmap, check_path, (any_t*)&p_data) )
				{
					*entry_idx = p_data->index;
					return 1;
				}
			}
		}
		else if ( 'm' == domain_url[0]
			&& '.' == domain_url[1] )
		{
			snprintf(check_path, sizeof(check_path), "%s", &domain_url[2]);
			if ( MAP_OK == hashmap_get(dbmap, check_path, (any_t*)&p_data) )
			{
				*entry_idx = p_data->index;
				return 1;
			}

			/* check yyy */
			snprintf(check_path, sizeof(check_path), "%s", domain_path[1]);
			if ( MAP_OK == hashmap_get(dbmap, check_path, (any_t*)&p_data) )
			{
				*entry_idx = p_data->index;
				return 1;
			}
			
			/* check yyy.com */
			snprintf(check_path, sizeof(check_path), "%s.%s", domain_path[1], domain_path[2]);
			if ( MAP_OK == hashmap_get(dbmap, check_path, (any_t*)&p_data) )
			{
				*entry_idx = p_data->index;
				return 1;
			}

			if ( domain_depth >= 4 )
			{
				/* check yyy.com.cn */
				snprintf(check_path, sizeof(check_path), "%s.%s.%s", domain_path[1], domain_path[2], domain_path[3]);
				if ( MAP_OK == hashmap_get(dbmap, check_path, (any_t*)&p_data) )
				{
					*entry_idx = p_data->index;
					return 1;
				}
			}
		}
		else
		{
			/* ex: news.sina.com.cn / sina.com.cn */
			/* check news or sina */
			snprintf(check_path, sizeof(check_path), "%s", domain_path[0]);
			if ( MAP_OK == hashmap_get(dbmap, check_path, (any_t*)&p_data) )
			{
				*entry_idx = p_data->index;
				return 1;
			}

			/* check news.sina or sina.com */
			snprintf(check_path, sizeof(check_path), "%s.%s", domain_path[0], domain_path[1]);
			if ( MAP_OK == hashmap_get(dbmap, check_path, (any_t*)&p_data) )
			{
				*entry_idx = p_data->index;
				return 1;
			}

			/* check sina.com or com.cn */
			snprintf(check_path, sizeof(check_path), "%s.%s", domain_path[1], domain_path[2]);
			if ( MAP_OK == hashmap_get(dbmap, check_path, (any_t*)&p_data) )
			{
				*entry_idx = p_data->index;
				return 1;
			}

			if ( domain_depth >= 4 )
			{
				/* check sina.com.cn */
				snprintf(check_path, sizeof(check_path), "%s.%s.%s", domain_path[1], domain_path[2], domain_path[3]);
				if ( MAP_OK == hashmap_get(dbmap, check_path, (any_t*)&p_data) )
				{
					*entry_idx = p_data->index;
					return 1;
				}
			}
		}	
	}


	return 0;
}

/*
0: DROP PACKET
1: CONTINUE
*/
int url_list_match_hash(char *domain, unsigned char *mac, int *index, int type, int *mode, int *ismatchall, int *action)
{
	char mac_addr[20] = {0};
	mac_filter_node_info *p_mac_data = NULL;
	domain_filter_node_info *p_domain_data = NULL;
	int ret_code = PKT_CONTINUE;
	char host[256] = {0};
	global_domain_filter_node_info *gDomainDB_p = NULL;
	global_mac_filter_node_info *gMACDB_p = NULL;

	if ( !domain || !index || !mac || !mode || !ismatchall)
		return PKT_CONTINUE;

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

	snprintf(mac_addr, sizeof(mac_addr), "%02X%02X%02X%02X%02X%02X",
					mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	memset(host, 0, sizeof(host));
	snprintf(host, sizeof(host), "%s", domain);
	/* check MAC entry */
	if ( MAP_OK == hashmap_get(gMACDB_p->mac_map, mac_addr, (any_t*)&p_mac_data) )
	{
		if ( 0 == p_mac_data->mode ) /* black */
		{
			if ( 1 == p_mac_data->match_all)
			{
				*ismatchall = 1;
				ret_code = PKT_DROP;;
				blocked_dbmap_count_plus(gDomainDB_p->blocked_map, p_mac_data->index);
				goto unlock_end_match;
			}
			if ( 1 == domain_hash_check(p_mac_data->mac_url_map, host, index) )
			{
				ret_code = PKT_DROP;
				blocked_dbmap_count_plus(gDomainDB_p->blocked_map, p_mac_data->index);
				goto unlock_end_match;
			}

		}
		else if ( 1 == p_mac_data->mode ) /* white */ 
		{
			*mode = 1;
			if ( 1 == p_mac_data->match_all )
			{
				*ismatchall = 1;
				ret_code = PKT_CONTINUE;
				goto unlock_end_match;
			}
		
			if ( 1 == domain_hash_check(p_mac_data->mac_url_map, host, index) )
			{
				ret_code = PKT_CONTINUE;
				goto unlock_end_match;
			}

			ret_code = PKT_DROP;
			blocked_dbmap_count_plus(gDomainDB_p->blocked_map, p_mac_data->index);
			goto unlock_end_match;
		}
	}
	/* check Global entry */
	if ( 0 == gDomainDB_p->mode ) /* black */
	{
		if ( 1 == gDomainDB_p->match_all )
		{
			*ismatchall = 1;
			ret_code = PKT_DROP;
			blocked_dbmap_count_plus(gDomainDB_p->blocked_map, gDomainDB_p->index);
			goto unlock_end_match;
		}

		if ( 1 == domain_hash_check(gDomainDB_p->url_map, host, index) )
		{
			ret_code = PKT_DROP;		
			blocked_dbmap_count_plus(gDomainDB_p->blocked_map, *index);
			goto unlock_end_match;
		}
	}
	else if ( 1 == gDomainDB_p->mode ) /* white */ 
	{
		*mode = 1;
		if ( 1 == gDomainDB_p->match_all )
		{
			*ismatchall = 1;
			ret_code = PKT_CONTINUE;
			goto unlock_end_match;
		}

		if ( 1 == domain_hash_check(gDomainDB_p->url_map, host, index) )
		{
			ret_code = PKT_CONTINUE;
			goto unlock_end_match;
		}
		
		blocked_dbmap_count_plus(gDomainDB_p->blocked_map, gDomainDB_p->index);
		ret_code = PKT_DROP;
		
		goto unlock_end_match;
	}

	return PKT_CONTINUE;

unlock_end_match:
	
	if(action)
	{
		if(p_mac_data)
			*action = p_mac_data->action;
		else
			*action = gDomainDB_p->action;
	}
	
	return ret_code;
}

