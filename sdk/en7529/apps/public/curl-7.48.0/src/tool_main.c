/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2014, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/
#include "tool_setup.h"

#include <sys/stat.h>

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef USE_NSS
#include <nspr.h>
#include <plarenas.h>
#endif

#if defined(TCSUPPORT_TR143_CURL_UPLOAD)
/*
    TR143 error code.
*/
const unsigned char State_None = 0;
const unsigned char State_Complete = 1;
const unsigned char State_Error_InitConnectionFailed = 2;
const unsigned char State_Error_NoResponse = 3;
const unsigned char State_Error_TransferFailed = 4;
/*
    TR143 input parameters
*/
char tr143_uldoutdoc[128];
char tr143_uldpidfile[128];
char tr143_uldlogfile[128];
char tr143_uldinterface[64];
unsigned int tr143_uldwantsize;
unsigned int tr143_uldrdsize;
int tr143_uldethpri;
int tr143_ulddiag_flg;
/*
    TR143 upload statistics info
*/
struct timeval g_sysTimeTCPOpenRequest;
struct timeval g_sysTimeTCPOpenResponse;
struct timeval g_sysTimeROM;
struct timeval g_sysTimeBOM;
struct timeval g_sysTimeEOM;
unsigned int g_BOMOutOctet;
unsigned int g_EOMOutOctet;
unsigned int g_diagstate;
/*
    TR143 upload output struct
*/
typedef struct _TR143UldDiagInfo_
{
	struct timeval sysTimeTCPOpenRequest;
	struct timeval sysTimeTCPOpenResponse;
	struct timeval sysTimeROM;
	struct timeval sysTimeBOM;
	struct timeval sysTimeEOM;
	unsigned int BOMOutOctet;
	unsigned int EOMOutOctet;
	unsigned int diagtate;
} TR143UldDiagInfo;
#endif

const unsigned char tr69UldState_None = 0;
const unsigned char tr69UldState_Complete = 1;
const unsigned char tr69UldState_Error_UploadFailed = 2;
const unsigned char tr69UldState_Error_NoAuthentication = 3;
const unsigned char tr69UldState_Error_UnSupport = 4;



char tr069_uploadpidfile[128];
char tr069_uploadlogfile[128];
int tr069_upload_flg;

unsigned int g_tr069uldStatus;
struct timeval g_tr069uldStartTime;
struct timeval g_tr069uldCompleteTime;

typedef struct _TR069UploadInfo_
{
	unsigned int tr069uldStatus;
	struct timeval tr069uldStartTime;
	struct timeval tr069uldCompleteTime;
} TR069UploadInfo;

char speedtest_mode[16];

#define ENABLE_CURLX_PRINTF
/* use our own printf() functions */
#include "curlx.h"

#include "tool_cfgable.h"
#include "tool_convert.h"
#include "tool_msgs.h"
#include "tool_operate.h"
#include "tool_panykey.h"
#include "tool_vms.h"
#include "tool_main.h"
#include "tool_libinfo.h"

/*
 * This is low-level hard-hacking memory leak tracking and similar. Using
 * the library level code from this client-side is ugly, but we do this
 * anyway for convenience.
 */
#include "memdebug.h" /* keep this as LAST include */

#ifdef __VMS
/*
 * vms_show is a global variable, used in main() as parameter for
 * function vms_special_exit() to allow proper curl tool exiting.
 * Its value may be set in other tool_*.c source files thanks to
 * forward declaration present in tool_vms.h
 */
int vms_show = 0;
#endif

/* if we build a static library for unit tests, there is no main() function */
#ifndef UNITTESTS

/*
 * Ensure that file descriptors 0, 1 and 2 (stdin, stdout, stderr) are
 * open before starting to run.  Otherwise, the first three network
 * sockets opened by curl could be used for input sources, downloaded data
 * or error logs as they will effectively be stdin, stdout and/or stderr.
 */
static void main_checkfds(void)
{
#ifdef HAVE_PIPE
  int fd[2] = { STDIN_FILENO, STDIN_FILENO };
  while(fd[0] == STDIN_FILENO ||
        fd[0] == STDOUT_FILENO ||
        fd[0] == STDERR_FILENO ||
        fd[1] == STDIN_FILENO ||
        fd[1] == STDOUT_FILENO ||
        fd[1] == STDERR_FILENO)
    if(pipe(fd) < 0)
      return;   /* Out of handles. This isn't really a big problem now, but
                   will be when we try to create a socket later. */
  close(fd[0]);
  close(fd[1]);
#endif
}

#ifdef CURLDEBUG
static void memory_tracking_init(void)
{
  char *env;
  /* if CURL_MEMDEBUG is set, this starts memory tracking message logging */
  env = curlx_getenv("CURL_MEMDEBUG");
  if(env) {
    /* use the value as file name */
    char fname[CURL_MT_LOGFNAME_BUFSIZE];
    if(strlen(env) >= CURL_MT_LOGFNAME_BUFSIZE)
      env[CURL_MT_LOGFNAME_BUFSIZE-1] = '\0';
    strcpy(fname, env);
    curl_free(env);
    curl_memdebug(fname);
    /* this weird stuff here is to make curl_free() get called
       before curl_memdebug() as otherwise memory tracking will
       log a free() without an alloc! */
  }
  /* if CURL_MEMLIMIT is set, this enables fail-on-alloc-number-N feature */
  env = curlx_getenv("CURL_MEMLIMIT");
  if(env) {
    char *endptr;
    long num = strtol(env, &endptr, 10);
    if((endptr != env) && (endptr == env + strlen(env)) && (num > 0))
      curl_memlimit(num);
    curl_free(env);
  }
}
#else
#  define memory_tracking_init() Curl_nop_stmt
#endif

/*
 * This is the main global constructor for the app. Call this before
 * _any_ libcurl usage. If this fails, *NO* libcurl functions may be
 * used, or havoc may be the result.
 */
static CURLcode main_init(struct GlobalConfig *config)
{
  CURLcode result = CURLE_OK;

#if defined(__DJGPP__) || defined(__GO32__)
  /* stop stat() wasting time */
  _djstat_flags |= _STAT_INODE | _STAT_EXEC_MAGIC | _STAT_DIRSIZE;
#endif

  /* Initialise the global config */
  config->showerror = -1;             /* Will show errors */
  config->errors = stderr;            /* Default errors to stderr */

  /* Allocate the initial operate config */
  config->first = config->last = malloc(sizeof(struct OperationConfig));
  if(config->first) {
    /* Perform the libcurl initialization */
    result = curl_global_init(CURL_GLOBAL_DEFAULT);
    if(!result) {
      /* Get information about libcurl */
      result = get_libcurl_info();

      if(!result) {
        /* Get a curl handle to use for all forthcoming curl transfers */
        config->easy = curl_easy_init();
        if(config->easy) {
          /* Initialise the config */
          config_init(config->first);
          config->first->easy = config->easy;
          config->first->global = config;
        }
        else {
          helpf(stderr, "error initializing curl easy handle\n");
          result = CURLE_FAILED_INIT;
          free(config->first);
        }
      }
      else {
        helpf(stderr, "error retrieving curl library information\n");
        free(config->first);
      }
    }
    else {
      helpf(stderr, "error initializing curl library\n");
      free(config->first);
    }
  }
  else {
    helpf(stderr, "error initializing curl\n");
    result = CURLE_FAILED_INIT;
  }

  return result;
}

static void free_config_fields(struct GlobalConfig *config)
{
  Curl_safefree(config->trace_dump);

  if(config->errors_fopened && config->errors)
    fclose(config->errors);
  config->errors = NULL;

  if(config->trace_fopened && config->trace_stream)
    fclose(config->trace_stream);
  config->trace_stream = NULL;

  Curl_safefree(config->libcurl);
}

/*
 * This is the main global destructor for the app. Call this after
 * _all_ libcurl usage is done.
 */
static void main_free(struct GlobalConfig *config)
{
  /* Cleanup the easy handle */
  curl_easy_cleanup(config->easy);
  config->easy = NULL;

  /* Main cleanup */
  curl_global_cleanup();
  convert_cleanup();
  metalink_cleanup();
#ifdef USE_NSS
  if(PR_Initialized()) {
    /* prevent valgrind from reporting still reachable mem from NSRP arenas */
    PL_ArenaFinish();
    /* prevent valgrind from reporting possibly lost memory (fd cache, ...) */
    PR_Cleanup();
  }
#endif
  free_config_fields(config);

  /* Free the config structures */
  config_free(config->last);
  config->first = NULL;
  config->last = NULL;
}

/*
** curl tool main function.
*/
int main(int argc, char *argv[])
{
  CURLcode result = CURLE_OK;
  struct GlobalConfig global;
  memset(&global, 0, sizeof(global));

  main_checkfds();

#if defined(HAVE_SIGNAL) && defined(SIGPIPE)
  (void)signal(SIGPIPE, SIG_IGN);
#endif

  /* Initialize memory tracking */
  memory_tracking_init();

  /* Initialize the curl library - do not call any libcurl functions before
     this point */
  result = main_init(&global);
  if(!result) {
    /* Start our curl operation */
#if defined(TCSUPPORT_TR143_CURL_UPLOAD)
	/* init tr143 input params */
	memset(tr143_uldoutdoc, 0x00, sizeof(tr143_uldoutdoc));
	memset(tr143_uldpidfile, 0x00, sizeof(tr143_uldpidfile));
	memset(tr143_uldlogfile, 0x00, sizeof(tr143_uldlogfile));
	memset(tr143_uldinterface, 0x00, sizeof(tr143_uldinterface));
	tr143_uldwantsize = 0;
	tr143_uldrdsize = 0;
	tr143_uldethpri = 0;
	tr143_ulddiag_flg = 0;

	memset(&g_sysTimeTCPOpenRequest, 0x00, sizeof(g_sysTimeTCPOpenRequest));
	memset(&g_sysTimeTCPOpenResponse, 0x00, sizeof(g_sysTimeTCPOpenResponse));
	memset(&g_sysTimeROM, 0x00, sizeof(g_sysTimeROM));
	memset(&g_sysTimeBOM, 0x00, sizeof(g_sysTimeBOM));
	memset(&g_sysTimeEOM, 0x00, sizeof(g_sysTimeEOM));
	g_BOMOutOctet = 0;
	g_EOMOutOctet = 0;
	g_diagstate = State_None;	
#endif
	memset(tr069_uploadpidfile, 0x00, sizeof(tr069_uploadpidfile));
	memset(tr069_uploadlogfile, 0x00, sizeof(tr069_uploadlogfile));
	tr069_upload_flg = 0;
	g_tr069uldStatus = 0;
	memset(&g_tr069uldStartTime, 0x00, sizeof(g_tr069uldStartTime));
	memset(&g_tr069uldCompleteTime, 0x00, sizeof(g_tr069uldCompleteTime));
	memset(speedtest_mode, 0x00, sizeof(speedtest_mode));

	gettimeofday(&g_tr069uldStartTime, NULL);
    result = operate(&global, argc, argv);
    gettimeofday(&g_tr069uldCompleteTime, NULL);

    switch(result)
    {
	    case CURLE_OK:
	    case CURLE_WRITE_ERROR:
		    g_tr069uldStatus = tr69UldState_Complete;
		    break;
	    case CURLE_UPLOAD_FAILED:
		    g_tr069uldStatus = tr69UldState_Error_UploadFailed;
		    break;
	    case CURLE_LOGIN_DENIED:
		    g_tr069uldStatus = tr69UldState_Error_NoAuthentication;
		    break;
	    case CURLE_UNSUPPORTED_PROTOCOL:
		    g_tr069uldStatus = tr69UldState_Error_UnSupport;
		    break;
	    default:
		    g_tr069uldStatus = tr69UldState_None;
		    break;
    }

#ifdef __SYMBIAN32__
    if(global.showerror)
      tool_pressanykey();
#endif

    /* Perform the main cleanup */
    main_free(&global);
  }

#ifdef __NOVELL_LIBC__
  if(getenv("_IN_NETWARE_BASH_") == NULL)
    tool_pressanykey();
#endif

#ifdef __VMS
  vms_special_exit(result, vms_show);
#else
#if defined(TCSUPPORT_TR143_CURL_UPLOAD)
	/* write statistics to temp file. */
	if (tr143_uldlogfile[0] && tr143_ulddiag_flg)
	{
		TR143UldDiagInfo inf;
		memset(&inf, 0x00, sizeof(TR143UldDiagInfo));
		g_EOMOutOctet = tr143_uldrdsize;
		
		inf.sysTimeTCPOpenRequest = g_sysTimeTCPOpenRequest;
		inf.sysTimeTCPOpenResponse = g_sysTimeTCPOpenResponse;
		inf.sysTimeROM = g_sysTimeROM;
		inf.sysTimeBOM = g_sysTimeBOM;
		inf.sysTimeEOM = g_sysTimeEOM;
		inf.BOMOutOctet = g_BOMOutOctet;
		inf.EOMOutOctet = g_EOMOutOctet;
		inf.diagtate = g_diagstate;

	  	FILE* fp = fopen(tr143_uldlogfile, "w");
	  	if (fp)
	  	{
			fwrite(&inf, sizeof(TR143UldDiagInfo), 1, fp);
			fclose(fp);
		}
	}
#endif
	if (tr069_uploadlogfile[0] && tr069_upload_flg)
	{
		TR069UploadInfo tr069updinf;
		memset(&tr069updinf, 0x00, sizeof(TR069UploadInfo));

		tr069updinf.tr069uldStatus = g_tr069uldStatus;
		tr069updinf.tr069uldStartTime = g_tr069uldStartTime;
		tr069updinf.tr069uldCompleteTime = g_tr069uldCompleteTime;

		FILE* tr069uldfp = fopen(tr069_uploadlogfile, "w");
		if (tr069uldfp)
		{
			fwrite(&tr069updinf, sizeof(TR069UploadInfo), 1, tr069uldfp);
			fclose(tr069uldfp);
		}
	}
  return (int)result;
#endif
}

#endif /* ndef UNITTESTS */
