/*  Copyright(c) 2009-2011 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 * file		os_log.h
 * brief		
 * details	
 *
 * author	wangwenhao
 * version	
 * date		16Jun11
 *
 * history \arg	1.0, 16Jun11, wangwenhao, create file
 */
#ifndef __OS_LOG_H__
#define __OS_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */


/**************************************************************************************************/
/*                                           DEFINES                                              */
/**************************************************************************************************/
/* increase the len of one log from 256 to 512, \
   because we add firewall log to system log and \
   the len of most of firewall log is about 300 */
#define LOG_CONTENT_LEN 512
#define LOG_MSG_SIZE	(524)
#define SYSLOGD_SHM_LOG_LEN	(32 * 1024)

#define SYSLOG_SEM_KEY				(25000 + 4)	/* "sysl" */
#define SYSLOG_SHARED_MEM_KEY		(15000 + 4)

#ifdef INCLUDE_ADVANCED_FIREWALL_LOG
	#define SYSLOGD_FIREWALL_SHM_LOG_LEN 		(INCLUDE_ADVANCED_FIREWALL_LOG_SHM_LEN * 1024)
	#define SYSLOG_FIREWALL_SEM_KEY				(25000 + 10)
	#define SYSLOG_FIREWALL_SHARED_MEM_KEY		(15000 + 10)
#endif /* INCLUDE_ADVANCED_FIREWALL_LOG */

#define CMMLOG_DEBUG(fmt, ...)		//printf("CMMLOG DBG: pid(%d) %s():%d, "fmt"\n", getpid(), __FUNCTION__, __LINE__, ##__VA_ARGS__)

#ifdef INCLUDE_REMOTE_LOG_STORE
#ifndef MACRO_TO_STR
#define _MACRO_TO_STR(x) #x
#define MACRO_TO_STR(x) _MACRO_TO_STR(x)
#endif
#define SYSLOG_REMOTE_URL			MACRO_TO_STR(INCLUDE_REMOTE_LOG_SRV)
#define SYSLOG_REMOTE_ADDR			0x8080808

#define PING_RST_FILE				"/var/run/ping_rst"
#define SYSLOG_FILE 				"/var/run/syslogs_"
#define MAX_SYSLOG_FILE_SIZE		(1 << 20)	//1MB

#define LOG_STORE_FMT_HDR			"%s %u %u %u "

#define LOG_WAN_DETECT_TMO			3		//timeout for wan status detect before each log send
#define STAT_WAN_DETECT_TMO			30		//timeout for wan status detect for main loop
#define URL_LEN						128		//support url length

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#ifndef min
#define min(x,y) (((x) < (y))?(x):(y))
#endif
void updateWanConnStatus(void);

#endif /* INCLUDE_REMOTE_LOG_STORE */

/**************************************************************************************************/
/*                                           TYPES                                                */
/**************************************************************************************************/
typedef enum
{
	LOG_EMERG = 0,
	LOG_ALERT = 1,
	LOG_CRIT = 2,
	LOG_ERROR = 3,
	LOG_WARN = 4,
	LOG_NOTICE = 5,
	LOG_INFORM = 6,
	LOG_DEBUG = 7
} LOG_SEVERITY;

typedef enum
{
	LOG_USER = 1,
	LOG_LOCAL0 = 16,
	LOG_LOCAL1 = 17,
	LOG_LOCAL2 = 18,
	LOG_LOCAL3 = 19,
	LOG_LOCAL4 = 20,
	LOG_LOCAL5 = 21,
	LOG_LOCAL6 = 22,
	LOG_LOCAL7 = 23
} LOG_FACILITY;

typedef enum
{
	LOG_SYSTEM = 0,
	LOG_INTERNET = 1,
	LOG_DHCPD = 2,
	LOG_HTTPD = 3,
	LOG_CMM_PPP = 4,
	LOG_OTHER = 5,
	LOG_DHCPC = 6,
	LOG_DSL	= 7,
	LOG_IGMP = 8,
	LOG_MOBILE = 9,
	LOG_VOIP = 10,
	LOG_KERNEL = 11,
	LOG_LTE = 12,
	LOG_VOLTE = 13,
	LOG_MESH = 14,
	LOG_SECURITY = 15,
	
#ifdef INCLUDE_TTNET_EXT_SYSLOG
	LOG_WIRELESS = 16,
#endif /* INCLUDE_TTNET_EXT_SYSLOG */
	LOG_CMGR = 17,

#ifdef INCLUDE_TTNET_EXT_SYSLOG
	LOG_PORTMAPPING = 18,
#endif /* INCLUDE_TTNET_EXT_SYSLOG */

#ifdef INCLUDE_ADVANCED_FIREWALL_LOG
	LOG_FIREWALL = 19,
#endif /* INCLUDE_ADVANCED_FIREWALL_LOG */

	LOG_MODULE_MAX
} LOG_MODULE;

typedef enum
{
	LOG_NORM_MSG = 0,
	LOG_CFG_MSG = 1
}LOG_TYPE;

typedef struct _LOG_MSG
{
	LOG_TYPE type;
	LOG_SEVERITY severity;
	LOG_MODULE module;
	char content[LOG_CONTENT_LEN];
} LOG_MSG;

typedef struct _LOG_CFG_BLOCK
{
	unsigned char logToRemote;	/* LogToRemote */
	unsigned int  remoteSeverity;	/* RemoteSeverity */
	char   		  serverIP[16];	/* ServerIP */
	unsigned int  serverPort;	/* ServerPort */
	unsigned int  facility;	/* Facility */
	unsigned char logToLocal;	/* LogToLocal */
	unsigned int  localSeverity;	/* LocalSeverity */
} LOG_CFG_BLOCK;

typedef struct
{
	unsigned int   	logHeadPos;	/* LogHeadPos */
	unsigned int   	logTailPos;	/* LogTailPos */	
}SYSLOGD_SHM_CONTROL_BLOCK;

/**************************************************************************************************/
/*                                           VARIABLES                                            */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                           FUNCTIONS                                            */
/**************************************************************************************************/
/* 
 * fn			void cmmlog(LOG_SEVERITY severity, LOG_MODULE module, const char *format, ...)
 *															 
 * brief		Recording the log to syslogd.
 *
 * param[in]	  severity   - the level of the log we want to record.
 * param[in]	  module     - which module the log from.
 * param[in]	  format    - the content we want to log.
 *
 * return		N/A
 */
void cmmlog(LOG_SEVERITY severity, LOG_MODULE module, const char *format, ...);

/* 
 * fn			void cmmlogcfg(LOG_CFG_BLOCK *pLogCfg)
 *															 
 * brief		Send the configure of log module to syslogd.
 *
 * param[in]	  pLogCfg   - the configure of the log module we want to send.
 *
 * return		N/A
 */
void cmmlogcfg(LOG_CFG_BLOCK *pLogCfg);

/* 
 * fn			void cmmlog_logRemote(unsigned int ip, unsigned short port, 
 									  const char *content, unsigned int len)
 *															 
 * brief		Recording the log to a remote syslog server.
 *
 * param[in]	  ip     - the ip address of the remote syslog server.
 * param[in]	  port   - the port of the remote syslog server.
 * param[in]	  content   - the content we want to log.
 * param[in]	  len   - the length of the log.
 *
 * return		N/A
 */
void cmmlog_logRemote(unsigned int ip, unsigned short port, const char *content, unsigned int len);


/* 
 * fn			int cmmlog_attachLogShm(void **pShmAddr)
 *															 
 * brief		attach to the shared memory of syslogd where stored the logs.
 *
 * param[out]	  pShmAddr     - the address of shared memory.
 *
 * return		0/-1
 */
int cmmlog_attachLogShm(void **pShmAddr);

/* 
 * fn			int cmmlog_detachLogShm(void *pShmAddr)
 *															 
 * brief		detach the shared memory of syslogd where stored the logs.
 *
 * param[in]	  pShmAddr     - the address of shared memory.
 *
 * return		0/-1
 */
int cmmlog_detachLogShm(void *pShmAddr);

#ifdef INCLUDE_TTNET_OFFLINE_REMOTE_LOG
/* 
 * fn			int cmmlogSendOfflineLog()
 *															 
 * brief		Recording all locale log to a remote syslog server.
 *
 * return		0/-1
 */
int cmmlogSendOfflineLog();
#endif /* INCLUDE_TTNET_OFFLINE_REMOTE_LOG */

#ifdef INCLUDE_ADVANCED_FIREWALL_LOG
/* 
 * fn			int cmmlog_attachFirewallLogShm(void **pShmAddr)
 *
 * brief		attach to the shared memory of syslogd where stored the firewall logs.
 *
 * param[out]	  pShmAddr     - the address of shared memory.
 *
 * return		0/-1
 */
int cmmlog_attachFirewallLogShm(void **pShmAddr);

/* 
 * fn			int cmmlog_detachFirewallLogShm(void *pShmAddr)
 *
 * brief		detach the shared memory of syslogd where stored the firewall logs.
 *
 * param[in]	  pShmAddr     - the address of shared memory.
 *
 * return		0/-1
 */
int cmmlog_detachFirewallLogShm(void *pShmAddr);
#endif /* INCLUDE_ADVANCED_FIREWALL_LOG */

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif	/* __OS_LOG_H__ */
