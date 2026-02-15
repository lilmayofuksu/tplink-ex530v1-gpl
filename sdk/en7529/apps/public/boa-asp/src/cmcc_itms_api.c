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

/*
	CMCC ZERO Configuration
*/
int cmcc_itms_auth
(asp_reent* reent, const asp_text* params, char *p_action, char *p_area, char **result)
{
	char *pval_chk_name = NULL, *pval_chk_pwd = NULL;
	char *pval_gpon_password = NULL, *pval_gpon_sn = NULL;
	char *p_result = NULL;
	char buf_val[64] = {0}, phyStatus[32] = {0}, trafficStatus[32] = {0};
	char cwmpIfname[32] = {0}, informStatus[32] = {0};
	char gpon_password[256] = {0}, time_buf[24] = {0}, gpon_sn[128] = {0};
	char active[12] = {0}, web_username[256] = {0}, web_passwd[256] = {0};
	int r_status = -1, r_result = -1, r_start_time = 0, r_inform_status = 0;
	int r_new_status = -1, r_new_result = -1, do_register = 0;
	struct timespec req_time_now;

	ZERO(buf_val);
	if ( tcapi_get("deviceAccount_Entry", "startTime", buf_val) < 0 )
		strcpy(buf_val, "0");
	r_start_time = atoi(buf_val);
	/* GET current time. */
	bzero(&req_time_now, sizeof(req_time_now));
	clock_gettime(CLOCK_MONOTONIC, &req_time_now);

	/* GET XPON status. */
	tcapi_get("XPON_Common", "phyStatus", phyStatus);
	tcapi_get("XPON_Common", "trafficStatus", trafficStatus);

	/* GET device register status & result. */
	ZERO(buf_val);
	tcapi_get("deviceAccount_Entry", "registerStatus", buf_val);
	r_status = atoi(buf_val);
	ZERO(buf_val);
	tcapi_get("deviceAccount_Entry", "registerResult", buf_val);
	r_result = atoi(buf_val);

	ZERO(buf_val);
	tcapi_get("deviceAccount_Entry", "newStatus", buf_val);
	r_new_status = atoi(buf_val);
	ZERO(buf_val);
	tcapi_get("deviceAccount_Entry", "newResult", buf_val);
	r_new_result = atoi(buf_val);

	/* Check register status & result. */
	if ( 1 != r_new_status )
		r_status = 99;
	if ( 1 != r_new_result )
		r_result = 99;

	/* GET GPON password. */
	pval_gpon_password = get_param(g_var_post, "password");
	ZERO(gpon_password);
	tcapi_get("GPON_ONU", "Password", gpon_password);

	cmcc_dbg_info(1);

	/* 1. Check action. */
	if ( p_action 
		&& 0 == strcmp(p_action, "GetRegStat") )
	{
		/* check normal username & password. */
		tcapi_get("Account_Entry1", "Active", active);
		tcapi_get("Account_Entry1", "username", web_username);
		tcapi_get("Account_Entry1", "web_passwd", web_passwd);

		pval_chk_name = get_param(g_var_post, "username");
		pval_chk_pwd = get_param(g_var_post, "userpwd");
		if ( 0 != strcmp(active, "Yes")
			|| NULL == pval_chk_name || NULL == pval_chk_pwd
			|| 0 != strcmp(pval_chk_name, web_username)
			|| 0 != strcmp(pval_chk_pwd, web_passwd) )
		{
			cmcc_dbg_info(2);
			p_result = "Auth Failed";
			goto JSON_CREATE;
		}
		
		if ( 0 == r_start_time )
		{
			cmcc_dbg_info(3);
			p_result = "Ready";
			goto JSON_CREATE;
		}

		/* WHEN PHY UP */
		if ( strstr(phyStatus, "up") )
		{
			if ( 0 == gpon_password[0] )
			{
				cmcc_dbg_info(4);
				p_result = "Ready";
				goto JSON_CREATE;
			}

			/* WHEN  TRAFFIC UP */
			if ( strstr(trafficStatus, "up") )
			{
				if ( tcapi_get("CwmpRoute_Entry", "ifName0", cwmpIfname) < 0 )
					ZERO(cwmpIfname);

				/* Check cwmp wan ip */
				if ( 0 == cwmpIfname[0] )
				{
					/* check timeout */
					if ( req_time_now.tv_sec - r_start_time <= 120 )
					{
						cmcc_dbg_info(5);
						p_result = "Getting WAN Address";
						goto JSON_CREATE;
					}
					else
					{
						cmcc_dbg_info(6);
						tcapi_set("deviceAccount_Entry", "startTime", "0");
						p_result = "Getting Address Failed";
						goto JSON_CREATE;
					}
				}
				else
				{
					if ( 0 == r_status )
					{
						if ( 0 == r_result )
						{
							cmcc_dbg_info(7);
							p_result = "Service Being Delivering";
							goto JSON_CREATE;
						}
						else if ( 1 == r_result )
						{
							cmcc_dbg_info(8);
							tcapi_set("deviceAccount_Entry", "startTime", "0");
							p_result = "Reg And Service Succ";
							goto JSON_CREATE;

						}
						else if ( 2 == r_result )
						{
							cmcc_dbg_info(9);
							tcapi_set("deviceAccount_Entry", "startTime", "0");
							p_result = "Service Failed";
							goto JSON_CREATE;
						}
						else
						{
							cmcc_dbg_info(10);
							p_result = "Waitting Service";
							goto JSON_CREATE;
						}
					}
					else if ( 1 == r_status || 2 == r_status || 3 == r_status )
					{
						cmcc_dbg_info(11);
						tcapi_set("deviceAccount_Entry", "startTime", "0");
						p_result = "Service Failed";
						goto JSON_CREATE;
					}
					else if ( 4 == r_status )
					{
						cmcc_dbg_info(12);
						tcapi_set("deviceAccount_Entry", "startTime", "0");
						p_result = "Service Timeout";
						goto JSON_CREATE;
					}
					else if ( 5 == r_status )
					{
						cmcc_dbg_info(13);
						tcapi_set("deviceAccount_Entry", "startTime", "0");
						p_result = "No Need Update Service";
						goto JSON_CREATE;
					}
					else
					{
						/* check timeout */
						if ( req_time_now.tv_sec - r_start_time <= 300 )
						{
							cmcc_dbg_info(14);
							p_result = "Registing RMS";
							goto JSON_CREATE;
						}
						else
						{
							cmcc_dbg_info(15);
							tcapi_set("deviceAccount_Entry", "startTime", "0");
							p_result = "RMS Timeout";
							goto JSON_CREATE;
						}
					}
				}
			}
			else
			{
				/* check timeout */
				if ( req_time_now.tv_sec - r_start_time > 80 )
				{
					cmcc_dbg_info(16);
					tcapi_set("deviceAccount_Entry", "startTime", "0");
					p_result = "Olt Failed";
					goto JSON_CREATE;
				}
				else
				{
					cmcc_dbg_info(17);
					p_result = "Registing Olt";
					goto JSON_CREATE;
				}
			}
		}
		else /* WHEN PHY DOWN */
		{
			/* check timeout */
			if ( req_time_now.tv_sec - r_start_time > 60 )
			{
				cmcc_dbg_info(18);
				tcapi_set("deviceAccount_Entry", "startTime", "0");
				p_result = "Olt Timeout";
				goto JSON_CREATE;
			}
			else
			{
				cmcc_dbg_info(19);
				p_result = "Registing Olt";
				goto JSON_CREATE;
			}
		}
	}
	/* 2. Check GPON password. */
	else if ( pval_gpon_password 
		&& 0 != pval_gpon_password[0] )
	{
		/* Check admin username & password */
		tcapi_get("Account_Entry0", "Active", active);
		tcapi_get("Account_Entry0", "username", web_username);
		tcapi_get("Account_Entry0", "web_passwd", web_passwd);
		tcapi_get("GPON_ONU", "SerialNumber", gpon_sn);

		pval_chk_name = get_param(g_var_post, "adminname");
		pval_chk_pwd = get_param(g_var_post, "adminpwd");
		pval_gpon_sn = get_param(g_var_post, "sn");

		if ( 0 != strcmp(active, "Yes")
			|| NULL == pval_chk_name || NULL == pval_chk_pwd
			|| 0 != strcmp(pval_chk_name, web_username)
			|| 0 != strcmp(pval_chk_pwd, web_passwd) )
		{
			cmcc_dbg_info(20);
			p_result = "Adminauth Failed";
			goto JSON_CREATE;
		}

		if ( strlen(pval_gpon_password) > MAX_PASSWORD_LEN )
		{
			cmcc_dbg_info(21);
			p_result = "Loid Param Wrong";
			goto JSON_CREATE;
		}

		if ( NULL == pval_gpon_sn
			|| 0 != strcmp(pval_gpon_sn, gpon_sn) )
		{
			cmcc_dbg_info(22);
			p_result = "SN Error";
			goto JSON_CREATE;
		}

		/* WHEN PHY UP */
		if ( strstr(phyStatus, "up") )
		{
			/* WHEN  TRAFFIC UP */
			if ( strstr(trafficStatus, "up") )
			{
				if ( 0 == r_status && 0 == r_result )
				{
					cmcc_dbg_info(23);
					p_result = "Register is running";
					goto JSON_CREATE;
				}

				do_register = 1;
			}
			else /* WHEN  TRAFFIC DOWN */
			{
				if ( 0 == strcmp(gpon_password, pval_gpon_password) )
				{
					if ( 0 != r_start_time ) /* registration is already started. */
					{
						/* check timeout */
						if ( req_time_now.tv_sec - r_start_time > 60 )
						{
							do_register = 1;
						}
						else
						{
							cmcc_dbg_info(24);
							p_result = "Register is running";
							goto JSON_CREATE;
						}
					}
					else
					{
						/* registration NOT start. */
						do_register = 1;
					}
				}
				else
				{
					do_register = 1;
				}
			}
		}
		else /* WHEN PHY DOWN */
		{
			do_register = 1;
		}
	}
	else /* error case */
	{
		cmcc_dbg_info(25);
		p_result = "Param Wrong";
		goto JSON_CREATE;
	}

	/* do register now. */
	tcapi_set("GPON_ONU", "Password", pval_gpon_password);
	tcapi_commit("GPON_ONU");
	tcapi_set("deviceAccount_Entry", "userPasswordDEV", pval_gpon_password);
	tcapi_set("Cwmp_Entry", "devregInform", "1");
	tcapi_commit("Cwmp_Entry");
	tcapi_set("deviceAccount_Entry", "newStatus", "0");
	tcapi_set("deviceAccount_Entry", "newResult", "0");
	snprintf(time_buf, sizeof(time_buf), "%d", req_time_now.tv_sec);
	tcapi_set("deviceAccount_Entry", "startTime", time_buf);
	tcapi_commit("deviceAccount_Entry");
	p_result = "Succ";
	cmcc_dbg_info(26);

JSON_CREATE:
	asprintf(result, AUTH_RESULT, p_result);

	return 0;
}

int cmcc_dbg_info(int step)
{
	char buf[128] = {0};

	ZERO(buf);
	snprintf(buf, sizeof(buf), "%d", step);
	doValPut("/tmp/cmcc_zero_itms_step", buf);

	return 0;
}


