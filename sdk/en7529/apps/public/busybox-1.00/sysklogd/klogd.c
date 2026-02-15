/* vi: set sw=4 ts=4: */
/*
 * Mini klogd implementation for busybox
 *
 * Copyright (C) 2001 by Gennady Feldman <gfeldman@gena01.com>.
 * Changes: Made this a standalone busybox module which uses standalone
 * 					syslog() client interface.
 *
 * Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * Copyright (C) 2000 by Karl M. Hegbloom <karlheg@debian.org>
 *
 * "circular buffer" Copyright (C) 2000 by Gennady Feldman <gfeldman@gena01.com>
 *
 * Maintainer: Gennady Feldman <gfeldman@gena01.com> as of Mar 12, 2001
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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>		/* for our signal() handlers */
#include <string.h>		/* strncpy() */
#include <errno.h>		/* errno and friends */
#include <unistd.h>
#include <ctype.h>
#include <sys/syslog.h>
#include <sys/klog.h>

#include "busybox.h"

static void klogd_signal(int sig)
{
	klogctl(7, NULL, 0);
	klogctl(0, 0, 0);
	/* logMessage(0, "Kernel log daemon exiting."); */
	syslog(LOG_NOTICE, "Kernel log daemon exiting.");
	exit(EXIT_SUCCESS);
}

static void doKlogd(const int console_log_level) __attribute__ ((noreturn));
static void doKlogd(const int console_log_level)
{
	int priority = LOG_INFO;
	char log_buffer[4096];
	int i, n, lastc;
	char *start;
#if defined(TCSUPPORT_CT)
	int TCSysLog = 0;
#endif
#if defined(TCSUPPORT_CT_JOYME4) && defined(TCSUPPORT_SYSLOG)
	char str_firewall[] = "Firewall";
	char str_smurf[] = "Internet attack detected(SMURF)";
	char str_synflood[] = "Internet attack detected(SYNFLOOD)";
	char str_land[] = "Internet attack detected(LAND)";
	char str_PingOfDeath[] = "Internet attack detected(Ping Of Death)";
	char str_ICMP_Redirection[] = "Internet attack detected(ICMP Redirection)";
	char str_oversized_pkt[] = "Oversized IP packet from";
	static int count = 0;
	static time_t last_time = 0;
	time_t this_time = 0;
#endif

	openlog("kernel", 0, LOG_KERN);

	/* Set up sig handlers */
	signal(SIGINT, klogd_signal);
	signal(SIGKILL, klogd_signal);
	signal(SIGTERM, klogd_signal);
	signal(SIGHUP, SIG_IGN);

	/* "Open the log. Currently a NOP." */
	klogctl(1, NULL, 0);

	/* Set level of kernel console messaging.. */
	if (console_log_level != -1)
		klogctl(8, NULL, console_log_level);

	syslog(LOG_NOTICE, "klogd started: " BB_BANNER);

	while (1) {
		/* Use kernel syscalls */
		memset(log_buffer, '\0', sizeof(log_buffer));
		n = klogctl(2, log_buffer, sizeof(log_buffer));
		if (n < 0) {
			if (errno == EINTR)
				continue;
			syslog(LOG_ERR, "klogd: Error return from sys_sycall: %d - %m.\n", errno);
			exit(EXIT_FAILURE);
		}

		/* klogctl buffer parsing modelled after code in dmesg.c */
		start = &log_buffer[0];
		lastc = '\0';
		for (i = 0; i < n; i++) {
			if (lastc == '\0' && log_buffer[i] == '<') {
				priority = 0;
				i++;
				while (isdigit(log_buffer[i])) {
					priority = priority * 10 + (log_buffer[i] - '0');
					i++;
				}
				if (log_buffer[i] == '>')
					i++;
				start = &log_buffer[i];
			}
			if (log_buffer[i] == '\n') {
				log_buffer[i] = '\0';	/* zero terminate this message */
#if defined(TCSUPPORT_CT)	
				if(strncmp(start, "TCSysLog", 8) == 0)
				{
					if(TCSysLog == 0)
					{
						closelog();
						openlog("TCSysLog FTP", 0, LOG_DAEMON);
						TCSysLog = 1;
					}					
					start+=9;
				}
				else
				{				
					if(TCSysLog == 1)
					{
						closelog();
						openlog("kernel", 0, LOG_KERN);
						TCSysLog = 0;
					}
				}				
#endif
				syslog(priority, "%s", start);
#if defined(TCSUPPORT_CT_JOYME4) && defined(TCSUPPORT_SYSLOG)
				if(strstr(start, str_oversized_pkt) != NULL)
				{
					this_time = time(NULL);
					/* Timeout is 10 sec. */
					if((this_time - last_time) < 10)
						count ++;
					else
						count = 0;

					last_time = this_time;

					if(count == 5)
					{
						closelog();
						openlog("TCSysLog Firewall", 0, LOG_KERN);
						start = start + strlen(str_firewall) + 1;
						priority = LOG_ALERT;
						syslog(priority, "%s", str_PingOfDeath);
						closelog();
						openlog("kernel", 0, LOG_KERN);
						count = 0;
					}
				}
				if(strncmp(start, str_firewall, strlen(str_firewall)) == 0)
				{
					char *reason = NULL;
					closelog();
					openlog("TCSysLog Firewall", 0, LOG_KERN);
					start = start + strlen(str_firewall) + 1;
					priority = LOG_ALERT;
					switch(start[0])
					{
						case 's':
							syslog(priority, "%s", str_smurf);
							break;
						case 'f':
							syslog(priority, "%s", str_synflood);
							break;
						case 'l':
							syslog(priority, "%s", str_land);
							break;
						case 'r':
							syslog(priority, "%s", str_ICMP_Redirection);
							break;
						default:
							syslog(priority, "%s", "Cannot get firewall substring");
					}
					closelog();
					openlog("kernel", 0, LOG_KERN);
				}
#endif
				start = &log_buffer[i + 1];
				priority = LOG_INFO;
			}
			lastc = log_buffer[i];
		}
	}
}

extern int klogd_main(int argc, char **argv)
{
	/* no options, no getopt */
	int opt;
	int doFork = TRUE;
	unsigned char console_log_level = -1;

	/* do normal option parsing */
	while ((opt = getopt(argc, argv, "c:n")) > 0) {
		switch (opt) {
		case 'c':
			if ((optarg == NULL) || (optarg[1] != '\0')) {
				bb_show_usage();
			}
			/* Valid levels are between 1 and 8 */
			console_log_level = *optarg - '1';
			if (console_log_level > 7) {
				bb_show_usage();
			}
			console_log_level++;

			break;
		case 'n':
			doFork = FALSE;
			break;
		default:
			bb_show_usage();
		}
	}

	if (doFork) {
#if defined(__uClinux__)
		vfork_daemon_rexec(0, 1, argc, argv, "-n");
#else /* __uClinux__ */
		if (daemon(0, 1) < 0)
			bb_perror_msg_and_die("daemon");
#endif /* __uClinux__ */
	}
	doKlogd(console_log_level);

	return EXIT_SUCCESS;
}

/*
Local Variables
c-file-style: "linux"
c-basic-offset: 4
tab-width: 4
End:
*/
