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
#include "http-get-utils.h"
#include "boa.h"
#include "cmcc_itms_api.h"


/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************/
#define ZERO(x) bzero(x, sizeof(x))
#define AUTH_RESULT \
"{\n\
\"Result\":\"%s\"\n\
}"


/************************************************************************
*                  M A C R O S
*************************************************************************/
#define MAX_PASSWORD_LEN 12

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
int doValPut(char *path, char *value)
{
	FILE *fp = NULL;

	fp = fopen(path,"w");
	if (fp )
	{
		fprintf(fp, value);
		fclose(fp);
	}
	else
	{
		return -1;
	}

	return 0;
}
static int upgrde_url_for_region(char *region)
{
	if(region == NULL)
		return 0;

	tcapi_set("if6regioninfo_Common", "area", region);
	tcapi_commit("if6regioninfo_Common");
	tcapi_save();
	return 0;
}
/*
	CMCC ZERO Configuration
*/
int cmcc_itms_auth
(asp_reent* reent, const asp_text* params, char *p_action, char *p_area, char **result)
{
	char *pval_chk_name = NULL, *pval_chk_pwd = NULL;
	char *pval_PPPOEuser = NULL, *pval_PPPOEpassword = NULL;
	char *p_result = NULL;
	char buf_val[64] = {0}, wanStatus[32] = {0}, pppConnStatus[64] = {0};
	char if6Status[32] = {0}, curAPMode[32] = {0}, time_buf[32] = {0};
	char active[12] = {0}, web_username[256] = {0}, web_passwd[256] = {0};
	int r_start_time = 0, i_if6_status = 0;
	struct timespec req_time_now;
	char *pval_region = NULL;

	ZERO(buf_val);
	if ( tcapi_get("Globalstate_Common", "zeroStartTime", buf_val) < 0 )
		strcpy(buf_val, "0");
	r_start_time = atoi(buf_val);
	/* GET current time. */
	bzero(&req_time_now, sizeof(req_time_now));
	clock_gettime(CLOCK_MONOTONIC, &req_time_now);

	cmcc_dbg_info(1);
	/* check normal username & password. */
	tcapi_get("Account_Entry1", "Active", active);
	tcapi_get("Account_Entry1", "username", web_username);
	tcapi_get("Account_Entry1", "web_passwd", web_passwd);

	pval_chk_name = get_param(g_var_post, "username");
	pval_chk_pwd = get_param(g_var_post, "userpwd");
	pval_PPPOEuser = get_param(g_var_post, "PPPOEuser");
	pval_PPPOEpassword = get_param(g_var_post, "PPPOEpassword");
	pval_region = get_param(g_var_post, "area");

	if ( 0 != strcmp(active, "Yes")
		|| NULL == pval_chk_name || NULL == pval_chk_pwd
		|| 0 != strcmp(pval_chk_name, web_username)
		|| 0 != strcmp(pval_chk_pwd, web_passwd) )
	{
		cmcc_dbg_info(2);
		p_result = "Auth Failed";
		goto JSON_CREATE;
	}

	/* 1. Check action. */
	if ( p_action 
		&& 0 == strcmp(p_action, "GetRegStat") )
	{
		if ( req_time_now.tv_sec - r_start_time <= 2 )
		{
			cmcc_dbg_info(4);
			p_result = "Setting Default Configuration";
			goto JSON_CREATE;
		}

		/* Check Wan Status. */
		tcapi_get("WanInfo_Entry0", "Status", wanStatus);
		if ( 0 != strcmp(wanStatus, "up") )
		{
			tcapi_get("Wan_PVC0_Entry0", "ConnectionError", pppConnStatus);
			if ( 0 == strcmp(pppConnStatus, "ERROR_AUTHENTICATION_FAILURE") )
			{
				cmcc_dbg_info(5);
				p_result = "PPPOE Failed";
				tcapi_set("Globalstate_Common", "zeroStartTime", "0");
				goto JSON_CREATE;
			}
			else
			{
				if ( req_time_now.tv_sec - r_start_time <= 20 )
				{
					cmcc_dbg_info(6);
					p_result = "Getting WAN Address";
					goto JSON_CREATE;
				}
				else
				{
					cmcc_dbg_info(7);
					p_result = "Getting Address Failed";
					tcapi_set("Globalstate_Common", "zeroStartTime", "0");
					goto JSON_CREATE;
				}
			}
		}
		else
		{
			cmcc_dbg_info(8);
			/* Check IF6 status. */
			if ( 0 != tcapi_get("alinkmgr_Entry", "IF6Status", if6Status) )
				strcpy(if6Status, "-10");

			i_if6_status = atoi(if6Status);
			switch ( i_if6_status )
			{
				case -1:
					p_result = "Intelligent Platform Normal Error";
					tcapi_set("Globalstate_Common", "zeroStartTime", "0");
					goto JSON_CREATE;
				case -2:
					p_result = "Intelligent Platform Auth Failed";
					tcapi_set("Globalstate_Common", "zeroStartTime", "0");
					goto JSON_CREATE;
				case -3:
					p_result = "Re-Registering Intelligent Platform";
					goto JSON_CREATE;
				case -5:
					p_result = "Intelligent Platform Register to Another Server";
					goto JSON_CREATE;
				case 0:
					p_result = "Reg And Service Succ";
					tcapi_set("Globalstate_Common", "zeroStartTime", "0");
					goto JSON_CREATE;
				case 2:
					p_result = "Intelligent Platform Auth Succ";
					tcapi_set("Globalstate_Common", "zeroStartTime", "0");
					goto JSON_CREATE;
				case -10:
				default:
					p_result = "Registering Intelligent Platform";
					goto JSON_CREATE;
			}
		}

	}
	/* 2. Check GPON password. */
	else if ( pval_PPPOEuser 
		&& 0 != pval_PPPOEuser[0]
		&& pval_PPPOEpassword
		&& 0 != pval_PPPOEpassword[0] )
	{
		/* Check bridge mode */
		tcapi_get("WanInfo_Common", "CurAPMode", curAPMode);
		if ( 0 != strcmp("Route", curAPMode) )
		{
			cmcc_dbg_info(9);
			p_result = "Bridge Mode Failed";
			tcapi_set("Globalstate_Common", "zeroStartTime", "0");
			goto JSON_CREATE;
		}

		if ( 0 != r_start_time ) /* registration is already started. */
		{
			/* check timeout */
			if ( req_time_now.tv_sec - r_start_time < 40 )
			{
				cmcc_dbg_info(10);
				p_result = "Register is running";
				goto JSON_CREATE;
			}
		}
	}
	else /* error case */
	{
		cmcc_dbg_info(11);
		p_result = "Param Wrong";
		goto JSON_CREATE;
	}
	if(pval_region != NULL)
	{
		upgrde_url_for_region(pval_region);
	}
	/* do register now. */
	tcapi_set("Wan_PVC0_Entry0", "LinkMode", "linkPPP");
	tcapi_set("Wan_PVC0_Entry0", "MTU", "1492");
	tcapi_set("Wan_PVC0_Entry0", "PPPGETIP", "Dynamic");
	tcapi_set("Wan_PVC0_Entry0", "USERNAME", pval_PPPOEuser);
	tcapi_set("Wan_PVC0_Entry0", "PASSWORD", pval_PPPOEpassword);
	tcapi_set("Wan_PVC0_Entry0", "PPPManualStatus", "disconnect");
	tcapi_set("Wan_PVC0_Entry0", "ISP", "2");

	snprintf(time_buf, sizeof(time_buf), "%d", req_time_now.tv_sec);
	tcapi_set("Globalstate_Common", "zeroStartTime", time_buf);
	tcapi_save();
	tcapi_commit("Wan_PVC0_Entry0");

	p_result = "Default Configuration Succ";
	cmcc_dbg_info(12);

JSON_CREATE:
	asprintf(result, AUTH_RESULT, p_result);

	return 0;
}

int cmcc_dbg_info(int step)
{
	char buf[128] = {0};

	ZERO(buf);
	snprintf(buf, sizeof(buf), "%d", step);
	doValPut("/tmp/cmcc_zero_np_step", buf);

	return 0;
}


