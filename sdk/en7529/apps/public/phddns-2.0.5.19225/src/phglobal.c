#include "phglobal.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>      /* Needed only for _O_RDWR definition */
#include <stdlib.h>
#include <stdio.h>

#ifndef WIN32
#include <termios.h>
#endif

#include <time.h>

const char *convert_status_code(int nCode)
{
	static char buf[64] = "";
	FILE *ff;
	char put_stat[32] = {0};
	char put_statval[100] = {0};
	switch (nCode)
	{
	case okConnecting:
		strcpy(buf,"okConnecting");
		break;
	case okConnected:
		strcpy(buf,"okConnected");
		break;
	case okAuthpassed:
		strcpy(buf,"okAuthpassed");
		break;
	case okDomainListed:
		strcpy(buf,"okDomainListed");
		break;
	case okDomainsRegistered:
		{
		strcpy(buf,"okDomainsRegistered");
		strncpy(put_stat, "echo -n 0 > /tmp/ddnsstat", sizeof(put_stat) - 1);
		strncpy(put_statval, "echo -n > /tmp/statval", sizeof(put_statval) - 1);
		system(put_stat);
		system(put_statval);
		}
		break;
	case okKeepAliveRecved:
		strcpy(buf,"okKeepAliveRecved");
		break;
	case okRetrievingMisc:
		strcpy(buf,"okRetrievingMisc");
		break;
	case errorConnectFailed:
		{
		strcpy(buf,"errorConnectFailed");
		strncpy(put_stat, "echo -n 3 > /tmp/ddnsstat", sizeof(put_stat) - 1);
		strncpy(put_statval, "echo -n errorConnectFailed > /tmp/statval", sizeof(put_statval) - 1);
		system(put_stat);
		system(put_statval);
		}
		break;
	case errorSocketInitialFailed:
			{
		strcpy(buf,"errorSocketInitialFailed");
		strncpy(put_stat, "echo -n 10 > /tmp/ddnsstat", sizeof(put_stat) - 1);
		strncpy(put_statval, "echo -n errorSocketInitialFailed > /tmp/statval", sizeof(put_statval) - 1);
		system(put_stat);
		system(put_statval);
		}
		break;
	case errorAuthFailed:
			{
		strcpy(buf,"errorAuthFailed");
		strncpy(put_stat, "echo -n 4 > /tmp/ddnsstat", sizeof(put_stat) - 1);
		strncpy(put_statval, "echo -n > /tmp/statval", sizeof(put_statval) - 1);
		system(put_stat);
		system(put_statval);
		}
		break;
	case errorDomainListFailed:
			{
		strcpy(buf,"errorDomainListFailed");
		strncpy(put_stat, "echo -n 9 > /tmp/ddnsstat", sizeof(put_stat) - 1);
		strncpy(put_statval, "echo -n errorDomainListFailed > /tmp/statval", sizeof(put_statval) - 1);
		system(put_stat);
		system(put_statval);
		
		}
		break;
	case errorDomainRegisterFailed:
			{
		strcpy(buf,"errorDomainRegisterFailed");
				strncpy(put_stat, "echo -n 9 > /tmp/ddnsstat", sizeof(put_stat) - 1);
				strncpy(put_statval, "echo -n errorDomainRegisterFailed > /tmp/statval", sizeof(put_statval) - 1);
		system(put_stat);
		system(put_statval);
		}
		break;
	case errorUpdateTimeout:
			{
		strcpy(buf,"errorUpdateTimeout");
		strncpy(put_stat, "echo -n 9 > /tmp/ddnsstat", sizeof(put_stat) - 1);
		strncpy(put_statval, "echo -n errorUpdateTimeout > /tmp/statval", sizeof(put_statval) - 1);
		system(put_stat);
		system(put_statval);
		}
		break;
	case errorKeepAliveError:
			{
		strcpy(buf,"errorKeepAliveError");
		strncpy(put_stat, "echo -n 9 > /tmp/ddnsstat", sizeof(put_stat) - 1);
		strncpy(put_statval, "echo -n errorKeepAliveError > /tmp/statval", sizeof(put_statval) - 1);
		system(put_stat);
		system(put_statval);
		}
		break;
	case errorRetrying:
			{
		strcpy(buf,"errorRetrying");
		strncpy(put_stat, "echo -n 9 > /tmp/ddnsstat", sizeof(put_stat) - 1);
		strncpy(put_statval, "echo -n errorRetrying > /tmp/statval", sizeof(put_statval) - 1);
		system(put_stat);
		system(put_statval);
		}
		break;
	case okNormal:
		strcpy(buf,"okNormal");
		break;
	case okNoData:
		strcpy(buf,"okNoData");
		break;
	case okServerER:
		strcpy(buf,"okServerER");
		break;
	case errorOccupyReconnect:
			{
		strcpy(buf,"errorOccupyReconnect");
		strncpy(put_stat, "echo -n 9 > /tmp/ddnsstat", sizeof(put_stat) - 1);
		strncpy(put_statval, "echo -n errorOccupyReconnect > /tmp/statval", sizeof(put_statval) - 1);
		system(put_stat);
		system(put_statval);
		}
		break;
	case okRedirecting:
		strcpy(buf,"okRedirecting");
		break;
	case errorAuthBusy:
			{
		strcpy(buf,"errorAuthBusy");
		strncpy(put_stat, "echo -n 9 > /tmp/ddnsstat", sizeof(put_stat) - 1);
		strncpy(put_statval, "echo -n errorAuthBusy > /tmp/statval", sizeof(put_statval) - 1);
		system(put_stat);
		system(put_statval);
		}
		break;
	case errorStatDetailInfoFailed:
			{
		strcpy(buf,"errorAuthBusy");
		strncpy(put_stat, "echo -n 9 > /tmp/ddnsstat", sizeof(put_stat) - 1);
		strncpy(put_statval, "echo -n errorStatDetailInfoFailed > /tmp/statval", sizeof(put_statval) - 1);
		system(put_stat);
		system(put_statval);
		}
		break;
	}

	return buf;
}


const char *my_inet_ntoa(int ip)
{
	struct in_addr addr;
	addr.s_addr = ip;
	return inet_ntoa(addr);
}

static void defOnStatusChanged(int status, int data)
{
	printf("defOnStatusChanged %s", convert_status_code(status));
	if (status == okKeepAliveRecved)
	{
		printf(", IP: %d", data);
	}
	if (status == okDomainsRegistered)
	{
		printf(", UserType: %d", data);
	}
	printf("\n");
}

static void defOnDomainRegistered(char *domain)
{
	printf("defOnDomainRegistered %s\n", domain);
}

static void defOnUserInfo(char *userInfo, int len)
{
	printf("defOnUserInfo %s\n", userInfo);
}

static void defOnAccountDomainInfo(char *domainInfo, int len)
{
	printf("defOnAccountDomainInfo %s\n", domainInfo);
}

void init_global(PHGlobal *global)
{
	strcpy(global->szHost,"phddns60.oray.net");
	strcpy(global->szUserID,"");
	strcpy(global->szUserPWD,"");
	strcpy(global->szBindAddress,"");
	global->nUserType = 0;
	global->nPort = 6060;

	global->bTcpUpdateSuccessed = FALSE;
	strcpy(global->szChallenge,"");
	global->nChallengeLen = 0;
	global->nChatID = global->nStartID = global->nLastResponseID = global->nAddressIndex = 0;
	global->tmLastResponse = -1;
	global->ip = 0;
	strcpy(global->szTcpConnectAddress,"");

	global->cLastResult = -1;

	global->uptime = time(0);
	global->lasttcptime = 0;

	strcpy(global->szActiveDomains[0],".");

	global->bNeed_connect = TRUE;
	global->tmLastSend = 0;

	global->m_tcpsocket = global->m_udpsocket = INVALID_SOCKET;
	
	global->cbOnStatusChanged = NULL;
	global->cbOnDomainRegistered = NULL;
	global->cbOnUserInfo = NULL;
	global->cbOnAccountDomainInfo = NULL;
}

void set_default_callback(PHGlobal *global)
{
	global->cbOnStatusChanged = defOnStatusChanged;
	global->cbOnDomainRegistered = defOnDomainRegistered;
	global->cbOnUserInfo = defOnUserInfo;
	global->cbOnAccountDomainInfo = defOnAccountDomainInfo;
}

