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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#ifdef TCSUPPORT_SYSLOG
#include <syslog.h>
#endif
#include "boa.h"
#include "http-get-utils.h"
#include "ecnt_json_api.h"


/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************/
#define ZERO(x) bzero(x, sizeof(x))


/************************************************************************
*                  M A C R O S
*************************************************************************/

/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************/


/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************/


/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
************************************************************************/

/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************/

/************************************************************************
*                  P R I V A T E   D A T A
*************************************************************************/


/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
*************************************************************************/
int ecnt_json_get_portal_info(asp_reent* reent, const asp_text* params, char *p_val1, char *p_val2, void *au_param, char **result)
{
	char *p_val3 = NULL, *p_val4 = NULL;
	char cmdbuf[256] = {0};

	p_val3 = get_param(g_var_post, "lanip");
	if ( !p_val1 || !p_val2 || !p_val3 )
	{
		asp_send_format_response("{}"); /* not valid redirect.*/
		return 0;
	}

	p_val4 = get_param(g_var_post, "chkui");
	if ( p_val4 && 0 == strcmp(p_val4, "OK") )
	{
		ZERO(cmdbuf);
		snprintf(cmdbuf, sizeof(cmdbuf), "iptables -t nat -D PRE_PORTAL -s %s -j RETURN", p_val1);	
		system(cmdbuf);
		
		ZERO(cmdbuf);
		snprintf(cmdbuf, sizeof(cmdbuf), "iptables -t nat -I PRE_PORTAL 1 -s %s -j RETURN", p_val1);	
		system(cmdbuf);

		asp_send_format_response("{\"check\":\"done\", \"lanip\":\"%s\", \"pc\":\"%s\", \"host\":\"%s\"}", p_val3, p_val1, p_val2);
	}
	else
	{
		asp_send_format_response("{\"check\":\"checking\", \"lanip\":\"%s\", \"pc\":\"%s\", \"host\":\"%s\"}", p_val3, p_val1, p_val2);

	}

	return 0;
}

int ecnt_json_get_wandata_info(asp_reent* reent, const asp_text* params, char *p_val1, char *p_val2, void *au_param, char **result)
{
	int pvc_idx = 0, entry_idx = 0, cnt = 0;
	char node_name[32] = {0}, vlan_mode[32] = {0}, vlan_id[32] = {0}, pbit[32] = {0};

	asp_send_format_response("{");
	for ( pvc_idx = 0; pvc_idx < WAN_PVC_CNT; pvc_idx ++ )
	{
		for ( entry_idx = 0; entry_idx < WAN_ENTRY_CNT; entry_idx ++ )
		{
			ZERO(node_name);
			ZERO(vlan_mode);
			ZERO(vlan_id);
			snprintf(node_name, sizeof(node_name), "Wan_PVC%d_Entry%d", pvc_idx, entry_idx);
			if ( 0 != tcapi_get(node_name, "VLANMode", vlan_mode) )
				continue;

			if ( 0 == strcmp(vlan_mode, "TAG") )
			{
				tcapi_get(node_name, "VLANID", vlan_id);
				tcapi_get(node_name, "dot1pData", pbit);
			}

			asp_send_format_response("%s \"%d\" : {\"VLANMode\":\"%s\", \"VLANID\":\"%s\", \"DOT1P\":\"%s\"}"
					, (0 == cnt ? "" : ",")
					, pvc_idx * WAN_ENTRY_CNT + entry_idx
					, vlan_mode, vlan_id, pbit);
			cnt ++;
		}
	}
	asp_send_format_response("}");


	return 0;
}




