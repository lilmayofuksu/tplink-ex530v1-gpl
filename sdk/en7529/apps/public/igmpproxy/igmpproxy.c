/*
**  igmpproxy - IGMP proxy based multicast router 
**  Copyright (C) 2005 Johnny Egeland <johnny@rlo.org>
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
**----------------------------------------------------------------------------
**
**  This software is derived work from the following software. The original
**  source code has been modified from it's original state by the author
**  of igmpproxy.
**
**  smcroute 0.92 - Copyright (C) 2001 Carsten Schill <carsten@cschill.de>
**  - Licensed under the GNU General Public License, version 2
**  
**  mrouted 3.9-beta3 - COPYRIGHT 1989 by The Board of Trustees of 
**  Leland Stanford Junior University.
**  - Original license can be found in the "doc/mrouted-LINCESE" file.
**
*/
/**
*   igmpproxy.c - The main file for the IGMP proxy application.
*
*   February 2005 - Johnny Egeland
*/

#include "defs.h"
#include <fcntl.h>
#if defined(TCSUPPORT_MULTI_USER_ITF)
#include <lan_port/lan_port_info.h>
#endif

#include "version.h"
#include "build.h"



// Constants
static const char Version[] = 
"igmpproxy, Version " VERSION ", Build" BUILD "\n"
"Copyright 2005 by Johnny Egeland <johnny@rlo.org>\n"
"Distributed under the GNU GENERAL PUBLIC LICENSE, Version 2 - check GPL.txt\n"
"\n";

static const char Usage[] = 
"usage: igmpproxy [-h] [-d] [-c <configfile>]\n"
"\n" 
"   -h   Display this help screen\n"
"   -c   Specify a location for the config file (default is '/etc/igmpproxy.conf').\n"
"   -d   Run in debug mode. Does not fork deamon, and output all logmessages on stderr.\n"
"\n"
;

// Local function Prototypes
static void signalHandler(int);
int     igmpProxyInit();
void    igmpProxyCleanUp();
void    igmpProxyRun();

// Global vars...
static int sighandled = 0;
#define	GOT_SIGINT	0x01
#define	GOT_SIGHUP	0x02
#define	GOT_SIGUSR1	0x04
#define	GOT_SIGUSR2	0x08

// The upstream VIF index
int         upStreamVif;   

#if defined(TCSUPPORT_CT)
char devname[64] = {0};
int isInternetIface = 0;
#define MAX_PVC_NUM	 8
#define MAX_SMUX_NUM 8
#define VBIND_ACTIVE_PATH "/proc/tc3162/vbind_active"
#if !defined(TCSUPPORT_MULTI_USER_ITF)
#if defined(TCSUPPORT_WLAN_AC)
#define MAX_LAN_PORT_NUM     14
#else
#define MAX_LAN_PORT_NUM     8
#endif
#endif
#define MAX_WAN_IF_INDEX  (MAX_PVC_NUM*MAX_SMUX_NUM)

int setFWdevname(char *name)
{
	int pvc = 0, entry = 0, ifaceIndex = 0;
	char svrlist[128] = {0}, wan_node[64] = {0};

	tcdbg_printf("setFWdevname: name %s\n", name);

	if ( !name )
		return -1;

	strncpy(devname, name,sizeof(devname)-1);
	if ( strstr(devname, "ppp") )
	{
		sscanf(devname, "ppp%d", &ifaceIndex);
		pvc = ifaceIndex / MAX_PVC_NUM;
		entry = ifaceIndex % MAX_PVC_NUM;
	}
	else
	{
		sscanf(devname, "nas%d_%d", &pvc, &entry);
	}
	sprintf(wan_node,  "Wan_PVC%d_Entry%d", pvc, entry);

	if ( 0 == tcapi_get(wan_node, "ServiceList", svrlist)
		&& NULL != strstr(svrlist, "INTERNET") )
		isInternetIface = 1;
	else
		isInternetIface = 0;

	return 0;
}
char *getFWdevname()
{
	return devname;
}
int isInternetInterface()
{
	return isInternetIface;
}

/*
return value
0: check bind ok
1: check bind ignore
-1: check bind fail.
*/
static int check_bind(char *wan_if, uint32 mark)
{
	char wan_node[32], wan_if_nas[20];
#if defined(TCSUPPORT_MULTI_USER_ITF)
	uint32 pt_mark = GET_LAN_ITF_MARK(mark);
#else
	uint32 pt_mark = (mark & 0xf0000000) >> 28;
#endif
	char lan_if[10], lan_if_val[10];
	int i, j, ret, bind_flag = 0;
#if defined(TCSUPPORT_CT_VLAN_BIND)
	int fd=0, switchOn=0, ifaceIndex=0;
	char buf[4]={0}, vbindActive[5]={0},index[5]={0};
	int i_pvc = 0, i_entry = 0;
	uint32 vbind_mark = (mark & 0x7f0000) >> 16;
#endif

	/* the packet is from CPE, don't care binding info */
	if ( pt_mark == 0 )
	{
		return 1;
	}

#if defined(TCSUPPORT_CT_VLAN_BIND)
	fd = open(VBIND_ACTIVE_PATH,O_RDONLY|O_NONBLOCK);
	if (fd < 0)
	{
		tcdbg_printf("check_bind:open %s error.\n", VBIND_ACTIVE_PATH);
		return 1;
	}
	ret = read(fd, buf, sizeof(buf) );
	close(fd);
	if ( ret <= 0 )
	{
		tcdbg_printf("check_bind:read %s error.\n", VBIND_ACTIVE_PATH);
		return 1;
	}
	switchOn = atoi(buf);
	if ( pt_mark > 0 && pt_mark <= MAX_LAN_PORT_NUM )
	{
		sprintf(wan_node,  "VlanBind_Entry%d", pt_mark-1);
		ret = tcapi_get(wan_node, "Active", vbindActive);
		if (ret < 0) {
			tcdbg_printf("check_bind:tcapi_get %s Active error.\n",wan_node);
			return 1;
		}
	}

	if ( switchOn && !strcmp(vbindActive, "Yes") )
	{
		if ( vbind_mark == 0 )
		{
			if ( mark & 0x1 ) /* DROP TAG packets. */
				return -1;
			return 1;
		}
		else 
		{
			if(strstr(wan_if, "ppp")){
				sscanf(wan_if, "ppp%d", &ifaceIndex);
			}
			else{   /*nas interface,ext:nas0_1*/
				sscanf(wan_if, "nas%d_%d", &i_pvc, &i_entry);
				ifaceIndex = i_pvc * MAX_SMUX_NUM + i_entry;
			}

			if ( ifaceIndex < 0 || ifaceIndex >= MAX_WAN_IF_INDEX )
			{
				printf("check_bind:get wan interface index error.\n");
				return 1;
			}
			if ( (ifaceIndex + 1) == vbind_mark )
				return 0;

		}
	}
	else
#endif
	{
		printf("PORT BIND CASE, ignore.\n");
		return 1;
	}

	printf("check_bind():wan interface not match.\n");
	return -1;
}

/*
return code:
0 : OK or IGNORE
! 0 : DROP the packet.
*/
int check_igmp_packets(uint32 chk_skb_mark)
{
	int bind_reval = 0;

	bind_reval = check_bind(getFWdevname(), chk_skb_mark);
	if ( bind_reval < 0 )
	{
		printf("check_igmp_packets failed.\n"); 
		return -1;
	}

	return 0;
}
#endif

/**
*   Program main method. Is invoked when the program is started
*   on commandline. The number of commandline arguments, and a
*   pointer to the arguments are recieved on the line...
*/    
int main( int ArgCn, const char *ArgVc[] ) {

    int debugMode = 0;

    // Set the default config Filepath...
    char* configFilePath = IGMPPROXY_CONFIG_FILEPATH;

    // Display version 
    fputs( Version, stderr );

    // Parse the commandline options and setup basic settings..
    int i = 1;
    while (i < ArgCn) {

        if ( strlen(ArgVc[i]) > 1 && ArgVc[i][0] == '-') {

            switch ( ArgVc[i][1] ) {
            case 'h':
                fputs( Usage, stderr );
                exit( 0 );

            case 'd':
                Log2Stderr = LOG_DEBUG;
                /*
            case 'v':
                // Enable debug mode...
                if (Log2Stderr < LOG_INFO) {
                    Log2Stderr = LOG_INFO;
                }
                */
                debugMode = 1;
                break;

            case 'c':
                // Get new filepath...
                if (i + 1 < ArgCn && ArgVc[i+1][0] != '-') {
                    configFilePath = ArgVc[i+1];
                    i++;
                } else {
                    log(LOG_ERR, 0, "Missing config file path after -c option.");
                }
                break;
            }
        }
        i++;
    }

    // Chech that we are root
    if (geteuid() != 0) {
    	fprintf(stderr, "igmpproxy: must be root\n");
    	exit(1);
    }

    // Write debug notice with file path...
    IF_DEBUG log(LOG_DEBUG, 0, "Searching for config file at '%s'" , configFilePath);

    do {

        // Loads the config file...
        if( ! loadConfig( configFilePath ) ) {
            log(LOG_ERR, 0, "Unable to load config file...");
            break;
        }
    
        // Initializes the deamon.
        if ( !igmpProxyInit() ) {
            log(LOG_ERR, 0, "Unable to initialize IGMPproxy.");
            break;
        }
    
    
        // If not in debug mode, fork and detatch from terminal.
        if ( ! debugMode ) {
    
            IF_DEBUG log( LOG_DEBUG, 0, "Starting daemon mode.");
    
            // Only daemon goes past this line...
            if (fork()) exit(0);
    
            // Detach deamon from terminal
            if ( close( 0 ) < 0 || close( 1 ) < 0 || close( 2 ) < 0 
                 || open( "/dev/null", 0 ) != 0 || dup2( 0, 1 ) < 0 || dup2( 0, 2 ) < 0
                 || setpgrp() < 0
               ) {
                log( LOG_ERR, errno, "failed to detach deamon" );
            }
        }
        
        // Go to the main loop.
        igmpProxyRun();
    
        // Clean up
        igmpProxyCleanUp();

    } while ( FALSE );

    // Inform that we are exiting.
    log(LOG_INFO, 0, "Shutdown complete....");

    exit(0);
}



/**
*   Handles the initial startup of the daemon.
*/
int igmpProxyInit() {
    struct sigaction sa;
    int Err;


    sa.sa_handler = signalHandler;
    sa.sa_flags = 0;    /* Interrupt system calls */
    sigemptyset(&sa.sa_mask);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
#ifdef TCSUPPORT_IGMP_QOS
	sigaction(SIGUSR1, &sa, NULL); /* this message is from cfg manager when commit QoS node */
#endif

    // Loads configuration for Physical interfaces...
    buildIfVc();    
    
    // Configures IF states and settings
    configureVifs();

    switch ( Err = enableMRouter() ) {
    case 0: break;
    case EADDRINUSE: log( LOG_ERR, EADDRINUSE, "MC-Router API already in use" ); break;
    default: log( LOG_ERR, Err, "MRT_INIT failed" );
    }

    /* create VIFs for all IP, non-loop interfaces
     */
    {
        unsigned Ix;
        struct IfDesc *Dp;
        int     vifcount = 0;
        upStreamVif = -1;

        for ( Ix = 0; Dp = getIfByIx( Ix ); Ix++ ) {

            if ( Dp->InAdr.s_addr && ! (Dp->Flags & IFF_LOOPBACK) ) {
                if(Dp->state == IF_STATE_UPSTREAM) {
                    if(upStreamVif == -1) {
                        upStreamVif = Ix;
                    } else {
                        log(LOG_ERR, 0, "Vif #%d was already upstream. Cannot set VIF #%d as upstream as well.",
                            upStreamVif, Ix);
                    }
                }

                addVIF( Dp );
                vifcount++;
            }
        }

        // If there is only one VIF, or no defined upstream VIF, we send an error.
        if(vifcount < 2 || upStreamVif < 0) {
            log(LOG_ERR, 0, "There must be at least 2 Vif's where one is upstream.");
        }
    }  
    
    // Initialize IGMP
    initIgmp();
    // Initialize Routing table
    initRouteTable();
    // Initialize timer
    callout_init();


    return 1;
}

/**
*   Clean up all on exit...
*/
void igmpProxyCleanUp() {

    log( LOG_DEBUG, 0, "clean handler called" );
    
    free_all_callouts();    // No more timeouts.
    clearAllRoutes();       // Remove all routes.
    disableMRouter();       // Disable the multirout API

}

/**
*   Main daemon loop.
*/
void igmpProxyRun() {
    // Get the config.
    //struct Config *config = getCommonConfig();
    // Set some needed values.
    register int recvlen;
    int     MaxFD, Rt, secs;
    fd_set  ReadFDS;
    int     dummy = 0, sock_opt = 1;
#ifdef TRENDCHIP
    struct  timespec  curtime, lasttime, difftime, tv; 
#else
    struct  timeval  curtime, lasttime, difftime, tv; 
#endif
    // The timeout is a pointer in order to set it to NULL if nessecary.
    struct  timeval  *timeout = &tv;

    // Initialize timer vars
#ifdef TRENDCHIP
    difftime.tv_nsec = 0;
	clock_gettime(CLOCK_MONOTONIC, &curtime);
#else
    difftime.tv_usec = 0;
    gettimeofday(&curtime, NULL);
#endif
    lasttime = curtime;

    // First thing we send a membership query in downstream VIF's...
    sendGeneralMembershipQuery();
#if defined (TCSUPPORT_CT_JOYME4)
	if ((setsockopt(MRouterFD, SOL_SOCKET, SO_TYPE_COPY_SKB_MARK, (const void*)&sock_opt, sizeof(sock_opt))) < 0) 
	{
		printf("error: set socket option failed");
	}
#endif

    // Loop until the end...
    for (;;) {

        // Process signaling...
        if (sighandled) {
            if (sighandled & GOT_SIGINT) {
                sighandled &= ~GOT_SIGINT;
                log(LOG_NOTICE, 0, "Got a interupt signal. Exiting.");
                break;
            }
        }

        // Prepare timeout...
        secs = timer_nextTimer();
        if(secs == -1) {
            timeout = NULL;
        } else {
            timeout->tv_usec = 0;
            timeout->tv_sec = secs;
        }

        // Prepare for select.
        MaxFD = MRouterFD;

        FD_ZERO( &ReadFDS );
        FD_SET( MRouterFD, &ReadFDS );

        // wait for input
        Rt = select( MaxFD +1, &ReadFDS, NULL, NULL, timeout );

        // log and ignore failures
        if( Rt < 0 ) {
            log( LOG_WARNING, errno, "select() failure" );
            continue;
        }
        else if( Rt > 0 ) {

            // Read IGMP request, and handle it...
            if( FD_ISSET( MRouterFD, &ReadFDS ) ) {
    
                recvlen = recvfrom(MRouterFD, recv_buf, RECV_BUF_SIZE,
                                   0, NULL, &dummy);
                if (recvlen < 0) {
                    if (errno != EINTR) log(LOG_ERR, errno, "recvfrom");
                    continue;
                }
			#ifdef TCSUPPORT_IGMP_QOS
				int len = sizeof(uint32);
				if (getsockopt(MRouterFD, SOL_IP, IP_SKB_MARK, &skb_mark, &len)) {
					/* dbg_info */
					;//tcdbg_printf("%s,getsockopt error\n", __FUNCTION__);
				}
				/*tcdbg_printf("xyz_dbg:%s, skb_mark is %x\n", __FUNCTION__, skb_mark);	*/
#if defined(TCSUPPORT_CT_JOYME4)
				if ( 0 != check_igmp_packets(skb_mark) )
					continue;
#endif
			#endif
                acceptIgmp(recvlen);
            }
        }

        // At this point, we can handle timeouts...
        do {
            /*
             * If the select timed out, then there's no other
             * activity to account for and we don't need to
             * call gettimeofday.
             */
            if (Rt == 0) {
                curtime.tv_sec = lasttime.tv_sec + secs;
#ifdef TRENDCHIP
                curtime.tv_nsec = lasttime.tv_nsec;
#else
                curtime.tv_usec = lasttime.tv_usec;
#endif
                Rt = -1; /* don't do this next time through the loop */
            } else {
#ifdef TRENDCHIP
				clock_gettime(CLOCK_MONOTONIC, &curtime);
#else
                gettimeofday(&curtime, NULL);
#endif
            }
#ifdef TRENDCHIP
            difftime.tv_sec = curtime.tv_sec - lasttime.tv_sec;
            difftime.tv_nsec += curtime.tv_nsec - lasttime.tv_nsec;
            while (difftime.tv_nsec > 1000000000) {
                difftime.tv_sec++;
                difftime.tv_nsec -= 1000000000;
            }
            if (difftime.tv_nsec < 0) {
                difftime.tv_sec--;
                difftime.tv_nsec += 1000000000;
            }
#else
            difftime.tv_sec = curtime.tv_sec - lasttime.tv_sec;
            difftime.tv_usec += curtime.tv_usec - lasttime.tv_usec;
            while (difftime.tv_usec > 1000000) {
                difftime.tv_sec++;
                difftime.tv_usec -= 1000000;
            }
            if (difftime.tv_usec < 0) {
                difftime.tv_sec--;
                difftime.tv_usec += 1000000;
            }
#endif
            lasttime = curtime;
            if (secs == 0 || difftime.tv_sec > 0)
                age_callout_queue(difftime.tv_sec);
            secs = -1;
        } while (difftime.tv_sec > 0);

    }

}

/*
 * Signal handler.  Take note of the fact that the signal arrived
 * so that the main loop can take care of it.
 */
static void signalHandler(int sig) {
    switch (sig) {
    case SIGINT:
    case SIGTERM:
        sighandled |= GOT_SIGINT;
        break;
	/* this message is used for cfg_manager when commit QoS cfg node */
#ifdef TCSUPPORT_IGMP_QOS
	case SIGUSR1:
		/* send query to lan side */
		//tcdbg_printf("receive a signal.\n");
		sendGeneralMembershipQueryBySignal();
		break;
#endif
        /* XXX: Not in use.
        case SIGHUP:
            sighandled |= GOT_SIGHUP;
            break;
    
        case SIGUSR1:
            sighandled |= GOT_SIGUSR1;
            break;
    
        case SIGUSR2:
            sighandled |= GOT_SIGUSR2;
            break;
        */
    }
}
