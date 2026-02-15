#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <upnp.h>
#include <string.h>
#include "globals.h"
#include "config.h"
#include "pmlist.h"
#include "gatedevice.h"
#include "util.h"


#if HAVE_LIBIPTC
#include "iptc.h"
#endif

/* UPNP Natloopback, yuanshang, 2013-04-17 */
#define IPSTR_LEN 16

#ifdef WAN_DUAL_ACCESS
#define PRV_IP_PREFIX "169.254."

#define IS_PRV_IP(ip) (strncmp((ip), PRV_IP_PREFIX, strlen(PRV_IP_PREFIX)) == 0)
#endif /* WAN_DUAL_ACCESS */

static int getIpAddress(char *iface_name, char *ip_addr, int len)
{
    int sockfd = -1;
    struct ifreq ifr;
    struct sockaddr_in *addr = NULL;

    memset(&ifr, 0, sizeof(struct ifreq));
    strcpy(ifr.ifr_name, iface_name);
    addr = (struct sockaddr_in *)&ifr.ifr_addr;
    addr->sin_family = AF_INET;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("create socket error!\n");
        return -1;
    }
    
    if (ioctl(sockfd, SIOCGIFADDR, &ifr) == 0) {
        strncpy(ip_addr, inet_ntoa(addr->sin_addr), len);
        close(sockfd);
        return 0;
    }

    close(sockfd);

    return -1;
}

static int getLanInterface(char *iface_name, UINT32 *lan_ip_addr, UINT32 *lan_ip_mask, char *ip_addr, int len)
{
	int ret = -1;
	struct in_addr in;
	int sockfd = -1;
	int i = 0, ifcNum = 0;
	struct ifreq *ifr;
	struct ifreq ifrMask;
	struct ifconf ifc;
	char *strBuf;
	int bufLen = 0;
	
	if (!inet_aton(ip_addr, &in))
	{
		perror("ip_addr error!\n");
		return -1;
	}
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("create socket error!\n");
		return -1;
	}
	
	bufLen = 16 * sizeof(struct ifreq);
	strBuf = malloc(bufLen);
	memset(strBuf, 0, bufLen);
		
	ifc.ifc_len = bufLen;
	ifc.ifc_buf = strBuf;
		
	if (ioctl(sockfd, SIOCGIFCONF, &ifc) == 0) {
		ifcNum = ifc.ifc_len/sizeof(struct ifreq);
		for(i=0; i < ifcNum; i++)
		{
			struct sockaddr_in *addrIp = NULL;
			struct sockaddr_in *addrMask = NULL;
			ifr = (struct ifreq *)(ifc.ifc_buf + (i * sizeof(struct ifreq)));
			if (ifr->ifr_addr.sa_family == AF_INET)
			{
				
				/* Get Ip Addr */
				addrIp = (struct sockaddr_in *)&ifr->ifr_addr;
				
				/* Get NetMask */
				memset(&ifrMask, 0, sizeof(struct ifreq));
				strcpy(ifrMask.ifr_name, ifr->ifr_name);
				addrMask = (struct sockaddr_in *)&ifrMask.ifr_addr;
				addrMask->sin_family = AF_INET;
				
				ioctl(sockfd, SIOCGIFNETMASK, &ifrMask);
				
				/* Lan  Ip, in.s_addr
				 * IF   Ip, addrIp->sin_addr.s_addr
				 * IF Mask, addrMask->sin_addr.s_addr
				 */
				 if ((addrIp->sin_addr.s_addr & addrMask->sin_addr.s_addr) == (in.s_addr & addrMask->sin_addr.s_addr))
				 {
					/* printf("find you\n"); */
					if(iface_name) strncpy(iface_name, ifr->ifr_name, len);
					if (lan_ip_addr) *lan_ip_addr = addrIp->sin_addr.s_addr;
					if (lan_ip_mask) *lan_ip_mask = addrMask->sin_addr.s_addr;
					ret = 0;
					break;
				 }
			}
		}
	}
	free(strBuf);
	close(sockfd);

    return ret;
}
/* end */

int curEntryCount = 0;

/* 
 * fn		static int pmlist_UpdateEntryCount(ENTRY_OPT opt)
 * brief	Record the number of entries.
 * details	
 *
 * param[in]	opt	Option on the entries.
 * param[out]	N/A
 *
 * return		int	
 * retval		0	success
 *			-1	failure
 *
 * note		Add by Chen Zexian, 20121122.
 */
static int pmlist_UpdateEntryCount(ENTRY_OPT opt)
{
	switch(opt)
	{
		case ENTRY_OPT_ADD: curEntryCount++; break;
		case ENTRY_OPT_DEL: curEntryCount--; break;
		case ENTRY_OPT_FLUSH: curEntryCount = 0; break;
		default: fprintf(stderr, "Entry option error!opt=%d\n", opt); return -1;
	}

	if (curEntryCount < 0 || curEntryCount > MAX_ENTRY_COUNT)
	{
		fprintf(stderr, "Entry count is %d, not between 0 and %d\n", curEntryCount, MAX_ENTRY_COUNT);
		return -1;
	}

	return 0;
}

/* 
 * fn		static int pmipt_System(char *cmd)
 * brief	Excute commands of iptables.
 * details	
 *
 * param[in]	cmd		The command to excute.
 * param[out]		N/A
 *
 * return		int 
 * retval		0	success
 *			-1 	failure
 *
 * note		Add by Chen Zexian, 20121122
 */
static int pmipt_System(char *cmd)
{
	int status = 0;
	int exe_ok = 0;

	while(exe_ok < 3)
	{
		status = tp_system(cmd);
		if (status < 0)
		{
			if (status == -1)
				fprintf(stderr, "system fork failed.\ncmd:%s\n", cmd);
			else
				fprintf(stderr, "pmipt_System call error, errno:%d\ncmd:%s\n", errno, cmd);

			exe_ok++;
			continue;
		}

		if ((WIFEXITED(status)))
		{
			if (WEXITSTATUS(status))
				fprintf(stderr, "cmd:\"%s\" execute ok, exit status = %d\n", cmd, WEXITSTATUS(status));

			break;
		}
		else if (WIFSIGNALED(status))
			fprintf(stderr, "abnormal termination, signal number = %d\ncmd:%s\n", WTERMSIG(status), cmd);
		else if (WIFSTOPPED(status))
			fprintf(stderr, "process stopped, signal number = %d\ncmd:%s\n", WSTOPSIG(status), cmd);
		else
			fprintf(stderr, "Oh,no possible here. status = %d\ncmd:%s\n", status, cmd);

		exe_ok++;
	}
	
	return 0;
}

struct portMap* pmlist_NewNode(int enabled, long int duration, char *remoteHost,
			       char *externalPort, char *internalPort,
			       char *protocol, char *internalClient, char *desc)
{
	struct portMap* temp = NULL;

	temp = (struct portMap*) malloc(sizeof(struct portMap));
	
	if (NULL == temp)
	{
		fprintf(stderr, "\npointer temp is NULL, return!\n");
		return NULL;
	}
	temp->m_PortMappingEnabled = enabled;
	
	if (remoteHost && strlen(remoteHost) < sizeof(temp->m_RemoteHost)) strcpy(temp->m_RemoteHost, remoteHost);
		else strcpy(temp->m_RemoteHost, "");
	if (strlen(externalPort) < sizeof(temp->m_ExternalPort)) strcpy(temp->m_ExternalPort, externalPort);
		else strcpy(temp->m_ExternalPort, "");
	if (strlen(internalPort) < sizeof(temp->m_InternalPort)) strcpy(temp->m_InternalPort, internalPort);
		else strcpy(temp->m_InternalPort, "");
	if (strlen(protocol) < sizeof(temp->m_PortMappingProtocol)) strcpy(temp->m_PortMappingProtocol, protocol);
		else strcpy(temp->m_PortMappingProtocol, "");
	if (strlen(internalClient) < sizeof(temp->m_InternalClient)) strcpy(temp->m_InternalClient, internalClient);
		else strcpy(temp->m_InternalClient, "");
	if (strlen(desc) < sizeof(temp->m_PortMappingDescription)) strcpy(temp->m_PortMappingDescription, desc);
		else strcpy(temp->m_PortMappingDescription, "");
	temp->m_PortMappingLeaseDuration = duration;

	temp->next = NULL;
	temp->prev = NULL;
	
	return temp;
}
	
#ifdef PATCH_MUTI_FORWARD_UPNP_LIST
/*wanggankun@2012-11-10, add, for iptables forward list*/
int pmlist_MutiUser_IptablesForwardList(struct portMap * SearchFrom, int maxAssert, char *InternalPort, char *proto, char *internalClient)
{
	struct portMap* temp;

	int counter = 0;

	if (NULL == SearchFrom)
	{
		temp = pmlist_Head;
	}
	else
	{
		temp = SearchFrom;
	}

	if (temp == NULL)
	{
		return 0;
	}
	
	do 
	{
		if ( (strcmp(temp->m_InternalPort, InternalPort) == 0) &&
				(strcmp(temp->m_PortMappingProtocol, proto) == 0) &&
				(strcmp(temp->m_InternalClient, internalClient) == 0) )
		{
			counter++; // We found a match, return pointer to it
			temp = temp->next;
		}
		else
		{
			temp = temp->next;
		}

		if (counter >= maxAssert)
		{
			break;
		}
		
	} while (temp != NULL);
	
	// If we made it here, we didn't find it, so return NULL
	return counter;
}
#endif
	
struct portMap* pmlist_Find(char *externalPort, char *proto, char *internalClient)
{
	struct portMap* temp;
	
	temp = pmlist_Head;
	if (temp == NULL)
		return NULL;
	
	do 
	{
		if ( (strcmp(temp->m_ExternalPort, externalPort) == 0) &&
				(strcmp(temp->m_PortMappingProtocol, proto) == 0) &&
				(strcmp(temp->m_InternalClient, internalClient) == 0) )
			return temp; // We found a match, return pointer to it
		else
			temp = temp->next;
	} while (temp != NULL);
	
	// If we made it here, we didn't find it, so return NULL
	return NULL;
}

struct portMap* pmlist_FindByIndex(int index)
{
	int i=0;
	struct portMap* temp;

	temp = pmlist_Head;
	if (temp == NULL)
		return NULL;
	do
	{
		if (i == index)
			return temp;
		else
		{
			temp = temp->next;	
			i++;
		}
	} while (temp != NULL);

	return NULL;
}	

struct portMap* pmlist_FindSpecific(char *externalPort, char *protocol)
{
	struct portMap* temp;
	
	temp = pmlist_Head;
	if (temp == NULL)
		return NULL;
	
	do
	{
		if ( (strcmp(temp->m_ExternalPort, externalPort) == 0) &&
				(strcmp(temp->m_PortMappingProtocol, protocol) == 0))
			return temp;
		else
			temp = temp->next;
	} while (temp != NULL);

	return NULL;
}

int pmlist_IsEmtpy(void)
{
	if (pmlist_Head)
		return 0;
	else
		return 1;
}

int pmlist_Size(void)
{
	struct portMap* temp;
	int size = 0;
	
	temp = pmlist_Head;
	if (temp == NULL)
		return 0;
	
	while (temp->next)
	{
		size++;
		temp = temp->next;
	}
	size++;
	return size;
}	

int pmlist_FreeList(void)
{
  struct portMap *temp, *next;

  temp = pmlist_Head;
  while(temp) {
    CancelMappingExpiration(temp->expirationEventId);
    next = temp->next;
    free(temp);
    temp = next;
  }
  pmlist_Head = pmlist_Tail = NULL;
  pmlist_UpdateEntryCount(ENTRY_OPT_FLUSH);
  pmlist_FlushPortMapping();
  return 1;
}
		
/* 
 * fn		int pmlist_PushBack(struct portMap* item)
 * brief	ÐÂÌí¼ÓÒ»¸ö¶Ë¿ÚÓ³ÉäÊ±»áµ÷ÓÃ¸ÃapiÌí¼Óµ½Á´±íÎ²²¿.	
 * details	×îÖÕ»áµ÷ÓÃ¸Ãº¯ÊýÌí¼Ó¶Ë¿ÚÓ³Éäµ½Á´±íºÍµ½iptables.	
 *
 * param[in]	item  ¶Ë¿ÚÓ³ÉäÌõÄ¿.
 *
 * return		int
 * retval		0/1
 *				0	Ê§°Ü
 *				1	³É¹¦
 *
 * note	written by  09Nov11, LI CHENGLONG	
 */
int pmlist_PushBack(struct portMap* item)
{
	int action_succeeded = 0;
	char lanIfName[IFNAMSIZ];

	if( getLanInterface(lanIfName,NULL, NULL,  item->m_InternalClient, IFNAMSIZ) )
	{
	  return 0;
	}
	if ( strcmp(lanIfName, g_vars.intInterfaceName) )
	{
	  return 0;
	}
	
#ifdef PATCH_MUTI_FORWARD_UPNP_LIST
	int nUser = 0;

	nUser = pmlist_MutiUser_IptablesForwardList(NULL, 1, item->m_InternalPort, item->m_PortMappingProtocol, item->m_InternalClient);
#endif	
	if (pmlist_Tail) // We have a list, place on the end
	{
		pmlist_Tail->next = item;
		item->prev = pmlist_Tail;
		item->next = NULL;
		pmlist_Tail = item;
		action_succeeded = 1;
	}
	else // We obviously have no list, because we have no tail :D
	{
		pmlist_Head = pmlist_Tail = pmlist_Current = item;
		item->prev = NULL;
		item->next = NULL;
 		action_succeeded = 1;
		trace(3, "appended %d %s %s %s %s %ld", item->m_PortMappingEnabled, 
				    item->m_PortMappingProtocol, item->m_ExternalPort, item->m_InternalClient, item->m_InternalPort,
				    item->m_PortMappingLeaseDuration);
	}
	if (action_succeeded == 1)
	{
		pmlist_UpdateEntryCount(ENTRY_OPT_ADD);

#ifndef PATCH_MUTI_FORWARD_UPNP_LIST
		pmlist_AddPortMapping(item->m_PortMappingEnabled, item->m_PortMappingProtocol,
				      item->m_ExternalPort, item->m_InternalClient, item->m_InternalPort);
#else

		pmlist_AddPortMapping_PeroutingList(item->m_PortMappingEnabled, item->m_PortMappingProtocol,
				      item->m_ExternalPort, item->m_InternalClient, item->m_InternalPort);

		if (0 == nUser)
		{
			/*wanggankun@2012-11-10, if the new one, must be added*/
			pmlist_AddPortMapping_ForwardList(item->m_PortMappingEnabled, item->m_PortMappingProtocol,
				      item->m_ExternalPort, item->m_InternalClient, item->m_InternalPort);			
		}

		pmlist_AddPortMapping_Conntrack(item->m_PortMappingEnabled, item->m_PortMappingProtocol,
				      item->m_ExternalPort, item->m_InternalClient, item->m_InternalPort);

#endif		
		return 1;
	}
	else
		return 0;
}

		
/* 
 * fn		int pmlist_Delete(struct portMap** item)
 * brief	´ÓÁ´±íÖÐÉ¾³ýÒ»¸ö¶Ë¿ÚÓ³Éä.	
 * details	´ÓÁ´±íºÍiptablesÖÐÉ¾³ýÒ»¸ö¶Ë¿ÚÓ³Éä.		
 *
 * param[in]	item ´ýÉ¾³ýµÄ¶Ë¿ÚÓ³ÉäÌõÄ¿.
 *
 * return		
 * retval		
 *
 * note	written by  09Nov11, LI CHENGLONG	
 */
int pmlist_Delete(struct portMap** item)
{
	struct portMap *temp;
	int action_succeeded = 0;

	temp = pmlist_Find((*item)->m_ExternalPort, (*item)->m_PortMappingProtocol, (*item)->m_InternalClient);
	
	if (temp) // We found the (*item) to delete
	{
		#ifdef PATCH_MUTI_FORWARD_UPNP_LIST
		int counter = 0 ;
		#endif
	  CancelMappingExpiration(temp->expirationEventId);
#ifndef PATCH_MUTI_FORWARD_UPNP_LIST
		pmlist_DeletePortMapping((*item)->m_PortMappingEnabled, (*item)->m_PortMappingProtocol, (*item)->m_ExternalPort, 
				(*item)->m_InternalClient, (*item)->m_InternalPort);
#else
		pmlist_DeletePortMapping_PeroutingList((*item)->m_PortMappingEnabled, (*item)->m_PortMappingProtocol, (*item)->m_ExternalPort, 
					(*item)->m_InternalClient, (*item)->m_InternalPort);
		counter = pmlist_MutiUser_IptablesForwardList(NULL, 2, (*item)->m_InternalPort, (*item)->m_PortMappingProtocol, (*item)->m_InternalClient);
		if ( counter < 2)
		{
			/*wanggankun@2012-11-10, when not muti-user, we can del it*/
			pmlist_DeletePortMapping_ForwardList(temp->m_PortMappingEnabled, temp->m_PortMappingProtocol, temp->m_ExternalPort,
			     	temp->m_InternalClient, temp->m_InternalPort);
		}
		pmlist_DeletePortMapping_Conntrack((*item)->m_PortMappingEnabled, (*item)->m_PortMappingProtocol, (*item)->m_ExternalPort, 
					(*item)->m_InternalClient, (*item)->m_InternalPort);
#endif	
		if (temp == pmlist_Head) // We are the head of the list
		{
			if (temp->next == NULL) // We're the only node in the list
			{
				pmlist_Head = pmlist_Tail = pmlist_Current = NULL;
				free (temp);
				/*wanggankun@2012-11-19, this is NULL point, so set it to NULL*/
				temp= NULL;
				(*item) = NULL;
				
				action_succeeded = 1;
				pmlist_UpdateEntryCount(ENTRY_OPT_DEL);
			}
			else // we have a next, so change head to point to it
			{
				pmlist_Head = temp->next;
				pmlist_Head->prev = NULL;
				free (temp);
				/*wanggankun@2012-11-19, this is NULL point, so set it to NULL*/
				temp= NULL;
				(*item) = NULL;
				
				action_succeeded = 1;	
				pmlist_UpdateEntryCount(ENTRY_OPT_DEL);
			}
		}
		else if (temp == pmlist_Tail) // We are the Tail, but not the Head so we have prev
		{
			pmlist_Tail = pmlist_Tail->prev;
			free (pmlist_Tail->next);
			pmlist_Tail->next = NULL;
			action_succeeded = 1;
			(*item) = NULL;
			pmlist_UpdateEntryCount(ENTRY_OPT_DEL);
		}
		else // We exist and we are between two nodes
		{
			temp->prev->next = temp->next;
			temp->next->prev = temp->prev;
			pmlist_Current = temp->next; // We put current to the right after a extraction
			free (temp);	
			/*wanggankun@2012-11-19, this is NULL point, so set it to NULL*/
			temp= NULL;
			(*item) = NULL;
					
			action_succeeded = 1;
			pmlist_UpdateEntryCount(ENTRY_OPT_DEL);
		}
	}
	else  // We're deleting something that's not there, so return 0
	{
		/* ¼´Ê¹ÕÒ²»µ½ÏàÓ¦ÌõÄ¿£¬(*item)Ò²Ó¦¸ÃÖÃÎªNULL£¬·ñÔòExpireMapping()µ÷ÓÃ
		 *  free_expiration_event()Ê±ÒÀÈ»¿ÉÄÜÒòÎª²Ù×÷Ò°Ö¸Õë¶øÍË³ö¡£Chen Zexian, 20121120
		 */
		(*item) = NULL;
		
		action_succeeded = 0;
	}

	return action_succeeded;
}

#ifdef PATCH_MUTI_FORWARD_UPNP_LIST

int pmlist_AddPortMapping_PeroutingList(int enabled, char *protocol, char *externalPort, char *internalClient, char *internalPort)
{
	if (g_vars.extInterfaceName[0] == '\0')
	{
		fprintf(stderr, "extInterfaceName not available\n\n");
	}

	if (enabled)
	{
		char command[COMMAND_LEN];
		snprintf(command, COMMAND_LEN, "%s -t nat -A %s -i %s -p %s --dport %s -j DNAT --to %s:%s", g_vars.iptables, "PREROUTING_UPNP"/*g_vars.preroutingChainName*/, g_vars.extInterfaceName, protocol, externalPort, internalClient, internalPort);
		trace(3, "%s", command);
		pmipt_System(command);
	}
	
   	return 1;	
}

int pmlist_AddPortMapping_ForwardList(int enabled, char *protocol, char *externalPort, char *internalClient, char *internalPort)
{
	if (g_vars.extInterfaceName[0] == '\0')
	{
		fprintf(stderr, "extInterfaceName not available\n\n");
	}

	if (enabled && g_vars.forwardRules)
	{
		char command[COMMAND_LEN];
		snprintf(command, COMMAND_LEN, "%s -A %s -i %s -p %s -d %s --dport %s -j ACCEPT",
		         g_vars.iptables, "FORWARD_UPNP"/*g_vars.forwardChainName*/, g_vars.extInterfaceName,
		         protocol, internalClient, internalPort);
		trace(3, "%s", command);
		pmipt_System(command);
	}
	
   	return 1;	
}

int pmlist_AddPortMapping_Conntrack(int enabled, char *protocol, char *externalPort, char *internalClient, char *internalPort)
{
	char cmdConntrack[COMMAND_LEN];
	char wanIpStr[IPSTR_LEN];
	unsigned char protonum = 0;

	if (enabled)
	{
		if (0 == strcmp(protocol, "TCP"))
		{
			protonum = 6;
		}
		else if(0 == strcmp(protocol, "UDP"))
		{
			protonum = 17;
		}
		else
		{
			fprintf(stderr, "unknown protocol %s\n", protocol);
			return 0;
		}

		if (0 == getIpAddress(g_vars.extInterfaceName, wanIpStr, IPSTR_LEN))
		{
			/* clean conntrack entry to DUT WAN*/
			snprintf(cmdConntrack, sizeof(cmdConntrack),
				"echo %s "
				"%hhu "
				"%s %s %s %s "
				"%s %s %s %s "
				"%s %s %s %s "
				"%s %s %s %s "
				"> /proc/net/nf_conntrack",	/* extended proc function, patch your SDK(refer to sdk/mt7621) */
				"0.0.0.0",			/* compatible with old version proc, must be 0.0.0.0 */
				protonum,			/* ip payload protocol number */
				"0.0.0.0",			/* original src ip addr */
				"0.0.0.0",			/* original src ip mask */
				wanIpStr,			/* original dest ip addr */
				"255.255.255.255",	/* original dest ip mask */
				"0",				/* original src port start */
				"0",				/* original src port end */
				externalPort,		/* original dst port start */
				externalPort,		/* original dst port end */
				wanIpStr,			/* reply src ip addr */
				"255.255.255.255",	/* reply src ip mask */
				"0.0.0.0",			/* reply dest ip addr */
				"0.0.0.0",			/* reply dest ip mask */
				externalPort,		/* reply src port start */
				externalPort,		/* reply src port end */
				"0",				/* reply dst port start */
				"0"					/* reply dst port end */
			);

			trace(3, "%s", cmdConntrack);
			pmipt_System(cmdConntrack);
		}
	}
	return 1;
}
#endif


#ifndef PATCH_MUTI_FORWARD_UPNP_LIST
static int pmlist_delDualWanPortMapping(void)
{
	char command[COMMAND_LEN];
	char wanIpStr[IPSTR_LEN];

	if (strcmp(g_vars.extInterfaceName, g_vars.primInterfaceName))
	{
		if (0 == getIpAddress(g_vars.primInterfaceName, wanIpStr, IPSTR_LEN))
		{
			if (strcmp(wanIpStr, g_vars.primIntfAddr))
			{
				if(!IS_PRV_IP(wanIpStr))
				{
					strncpy(g_vars.primIntfAddr, wanIpStr, sizeof(g_vars.primIntfAddr));
				}

				{
					snprintf(command, COMMAND_LEN, "%s -t nat -F %s",
					g_vars.iptables, "PREROUTING_UPNP_SECCONN"/*g_vars.preroutingChainName*/);
					trace(3, "%s", command);

					pmipt_System(command);
				}

				{
					snprintf(command, COMMAND_LEN, "%s -t nat -F %s",
					g_vars.iptables, "NATLOOPBACK_UPNP_SECCONN"/*g_vars.preroutingChainName*/);
					trace(3, "%s", command);

					pmipt_System(command);
				}

				if (g_vars.forwardRules)
				{
					snprintf(command, COMMAND_LEN, "%s -F %s", 
					g_vars.iptables, "FORWARD_UPNP_SECCONN"/*g_vars.forwardChainName*/);
					trace(3, "%s", command);

					pmipt_System(command);
				}
			}
		}
		else
		{
			memset(g_vars.primIntfAddr, 0, BUFLEN_16);

			{
				snprintf(command, COMMAND_LEN, "%s -t nat -F %s",
				g_vars.iptables, "PREROUTING_UPNP_SECCONN"/*g_vars.preroutingChainName*/);
				trace(3, "%s", command);

				pmipt_System(command);
			}

			{
				snprintf(command, COMMAND_LEN, "%s -t nat -F %s",
				g_vars.iptables, "NATLOOPBACK_UPNP_SECCONN"/*g_vars.preroutingChainName*/);
				trace(3, "%s", command);

				pmipt_System(command);
			}

			if (g_vars.forwardRules)
			{
				snprintf(command, COMMAND_LEN, "%s -F %s", 
				g_vars.iptables, "FORWARD_UPNP_SECCONN"/*g_vars.forwardChainName*/);
				trace(3, "%s", command);

				pmipt_System(command);
			}
		}
	}

	return 0;
}

static int pmlist_addDualWanPortMapping(char *protocol, char *externalPort, char *internalClient, char *internalPort)
{
	UINT32 lanIp;
	UINT32 lanMask;
	char lanIpStr[IPSTR_LEN];
	char lanMaskStr[IPSTR_LEN];
	char lanIfName[IFNAMSIZ];
	char wanIpStr[IPSTR_LEN];
	char command[COMMAND_LEN];

	if (!STR_IS_EMPTY(g_vars.primInterfaceName) && strcmp(g_vars.extInterfaceName, g_vars.primInterfaceName))
	{
		if (0 == getIpAddress(g_vars.primInterfaceName, wanIpStr, IPSTR_LEN))
		{
			if (!IS_PRV_IP(wanIpStr))
			{
				if ( !getLanInterface(lanIfName, &lanIp, &lanMask, internalClient, IFNAMSIZ))
				{
					snprintf(command, COMMAND_LEN, "%s -t nat -A %s -o %s -d %s -p %s --dport %s -j SNAT --to-source %s", g_vars.iptables, "NATLOOPBACK_UPNP_SECCONN",
					         lanIfName, internalClient, protocol, internalPort, wanIpStr);
					trace(3, "%s", command);
					pmipt_System(command);
				}

				snprintf(command, COMMAND_LEN, "%s -t nat -A %s -d %s -p %s --dport %s -j DNAT --to %s:%s", g_vars.iptables, "PREROUTING_UPNP_SECCONN"/*g_vars.preroutingChainName*/, 
				         wanIpStr, protocol, externalPort, internalClient, internalPort);

				trace(3, "%s", command);

				pmipt_System(command);
			}
			else
			{
				if (!IS_PRV_IP(g_vars.primIntfAddr))
				{
					if (!getLanInterface(lanIfName, &lanIp, &lanMask, internalClient, IFNAMSIZ))
					{
						snprintf(command, COMMAND_LEN, "%s -t nat -A %s -o %s -d %s -p %s --dport %s -j SNAT --to-source %s", g_vars.iptables, "NATLOOPBACK_UPNP_SECCONN",
						         lanIfName, internalClient, protocol, internalPort, g_vars.primIntfAddr);
						trace(3, "%s", command);
						pmipt_System(command);
					}

					snprintf(command, COMMAND_LEN, "%s -t nat -A %s -d %s -p %s --dport %s -j DNAT --to %s:%s", g_vars.iptables, "PREROUTING_UPNP_SECCONN"/*g_vars.preroutingChainName*/, g_vars.primIntfAddr, protocol, externalPort, internalClient, internalPort);

					trace(3, "%s", command);

					pmipt_System(command);
				}
			}
		}
	}

	return 0;
}

int pmlist_AddPortMapping (int enabled, char *protocol, char *externalPort, char *internalClient, char *internalPort)
{
	UINT32 lanIp;
	UINT32 lanMask;
	char lanIpStr[IPSTR_LEN];
	char lanMaskStr[IPSTR_LEN];
	char lanIfName[IFNAMSIZ];
	char wanIpStr[IPSTR_LEN];
	char command[COMMAND_LEN];

	if (g_vars.extInterfaceName[0] == '\0')
	{
		fprintf(stderr, "extInterfaceName not available\n\n");
	}

	if (enabled)
	{
#if HAVE_LIBIPTC
		char *buffer = malloc(strlen(internalClient) + strlen(internalPort) + 2);
		if (buffer == NULL) 
		{
			fprintf(stderr, "failed to malloc memory\n");
			return 0;
		}

		strcpy(buffer, internalClient);
		strcat(buffer, ":");
		strcat(buffer, internalPort);

		if (g_vars.forwardRules)
		{
			iptc_add_rule("filter", g_vars.forwardChainName, protocol, NULL, NULL, NULL, internalClient, NULL, internalPort, "ACCEPT", NULL, FALSE);
		}

		iptc_add_rule("nat", g_vars.preroutingChainName, protocol, g_vars.extInterfaceName, NULL, NULL, NULL, NULL, externalPort, "DNAT", buffer, TRUE);
		free(buffer);
#else

		#ifdef WAN_DUAL_ACCESS
		pmlist_addDualWanPortMapping(protocol, externalPort, internalClient, internalPort);
		#endif /* WAN_DUAL_ACCESS */

		#if 0
		int status;
		#endif
		{
			/*
			** char dest[DEST_LEN];
			** char wanIpStr[IPSTR_LEN];
			** char lanIfName[IFNAMSIZ];
			** g_vars.extInterfaceName,
			**
			** // å¦‚æžœextInterfaceName ä¸ºç©º åˆ™ æ˜¯ä½¿æ‰€æœ‰æŽ¥å£éƒ½ç”Ÿæ•ˆ. is defined by command line  by Li Chenglong
			** // important!, Added by LI CHENGLONG , 2011-Nov-09.
			** // changed "-I" to "-A", g_vars.preroutingChainName to "PREROUTING_UPNP"
			** // g_vars.forwardChainName to "FORWARD_UPNP"
			** // use system call instead of fork
			** // yangxv, 2011.11.20

			** char *args[] = {"iptables", "-t", "nat", "-A", g_vars.preroutingChainName, "-i", g_vars.extInterfaceName, "-p", protocol, "--dport", externalPort, "-j", "DNAT", "--to", dest, NULL};
		 	** // Ended by LI CHENGLONG , 2011-Nov-09.
			** snprintf(dest, DEST_LEN, "%s:%s", internalClient, internalPort);
			** // UPNP Natloopback, yuanshang, 2013-04-17 
			*/
		  if (0 == getIpAddress(g_vars.extInterfaceName, wanIpStr, IPSTR_LEN))
		  {
			/* 
			** UPNP Natloopback, yuanshang, 2013-04-17
			** NAT_LOOPBACK CMD:iptables -t nat -A POSTROUTING_NAT_LOOPBACK_VS -o br0 -d 192.168.1.100 -p tcp --dport 80 -j SNAT --to-source 10.0.0.129
			*/
			if( !getLanInterface(lanIfName, &lanIp, &lanMask, internalClient, IFNAMSIZ))
			{
				inet_ntop(AF_INET, &lanIp, lanIpStr, IPSTR_LEN);
				inet_ntop(AF_INET, &lanMask, lanMaskStr, IPSTR_LEN);
				snprintf(command, COMMAND_LEN, "%s -t nat -A %s -m conntrack --ctorigdst %s -o %s -d %s -s %s/%s -p %s --dport %s:%s -j SNAT --to-source %s",
				g_vars.iptables, "POSTROUTING_NATLOOPBACK_UPNP", wanIpStr , lanIfName, internalClient,lanIpStr,lanMaskStr ,protocol , internalPort, internalPort ,wanIpStr);
				trace(3, "%s", command);
				pmipt_System(command);
			}

			snprintf(command, COMMAND_LEN, "%s -t nat -A %s -d %s -p %s --dport %s -j DNAT --to %s:%s", g_vars.iptables, "PREROUTING_UPNP"/*g_vars.preroutingChainName*/, wanIpStr, protocol, externalPort, internalClient, internalPort);
		}
		else
		 {
			snprintf(command, COMMAND_LEN, "%s -t nat -A %s -i %s -p %s --dport %s -j DNAT --to %s:%s", g_vars.iptables, "PREROUTING_UPNP"/*g_vars.preroutingChainName*/, g_vars.extInterfaceName, protocol, externalPort, internalClient, internalPort);
		}
		trace(3, "%s", command);

		pmipt_System(command);

#ifdef WAN_DUAL_ACCESS
		pmlist_delDualWanPortMapping();
#endif /* WAN_DUAL_ACCESS */

		  #if 0
		  if (!fork()) 
		  {
		    int rc = execv(g_vars.iptables, args);
		    exit(rc);
		  }
		  else 
		  {
		    wait(&status);
		  }
		  #endif
		}

		if (g_vars.forwardRules)
		{
			/*char *args[] = {"iptables", "-A", g_vars.forwardChainName, "-p", protocol, "-d", internalClient, "--dport", internalPort, "-j", "ACCEPT", NULL};*/
			
			snprintf(command, COMMAND_LEN, "%s -A %s -i %s -p %s -d %s --dport %s -j ACCEPT", 
			         g_vars.iptables, "FORWARD_UPNP"/*g_vars.forwardChainName*/, g_vars.extInterfaceName, 
			         protocol, internalClient, internalPort);
			trace(3, "%s", command);

			pmipt_System(command);

		#ifdef WAN_DUAL_ACCESS
			if (strcmp(g_vars.extInterfaceName, g_vars.primInterfaceName))
			{
				char wanIpStr[IPSTR_LEN];
				if (0 == getIpAddress(g_vars.primInterfaceName, wanIpStr, IPSTR_LEN))
				{
					if (!IS_PRV_IP(wanIpStr) || !IS_PRV_IP(g_vars.primIntfAddr))
					{
						snprintf(command, COMMAND_LEN, "%s -A %s -i %s -p %s -d %s --dport %s -j ACCEPT", 
						         g_vars.iptables, "FORWARD_UPNP_SECCONN"/*g_vars.forwardChainName*/, g_vars.primInterfaceName, 
						         protocol, internalClient, internalPort);
						trace(3, "%s", command);

						pmipt_System(command);
					}
				}
			}
		#endif /* WAN_DUAL_ACCESS */

		#if 0
			  if (!fork()) 
			  {
			    int rc = execv(g_vars.iptables, args);
			    exit(rc);
			  }
			  else
			  {
			    wait(&status);		
			  }
		#endif
		}
#endif
		{
			char cmdConntrack[COMMAND_LEN];
			char wanIpStr[IPSTR_LEN];
			unsigned char protonum = 0;

			if (0 == strcmp(protocol, "TCP"))
			{
				protonum = 6;
			}
			else if(0 == strcmp(protocol, "UDP"))
			{
				protonum = 17;
			}
			else
			{
				fprintf(stderr, "unknown protocol %s\n", protocol);
				return 0;
			}

			if (0 == getIpAddress(g_vars.extInterfaceName, wanIpStr, IPSTR_LEN))
			{
				/* clean conntrack entry to DUT WAN*/
				snprintf(cmdConntrack, sizeof(cmdConntrack),
					"echo %s "
					"%hhu "
					"%s %s %s %s "
					"%s %s %s %s "
					"%s %s %s %s "
					"%s %s %s %s "
					"> /proc/net/nf_conntrack",	/* extended proc function, patch your SDK(refer to sdk/mt7621) */
					"0.0.0.0",			/* compatible with old version proc, must be 0.0.0.0 */
					protonum,			/* ip payload protocol number */
					"0.0.0.0",			/* original src ip addr */
					"0.0.0.0",			/* original src ip mask */
					wanIpStr,			/* original dest ip addr */
					"255.255.255.255",	/* original dest ip mask */
					"0",				/* original src port start */
					"0",				/* original src port end */
					externalPort,		/* original dst port start */
					externalPort,		/* original dst port end */
					wanIpStr,			/* reply src ip addr */
					"255.255.255.255",	/* reply src ip mask */
					"0.0.0.0",			/* reply dest ip addr */
					"0.0.0.0",			/* reply dest ip mask */
					externalPort,		/* reply src port start */
					externalPort,		/* reply src port end */
					"0",				/* reply dst port start */
					"0"					/* reply dst port end */
				);

				trace(3, "%s", cmdConntrack);
				pmipt_System(cmdConntrack);
			}
		}
    }
    return 1;
}
#endif

#ifdef PATCH_MUTI_FORWARD_UPNP_LIST
int pmlist_DeletePortMapping_PeroutingList(int enabled, char *protocol, char *externalPort, char *internalClient, char *internalPort)
{		
	if (g_vars.extInterfaceName[0] == '\0')
	{
		fprintf(stderr, "extInterfaceName not available\n");
	}

	if (enabled)
	{
		char command[COMMAND_LEN];
		snprintf(command, COMMAND_LEN, "%s -t nat -D %s -i %s -p %s --dport %s -j DNAT --to %s:%s",  
		         g_vars.iptables, "PREROUTING_UPNP", g_vars.extInterfaceName, protocol, externalPort, internalClient, internalPort);
		
		trace(3, "%s", command);
		pmipt_System(command);
	}
	
	return 1;
		
}

int pmlist_DeletePortMapping_ForwardList(int enabled, char *protocol, char *externalPort, char *internalClient, char *internalPort)
{
	if (g_vars.extInterfaceName[0] == '\0')
	{
		fprintf(stderr, "extInterfaceName not available\n");
	}

	if (enabled && g_vars.forwardRules)
	{
		char command[COMMAND_LEN];
		snprintf(command, COMMAND_LEN, "%s -D %s -i %s -p %s -d %s --dport %s -j ACCEPT", 
		         g_vars.iptables, "FORWARD_UPNP", g_vars.extInterfaceName, protocol, internalClient, internalPort);
		trace(3, "%s", command);
		pmipt_System(command);
	}
	return 1;
}

int pmlist_DeletePortMapping_Conntrack(int enabled, char *protocol, char *externalPort, char *internalClient, char *internalPort)
{
	char cmdConntrack[COMMAND_LEN];
	char wanIpStr[IPSTR_LEN];
	unsigned char protonum = 0;

	if (enabled)
	{
		if (0 == strcmp(protocol, "TCP"))
		{
			protonum = 6;
		}
		else if(0 == strcmp(protocol, "UDP"))
		{
			protonum = 17;
		}
		else
		{
			fprintf(stderr, "unknown protocol %s\n", protocol);
			return 0;
		}
	
		if (0 == getIpAddress(g_vars.extInterfaceName, wanIpStr, IPSTR_LEN))
		{
			/* clean conntrack entry to internal client */
			snprintf(cmdConntrack, sizeof(cmdConntrack),
				"echo %s "
				"%hhu "
				"%s %s %s %s "
				"%s %s %s %s "
				"%s %s %s %s "
				"%s %s %s %s "
				"> /proc/net/nf_conntrack", /* extended proc function, patch your SDK(refer to sdk/mt7621) */
				"0.0.0.0",			/* compatible with old version proc, must be 0.0.0.0 */
				protonum,			/* ip payload protocol number */
				"0.0.0.0",			/* original src ip addr */
				"0.0.0.0",			/* original src ip mask */
				wanIpStr,			/* original dest ip addr */
				"255.255.255.255",	/* original dest ip mask */
				"0",				/* original src port start */
				"0",				/* original src port end */
				externalPort,		/* original dst port start */
				externalPort,		/* original dst port end */
				internalClient,		/* reply src ip addr */
				"255.255.255.255",	/* reply src ip mask */
				"0.0.0.0",			/* reply dest ip addr */
				"0.0.0.0",			/* reply dest ip mask */
				internalPort,		/* reply src port start */
				internalPort,		/* reply src port end */
				"0",				/* reply dst port start */
				"0" 				/* reply dst port end */
			);
	
			trace(3, "%s", cmdConntrack);
			pmipt_System(cmdConntrack);
		}
	}
	return 1;
}

#endif

#ifndef PATCH_MUTI_FORWARD_UPNP_LIST
/* 
 * fn		int pmlist_DeletePortMapping(int enabled, 
 *										 char *protocol,
 *										 char *externalPort,
 *										 char *internalClient,
 *										 char *internalPort);
 * brief		´ÓiptablesÖÐÉ¾³ý¶Ë¿ÚÓ³Éä.
 * details		´ÓiptablesÖÐÉ¾³ý¶Ë¿ÚÓ³Éä.
 *
 * param[in]	enabled					ÊÇ·ñÆôÓÃ
 * param[in]	protocol				¶Ë¿ÚÓ³ÉäÐ­Òé
 * param[in]	externalPort			Íâ²¿¶Ë¿ÚºÅ
 * param[in]	internalClient			ÄÚ²¿¿Í»§¶Ëip
 * param[in]	internalPort			ÄÚ²¿¶Ë¿ÚºÅ
 *
 * return		int
 * retval		
 *
 * note	written by  09Nov11, LI CHENGLONG	
 */
int pmlist_DeletePortMapping(int enabled, char *protocol, char *externalPort, char *internalClient, char *internalPort)
{
	char lanIpStr[IPSTR_LEN];
	char lanMaskStr[IPSTR_LEN];
	UINT32 lanIp = 0;
	 UINT32 lanMask = 0;

	if (g_vars.extInterfaceName[0] == '\0')
	{
		fprintf(stderr, "extInterfaceName not available\n");
	}
	
    if (enabled)
    {
#if HAVE_LIBIPTC
	char *buffer = malloc(strlen(internalClient) + strlen(internalPort) + 2);
	strcpy(buffer, internalClient);
	strcat(buffer, ":");
	strcat(buffer, internalPort);

	if (g_vars.forwardRules)
	    iptc_delete_rule("filter", g_vars.forwardChainName, protocol, NULL, NULL, NULL, internalClient, NULL, internalPort, "ACCEPT", NULL);

	iptc_delete_rule("nat", g_vars.preroutingChainName, protocol, g_vars.extInterfaceName, NULL, NULL, NULL, NULL, externalPort, "DNAT", buffer);
	free(buffer);
#else
	char command[COMMAND_LEN];
	#if 0
	int status;
	#endif
	{
	  /*char dest[DEST_LEN];*/
	  char wanIpStr[IPSTR_LEN];
	  char lanIfName[IFNAMSIZ];
	  int wanIpFlag = 0;
	  /*char *args[] = {"iptables", "-t", "nat", "-D", g_vars.preroutingChainName, "-i", g_vars.extInterfaceName, "-p", protocol, "--dport", externalPort, "-j", "DNAT", "--to", dest, NULL};

	  snprintf(dest, DEST_LEN, "%s:%s", internalClient, internalPort);*/
	  /* UPNP Natloopback, yuanshang, 2013-04-17 */
	  if (0 == getIpAddress(g_vars.extInterfaceName, wanIpStr, IPSTR_LEN))
	  {
		wanIpFlag = 1;
		snprintf(command, COMMAND_LEN, "%s -t nat -D %s -d %s -p %s --dport %s -j DNAT --to %s:%s", g_vars.iptables, "PREROUTING_UPNP"/*g_vars.preroutingChainName*/, wanIpStr, protocol, externalPort, internalClient, internalPort);
	  }
	  else
	  {
		  snprintf(command, COMMAND_LEN, "%s -t nat -D %s -i %s -p %s --dport %s -j DNAT --to %s:%s",
		           g_vars.iptables, "PREROUTING_UPNP"/*g_vars.preroutingChainName*/, g_vars.extInterfaceName, protocol, externalPort, internalClient, internalPort);
	  }
	  
	  trace(3, "%s", command);
	  
	 pmipt_System(command);
	 
	 /* 
	   * UPNP Natloopback, yuanshang, 2013-04-17
	   * NAT_LOOPBACK CMD:iptables -t nat -D POSTROUTING_NAT_LOOPBACK_VS -o br0 -d 192.168.1.100 -p tcp --dport 80 -j SNAT --to-source 10.0.0.129
	   */
	  if( !getLanInterface(lanIfName, &lanIp, &lanMask, internalClient, IFNAMSIZ) && wanIpFlag)
	  {
		inet_ntop(AF_INET, &lanIp, lanIpStr, IPSTR_LEN);
		inet_ntop(AF_INET, &lanMask, lanMaskStr, IPSTR_LEN);
		snprintf(command, COMMAND_LEN, "%s -t nat -D %s -m conntrack --ctorigdst %s -o %s -d %s -s %s/%s -p %s --dport %s -j SNAT --to-source %s", g_vars.iptables, "POSTROUTING_NATLOOPBACK_UPNP",
		wanIpStr, lanIfName, internalClient, lanIpStr, lanMaskStr, protocol, internalPort, wanIpStr);
		trace(3, "%s", command);
		pmipt_System(command);
	  }

#ifdef WAN_DUAL_ACCESS
	if (strcmp(g_vars.extInterfaceName, g_vars.primInterfaceName))
	{
		if (0 == getIpAddress(g_vars.primInterfaceName, wanIpStr, IPSTR_LEN))
		{
			if (!IS_PRV_IP(wanIpStr))
			{
				snprintf(command, COMMAND_LEN, "%s -t nat -D %s -d %s -p %s --dport %s -j DNAT --to %s:%s", g_vars.iptables, "PREROUTING_UPNP_SECCONN"/*g_vars.preroutingChainName*/, wanIpStr, protocol, externalPort, internalClient, internalPort);

				trace(3, "%s", command);

				pmipt_System(command);

				if (0 == getLanInterface(lanIfName, &lanIp, &lanMask, internalClient, IFNAMSIZ))
				{
						snprintf(command, COMMAND_LEN, "%s -t nat -D %s -o %s -d %s -p %s --dport %s -j SNAT --to-source %s", g_vars.iptables, "NATLOOPBACK_UPNP_SECCONN", 
						         lanIfName, internalClient, protocol, internalPort, wanIpStr);
						trace(3, "%s", command);
						pmipt_System(command);
				}
			}
		}
	}
#endif /* WAN_DUAL_ACCESS */

#if 0
	  if (!fork()) {
	    int rc = execv(g_vars.iptables, args);
	    exit(rc);
	  } else {
	    wait(&status);		
	  }
#endif
	}

	if (g_vars.forwardRules)
	{
		/*char *args[] = {"iptables", "-D", g_vars.forwardChainName, "-p", protocol, "-d", internalClient, "--dport", internalPort, "-j", "ACCEPT", NULL};*/
		
		snprintf(command, COMMAND_LEN, "%s -D %s -i %s -p %s -d %s --dport %s -j ACCEPT", 
		         g_vars.iptables, "FORWARD_UPNP"/*g_vars.forwardChainName*/, g_vars.extInterfaceName, 
		         protocol, internalClient, internalPort);
		trace(3, "%s", command);

		pmipt_System(command);

#ifdef WAN_DUAL_ACCESS
		if (strcmp(g_vars.extInterfaceName, g_vars.primInterfaceName))
		{
			char wanIpStr[IPSTR_LEN];
			if (0 == getIpAddress(g_vars.primInterfaceName, wanIpStr,  IPSTR_LEN))
			{
				if (!IS_PRV_IP(wanIpStr))
				{
					snprintf(command, COMMAND_LEN, "%s -D %s -i %s -p %s -d %s --dport %s -j ACCEPT", 
					         g_vars.iptables, "FORWARD_UPNP_SECCONN"/*g_vars.forwardChainName*/, g_vars.primInterfaceName, 
					         protocol, internalClient, internalPort);
					trace(3, "%s", command);

					pmipt_System(command);
				}
			}
		}

		pmlist_delDualWanPortMapping();

	}
#endif /* WAN_DUAL_ACCESS */

#if 0
	  if (!fork()) {
	    int rc = execv(g_vars.iptables, args);
	    exit(rc);
	  } else {
	    wait(&status);		
	  }
#endif
	}
#endif
	{
		char cmdConntrack[COMMAND_LEN];
		char wanIpStr[IPSTR_LEN];
		unsigned char protonum = 0;
	
		if (0 == strcmp(protocol, "TCP"))
		{
			protonum = 6;
		}
		else if(0 == strcmp(protocol, "UDP"))
		{
			protonum = 17;
		}
		else
		{
			fprintf(stderr, "unknown protocol %s\n", protocol);
			return 0;
		}
	
		if (0 == getIpAddress(g_vars.extInterfaceName, wanIpStr, IPSTR_LEN))
		{
			/* clean conntrack entry to internal client */
			snprintf(cmdConntrack, sizeof(cmdConntrack),
				"echo %s "
				"%hhu "
				"%s %s %s %s "
				"%s %s %s %s "
				"%s %s %s %s "
				"%s %s %s %s "
				"> /proc/net/nf_conntrack", /* extended proc function, patch your SDK(refer to sdk/mt7621) */
				"0.0.0.0",			/* compatible with old version proc, must be 0.0.0.0 */
				protonum,			/* ip payload protocol number */
				"0.0.0.0",			/* original src ip addr */
				"0.0.0.0",			/* original src ip mask */
				wanIpStr,			/* original dest ip addr */
				"255.255.255.255",	/* original dest ip mask */
				"0",				/* original src port start */
				"0",				/* original src port end */
				externalPort,		/* original dst port start */
				externalPort,		/* original dst port end */
				internalClient,		/* reply src ip addr */
				"255.255.255.255",	/* reply src ip mask */
				"0.0.0.0",			/* reply dest ip addr */
				"0.0.0.0",			/* reply dest ip mask */
				internalPort,		/* reply src port start */
				internalPort,		/* reply src port end */
				"0",				/* reply dst port start */
				"0" 				/* reply dst port end */
			);
	
			trace(3, "%s", cmdConntrack);
			pmipt_System(cmdConntrack);
		}
	}

    return 1;
}
#endif

/* 
 * fn		int pmlist_FlushPortMapping()
 * brief	Delete all the rules in PREROUTING_UPNP and FORWARD_UPNP.
 * details	
 *
 * param[in]	N/A
 * param[out]	N/A
 *
 * return	int
 * retval	0	-success
 * 		-1	-failed
 *
 * note		Write by Chen Zexian, 20121115
 */
int pmlist_FlushPortMapping()
{
#if 0
	if (g_vars.extInterfaceName[0] == '\0')
	{
		fprintf(stderr, "extInterfaceName not available\n");
	}
#endif

#if HAVE_LIBIPTC
	
#else
	char command[COMMAND_LEN];
	
	{
		snprintf(command, COMMAND_LEN, "%s -t nat -F %s",
		         g_vars.iptables, "PREROUTING_UPNP"/*g_vars.preroutingChainName*/);
		trace(3, "%s", command);

		pmipt_System(command);
	}

	{
		snprintf(command, COMMAND_LEN, "%s -t nat -F %s",
		         g_vars.iptables, "POSTROUTING_NATLOOPBACK_UPNP"/*g_vars.preroutingChainName*/);
		trace(3, "%s", command);

		pmipt_System(command);
	}

	if (g_vars.forwardRules)
	{
		snprintf(command, COMMAND_LEN, "%s -F %s", 
		         g_vars.iptables, "FORWARD_UPNP"/*g_vars.forwardChainName*/);
		trace(3, "%s", command);

		pmipt_System(command);
	}

	#ifdef WAN_DUAL_ACCESS
	{
		snprintf(command, COMMAND_LEN, "%s -t nat -F %s",
		         g_vars.iptables, "PREROUTING_UPNP_SECCONN"/*g_vars.preroutingChainName*/);
		trace(3, "%s", command);

		pmipt_System(command);
	}

	{
		snprintf(command, COMMAND_LEN, "%s -t nat -F %s",
		         g_vars.iptables, "NATLOOPBACK_UPNP_SECCONN"/*g_vars.preroutingChainName*/);
		trace(3, "%s", command);

		pmipt_System(command);
	}

	if (g_vars.forwardRules)
	{
		snprintf(command, COMMAND_LEN, "%s -F %s", 
		         g_vars.iptables, "FORWARD_UPNP_SECCONN"/*g_vars.forwardChainName*/);
		trace(3, "%s", command);

		pmipt_System(command);
	}
	#endif /* WAN_DUAL_ACCESS */
#endif
	
	return 0;
}

