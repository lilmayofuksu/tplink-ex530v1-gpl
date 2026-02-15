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
#ifndef _CTC_AUTH_API_H__
#define _CTC_AUTH_API_H__


/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/
#include <stdint.h>
#include "mini-asp.h"
#if defined(TCSUPPORT_CUC)
#include <stdarg.h>
#include <stddef.h>
#endif
/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************/

/************************************************************************
*                  M A C R O S
*************************************************************************/
#if defined(TCSUPPORT_CUC)
typedef struct jsonmem{
	int offset;
	int size;
	struct jsonmem *next;
	struct jsonmem *current;
	char payload[]; 
}jsonmem_t;

#define json_malloc( buf, size, reterr) { \
	buf = malloc(size); \
	if( NULL == buf ){ \
		tcdbg_printf("%s:>>malloc err!!\n", __FUNCTION__); \
		return reterr; \
		} \
	memset(buf, 0, size); \
}

#define json_jmem_addbuf( reterr, jmem, text, size) { \
	if( json_mem_addbuf(jmem, text, size) ){ \
		tcdbg_printf("%s:>>json_mem_addbuf err!!\n", __FUNCTION__); \
		return reterr; \
	} \
}

#define json_jmem_addfmt(reterr, jmem, maxsize, args...) { \
	if( json_mem_addfmt(jmem, maxsize, ## args) ){ \
		tcdbg_printf("%s:>>json_jmem_addfmt err!!\n", __FUNCTION__); \
		return reterr; \
	} \
}
#endif
/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************/
enum _PRIVILEGE_
{
	TYPE_NONE = 0,
	TYPE_ADMIN,
	TYPE_USER
};

struct ctc_au_param
{
	int login_via_luci;
};


/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************/


/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
************************************************************************/
int ctc_check_auth
(asp_reent* reent, const asp_text* params, char *p_username, char *p_password, struct ctc_au_param *au_param, char **result);
int set_lanhost_hostName
(asp_reent* reent, const asp_text* params, char *entryidx, char *new_hostname, struct ctc_au_param *au_param, char **result);
int set_lanhost_hostNamelist
(asp_reent* reent, const asp_text* params, char *p_gatewayname, char *p_newhostnamelist, struct ctc_au_param *au_param, char **result);
#if defined(TCSUPPORT_CT_JOYME4)
int set_urlfilter
(asp_reent* reent, const asp_text* params, char *action, char *p_urlinfo, struct ctc_au_param *au_param, char **result);
#endif

#if defined(TCSUPPORT_CUC)
jsonmem_t* json_mem_new(int size);
int json_mem_addbuf(jsonmem_t *jmem, char *text, int size);
int json_mem_addfmt(jsonmem_t *jmem, int maxsize, const char *format, ...);
int json_buf_fmt(char **pbuf, int size, const char *format, ...);
void json_buf_free(char **pbuf);
int json_mem_addnum(jsonmem_t* jmem, int maxsize, char *label, int number);
int json_mem_addtext(jsonmem_t* jmem, int maxsize, char *label, char *text);
int json_mem_addnumtext(jsonmem_t* jmem, int maxsize, char *label, char *text);
void json_mem_print(jsonmem_t *jmem);
char* json_mem2buf(jsonmem_t *jmem);
int json_mem2lenbuf(jsonmem_t *jmem, char **pkt);
void json_mem_free(jsonmem_t *jmem);
int ecnt_get_ssid1_info
(asp_reent* reent, const asp_text* params, char *param1, char *param2, struct ctc_au_param *au_param, char **result);
int ecnt_get_wan_interface_name
(asp_reent* reent, const asp_text* params, char *param1, char *param2, struct ctc_au_param *au_param, char **result);
int ecnt_get_internet_ppp_waninfo
(asp_reent* reent, const asp_text* params, char *param1, char *param2, struct ctc_au_param *au_param, char **result);
int ecnt_set_internet_ppp_waninfo
(asp_reent* reent, const asp_text* params, char *param1, char *param2, struct ctc_au_param *au_param, char **result);
int ecnt_get_lanhost2_info
(asp_reent* reent, const asp_text* params, char *param1, char *param2, struct ctc_au_param *au_param, char **result);
#endif
/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************/


/************************************************************************
*                  P R I V A T E   D A T A
*************************************************************************/


/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
*************************************************************************/



#endif

