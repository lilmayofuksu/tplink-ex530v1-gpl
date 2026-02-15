/* vi: set sw=4 ts=4: */
/*
 * Mini reboot implementation for busybox
 *
 * Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <time.h>
#include "busybox.h"
#include "init_shared.h"
#include "blapi_traffic.h"

#if defined(TCSUPPORT_CFG_NG)
#if defined(TCSUPPORT_SNMP_FULL)
#include <ecnt_event_global/ecnt_event_global.h>
#include <ecnt_event_global/ecnt_event_system.h>
#endif
#endif

#if defined(TCSUPPORT_START_TRAP) || defined(TCSUPPORT_SYSLOG_ENHANCE)
#define		SIGNAL_PATH		"/var/tmp/signal_reboot"
static int quit_signal(void)
{
	int ret;
	FILE *fp = NULL;
	char buf[8];

	memset(buf, 0, sizeof(buf));
	
	fp = fopen(SIGNAL_PATH, "r");

	if (fp == NULL)
		ret = 0;
	else {
		fgets(buf, 8, fp);
		
		if (buf[0] == '1')
			ret = 1;
		else 
			ret = 0;

		fclose(fp);
		unlink(SIGNAL_PATH);
	}

	return ret;
}
#endif
/*_____________________________________________________________________________
**      function name: executeWifiDownOp
**      descriptions:
**            This function is use for doing wifi interface down 
**
**      parameters:
**             mode:
**             1:11ac;0:11n
**
**      global:
**            none
**
**      return:
**            None
**
**      call:
**   	 None
**
**      revision:
**      Brian.shi 20151111
**____________________________________________________________________________
*/
void executeWifiDownOp(int mode)
{
	FILE *fp = NULL;
	char buf[65] = {0};
	char APOn[4] = {0};
	char BssidNum[4] = {0};
	char WDSSwitch[4] = {0};
	char tmp[32] = {0};
	int i;
	char wifipatch[32];
	char wificmd[32];
	char wifiwdscmd[32];
	if(mode)
	{
		strcpy(wifipatch,"/etc/Wireless/WLAN_APOn_AC");
		strcpy(wificmd,"ifconfig rai%d down");
		strcpy(wifiwdscmd,"ifconfig wdsi%d down");
	}
	else
	{
		strcpy(wifipatch,"/etc/Wireless/WLAN_APOn");
		strcpy(wificmd,"ifconfig ra%d down");
		strcpy(wifiwdscmd,"ifconfig wds%d down");
	}
	
	fp = fopen(wifipatch, "r");
	if(!fp){
		printf("\ncan't open %s",wifipatch);
		for (i=0; i<8; i++) {
			sprintf(tmp, wificmd,i);
			system(tmp);
		}
#ifdef TCSUPPORT_WLAN_WDS
		for (i=0; i<4; i++) {
			sprintf(tmp, wifiwdscmd,i);
			system(tmp);
		}
#endif
	}
	else{
		while(fgets(buf, 64, fp) != NULL){
			if(strstr(buf, "APOn=") != NULL)
				sscanf(buf, "APOn=%s", APOn);
			else if(strstr(buf, "Bssid_num=") != NULL)
				sscanf(buf, "Bssid_num=%s", BssidNum);
			#ifdef TCSUPPORT_WLAN_WDS
			else if(strstr(buf, "WdsEnable=") != NULL)
				sscanf(buf, "WdsEnable=%s", WDSSwitch);
			#endif
			memset(buf,0,sizeof(buf));
		}
		fclose(fp);
		
		if (!strcmp(APOn,"1")){
			#ifdef TCSUPPORT_WLAN_WDS
			if (!strcmp(WDSSwitch,"1")){
				for (i=0; i<4; i++) {
				sprintf(tmp, wifiwdscmd,i);
				system(tmp);
				}
			}
			#endif
			for (i=0; i<atoi(BssidNum); i++) {
				sprintf(tmp, wificmd,i);
				system(tmp);
			}		
		}
	}
}
#if defined(TCSUPPORT_CT_SIMCARD_SEPARATION)
#define SIMSIGFILE  "/var/tmp/simsignal"
int readSingalFile(void){
	FILE *fp = NULL;
	char buf[10] = "\0";
	int ret = 0;
	
	fp = fopen(SIMSIGFILE, "r");
	if(fp == NULL){
		printf("\n readSingalFile: open failed");
		return 0;
	}
	if(fgets(buf, 10, fp)){
		ret = atoi(buf);
	}
	fclose(fp);
	return ret;
}
#endif

#if defined(TCSUPPORT_CT_JOYME2)
#define FILE_REBOOT_SRC		"/opt/upt/apps/info/reboot_source"
#define TMP_THREAD_PID		"/tmp/reboot_thread_pid"
void save_dev_reboot_info()
{
	int ppid = 0, found = 0, len = 0;
	FILE *fp = NULL;
	char buf[256] = {0}, pidbuf[32] = {0}, source[16] = {0};
	int year, mon, day, hour, min, sec = 0;
	struct timespec curtime;
	struct tm nowTime;
	char now_time[20] = {0};
	char cmdbuf[128] = {0};
	
	const char* json_format = "{\"Time\":\"%s\","
							  "\"Source\":\"%s\"}";
	/* get nowtime */
	clock_gettime(CLOCK_REALTIME, &curtime);
	localtime_r(&curtime.tv_sec, &nowTime);
	year = nowTime.tm_year + 1900;
	mon = nowTime.tm_mon + 1;
	day = nowTime.tm_mday;
	hour = nowTime.tm_hour;
	min = nowTime.tm_min;
	sec = nowTime.tm_sec;
	snprintf(now_time, sizeof(now_time), "%04d-%02d-%02d %02d:%02d:%02d", year, mon, day, hour, min, sec);

	/* get reboot souce */
	memset(source, 0, sizeof(source));
	ppid = getppid();

	memset(cmdbuf, 0, sizeof(cmdbuf));
	snprintf(cmdbuf, sizeof(cmdbuf), "pidof sh > %s", TMP_THREAD_PID);
	system(cmdbuf);

	/* check source: terminal */
	fp = fopen(TMP_THREAD_PID, "r");
	if( !fp ){
		return;
	}
	else{
		memset(pidbuf, 0, sizeof(pidbuf));
		len = fread(pidbuf, sizeof(char), sizeof(pidbuf) - 1, fp);
		fclose(fp);
		fp = NULL;
		if (len <= 0){
			return;
		}
		else{
			if (ppid == atoi(pidbuf)){
				found = 1;
			}
		}
		len = 0;
	}

	if (found){
		strncpy(source, "Terminal", sizeof(source) - 1);
	}
	else{
		fp = fopen("/tmp/reboot_source", "r");
		if( NULL == fp ){
			printf("\n file: /tmp/reboot_source open failed\n");
			return;
		}
		len = fread(source, sizeof(char), sizeof(source) - 1, fp);
		fclose(fp);
		if (len <= 0){
			return;
		}
		else
			source[len] = 0;
	}

	/* write to reboot_info */
	fp = fopen(FILE_REBOOT_SRC, "w+");
	if( NULL == fp ){
		printf("\n file: %s open failed\n", FILE_REBOOT_SRC);
		return;
	}
	
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), json_format, now_time, source);
	fputs(buf, fp);
	fclose(fp);
	unlink(TMP_THREAD_PID);
	unlink("/tmp/reboot_source");

	return;
}
#endif

#if defined(TCSUPPORT_CMCCV2)
#define TMP_THREAD_PID		"/tmp/reboot_thread_pid"

	int	check_term_reboot()
{
	char cmdbuf[128] = {0};
	int ppid = 0, len = 0;
	char buf[256] = {0}, pidbuf[32] = {0};
	FILE *fp = NULL;

	ppid = getppid();

	memset(cmdbuf, 0, sizeof(cmdbuf));
	snprintf(cmdbuf, sizeof(cmdbuf), "pidof sh > %s", TMP_THREAD_PID);
	system(cmdbuf);

/* check source: terminal */
	fp = fopen(TMP_THREAD_PID, "r");
	if( !fp ){
		return 0;
	}
	else{
		memset(pidbuf, 0, sizeof(pidbuf));
		len = fread(pidbuf, sizeof(char), sizeof(pidbuf) - 1, fp);
		fclose(fp);
		fp = NULL;
		if (len <= 0){
			return 0;
		}
		else{
			if (ppid == atoi(pidbuf)){
				return 1;
			}
		}
		len = 0;
	}

	return 0;
}
#endif
extern int reboot_main(int argc, char **argv)
{
	int rt = -1;
	char *delay; /* delay in seconds before rebooting */
#if defined(TCSUPPORT_CT_SIMCARD_SEPARATION)
	int pid = 0,num = 0;
	union sigval simsigval;
#endif
	char data[64] = {0};

#if defined(TCSUPPORT_CMCCV2)
	if(1 == check_term_reboot())
	{
			write_logMsg_to_flash(1);
	}
#endif

#if defined(TCSUPPORT_START_TRAP) || defined(TCSUPPORT_SYSLOG_ENHANCE)
	int count = 0;
#endif
	if(bb_getopt_ulflags(argc, argv, "d:", &delay)) {
		sleep(atoi(delay));
	}

#if defined(TCSUPPORT_CFG_NG)
#if defined(TCSUPPORT_SNMP_FULL)
	snprintf(data, sizeof(data), "%s", "warm reboot");
	rt = ecnt_event_send(ECNT_EVENT_SYSTEM, ECNT_EVENT_SNMP, data, strlen(data) + 1);
#endif
#endif

#if defined(TCSUPPORT_CT_SIMCARD_SEPARATION)
	pid = readSingalFile();
	if(pid){
		simsigval.sival_int = 8;
		printf("sigqueue:%d!\n",pid);
		sigqueue(pid, SIGUSR2, simsigval);
	}
	while(pid = readSingalFile() && num++ < 10){
		sleep(1);
	}
#endif
#if defined(TCSUPPORT_START_TRAP) || defined(TCSUPPORT_SYSLOG_ENHANCE)	
	system("killall -SIGUSR1 cfg_manager");
	/* wait cfg_manager done */
	while (!quit_signal() && count++ < 5)
		sleep(1);
	
#endif
#ifdef TCSUPPORT_SYSLOG_ENHANCE
	system("killall -9 syslogd");
#endif

#if defined(TCSUPPORT_CFG_NG)
	system("killall boa");
	//delay the timing of closing wifi interface due to avoiding crash when wlan is init.
	system("/usr/script/check_reboot_need_delay.sh ");
#endif
#ifdef TCSUPPORT_WLAN
	executeWifiDownOp(0);
#endif
#ifdef TCSUPPORT_WLAN_AC
	executeWifiDownOp(1);
#endif
#if defined(TCSUPPORT_CT_JOYME2)
	save_dev_reboot_info();
#endif

/*
 * When system reboot, set qdma rx_ratelimit to 0.
 * Otherwise, reboot will stuck when high throughput "LAN to WAN" or "WAN to LAN" traffic.
 */
	rt = blapi_traffic_set_general_rx_ratelimit();

#ifndef CONFIG_INIT
#ifndef RB_AUTOBOOT
#define RB_AUTOBOOT		0x01234567
#endif
	return(bb_shutdown_system(RB_AUTOBOOT));
#else
#if !defined(TCSUPPORT_CT)
	return kill_init(SIGTERM);
#else
#define RB_AUTOBOOT		0x01234567
	printf("\nbb_shutdown_system!!!\n");
	return(bb_shutdown_system(RB_AUTOBOOT));
#endif
#endif
}

/*
Local Variables:
c-file-style: "linux"
c-basic-offset: 4
tab-width: 4
End:
*/
