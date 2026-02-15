/*	Copyright(c) 2010-2011 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 * file	os_msg.h
 * brief	The msg lib declarations.
 * note		1. don't use macro to control the MSG_CONTENT_SIZE, modify it directly if necessary
 * 			2. when it is necessary to add some members to the xxx_MSG struct, don't use macro to control them
 * author	Yang Xv
 * version	1.0.0
 * date	28Apr11
 *
 * history	\arg 1.0.0, 28Apr11, Yang Xv, Create the file.
 */

#ifndef __OS_MSG_H__
#define __OS_MSG_H__

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */


/* ע�����ͷ�ļ���Ҫ����<cstd.h>ͷ�ļ� */
#include <unistd.h>

#include <time.h>

/* do not include <netinet/in.h> */

#include <stdint.h> //Used for uint8_t etc.
#ifdef INCLUDE_RECORD_UPGRADE_INFO
#include <upgrade_info.h>
#endif /*INCLUDE_RECORD_UPGRADE_INFO*/

/**************************************************************************************************/
/*											 DEFINES											  */
/**************************************************************************************************/

#ifndef GNU_PACKED
#define GNU_PACKED	__attribute__ ((packed))
#endif /* GNU_PACKED */

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

/* don't use macro to control the MSG_CONTENT_SIZE, modify it directly if necessary */
#define MSG_CONTENT_SIZE	1024*4


/* for all message type */

#define SNTP_DM_SERVER_NUM	5

/* 
 * brief status of DHCP6C_INFO_MSG_BODY
 */
#ifdef INCLUDE_IPV6
#define DHCP6C_ASSIGNED_DNS 	0x1
#define DHCP6C_ASSIGNED_ADDR	0x2
#define DHCP6C_ASSIGNED_PREFIX	0x4
#define DHCP6C_ASSIGNED_DSLITE_NAME	0x8  /* Add by YuanRui: support DS-Lite, 21Mar12 */
#ifdef INCLUDE_IPV6_MAP
#define DHCP6C_ASSIGNED_MAPT		0x10	
#endif /* INCLUDE_IPV6_MAP	*/

#define DHCP6C_ASSIGNED_DOMAIN		0x20
#define DHCP6C_ASSIGNED_SNTP		0x40

#define IPV6_INFO_DEL 0
#define IPV6_INFO_ADD 1
#define IPV6_INFO_UPDATE 2

#endif	/* INCLUDE_IPV6 */

#ifdef INCLUDE_CLOUD

#define OS_PATH_PREFIX "/var/tmp/"

#endif

/* 
 * brief Max path length, should smaller than sockaddr_un.sun_path length	
 */
#define MAX_PATH_LEN	64
#define MAX_FILE_NAME_LEN 256

#ifdef INCLUDE_EASYMESH
#define MAX_TS_POLICY (14 * 2)
#endif

#ifdef INCLUDE_EASYMESH_MULTI_UPGRADE
#define MSG_PATH_SIZE (32)
#define MU_BUFFLEN (700)
#define MAX_AGENT (16)
#endif /*INCLUDE_EASYMESH_MULTI_UPGRADE*/

#ifdef INCLUDE_PKTCAP_UPLOAD
#define FILE_MAX_PATH 256
#define URL_MAX_PATH 256
#define HOST_MAX_PATH 64
#define UPLOAD_FTP "ftp"
#define UPLOAD_TFTP "tftp"
#endif /* INCLUDE_PKTCAP_UPLOAD */

#ifdef INCLUDE_SAVE_KEY_AS_HASH
#define HASHKEY_SALT_LEN (48)
#endif /* INCLUDE_SAVE_KEY_AS_HASH */

#ifdef INCLUDE_DECT
#define REG_STATUS_CODE 100
#endif

#ifdef INCLUDE_SMART_HOME_V2
#define SMART_HOME_TIME_LEN					32
#define SMART_HOME_DU_FILE_PATH_LEN			256
#define SMART_HOME_FAULT_STRING_LEN			256
#define SMART_HOME_DU_FULLPATH_LEN			128
#define SMART_HOME_EU_CMD_LEN				128

#endif /* INCLUDE_SMART_HOME_V2 */

/**************************************************************************************************/
/*											 TYPES												  */
/**************************************************************************************************/
typedef unsigned char MAC_ADDR[6];

#ifdef __LINUX_OS_FC__
#include <sys/un.h>

typedef struct
{
	int fd;
	struct sockaddr_un _localAddr;
	struct sockaddr_un _remoteAddr;
}CMSG_FD;
#endif	/* __LINUX_OS_FC__ */

#ifdef __VXWORKS_OS_FC__
typedef struct
{
	int fd;
}CMSG_FD;
#endif /* __VXWORKS_OS_FC__ */


/* 
 * brief	Enumeration message type
 *			Convention:
 *			System message types - 1000 ~ 1999
 *			Common application message types - 2000 ~ 2999
 */
typedef enum
/* ���һ����Ϣֻ��һ��UINT32�����ݣ���ô����ʡ�Ը�message type��Ӧ�Ľṹ�壬
 * ֻʹ��CMSG_BUFF�ṹ�е�priv��Ա����
 */
{
	CMSG_NULL = 0,
	CMSG_REPLY = 1,
	CMSG_LOG = 2000,
	CMSG_SNTP_CFG = 2001,
	CMSG_SNTP_STATUS = 2002,	/* only have one word value */
	CMSG_SNTP_START = 2003,

	CMSG_DNS_PROXY_CFG = 2004,
	CMSG_DNS_SERVER	= 2005,		/* only have one word value */
	CMSG_PPP_STATUS = 2006,

#ifdef INCLUDE_TR111_PART1
	CMSG_DHCPS_ADD_TR111DEV = 2007,/*Add by YuChuwei*/
#endif	/* INCLUDE_TR111_PART1 */
#ifdef INCLUDE_TR111_PART1
	CMSG_DHCPS_DEL_TR111DEV = 2008, /*Add by YuChuwei*/
#endif	/* INCLUDE_TR111_PART1 */

	/* Added by xcl, 2011-06-13.*/
	CMSG_DHCPS_RELOAD_CFG = 2010, 
	CMSG_DHCPS_WRITE_LEASE_TO_SHM = 2011,
	CMSG_DHCPS_WAN_STATUS_CHANGE = 2012,
	CMSG_DHCPC_STATUS = 2013,
	CMSG_DHCPC_START = 2014,
	CMSG_DHCPC_RENEW = 2015, 
	CMSG_DHCPC_RELEASE = 2016,
	CMSG_DHCPC_SHUTDOWN = 2017,
	CMSG_DHCPC_MAC_CLONE = 2018,
#ifdef INCLUDE_SMART_DHCP
	CMSG_DHCPC_DETECT_CFG = 2019,
#endif /* INCLUDE_SMART_DHCP */
	/* End added by xcl, 2011-06-13.*/

	CMSG_DDNS_PH_RT_CHANGED = 2020,
	CMSG_DDNS_PH_CFG_CHANGED = 2021,
	CMSG_DDNS_PH_GET_RT	= 2022,
	/* Added by xcl, for dyndns, 24Nov11 */
	CMSG_DYNDNS_RT_CHANGED = 2023,
	CMSG_DYNDNS_CFG_CHANGED = 2024,
	CMSG_DYNDNS_STATE_CHANGED = 2025,
	/* Added by tpj, for noipdns, 17Jan12 */
	CMSG_NOIPDNS_RT_CHANGED = 2026,
	CMSG_NOIPDNS_CFG_CHANGED = 2027,
	CMSG_NOIPDNS_STATE_CHANGED = 2028,
	/*end (n & n)*/

	CMSG_HTTPD_SERVICE_CFG = 2030,
	CMSG_HTTPD_USER_CFG = 2031,
	/* CMSG_HTTPD_HOST_CFG = 2032, */
	CMSG_HTTPD_CHALLENGE_CFG = 2033,
#ifdef INCLUDE_HTTP_SSL
	CMSG_HTTPD_CERT_UPDATE = 2034,
#endif /*INCLUDE_HTTP_SSL*/
	CMSG_HTTPD_LOGOUT_ALL_USERS = 2035,

#ifdef INCLUDE_USER_RESTRICTION
	CMSG_HTTPD_USER_PAGE_CONFIG_CHANGED = 2036,
  #ifdef INCLUDE_ACCESS_CTRL_EXT
	CMSG_HTTPD_USER_ACCESS_RIGHT_CHANGED = 2037,
  #endif /*INCLUDE_ACCESS_CTRL_EXT*/
#endif /*INCLUDE_USER_RESTRICTION*/
#ifdef INCLUDE_CLOUD_V2
	CMSG_TPLINKDNS_CFG_CHANGED = 2039,
#endif /*INCLUDE_CLOUD_V2*/

	CMSG_CLI_USRCFG_UPDATE	= 2040,

	/* Added by zj, for userdefined ddns, 28Jan14 */
	CMSG_DDNS_UD_RT_CHANGED = 2041,
	CMSG_DDNS_UD_CFG_CHANGED = 2042,
	CMSG_DDNS_UD_STATE_CHANGED = 2043,
#ifdef INCLUDE_PORTABLE_APP
	CMSG_PORTABLE_APP_USRCFG_UPDATE = 2045,
	CMSG_PORTABLE_APP_WLANCFG_UPDATE = 2046,
	CMSG_PORTABLE_APP_LOGOUT_ALL_USERS = 2047,
	CMSG_PORTABLE_APP_OWNERINFO_UPDATE = 2048,
#endif /* INCLUDE_PORTABLE_APP */

	/* Added  by  Li Chenglong , 11-Jul-31.*/
	CMSG_UPNP_ENABLE = 2050,
#ifdef INCLUDE_UPNP_PORTMAP_SWITCH
	CMSG_UPNP_PORTMAPPING_ENABLE = 2051,
#endif /*INCLUDE_UPNP_PORTMAP_SWITCH*/
	CMSG_DEFAULT_GW_CH = 2052,
	/* Ended  by  Li Chenglong , 11-Jul-31.*/

	/* Add by chz, 2012-12-24 */
	CMSG_UPNP_DEL_ENTRY = 2053,
	/* end add */
	/* Add by Chen Zexian, 20130116 */
	CMSG_UPNP_LAN_IP_CH = 2054,
	/* End of add */

#ifdef INCLUDE_MESH_TETHER_WEBVIEW
	CMSG_HTTPD_CREATE_APP_USER = 2055,	//2039
#endif /* INCLUDE_MESH_TETHER_WEBVIEW */

#ifdef INCLUDE_SMART_DHCP
	CMSG_SMART_DHCP_CFG = 2056,
	CMSG_SMART_DHCP_MESH_NOTIFY = 2057,
	CMSG_SMART_DHCP_LINK_NOTIFY = 2058,
#endif /* INCLUDE_SMART_DHCP */
#ifdef INCLUDE_AVOID_UPNP_USE_CWMP_PORT
	CMSG_UPNP_CWMP_PORT_CHANGE = 2059,
#endif /*INCLUDE_AVOID_UPNP_USE_CWMP_PORT*/

	/* added by Yuan Shang for diagTool, 2011-08-18 */
	CMSG_DIAG_TOOL_COMMAND = 2060,

	/* added by wuzhiqin, 2011-09-26 */
	CMSG_CWMP_CFG_UPDATE = 2070, 
	CMSG_CWMP_PARAM_VAL_CHANGED = 2071,
	CMSG_CWMP_WAN_CONNECTION_UP = 2072,
	CMSG_CWMP_TIMER = 2073,
	CMSG_CWMP_WAN_CONNECTION_DOWN = 2074,
	CMSG_CWMP_WAN_DNS_CHG = 2075,
	/* ended by wuzhiqin, 2011-09-26 */
#ifdef INCLUDE_EASYDIAG
	CMSG_CWMP_GET_CPE_STATE = 2076,
#endif /* INCLUDE_EASYDIAG */

#ifdef INCLUDE_DSL_DIAGNOSTICS
	CMSG_CWMP_DIAGNOSTIC_COMPLETED = 2077,
#endif /* INCLUDE_DSL_DIAGNOSTICS */

	/* Added by GuoChubin 2016-04. */
#ifdef INCLUDE_TR143 
	CMSG_DIAG_START = 2079,
#endif /* INCLUDE_TR143 */
	CMSG_DIAG_COMPLETE = 2080, /* Generic */
#ifdef INCLUDE_TR143
#ifdef INCLUDE_IPPING_DIAG		  
	CMSG_IPPING_CFG_MSG = 2081,
#endif /* INCLUDE_IPPING_DIAG */
#ifdef INCLUDE_TRACEROUTE_DIAG
	CMSG_TRACEROUTE_CFG_MSG = 2082,
#endif /* INCLUDE_TRACEROUTE_DIAG */
#ifdef INCLUDE_DOWNLOAD_DIAG
	CMSG_DOWNLOAD_CFG_MSG = 2083,
#endif /* INCLUDE_DOWNLOAD_DIAG */
#ifdef INCLUDE_UPLOAD_DIAG
	CMSG_UPLOAD_CFG_MSG = 2084,
#endif /* INCLUDE_UPLOAD_DIAG */
#ifdef INCLUDE_UDPECHO_DIAG
	CMSG_UDPECHO_CFG_MSG = 2085,
#endif /* INCLUDE_UDPECHO_DIAG */
#ifdef INCLUDE_NSLOOKUP_DIAG
	CMSG_NSLOOKUP_CFG_MSG = 2086,
#endif
#ifdef INCLUDE_TR143_V1_1_0
	CMSG_DIAG_LOAD_MULTI  = 2087,
#endif

#endif /* INCLUDE_TR143 */

	/* added by Wang Wenhao for IGMPv3 Proxy, 2011-11-24 */
	CMSG_IGMPD_ADD_LAN_IF = 2100,
	CMSG_IGMPD_ADD_WAN_IF = 2101,
	CMSG_IGMPD_DEL_IF = 2102,
	/* add by chen yingtao for omci -- igmp, 2014-10-09 */
	CMSG_IGMPD_OMCI_RESET = 2103,
	CMSG_IGMPD_OMCI_RELOAD = 2104,
	CMSG_IGMPD_CHG_VERSION = 2105,
#ifdef INCLUDE_TRTC
	CMSG_IGMPD_CHG_RTCONF = 2106,
#endif
	/* Added by LI CHENGLONG , 2011-Dec-15.*/
	CMSG_DLNA_MEDIA_SERVER_INIT = 2110,
	CMSG_DLNA_MEDIA_SERVER_MANUAL_SCAN = 2111,
	CMSG_DLNA_MEDIA_SERVER_OP_FOLDER = 2112,
	CMSG_DLNA_MEDIA_SERVER_RELOAD = 2113,
	CMSG_DLNA_MEDIA_DEVICE_STATE_CHANGE = 2114,
	/* Ended by LI CHENGLONG , 2011-Dec-15.*/

	/* Added by jinshuaichen for login role, 2022-06-22*/
	CMSG_ACCOUNT_LOGIN = 2120,

	CMSG_LOGIN_REMOTE = 2121,

#ifdef INCLUDE_ONLINE_DETECTION
	CMSG_ONLINE_DETECTION_MSG = 2131,
#endif /* INCLUDE_ONLINE_DETECTION */

	CMSG_USB_PRINTER_HANDLE_EVENT = 2200,

#ifdef INCLUDE_WIFI
	CMSG_WPS_CFG = 2700,
	CMSG_WLAN_SWITCH = 2701,
	CMSG_WPS_PIN_LOCK = 2702,
	CMSG_WLAN_MODE_SWITCH = 2703,
#ifdef INCLUDE_EASYMESH
	CMSG_WLAN_ASSOC_PRI_VLAN = 2704,
#endif /* INCLUDE_EASYMESH */
	CMSG_WLAN_START_WPS_PBC = 2705,
#endif /* INCLUDE_WIFI */

	/* 
	 * for voice process. added by zhonglianbo 2011-8-10
	 */
#ifdef INCLUDE_VOIP
	CMSG_VOIP_CMCNXCOUNT			= 2800,
	CMSG_VOIP_WAN_STS_CHANGED		= 2801,	/* WAN status changed
											 * priv means RSL_VOIP_INTF_STSCODE
											 * content means IP address
											 */
	CMSG_VOIP_CONFIG_CHANGED		= 2802,
	CMSG_VOIP_RESTART_CALLMGR		= 2803,
	CMSG_VOIP_CONFIG_WRITTEN		= 2804,	/* notice VoIP CM writing flash */
	CMSG_VOIP_FLASH_WRITTEN 		= 2805,	/* notice VoIP CM that flash has been written */
	CMSG_VOIP_STATISTICS_RESET		= 2806, 
#ifdef INCLUDE_USB_VOICEMAIL	
	CMSG_VOIP_UVM_USING_RECORDED_FILE	= 2807,		/* is USB voicemail using the recored file ? */
	CMSG_VOIP_USB_MOUNT_NEW = 2808,				/* A new disk that mounted can be used by usbvm  */
	CMSG_VOIP_USB_UMOUNT_CHANGE = 2809,			/* Change to another disk's path that is effective*/
	CMSG_VOIP_USB_UMOUNT_NULL = 2810,			/* There is not any disk can be used by usbvm  */
#endif /* INCLUDE_USB_VOICEMAIL */												 
	CMSG_VOIP_CALLLOG_CLEAR = 2811, 			/* Clear call log */
	CMSG_VOIP_RING = 2812,
	CMSG_VOIP_STATISTICS_QUERY = 2815,			/* Get VOICE_PROF_LINE_STATS Info */
#ifdef INCLUDE_VOICEAPP
	CMSG_VOICEAPP_BASESTATION_UPDATE = 2820,
	CMSG_VOICEAPP_SESSIONCFG_UPDATE = 2821,
	CMSG_VOICEAPP_PHONENAME_UPDATE = 2822,
#endif /* INCLUDE_VOICEAPP */
	CMSG_FXS_SET_PIN_PUK			= 2823,
#ifdef INCLUDE_DECT
	//CMSG_VOIP_USBMAIL_UNREAD_COUNT	= 2824,   // redefined as below.
#endif /* INCLUDE_DECT */
#endif /* INCLUDE_VOIP */
	/* end of voice process */

	/* Added by xcl, 21Sep11 */
	CMSG_SNMP_CFG_CHANGED = 2850,
	CMSG_SNMP_LINK_UP	  = 2851,
	CMSG_SNMP_LINK_DOWN   = 2852,
	CMSG_SNMP_WAN_UP	  = 2853,
	/* End added by xcl, 21Sep11 */

#ifdef INCLUDE_IPV6	/* Add by CM, 16Nov11 */
	CMSG_IPV6_PPP_STATUS		= 2900,
	CMSG_IPV6_DHCP6C_STATUS		= 2901,
#ifdef INCLUDE_IPV6_MAP
	CMSG_IPV6_MAP_STATUS		= 2902,
#endif /* INCLUDE_IPV6_MAP */
#ifdef INCLUDE_IPV6_MLD	/* Add by HYY: MLDv2 Proxy, 10Jul13 */
	CMSG_MLDPROXY_ADD_LAN_IF	= 2903,
	CMSG_MLDPROXY_ADD_WAN_IF	= 2904,
	CMSG_MLDPROXY_DEL_IF		= 2905,
#endif /* INCLUDE_IPV6_MLD */

#ifdef INCLUDE_DNS_PROXY
	CMSG_DNS6_PROBE_CFG = 2950,
	CMSG_DNS6_SERVER = 2951,
#endif	/* INCLUDE_DNS_PROXY */
#endif	/* INCLUDE_IPV6 */

#ifdef INCLUDE_IPSEC
	CMSG_IPSEC_CFG_CHANGED = 3000,
	CMSG_IPSEC_WAN_CHANGED = 3001,
#endif /* INCLUDE_IPSEC */

/*Code transplanting by ljn from Wang Yang for DHCP Option66 2017.8.15*/
#ifdef INCLUDE_OPTION66
	CMSG_OPTION66 = 3004,
#endif /* INCLUDE_OPTION66 */

#ifdef INCLUDE_OPENVPN_SERVER /* added by CCy, 27Jul2015 */
	CMSG_OVPN_STATUS = 3005,
#endif /*INCLUDE_OPENVPN_SERVER*/

#ifdef INCLUDE_USB_3G_DONGLE
	CMSG_USB_3G_HANDLE_EVENT = 3100,
#endif	/* INCLUDE_USB_3G_DONGLE */

	/* added by meizaihong, 2016-12-19*/
#ifdef INCLUDE_VOIP
	CMSG_VOIP_USBMAIL_UNREAD_COUNT		= 4000,
#endif /* INCLUDE_VOIP */

#ifdef INCLUDE_HW_QOS
	CMSG_HW_QOS_LINK_STATE_CHANGED		= 4100,
#endif

#ifdef INCLUDE_QUALCOMM
	CMSG_WPS_STATUS_CHANGED = 4200,
#endif

#ifdef INCLUDE_LTEWAN
CMSG_MOBILE_LTE_DISCONNECTED = 4300,
#endif

#ifdef INCLUDE_WEB_WARN
	CMSG_WEBWARN_EWAN_STATUS_CHANGE = 4400,
	CMSG_WEBWARN_DSL_STATUS_CHANGE = 4401,
	CMSG_WEBWARN_3G_STATUS_CHANGE = 4402,
 #ifdef INCLUDE_WEB_WARN_WIFI_NOSEC
	CMSG_WEBWARN_WIFI_SECURITY_CHANGE = 4403,
   #ifdef INCLUDE_WIFI_BANDSTEERING
	CMSG_WEBWARN_WIFI_BANDSTEERING_CHANGE = 4404,
   #endif /* INCLUDE_WIFI_BANDSTEERING */
 #endif /* INCLUDE_WEB_WARN_WIFI_NOSEC */
	CMSG_WEBWARN_RESET_STATUS = 4405,
#endif /* INCLUDE_WEB_WARN */

#ifdef INCLUDE_ONEMESH
/*add by liaoyilu 2020-6-9 for onemesh*/
	CMSG_ONEMESH_LIST_DEVICES = 5001,
	CMSG_ONEMESH_LIST_AVAILABLE_DEVICES = 5002,
	CMSG_ONEMESH_CLEAN_DEVICE = 5003,
	CMSG_ONEMESH_PROBE = 5004,
	CMSG_ONEMESH_ADD_SLAVE = 5005,
	CMSG_ONEMESH_ATTACH_MASTER = 5006,
	CMSG_ONEMESH_AUTO_ATTACH_MASTER = 5007,
	CMSG_ONEMESH_SYNC_WIFI = 5008,
	CMSG_ONEMESH_SET_RSA = 5009,
	CMSG_ONEMESH_REMOVE_SLAVE = 5010,
	CMSG_ONEMESH_SET_JOINED = 5011,
	CMSG_ONEMESH_SYNC_DAEMON = 5012,
#endif /*INCLUDE_ONEMESH*/
#ifdef INCLUDE_MESH_TETHER_WEBVIEW
	CMSG_MESH_WEBVIEW_PROVISION	=5020,
#endif /* INCLUDE_MESH_TETHER_WEBVIEW */

#ifdef INCLUDE_EASYMESH
	// add by liaoyilu 2020-12-28 for easymesh 
	CMSG_EASYMESH_AUTOCONFIG_SEND = 5050,
	CMSG_EASYMESH_AUTOCONFIG_RECV = 5051,
	CMSG_EASYMESH_CHANNELSELECTION_SEND = 5052,
	CMSG_EASYMESH_CHANNELSELECTION_RECV = 5053,
	CMSG_EASYMESH_WORKMODE_CHANGED = 5054,
	CMSG_EASYMESH_REMOVE_APDEV_SEND = 5055,
	CMSG_EASYMESH_REMOVE_APDEV_RECV = 5056,
	CMSG_EASYMESH_BACKHAULSTA_SET= 5057,
	CMSG_EASYMESH_TP_VENDORCONF_RENEW  = 5058,
	CMSG_EASYMESH_TP_VENDORCONF_RSP = 5059,
	CMSG_EASYMESH_MAP_RECONFIGURE = 5060,
	CMSG_EASYMESH_CONTROLLER_FIXED = 5061,
	CMSG_EASYMESH_TOPOLOGY_REFRESH = 5062,
	CMSG_EASYMESH_AUTO_ADAPT_CHANSPEC = 5063,
	CMSG_EASYMESH_AGNET_CONFIG_RECV = 5064,
#ifdef INCLUDE_EASYMESH_DEVICE_ONBOARDED
	CMSG_EASYMESH_ADD_APDEV = 5065,
#endif /* INCLUDE_EASYMESH_DEVICE_ONBOARDED */

#if defined(INCLUDE_EASYMESH_CERTIFICATION)
	CMSG_EASYMESH_CERT_RESTART_MAP = 5066,
#endif //INCLUDE_EASYMESH_CERTIFICATION
	CMSG_EASYMESH_MULTIAP_POLICY_RECV = 5067,
	CMSG_EASYMESH_REBOOT_APDEV_SEND = 5068,
	CMSG_EASYMESH_REBOOT_APDEV_RECV = 5069,

	//APSD
	CMSG_EASYMESH_APSD_BACKHAUL_CHANGED = 5070,
	CMSG_EASYMESH_APSD_RECONFIGURE = 5071,

#if 1//def INCLUDE_AI_ROAMING
	CMSG_AI_ROAMING_INFO_SEND = 5072,
	CMSG_AI_ROAMING_INFO_RECV = 5073,
#endif /* INCLUDE_AI_ROAMING */

	CMSG_EASYMESH_MAP_UPDATE_INTF = 5074,

#ifdef INCLUDE_EASYMESH_TP_ONBOARDING_ZERO_TOUCH
	CMSG_EASYMESH_AGENT_CONFIGED = 5075,
	CMSG_EASYMESH_DHCP_IP_BIND = 5076,
	CMSG_EASYMESH_CHANGE_TO_CONTROLLER = 5077,
#endif /* INCLUDE_EASYMESH_TP_ONBOARDING_ZERO_TOUCH */

	CMSG_EASYMESH_TRIGGER_WPS_PBC = 5078,
	CMSG_EASYMESH_CONFIG_SET_DONE = 5079,

	/* backhaul optimization */
	CMSG_EASYMESH_TOPOLOGY_CHANGE = 5080,
	CMSG_EASYMESH_LINKMETRICS_START = 5081,
	CMSG_EASYMESH_LINKMETRICS_RESULT = 5082,
	CMSG_EASYMESH_STASTEERING_REQ = 5083,
	CMSG_EASYMESH_STASTEERING_ACK = 5084,
	CMSG_EASYMESH_BHOPT_RELOADCONFIG = 5085,
	CMSG_EASYMESH_UPDATE_MULTI_BHLINKS = 5086,
	CMSG_EASYMESH_SAVE_BEST_TOPOLOGY = 5087,
	CMSG_EASYMESH_BHOPT_ALGO_TEST = 5088,

#ifdef INCLUDE_WIFI_BCM
	CMSG_EASYMESH_BACKHAUL_CHANGED = 5090,
#endif

#ifdef INCLUDE_EASYMESH_MULTI_UPGRADE
    /* MU is EasyMesh Upgrade */
    /* --------------------  Controller Inner Msg Start ----------------------- */
    CMSG_MU_START_UPGRADE = 5100, // Controller: cloud_client -> cos -> meshMonitor -> mapController, Use MU_TRIGGER_MSG
    CMSG_MU_REQ_DOWN_FIRM = 5101,  // Controller: meshMonitor -> mapController, Use MU_MSG
    CMSG_MU_REQ_CHECK_FIRM_PROGRESS = 5102,// Controller: meshMonitor -> mapController, Use MU_MSG
    CMSG_MU_REQ_DISPATCH = 5103,// Controller: meshMonitor -> mapController, Use MU_MSG
    CMSG_MU_REQ_RECEIVE = 5104,// Controller: meshMonitor -> mapController, Use MU_MSG
    CMSG_MU_REQ_UPGRADE = 5105,// Controller: meshMonitor -> mapController, Use MU_MSG
    CMSG_MU_RSP = 5106, // Controller: mapController -> meshMonitor, Include response progress/upgrade result/ack, Use MU_MSG
    CMSG_MU_RSP_AUTH_KEY = 5108, // Controller: mapController -> meshMonitor -> cos -> httpd, Use MU_HTTP_AUTH_MSG
    //CMSG_MU_SEND_ACK = ,// Controller: meshMonitor -> mapController,  Use MU_MSG, Current no need

    // Controller: meshMonitor -> cos, to save dm data, Use MU_SYNC_RESULT_MSG to transfer data.
    CMSG_MU_SYNC_MESH_UPGRADE_STATUS = 5130, 
    CMSG_MU_SYNC_MESH_UPGRADE_PROGRESS = 5131, 
#ifdef INCLUDE_RECORD_UPGRADE_INFO
    // Controller: AgentD -> cos, to save X_TP_Upgrade_Status, Use MU_SYNC_UPGRADE_STATUS_MSG to transfer data.
    CMSG_MU_SYNC_X_TP_UPGRADE_STATUS = 5132, 
#endif /*INCLUDE_RECORD_UPGRADE_INFO*/
    CMSG_MU_PARSE_EVENT = 5133,
    CMSG_MU_CLEAN_UPGRADE_STATUS = 5134,

    /* --------------------  Controller Inner Msg End ----------------------- */

    /* --------------------    Agent Inner Msg Start   ----------------------- */
    CMSG_MU_AGENT_REQ_DOWN_FIRM = 5150, // Agent: mapAgent -> meshMonitor -> cos -> cloud_client, Use MU_AGENT_DOWN_FIRM_MSG
    CMSG_MU_AGENT_FW_DOWN_RSP = 5151, // Agent: cloud_client -> cos -> meshMonitor -> mapAgent, Use MU_AGENT_FW_DOWN_RSP_MSG
    CMSG_MU_AGENT_FW_DOWNLOAD_RESULT = 5152, // Agent: cloud_client -> cos -> meshMonitor -> mapAgent, Use MU_AGENT_FW_DOWN_RESULT_MSG
    CMSG_MU_AGENT_CHECK_DOWN_PROGRESS = 5153, // Agent: mapAgent -> meshMonitor -> cos -> cloud_client, No need use msg
    CMSG_MU_AGENT_RSP_PROGRESS = 5154, // Agent: mapAgent -> meshMonitor -> cos -> cloud_client, Use MU_AGENT_RSP_PROGRESS_MSG
    CMSG_MU_AGENT_REQ_DISPATCH = 5155, // Agent: mapAgent -> meshMonitor, Use MU_AGENT_REQ_DISPATCH_MSG
    CMSG_MU_AGENT_REQ_RECEIVE = 5156, // Agent: mapAgent -> meshMonitor, Use MU_AGENT_REQ_RECEIVE_MSG
    CMSG_MU_AGENT_RSP_SRVPORT = 5157, // Agent: meshMonitor -> mapAgent, Use MU_AGENT_RSP_SRVPORT_MSG
    CMSG_MU_AGENT_REQ_UPGRADE = 5158, // Agent: mapAgent -> meshMonitor -> cos, Use MU_AGENT_UPGRADE_REQ_MSG

    /* Agent: cos -> meshMonitor -> mapAgent, Use MU_AGENT_UPGRADE_RESULT_MSG, if upgrade failed, 
     * will use this msg, but if upgrade success, will not return any msg
     */
    CMSG_MU_AGENT_UPGRADE_RESULT = 5159, 
	
    CMSG_MU_AGENT_RSP_AUTH_KEY = 5161,// Agent: httpd -> cos -> meshMonitor -> mapAgent, Use MU_HTTP_AUTH_MSG
    /* --------------------    Agent Inner Msg End   ----------------------- */
#endif /* INCLUDE_EASYMESH_MULTI_UPGRADE*/

#if defined(INCLUDE_EASYMESH_CERTIFICATION)
	CMSG_SIGMA_RESET_DEFAULT = 5180,
	CMSG_SIGMA_ONBOARDING_TYPE = 5181,
	CMSG_SIGMA_START_WPS = 5182,
#endif //INCLUDE_EASYMESH_CERTIFICATION
#ifdef INCLUDE_EASYMESH_TPVDMP

/* --------------------    TPVDMP Msg begin	 ----------------------- *//*add by zxy 20221103*/
	CMSG_EASYMESH_TP_VENDOR_TIME_UPDATE = 5190,	//for tpvdmp update time
	CMSG_EASYMESH_TP_VENDOR_LED_UPDATE = 5191,	//for tpvdmp update LED
/* --------------------    TPVDMP Msg end  ----------------------- *//*add by zxy 20221103*/
#endif
#ifdef INCLUDE_QOE
	/* As QOE feature requires to obtain the data in datamodel, to follow the design,
	 * we need to add new flag to access cmm lib and transfer those data from cos.
	 */
	CMSG_QOE_AGENT_REQ_DEVDATA = 5200, // Agent: mapAgent -> meshMonitor -> cos, Use CMSG_QOE_AGENT_REQ_DEVDATA
	CMSG_QOE_AGENT_RSP_DEVDATA = 5201, // Agent: cos -> meshMonitor -> mapAgent, Use CMSG_QOE_AGENT_RSP_DEVDATA
	CMSG_QOE_SEND_CONTROLLER_PROFILE = 5210, // obuspa -> meshMonitor -> mapController, Use CMSG_QOE_SEND_CONTROLLER_PROFILE
	CMSG_QOE_BULKDATA_PROFILE_RECONFIG = 5211,
	CMSG_QOE_BULKDATA_RECONFIG = 5212,
#endif /* INCLUDE_QOE */

#ifdef INCLUDE_EASYMESH_TP_ONBOARDING
	CMSG_PRE_NETWORKING_BACKHAUL_CONF = 5300,
	CMSG_PRE_NETWORKING_DEVICE_JOIN_SUCCESS = 5301,
	CMSG_PRE_NETWORKING_BACKHAULAP_STATUS_SEND = 5302,
	CMSG_PRE_NETWORKING_BACKHAULAP_STATUS_RECV = 5303,
	CMSG_PRE_NETWORKING_UPDATE_PRECONFIG_MESH_INFO = 5304,
#endif /*INCLUDE_EASYMESH_TP_ONBOARDING*/	
	CMSG_PRE_NETWORKING_UPDATE_AWND_STATUS= 5305,

#ifdef INCLUDE_AGINET_APP_V2
	CMSG_AGINET_BACKHAULAP_STATUS_SEND = 5306,
#endif /*INCLUDE_AGINET_APP_V2 */

	/* added by jinshuaichen for device inform, 2022-07-07*/
	CMSG_AGINET_INFORM_DEVICE = 5310,

	CMSG_EASYMESH_UPDATE_ETH_CLIENT_INFO = 5037,
	CMSG_EASYMESH_SYNC_ETH_CLIENT_INFO = 5038,

#endif /* INCLUDE_EASYMESH */

#ifdef INCLUDE_TR111_PART1
	CMSG_DHCPS_VIVSIO_CFG = 6001,
	CMSG_DHCPC_VIVSIO_CFG = 6002,

	CMSG_DHCPS_ADD_MANAGE_DEV = 6003,
	CMSG_DHCPS_DEL_MANAGE_DEV = 6004,
#endif /* INCLUDE_TR111_PART1 */

#ifdef INCLUDE_XMPP
	CMSG_XMPP_CFG_UPDATE	= 6005,
	CMSG_XMPP_WAN_UP		= 6006,
	CMSG_XMPP_WAN_DOWN		= 6007,
	CMSG_XMPP_CWMP_UPDATE	= 6008,
	CMSG_XMPP_CONN_REQUEST	= 6009,
#endif 

#ifdef INCLUDE_DHCP_OPTION42_LAN
	CMSG_DHCPS_UPDATE_NTP_SERVER  = 6020,
#endif/*INCLUDE_DHCP_OPTION42_LAN*/

#ifdef INCLUDE_PON
#ifdef INCLUDE_PON_GPON
	CMSG_GPON_OMCI = 7000,
	CMSG_GPON_OMCI_MIBSET = 7001,
	CMSG_GPON_OMCI_MIBRESET = 7002,
	CMSG_GPON_OMCI_IP_HOST = 7010,
#endif
#ifdef INCLUDE_PON_EPON
	CMSG_EPON_OAM = 7100,
#endif
#endif /* INCLUDE_PON */
#ifdef INCLUDE_DECT
	CMSG_DECT_ALLOW_REGISTER		= 9000,
	CMSG_DECT_BASE_CFG_CHANGE		= 9001,
	CMSG_DECT_HANDSET_PAGING		= 9002,
	CMSG_DECT_HANDSET_UNREGISTER		= 9003,
	CMSG_DECT_HANDSET_DATETIME_SYNC		= 9004,	
	CMSG_DECT_HANDSET_STATUS_CHANGE 	= 9005,		
	CMSG_DECT_HANDSET_NAME_CHANGE		= 9006,
/* Add by wang haobin, 20140124, for dect cli */
	CMSG_DECT_CLI_DIAG_MODE_SET		= 9007,
	CMSG_DECT_CLI_MODEM_RESET 		= 9008,
	CMSG_DECT_CLI_SET_BMC_REQ 		= 9009,
	CMSG_DECT_CLI_SET_OSC_REQ 		= 9010,
	CMSG_DECT_CLI_SET_TBR6_REQ	 	= 9011,
	CMSG_DECT_CLI_SET_RFPI_REQ 		= 9012,
	CMSG_DECT_CLI_SET_XRAM_REQ 		= 9013,
	CMSG_DECT_CLI_SET_GFSK_REQ 		= 9014,
	CMSG_DECT_CLI_SET_RFMODE_REQ 		= 9015,
	CMSG_DECT_CLI_SET_FREQ_REQ 		= 9016,
	CMSG_DECT_CLI_SET_TPC_REQ 		= 9017,
	CMSG_DECT_CLI_GET_BMC_REQ 		= 9018,
	CMSG_DECT_CLI_GET_XRAM_REQ 		= 9019,
	CMSG_DECT_CLI_GET_TPC_REQ 		= 9020,
	CMSG_DECT_CLI_GET_BMC_REP		= 9021,
	CMSG_DECT_CLI_GET_XRAM_REP 		= 9022,
	CMSG_DECT_CLI_GET_TPC_REP 		= 9023,
	CMSG_DECT_CLI_PROCESS_RESULT		= 9024,
	CMSG_DECT_CLI_GET_RFPI_REQ      	= 9025,
	/* Added by wanglongmeng, 20141201, for sending back the ATE test results. 
	 * This is useful for manually cmd sending.
	 */
	CMSG_DECT_CLI_ATE_MEASUREMENT   	= 9026,
	/* end added */
/* end add */
	CMSG_DECT_HANDSET_TEST_START 		= 9027,
	CMSG_DECT_HANDSET_TEST_STOP		= 9028,
	//CMSG_VOIP_USBMAIL_UNREAD_COUNT 		= 9029,
	CMSG_DECT_LINE_SETTINGS_CHANGE		= 9030,
	CMSG_DECT_CONTACT_CHANGE		= 9031,
	CMSG_DECT_QUERY_BASE_MODE       	= 9032,
	CMSG_DECT_HANDSET_UNREGISTER_RESET	= 9033,
	CMSG_DECT_START					= 9034,
	/*
	 * brief: order server to check the md5 of DECT Image file
	 */
	CMSG_DECT_VERIFY	 = 9100,
	/*
	 * brief: order server to begin write the fw
	 */
	CMSG_DECT_WRITE		 = 9101,
	/*
	 * brief: wait for the completion of DECT firmware upgrade
	 */
	CMSG_DECT_WRITE_OVER = 9102,
	CMSG_DECT_REBOOT     = 9103,
	CMSG_DECT_END		 = 9104,

#endif /* INCLUDE_DECT */

#ifdef INCLUDE_CLOUD /* Added by zjj, 20150922, for cloud service message. */
	CMSG_CLOUD_UPGRADE_FIRMWARE 	= 9200,
	CMSG_CLOUD_CHECK_FW_UPDATE		= 9201,
	/*notify cloud_client to reconnect cloud server when default gateway changed.*/
	CMSG_CLOUD_NOTIFY_RECONNECT 	= 9202,
#ifdef INCLUDE_CLOUD_V2
	/*get device status*/
	CMSG_CLOUD_GET_DEV_STATUS		= 9203,
	CMSG_CLOUD_GET_DEV_STATUS_HTTPS = 9204,
	/* bind is not used by now, it will be called when first login */
	CMSG_CLOUD_ACCOUNT_BIND 		= 9210,
	CMSG_CLOUD_ACCOUNT_UNBIND		= 9211,
	CMSG_CLOUD_ACCOUNT_LOGIN		= 9212,
	/* for cloud ddns service. */
	CMSG_CLOUD_DDNS_REGISTER		= 9220,
	CMSG_CLOUD_DDNS_BIND			= 9221,
	CMSG_CLOUD_DDNS_UNBIND			= 9222,
	CMSG_CLOUD_DDNS_UNBIND_ALL		= 9223,
	CMSG_CLOUD_DDNS_GET_LIST		= 9224,
	CMSG_CLOUD_DDNS_DELETE			= 9225,
	/*for device management*/
	CMSG_CLOUD_DEVMGMT_GET_TOKEN	= 9230,
	CMSG_CLOUD_DEVMGMT_ADD_DEV_USER = 9231,
	CMSG_CLOUD_DEVMGMT_RM_DEV_USER	 = 9232,
	CMSG_CLOUD_DEVMGMT_GET_DEV_USERINFO = 9233,
	CMSG_CLOUD_DEVMGMT_PASSTHROUGH	= 9234,
	
#ifdef INCLUDE_CLOUD_HTTPS
	/* for cloud https interface */
	CMSG_CLOUD_HTTPS_DEVMGMT_GET_TOKEN	= 9240,
	CMSG_CLOUD_HTTPS_ACCOUNT_CHECK_BIND = 9241,
	CMSG_CLOUD_HTTPS_ACCOUNT_BIND		= 9242,
	CMSG_CLOUD_HTTPS_ACCOUNT_UNBIND 	= 9243,
	CMSG_CLOUD_HTTPS_DEVMGMT_RM_DEV_USER	= 9244,
	CMSG_CLOUD_HTTPS_CHECK_CONNECT		= 9245,
	CMSG_CLOUD_SET_DST_RULE				= 9246,
#endif
	
#ifdef INCLUDE_LTEWAN
	CMSG_CLOUD_USAGE_ALERT = 9246,
#endif /* INCLUDE_LTEWAN */

#ifdef INCLUDE_CLOUD_EASYMESH_GROUP_MANAGEMENT
	CMSG_CLOUD_GRPMGMT_UPDATE_FEATUREINFO	= 9250,
#endif /* INCLUDE_CLOUD_EASYMESH_GROUP_MANAGEMENT */

#endif /* INCLUDE_CLOUD_V2 */
	
#endif /* INCLUDE_CLOUD */
#ifdef  INCLUDE_WAN_BLOCK
CMSG_WAN_BLOCK_STOP_BLOCK = 9300,
#endif

#ifdef INCLUDE_SPEEDTEST
	CMSG_SPEEDTEST_SET_STATUS = 9400,
#endif /* INCLUDE_SPEEDTEST */

#ifdef INCLUDE_PORT_MIRROR
	CMSG_START_PORT_MIRROR = 9500,
	CMSG_STOP_PORT_MIRROR = 9501,
#endif /*INCLUDE_PORT_MIRROR*/
#ifdef INCLUDE_PACKET_CAPTURE
	CMSG_PKTCAP_START_CAPTURE  = 9601,
	CMSG_PKTCAP_STOP_CAPTURE = 9602,
	CMSG_PKTCAP_SET_STATUS	 = 9603,
	CMSG_PKTCAP_UPDATE_LEFTIME = 9604,
#endif /* INCLUDE_PACKET_CAPTURE */
#ifdef INCLUDE_PKTCAP_UPLOAD
	CMSG_UPLOAD_START = 10000,
	CMSG_UPLOAD_STATE_CHANGE = 10001,
#endif /* INCLUDE_PKTCAP_UPLOAD */

#ifdef INCLUDE_TR369
	CMSG_LOCALAGENT_SUBSCRIPTION =9600,
	CMSG_USPA_RECONFIG = 10100,
	CMSG_USPA_WAN_UPDATE = 10101, 
#endif

#ifdef INCLUDE_CONTAINER
	CMSG_CWMP_DUSTCHG_COMPLETE = 10200,
	CMSG_CWMP_DUSTCHG_OP	= 10201,
	CMSG_TP_EE_ENABLE = 10202,
	CMSG_TP_EE_DISABLE = 10203,
	CMSG_TP_EE_INSTALL_APP = 10204,
	CMSG_TP_EE_UNINSTALL_APP = 10205,
	CMSG_TP_EE_UPDATE_APP = 10206,
	CMSG_TP_EE_APP_STATUS_CHANGE = 10207,
#endif /* INCLUDE_CONTAINER */

#ifdef INCLUDE_EASYDIAG
	CMSG_SNTP_QUERY_START = 10300,
#endif /* INCLUDE_EASYDIAG */

#ifdef INCLUDE_IGMP_DIAG
	CMSG_IGMPDIAG_CFG_MSG = 10400,
#endif /* INCLUDE_IGMP_DIAG */

#ifdef INCLUDE_LED_MATRIX
	CMSG_LED_MATRIX_DEBUG = 10500,
	CMSG_LED_MATRIX_MODE,
	CMSG_LED_MATRIX_SETTINGS,
	CMSG_LED_MATRIX_REC_SLIDES,
	CMSG_LED_MATRIX_REC_ANIMATION,
	CMSG_LED_MATRIX_REC_TEXT,
	CMSG_LED_MATRIX_SYS_EVENT,
#endif
#ifdef INCLUDE_VOIP_REMOTE
	CMSG_REMOTE_BOARD_REBOOT = 10600,
	CMSG_REMOTE_BOARD_UPGRADE,
#endif /*INCLUDE_VOIP_REMOTE*/

	CMSG_CMGR_EE_CREATE = 10700,
	CMSG_CMGR_EE_DESTROY,
	CMSG_CMGR_EE_START,
	CMSG_CMGR_EE_STOP,
	CMSG_CMGR_EE_CONFIG,
	CMSG_CMGR_EE_QUERY,
	CMSG_CMGR_EE_LOG,
	CMSG_CMGR_DU_INSTALL,
	CMSG_CMGR_DU_UNINSTALL,
	CMSG_CMGR_DU_UPDATE,
	CMSG_CMGR_EU_STATE_CHANGE,
	CMSG_CMGR_EE_CREATE_RET,
	CMSG_CMGR_EE_DESTROY_RET,
	CMSG_CMGR_EE_START_RET,
	CMSG_CMGR_EE_STOP_RET,
	CMSG_CMGR_EE_CONFIG_RET,
	CMSG_CMGR_EE_QUERY_RET,
	CMSG_CMGR_EE_LOG_RET,
	CMSG_CMGR_DU_INSTALL_RET,
	CMSG_CMGR_DU_UNINSTALL_RET,
	CMSG_CMGR_DU_UPDATE_RET,
	CMSG_CMGR_EU_STA_CHANGE_RET,
	CMSG_COS_DU_OPERATE_RET,
	CMSG_COS_EE_OPERATE_RET,
	CMSG_CWMP_DU_OPERATE_RET,	/* error of DUop happen before cmgr */
	CMSG_CWMP_EE_OPERATE_RET,	/* error of EEop happen before cmgr */
	CMSG_HTTP_DU_OPERATE_RET,	/* error of DUop happen before cmgr */
	CMSG_HTTP_EE_OPERATE_RET,	/* error of EEop happen before cmgr */

	CMSG_MAX
}CMSG_TYPE;

typedef enum _ENUM_MULTIMODE
{
	MULTIMODE_NONE_MODE = -1,
	MULTIMODE_ROUTER_MODE = 0,
	MULTIMODE_AP_MODE,
	MULTIMODE_RE_MODE,
	MULTIMODE_NUM
}ENUM_MULTIMODE;


/* 
 * brief	Message struct
 */
typedef struct
{
	CMSG_TYPE type;		/* specifies what message this is */
	unsigned int priv;		/* private data, one word of user data etc. */
	unsigned char content[MSG_CONTENT_SIZE];
}CMSG_BUFF;

typedef struct
{
	char mac[64];
	char ipaddr[45];
	char hostname[64];
}DHCP_INFO_MSG;


typedef struct{
	unsigned char updateFlag;
	char vendor_class_identifier[17];
	unsigned char mac[6];
	unsigned char ipaddr[4];
	char name[64];
}TP_DHCP_INFO;

/* 
 * brief	Message type identification	
 */
typedef enum
{
	CMSG_ID_NULL = 5,	/* start from 5 */
	CMSG_ID_COS = 6,
	CMSG_ID_LOG = 7,
	CMSG_ID_DHCPS = 8,	/* Added by xcl, 2011-06-13.*/
	CMSG_ID_DHCPC = 9, 
	CMSG_ID_CLI = 10,
	CMSG_ID_DNS_PROXY = 11,
#ifdef INCLUDE_IPV6_MLD	/* Add by HYY: MLDv2 Proxy, 01Jul13 */
	CMSG_ID_MLD	= 12,
#endif /* INCLUDE_IPV6_MLD */
	CMSG_ID_SNTP = 13,
	CMSG_ID_HTTP = 14,
	CMSG_ID_IGMP = 15,	/* Added by Wang Wenhao, 2011-11-18 */
	CMSG_ID_DDNS_PH = 16,	/* addde by tyz, 2011-07-21 */
	CMSG_ID_PH_RT = 17,
	CMSG_ID_UPNP =18,	/* Added  by  Li Chenglong , 11-Jul-31.*/
	CMSG_ID_DIAGTOOL =19, /*Added by Yuan Shang, 2011-08-18 */
	CMSG_ID_CWMP = 20, /* add by wuzhiqin, 2011-09-26 */
	CMSG_ID_SNMP = 21, /* Added by xcl, 21Sep11 */
	CMSG_ID_DYNDNS = 22, /* Added by xcl, 24Nov11 */
	CMSG_ID_NOIPDNS = 23, /*added by tpj, 2012-2-1*/
	CMSG_ID_DDNS_UD = 24, /* added by zj, for userdefine ddns, 28May14 */
	CMSG_ID_IPSEC = 25,
	/* Added by LI CHENGLONG , 2011-Dec-15.*/
	CMSG_ID_DLNA_MEDIA_SERVER = 26,
	/* Ended by LI CHENGLONG , 2011-Dec-15.*/
#ifdef INCLUDE_VOIP
	CMSG_ID_VOIP = 27,	/* for voice process, added by zhonglianbo 2011-8-10 */
#endif /* INCLUDE_VOIP */
#ifdef INCLUDE_TR143   
	CMSG_ID_TR143 = 28,
#endif /* INCLUDE_TR143 */
#ifdef INCLUDE_WIFI_MESH_SUPPORT
	CMSG_ID_MESHMONITOR = 29,
#endif /* INCLUDE_WIFI_MESH_SUPPORT */

#ifdef INCLUDE_PORTABLE_APP
	CMSG_ID_PORTABLE_APP = 30,
#endif /* INCLUDE_PORTABLE_APP */


#ifdef INCLUDE_WIFI_MESH_SUPPORT
	CMSG_ID_COS_MESH_MSG_RELAY = 32,
#endif
#ifdef INCLUDE_CLOUD /* Added by zjj, 20150922, for cloud service message. */
		CMSG_ID_CLOUD_CLIENT = 33,
#endif /* INCLUDE_CLOUD */

#ifdef INCLUDE_CLOUD_HTTPS /* Added by pudongfang, 20180905, for cloud service message. */
		CMSG_ID_CLOUD_HTTPS_CLIENT = 34,
#endif /* INCLUDE_CLOUD_HTTPS */

#ifdef INCLUDE_XMPP
	CMSG_ID_XMPP = 35,
#endif

#ifdef INCLUDE_PON
#ifdef INCLUDE_PON_GPON
	CMSG_ID_GPON_OMCI = 36,
#endif
#ifdef INCLUDE_PON_EPON
	CMSG_ID_EPON_OAM = 37,
#endif
#endif

#ifdef INCLUDE_ONLINE_DETECTION
	CMSG_ID_WANCONN2 = 39,
#endif /* INCLUDE_ONLINE_DETECTION */

#ifdef INCLUDE_EASYMESH /* Added by liaoyilu, 20201218, for easymesh app message. */
	CMSG_ID_APSD = 40,
	CMSG_ID_AWND = 41,
	CMSG_ID_MAP_CONTROLLER = 42,
	CMSG_ID_MAP_AGENT = 43,
	CMSG_ID_BHOPT = 44,
#endif
#if 1//def INCLUDE_AI_ROAMING
	CMSG_ID_NRD = 45,
#endif
#ifdef INCLUDE_PORT_MIRROR
	CMSG_ID_PORTMIRROR_TIMER = 46,
#endif

#ifdef INCLUDE_WAN_BLOCK
	CMSG_ID_WAN_BLOCK = 47,
#endif

#ifdef INCLUDE_EASYMESH_MULTI_UPGRADE
	CMSG_ID_UPGRADE_CORE = 48,
#endif /* INCLUDE_EASYMESH_MULTI_UPGRADE */

#ifdef INCLUDE_TR369
	CMSG_ID_USPA = 49,
#endif
#ifdef INCLUDE_PKTCAP_UPLOAD
	CMSG_ID_UPLOAD = 50,
#endif /*INCLUDE_PKTCAP_UPLOAD*/
#ifdef INCLUDE_PACKET_CAPTURE
	CMSG_ID_PKTCAP = 51,
#endif /* INCLUDE_PACKET_CAPTURE */

#ifdef INCLUDE_WEB_WARN
	CMSG_ID_WEB_WARN = 52,
#endif /* INCLUDE_WEB_WARN */
#ifdef INCLUDE_DECT
	CMSG_ID_VOIP_SERVER = 53,
	CMSG_ID_VOIP_CLIENT_DECT = 54,
#endif /* INCLUDE_DECT */


#ifdef INCLUDE_CONTAINER   
		CMSG_ID_TR157 = 55,
#endif /* INCLUDE_CONTAINER */

#ifdef INCLUDE_IGMP_DIAG
		CMSG_ID_IGMPDIAG = 56,
#endif /* INCLUDE_IGMP_DIAG */

#ifdef INCLUDE_LED_MATRIX
	CMSG_ID_LED_MATRIX	= 57,
#endif

#ifdef INCLUDE_EASYMESH_TPVDMP
	CMSG_ID_TPVDMP_UPGRADE_CORE = 58,
#endif /*INCLUDE_EASYMESH_TPVDMP*/

#ifdef INCLUDE_VOIP_REMOTE
	CMSG_ID_REMOTE_BOARD = 59,
#endif /*INCLUDE_VOIP_REMOTE*/

#ifdef INCLUDE_SMART_HOME_V2
	CMSG_ID_CMGR = 60,
#endif /*INCLUDE_SMART_HOME_V2*/

#ifdef INCLUDE_ADVANCED_FIREWALL_LOG
	CMSG_ID_FIREWALL_LOG_ENABLE = 61,
#endif /* INCLUDE_ADVANCED_FIREWALL_LOG */

	CMSG_ID_MAX
}CMSG_ID;


/* for all message type 
 * ע�ⲻҪʹ��UINT8�������Զ������������
 */

/* 
 * brief	CMSG_SNTP_CFG message type content
 */
typedef struct
{
	char	ntpServers[SNTP_DM_SERVER_NUM][32];
	unsigned int primaryDns;
	unsigned int secondaryDns;
	unsigned int timeZone;
}SNTP_CFG_MSG;

/* 
 * brief: Added by LI CHENGLONG, 2011-Nov-21.
 * ������ص���Ϣ����DLNA_MEDIA_SERVER��������ʱͨ��INIT��Ϣ���͸�DLNA_MEDIA_SERVER���̣�
 * DLNA_MEDIA_SERVER�����ڷ���ssdp ͨ��ʱ��ͨ���ض����̵���Ϣ.
 */
typedef struct _MANUFACT_SPEC_INFO
{
	char	devManufacturerURL[64];
	char	manufacturer[64];
	char	modelName[64];
	char	devModelVersion[16];
	char	description[256];
}MANUFACT_SPEC_INFO;

/* 
 * brief: Added by LI CHENGLONG, 2011-Dec-15.
 *		  ����һ������Ŀ¼�Ľṹ.
 */
typedef struct _DMS_FOLDER_INFO
{
	char	dispName[32];
	char	path[128];
	char	uuid[64];
	int enable;	/*added by LY to record whether this item is enabled, in 20141203*/
}DMS_FOLDER_INFO;


/* 
 * brief: Added by LI CHENGLONG, 2011-Dec-16.
 *		  ��Ŀ¼�Ĳ�������.
 */
typedef enum _DMS_FOLDER_OP 
{
	DMS_INIT_FOLDER = 0,
	DMS_DEL_FOLDER = 1,
	DMS_ADD_FOLDER = 2
}DMS_FOLDER_OP;

/* 
 * brief: Added by LAI LINZHI, 2022-MAR-10.
 *		  for interface separation of IGMP Proxy
 */
typedef struct _IGMPD_ADD_IF_MSG
{
	char	addIfName[64];
	char	boundIfName[64];
}IGMPD_ADD_IF_MSG;

#ifdef INCLUDE_TRTC
typedef struct _IGMPD_SET_RT_MSG
{
	char ifName[64];
	unsigned char igmpVer;
	unsigned char qqic; 		/* Querier Query Interval code */
	unsigned char qric; 		/* Query Response Interval code */
	unsigned char lmqic;		/* Group-specific Query Response Interval Code */
	unsigned char robustness;		/* Robustness Veriable */
}IGMPD_SET_RT_MSG;
typedef struct _IGMPD_GET_RT_MSG
{
	char ifName[64];
	unsigned char sqc;
	unsigned char groupNum;
}IGMPD_GET_RT_MSG;
#endif
/* 
 * brief: Added by LI CHENGLONG, 2011-Dec-15.
 * ����DLNA_MEDIA_SERVER���̺���������,����ʼ��������Ϣ���͸�DLNA_MEDIA_SERVER����.
 */
typedef struct _DMS_INIT_INFO_MSG
{
	unsigned char		scanFlag;				/*scan*/
	unsigned char		serverState;			/* ServerState */
	unsigned int		folderCnt;			/*how many folde is shared now*/
	int			shareAll;			/*indicate whether share all the volumes, added by LY in 2014.09.05 */
	unsigned int		scanInterval;		/*scan interval*/
	MANUFACT_SPEC_INFO	manuInfo;				/*oem�Ȳ�ͬ���̵���Ϣ*/
	char			serverName[16];

}DMS_INIT_INFO_MSG;

/* 
 * brief: Added by LI CHENGLONG, 2011-Dec-15.
 * �ϲ�UI������DLNA_MEDIA_MEDIA_SERVER���ú�ֱ�ӽ��������ô���DLNA_MEDIA_SERVER����,,
 * ���ٶԸ����������з���.
 */
typedef struct _DMS_RELOAD_MSG
{
	unsigned char		serverState;			/* ServerState */
	char				serverName[16];
	unsigned char		scanFlag;				/*scan*/
	unsigned int		scanInterval;		/*scan interval*/	
	int			shareAll;				/*added by LY to indicate whether to share all the volumes*/						
}DMS_RELOAD_MSG;

/* 
 * brief: Added by LI CHENGLONG, 2011-Dec-16.
 * ����Ŀ¼����Ϣ.
 */
typedef struct _DMS_OP_FOLDER_MSG
{
	 DMS_FOLDER_OP			op;
	 DMS_FOLDER_INFO		folder;
}DMS_OP_FOLDER_MSG;

/* Ended by LI CHENGLONG , 2011-Dec-15.*/
 
/* 
 * brief	CMSG_ID_CLI message type content
 */
typedef struct _CLI_USR_CFG_MSG
{
	char		rootName[16];	/* RootName */
	char		rootPwd[65];	/* RootPwd */
	char		adminName[16];	/* AdminName */
	char		adminPwd[65];	/* AdminPwd */
	char		userName[16];	/* UserName */
	char		userPwd[65];	/* UserPwd */
	char		manufact[32];/* Added by Li Chenglong , 2011-Oct-12.*/
#ifdef INCLUDE_SAVE_KEY_AS_HASH
	char		rootSalt[HASHKEY_SALT_LEN];
	char 		adminSalt[HASHKEY_SALT_LEN];
	char		userSalt[HASHKEY_SALT_LEN];
#endif /* INCLUDE_SAVE_KEY_AS_HASH */
}CLI_USR_CFG_MSG;

/* 
 * brief	ACCOUNT_LOGIN_MSG message type content
 */
typedef struct _ACCOUNT_LOGIN_MSG
{
	char		loginName[16];	/* LoginName */
}ACCOUNT_LOGIN_MSG;

/* 
 * brief: Added by Li Chenglong, 11-Jul-31.
 *		  UPnP enable message
 */
typedef struct _UPNP_ENABLE_MSG
{
	unsigned int enable; 
}UPNP_ENABLE_MSG;

/* Add by chz, 2012-12-24 */
typedef struct _UPNP_DEL_MSG
{
	unsigned int port;
	char protocol[16];
}UPNP_DEL_MSG;
/* end add */

#ifdef INCLUDE_UPNP_PORTMAP_SWITCH
typedef struct _UPNP_PORTMAPPING_ENABLE_MSG
{
	unsigned char enable;
}UPNP_PORTMAPPING_ENABLE_MSG;
#endif /*INCLUDE_UPNP_PORTMAP_SWITCH*/

/* 
 * brief: Added by Li Chenglong, 11-Jul-31.
 * Ĭ������״̬�ı����Ϣ��
 */
typedef struct _UPNP_DEFAULT_GW_CH_MSG
{
	char gwName[64];
	char gwAddr[16];
	char gwL2Name[64];
	char gwL2Addr[16];
	unsigned char natEnabled;
	unsigned char upDown;
}UPNP_DEFAULT_GW_CH_MSG;

/* Added by xcl, 2011-07-25 */
typedef struct 
{
	unsigned int delLanIp;
	unsigned int delLanMask;
}DHCPS_RELOAD_MSG_BODY;

typedef struct _PPP_CFG_MSG
{
	unsigned int pppDevUnit;
	char connectionStatus[18];
	unsigned int pppLocalIp;
	unsigned int pppSvrIp;
	unsigned int uptime;
	char lastConnectionError[32];
	unsigned int dnsSvrs[2];
	unsigned int currentMRU;
#ifdef INCLUDE_PADT_PRECEDE
	/* ...Added by Wang Jianfeng 2014-05-06 */
	char peerETH[18];
	unsigned short sessionID;
	/* end wjf */
#endif
}PPP_CFG_MSG;

#ifdef INCLUDE_WIFI
typedef enum _ENUM_WIFI_AUTHMODE
{
	WIFI_AUTHMODE_OPEN = 1,
	WIFI_AUTHMODE_SHARED = 2,
	WIFI_AUTHMODE_OPEN_SHARED = 3,
	WIFI_AUTHMODE_WPA = 4,
	WIFI_AUTHMODE_WPA2 = 5,
	WIFI_AUTHMODE_PSK = 6,
	WIFI_AUTHMODE_PSK2 = 7,
	WIFI_AUTHMODE_WPA_WPA2 = 8,
	WIFI_AUTHMODE_PSK_PSK2 = 9,
	WIFI_AUTHMODE_PSK3 = 10,
	WIFI_AUTHMODE_PSK2_PSK3 = 11
}ENUM_WIFI_AUTHMODE;

typedef enum _ENUM_WIFI_ENCRYPTMODE
{
	WIFI_ENCRYPTMODE_NONE = 1,
	WIFI_ENCRYPTMODE_WEP = 2,
	WIFI_ENCRYPTMODE_TKIP = 3,
	WIFI_ENCRYPTMODE_AES = 4,
	WIFI_ENCRYPTMODE_TKIP_AES = 5
}ENUM_WIFI_ENCRYPTMODE;

/* Added by Zeng Yi. 2011-07-08 */
typedef struct
{
	unsigned char isError;
#ifdef INCLUDE_WIFI_DUALBAND
	char iface[16];
#endif /* INCLUDE_WIFI_DUALBAND */
	char SSID[33];
	char BSSID[18];
	int authMode;
	int encryMode;
	char key[65];
	unsigned char keyIndex;
}WPS_CFG_MSG;
#endif /* INCLUDE_WIFI */

#ifdef INCLUDE_IPV6	
/*
 *	IPv6 address structure
 */

typedef struct
{
	union 
	{
		unsigned char	u6_addr8[16];
		unsigned short	u6_addr16[8];
		unsigned int	u6_addr32[4];
	} in6_u;
}IN6_ADDR;


/* Support dynamic 6RD */
typedef struct
{
	unsigned char ipv4MaskLen;
	unsigned char sit6rdPrefixLen;
	IN6_ADDR sit6rdPrefix;
	unsigned int sit6rdBRIPv4Addr;
}DHCPC_6RD_INFO;

typedef struct _PPP6_CFG_MSG
{
	unsigned int pppDevUnit;
	unsigned char pppIPv6CPUp;
	IN6_ADDR remoteID;
	IN6_ADDR localID;
#ifdef INCLUDE_PADT_PRECEDE
	/* Fix bug 359034, by Lai Daokuan, 2020-04-15 */
	char peerETH[18];
	unsigned short sessionID;
	/* end Lai Daokuan */
#endif
}PPP6_CFG_MSG;

typedef struct 
{
	IN6_ADDR addr;
	unsigned int pltime;
	unsigned int vltime;
	int plen;
	char mode;
}DHCP6C_ADDR_INFO;

#ifdef INCLUDE_IPV6_MAP

/* 
 * brief	Keep same with 16, 
 *			
 */
#define RULE_NUMS_MAX	16

typedef struct 
{
	IN6_ADDR ipv6Prefix;
	unsigned int ipv4Prefix;
	unsigned char ipv6PrefixLen;
	unsigned char ipv4PrefixLen;
	unsigned char EALen;
	unsigned char isFMR;
}DHCP6C_MAP_RULE;

typedef struct 
{
	unsigned int status;
	char intfName[64];
	IN6_ADDR dmrPrefix;
	
	#ifdef INCLUDE_IPV6_MAP_MAPE
	IN6_ADDR dmrAddress;
	char transportMode[14];
	#endif /*INCLUDE_IPV6_MAP_MAPE*/
	
	unsigned char dmrPrefixLen;
	unsigned char PSIDOffset;
	unsigned char PSIDLen;
	unsigned char rulesCnt;
	unsigned int PSID;
	DHCP6C_MAP_RULE rule[RULE_NUMS_MAX];
}DHCP6C_MAP_DOMAIN;
#endif /* INCLUDE_IPV6_MAP	*/

typedef struct 
{
	unsigned int status;
	char intfName[64];  
	DHCP6C_ADDR_INFO ip;
	DHCP6C_ADDR_INFO prefix;
	IN6_ADDR dns[2];
	IN6_ADDR sntp[2];
	char domain[92];
	char dsliteName[64];
	unsigned int replyStaCode;
}DHCP6C_INFO_MSG_BODY;

#ifdef INCLUDE_DNS_PROXY
typedef struct
{
	IN6_ADDR primaryDnsv6;
	IN6_ADDR secondaryDnsv6;
	int connId;
}DNS6_PROBE_CFG_MSG;
#endif	/* INCLUDE_DNS_PROXY */
#endif /* INCLUDE_IPV6 */


/* 
 * brief CMSG_DNS_PROXY_CFG	message type content
 */
typedef struct
{
	unsigned int primaryDns;
	unsigned int secondaryDns;
	int connId;
} DNS_PROXY_CFG_MSG;

/* 
 * brief CMSG_DNS_SERVER message type content
 */
typedef struct
{
	unsigned int xxx1;
	unsigned int dns;
	unsigned int xxx2;
	int connId;
} DNS_SERVER_MSG;
#ifdef INCLUDE_DNS_PROXY
/* 
 * brief CMSG_DNS6_SERVER message type content
 */
typedef struct
{
	unsigned int xxx1;
	char dns[40];
	unsigned int xxx2;
	int connId;
} DNS6_SERVER_MSG;

#endif

typedef struct 
{
	unsigned char status; /* Have we been assigned an IP address ? */
	char lastConnectionError[31];
	char ifName[64];  
	unsigned int ip;
	unsigned int mask;
	unsigned int gateway;
	unsigned int dns[2];
	unsigned int server;
#if defined(INCLUDE_PPTP) || defined(INCLUDE_L2TP)
	char connName[8];
#endif/* defined(INCLUDE_PPTP) || defined(INCLUDE_L2TP) */
#ifdef INCLUDE_OPTION66 /*Code transplanting by ljn from Wang Yang for DHCP Option66 2017.8.15*/
	char tftpIP[16];
#endif /* INCLUDE_OPTION66 */
#ifdef INCLUDE_IPV6	/* Add by HYY: support dynamic 6RD, 20Mar12 */
	DHCPC_6RD_INFO sit6rdInfo;
#endif /* INCLUDE_IPV6 */

#ifdef INCLUDE_TR069_ACSURL_FROM_DHCP
	char acsUrl[256];
#endif
	unsigned int leaseTime;
}DHCPC_INFO_MSG_BODY;

typedef struct 
{
	unsigned char unicast;
	char ifName[64];
	char hostName[64];
#if defined(INCLUDE_PPTP) || defined(INCLUDE_L2TP)
	char connName[32];
#endif /* defined(INCLUDE_PPTP) || defined(INCLUDE_L2TP) */
#ifdef INCLUDE_IPV6	/* Add by HYY: support dynamic 6RD, 19Mar12 */
	unsigned char sit6rdEnabled;
#endif /* INCLUDE_IPV6 */
#ifdef INCLUDE_MULTIMODE
	int multiMode;
#endif
#ifdef INCLUDE_OPTION66_LAN
	unsigned char initHookOpt66Flag;
#endif /* INCLUDE_OPTION66_LAN */
#ifdef INCLUDE_DHCP_OPTION60
	char op60VendorId[64];
#endif /* INCLUDE_DHCP_OPTION60 */
}DHCPC_CFG_MSG_BODY;

#ifdef INCLUDE_SMART_DHCP
typedef struct
{
	unsigned char enable;
	char ifName[16];
}SMART_DHCP_CFG_MSG_BODY;

typedef struct
{
	unsigned int event;
}SMART_DHCP_MESH_MSG_BODY;

typedef struct
{
	unsigned int status;
}SMART_DHCP_LINK_MSG_BODY;

#endif /* INCLUDE_SMART_DHCP */

#ifdef INCLUDE_AVOID_UPNP_USE_CWMP_PORT
typedef struct _UPNP_CWMP_PORT_CHANGE_MSG
{
	uint16_t portCwmpUsed;
	uint16_t portStunUsed;
}UPNP_CWMP_PORT_CHANGE_MSG;
#endif /*INCLUDE_AVOID_UPNP_USE_CWMP_PORT*/

/* Added by tyz 2011-08-02 (n & n) */
/* the msg of the interface */
typedef struct
{
	int ifUp;
	unsigned int ip;
	unsigned int gateway;
	unsigned int mask;
	unsigned int dns[2];
	char ifName[16];
}DDNS_RT_CHAGED_MSG;

/*
the msg of the ph running time
*/
typedef struct 
{
	unsigned char state;
	unsigned char sevType;
	unsigned short isEnd;
}DDNS_RT_PRIV_MSG;
/*
the msg of the cfg 
*/
typedef struct
{	
	int enabled;
	int reg;
	int userLen;
	char phUserName[64];
	int pwdLen;
	char phPwd[64];	
}DDNS_PH_CFG_MSG;

/* Added by xcl, 24Nov11 */
/* dynDns config msg struct */
typedef struct 
{
	unsigned char	enable;
	char			userName[256];
	char			password[256];
	char			domain[256];
	char			server[64];
	unsigned char	login;
}DYN_DNS_CFG_MSG;

typedef struct 
{
	unsigned int state;
}DYN_DNS_STATE_MSG;
/* End added by xcl */

/* Added by tpj, 17Jan12 */
/* noipDns config msg struct */
typedef struct 
{
	unsigned char	enable;
	char			userName[64];
	char			password[64];
	char			domain[128];
	char			server[64];
	unsigned char	login;
}NOIP_DNS_CFG_MSG;

typedef struct 
{
	unsigned int state;
}NOIP_DNS_STATE_MSG;
/* End added by tpj */

/* Add by ZJ, 28May14 */
typedef struct 
{
	unsigned char	enable;
	unsigned char	login;
	unsigned short	IPStartOffset;
	unsigned short	IPEndOffset;
	char		grabServer[64];
	char		grabAuth[64];
	char			grabRequest[256];
	char			grabDomain[120];
}DDNS_UD_CFG_MSG;
/* End add */

typedef struct
{
	unsigned int state;
}DDNS_UD_STATE_MSG;
/* end added by hx, 2015.04.13 */
typedef struct
{
	unsigned int command;
	char host[16];
	unsigned int result;
#ifdef INCLUDE_IPV6
	char hostv6[40];
#endif
#ifdef INCLUDE_EASYDIAG
	char ifname[64];
	char gw[16];
	char dns1[16];
	char dns2[16];
	unsigned int repeatNum;
	unsigned int succNum;
#endif /* INCLUDE_EASYDIAG */
}DIAG_COMMAND_MSG;


/* Added by xcl, 17Oct11, snmp msg struct */
typedef struct 
{
	unsigned short ifIndex;
}SNMP_LINK_STAUS_CHANGED_MSG;
/* End added */

/* added by LY for printer hotplug event msg */
typedef enum _USB_PRINTER_ACTION_TYPE
{
	USB_PRINTER_ADD = 0,
	USB_PRINTER_REMOVE
}USB_PRINTER_ACTION_TYPE;

typedef struct 
{
	USB_PRINTER_ACTION_TYPE printerActionType;
	char printerName[64];
}USB_PRINTER_HOTPLUG_MSG;
/* end added by LY */

#ifdef INCLUDE_IPSEC
typedef struct
{
	unsigned short currDepth;								
	unsigned short numInstance[6];	
}IPSEC_OLD_NUM_STACK;

typedef struct 
{
	int state;
	char default_gw_ip[16];
	char default_gw_ifName[64];
}IPSEC_WAN_STATE_CHANGED_MSG;

typedef struct
{
	char local_ip[16];
	char local_mask[16];
	unsigned int  local_ip_mode;
	char remote_ip[16];
	char remote_mask[16];
	unsigned int  remote_ip_mode;
	char real_remote_gw_ip[16];
	char spi[16];
	char second_spi[16];
	unsigned int entryID;
	unsigned int  op;
	unsigned char  enable;
	unsigned int key_ex_type; /*Added for vxWorks*/
	IPSEC_OLD_NUM_STACK stack;
}IPSEC_CFG_CHANGED_MSG;
#endif

#ifdef INCLUDE_OPENVPN_SERVER /* added by CCy for OpenVpn, 27Jul215 */
typedef enum
{
	OVPN_KEY_NOT_GENERATED = 0,
	OVPN_KEY_GENERATED	   = 1,
	OVPN_KEY_GENERATING    = 2
}OVPN_KEY_GEN_STATUS;

typedef struct _OVPN_STATUS_CHANGED_MSG
{
	OVPN_KEY_GEN_STATUS	 keyStatus;
} OVPN_STATUS_CHANGED_MSG;
#endif /*INCLUDE_OPENVPN_SERVER*/


#ifdef INCLUDE_USB_VOICEMAIL

typedef enum
{
	USBVM_SC_NEW = 0,			/* new voice mail, notify dect and voiceApp */
	USBVM_SC_LISTEN = 1,		/* voice mail listened, notify dect and voiceApp */
	USBVM_SC_DEL_UNREAD = 2,	/* del unread voice mail, notify dect and voiceApp */
	USBVM_SC_DEL_READ = 3		/* del read voice mail, only notify voiceApp */
} USBVM_STATUC_CHANGE_TYPE;

typedef struct
{
	USBVM_STATUC_CHANGE_TYPE type;
	int unreadCount;
	int endpt;
} USBVM_RECORD_STATUS_CHANGE_MSG;

#endif	/* INCLUDE_USB_VOICEMAIL */

#ifdef INCLUDE_IPPING_DIAG
typedef struct _IPPING_PARAM_MSG
{
	unsigned int	DSCP;	/* DSCP */
	unsigned int	dataBlockSize;	/* DataBlockSize */
	unsigned int	timeout;	/* Timeout */
	unsigned int	numberOfRepetitions;	/* NumberOfRepetitions */
	char			ifName[16];
	char			host[256];
	char			dns[64];
	char			diagnosticsState[28];
#ifdef INCLUDE_DIAG_TR143_IPPING_IPV6
	char			host6[256];
	char			dns6[92];
	char			diagnosticsStateV6[28];
#endif /* INCLUDE_DIAG_TR143_IPPING_IPV6 */

#ifdef INCLUDE_TR369
	unsigned int	instance; /* TR369 instance number*/
#endif
}IPPING_PARAM_MSG;

typedef struct _IPPING_RESULT_MSG
{
	unsigned int	maximumResponseTime;	/* MaximumResponseTime */
	unsigned int	minimumResponseTime;	/* MinimumResponseTime */
	unsigned int	averageResponseTime;	/* AverageResponseTime */
	unsigned int	maximumResponseTimeDetailed;	/* MaximumResponseTimeDetailed */
	unsigned int	minimumResponseTimeDetailed;	/* MinimumResponseTimeDetailed */
	unsigned int	averageResponseTimeDetailed;	/* AverageResponseTimeDetailed */
	unsigned int	failureCount;	/* FailureCount */
	unsigned int	successCount;	/* SuccessCount */
	char			diagnosticsState[28];	/* DiagnosticsState */
#ifdef INCLUDE_TR143D_FOR_CAF_TEST
	unsigned int	startTest;				/* Record send time */
#endif /* INCLUDE_TR143D_FOR_CAF_TEST */
	char			ipTarget[16];			/* IP parse form URL */
#ifdef INCLUDE_DIAG_TR143_IPPING_IPV6
	unsigned int	maximumResponseTimeV6;
	unsigned int	minimumResponseTimeV6;
	unsigned int	averageResponseTimeV6;
	unsigned int	maximumResponseTimeDetailedV6;
	unsigned int	minimumResponseTimeDetailedV6;
	unsigned int	averageResponseTimeDetailedV6;
	unsigned int	failureCountV6;
	unsigned int	successCountV6;
	char			diagnosticsStateV6[28];
	char			ipv6Target[48];
#endif /*INCLUDE_DIAG_TR143_IPPING_IPV6*/
#ifdef INCLUDE_TR369
	unsigned int	instance; /* TR369 instance number */
#endif
}IPPING_RESULT_MSG;
#endif /* INCLUDE_IPPING_DIAG */

#ifdef INCLUDE_TRACEROUTE_DIAG
typedef struct _TRACEROUTE_PARAM_MSG
{
	char			diagnosticsState[28];	/* DiagnosticsState */
	char			ifName[16];			/* Interface */
	char			host[256];			/* Host */
	unsigned int	numberOfTries;		/* NumberOfTries */
	unsigned int	timeout;			/* Timeout */
	unsigned int	dataBlockSize;		/* DataBlockSize */
	unsigned int	DSCP;				/* DSCP */
	unsigned int	maxHopCount;		/* MaxHopCount */	
	char			dns[64];
#ifdef INCLUDE_DIAG_TR143_TRACE_IPV6
	char			diagnosticsStateV6[28];
	char			host6[256];
	char			dns6[92];
#endif /* INCLUDE_DIAG_TR143_TRACE_IPV6 */

#ifdef INCLUDE_TR369
	unsigned int	instance; 			/* TR369 instance number */
#endif
}TRACEROUTE_PARAM_MSG;

typedef struct _TRACEROUTE_RESULT_MSG
{
	unsigned int	responseTime;		/* ResponseTime */
	unsigned int	routeHopsNumberOfEntries;	/* RouteHopsNumberOfEntries */
	unsigned int	hopErrorCode;		/* HopErrorCode */
	char			hopHost[256];		/* HopHost */
	char			hopHostAddress[16]; /* HopHostAddress */
	char			hopRTTimes[16];		/* HopRTTimes */
	char			diagnosticsState[28];	/* DiagnosticsState */
	char			X_TP_Finish;			/* X_TP_Finish */
#ifdef INCLUDE_DIAG_TR143_TRACE_IPV6
	unsigned int	responseTimeV6;
	unsigned int	routeHopsNumberOfEntriesV6;
	unsigned int	hopErrorCodeV6;
	char			hopHostV6[256];
	char			hopHostAddressV6[48];
	char			hopRTTimesV6[16];
	char			diagnosticsStateV6[28];
#endif /* INCLUDE_DAIG_TR143_TRACEROUTE_IPV6*/
#ifdef INCLUDE_TR369
	unsigned int	instance; 				/* TR369 instance number */
#endif
}TRACEROUTE_RESULT_MSG;
#endif /* INCLUDE_TRACEROUTE_DIAG */

#ifdef INCLUDE_NSLOOKUP_DIAG
typedef struct _NSLOOKUP_PARAM_MSG
{
	char			diagnosticsState[28];	/* DiagnosticsState */
	char			ifName[16];			/* Interface */
	char			host[256];			/* Host */
	unsigned int	numberOfTries;		/* NumberOfTries */
	unsigned int	timeout;			/* Timeout */
	char			dns[256];
#ifdef INCLUDE_TR369
	unsigned int	instance; 			/* TR369 instance number */
#endif
}NSLOOKUP_PARAM_MSG;

typedef struct _NSLOOKUP_RESULT_MSG
{
	unsigned int	responseTime;		/* ResponseTime */
	unsigned int	resultNumberOfEntries;	/* resultNumberOfEntries */
	unsigned int	successCount;	/* SuccessCount */
	char			HostNameReturned[256];		/* HostNameReturned */
	char			IPAddresses[256]; /* IPAddresses */
	char			DNSServerIP[16]; /* IPAddresses */
	char			AnswerType[18];		/* AnswerType */
	char			status[28];
	char			diagnosticsState[28];	/* DiagnosticsState */
	char			X_TP_Finish;			/* X_TP_Finish */
#ifdef INCLUDE_TR369
	unsigned int	instance; 				/* TR369 instance number */
#endif
}NSLOOKUP_RESULT_MSG;
#endif /* INCLUDE_NSLOOKUP_DIAG */

#if defined(INCLUDE_DOWNLOAD_DIAG) || defined(INCLUDE_UPLOAD_DIAG)
typedef struct _LOAD_PARAM_MSG
{
	unsigned int	DSCP;				/* DSCP */
	unsigned int	ethernetPriority;	/* EthernetPriority */
	unsigned int	testFileLength;		/* testFileLength */
	char			ifName[16];		
	char			url[256];			/* DownloadURL */
	char			dns[64];
	char			diagnosticsState[28];/* DiagnosticsState */
}LOAD_PARAM_MSG;

typedef struct _LOAD_RESULT_MSG
{
	unsigned int	testBytes;			/* TestBytes */
	unsigned int	totalBytes;			/* TotalBytes */
	char			ROMTime[32];	/* ROMTime */
	char			BOMTime[32];	/* BOMTime */
	char			EOMTime[32];	/* EOMTime */
	char			TCPOpenRequestTime[32];		/* TCPOpenRequestTime */
	char			TCPOpenResponseTime[32];	/* TCPOpenResponseTime */
	char			diagnosticsState[28];/* DiagnosticsState */
}LOAD_RESULT_MSG;
#endif /* defined(INCLUDE_DOWNLOAD_DIAG) || defined(INCLUDE_UPLOAD_DIAG) */

#ifdef INCLUDE_UDPECHO_DIAG
typedef struct _UDPECHO_PARAM_MSG
{
	char				ifName[16];		/* Interface */
	char				sourceIPAddress[16];/* SourceIPAddress */
	unsigned int		udpPort;			/* UDPPort */
	unsigned char		echoPlusEnabled;	/* EchoPlusEnabled */
	unsigned char		enable;				/* Enable */
}UDPECHO_PARAM_MSG;

typedef struct _UDPECHO_RESULT_MSG
{
	unsigned int	packetsReceived;	/* PacketsReceived */
	unsigned int	packetResponsed;	/* PacketResponsed */
	unsigned int	bytesReceived;		/* BytesReceived */
	unsigned int	bytesResponsed;		/* BytesResponsed */
	char			timeFirstPacketReceived[32];	/* TimeFirstPacketReceived */
	char			timeLastPacketReceived[32];		/* TimeLastPacketReceived */
	unsigned char	echoPlusSupported;				/* EchoPlusSupported */
}UDPECHO_RESULT_MSG;
#endif /* INCLUDE_UDPECHO_DIAG */
#ifdef INCLUDE_HW_QOS
typedef struct 
{
	unsigned char	 type;
	unsigned char	 code;
	unsigned int state;
}HW_QOS_LINK_STATE_MSG;
#endif

/* For Ip host, add by zengdongbiao, 19May16. */
#ifdef INCLUDE_PON_GPON
typedef struct _OMCI_IP_HOST_MSG
{
	unsigned int ipHostId;
	unsigned int ipAddr;
	unsigned int netMask;
	unsigned int gateway;
	unsigned int dns[2];
	int vlanId;				/* -1: vlan is disable. 0~4094: vlan id. */ 	
	char isp;				/* 1: static ip; 0: dhcp */
}OMCI_IP_HOST_MSG;
#endif /* INCLUDE_PON_GPON */
#ifdef INCLUDE_PACKET_CAPTURE
typedef struct _PKTCAP_MSG_BODY
{
	char ifname[16];
	unsigned char bFilter;
	char filter[8][64];
	char direction[8];
	char storage[9];
	char directory[128];
	char fileName[64];
	unsigned int timeout;
}PKTCAP_MSG_BODY;

typedef struct _PKTCAP_STATUS_ERROR_MSG
{
	char realFileName[64];
	unsigned char errorNum;
}PKTCAP_STATUS_ERROR_MSG;
#endif /* INCLUDE_PACKET_CAPTURE */

#ifdef INCLUDE_PKTCAP_UPLOAD
typedef struct _UPLOAD_MSG_BODY
{
	char url[URL_MAX_PATH];
	char filePath[FILE_MAX_PATH];
	char username[32];
	char password[32];
	char protocol[8];
	char dns[64];
	int needTimer;
	int overwrite;
	unsigned int establishedTime;
	unsigned int requestOid;
}UPLOAD_MSG_BODY;

typedef struct _UPLOAD_STATE_CHANGE_MSG
{
	char newState[16];
	unsigned int establishedTime;
	unsigned int requestOid;
	unsigned char errorNum;
}UPLOAD_STATE_CHANGE_MSG;
#endif /*INCLUDE_PKTCAP_UPLOAD*/

#ifdef INCLUDE_WEB_WARN
typedef struct __LAN_IP_INFO
{
	char IPAddress[16];
	char subnetMask[16];
	char ifName[16];
#ifdef INCLUDE_WEB_WARN_WIFI_NOSEC
	char vapName[16];
#endif /* INCLUDE_WEB_WARN_WIFI_NOSEC */
} LAN_IP_INFO;
#endif /* INCLUDE_WEB_WARN */

#ifdef INCLUDE_TR111_PART1
/* for dhcp option 125 config */
typedef struct _DHCP_VIVSIO_CFG_MSG
{
	char manufacturerOUI[7];
	char productClass[64];
	char serialNumber[64];

	/*add by huangjx,2014-12-22*/
	unsigned int ifIp;
	char portName[16];
	unsigned int hostIp;
	char mac[18];
}DHCP_VIVSIO_CFG_MSG;


typedef struct _ARP_CFG_INFO
{
	unsigned int ifIp;
	char portName[16];
	unsigned hostIp;

	char mac[18];
}ARP_CFG_INFO;

typedef struct _Tr111Dev_OfferedAddr {
	char	ifName[16];
	unsigned int yiaddr;
	char chaddr[18];
}Tr111DevOfferedAddr;

enum DHCPS_HOST_TYPE
{
	HOST_ETHERNET,
	HOST_WIRELESS,
	HOST_HOMEPLUG,
	HOST_OTHER
};
#endif	/* INCLUDE_TR111_PART1 */

#ifdef INCLUDE_DECT
typedef struct 
{
	unsigned char needReset;
	unsigned char name[16];
    unsigned char enabled;
	char pin[15];
	unsigned char greenModeEnabled;
	unsigned char ECOModeEnabled;
	unsigned char SecurityModeEnabled;
	unsigned char clockMaster;
}DECT_BASE_CFG;

typedef struct 
{
	unsigned int endpt;
	char name[16];
	char interNum[16];
	unsigned char widebandEnabled;
	unsigned char status;	/* Registered  status */
	char IPUI[9];	/* IPUI */
	char TPUI[5];	/* TPUI */
	char authKey[25];	/* AuthKey */
	char cipherKey[13];	/* CipherKey */
	unsigned char serviceClass;	/* ServiceClass */
	unsigned char modelId;	/* ModelId */
	unsigned int termCap;	/* TermCap */	
}DECT_HANDSET_INFO;
#endif /* INCLUDE_DECT */

#ifdef INCLUDE_ONEMESH
typedef enum
{
	SYNC_ALL = 0,
	SYNC_ONE = 1,
	SYNC_WITH_REPLY = 2
} SYNC_WIFI_TYPE;

typedef enum
{
	SLAVE_REMOVE = 0,
	SLAVE_ADD = 1
} ADD_REOVE_TYPE;

typedef enum
{
	BAND_2_4G = 0,
	BAND_5G = 1,
	
	BAND_NUM
}ENUM_BAND;


typedef struct _ONEMESH_PROBE_MSG
{
	unsigned int count;
	unsigned int timeout;
} ONEMESH_PROBE_MSG;
/*
typedef struct _ONEMESH_ADD_REMOVE_SLAVE_MSG
{
	char slaveMac[18];
	char slaveIp[16];
	unsigned char isAdd;
} ONEMESH_ADD_REMOVE_SLAVE_MSG;
*/
typedef struct _ONEMESH_ATTACH_MASTER_MSG
{
	char masterMac[18];
	unsigned int wantJoin;
	unsigned int timeout;
	unsigned int autoEnabled;
} ONEMESH_ATTACH_MASTER_MSG;

typedef struct _WIFI_SECURITY
{
	char		modeEnabled[20];	/* ModeEnabled */
	char		primaryPSK[65];	/* PreSharedKey */
	char		backhaulKeyPassphrase[64]; /*backhaul KeyPassphrase*/
	char		X_TP_WEPKey[129];	/* X_TP_WEPKey */
	char		X_TP_WEPAuthenticationMode[7];	/* X_TP_WEPAuthenticationMode */
	char		X_TP_WPAWPA2EncryptionMode[11];	/* X_TP_WPAWPA2EncryptionMode */
} WIFI_SECURITY;



typedef struct _WIFI_RADIO_CONF
{
	/*From DEV2_WIFI_RADIO_OBJ*/
	char band[7];
	unsigned int channel;
	unsigned char autoChannelEnable;
	/*From DEV2_WIFI_SSID_OBJ*/
	char ssid[33];
	char backhaulSSID[33];
	/*From DEV2_WIFI_AP_OBJ*/
	unsigned char apEnable;
	unsigned char ssidAdvertisementEnabled;
	/*From DEV2_WIFI_SECURITY_OBJ*/
	WIFI_SECURITY wifiSecurity;
}WIFI_RADIO_CONF;


typedef struct _ONEMESH_SYNC_WIFI_MSG
{
	struct _WIFI_RADIO_CONF radio_cfg[BAND_NUM];
	char mac[18];
	char ip[16];
	SYNC_WIFI_TYPE type;
	CMSG_FD msgFd;
} ONEMESH_SYNC_WIFI_MSG;

typedef struct _ONEMESH_ADD_REMOVE_SLAVE_MSG
{
	struct _WIFI_RADIO_CONF radio_cfg[BAND_NUM];
	char slaveMac[18];
	char slaveIp[16];
	ADD_REOVE_TYPE isAdd;
} ONEMESH_ADD_REMOVE_SLAVE_MSG;


typedef struct _ONEMESH_SET_RSA_MSG
{
	char masterRsaN[257];
	char masterRsaE[257];
	char masterPri[257];
} ONEMESH_SET_RSA_MSG;

typedef struct _ONEMESH_SET_JOINED_MSG
{
	char slaveMac[18];
	ADD_REOVE_TYPE isAdd;
} ONEMESH_SET_JOINED_MSG;

typedef struct _ONEMESH_SET_ACTION_MSG
{
	unsigned char action;
} ONEMESH_SET_ACTION_MSG;

#endif /*INCLUDE_ONEMESH*/

#ifdef INCLUDE_PORT_MIRROR
typedef struct _PORT_MIRROR_CFG_MSG
{
	int timeout;
}PORT_MIRROR_CFG_MSG;
#endif /*INCLUDE_PORT_MIRRORING*/

#ifdef INCLUDE_CLOUD_V2
typedef struct 
{
	char	email[65];
	char    passwd[33];
}CLOUD_APP_LOGIN_MSG;

typedef struct 
{
	unsigned char	enable;
	signed int status;
	char domain[36];
}TPLINKDNS_CFG_MSG;


typedef struct _CLOUD_ACCOUNT_USER_INFO
{
	char account[65];
} CLOUD_ACCOUNT_USER_INFO;


typedef struct _CLOUD_UNBIND_MSG
{
	unsigned int wanStatus;		/* add by chenyingbo 2018_07_13 */
	char userAccountId[65];
	char deviceId[41];
} CLOUD_UNBIND_MSG;


#ifdef INCLUDE_CLOUD_HTTPS
typedef struct _CLOUD_HTTPS_UNBIND_MSG
{
	char userAccountId[65];
	char accountManagerToken[34];
} CLOUD_HTTPS_UNBIND_MSG;
#endif	/* INCLUDE_CLOUD_HTTPS */


#endif
#ifdef INCLUDE_SPEEDTEST
typedef struct _SPEEDTEST_STATUS_MSG_BODY
{
	unsigned int download;
	unsigned int upload;
	unsigned int date;
}SPEEDTEST_STATUS_MSG_BODY;
#endif /* INCLUDE_SPEEDTEST */

#ifdef INCLUDE_QUALCOMM
// used when wps is handled by hostapd
typedef struct _WPS_STATUS_MSG
{
	char intf[16];
	char status[32];
} WPS_STATUS_MSG;
#endif

#ifdef INCLUDE_EASYMESH

//APSD
enum backhaul_type
{
	BACKHAUL_TYPE_ETHER,
	BACKHAUL_TYPE_WIFI_2_4G,
	BACKHAUL_TYPE_WIFI_5G,
	BACKHAUL_TYPE_WIFI_5G_2,
	BACKHAUL_TYPE_WIFI_6G,
	BACKHAUL_TYPE_MAX	/* invalid type */
};

enum work_mode
{
	WORK_MODE_UNCONFEANENT,
	WORK_MODE_AGENT,   //corresponding to RE
	WORK_MODE_LIMBO,	//corresponding to HAP
	WORK_MODE_CONTROLLER,  //corresponding to FAP
	WORK_MODE_INVALID
};

enum AUTO_TO_CONTROLLER_TYPE
{
	TYPE_ONLINE = 1, /* should be 2^n */
	TYPE_LOGIN = 2 /* should be 2^n */
};


typedef struct _EASYMESH_APSD_BACKHAUL_CHAGED_MSG{
	unsigned char backhaul_status[BACKHAUL_TYPE_MAX];
}EASYMESH_APSD_BACKHAUL_CHAGED_MSG;

typedef struct _EASYMESH_APSD_RECONFIGURE_MSG{
	unsigned int mode;
}EASYMESH_APSD_RECONFIGURE_MSG;

typedef enum 
{	
	EASYMESH_BSS_TYPE_PRIMARY				= 1,  /* Fronthaul Primary BSS */
	EASYMESH_BSS_TYPE_GUEST 				= 2,  /* Fronthaul Guest BSS, such as Multi SSID and Guest Network. */
	EASYMESH_BSS_TYPE_BSTA					= 3,  /* bSTA */
	EASYMESH_BSS_TYPE_BAKCHAULR1			= 4,  /* Backhaul BSS R1*/
	EASYMESH_BSS_TYPE_BAKCHAULR2			= 5,  /* Backhaul BSS R2*/
	EASYMESH_BSS_TYPE_PUBLIC 				= 6,  /* Backhaul Pre-config BSS */
	EASYMESH_BSS_TYPE_COEXIST				= 7,  /* Fronahaul Primary BSS and Backhaul BSS Co-existence */
	EASYMESH_BSS_TYPE_MAX		
}EASYMESH_BSS_TYPE;


typedef struct _EASYMESH_AUTO_CONFIG_MSG
{
	unsigned char		enable;
	unsigned char		support;
	char				band[7];
	char				ssid[33];
	char				modeEnabled[25]; 
	char				encryptionMode[11]; 
	char				networkKey[65];
	EASYMESH_BSS_TYPE	bssType;
	unsigned char		tsNeedSet;
	unsigned short		vlan;
#ifdef INCLUDE_WIFI_MSSID
	unsigned char		mssidIndex;
#endif /*INCLUDE_WIFI_MSSID*/
} EASYMESH_AUTO_CONFIG_MSG;

typedef struct _EASYMESH_CHANNNEL_SELECT_MSG
{
	char                band[7];
	char                bandWidth[9];
	unsigned int        channel;	
#if defined(INCLUDE_EASYMESH_CERTIFICATION)
	unsigned char       transmitPowerLimit; /* Transmit Power Limit EIRP per 20 MHz bandwidth representing the nominal transmit power limit for this radio. */
#endif /*INCLUDE_EASYMESH_CERTIFICATION*/
} EASYMESH_CHANNNEL_SELECT_MSG;

typedef struct _EASYMESH_WORKMODE_CHANGE_MSG
{
	char				currMode[12];
	char				newMode[12];	
} EASYMESH_WORKMODE_CHANGE_MSG;

typedef struct _EASYMESH_SET_BACKHUAL_STA_MSG
{
	char				band[7];
	unsigned char		enable;
	unsigned char		newParent;	/* set backhaul sta connect to new parent */
	int					channel;
	char				bssid[18];
	char				ssid[33];
	char				key[65];
} EASYMESH_SET_BACKHUAL_STA_MSG;


typedef struct _EASYMESH_APDEVICE_ACTION_INFO_MSG
{
	unsigned char inUse;
	unsigned char macAddress[6];	
} EASYMESH_APDEVICE_ACTION_INFO_MSG;

typedef struct _EASYMESH_APDEVICE_ACTION_MSG
{
	EASYMESH_APDEVICE_ACTION_INFO_MSG info[16];
} EASYMESH_APDEVICE_ACTION_MSG;


typedef struct GNU_PACKED _EASYMESH_ASSOC_TS_MSG
{
	char ifname[IFNAMSIZ];
	unsigned short vid;
} EASYMESH_ASSOC_TS_MSG;

typedef struct	_EASYMESH_CONTROLLER_FIXED_MSG
{
	char		controllerMac[18];
} EASYMESH_CONTROLLER_FIXED_MSG;

typedef struct  _EASYMESH_TS_POLICY_ENTRY
{
	char ssid[33]; /* ssid name */
	unsigned short vlanId;
} EASYMESH_TS_POLICY_ENTRY;

typedef struct  _EASYMESH_TS_POLICY_MSG
{
	unsigned char defaultPCP;
	unsigned short primaryVlanId; /* Primary VLAN ID. */
	unsigned char tsEntryNum;
	EASYMESH_TS_POLICY_ENTRY tsEntryList[MAX_TS_POLICY];
} EASYMESH_TS_POLICY_MSG;

typedef struct  _EASYMESH_MULTIAP_POLICY_MSG
{
	unsigned char tsPolicySet;
	EASYMESH_TS_POLICY_MSG tsPolicy;
} EASYMESH_MULTIAP_POLICY_MSG;

#ifdef INCLUDE_WIFI_BCM
typedef struct  _EASYMESH_BACKHAUL_CHANGED_MSG
{
	char ifname[IFNAMSIZ];
	unsigned int action;
} EASYMESH_BACKHAUL_CHANGED_MSG;
#endif

#endif/*INCLUDE_EASYMESH*/

#ifdef INCLUDE_MESH_TETHER_WEBVIEW
#define TPAPP_WEBVIEW_TOKEN_LEN 32
#define TPAPP_WEBVIEW_TOKEN_BUFLEN (TPAPP_WEBVIEW_TOKEN_LEN+1)
#define TPAPP_WEBVIEW_COOKIE_LEN 32
#define TPAPP_WEBVIEW_COOKIE_BUFLEN (TPAPP_WEBVIEW_COOKIE_LEN+1)
#define TPAPP_WEBVIEW_JWT_LEN 512
#define TPAPP_WEBVIEW_JWT_BUFLEN (TPAPP_WEBVIEW_JWT_LEN+1)

typedef struct _MESH_WEBVIEW_PROV_DATA
{
	char user_pwd[65];
	char wl_bh_ssid[33];
	char wl_bh_key[65];
	char wl_bh_sec[25];
	char wl_bh_encrypt[11];
	unsigned int channel_24g;
	unsigned int channel_5g;
} MESH_WEBVIEW_PROV_DATA;
typedef enum
{
	TPAPP_WEBVIEW_MSG_AUTH_NULL = 0,
	TPAPP_WEBVIEW_MSG_AUTH_COOKIE,
	TPAPP_WEBVIEW_MSG_AUTH_JWT
} TPAPP_WEBVIEW_MSG_AUTHTYPE;

typedef struct
{
	char webViewToken[TPAPP_WEBVIEW_TOKEN_BUFLEN];
	char webViewCookie[TPAPP_WEBVIEW_COOKIE_BUFLEN];
} TPAPP_WEBVIEW_MSG_COOKIEINFO;

typedef struct
{
	TPAPP_WEBVIEW_MSG_AUTHTYPE authType;
	union {
		TPAPP_WEBVIEW_MSG_COOKIEINFO cookieInfo;
		char webViewJwt[TPAPP_WEBVIEW_JWT_BUFLEN];
	} data;
} TPAPP_WEBVIEW_MSG;
#endif /* INCLUDE_MESH_TETHER_WEBVIEW */

#ifdef INCLUDE_ONLINE_DETECTION
typedef enum
{
	NSLOOKUP_PING_TYPE_ANY = 0,
	NSLOOKUP_PING_TYPE_ALL
} NSLOOKUP_PING_TYPE;

/* same as DEV2_DETECTIONTHREAD_OBJ */
typedef struct
{
	char   		opCode[6];	/* OpCode */
	char   		name[9];	/* Name */
	char   		type[5];	/* Type */
	unsigned int   	SN;	/* SN */
	char   		mode[9];	/* Mode */
	unsigned short   	onlineTestInterval;	/* OnlineTestInterval */
	unsigned short   	offlineTestInterval;	/* OfflineTestInterval */
} DETECTIONTHREAD_CFG;

/* same as DEV2_ONLINEDETECTION_CONFIG_OBJ */
typedef struct
{
	char   		interface[48];	/* Interface */
	char   		ifname[16];	/* Ifname */
	unsigned char   	defaultGw;	/* DefaultGw */
	unsigned char   	nslookup;	/* Nslookup */
	char   		hostnames[512];	/* Hostnames */
	NSLOOKUP_PING_TYPE	nslookupType;
	unsigned short   	nslookupTrytimes;	/* NslookupTrytimes */
	unsigned short   	nslookupTimeout;	/* NslookupTimeout */
	unsigned char   	pingCheck;	/* PingCheck */
	NSLOOKUP_PING_TYPE	pingType;
	unsigned short   	pingTrytimes;	/* PingTrytimes */
	unsigned short   	pingTimeout;	/* PingTimeout */
	union
	{
		struct
		{
			char   		pingIPv4Addrs[49];	/* PingIPv4Addrs */
			char   		v4Gateway[16];	/* V4Gateway */
			char   		v4DnsServers[92];	/* V4DnsServers */
		}v4;
#ifdef INCLUDE_IPV6
		struct
		{
			char   		pingIPv6Addrs[139];	/* PingIPv6Addrs */
			char   		v6Gateway[46];	/* V6Gateway */
			char   		v6DnsServers[92];	/* V6DnsServers */
		}v6;
#endif /* INCLUDE_IPV6 */
	}info;
} ONLINEDETECTION_CFG;

/* same as DEV2_ONLINEDETECTION_RESULT_OBJ */
typedef struct
{
	unsigned char   	received;	/* Received */
	unsigned char   	online;	/* Online */
} ONLINEDETECTION_RESULT;

typedef struct _ONLINE_DETECTION_MSG
{
	DETECTIONTHREAD_CFG threadCfg;
	ONLINEDETECTION_CFG detectionCfg;
	ONLINEDETECTION_RESULT detectionResult;
}ONLINE_DETECTION_MSG;
#endif /* INCLUDE_ONLINE_DETECTION */

#ifdef INCLUDE_EASYMESH_MULTI_UPGRADE
typedef enum
{
    MU_FW_DOWN_SUCCESS = 0,
    MU_FW_DOWN_FAILED
} FW_DOWN_RESULT;

typedef enum
{
    MESH_UPGRADE_ROLE_NORMAL = 0,
    MESH_UPGRADE_ROLE_ELECTED
} MESH_UPGRADE_ROLE;

// MU_MSG used for Requset msg from meshMonitor to mapController, and Response msg from mapController to mesh Monitor
typedef struct _MU_MSG
{
    MAC_ADDR alMac;
    uint8_t type;
    uint8_t isAgentOnController;
    uint16_t jsonLen;
    char jsonPayload[MU_BUFFLEN];
} MU_MSG;

typedef struct _MU_TRIGGER_ELEMENT
{
	uint8_t isController; // 1 for Controller, 0 for Agent
	uint8_t isElected; // 1 for Elected Agent, 0 for Normal Agent
	MAC_ADDR alMac;
} MU_TRIGGER_ELEMENT;

/* The element of MU_TRIGGER_MSG require same type of device should 
 * put together, just like:
* apDev[0] - HC220, elected agent
* apDev[1] - HC220, normal agent
* apDev[2] - HX220, elected agent
*/
typedef struct _MU_TRIGGER_MSG
{
    MU_TRIGGER_ELEMENT apDev[MAX_AGENT];
    unsigned int isLocalUpgrade;
    unsigned int isCwmpUpgrade;
} MU_TRIGGER_MSG;

// Agent Inner Message Type
typedef struct _MU_AGENT_FW_DOWN_RESULT_MSG
{
    FW_DOWN_RESULT result; // 0 for success, 1 for failed
    unsigned int progress; // download percentage
    unsigned int fileSize;
    char filePath[MAX_FILE_NAME_LEN];
} MU_AGENT_FW_DOWN_RESULT_MSG;

typedef struct _MU_AGENT_FW_DOWN_RSP_MSG
{
    unsigned int fileSize;
    char filePath[MAX_FILE_NAME_LEN];
} MU_AGENT_FW_DOWN_RSP_MSG;

typedef struct _MU_AGENT_UPGRADE_RESULT_MSG
{
    unsigned int ret;
} MU_AGENT_UPGRADE_RESULT_MSG;

typedef struct _MU_AGENT_UPGRADE_REQ_MSG
{
    MESH_UPGRADE_ROLE muRole; // Is Elected AP or Normal AP?
    char path[MAX_FILE_NAME_LEN];
    unsigned int fileSize;
    unsigned int isLocalUpgrade;
    unsigned int isCwmpUpgrade;
} MU_AGENT_UPGRADE_REQ_MSG;

typedef struct _MU_AGENT_DOWN_FIRM_MSG
{
    unsigned int isLocalUpgrade;
    unsigned int isCwmpUpgrade;
} MU_AGENT_DOWN_FIRM_MSG;

typedef struct _MU_AGENT_CHECK_FIRM_PROGRESS_REQ_MSG
{
    unsigned int isLocalUpgrade;
    unsigned int isCwmpUpgrade;
} MU_AGENT_CHECK_FIRM_PROGRESS_REQ_MSG;

typedef struct _MU_AGENT_RSP_PROGRESS_MSG
{
    uint8_t progress;
} MU_AGENT_RSP_PROGRESS_MSG;

typedef struct _MU_AGENT_REQ_DISPATCH_MSG
{
    uint8_t numOfAgent;
    unsigned int fileSize;
    unsigned int isLocalUpgrade;
    unsigned int isCwmpUpgrade;
} MU_AGENT_REQ_DISPATCH_MSG;

typedef struct _MU_AGENT_REQ_RECEIVE_MSG
{
} MU_AGENT_REQ_RECEIVE_MSG;

typedef struct _MU_AGENT_RSP_SRVPORT_MSG
{
    uint16_t port;
} MU_AGENT_RSP_SRVPORT_MSG;

typedef struct _MU_SYNC_RESULT_MSG 
{
    uint8_t isController;
    char lanMac[18];
    int32_t status;
    uint8_t progress;
    unsigned int isLocalUpgrade;
    unsigned int isCwmpUpgrade;
} MU_SYNC_RESULT_MSG;

typedef struct _MU_HTTP_AUTH_MSG
{
	char key[17];
	MAC_ADDR alMac;
} MU_HTTP_AUTH_MSG;

typedef struct _MU_CLEAN_UPGRADE_STATUS_MSG
{
    unsigned int isLocalUpgrade;
    unsigned int isCwmpUpgrade;
} MU_CLEAN_UPGRADE_STATUS_MSG;

#ifdef INCLUDE_RECORD_UPGRADE_INFO
typedef struct _MU_SYNC_UPGRADE_STATUS_MSG 
{
	UPGRADE_STATUS upStatus;
	uint8_t saveFlash;
} MU_SYNC_UPGRADE_STATUS_MSG;
#endif /*INCLUDE_RECORD_UPGRADE_INFO*/

typedef struct _MU_PARSE_EVENT_MSG
{
	uint32_t devIndex;
	uint8_t eventType;
	char firmLoc[MAX_FILE_NAME_LEN];
	uint32_t firmSize;
	uint8_t progress;
	uint16_t port;
	uint16_t errCode;
} MU_PARSE_EVENT_MSG;

#endif /*INCLUDE_EASYMESH_MULTI_UPGRADE*/

#ifdef INCLUDE_QOE
#define MAX_RSP_CLIENT_NUMBER 32
#define LAN_MAC_LENGTH 18
#define MAX_PROFILE_COUNT 5 // currently we have 3 profiles, set to 5 here for future use. 20211229
typedef struct _QOE_RSP_PROFILE_MSG
{
	char name[64];	/* current profile name */
	uint32_t enable;							/* current profile enable status */
	uint32_t collectingInterval;				/* current profile collecting interval */
	uint32_t collectingInterval_P1;				/* ap or client collecting interval */
} QOE_RSP_PROFILE_MSG;

typedef struct _QOE_REQ_DEVDATA_MSG
{
	uint8_t profile;
	char interface[16];
	uint8_t isController;
} QOE_REQ_DEVDATA_MSG;

typedef struct _QOE_RSP_DEVDATA_MSG
{
	uint8_t profile;
	uint32_t bytesReceived;
	uint32_t bytesSent;
	uint32_t downloadSpeed;
	uint32_t maxJitter;
	uint32_t maxLatency;
	uint32_t wanConnect;
	time_t timeStamp;
} QOE_RSP_DEVDATA_MSG;

typedef struct _QOE_REQ_CLIDATA_MSG
{
	uint8_t profile;
} QOE_REQ_CLIDATA_MSG;

typedef struct _QOE_RSP_CLIDATA_MSG
{
	uint8_t profile;
	uint8_t nr;
	char lanMac[MAX_RSP_CLIENT_NUMBER][LAN_MAC_LENGTH];
	int32_t networkReadyTime[MAX_RSP_CLIENT_NUMBER];
} QOE_RSP_CLIDATA_MSG;

typedef struct _QOE_BULKDATA_PROFILE_RECONFIG_MSG
{
	unsigned int instance;
	char oldProtocol[16];
} QOE_BULKDATA_PROFILE_RECONFIG_MSG;

#endif /* INCLUDE_QOE */

#ifdef INCLUDE_TR369
#define SUBS_CREATE_DATA_MAX_LEN	(32)
#define SUBS_RECIPIENT_MAX_LEN		(256)
#define SUBS_REFERENCELIST_MAX_LEN	(256)
#define SUBS_NOTIFTYPE_MAX_LEN		(32)
#define SUBS_ID_MAX_LEN				(64)
#define OBJ_PATH_MAX_LEN			(256)

typedef enum _LOCALAGENT_SUBSCRIPTION_MSG_TYPE
{
	SUBSCRIPTION_MSG_TYPE_ADD = 0,
	SUBSCRIPTION_MSG_TYPE_DEL = 1,
	SUBSCRIPTION_MSG_TYPE_MODIFY =2,
	SUBSCRIPTION_MSG_TYPE_NUM
}LOCALAGENT_SUBSCRIPTION_MSG_TYPE;

typedef struct _LOCALAGENT_SUBSCRIPTION_MSG
{
	int32_t type;
	int32_t instance;
	uint8_t enable;
	uint32_t timeToLive;
	char createDate[SUBS_CREATE_DATA_MAX_LEN];
	char recipient[SUBS_RECIPIENT_MAX_LEN];
	char ID[SUBS_ID_MAX_LEN];
	uint8_t notifRetry;
	uint32_t notifExpiration;
	char referenceList[SUBS_REFERENCELIST_MAX_LEN];
	char notifType[SUBS_REFERENCELIST_MAX_LEN];
} LOCALAGENT_SUBSCRIPTION_MSG;

typedef enum _USPA_RECONFIG_TYPE
{
	USPA_RECONFIG_TYPE_SET_OBJ	= 0, /* Set(modify or add) object. */
	USPA_RECONFIG_TYPE_DEL_OBJ	= 1, /* Delele object. */
	USPA_RECONFIG_TYPE_NUM
} USPA_RECONFIG_TYPE;

typedef struct _USPA_RECONFIG_MSG
{
	USPA_RECONFIG_TYPE type;
	char objFullPath[OBJ_PATH_MAX_LEN + 1];
} USPA_RECONFIG_MSG;

#endif /* INCLUDE_TR369 */

typedef enum _PRE_NETWORKING_AWND_STATUS
{
	AWND_STATUS_PAUSE_BY_WPS = 0,			/*wps onboarding stop scanning*/
	AWND_STATUS_PAUSE_BY_CONFIGURATION = 1,	/*recv configuration from controller, stop scanning*/
	AWND_STATUS_RESTART,					/*wps onboarding error or timeout, restsrt scanning*/
	AWND_STATUS_SKIP_BY_CONTROLLER,			/*skkip this entry, scanning next */
	AWND_STATUS_ETHERNET_CHANGE,			/*ethernet status change*/
	AWND_STATUS_CHANGE_TO_CONTROLLER,		/*mesh role change to controller*/
	AWND_STATUS_CHANGE_TO_CONFAGENT,		/*mesh role change to controller*/
	AWND_STATUS_RELOAD_CONFIG,				/*reload config*/
	AWND_STATUS_END
} PRE_NETWORKING_AWND_STATUS;
	
typedef struct _PRE_NETWORKING_AWND_STATUS_CHANGE_MSG
{
	int status;
} PRE_NETWORKING_AWND_STATUS_CHANGE_MSG;

typedef struct _AGINET_BACKHAULAP_STATUS_CHANGE_MSG
{
	unsigned char enable;
} AGINET_BACKHAULAP_STATUS_CHANGE_MSG;

typedef struct _EASYMESH_CHANGE_TO_CONTROLLER_MSG
{
	int type;
} EASYMESH_CHANGE_TO_CONTROLLER_MSG;

#ifdef INCLUDE_CONTAINER
#define DEFALUT_TR157_REQID 	10 
#define CWMP_TR157_REQID 		20
#define HTTP_TR157_REQID 		30
#define CLI_TR157_REQID 		40

#define FILE_MSG_PATH_LEN		256

typedef struct
{
	char  	version[128];
	char	UUID[37];
	char	path[FILE_MSG_PATH_LEN];
	unsigned int 	checksum;
}DU_APP_MSG;


typedef struct
{
	char   	EUID[64];
	char   	name[32];
	char   	vendor[32];
	char   	version[32];
	char 	status[20];
}EU_APP_STATUS_CHANGE_MSG;


#endif /* INCLUDE_CONTAINER */

#ifdef INCLUDE_SMART_HOME_V2

typedef enum
{
	SMART_HOME_USP_REQID = 0,
	SMART_HOME_CWMP_REQID,
	SMART_HOME_WEB_REQID,
	SMART_HOME_AGINETAPP_REQID,
	SMART_HOME_DEFALUT_REQID,
}SMHM_REQ_ID;

typedef enum
{
	SMART_HOME_INTERNEL = 0,
	SMART_HOME_USP,
	SMART_HOME_CWMP,
	SMART_HOME_WEB,
	SMART_HOME_AGINETAPP
}SMHM_RET_MSG_FLAG;


typedef enum
{
	CREATE_EE = 0,
	START_EE,
	CONFIG_EE,
	UPDATE_EE,
	UPLOAD_LOG_EE,
	STOP_EE,
	RESET_EE,
	DESTROY_EE
}EE_OP_TYPE;


typedef enum
{
	INSTALL_DU,
	UPDATE_DU,
	UNINSTALL_DU
}DU_OP_TYPE;

typedef enum
{
	START_EU,
	STOP_EU
}EU_OP_TYPE;


typedef struct
{
#ifdef INCLUDE_TR369
	uint16_t			reqInstance;
#endif /* INCLUDE_TR369 */
	uint16_t			eeInstance;
	uint16_t			cpuQuota; /* Percent */;
	int		 			appSize; /* KB */
	int 				logSize;
	int 				cfgSize;
	int					memLimit; /* KB */
	SMHM_RET_MSG_FLAG	internal; /* When internal is SMART_HOME_INTERNEL, that means obuspa,cwmp,aginet app or httpd 
									doesn't need send msg back to server*/
	SMHM_REQ_ID			reqID;
	char				startTime[SMART_HOME_TIME_LEN];
	char				ctName[32];
}CMGR_EE_CREATE_MSG;


typedef struct
{
#ifdef INCLUDE_TR369
	uint16_t			reqInstance;
#endif /* INCLUDE_TR369 */
	uint16_t			eeInstance;
	SMHM_RET_MSG_FLAG	internal; /* When internal is SMART_HOME_INTERNEL, that means obuspa,cwmp,aginet app or httpd 
									doesn't need send msg back to server*/
	SMHM_REQ_ID			reqID;
	char				startTime[SMART_HOME_TIME_LEN];
	char 				ctName[32];
}CMGR_EE_DESTROY_MSG;


typedef struct
{
#ifdef INCLUDE_TR369
	uint16_t			reqInstance;
#endif /* INCLUDE_TR369 */
	uint16_t			eeInstance;
	SMHM_RET_MSG_FLAG	internal; /* When internal is SMART_HOME_INTERNEL, that means obuspa,cwmp,aginet app or httpd 
									doesn't need send msg back to server*/
	SMHM_REQ_ID			reqID;
	char				startTime[SMART_HOME_TIME_LEN];
	char 				ctName[32];
}CMGR_EE_START_MSG;

typedef struct
{
#ifdef INCLUDE_TR369
	uint16_t			reqInstance;
#endif /* INCLUDE_TR369 */
	uint16_t			eeInstance;
	SMHM_RET_MSG_FLAG	internal; /* When internal is SMART_HOME_INTERNEL, that means obuspa,cwmp,aginet app or httpd 
									doesn't need send msg back to server*/
	SMHM_REQ_ID			reqID;
	char				startTime[SMART_HOME_TIME_LEN];
	char 				ctName[32];
}CMGR_EE_STOP_MSG;

typedef struct
{
	uint8_t				cfgPatition; /* config patition or not */
	uint16_t			usb;
	uint16_t			cpuQuota; /* Percent */;
#ifdef INCLUDE_TR369
	uint16_t			reqInstance;
#endif /* INCLUDE_TR369 */
	uint16_t			eeInstance;
	int		 			appSize; /* KB, EE's status must be Disabled */
	int 				cfgSize; /* KB, EE's status must be Disabled */
	int 				logSize; /* KB, EE's status must be Disabled */
	int					memLimit; /* KB */
	int					wanrx;
	int					wantx;
	int					lanrx;
	int					lantx;
	SMHM_RET_MSG_FLAG	internal; /* When internal is SMART_HOME_INTERNEL, that means obuspa,cwmp,aginet app or httpd 
									doesn't need send msg back to server*/
	SMHM_REQ_ID			reqID;
	char				preStatus[10];
	char				startTime[SMART_HOME_TIME_LEN];
	char				ctName[32];
}CMGR_EE_CONFIG_MSG;

typedef struct
{
#ifdef INCLUDE_TR369
	uint16_t			reqInstance;
#endif /* INCLUDE_TR369 */
	SMHM_REQ_ID			reqID;
	char 				ctName[32];
}CMGR_EE_QUERY_MSG;

typedef struct
{
#ifdef INCLUDE_TR369
	uint16_t			reqInstance;
#endif /* INCLUDE_TR369 */
	SMHM_REQ_ID			reqID;
	char				startTime[SMART_HOME_TIME_LEN];
	char 				ctName[32];
}CMGR_EE_LOG_MSG;

typedef struct
{
#ifdef INCLUDE_TR369
	uint16_t			reqInstance;
#endif /* INCLUDE_TR369 */
	uint16_t			duInstance;
	SMHM_REQ_ID			reqID;
	char				ctName[32];
	char				startTime[SMART_HOME_TIME_LEN];
	char				duFilePath[SMART_HOME_DU_FILE_PATH_LEN];
}CMGR_DU_INSTALL_MSG;

typedef struct
{
#ifdef INCLUDE_TR369
	uint16_t			reqInstance;
#endif /* INCLUDE_TR369 */
	uint16_t			duInstance;
	SMHM_REQ_ID			reqID;
	char				startTime[SMART_HOME_TIME_LEN];
	char				ctName[32];
	char				duName[64];
}CMGR_DU_UNINSTALL_MSG;

typedef struct
{
#ifdef INCLUDE_TR369
	uint16_t			reqInstance;
#endif /* INCLUDE_TR369 */
	uint16_t			duInstance;
	SMHM_REQ_ID			reqID;
	char				startTime[SMART_HOME_TIME_LEN];
	char				ctName[32];
	char				duName[64];
	char				duFilePath[SMART_HOME_DU_FILE_PATH_LEN];
}CMGR_DU_UPDATE_MSG;

typedef struct
{
#ifdef INCLUDE_TR369
	uint16_t			reqInstance;
#endif /* INCLUDE_TR369 */
	uint16_t			eeInstance;
	int					faultCode;			
	SMHM_RET_MSG_FLAG	internal; /* When internal is SMART_HOME_INTERNEL, that means obuspa,cwmp,aginet app or httpd 
									doesn't need send msg back to server*/
	SMHM_REQ_ID			reqID;
	char				startTime[SMART_HOME_TIME_LEN];
}CMGR_EE_COM_MSG_RET;

typedef struct
{
#ifdef INCLUDE_TR369
	uint16_t			reqInstance;
#endif /* INCLUDE_TR369 */
	uint16_t			eeInstance;
	uint16_t			cpuQuota; /* Percent */;
	int					faultCode;			
	SMHM_RET_MSG_FLAG	internal; /* When internal is SMART_HOME_INTERNEL, that means obuspa,cwmp,aginet app or httpd 
									doesn't need send msg back to server*/
	SMHM_REQ_ID			reqID;
	int		 			appSize; /* KB */
	int 				cfgSize; /* KB */
	int 				logSize; /* KB */
	int 				memLimit; /* KB */
	char				startTime[SMART_HOME_TIME_LEN];
}CMGR_EE_CREATE_MSG_RET;


typedef struct
{
	int					status; /* 0 means disabled 1 means running */
	int 				appSize; /* KB */
	int 				appUsed; /* KB */
	int 				cfgSize; /* KB */
	int 				cfgUsed; /* KB */
	int 				logSize; /* KB */
	int 				logUsed; /* KB */
	int					memUsed; /* KB */
	uint64_t			cpuUsed; /* Nanosecond */
	char				ctName[32];
}CMGR_EE_QUERY_MSG_RET;

typedef struct
{
	uint8_t				cfgPatition; /* config patition or not */
#ifdef INCLUDE_TR369
	uint16_t			reqInstance;
#endif /* INCLUDE_TR369 */
	uint16_t			eeInstance;
	uint16_t			usb;
	uint16_t			cpuQuota; /* Percent */
	int		 			appSize; /* KB, EE's status must be Disabled */
	int 				cfgSize; /* KB, EE's status must be Disabled */
	int 				logSize; /* tmpfs KB */
	int					memLimit; /* KB */
	int					wanrx;
	int					wantx;
	int					lanrx;
	int					lantx;
	int					faultCode;
	SMHM_RET_MSG_FLAG	internal;/* When internal is SMART_HOME_INTERNEL,
									that means obuspa,cwmp,aginet app or httpd doesn't need send msg back to server*/
	SMHM_REQ_ID			reqID;
	char				preStatus[10];
	char				startTime[SMART_HOME_TIME_LEN];
	}CMGR_EE_CONFIG_MSG_RET;



typedef struct
{
	uint16_t			euInstance;
	EU_OP_TYPE			euOptype;
	char				ctName[32];
	char				cmd[SMART_HOME_EU_CMD_LEN];
}CMGR_EU_CMD;

typedef struct
{
#ifdef INCLUDE_TR369
	uint16_t			reqInstance;
#endif /* INCLUDE_TR369 */
	uint16_t			duInstance;
	int					faultCode;
	SMHM_REQ_ID			reqID;
	char				startTime[SMART_HOME_TIME_LEN];
}CMGR_DU_COM_MSG_RET;

typedef struct
{
#ifdef INCLUDE_TR369
	uint16_t			reqInstance;
#endif /* INCLUDE_TR369 */
	uint16_t			duInstance;
	int					faultCode;
	char				UUID[37];
	char				version[32];
	char				currentState[13];
	DU_OP_TYPE			optType;
	char				startTime[SMART_HOME_TIME_LEN];
	char				duRef[SMART_HOME_DU_FULLPATH_LEN];
	char				euRefList[512];
	char				eeRef[128];
}COS_DU_COM_MSG_RET;

typedef struct
{
#ifdef INCLUDE_TR369
	uint16_t			reqInstance;
#endif /* INCLUDE_TR369 */
	uint16_t			eeInstance;
	int					faultCode;
	EE_OP_TYPE			optType;
	char				startTime[SMART_HOME_TIME_LEN];
}COS_EE_COM_MSG_RET;

#endif /* INCLUDE_SMART_HOME_V2 */

#ifdef INCLUDE_IGMP_DIAG
typedef struct _IGMPDIAG_CFG_MSG
{
	char			diagnosticsState[15];
	char			channelAddr[22];
	unsigned int	channelPort;
	char			channelStatus[5];
	char			serverAddr[16];
	char			ifname[64];
	unsigned int	timeDelay;
	unsigned int	numJoins;
	unsigned int	numLeaves;
	unsigned int	joinDelay;
	int				timeOut;
	unsigned int	phase;
}IGMPDIAG_CFG_MSG;
#endif /* INCLUDE_IGMP_DIAG */


#ifdef INCLUDE_ACCESS_CTRL_EXT
typedef struct _HTTPD_USER_ACCESS_RIGHT_CHANGED_MSG
{
	/*
	 * role: check index in http_auth.h
	 * accessType: check index in http_filter.h
	 */
	unsigned short	role;
	unsigned char	localAccessCapable;
	unsigned char	remoteAccessCapable;
	char	allowed_LA_Protocols[32];
	char	allowed_RA_Protocols[32];
} HTTPD_USER_ACCESS_RIGHT_CHANGED_MSG;
#endif /*INCLUDE_ACCESS_CTRL_EXT*/

typedef struct _EASYMESH_CONFIG_SET_DONE_MSG
{
	int type;
} EASYMESH_CONFIG_SET_DONE_MSG;


/**************************************************************************************************/
/*											 FUNCTIONS											  */
/**************************************************************************************************/

/* 
 * fn		int msg_init(CMSG_FD *pMsgFd)
 * brief	Create an endpoint for msg
 *	
 * param[out]	pMsgFd - return msg descriptor that has been create	
 *
 * return	-1 is returned if an error occurs, otherwise is 0
 *
 * note 	Need call msg_cleanup() when you no longer use this msg which is created by msg_init()
 */
int msg_init(CMSG_FD *pMsgFd);


/* 
 * fn		int msg_srvInit(CMSG_ID msgId, CMSG_FD *pMsgFd)
 * brief	Init an endpoint as a server and bind a name to this endpoint msg	
 *
 * param[in]	msgId - server name	
 * param[in]	pMsgFd - server endpoint msg fd
 *
 * return	-1 is returned if an error occurs, otherwise is 0	
 */
int msg_srvInit(CMSG_ID msgId, CMSG_FD *pMsgFd);



/* 
 * fn		int msg_connSrv(CMSG_ID msgId, CMSG_FD *pMsgFd)
 * brief	Init an endpoint as a client and specify a server name	
 *
 * param[in]		msgId - server name that we want to connect	
 * param[in/out]	pMsgFd - client endpoint msg fd	
 *
 * return	-1 is returned if an error occurs, otherwise is 0
 */
int msg_connSrv(CMSG_ID msgId, CMSG_FD *pMsgFd);


/* 
 * fn		int msg_recv(const CMSG_FD *pMsgFd, CMSG_BUFF *pMsgBuff)
 * brief	Receive a message form a msg	
 *
 * param[in]	pMsgFd - msg fd that we want to receive message
 * param[out]	pMsgBuff - return recived message
 *
 * return	-1 is returned if an error occurs, otherwise is 0
 *
 * note		we will clear msg buffer before recv
 */
int msg_recv(const CMSG_FD *pMsgFd, CMSG_BUFF *pMsgBuff);


/* 
 * fn		int msg_send(const CMSG_FD *pMsgFd, const CMSG_BUFF *pMsgBuff)
 * brief	Send a message from a msg	
 *
 * param[in]	pMsgFd - msg fd that we want to send message	
 * param[in]	pMsgBuff - msg that we wnat to send
 *
 * return	-1 is returned if an error occurs, otherwise is 0
 *
 * note 	This function will while call sendto() if sendto() return ENOENT error
 */
int msg_send(const CMSG_FD *pMsgFd, const CMSG_BUFF *pMsgBuff);


/* 
 * fn		int msg_cleanup(CMSG_FD *pMsgFd)
 * brief	Close a message fd
 * details	
 *
 * param[in]	pMsgFd - message fd that we want to close		
 *
 * return	-1 is returned if an error occurs, otherwise is 0		
 */
int msg_cleanup(CMSG_FD *pMsgFd);


/* 
 * fn		int msg_connCliAndSend(CMSG_ID msgId, CMSG_FD *pMsgFd, CMSG_BUFF *pMsgBuff)
 * brief	init a client msg and send msg to server which is specified by msgId	
 *
 * param[in]	msgId -	server ID that we want to send
 * param[in]	pMsgFd - message fd that we want to send
 * param[in]	pMsgBuff - msg that we wnat to send
 *
 * return	-1 is returned if an error occurs, otherwise is 0	
 */
int msg_connCliAndSend(CMSG_ID msgId, CMSG_FD *pMsgFd, CMSG_BUFF *pMsgBuff);


/* 
 * fn		int msg_sendAndGetReply(CMSG_FD *pMsgFd, CMSG_BUFF *pMsgBuff)
 * brief	
 *
 * param[in]	pMsgFd - msg fd that we want to use
 * param[in/out]pMsgBuff - send msg and get reply
 * param[in]	timeSeconds - timeout in second
 *
 * return	-1 is returned if an error occurs, otherwise is 0	
 */
int msg_sendAndGetReplyWithTimeout(CMSG_FD *pMsgFd, CMSG_BUFF *pMsgBuff, int timeSeconds);

/* 
 * fn		int msg_reply_send(const CMSG_FD *pMsgFd, const CMSG_BUFF *pMsgBuff)
 * brief	Send a reply(2011) from a msg	
 *
 * param[in]	pMsgFd - msg fd that we want to send message	
 * param[in]	pMsgBuff - msg that we wnat to send
 *
 * return	-1 is returned if an error occurs, otherwise is 0
 *
 * note 	This function is copied from msg_send()
 *			This function is only used for msg_sendAndGetReplyWithTimeout reply(2011)
 *			and remove the voip
 */
int msg_reply_send(const CMSG_FD *pMsgFd, const CMSG_BUFF *pMsgBuff);

#ifdef INCLUDE_TEST_STRACE_CMM_MSG
/*
 * fn		int msg_startStrace(const CMSG_FD *pMsgFd)
 * brief	start strace to monitor program with specific CMSG_FD
 *
 * param[in]	pMsgFd - msg fd that we want to monitor
 *
 * return	-1 is returned if an error occurs, otherwise is 0
 */
int msg_startStrace(const CMSG_FD *pMsgFd);

/*
 * fn		int msg_startStrace(const CMSG_FD *pMsgFd)
 * brief	stop monitor program with specific CMSG_FD
 *
 * param[in]	pMsgFd - msg fd that we want to monitor
 *
 * return	-1 is returned if an error occurs, otherwise is 0
 */
int msg_stopStrace(const CMSG_FD *pMsgFd);

#define MSG_DEBUG_START_STRACE(pMsgFd) msg_startStrace(pMsgFd)
#define MSG_DEBUG_STOP_STRACE(pMsgFd)  msg_stopStrace(pMsgFd)

#else

#define MSG_DEBUG_START_STRACE(pMsgFd)
#define MSG_DEBUG_STOP_STRACE(pMsgFd)

#endif /* INCLUDE_TEST_STRACE_CMM_MSG */

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */
#endif /* __OS_MSG_H__ */

