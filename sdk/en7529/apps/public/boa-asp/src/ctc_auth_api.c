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
#include "ctc_auth_api.h"
#include "md5.h"
#if defined(TCSUPPORT_CUC)
#include "http-get-utils.h"
#endif
#include <common/ecnt_global_macro.h>
/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************/
#define ZERO(x) bzero(x, sizeof(x))
#if defined(TCSUPPORT_CGNX)
#define AUTH_RESULT \
"{\n\
	\"Locked\":\"%d\",\n\
	\"Logged\":\"%d\",\n\
	\"Privilege\":\"%d\",\n\
	\"CaptchaOK\":\"%d\",\n\
	\"Active\":\"%d\",\n\
	\"Luci\":\"%d\",\n\
	\"ecntToken\":\"%d%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\"\n\
}"
#else
#define AUTH_RESULT \
"{\n\
	\"Locked\":\"%d\",\n\
	\"Logged\":\"%d\",\n\
	\"Privilege\":\"%d\",\n\
	\"Active\":\"%d\",\n\
	\"Luci\":\"%d\",\n\
	\"ecntToken\":\"%d%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\"\n\
}"
#endif
#define REQ_SHORT_TIMEOUT 10

#if defined(TCSUPPORT_CUC)
#define RES_HEADER \
"{\n\
\"Result\":\"%s\",\n\
\"Objects\":["

#define RES_END \
"]\n\
}"

#define RES_HEADER_SINGLE \
"{\n\
\"Result\":\"%s\",\n\
\"OneObject\":{"

#define RES_END_SINGLE \
"}\n\
}"
#endif
/************************************************************************
*                  M A C R O S
*************************************************************************/

/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************/
typedef struct param_s {
	char	*name;
	char	*value;
} s_param;

/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************/
extern void free_param(s_param **var);
extern s_param **param_line (char *line, char separate1, char separate2);
extern char cur_username[];
extern int bReq_TimeOut[];
extern struct timespec pre_time_new[];
extern struct timespec pre_user_time_new[];

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
************************************************************************/

/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************/

/************************************************************************
*                  P R I V A T E   D A T A
*************************************************************************/
static int g_privilege = TYPE_NONE;

/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
*************************************************************************/

/*
	CTC version there is no check for username.
*/
int ctc_check_auth
(asp_reent* reent, const asp_text* params, char *p_username, char *p_password, struct ctc_au_param *au_param, char **result)
{
	char web_passwd[256] = {0}, web_account[256] = {0}, isLockState[16] = {0}, isLogin[16] = {0};
	char node_name[64] = {0}, node_val[64] = {0};
	int is_locked = 0, privilege_type = TYPE_NONE, logined_type = TYPE_NONE;
	char base64_username[256] = {0}, path[256] = {0}, token[128] = {0};
	char log[128] = {0};
	char remoteIp[64] = {0};
	char* p  = NULL;
	char active[8] = {0};
	int is_active = 0;
	unsigned char md5_result[16] = {0};
	int r = 0, idx = 0, is_ui = 0, login_times = 0, luci_login = 0;
	#if defined(TCSUPPORT_CGNX)
	int is_captchaok = 0;
	#endif
	struct MD5Context md5;
	request *req = NULL;
	s_param **var = NULL;
	FILE *fp = NULL;
	struct timespec req_time_now = {0}, req_time_diff = {0};
#if defined(TCSUPPORT_CUC)
	int login_right = 0;
#endif

	ZERO(md5_result);

	/* attack defense. */
	req = (request *)reent->server_env;
	if ( NULL== req )
		goto JSON_CREATE;

	if ( req->cookie )
	{
		var = param_line (req->cookie, '=',';');
		for(  idx = 0; var[idx]; idx++)
		{
			if ( 
#if defined(TCSUPPORT_CUC)
				0 == login_times && 
#endif
				NULL != strstr(var[idx]->name, "loginTimes") )
			{
				is_ui = 1;
				login_times = atoi(var[idx]->value) + 1;
#if !defined(TCSUPPORT_CUC)
				break;
#endif
			}
#if defined(TCSUPPORT_CUC)
			if ( 0 == login_right && NULL != strstr(var[idx]->name, "loginRight") )
			{
				login_right = atoi(var[idx]->value);
			}

			if( 0 != login_times && 0 != login_right )
				break;
#endif	
		}
		free_param(var);
	}

	if ( au_param && au_param->login_via_luci && p_password )
		luci_login = 1;

	if ( 0 == is_ui
		&& 0 == luci_login )
	{
		goto JSON_CREATE;
	}

	/* 1. check locked. */
	ZERO(isLockState);
	if ( 0 == tcapi_get("WebCurSet_Entry", "isLockState", isLockState)
		&& 1 == atoi(isLockState) )
	{
		privilege_type = TYPE_NONE;
		is_locked = 1;
		goto JSON_CREATE;
	}

	/* 2. check account */
#if defined(TCSUPPORT_CUC)
	ZERO(web_passwd);
	ZERO(active);

	tcapi_get("Account_Entry0", "Active", active);
	tcapi_get("Account_Entry0", "web_passwd", web_passwd);

	if ( 1 == login_right &&
		TYPE_NONE == privilege_type &&
		0 == strcmp(active, "Yes") &&
		p_password && 0 == strcmp(web_passwd, p_password) )
	{
		privilege_type = TYPE_ADMIN;
		is_active++;
	}
	else
	{
		ZERO(web_account);
		ZERO(web_passwd);
		ZERO(active);
		tcapi_get("Account_Entry1", "username", web_account);
		tcapi_get("Account_Entry1", "Active", active);
		tcapi_get("Account_Entry1", "web_passwd", web_passwd);
		if ( TYPE_NONE == privilege_type
			&& 0 == strcmp(active, "Yes")
			&& p_username && 0 == strcmp(web_account, p_username)
			&& p_password && 0 == strcmp(web_passwd, p_password) )
		{
			privilege_type = TYPE_USER;
			is_active++;
		}
	}
#else
	ZERO(web_passwd);
	tcapi_get("Account_Entry1", "web_passwd", web_passwd);
	if ( TYPE_NONE == privilege_type && p_password && 0 == strcmp(web_passwd, p_password) )
	{
		privilege_type = TYPE_USER;
		ZERO(active);
		tcapi_get("Account_Entry1", "Active", active);
		if ( 0 == strcmp(active, "Yes") )
			is_active++;
	}
	
	ZERO(web_passwd);
	tcapi_get("Account_Entry0", "web_passwd", web_passwd);
	if ( TYPE_NONE == privilege_type && p_password && 0 == strcmp(web_passwd, p_password) )
	{
		privilege_type = TYPE_ADMIN;
		ZERO(active);
		tcapi_get("Account_Entry0", "Active", active);
		if ( 0 == strcmp(active, "Yes") )
			is_active++;
	}
#endif
#if defined(TCSUPPORT_CGNX)
	if ( 0 == captchaValuecheck("WebCurSet_Entry", "CaptchaOK", NULL))
	{
		privilege_type = TYPE_NONE;
		is_captchaok = 2;
		goto JSON_CREATE;
	}
#endif
	if ( TYPE_NONE == privilege_type )
	{
#ifdef TCSUPPORT_SYSLOG
		openlog("TCSysLog WEB", 0, LOG_LOCAL2);
		syslog(LOG_INFO, "WEB login failed!\n");
		closelog();
#endif
		if ( login_times >= 3 )
		{
			tcapi_set("Account_Entry0", "LoginTimes", "3");
			tcapi_commit("Account_Entry0");
			tcapi_set("Account_Entry0","LoginTimes", "0");
			tcapi_set("WebCurSet_Entry","isLockState","1");
			is_locked = 1;
		}
		goto JSON_CREATE; /* password check fail, do not create token. */
	}
	else
	{
		/* check whether user close browser directly. */
		for ( idx = 0 ; idx < 2; idx ++ )
		{
			req_time_diff.tv_nsec = 0;
			clock_gettime(CLOCK_MONOTONIC, &req_time_now);
			req_time_diff.tv_sec = req_time_now.tv_sec - pre_time_new[idx].tv_sec;
			req_time_diff.tv_nsec += req_time_now.tv_nsec - pre_time_new[idx].tv_nsec;
			while(req_time_diff.tv_nsec > 1000000000)
			{
				req_time_diff.tv_sec++;
				req_time_diff.tv_nsec -= 1000000000;
			}
			if(req_time_diff.tv_nsec < 0)
			{
				req_time_diff.tv_sec--;
			}

			if ( req_time_diff.tv_sec > REQ_SHORT_TIMEOUT )
			{
				bzero(node_name, sizeof(node_name));
				snprintf(node_name, sizeof(node_name), "Account_Entry%d", idx);
				if ( 0 != tcapi_get(node_name, "Logged", node_val) )
					strcpy(node_val, "0");
				if ( 0 == strcmp(node_val, "1") )
				{
					tcapi_set(node_name, "Logged", "0");
					bReq_TimeOut[idx] = 1;
				}
			}
		}


		/* 3. check logined */
		ZERO(isLogin);
		if ( 0 == tcapi_get("Account_Entry0", "Logged", isLogin)
			&& 1 == atoi(isLogin) )
		{
			privilege_type = TYPE_NONE;
			logined_type = TYPE_ADMIN;
			goto JSON_CREATE;
		}
		ZERO(isLogin);
		if ( 0 == tcapi_get("Account_Entry1", "Logged", isLogin)
			&& 1 == atoi(isLogin) )
		{
			privilege_type = TYPE_NONE;
			logined_type = TYPE_USER;
			goto JSON_CREATE;
		}
	}

	if ( TYPE_NONE != privilege_type && 0 == is_active )
		goto JSON_CREATE; /* no access to login */

TOKEN_CREATE:
	srand(time(NULL));
	r = rand();

	MD5Init(&md5);
	MD5Update(&md5, &r, sizeof(r));
	MD5Final(md5_result, &md5);

	if ( TYPE_ADMIN == privilege_type )
	{
		tcapi_get("Account_Entry0", "username", cur_username);
		tcapi_set("Account_Entry0", "Logged", "1");
		tcapi_set("Account_Entry0", "Logoff", "0");
		tcapi_set("WebCurSet_Entry", "CurrentAccess", "0");

		clock_gettime(CLOCK_MONOTONIC, &pre_time_new[0]);
		if ( 0 != strcmp(req->pathname,"/boaroot/cgi-bin/refresh.asp") )
			clock_gettime(CLOCK_MONOTONIC,&pre_user_time_new[0]);

		bReq_TimeOut[0] = 0;
	}
	else
	{
		tcapi_get("Account_Entry1", "username", cur_username);
		tcapi_set("Account_Entry1", "Logged", "1");
		tcapi_set("Account_Entry1", "Logoff", "0");
		tcapi_set("WebCurSet_Entry", "CurrentAccess", "1");

		clock_gettime(CLOCK_MONOTONIC, &pre_time_new[1]);
		if ( 0 != strcmp(req->pathname,"/boaroot/cgi-bin/refresh.asp") )
			clock_gettime(CLOCK_MONOTONIC,&pre_user_time_new[1]);

		bReq_TimeOut[1] = 0;
	}

#ifdef TCSUPPORT_SYSLOG
	snprintf(remoteIp, sizeof(remoteIp), "%s", ((request *)reent->server_env)->remote_ip_addr);
	p =strstr(remoteIp, "%");
	if(p){
		*p = '\0';
	}
	snprintf(log, sizeof(log), "WEB user <%s> login , IP: %s .\n", cur_username, remoteIp);
	openlog("TCSysLog WEB", 0, LOG_LOCAL2);
	syslog(LOG_INFO, log);
	closelog();
#endif

	snprintf(token, sizeof(token), "%d%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
					, privilege_type
					, md5_result[0], md5_result[1], md5_result[2], md5_result[3]
					, md5_result[4], md5_result[5], md5_result[6], md5_result[7]
					, md5_result[8], md5_result[9], md5_result[10], md5_result[11]
					, md5_result[12], md5_result[13], md5_result[14], md5_result[15]);
	snprintf(path, sizeof(path), "/var/tmp/ecntToken_%s", token);
	fp = fopen(path, "w");
	if ( fp )
	{
		fprintf(fp, "ecntToken");
		fclose(fp);
	}

JSON_CREATE:
#if defined(TCSUPPORT_CGNX)
	asprintf(result, AUTH_RESULT, is_locked, logined_type, privilege_type, is_captchaok, is_active
					, luci_login
					, privilege_type
					, md5_result[0], md5_result[1], md5_result[2], md5_result[3]
					, md5_result[4], md5_result[5], md5_result[6], md5_result[7]
					, md5_result[8], md5_result[9], md5_result[10], md5_result[11]
					, md5_result[12], md5_result[13], md5_result[14], md5_result[15]);
#else
	asprintf(result, AUTH_RESULT, is_locked, logined_type, privilege_type, is_active
					, luci_login
					, privilege_type
					, md5_result[0], md5_result[1], md5_result[2], md5_result[3]
					, md5_result[4], md5_result[5], md5_result[6], md5_result[7]
					, md5_result[8], md5_result[9], md5_result[10], md5_result[11]
					, md5_result[12], md5_result[13], md5_result[14], md5_result[15]);
#endif
	return 0;
}

/* token auth check. */
int ctc_http_token_authorize(request *req)
{
	char token[128] = {0}, path[256] = {0}, valbuf[128] = {0};
	int idx = 0;
	FILE *fp = NULL;
	s_param **var = NULL;

	if ( NULL == req->cookie )
		return -1;

	ZERO(token);
	var = param_line (req->cookie, '=',';');
	for(  idx = 0; var[idx]; idx++)
	{
		if ( 0 == strcmp(var[idx]->name, "ecntToken")
			|| 0 == strcmp(var[idx]->name, " ecntToken"))
		{
			snprintf(token, sizeof(token), "%s", var[idx]->value);
			break;
		}
	}
	free_param(var);

	/* there is no token. */
	if ( 0 == token[0] )
		return -1;

	snprintf(path, sizeof(path), "/var/tmp/ecntToken_%s", token);
	fp = fopen(path, "r");
	if ( fp )
	{
		fclose(fp);
		fp = NULL;

		/* update cur_username */
		if ( token[0] == '1' )
		{
			ZERO(valbuf);
			if ( bReq_TimeOut[0] == 1 || 
				(0 == tcapi_get("Account_Entry0", "Logoff", valbuf)
				&& 0 == strcmp(valbuf, "1")) )
			{
				tcapi_set("Account_Entry0", "Logoff", "0");
				system("/bin/rm -f /var/tmp/ecntToken_*");
				return -1;
			}
			
			tcapi_get("Account_Entry0", "username", cur_username);
			clock_gettime(CLOCK_MONOTONIC, &pre_time_new[0]);
			if ( 0 != strcmp(req->pathname,"/boaroot/cgi-bin/refresh.asp") )
				clock_gettime(CLOCK_MONOTONIC,&pre_user_time_new[0]);
			bReq_TimeOut[0] = 0;
		}
		else
		{
			ZERO(valbuf);
			if ( bReq_TimeOut[1] == 1 || 
				(0 == tcapi_get("Account_Entry1", "Logoff", valbuf)
				&& 0 == strcmp(valbuf, "1")) )
			{
				tcapi_set("Account_Entry1", "Logoff", "0");
				system("/bin/rm -f /var/tmp/ecntToken_*");
				return -1;
			}

			tcapi_get("Account_Entry1", "username", cur_username);
			clock_gettime(CLOCK_MONOTONIC, &pre_time_new[1]);
			if ( 0 != strcmp(req->pathname,"/boaroot/cgi-bin/refresh.asp") )
				clock_gettime(CLOCK_MONOTONIC,&pre_user_time_new[1]);
			bReq_TimeOut[1] = 0;
		}

		return 0;
	}

	return -1;
}


int set_lanhost_hostName
(asp_reent* reent, const asp_text* params, char *p_idx, char *p_newhostname, struct ctc_au_param *au_param, char **result)
{
	char newHostName[64] = {0}, node_name[64] = {0};
	int inlen = 0, outlen = 0;
	
	snprintf(node_name, sizeof(node_name), "LANHost2_Entry%s", p_idx);

	inlen = strlen(p_newhostname);
	outlen = sizeof(newHostName);
	if ( 0 == charsetconv("gb2312", "utf-8", &inlen, &outlen, p_newhostname, newHostName) )
		tcapi_set(node_name, "HostName", newHostName);
	else	
	{
		tcapi_set(node_name, "HostName", "");
	}

    tcapi_save();

	return 0;
}

int set_lanhost_hostNamelist
(asp_reent* reent, const asp_text* params, char *p_gatewayname, char *p_newhostnamelist, struct ctc_au_param *au_param, char **result)
{
	char newHostName[128] = {0}, coved_newHostName[128] = {0}, gatewayName[128] = {0}, node_name[64] = {0};
	int inlen = 0, outlen = 0, entry_idx = 0, ret = 0;
	char *temp = NULL, *safep1 = NULL;

	if(p_newhostnamelist)
	{
		temp = strtok_r(p_newhostnamelist, ";", &safep1);
		while (temp) {
			memset(newHostName, 0, sizeof(newHostName));
			memset(coved_newHostName, 0, sizeof(coved_newHostName));
			memset(node_name, 0, sizeof(node_name));
			ret = sscanf(temp, "%d:%s", &entry_idx, newHostName);
			if( ret == 2 || ret == 1)
			{
				snprintf(node_name, sizeof(node_name), "LANHost2_Entry%d", entry_idx);
				inlen = strlen(newHostName);
				outlen = sizeof(coved_newHostName);
				if ( 0 == charsetconv("gb2312", "utf-8", &inlen, &outlen, newHostName, coved_newHostName) )
					tcapi_set(node_name, "HostName", coved_newHostName);
				else	
				{
					tcapi_set(node_name, "HostName", "");
				}
			}	
		
			temp = strtok_r(NULL, ";", &safep1);
		}

		tcapi_save();
	}
	else if(p_gatewayname)
	{
		memset(gatewayName, 0, sizeof(gatewayName));

		inlen = strlen(p_gatewayname);
		outlen = sizeof(gatewayName);
		if ( 0 == charsetconv("gb2312", "utf-8", &inlen, &outlen, p_gatewayname, gatewayName) )
			tcapi_set("Sys_Entry", "DevName", gatewayName);
		else	
		{
			tcapi_set("Sys_Entry", "DevName", "");
		}

		tcapi_save();
	}

	return 0;
}

#if defined(TCSUPPORT_CT_JOYME4)	
int set_urlfilter
(asp_reent* reent, const asp_text* params, char *action, char *p_urlinfo, struct ctc_au_param *au_param, char **result)
{
	char newHostName[64] = {0}, node_name[64] = {0}, attr_value[32] = {0}, url[512]= {0}, url_num_s[16] = {0};
	char delIdx[300] = {0}, *safep = NULL, *p = NULL; 
	int inlen = 0, outlen = 0, i = 0, url_num_i = 0, exit_flag = 0;;

	snprintf(node_name, sizeof(node_name), "UrlFilter_Entry0");
	tcapi_get(node_name, "url_num", url_num_s);
	url_num_i = atoi(url_num_s);
	if(!strcmp(action, "add"))
	{
		for(i = 0; i < 100; i++)
		{
			memset(attr_value, 0, sizeof(attr_value));
			snprintf(attr_value, sizeof(attr_value), "URL%d", i);
		
			if ( 0 == tcapi_get(node_name, attr_value, url)
				&& 0 != url[0] )
			{
				continue;
			}
			else
			{
				tcapi_set(node_name, attr_value, p_urlinfo);
				snprintf(url_num_s, sizeof(url_num_s), "%d", url_num_i);
				tcapi_set(node_name, "url_num", url_num_s);
				break;
			}
		}
	}
	else if(!strcmp(action, "del"))
	{
		memset(url_num_s, 0, sizeof(url_num_s));
		strncpy(url_num_s, p_urlinfo, sizeof(url_num_s) - 1);
		p = strtok_r(p_urlinfo, ",", &safep);
		while (p)
		{
			exit_flag++;
			if(exit_flag > 10)
				return;
			memset(attr_value, 0, sizeof(attr_value));
			snprintf(attr_value, sizeof(attr_value), "URL%s", p);
			if(strcmp(p, "0"))
				url_num_i--;
			tcapi_set(node_name, attr_value, "");
			p = strtok_r(NULL, ",", &safep);
			
		}
		snprintf(url_num_s, sizeof(url_num_s), "%d", url_num_i);
		tcapi_set(node_name, "url_num", url_num_s);
	}
	tcapi_commit(node_name);
    tcapi_save();

	return 0;
}
#endif

#if defined(TCSUPPORT_CUC)
jsonmem_t* json_mem_new(int size){
	jsonmem_t *newjson = NULL;
	char *p = NULL;
	int allsize = offsetof(jsonmem_t, payload) + size + 1;

	json_malloc(p, allsize, NULL);
	
	newjson = (jsonmem_t *)p;
	newjson->size = size;
	newjson->current = newjson;

	return newjson;
}

int json_mem_addbuf(jsonmem_t *jmem, char *text, int size)
{
	int left = 0;
	jsonmem_t* jcur = jmem->current;
	jsonmem_t* jnew = NULL;

	left = jmem->size - jcur->offset; 
	
	if( size >= left ){
		jcur->offset += snprintf(jcur->payload+jcur->offset, left + 1, "%s", text);
		
		if( !(jnew = json_mem_new(jmem->size)) ){
			return -1;
		}
		
		jcur->next = jnew;
		jmem->current = jnew;
		jnew->size = jcur->size + jmem->size;
		if( size > left ){
			/* left : buffer copy before */
			json_mem_addbuf(jmem, text + left, size - left);
		}
	}
	else {
		jcur->offset += snprintf(jcur->payload+jcur->offset, left + 1, "%s", text);

	}

	return 0;
}

int json_mem_addfmt(jsonmem_t *jmem, int maxsize, const char *format, ...)
{
	va_list args;
	char *buf = NULL;
	int len = 0, ret = 0;
	
	json_malloc(buf, maxsize, -1);
	
	va_start(args, format);
	len = vsnprintf(buf, maxsize, format, args);
	va_end(args);

	ret = json_mem_addbuf(jmem, buf, len);
	free(buf);
	
	return ret;
}

int json_buf_fmt(char **pbuf, int size, const char *format, ...)
{
	va_list args;
	int len = 0;
	
	if( NULL == pbuf )
	{
		return -1;
	}

	if( *pbuf == NULL ){
		json_malloc(*pbuf, size, -1);
	}

	va_start(args, format);
	len = vsnprintf(*pbuf, size, format, args);
	va_end(args);

	return len;
}

void json_buf_free(char **pbuf)
{
	if( pbuf && (*pbuf ) ){
		free(*pbuf);
		*pbuf = NULL;
	}
}

void json_mem_print(jsonmem_t *jmem){
	jsonmem_t* jcur  = jmem;
	int i = 0;

	while(jcur){
		jcur=jcur->next;
	}
}

char* json_mem2buf(jsonmem_t *jmem){
	jsonmem_t* jcur  = jmem;
	char *buf = NULL;
	int len = 0, offset = 0;
	
	if(jmem == jmem->current){
		len = jmem->offset;
	}
	else{
		len = jmem->current->offset + jmem->current->size - jmem->size;
	}
	json_malloc(buf, len+1, NULL);

	while(jcur){
		offset += snprintf(buf + offset, len + 1 - offset, "%s", jcur->payload);
		jcur=jcur->next;
	}

	return buf;
}

int json_mem2lenbuf(jsonmem_t *jmem, char **pkt){
	jsonmem_t* jcur  = jmem;
	char *buf = NULL, *encrypt_buf = NULL;
	int len = 0, offset = 0, ret = 0;
	size_t out_len = 0;
	
	if(jmem == jmem->current){
		len = jmem->offset;
	}
	else{
		len = jmem->current->offset + jmem->current->size - jmem->size;
	}
	
	json_malloc(buf, len+5, -1);
	offset=4;
	while(jcur){
		offset += snprintf(buf + offset, len + 5 - offset, "%s", jcur->payload);
		jcur=jcur->next;
	}
	json_mem_free(jmem);
	len = htonl(len);
	memcpy(buf, (char *)&len, 4);
	*pkt = buf;

	return offset;
}

void json_mem_free(jsonmem_t *jmem){
	jsonmem_t *jcur  = jmem;
	char *p = NULL;
	int i = 0;

	while(jcur){
		p = (char *)jcur;
		jcur=jcur->next;
		free(p);
	}
}

int ecnt_get_ssid1_info
(asp_reent* reent, const asp_text* params, char *param1, char *param2, struct ctc_au_param *au_param, char **result)
{
	jsonmem_t *resValue;
	char EnableSSID[8] = {0}, HideSSID[8] = {0}, SSID[256] = {0}, AuthMode[64] = {0}, WPAPSK[64] = {0};
	char WEPAuthType[32] = {0}, Key1Str[32] = {0}, Key2Str[32] = {0}, Key3Str[32] = {0}, Key4Str[32] = {0}, DefaultKeyID[8] = {0};

	resValue = json_mem_new(1024);
	if ( NULL == resValue )
		return -1;
	json_jmem_addfmt(-1, resValue, 32, RES_HEADER_SINGLE, "OK");

	tcapi_get("Wlan_Entry0", "HideSSID", HideSSID);
	tcapi_get("Wlan_Entry0", "EnableSSID", EnableSSID);
	tcapi_get("Wlan_Entry0", "SSID", SSID);
	tcapi_get("Wlan_Entry0", "AuthMode", AuthMode);
	
	json_jmem_addfmt(-1, resValue, 120, "\"HideSSID\":\"%s\""
									, HideSSID);

	json_jmem_addfmt(-1, resValue, 120, "%s\"EnableSSID\":\"%s\""
									, ","
									, EnableSSID);

	json_jmem_addfmt(-1, resValue, 120, "%s\"SSID\":\"%s\""
									, ","
									, SSID);

	json_jmem_addfmt(-1, resValue, 120, "%s\"AuthMode\":\"%s\""
									, ","
									, AuthMode);

	if(strstr(AuthMode, "WEP"))
	{
		tcapi_get("Wlan_Entry0", "WEPAuthType", WEPAuthType);
		tcapi_get("Wlan_Entry0", "DefaultKeyID", DefaultKeyID);
		tcapi_get("Wlan_Entry0", "Key1Str", Key1Str);
		tcapi_get("Wlan_Entry0", "Key2Str", Key2Str);
		tcapi_get("Wlan_Entry0", "Key3Str", Key3Str);
		tcapi_get("Wlan_Entry0", "Key4Str", Key4Str);

		json_jmem_addfmt(-1, resValue, 120, ",\"WEPAuthType\":\"%s\""
			, WEPAuthType);

		json_jmem_addfmt(-1, resValue, 120, ",\"DefaultKeyID\":\"%s\""
			, DefaultKeyID);

		json_jmem_addfmt(-1, resValue, 120, ",\"Key1Str\":\"%s\""
			, Key1Str);

		json_jmem_addfmt(-1, resValue, 120, ",\"Key2Str\":\"%s\""
			, Key2Str);

		json_jmem_addfmt(-1, resValue, 120, ",\"Key3Str\":\"%s\""
			, Key3Str);

		json_jmem_addfmt(-1, resValue, 120, ",\"Key4Str\":\"%s\""
			, Key4Str);
													
	}
	else
	{	
		tcapi_get("Wlan_Entry0", "WPAPSK", WPAPSK);
		json_jmem_addfmt(-1, resValue, 120, ",\"WPAPSK\":\"%s\""
										, WPAPSK);
	}	
	
	json_jmem_addfmt(-1, resValue, 16, RES_END_SINGLE);

	*result = json_mem2buf(resValue);
	json_mem_free(resValue);

	return 0;
}

/*
must do <% tcWebApi_get("WanInfo_Common","CycleValue","h") %> on asp page first.
*/
int ecnt_get_wan_interface_name
(asp_reent* reent, const asp_text* params, char *param1, char *param2, struct ctc_au_param *au_param, char **result)
{
	int idx = 0, line = 0, pvc = 0;
	jsonmem_t *resValue;
	char waninfo_node[32] = {0}, str_guiinterfacename[128] = {0}, str_ifname[32] = {0};

	tcdbg_printf("%s %d enter \n",__FUNCTION__, __LINE__);

	resValue = json_mem_new(1024);
	if ( NULL == resValue )
		return -1;
	json_jmem_addfmt(-1, resValue, 32, RES_HEADER_SINGLE, "OK");

	for ( pvc = 0; pvc < 8; pvc ++ )
	{
		for ( idx = 0; idx < 8; idx ++ )
		{
			snprintf(waninfo_node, sizeof(waninfo_node), "WanInfo_Entry%d", 8*pvc + idx);
			if ( 0 == tcapi_get(waninfo_node, "GUIInterfaceName", str_guiinterfacename)
				&& 0 != str_guiinterfacename[0] )
			{
				snprintf(waninfo_node, sizeof(waninfo_node), "Wan_PVC%d_Entry%d", pvc, idx);
				tcapi_get(waninfo_node, "IFName", str_ifname);
				tcdbg_printf("%s %d enter str_ifname[%s] \n",__FUNCTION__, __LINE__, str_ifname);
				json_jmem_addfmt(-1, resValue, 120, "%s\"%s\":\"%s\""
												, (0 == line ? "" : ",")
												, str_ifname, str_guiinterfacename);
				line ++;
			}
		}

	}
	

	json_jmem_addfmt(-1, resValue, 120, "%s\"%s\":\"%s\""
											, (0 == line ? "" : ",")
											, "br0", "LAN");

	json_jmem_addfmt(-1, resValue, 16, RES_END_SINGLE);

	*result = json_mem2buf(resValue);
	json_mem_free(resValue);

	return 0;
}

/*
must do <% tcWebApi_get("WanInfo_Common","CycleValue","h") %> on asp page first.
*/
int ecnt_get_internet_ppp_waninfo
(asp_reent* reent, const asp_text* params, char *param1, char *param2, struct ctc_au_param *au_param, char **result)
{
	int idx = 0, line = 0, pvc = 0;
	jsonmem_t *resValue;
	char wan_node[32] = {0};
	char str_servicelist[128] = {0}, str_linkMode[24] = {0};
	char USERNAME[256] = {0}, PASSWORD[256] = {0}, WanMode[32] = {0};
	char waninfo_node[32] = {0}, str_guiinterfacename[128] = {0};

	resValue = json_mem_new(1024);
	if ( NULL == resValue )
		return -1;
	json_jmem_addfmt(-1, resValue, 32, RES_HEADER, "OK");

	/* CMCC only PVC0 */
	for ( pvc = 0; pvc < 8; pvc ++ )
	{
		for ( idx = 0; idx < 8; idx ++ )
		{
			/* find INTERNET PPP */
			snprintf(wan_node, sizeof(wan_node), "Wan_PVC%d_Entry%d", pvc, idx);
			snprintf(waninfo_node, sizeof(waninfo_node), "WanInfo_Entry%d", 8*pvc + idx);
			tcapi_get(wan_node, "ServiceList", str_servicelist);
			tcapi_get(wan_node, "LinkMode", str_linkMode);
			if ( strstr(str_servicelist, "INTERNET")
				&& 0 == strcmp(str_linkMode, "linkPPP") /* PPP */
				)
			{
				tcapi_get(waninfo_node, "GUIInterfaceName", str_guiinterfacename);
				tcapi_get(wan_node, "USERNAME", USERNAME);
				tcapi_get(wan_node, "PASSWORD", PASSWORD);
				tcapi_get(wan_node, "WanMode", WanMode);
				json_jmem_addfmt(-1, resValue, 120, "%s{\"ifidx\":\"%d\",\"guiname\":\"%s\",\"WanMode\":\"%s\",\"USERNAME\":\"%s\",\"PASSWORD\":\"%s\"}"
									, (0 == line ? "" : ",")
									, 8*pvc + idx, str_guiinterfacename, WanMode, USERNAME, PASSWORD);

				line ++;
			}
		}
	}

	json_jmem_addfmt(-1, resValue, 16, RES_END);

	*result = json_mem2buf(resValue);
	json_mem_free(resValue);

	return 0;
}

int ecnt_set_internet_ppp_waninfo
(asp_reent* reent, const asp_text* params, char *param1, char *param2, struct ctc_au_param *au_param, char **result)
{
	char wan_node[32] = {0};
	char str_servicelist[128] = {0}, str_linkMode[24] = {0};
	char *s_ifidx = NULL;
	int pvc = 0, idx = 0;

	s_ifidx = get_param(g_var_post, "WanIndex");
	if ( s_ifidx )
	{
		decode_uri(s_ifidx);
		pvc = atoi(s_ifidx)/8;
		idx = atoi(s_ifidx)%8;
	}
	else
		return -1;

	snprintf(wan_node, sizeof(wan_node), "Wan_PVC%d_Entry%d", pvc, idx);
	tcapi_get(wan_node, "ServiceList", str_servicelist);
	tcapi_get(wan_node, "LinkMode", str_linkMode);
	if ( strstr(str_servicelist, "INTERNET")
		&& 0 == strcmp(str_linkMode, "linkPPP") /* PPP */
		)
	{
		tcapi_set(wan_node, "USERNAME", param1);
		tcapi_set(wan_node, "PASSWORD", param2);
		tcapi_commit(wan_node);
	}

	return 0;
}

int ecnt_get_lanhost2_info
(asp_reent* reent, const asp_text* params, char *param1, char *param2, struct ctc_au_param *au_param, char **result)
{
	jsonmem_t *resValue = NULL;
	int idx = 0, cnt = 0;
	char node_name[32] = {0}, active[12] = {0}, macaddr[20] = {0};
	char port_s[12] = {0}, conn_type[12] = {0};

	resValue = json_mem_new(1024);
	if ( NULL == resValue )
		return -1;
	json_jmem_addfmt(-1, resValue, 32, RES_HEADER, "OK");

	for ( idx = 0; idx < MAX_LANHOST2_ENTRY_NUM; idx ++ )
	{
		bzero(node_name, sizeof(node_name));
		sprintf(node_name, "LANHost2_Entry%d", idx);
		if ( 0 != tcapi_get(node_name, "Active", active) )
			continue;

		tcapi_get(node_name, "MAC", macaddr);
		tcapi_get(node_name, "Port", port_s);
		tcapi_get(node_name, "ConnectionType", conn_type);

		json_jmem_addfmt(-1, resValue, 120, "%s{\"MAC\":\"%s\",\"Port\":\"%s\",\"ConnectionType\":\"%s\"}"
										, (0 == cnt ? "" : ",")
										, macaddr, port_s, conn_type);
		cnt ++;
	}

	json_jmem_addfmt(-1, resValue, 16, RES_END);

	*result = json_mem2buf(resValue);
	json_mem_free(resValue);

	return 0;
}

#endif
