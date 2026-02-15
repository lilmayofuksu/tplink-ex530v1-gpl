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
#include "mesh_getjson_api.h"
#include <inttypes.h>
#include <sys/ioctl.h>
#include <linux/if.h>

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************/
#define ZERO(x) bzero(x, sizeof(x))
#define MAPD_CTRL_FILE				"/tmp/mapd_ctrl"
#define MESH_INFO_FILE_CMD			"/tmp/mesh_info_stats"
#define STA_STEER_PROGRESS_FILE		"/tmp/sta_steer_progress"
#define STA_CONNSTATUS_FILE			"/tmp/sta_connstatus"
#define dump_file_path 				"/tmp/dump_topo.txt"
#define WLAN_PATH 					"/etc/Wireless/RT2860AP/RT2860AP.dat"
#define WLAN_AC_PATH 				"/etc/Wireless/RT2860AP_AC/RT2860AP.dat"

#define TOPO_BUF_LEN   				15000
#define MAX_INFO_LIST_NUM			8
#define MAX_APCLIENT_NUM				1
#define M_APCLIENT_2G_NAME_FORMAT		"apcli%d"
#define M_APCLIENT_5G_NAME_FORMAT		"apclii%d"

#define JSON_SUCCESS_FMT 			"{\\\"status\\\": \\\"SUCCESS\\\""
#define JSON_ERROR_FMT				"{\\\"status\\\": \\\"ERROR\\\""
#define JSON_COMMAND_FAILED_FMT		"{\\\"status\\\": \\\"Command Failed!\\\"}"
#define JSON_COMMAND_TIMEDOUT_FMT	"{\\\"status\\\": \\\"Command Timed-out!\\\"}"
#define JSON_MACLIST_FMT			"\\\"macList\\\": \\\"%s\\\","
#define JSON_NO_CLIENTS_AVALIABLE_FMT	"{\\\"status\\\": \\\"No Clients Available\\\"}"

#define JSON_RUNTIME_HASTOPO_FMT 	"%s,\\\"luaTopologyInfo\\\":\\\"%s\\\",\\\"luaTopologyInfoLen\\\": \\\"%d\\\" }"
#define JSON_AP_BACK_HAUL_INF_FMT 	"%s,%s\\\"apBhInfListStr\\\": \\\"%s\\\"}"
#define JSON_AP_FONT_HAUL_INF_FMT 	"%s,%s\\\"apFhInfListStr\\\": \\\"%s\\\"}"
#define JSON_STA_STEER_PROGRESS_FAILED_FMT	"{\\\"status\\\": \\\"Failed to open /tmp/sta_steer_progress file in read mode!\\\"}"
#define JSON_STA_STEERING_INFO_FMT	"{\\\"status\\\": \\\"SUCCESS\\\",\\\"sta_steering_info\\\": \\\"%s\\\"}"
#define JSON_STA_BHINFSTR_FMT		"\\\"staBhInfStr\\\":\\\"%s\\\""
#define JSON_STA_PROFILE_FMT		"\\\"profile\\\":\\\"%s\\\"}"
#define JSON_DEF_FMT				"%s"
#define CMD_GET_APCLI_CONNSTATUS_FMT	"/userfs/bin/iwconfig %s > %s"

#define MAP_ADDR_FMT_ARS(ea) \
	(ea)[0], (ea)[1], (ea)[2], (ea)[3], (ea)[4], (ea)[5]
#define MAP_ADDR_FMT_STR \
	"%02"PRIx8":%02"PRIx8":%02"PRIx8":%02"PRIx8":%02"PRIx8":%02"PRIx8

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

void fileRead(char *path, char *buf, int size)
{
	int  fd=0, ret=0;

	memset(buf,0, size);
	fd = open(path,O_RDONLY);
	if(fd == -1)
	{
		return;
	}

	ret = read(fd, buf, size - 1);
	if(ret <= 0)
	{
		close(fd);
		buf[0] = '\0';
		return;
	}
	buf[size - 1] = '\0';
	close(fd);
}

void strJsonConvertHTML(char *oriStr,char *desStr, int str_len)
{
	int i, j = 0;
	for(i = 0;i < str_len;i++)
	{
		if((oriStr[i] == '\n'))
		{
			strcpy(&(desStr[j])," ");
			j+=1;
		}
		else if(oriStr[i] == '"')
		{
    		strcpy(&(desStr[j]),"'");
    		j+=1;
		}
		else
		{
			desStr[j] = oriStr[i];
			j++;
		}
	}
	desStr[j] = '\0';
}

/*
*	Mesh get json for run_time_topology
*/
int mesh_get_run_time_topology_fun(asp_reent* reent, char **result)
{
	struct mapd_interface_ctrl *ctrl = NULL;
	int ret, write_topo_flag;
	char* topo_buf = NULL, *tmp_result_buf = NULL;
	size_t topo_buf_len, result_buf_len;
#ifdef TCSUPPORT_ECNT_MAP_ENHANCE
	FILE *fptr = NULL;
#endif
	
	ctrl = mapd_interface_ctrl_open(MAPD_CTRL_FILE);
	if(!ctrl)
	{
		return -1;
	}	
#ifdef TCSUPPORT_EASYMESH_R13	
	topo_buf = (char*)malloc(TOPO_BUF_LEN+1);
	if(topo_buf)
	{
		memset(topo_buf, 0, TOPO_BUF_LEN);
		topo_buf_len = TOPO_BUF_LEN;
		ret = mapd_interface_get_topology(ctrl, topo_buf, &topo_buf_len, NULL);
#else
	ret = mapd_interface_get_topology(ctrl, topo_buf, &topo_buf_len, dump_file_path);
#endif

	if(ret < 0)
	{
		topo_buf_len = 0;
		write_topo_flag = 0;
	}
	else
	{
		write_topo_flag = 1;
#ifdef TCSUPPORT_ECNT_MAP_ENHANCE
		fptr = fopen(dump_file_path, "r");
		if(fptr == NULL) {
			/*no topo info*/
			asprintf(result, JSON_DEF_FMT, JSON_COMMAND_FAILED_FMT);
			return 0;
		}
		fseek(fptr, 0, SEEK_END);
		topo_buf_len = ftell(fptr);
		if (topo_buf_len <= 0)
		{
			/*no topo info*/
			asprintf(result, JSON_DEF_FMT, JSON_COMMAND_FAILED_FMT);
			fclose(fptr);
			return 0;
		}
		
		topo_buf = (char*)malloc(topo_buf_len + 1);
		if(NULL == topo_buf) {
			/*no topo info*/
			asprintf(result, JSON_DEF_FMT, JSON_COMMAND_FAILED_FMT);
			fclose(fptr);
			return 0;
		}
		memset(topo_buf, 0, topo_buf_len + 1);
		fseek(fptr, 0, SEEK_SET);
		fread(topo_buf, topo_buf_len, 1, fptr);
		fclose(fptr);
#endif
	}
#ifdef TCSUPPORT_EASYMESH_R13		
	if( (1 == write_topo_flag) && (topo_buf != NULL) && (topo_buf_len > 0) )
#else
	if( 1 == write_topo_flag )
#endif
	{
		tmp_result_buf = (char*)malloc(topo_buf_len + 1);
		if(tmp_result_buf)
		{
			/*has topo info*/
#ifdef TCSUPPORT_EASYMESH_R13
			memset(tmp_result_buf, 0, sizeof(tmp_result_buf+1));
#else
			memset(tmp_result_buf, 0, topo_buf_len + 1);
#endif
			strJsonConvertHTML(topo_buf, tmp_result_buf, topo_buf_len);
			asprintf(result, JSON_RUNTIME_HASTOPO_FMT, JSON_SUCCESS_FMT, tmp_result_buf, topo_buf_len);
		}
		else
		{
			asprintf(result, JSON_DEF_FMT, JSON_COMMAND_FAILED_FMT);
		}
	}
	else
	{
		/*no topo info*/
		asprintf(result, JSON_DEF_FMT, JSON_COMMAND_FAILED_FMT);
	}
	/* Release Buf. */
	if(topo_buf)
	{
		free(topo_buf);
		topo_buf = NULL;
	}
	if(tmp_result_buf)
	{
		free(tmp_result_buf);
		tmp_result_buf = NULL;
	}
#ifdef TCSUPPORT_EASYMESH_R13
	}
#endif
	mapd_interface_ctrl_close(ctrl);
	return ret;
}

/*
*	Mesh get ap_Interface_List
*/
int read_interface(const char *itf_name, unsigned char *mac, int len)
{
	int sock = -1;
	struct ifreq ifbuf;
	unsigned char *if_mac = NULL;
	
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if ( sock < 0 ) 
	{
		return -1;
	}

	memset(&ifbuf, 0, sizeof(ifbuf));
	strncpy(ifbuf.ifr_name, itf_name, sizeof(ifbuf.ifr_name) - 1);
	if ( ioctl(sock, SIOCGIFHWADDR, &ifbuf) == -1 ) 
	{
		close(sock);
		return -1;
	}
	if_mac = (unsigned char *) ifbuf.ifr_hwaddr.sa_data;
	if(if_mac)
	{
		snprintf(mac, len, MAP_ADDR_FMT_STR, MAP_ADDR_FMT_ARS(if_mac));
	}
	
	close(sock);
	return 0;
}
static void get_apInfStr(char *mac, char *str, int len)
{
	int i = 0;
	char type[8]; 
	char buf[2048];
	char hwAddr[20];
	char cmd[64];
	char *hwAddrPtr = NULL;
	
	for(i = 0; i < MAX_INFO_LIST_NUM * 2; i++)
	{
		memset(type, 0, sizeof(type));
		if(i < MAX_INFO_LIST_NUM)
			snprintf(type, sizeof(type), "ra%d", i);
		else
			snprintf(type, sizeof(type), "rai%d", i % MAX_INFO_LIST_NUM);

		memset(hwAddr, 0, sizeof(hwAddr));
		read_interface(type, hwAddr, sizeof(hwAddr));
		if (!strcasecmp(hwAddr, mac))
		{
			snprintf(str, len, "%s", type);
			break;
		}
	}
}

static void get_apInfList(char *macList, char *strList, int strList_len)
{
	char str[8];
	char *port = NULL;
	int ret = 0;
	
	port = strtok(macList, ";");
	while(NULL != port)
	{
		memset(str, 0, sizeof(str));
		get_apInfStr(port, str, sizeof(str));
		ret += snprintf(strList + ret, strList_len - ret, "%s;", str);
		port = strtok(NULL, ";");
	}
}

/*
*	Mesh get json for ap_back-haul_infterface_list
*/
int mesh_get_ap_bh_inf_list_fun(asp_reent* reent, char **result)
{
	struct mapd_interface_ctrl *ctrl = NULL;
	char mac_buf[900], macList[1024], strList[128];
	size_t mac_buf_len = 900, strList_len = 128;
	int ret = 0;

	ctrl = mapd_interface_ctrl_open(MAPD_CTRL_FILE);
	if(!ctrl)
	{
		return -1;
	}
	
	memset(mac_buf, 0, sizeof(mac_buf));
	ret = mapd_interface_get_bh_ap(ctrl, mac_buf);

	if((-1 == ret) || (mac_buf_len > 900))
	{
		asprintf(result, JSON_DEF_FMT, JSON_COMMAND_FAILED_FMT);
	}
	else if(-2 == ret)
	{
		asprintf(result, JSON_DEF_FMT, JSON_COMMAND_TIMEDOUT_FMT);
	}
	else
	{
		memset(macList, 0, sizeof(macList));
		snprintf(macList, sizeof(macList), JSON_MACLIST_FMT, mac_buf);

		memset(strList, 0, sizeof(strList));
		get_apInfList(mac_buf, strList, strList_len);

		asprintf(result, JSON_AP_BACK_HAUL_INF_FMT, JSON_SUCCESS_FMT, macList, strList);
	}
	mapd_interface_ctrl_close(ctrl);	
	return ret; 
}

/*
*	Mesh get json for ap_font-haul_infterface_list
*/
int mesh_get_ap_fh_inf_list_fun(asp_reent* reent, char **result)
{
	struct mapd_interface_ctrl *ctrl = NULL;
	char mac_buf[900], macList[1024], strList[128];
	size_t mac_buf_len = 900, strList_len = 128;
	int ret = 0;

	ctrl = mapd_interface_ctrl_open(MAPD_CTRL_FILE);
	if(!ctrl)
	{
		return -1;
	}
	
	memset(mac_buf, 0, sizeof(mac_buf));
	ret = mapd_interface_get_fh_ap(ctrl, mac_buf);

	if((-1 == ret) || (mac_buf_len > 900))
	{
		asprintf(result, JSON_DEF_FMT, JSON_COMMAND_FAILED_FMT);
	}
	else if(-2 == ret)
	{
		asprintf(result, JSON_DEF_FMT, JSON_COMMAND_TIMEDOUT_FMT);
	}
	else
	{
		memset(macList, 0, sizeof(macList));
		snprintf(macList, sizeof(macList), JSON_MACLIST_FMT, mac_buf);

		memset(strList, 0, sizeof(strList));
		get_apInfList(mac_buf, strList, strList_len);

		asprintf(result, JSON_AP_FONT_HAUL_INF_FMT, JSON_SUCCESS_FMT, macList, strList);
	}
	mapd_interface_ctrl_close(ctrl);	
	return ret; 
}

/*
*	Mesh get json for ap_font-haul_infterface_list
*/
int mesh_get_client_capabilities_fun(asp_reent* reent, char **result)
{
	struct mapd_interface_ctrl *ctrl = NULL;	
	struct client_db *cli_dbs = NULL;
	int num_clis, ret_write, ret_cat, cli_idx, max_bw_idx, bit_idx;
	char cli_db_buf[4096], tmpBufStr[128], tmpStr[128], putStr[256];
	char *tmp_result = NULL;
	size_t cli_db_buf_len = sizeof(cli_db_buf);

	ctrl = mapd_interface_ctrl_open(MAPD_CTRL_FILE);
	if(!ctrl)
	{
		return -1;
	}

	memset(cli_db_buf, 0, sizeof(cli_db_buf));
	num_clis = mapd_interface_get_client_db(ctrl, (struct client_db*)cli_db_buf, &cli_db_buf_len);

	if((num_clis < 0) || (NULL == cli_db_buf) || (cli_db_buf_len <= 0) )
	{
		/*Command Failed*/
		asprintf(result, JSON_DEF_FMT, JSON_COMMAND_FAILED_FMT);
	}
	else if(0 == num_clis)
	{
		/*No Clients Available*/
		asprintf(result, JSON_DEF_FMT, JSON_NO_CLIENTS_AVALIABLE_FMT);
	}
	else if((num_clis > 0) && (cli_db_buf != NULL) && (cli_db_buf_len > 0))
	{
		tmp_result = (char*)malloc(TOPO_BUF_LEN+1);
		if(!tmp_result)
		{
			asprintf(result, JSON_DEF_FMT, JSON_COMMAND_FAILED_FMT);
			return 0;
		}
		ret_write = 0;
		memset(tmp_result, 0, TOPO_BUF_LEN);

		/*has cli_capab info*/
		memset(putStr, 0, sizeof(putStr));
		snprintf(putStr, sizeof(putStr), JSON_SUCCESS_FMT);
		ret_write += snprintf(tmp_result + ret_write , TOPO_BUF_LEN - ret_write, "%s,", putStr);
		
		memset(putStr, 0, sizeof(putStr));
		snprintf(putStr, sizeof(putStr), "\\\"num_clis\\\": \\\"%d\\\",", num_clis);
		ret_write += snprintf(tmp_result + ret_write , TOPO_BUF_LEN - ret_write, JSON_DEF_FMT, putStr);

		memset(putStr, 0, sizeof(putStr));
		snprintf(putStr, sizeof(putStr), "\\\"cli_db\\\": {");
		ret_write += snprintf(tmp_result + ret_write , TOPO_BUF_LEN - ret_write, JSON_DEF_FMT, putStr);
		
		cli_dbs = (struct client_db*)cli_db_buf;
		for(cli_idx=0; cli_idx < num_clis; cli_idx++)
		{
			/*clis_idx_info_start*/
			memset(tmpBufStr, 0, sizeof(tmpBufStr));
			if(0 == cli_idx)
			{
				snprintf(tmpBufStr, sizeof(tmpBufStr), "\\\"%d\\\": {", cli_idx);
			}
			else
			{
				snprintf(tmpBufStr, sizeof(tmpBufStr), ",\\\"%d\\\": {", cli_idx);
			}
			snprintf(putStr, sizeof(putStr), "%s", tmpBufStr);
			ret_write += snprintf(tmp_result + ret_write , TOPO_BUF_LEN - ret_write, JSON_DEF_FMT, putStr);

			/* Create child non-sequence table of size as per struct client_db */
			/*MAC Address*/
			memset(tmpBufStr, 0, sizeof(tmpBufStr));
			snprintf(tmpBufStr, sizeof(tmpBufStr), MAP_ADDR_FMT_STR, MAP_ADDR_FMT_ARS(cli_dbs[cli_idx].mac));
			memset(putStr, 0, sizeof(putStr));
			snprintf(putStr, sizeof(putStr), "\\\"MAC\\\":\\\"%s\\\",", tmpBufStr);
			ret_write += snprintf(tmp_result + ret_write , TOPO_BUF_LEN - ret_write, JSON_DEF_FMT, putStr);

			/*BSSID*/
			memset(tmpBufStr, 0, sizeof(tmpBufStr));
			snprintf(tmpBufStr, sizeof(tmpBufStr), MAP_ADDR_FMT_STR, MAP_ADDR_FMT_ARS(cli_dbs[cli_idx].bssid));
			memset(putStr, 0, sizeof(putStr));
			snprintf(putStr, sizeof(putStr), "\\\"BSSID\\\": \\\"%s\\\",", tmpBufStr);
			ret_write += snprintf(tmp_result + ret_write , TOPO_BUF_LEN - ret_write, JSON_DEF_FMT, putStr);

			/*Capability*/
			memset(tmpBufStr, 0, sizeof(tmpBufStr));
			tmpBufStr[0] = '\0';
			ret_cat = 0;
			for(bit_idx=0; bit_idx < 4; bit_idx++)
			{
				if(cli_dbs[cli_idx].capab & (1 << bit_idx))
				{
					switch(bit_idx)
					{
						case 0:
							ret_cat += snprintf(tmpBufStr + ret_cat, sizeof(tmpBufStr) - ret_cat, JSON_DEF_FMT, "DOT11K_SUPPORTED, ");
							break;
						case 1:
							ret_cat += snprintf(tmpBufStr + ret_cat, sizeof(tmpBufStr) - ret_cat, JSON_DEF_FMT, "DOT11V_SUPPORTED, ");
							break;
						case 2:
							ret_cat += snprintf(tmpBufStr + ret_cat, sizeof(tmpBufStr) - ret_cat, JSON_DEF_FMT, "DOT11R_SUPPORTED, ");
							break;
						case 3:
							ret_cat += snprintf(tmpBufStr + ret_cat, sizeof(tmpBufStr) - ret_cat, JSON_DEF_FMT, "MBO_SUPPORTED, ");
							break;
						default:
							break;
					}
				}
			}
			if('\0' == tmpBufStr[0])
			{
				memset(tmpBufStr, 0, sizeof(tmpBufStr));
				snprintf(tmpBufStr, sizeof(tmpBufStr), "N/A");
			}
			else
			{
				/*Remove last comma and space character*/
				if(strlen(tmpBufStr) > 2)
				{
					tmpBufStr[strlen(tmpBufStr) - 2] = '\0';
				}
				else
				{
					tmpBufStr[strlen(tmpBufStr)] = '\0';
				}
			}
			memset(putStr, 0, sizeof(putStr));
			snprintf(putStr, sizeof(putStr), "\\\"Capability\\\": \\\"%s\\\",", tmpBufStr);
			ret_write += snprintf(tmp_result + ret_write , TOPO_BUF_LEN - ret_write, JSON_DEF_FMT, putStr);

			/*Phy Mode*/
			memset(tmpBufStr, 0, sizeof(tmpBufStr));
			switch(cli_dbs[cli_idx].phy_mode)
			{
				case 0:
					snprintf(tmpBufStr, sizeof(tmpBufStr), "CCK");
					break;
				case 1:
					snprintf(tmpBufStr, sizeof(tmpBufStr), "OFDM");
					break;
				case 2:
					snprintf(tmpBufStr, sizeof(tmpBufStr), "HTMIX");
					break;
				case 3:
					snprintf(tmpBufStr, sizeof(tmpBufStr), "GREENFIELD");
					break;
				case 4:
					snprintf(tmpBufStr, sizeof(tmpBufStr), "VHT");
					break;
				default:
				snprintf(tmpBufStr, sizeof(tmpBufStr), "N/A");
			}
			memset(putStr, 0, sizeof(putStr));
			snprintf(putStr, sizeof(putStr), "\\\"Phy Mode\\\": \\\"%s\\\",", tmpBufStr);
			ret_write += snprintf(tmp_result + ret_write , TOPO_BUF_LEN - ret_write, JSON_DEF_FMT, putStr);

			/*Max. Bandwidth 0 and Max. Bandwidth 1*/
			memset(tmpBufStr, 0, sizeof(tmpBufStr));
			for(max_bw_idx=0; max_bw_idx < 2; max_bw_idx++)
			{
				switch(cli_dbs[cli_idx].max_bw[max_bw_idx])
				{
					case 0:
						snprintf(tmpBufStr, sizeof(tmpBufStr), "20");
						break;
					case 1:
						snprintf(tmpBufStr, sizeof(tmpBufStr), "40");
						break;
					case 2:
						snprintf(tmpBufStr, sizeof(tmpBufStr), "80");
						break;
					case 3:
						snprintf(tmpBufStr, sizeof(tmpBufStr), "160");
						break;
					case 4:
						snprintf(tmpBufStr, sizeof(tmpBufStr), "10");
						break;
					case 5:
						snprintf(tmpBufStr, sizeof(tmpBufStr), "5");
						break;
					case 6:
						snprintf(tmpBufStr, sizeof(tmpBufStr), "8080");
						break;
					default:
						snprintf(tmpBufStr, sizeof(tmpBufStr), "N/A");
				}
				memset(putStr, 0, sizeof(putStr));
				snprintf(putStr, sizeof(putStr), "\\\"Max. Bandwidth %d\\\": \\\"%s\\\",", max_bw_idx, tmpBufStr);
				ret_write += snprintf(tmp_result + ret_write , TOPO_BUF_LEN - ret_write, JSON_DEF_FMT, putStr);
			}

			/*Spatial Stream*/
			memset(tmpBufStr, 0, sizeof(tmpBufStr));
			snprintf(tmpBufStr, sizeof(tmpBufStr), "%d", cli_dbs[cli_idx].spatial_stream);
			memset(putStr, 0, sizeof(putStr));
			snprintf(putStr, sizeof(putStr), "\\\"Spatial Stream\\\": \\\"%s\\\",", tmpBufStr);
			ret_write += snprintf(tmp_result + ret_write , TOPO_BUF_LEN - ret_write, JSON_DEF_FMT, putStr);

			/*Known Band*/
			memset(tmpBufStr, 0, sizeof(tmpBufStr));
			tmpBufStr[0] = '\0';
			ret_cat = 0;
			for(bit_idx=0; bit_idx < 2; bit_idx++)
			{
				if(cli_dbs[cli_idx].know_band & (1 << bit_idx))
				{
					switch(bit_idx)
					{
						case 0:
							ret_cat += snprintf(tmpBufStr + ret_cat, sizeof(tmpBufStr) - ret_cat, JSON_DEF_FMT, "2GHz, ");
							break;
						case 1:
							ret_cat += snprintf(tmpBufStr + ret_cat, sizeof(tmpBufStr) - ret_cat, JSON_DEF_FMT, "5GHz, ");
							break;
						default:
							break;
					}
				}
			}
			if('\0' == tmpBufStr[0])
			{
				memset(tmpBufStr, 0, sizeof(tmpBufStr));
				snprintf(tmpBufStr, sizeof(tmpBufStr), "N/A");
			}
			else
			{
				/*Remove last comma and space character*/
				if(strlen(tmpBufStr) > 2)
				{
					tmpBufStr[strlen(tmpBufStr) - 2] = '\0';
				}
				else
				{
					tmpBufStr[strlen(tmpBufStr)] = '\0';
				}
			}
			memset(putStr, 0, sizeof(putStr));
			snprintf(putStr, sizeof(putStr), "\\\"Known Band\\\": \\\"%s\\\",", tmpBufStr);
			ret_write += snprintf(tmp_result + ret_write , TOPO_BUF_LEN - ret_write, JSON_DEF_FMT, putStr);

			/*Known Channels*/
			memset(tmpBufStr, 0, sizeof(tmpBufStr));
			tmpBufStr[0] = '\0';
			ret_cat = 0;
			for (bit_idx = 0; bit_idx < 38; bit_idx++) 
			{
				if (cli_dbs[cli_idx].know_channels[bit_idx / 8] & (1 << (bit_idx % 8))) 
				{
					if ((bit_idx >= 0) && (bit_idx <= 13))
					{
						snprintf(tmpStr, sizeof(tmpStr), "%d, ", bit_idx + 1);
						ret_cat += snprintf(tmpBufStr + ret_cat, sizeof(tmpBufStr) - ret_cat, JSON_DEF_FMT, tmpStr);
					}
					else if (bit_idx >= 14 && bit_idx <= 21)
					{
						snprintf(tmpStr, sizeof(tmpStr), "%d, ", 36 + ((bit_idx - 14) * 4));
						ret_cat += snprintf(tmpBufStr + ret_cat, sizeof(tmpBufStr) - ret_cat, JSON_DEF_FMT, tmpStr);
					}
					else if ((bit_idx >= 22) && (bit_idx <= 32))
					{
						snprintf(tmpStr, sizeof(tmpStr), "%d, ", 100 + ((bit_idx - 22) * 4));
						ret_cat += snprintf(tmpBufStr + ret_cat, sizeof(tmpBufStr) - ret_cat, JSON_DEF_FMT, tmpStr);
					}
					else if ((bit_idx >= 33) && (bit_idx <= 37))
					{
						snprintf(tmpStr, sizeof(tmpStr), "%d, ", 149 + ((bit_idx -33) * 4));
						ret_cat += snprintf(tmpBufStr + ret_cat, sizeof(tmpBufStr) - ret_cat, JSON_DEF_FMT, tmpStr);
					}
				}
			}
			if('\0' == tmpBufStr[0])
			{
				memset(tmpBufStr, 0, sizeof(tmpBufStr));
				snprintf(tmpBufStr, sizeof(tmpBufStr), "N/A");
			}
			else
			{
				/*Remove last comma and space character*/
				if(strlen(tmpBufStr) > 2)
				{
					tmpBufStr[strlen(tmpBufStr) - 2] = '\0';
				}
				else
				{
					tmpBufStr[strlen(tmpBufStr)] = '\0';
				}
			}
			memset(putStr, 0, sizeof(putStr));
			snprintf(putStr, sizeof(putStr), "\\\"Known Channels\\\": \\\"%s\\\"", tmpBufStr);
			ret_write += snprintf(tmp_result + ret_write , TOPO_BUF_LEN - ret_write, JSON_DEF_FMT, putStr);

			/*clis_idx_info_end*/
			memset(putStr, 0, sizeof(putStr));
			snprintf(putStr, sizeof(putStr), " } ");
			ret_write += snprintf(tmp_result + ret_write , TOPO_BUF_LEN - ret_write, JSON_DEF_FMT, putStr);
		}
		memset(putStr, 0, sizeof(putStr));
		snprintf(putStr, sizeof(putStr), " }  }");
		ret_write += snprintf(tmp_result + ret_write , TOPO_BUF_LEN - ret_write, JSON_DEF_FMT, putStr);

		/* set asp result */
		asprintf(result, JSON_DEF_FMT, tmp_result);
		
		/* Release Buf. */
		if(tmp_result)
		{
			free(tmp_result);
			tmp_result = NULL;
		}
	}
	else
	{
		asprintf(result, JSON_DEF_FMT, JSON_COMMAND_FAILED_FMT);
	}

	mapd_interface_ctrl_close(ctrl);	
	return 0; 
}

int mesh_get_sta_steering_progress_fun(asp_reent* reent, char **result)
{
	char sta_steering_info_str[128];
	memset(sta_steering_info_str, 0, sizeof(sta_steering_info_str));

	fileRead(STA_STEER_PROGRESS_FILE, sta_steering_info_str, sizeof(sta_steering_info_str));
	if('\0' == sta_steering_info_str[0])
	{
		/*Failed to open sta_steer_progress file*/
		asprintf(result, JSON_DEF_FMT, JSON_STA_STEER_PROGRESS_FAILED_FMT);
	}
	else
	{
		asprintf(result, JSON_STA_STEERING_INFO_FMT, sta_steering_info_str);
	}
	
	return 0;
}

static int get_sta_inf_connstatus(char *infname)
{
	int connStatus = 0, len = 0, ret = 0;
	char cmd[64];
	char *tmpbuf = NULL;
	char connInfo[128];
	FILE* fp = NULL;

	if(infname != NULL)
	{
		memset(cmd, 0, sizeof(cmd));
		snprintf(cmd, sizeof(cmd), CMD_GET_APCLI_CONNSTATUS_FMT, infname, STA_CONNSTATUS_FILE);
		system(cmd);
		
		fp = fopen(STA_CONNSTATUS_FILE, "r");
		if( NULL == fp )
		{
			unlink(MESH_INFO_FILE_CMD);
			return -1;
		}
		
		if((getline(&tmpbuf, &len, fp)) != -1)
		{
			memset(connInfo,0, sizeof(connInfo));
			sscanf(tmpbuf,"%*[^:]:%[^ ]", connInfo);
			if(strlen(connInfo) > 2)
			{
				connStatus = 1;
			}
			else
			{
				connStatus = 0;
			}
		}
		
		if(NULL != tmpbuf)
		{
			free(tmpbuf);
			tmpbuf = NULL;
		}
		
		if(NULL != fp)
		{
			fclose(fp);
			fp = NULL;
		}
	}
	unlink(MESH_INFO_FILE_CMD);
	
	return connStatus;
}

static void get_connected_sta_inf(char *out_infname, int len)
{
	int connStatus = 0, i = 0;
	char infname[32];

	connStatus = 0;
	memset(infname, 0, sizeof(infname));
	for(i = 0; i < MAX_APCLIENT_NUM; i++)
	{
		snprintf(infname, sizeof(infname), M_APCLIENT_2G_NAME_FORMAT, i);
		connStatus = get_sta_inf_connstatus(infname);
		if(1 == connStatus)
		{
			snprintf(out_infname, len, "%s", infname);
			return;
		}
	}

	connStatus = 0;
	memset(infname, 0, sizeof(infname));
	for(i = 0; i < MAX_APCLIENT_NUM; i++)
	{
		snprintf(infname, sizeof(infname), M_APCLIENT_5G_NAME_FORMAT, i);
		connStatus = get_sta_inf_connstatus(infname);
		if(1 == connStatus)
		{
			snprintf(out_infname, len, "%s", infname);
			return;
		}
	}

	return;	
}

int mesh_get_sta_bh_interface_fun(asp_reent* reent, char **result)
{
	char infname[32];
	char status_str[128];
	char staBhInfStr_str[128];
	char profile_str[128];
	
	memset(infname, 0, sizeof(infname));
	memset(status_str, 0, sizeof(status_str));
	memset(staBhInfStr_str, 0, sizeof(staBhInfStr_str));
	memset(profile_str, 0, sizeof(profile_str));
	
	get_connected_sta_inf(infname, sizeof(infname));
	if(strlen(infname))
	{
		/*apcli Connected*/
		snprintf(status_str, sizeof(status_str), JSON_DEF_FMT, JSON_SUCCESS_FMT);
		snprintf(staBhInfStr_str, sizeof(staBhInfStr_str), JSON_STA_BHINFSTR_FMT, infname);
		if(strstr(infname, "apcli") != NULL)
		{
			snprintf(profile_str, sizeof(profile_str), JSON_STA_PROFILE_FMT, WLAN_PATH);
		}
		else if(strstr(infname, "apclii") != NULL)
		{
			snprintf(profile_str, sizeof(profile_str), JSON_STA_PROFILE_FMT, WLAN_AC_PATH);
		}
		else
		{
			snprintf(profile_str, sizeof(profile_str), JSON_STA_PROFILE_FMT, "");
		}
	}
	else
	{
		/*apcli Disconnected*/
		snprintf(status_str, sizeof(status_str), JSON_DEF_FMT, JSON_SUCCESS_FMT);
		snprintf(staBhInfStr_str, sizeof(staBhInfStr_str), JSON_STA_BHINFSTR_FMT, "");
		snprintf(profile_str, sizeof(profile_str), JSON_STA_PROFILE_FMT, "");
	}
	asprintf(result, "%s,%s,%s", status_str, staBhInfStr_str, profile_str);
	
	return 0;
}

