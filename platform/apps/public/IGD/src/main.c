#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <arpa/inet.h>
#include <time.h>
#include <net/if.h>

#include <upnp.h>
#include "globals.h"
#include "config.h"
#include "gatedevice.h"
#include "util.h"
#include "pmlist.h"
#include <dirent.h>
#include <ctype.h>
#include "strFile.h"
#include <netinet/in.h>
#include <TimerThread.h>

#include <linux/if_ether.h>

#include <string.h>

#define _MACRO_TO_STR(x) #x
#define MACRO_TO_STR(x) _MACRO_TO_STR(x)

// Global variables
struct GLOBALS g_vars;

/*func prototype Added by LI CHENGLONG , 2011-Nov-11.*/
extern int exIntfChangedHandler(CMSG_BUFF *pCmsgBuff);
extern int syncMappingDatabase();

extern UBOOL32 switchOnUpnp(char *descDocUrl);
extern UBOOL32 switchOffUpnp();

/*some variables, Added by LI CHENGLONG , 2011-Nov-09.*/
int upnp_alreadyRunning = 0;
/* Ended by LI CHENGLONG , 2011-Nov-09.*/

extern UpnpDevice_Handle deviceHandle;

extern ithread_mutex_t DevMutex;

/* 
 * fn		static void exitHandler()
 * brief	upnp进程退出前的处理.
 *
 * return	void
 * retval	N/A	
 *
 * note	written by  11Nov11, LI CHENGLONG	
 */
static void exitHandler()
{

	switchOffUpnp();
	
	UpnpFinish();
	return;
}

/* 
 * fn		static void interrupt_handler()
 * brief	接收到中断和退出信号时的处理.	
 *
 *
 * return		void
 * retval		N/A
 *
 * note	written by  11Nov11, LI CHENGLONG	
 */
static void interrupt_handler()
{

	/* Added by LI CHENGLONG , 2011-Nov-03.*/
	switchOffUpnp();

	UpnpFinish();
	return;
}



/* 
 * fn		static UBOOL32 initUpnpEnv(char *intIpAddress)
 * brief	初始化upnp运行环境,初始化upnp库中的一些资源.	
 * details	将资源的分配等初始化剥离出来,初始化在upnpd进程运行中只执行一次.	
 *
 * param[in]	intIpAddress		internalInterface 的ip地址.
 *
 * return		UBOOL32
 * retval		FALSE		初始化upnpd运行环境失败,原因可能是资源不够.
 *				TRUE		初始化upnpd运行环境成功.
 *
 * note	written by  11Nov11, LI CHENGLONG	
 */
static UBOOL32 initUpnpEnv(char *intIpAddress)
{
	int ret;
	
	// Initialize UPnP SDK on the internal Interface
	trace(3, "Initializing UPnP SDK ... ");
	
	if ( (ret = UpnpInit(intIpAddress,1900) ) != UPNP_E_SUCCESS)
	{
		trace (1, "Error Initializing UPnP SDK on IP %s ",intIpAddress);
		trace (1, "  UpnpInit returned %d", ret);
		UpnpFinish();
		return FALSE;
	}
	trace(2, "UPnP SDK Successfully Initialized.");

	// Set the Device Web Server Base Directory
	trace(3, "Setting the Web Server Root Directory to %s",g_vars.xmlPath);
	if ( (ret = UpnpSetWebServerRootDir(g_vars.xmlPath)) != UPNP_E_SUCCESS )
	{
		trace (1, "Error Setting Web Server Root Directory to: %s", g_vars.xmlPath);
		trace (1, "  UpnpSetWebServerRootDir returned %d", ret); 
		UpnpFinish();
		return FALSE;
	}
	trace(2, "Succesfully set the Web Server Root Directory.");


	if (0 != (ret = syncMappingDatabase() ))
	{
		trace(1, "Error init sync mapping database.  Exiting ...");
		return FALSE;
	}


	return TRUE;
}



int main (int argc, char** argv)
{
	char descDocUrl[DESC_DOC_URL_LENGTH]; // http://ipaddr:port/randomPrefix/docName<null>
	char intIpAddress[16];     // Server internal ip address
	int ret, arg = 1, foreground = 0;
	struct in_addr inAddr;
	UBOOL32 upnpOn;
	char cmd[256];
	char mac[ETH_ALEN];
	char xmlPath[PATH_LEN];

	
 	/*for msg recv. Added by LI CHENGLONG , 2011-Nov-11.*/
	fd_set fds;
	int selRet;
	CMSG_FD fdUPnP;
	CMSG_BUFF msgBuff;
	UPNP_DEFAULT_GW_CH_MSG *pUPnPMsg = NULL;
 	/* Ended by LI CHENGLONG , 2011-Nov-11.*/

	/* Add by chz to delete the upnp entry from other program. 2012-12-24*/
	UPNP_DEL_MSG *pUpnpDelMsg = NULL;
	struct Upnp_Action_Request ca_event;
	IXML_Document actReq;
	IXML_Node xmlPort;
	IXML_Node xmlProtocol;
	IXML_Node xmlPortText;
	IXML_Node xmlProtocolText;
	char port[16];
	int tmpPort;
	/* end add */

	ret = initConfigFile();
	if (TRUE != ret)
	{
		trace(1, "init configfile error");
		exit(-1);
	}

	memset(&g_vars, 0, sizeof(g_vars));
	parseConfigFile(&g_vars);

	
 	/*parse cmdline options, Added by LI CHENGLONG , 2011-Nov-09.*/
	/*
	 * Each loop has two arg++, the first is for getting paramValue, and the second is getting paramName for next time loop
	 * Such as "upnpd -map 1 -nat 0", the first is for getting paramValue "1" for "-map",
	 * the second is for getting paramName "-nat" for next loop
	 */
	while(arg < argc)
	{
		// check for '-F' option
		if (strcmp(argv[arg], "-F") == 0)
		{
			foreground = 1;
			arg++;
		}
		else if (strcmp(argv[arg], "-L") == 0)
		{
			arg++;
			// Save interface names for later use
			strncpy(g_vars.intInterfaceName, argv[arg++], IFNAMSIZ);
		}
		else if (strcmp(argv[arg], "-W") == 0)
		{

			arg++;
			if (strcmp(argv[arg], "-en") == 0)
			{
				/*no wan interface now, Added by LI CHENGLONG , 2011-Nov-09.*/
				memset(g_vars.extInterfaceName, 0, sizeof(g_vars.extInterfaceName));
			}
			else
			{
				// Save interface names for later use
				strncpy(g_vars.extInterfaceName, argv[arg++], IFNAMSIZ);
			}
		}
		else if (strcmp(argv[arg], "-en") == 0)
		{
			arg++;
			if (atoi(argv[arg++]) > 0)
			{
				upnpOn = 1;
			}
			else
			{
				upnpOn = 0;
			}
		}
		else if (strcmp(argv[arg], "-nat") == 0)
		{
			arg++;
			if (atoi(argv[arg++]) > 0)
			{
				g_vars.natEnabled = 1;
			}
			else
			{
				g_vars.natEnabled = 0;
			}	
		}
		else if (strcmp(argv[arg], "-port") == 0)
		{
			arg++;
			tmpPort = atoi(argv[arg++]);
			
			if (tmpPort > 0 && tmpPort < 65536)
			{
				g_vars.webPort = tmpPort;
			}
			else
			{
				g_vars.webPort = IGD_DEFAULT_WEB_PORT;
			}
		}
		else if (strcmp(argv[arg], "-url") == 0)
		{
			arg++;
			if (strcmp(argv[arg], "-ma") != 0)
			{
				snprintf(g_vars.manufacterUrl, BUFLEN_128, "%s", argv[arg++]);
			}
			while(strcmp(argv[arg], "-ma") != 0)
			{	
				snprintf(g_vars.manufacterUrl, 
						 BUFLEN_128, 
						 "%s %s", 
						 g_vars.manufacterUrl, 
						 argv[arg++]);
			}
		}
		else if (strcmp(argv[arg], "-ma") == 0)
		{
			arg++;
			if (strcmp(argv[arg], "-mn") != 0)
			{
				snprintf(g_vars.manufacter, BUFLEN_64, "%s", argv[arg++]);
			}
			while(strcmp(argv[arg], "-mn") != 0)
			{	
				snprintf(g_vars.manufacter, 
						 BUFLEN_64, 
						 "%s %s", 
						 g_vars.manufacter, 
						 argv[arg++]);
			}
		}
		else if (strcmp(argv[arg], "-mn") == 0)
		{
			arg++;
			if (strcmp(argv[arg], "-mv") != 0)
			{
				snprintf(g_vars.modelName, BUFLEN_64, "%s", argv[arg++]);
			}
			while(strcmp(argv[arg], "-mv") != 0)
			{	
				snprintf(g_vars.modelName, 
						 BUFLEN_64, 
						 "%s %s", 
						 g_vars.modelName, 
						 argv[arg++]);
			}
		}
		else if (strcmp(argv[arg], "-mv") == 0)
		{
			arg++;
			if (strcmp(argv[arg], "-desc") != 0)
			{
				snprintf(g_vars.modelVer, BUFLEN_16, "%s", argv[arg++]);
			}
			while(strcmp(argv[arg], "-desc") != 0)
			{	
				snprintf(g_vars.modelVer, 
						 BUFLEN_16, 
						 "%s %s", 
						 g_vars.modelVer, 
						 argv[arg++]);
			}		
		}
		else if (strcmp(argv[arg], "-desc") == 0)
		{
			arg++;
			if (arg < argc)
			{
				snprintf(g_vars.description, BUFLEN_256, "%s", argv[arg++]);
			}
			while(arg < argc)
			{	
				snprintf(g_vars.description, 
						 BUFLEN_256, 
						 "%s %s", 
						 g_vars.description, 
						 argv[arg++]);
			}		
		}
#ifdef WAN_DUAL_ACCESS
		else if (strcmp(argv[arg], "-P") == 0)
		{
			arg++;
			if (strcmp(argv[arg], "-map") == 0)
			{
				memset(g_vars.primInterfaceName, 0, sizeof(g_vars.primInterfaceName));
			}
			else
			{
				// Save primInterfaceName names for later use
				strncpy(g_vars.primInterfaceName, argv[arg++], IFNAMSIZ);
			}
		}
#endif /* WAN_DUAL_ACCESS */
		else if (strcmp(argv[arg], "-map") == 0)
		{
			arg++;
			if (atoi(argv[arg++]) > 0)
			{
				g_vars.portMappingEnabled = 1;
			}
			else
			{
				DeleteAllPortMappings();
				g_vars.portMappingEnabled = 0;
			}
		}
		else if (strcmp(argv[arg], "-cwmp") == 0)
		{
			arg++;
			tmpPort = atoi(argv[arg++]);

			if (tmpPort > 0 && tmpPort < 65536)
			{
				g_vars.portCwmpUsed = tmpPort;
			}
			else
			{
				g_vars.portCwmpUsed = IGD_NO_CWMP_PORT;
			}
		}
		else if (strcmp(argv[arg], "-stun") == 0)
		{
			arg++;
			tmpPort = atoi(argv[arg++]);

			if (tmpPort > 0 && tmpPort < 65536)
			{
				g_vars.portStunUsed = tmpPort;
			}
			else
			{
				g_vars.portStunUsed = IGD_NO_CWMP_PORT;
			}
		}
		else
		{
			printf("Usage: \n"\
				   "upnpd [-F] -L <internal ifname> -W <external ifname>"\
				   "-en <enable>  -nat <enable> -url <manufacterUrl> -ma <manufacter>"\
				   "-mn <moduleName> -mv <modelVer> -desc <description>");
			printf("  -F\tdon't daemonize\n");
			exit(0);		
		}
	}
 	/* Ended by LI CHENGLONG , 2011-Nov-09.*/


	snprintf(g_vars.deviceVersion, BUFLEN_16, "%s", g_vars.modelVer);
	snprintf(g_vars.deviceName, BUFLEN_512, "%s", g_vars.modelName);
	// Get the internal ip address to start the daemon on
	if (GetIpAddressStr(intIpAddress, g_vars.intInterfaceName) == 0) {
		fprintf(stderr, "Invalid internal interface name '%s'\n", g_vars.intInterfaceName);
		exit(EXIT_FAILURE);
	}

	
	if (getMacAddress(g_vars.intInterfaceName, mac) == FALSE) {
		fprintf(stderr, "Invalid internal interface name '%s'\n", g_vars.intInterfaceName);
		exit(EXIT_FAILURE);
	}

	getRandomPrefix(mac, g_vars.randomPrefix);

	snprintf(xmlPath, PATH_LEN, "%s/%s", g_vars.xmlPath, g_vars.randomPrefix);

	if((access(xmlPath, F_OK)) < 0 && mkdir(xmlPath, S_IRWXU | S_IRWXG | S_IRWXO) < 0)
	{
		fprintf(stderr, "Invalid xmlPath '%s'\n", xmlPath);
		exit(EXIT_FAILURE);
	}

#if 0
	if (!foreground) {
		struct rlimit resourceLimit = { 0, 0 };
		pid_t pid, sid;
		unsigned int i;

		// Put igd in the background as a daemon process.
		pid = fork();
		if (pid < 0)
		{
			perror("Error forking a new process.");
			exit(EXIT_FAILURE);
		}
		if (pid > 0)
			exit(EXIT_SUCCESS);

		// become session leader
		if ((sid = setsid()) < 0)
		{
			perror("Error running setsid");
			exit(EXIT_FAILURE);
		}

		// close all file handles
		resourceLimit.rlim_max = 0;
		ret = getrlimit(RLIMIT_NOFILE, &resourceLimit);
		if (ret == -1) /* shouldn't happen */
		{
		    perror("error in getrlimit()");
		    exit(EXIT_FAILURE);
		}
		if (0 == resourceLimit.rlim_max)
		{
		    fprintf(stderr, "Max number of open file descriptors is 0!!\n");
		    exit(EXIT_FAILURE);
		}	
		for (i = 0; i < resourceLimit.rlim_max; i++)
		    close(i);
	
		// fork again so child can never acquire a controlling terminal
		pid = fork();
		if (pid < 0)
		{
			perror("Error forking a new process.");
			exit(EXIT_FAILURE);
		}
		if (pid > 0)
			exit(EXIT_SUCCESS);
	
		if ((chdir("/")) < 0)
		{
			perror("Error setting root directory");
			exit(EXIT_FAILURE);
		}
	}
		umask(0);
#endif /* 0 */


	openlog("upnpd", LOG_CONS | LOG_NDELAY | LOG_PID | (foreground ? LOG_PERROR : 0), LOG_LOCAL6);


	if (!inet_aton(intIpAddress, &inAddr))
	{
		trace(1, "invalid addr.\n");
		exit(-1);
	}
	
	/* 网络字节序到主机字节序 */
	inAddr.s_addr = ntohl(inAddr.s_addr);
	
	/*根据manufacturer...更新gatedesc.xml...... Added by LI CHENGLONG , 2011-Nov-02.*/
	ret = upnpDescInit(g_vars.manufacterUrl,
				 	   g_vars.manufacter,
				       g_vars.modelName,
				       g_vars.modelVer,
     				   g_vars.deviceName,
					   g_vars.description,
				       g_vars.webPort,
				       inAddr.s_addr);
	
	if (ret != TRUE)
	{
		trace(1, "upnp desc init failed");
		exit(-1);
	}

	
 	/*signals handler, exit handler, Added by LI CHENGLONG , 2011-Nov-11.*/
	atexit(exitHandler);
	signal(SIGINT, interrupt_handler);
	signal(SIGTERM, interrupt_handler);
 	/* Ended by LI CHENGLONG , 2011-Nov-11.*/

#ifndef INCLUDE_UPNP_DYNAMIC_INIT
	ret = initUpnpEnv(intIpAddress);
	if (ret != TRUE)
	{
		trace(1, "upnp resources init failed");
		exit(-1);
	}
#endif /* INCLUDE_UPNP_DYNAMIC_INIT */

	if (upnpOn)
	{
#ifdef INCLUDE_UPNP_DYNAMIC_INIT
		ret = initUpnpEnv(intIpAddress);
		if (ret != TRUE)
		{
			trace(1, "upnp resources init failed");
			exit(-1);
		}
#endif /* INCLUDE_UPNP_DYNAMIC_INIT */

		ret = switchOnUpnp(descDocUrl);
		if (ret != TRUE)
		{
			trace(1, "register upnp failed");
			exit(-1);
		}
	}

	
	memset(&fdUPnP, 0 , sizeof(CMSG_FD));
	memset(&msgBuff, 0 , sizeof(CMSG_BUFF));
	msg_init(&fdUPnP);
	msg_srvInit(CMSG_ID_UPNP, &fdUPnP);
	
	while(TRUE)
	{	
		/*listen 消息 Added by LI CHENGLONG , 2011-Nov-02.*/
		memset(&msgBuff, 0 , sizeof(CMSG_BUFF));

		FD_ZERO(&fds);
   		FD_SET(fdUPnP.fd, &fds);
		pUPnPMsg = NULL;
		
	    selRet = select(fdUPnP.fd + 1, &fds, NULL, NULL, NULL); 
		if (selRet <= 0)
		{
			continue;
		}
		if(selRet > 0)
		{
			if (FD_ISSET(fdUPnP.fd, &fds))
       		{
				msg_recv(&fdUPnP, &msgBuff);
				/*开启或关闭UPNP的消息, Added  by  Li Chenglong , 11-NOV-3.*/
				if (msgBuff.type == CMSG_UPNP_ENABLE)
				{

					if (TRUE == ((IGD_UPNP_CTL *)(&(msgBuff.priv)))->enable)
					{
						/*received start upnp msg, by Li Chenglong*/
						upnpOn = 1;
						
					}
					else if (FALSE == ((IGD_UPNP_CTL *)(&(msgBuff.priv)))->enable)
					{
						/*received stop upnp msg, by Li Chenglong*/
						upnpOn = 0;
					}
				}	
				/*WAN 默认路由UPDATE消息, Added  by  Li Chenglong , 11-NOV-3.*/
				else if (msgBuff.type == CMSG_DEFAULT_GW_CH)
				{

					pUPnPMsg = (UPNP_DEFAULT_GW_CH_MSG *)msgBuff.content;

					/* 
					 * brief: Added by Li Chenglong, 11-NOV-3.
					 *		  IGD功能已启用时接收到UPDATE消息时的处理.
					 */
					if (1 == upnp_alreadyRunning)
					{
						if (strcmp(g_vars.extInterfaceName, pUPnPMsg->gwName) ||
#ifdef WAN_DUAL_ACCESS
							strcmp(g_vars.primInterfaceName, pUPnPMsg->gwL2Name) ||
#endif /* WAN_DUAL_ACCESS */
							(pUPnPMsg->upDown && (strcmp(g_vars.extIntfAddr, pUPnPMsg->gwAddr) 
#ifdef WAN_DUAL_ACCESS
							|| strcmp(g_vars.primIntfAddr, pUPnPMsg->gwL2Addr)
#endif /* WAN_DUAL_ACCESS */
						)))
						{
							exIntfChangedHandler(&msgBuff);
							strncpy(g_vars.extIntfAddr, pUPnPMsg->gwAddr, sizeof(g_vars.extIntfAddr));
#ifdef WAN_DUAL_ACCESS
							strncpy(g_vars.primIntfAddr, pUPnPMsg->gwL2Addr, sizeof(g_vars.primIntfAddr));
#endif /* WAN_DUAL_ACCESS */
						}
					}
					else
					{
						snprintf(g_vars.extInterfaceName,IFNAMSIZ, "%s", pUPnPMsg->gwName);
#ifdef WAN_DUAL_ACCESS
						snprintf(g_vars.primInterfaceName,IFNAMSIZ, "%s", pUPnPMsg->gwL2Name);
#endif /* WAN_DUAL_ACCESS */
						/*更新新的接口, Added  by  Li Chenglong , 11-Nov-3.*/
						g_vars.natEnabled = (pUPnPMsg->natEnabled) ? TRUE:FALSE;					
					}
#if 0
					/* send reply msg.Add by Chen Zexian, 20130107. */
					msgBuff.type = CMSG_REPLY;
					msg_reply_send(&fdUPnP, &msgBuff);
#endif
				}
				/* Add by chz to delete the upnp entry from other program. 2012-12-24*/
				else if (msgBuff.type == CMSG_UPNP_DEL_ENTRY)
				{
					pUpnpDelMsg = (UPNP_DEL_MSG *)(msgBuff.content);

					memset((void*)&ca_event, 0, sizeof(struct Upnp_Action_Request));
					strcpy(ca_event.ServiceID, "urn:upnp-org:serviceId:WANIPConn1");
					strcpy(ca_event.ActionName, "DeletePortMapping");

					memset((void*)&actReq, 0, sizeof(IXML_Document));
					ca_event.ActionRequest = &actReq;
					actReq.n.nodeName = "#document";
					actReq.n.nodeType = eDOCUMENT_NODE;
					
					memset((void*)&xmlPort, 0, sizeof(IXML_Node));
					ca_event.ActionRequest->n.firstChild = &xmlPort;
					xmlPort.nodeName = "NewExternalPort";
					xmlPort.nodeType = eELEMENT_NODE;

					memset((void*)&xmlPortText, 0, sizeof(IXML_Node));
					xmlPort.firstChild = &xmlPortText;
					sprintf(port, "%d", pUpnpDelMsg->port);
					xmlPortText.nodeValue = port;
					
					memset((void*)&xmlProtocol, 0, sizeof(IXML_Node));
					xmlPort.nextSibling = &xmlProtocol;
					xmlProtocol.nodeName = "NewProtocol";
					xmlProtocol.nodeType = eELEMENT_NODE;

					memset((void*)&xmlProtocolText, 0, sizeof(IXML_Node));
					xmlProtocol.firstChild = &xmlProtocolText;
					xmlProtocolText.nodeValue = pUpnpDelMsg->protocol;

					if (UPNP_E_SUCCESS != HandleActionRequest(&ca_event))
					{
						printf("Fail to delete the upnp entry.\n");
					}
				}
				/* end add */
				else if (msgBuff.type == CMSG_UPNP_LAN_IP_CH)
				{
					g_vars.webPort = msgBuff.priv;
					
					goto out;
				}
#ifdef INCLUDE_UPNP_PORTMAP_SWITCH
				else if (msgBuff.type == CMSG_UPNP_PORTMAPPING_ENABLE)
				{
					if (FALSE == (g_vars.portMappingEnabled =
							((UPNP_PORTMAPPING_ENABLE_MSG *)(&(msgBuff.content)))->enable))
					{
						DeleteAllPortMappings();
					}
				}
#endif /*INCLUDE_UPNP_PORTMAP_SWITCH*/
#ifdef INCLUDE_AVOID_UPNP_USE_CWMP_PORT
				else if (msgBuff.type == CMSG_UPNP_CWMP_PORT_CHANGE)
				{
					g_vars.portCwmpUsed = ((UPNP_CWMP_PORT_CHANGE_MSG *)(&(msgBuff.content)))->portCwmpUsed;
					g_vars.portStunUsed = ((UPNP_CWMP_PORT_CHANGE_MSG *)(&(msgBuff.content)))->portStunUsed;
				}
#endif /*INCLUDE_AVOID_UPNP_USE_CWMP_PORT*/
	    	}

			
			/*每次接收消息后,判断是否需要开启或关闭UPNP. Added  by  Li Chenglong , 11-NOV-3.*/
			/*如果消息是开启UPNP, Added  by  Li Chenglong , 11-NOV-3.*/
			if (upnpOn)
			{
				if (1 == upnp_alreadyRunning)
				{
					printf("Port mapping is already running!\n");
				}
				else
				{
					printf("Port mapping switch on!\n");

#ifdef INCLUDE_UPNP_DYNAMIC_INIT
					ret = initUpnpEnv(intIpAddress);
					if (ret != TRUE)
					{
						trace(1, "upnp resources init failed");
						exit(-1);
					}
#endif /* INCLUDE_UPNP_DYNAMIC_INIT */

					ret = switchOnUpnp(descDocUrl);

					if (TRUE != ret)
					{
					}
				}
			}
			/*如果消息是关闭UPNP, Added  by  Li Chenglong , 11-Aug-25.*/	
			else
			{
				if (0 == upnp_alreadyRunning)
				{
				}
				else
				{
					printf("Port mapping switch off!\n");
					
					ret = switchOffUpnp();
					if (TRUE != ret)
					{
					}
#ifdef INCLUDE_UPNP_DYNAMIC_INIT
					UpnpFinish();
#endif /* INCLUDE_UPNP_DYNAMIC_INIT */
				}
			}
	
		}		
		
	}

out:
	// Cleanup UPnP SDK and free memory
	DeleteAllPortMappings();
	ExpirationTimerThreadShutdown();

	UpnpUnRegisterRootDevice(deviceHandle);
	
	UpnpFinish();

	msg_cleanup(&fdUPnP);

	ithread_mutex_destroy(&DevMutex);

	snprintf(cmd, BUFLEN_256,
			 "upnpd  -L  %s  -W  %s  -en  %d"
#ifdef WAN_DUAL_ACCESS
			 " -P %s"
#endif /* WAN_DUAL_ACCESS */
			 "  -map %d -cwmp %d -stun %d" /*custom parameter*/
			 "  -nat %d -port %d  -url  %s  -ma  %s  -mn  %s  -mv  %s  -desc  %s&\n",
			 g_vars.intInterfaceName,
			 g_vars.extInterfaceName,
			 upnpOn,
#ifdef WAN_DUAL_ACCESS
			 g_vars.primInterfaceName,
#endif /* WAN_DUAL_ACCESS */
			 g_vars.portMappingEnabled,
			 g_vars.portMappingEnabled,
			 g_vars.portMappingEnabled,
			 g_vars.natEnabled,
			 g_vars.webPort,
			 g_vars.manufacterUrl,
			 g_vars.manufacter,
			 g_vars.modelName,
			 g_vars.modelVer,
			 g_vars.description
			);

	if (-1 == tp_system(cmd))
	{
		perror("Restart upnpd error:");
	}
	
	/*remove file upnppid, Added by LI CHENGLONG , 2011-Nov-01.*/
	
	// Exit normally
	return (0);
}
