#ifndef	__DNS_URL_FILTER_DB_H__
#define	__DNS_URL_FILTER_DB_H__
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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "modules/url_filter_global_def.h"
#include "hashmap.h"

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
************************************************************************/
#define MUTEX_INIT(x) \
		x = g_mutex_new(); \
		if ( NULL == x ) \
		{ \
			tcdbg_printf("%s "#x" init failed.\n", __FUNCTION__); \
			return -1; \
		}
	
#define MUTEX_FREE(x) \
		if ( x ) \
			g_mutex_free(x)

/************************************************************************
*                  M A C R O S
************************************************************************/

/************************************************************************
*                  D A T A   T Y P E S
************************************************************************/
enum Filter_Type_e
{
	DNS_FILTER_TYPE=1,
	URL_FILTER_TYPE=2,
	PARENTAL_FILTER_TYPE=3,
	MAX_FILTER_TYPE
};


typedef struct _mac_filter_node_info_
{
	map_t mac_url_map;
	char *mac;
	int mode; /* 0:black, 1:white. */
	int match_all;
	int index;
	int action;
}mac_filter_node_info, *pt_mac_filter_node_info;

typedef struct _domain_filter_node_info_
{
	char *domain;
	int index;
}domain_filter_node_info, *pt_domain_filter_node_info;

typedef struct _global_domain_filter_node_info_
{
	map_t url_map;
	map_t blocked_map;		/* record blocked times */
	map_t iptables_map;		/* record iptable rule inserted */
	int mode; /* 0:black, 1:white. */
	int match_all;
	int index;				/* only for match all type .*/
	int action;
}global_domain_filter_node_info, *pt_global_domain_filter_node_info;

typedef struct _global_mac_filter_node_info_
{
	map_t mac_map;
}global_mac_filter_node_info, *pt_global_mac_filter_node_info;

typedef struct _blocked_times_node_info_
{
	unsigned long times;
}blocked_times_node_info, *pt_blocked_times_node_info;

typedef struct _iptables_node_info_
{
	int inserted_flag;
}iptables_node_info, *pt_iptables_node_info;


/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
************************************************************************/
extern global_domain_filter_node_info gDomainDB;
extern global_mac_filter_node_info gMACDB;

extern global_domain_filter_node_info gDomainDB_url;
extern global_mac_filter_node_info gMACDB_url;

extern global_domain_filter_node_info gDomainDB_parental;
extern global_mac_filter_node_info gMACDB_parental;

//extern GMutex *gObjFilterMutex;


/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
************************************************************************/
int UrlFilterDB_init(void);
void UrlFilterDB_exit(void);
int ParentalFilterDB_init(void);
void ParentalFilterDB_exit(void);

int get_db_length(void);
int blocked_dbmap_count_plus(map_t dbmap, int entryidx);
int urlfiter_get_blocked_times(dns_urlfilter_info *data, int type);

/************************************************************************
*                  P U B L I C   D A T A
************************************************************************/



#endif
