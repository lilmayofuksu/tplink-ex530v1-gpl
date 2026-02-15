#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include <arpa/inet.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include "globals.h"
#include <linux/if_ether.h>

void trace(int debuglevel, const char *format, ...);

int tp_system(char *cmd)
{
	int status = 0;
	int pid;

	if((pid = vfork()) < 0)
	{
		status = -1;
	}
	else if(pid == 0)
	{
		const char *new_argv[4];

		new_argv[0] = "sh";
		new_argv[1] = "-c";
		new_argv[2] = cmd;
		new_argv[3] = NULL;

		if(execve("/bin/sh", (char *const *)new_argv, NULL) < 0)
		{
			printf("fail to execve %s\n",cmd);
			exit(1);
		}

		else
		{
			exit(0);
		}
	}
	else
	{
		waitpid(pid, &status, 0);
	}

	return status;
}

static int get_sockfd(void)
{
   int sockfd;

   if ((sockfd = socket(PF_INET, SOCK_RAW, IPPROTO_RAW)) == -1)
   {
      perror("user: socket creating failed");
      return (-1);
   }
   
   return sockfd;
}

void getRandomPrefix(char *mac, char *prefix)
{
    int remainder = 0;
    int prefix_idx = 0;

    while (prefix_idx < ETH_ALEN)
    {
        remainder = ((remainder << 8) + (unsigned char)mac[ETH_ALEN - 1 - prefix_idx]) % 26;
        prefix[prefix_idx++] = (char)(remainder + 97); /* ASCII 97->'a' */
    }

    prefix[ETH_ALEN] = '\0';
}


int getMacAddress(char *ifname, char *mac)
{
    struct ifreq ifreq;
    int sock;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        return FALSE;
    }

    strcpy(ifreq.ifr_name, ifname);

    if(ioctl(sock, SIOCGIFHWADDR, &ifreq) < 0)
    {
        perror("ioctl");
        close(sock);
        return FALSE;
    }

    close(sock);

    memcpy(mac, ifreq.ifr_hwaddr.sa_data, ETH_ALEN);

    return TRUE;
}

int GetIpAddressStr(char *address, char *ifname)
{
   struct ifreq ifr;
   struct sockaddr_in *saddr;
   int succeeded = 0;
   int sockfd = -1;

   if (ifname[0] == '\0')
   {
   		strcpy(address, "0.0.0.0");
   }

   if (sockfd < 0)
   {
   	/* ATTENTION: g_sockfd should not be closed! */
   	sockfd = get_sockfd();
   }
   
   if (sockfd >= 0 )
   {
      strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
      ifr.ifr_addr.sa_family = AF_INET;
      if (ioctl(sockfd, SIOCGIFADDR, &ifr) == 0)
      {
         saddr = (struct sockaddr_in *)&ifr.ifr_addr;
         strcpy(address,inet_ntoa(saddr->sin_addr));
         succeeded = 1;
      }
      else
      {
         trace(1, "Failure obtaining ip address of interface %s", ifname);
         succeeded = 0;
      }

      close(sockfd);
      sockfd = -1;
   }
   else
   {
   	trace(1, "Create socket error!");
	succeeded = 0;
   }
  
   return succeeded;
}

void trace(int debuglevel, const char *format, ...)
{
    char buffer[BUFLEN_1024] = {0};
    va_list ap;
    va_start(ap, format);
    if (g_vars.debug >= debuglevel)
    {
        vsyslog(LOG_DEBUG, format, ap);
        vsnprintf(buffer, BUFLEN_1024-1, format, ap);
    }
    va_end(ap);

    if (g_vars.debug >= debuglevel)
    {
        printf("[ %s ] %03d%s:  %s\n", __FUNCTION__, __LINE__, ((3 == debuglevel) ? " - ERROR":""), buffer);
    }
}
