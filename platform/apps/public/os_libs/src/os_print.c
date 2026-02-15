/*  Copyright(c) 2009-2022 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 * file		os_print.c
 * brief		
 * details	
 *
 * author	Huang Qingjia
 * version	
 * date		2020.04.19
 *
 * history 	\arg	
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include "os_print.h"

/**************************************************************************************************/
/*                                           DEFINES                                              */
/**************************************************************************************************/

#ifndef gettid
#define gettid() syscall(__NR_gettid)
#endif /* !gettid */

#define OS_PRINT_BUFF_LEN	(1024)
#define OS_PRINT_CMD_LEN	(512)
#define OS_PRINT_PATH_LEN	(512)

#define OS_PRINT_DIR "/var/tmp/os_print"

#define OS_PRINT_SWITCH_DETAIL	"d"

#define OS_PRINT_INIT_DIR_MAX_TIME (300) /* sec */

#ifndef TRUE
#define TRUE	(1)
#endif

#ifndef FALSE
#define FALSE	(0)
#endif

/**************************************************************************************************/
/*                                           TYPES                                                */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                           EXTERN_PROTOTYPES                                    */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                           LOCAL_PROTOTYPES                                     */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                           VARIABLES                                            */
/**************************************************************************************************/

static const char * l_fileForLevel[OS_PRINT_LEVEL_NUM] =
{
	[OS_PRINT_LEVEL_ERROR] = "0",
	[OS_PRINT_LEVEL_WARN]  = "1",
	[OS_PRINT_LEVEL_DEBUG] = "2",
	[OS_PRINT_LEVEL_TRACE] = "3",
};

static const char * l_headForLevel[OS_PRINT_LEVEL_NUM + 1] =
{
	[OS_PRINT_LEVEL_FORCE + 1]  = ": ",
	[OS_PRINT_LEVEL_ERROR + 1]  = " - " CLRr "ERROR" CLRnorm ": ",
	[OS_PRINT_LEVEL_WARN  + 1]  = " - " CLRy "WARN" CLRnorm ": ",
	[OS_PRINT_LEVEL_DEBUG + 1]  = " - DEBUG: ",
	[OS_PRINT_LEVEL_TRACE + 1]  = " - TRACE: ",
};

/**************************************************************************************************/
/*                                           LOCAL_FUNCTIONS                                      */
/**************************************************************************************************/
static int getTime(unsigned long *pSec, unsigned long *pMsec)
{
	struct timespec ts;
	int ret = 0;

	memset(&ts, 0, sizeof(ts));

	ret = clock_gettime(CLOCK_MONOTONIC, &ts);
	if (ret < 0)
	{
		printf("get time fail (%d %d)\n", ret, __LINE__);
		return -1;
	}
	
	if (pSec)
	{
		*pSec = (unsigned long)ts.tv_sec;
	}
	
	if (pMsec)
	{
		*pMsec  = (unsigned long)(ts.tv_nsec/1000000); /* ms */
	}
	
	return 0;
}
static int addDir(const char *pPath)
{
	char cmdbuf[OS_PRINT_CMD_LEN] = {0};
	int ret = 0;

	ret = snprintf(cmdbuf,
		sizeof(cmdbuf),
		"mkdir -p %s",
		pPath);
	
	if (ret < 0)
	{
		printf("addDir fail %d %d\n", ret, __LINE__);
		return -1;
	}
	
	ret = system(cmdbuf);
	
	if (0 != ret)
	{
		printf("addDir fail %d %d\n", ret, __LINE__);
		return -1;
	}
	
	return 0;
}

static int addFile(const char *pPath)
{
	char cmdbuf[OS_PRINT_CMD_LEN] = {0};
	int ret = 0;

	ret = snprintf(cmdbuf,
		sizeof(cmdbuf),
		"touch %s",
		pPath);
	
	if (ret < 0)
	{
		printf("addFile fail %d %d\n", ret, __LINE__);
		return -1;
	}

	ret = system(cmdbuf);
	
	if (0 != ret)
	{
		printf("addFile fail %d %d\n", ret, __LINE__);
		return -1;
	}

	return 0;
}

static int delFile(const char *pPath)
{
	char cmdbuf[OS_PRINT_CMD_LEN] = {0};
	int ret = 0;

	ret = snprintf(cmdbuf,
		sizeof(cmdbuf),
		"rm -f %s",
		pPath);
	
	if (ret < 0)
	{
		printf("delFile fail %d %d\n", ret, __LINE__);
		return -1;
	}
	
	ret = system(cmdbuf);
	
	if (0 != ret)
	{
		printf("delFile %d %d\n", ret, __LINE__);
		return -1;
	}

	return 0;
}

static int checkFile(const char *pPath)
{
	if (0 == access(pPath, 0))
	{
		return 0;
	}

	return -1;
}

static int getModPath(const char *pModule, const char *pFile, char *pPath, size_t len)
{
	int ret = 0;

	ret = snprintf(pPath,
		len,
		"%s/%s/%s",
		OS_PRINT_DIR,
		pModule?pModule:"all",
		pFile?pFile:"");
	
	if (ret < 0)
	{
		printf("getModulePath fail %d %d\n", ret, __LINE__);
		return -1;
	}

	return 0;
}

static int getPidPath(const char *pModule, const char *pFile, char *pPath, size_t len)
{
	int ret = 0;

	ret = snprintf(pPath,
		len,
		"%s/%s/pid_%ld/%s",
		OS_PRINT_DIR,
		pModule?pModule:"all",
		(long)getpid(),
		pFile?pFile:"");
	
	if (ret < 0)
	{
		printf("getPidPath fail %d %d\n", ret, __LINE__);
		return -1;
	}

	return 0;
}

static int getTidPath(const char *pModule, const char *pFile, char *pPath, size_t len)
{
	int ret = 0;

	ret = snprintf(pPath,
		len,
		"%s/%s/pid_%ld/tid_%ld/%s",
		OS_PRINT_DIR,
		pModule?pModule:"all",
		(long)getpid(),
		(long)gettid(),
		pFile?pFile:"");
	
	if (ret < 0)
	{
		printf("getTidPath fail %d %d\n", ret, __LINE__);
		return -1;
	}

	return 0;
}

static int checkPaths(const char *pModule, const char *pFile)
{
	char pathBuf[OS_PRINT_PATH_LEN] = {0};
	int ret = 0;

	/************* check if module is enabled ******************/
	
	ret = getModPath(NULL, pFile, pathBuf, sizeof(pathBuf));
	
	if (0 != ret)
	{
		printf("print check fail %d %d\n", ret, __LINE__);
		return -1;
	}
	
	/* check /var/tmp/os_print/all/[file] */
	ret = checkFile(pathBuf);
	
	if (0 == ret)
	{
		return 0;
	}
	
	ret = getModPath(pModule, pFile, pathBuf, sizeof(pathBuf));
	
	if (0 != ret)
	{
		printf("print check fail %d %d\n", ret, __LINE__);
		return -1;
	}
	
	/* check /var/tmp/os_print/[module]/[file] */
	ret = checkFile(pathBuf);
	
	if (0 == ret)
	{
		return 0;
	}
	
	/************* check if process is enabled ******************/
	
	ret = getPidPath(NULL, pFile, pathBuf, sizeof(pathBuf));
	
	if (0 != ret)
	{
		printf("print check fail %d %d\n", ret, __LINE__);
		return -1;
	}
	
	/* check /var/tmp/os_print/all/pid_xxx/[file] */
	ret = checkFile(pathBuf);
	
	if (0 == ret)
	{
		return 0;
	}
	
	ret = getPidPath(pModule, pFile, pathBuf, sizeof(pathBuf));
	
	if (0 != ret)
	{
		printf("print check fail %d %d\n", ret, __LINE__);
		return -1;
	}
	
	/* check /var/tmp/os_print/[module]/pid_xxx/[file] */
	ret = checkFile(pathBuf);
	
	if (0 == ret)
	{
		return 0;
	}
	
	/************* check if thread is enabled ******************/
	
	ret = getTidPath(NULL, pFile, pathBuf, sizeof(pathBuf));
	
	if (0 != ret)
	{
		printf("print check fail %d %d\n", ret, __LINE__);
		return -1;
	}
	
	/* check /var/tmp/os_print/all/pid_xxx/tid_xxx/[file] */
	ret = checkFile(pathBuf);
	
	if (0 == ret)
	{
		return 0;
	}
	
	ret = getTidPath(pModule, pFile, pathBuf, sizeof(pathBuf));
	
	if (0 != ret)
	{
		printf("print check fail %d %d\n", ret, __LINE__);
		return -1;
	}
	
	/* check /var/tmp/os_print/[module]/pid_xxx/tid_xxx/[file] */
	ret = checkFile(pathBuf);
	
	if (0 == ret)
	{
		return 0;
	}

	return -1;
}

/* 
 * fn		static int initPrint(const char *pModule)
 * brief	init environment for os print
 * details	
 *
 * param[in]  pModule, module name
 * param[out]	
 *
 * return	0
 * retval	
 *
 * note
 */
static int initPrint(const char *pModule)
{
	unsigned long sec = 0;
	char pathBuf[OS_PRINT_PATH_LEN] = {0};
	int ret = 0;

	ret = getTime(&sec, NULL);
	
	if (ret < 0)
	{
		printf("get time fail (%d %d)\n", ret, __LINE__);
		return -1;
	}

	if (sec > OS_PRINT_INIT_DIR_MAX_TIME)
	{
		return 0;
	}
	
	/*********** init module dir ***************/
	ret = getModPath(NULL, NULL, pathBuf, sizeof(pathBuf));
	
	if (ret < 0)
	{
		printf("print init fail %d %d\n", ret, __LINE__);
		return -1;
	}

	ret = checkFile(pathBuf);

	if (0 != ret)
	{
		ret = addDir(pathBuf);

		if (ret < 0)
		{
			printf("print init fail %d %d\n", ret, __LINE__);
			return -1;
		}
	}
	
	ret = getModPath(pModule, NULL, pathBuf, sizeof(pathBuf));
	
	if (ret < 0)
	{
		printf("print init fail %d %d\n", ret, __LINE__);
		return -1;
	}

	ret = checkFile(pathBuf);

	if (0 != ret)
	{
		ret = addDir(pathBuf);

		if (ret < 0)
		{
			printf("print init fail %d %d\n", ret, __LINE__);
			return -1;
		}
	}

	/*********** init process dir ***************/
	
	ret = getPidPath(NULL, NULL, pathBuf, sizeof(pathBuf));
	
	if (ret < 0)
	{
		printf("print init fail %d %d\n", ret, __LINE__);
		return -1;
	}

	ret = checkFile(pathBuf);

	if (0 != ret)
	{
		ret = addDir(pathBuf);

		if (ret < 0)
		{
			printf("print init fail %d %d\n", ret, __LINE__);
			return -1;
		}
	}
	
	ret = getPidPath(pModule, NULL, pathBuf, sizeof(pathBuf));
	
	if (ret < 0)
	{
		printf("print init fail %d %d\n", ret, __LINE__);
		return -1;
	}

	ret = checkFile(pathBuf);

	if (0 != ret)
	{
		ret = addDir(pathBuf);

		if (ret < 0)
		{
			printf("print init fail %d %d\n", ret, __LINE__);
			return -1;
		}
	}

	/*********** init thread dir ***************/
	
	ret = getTidPath(NULL, NULL, pathBuf, sizeof(pathBuf));
	
	if (ret < 0)
	{
		printf("print init fail %d %d\n", ret, __LINE__);
		return -1;
	}

	ret = checkFile(pathBuf);

	if (0 != ret)
	{
		ret = addDir(pathBuf);

		if (ret < 0)
		{
			printf("print init fail %d %d\n", ret, __LINE__);
			return -1;
		}
	}
	
	ret = getTidPath(pModule, NULL, pathBuf, sizeof(pathBuf));
	
	if (ret < 0)
	{
		printf("print init fail %d %d\n", ret, __LINE__);
		return -1;
	}

	ret = checkFile(pathBuf);

	if (0 != ret)
	{
		ret = addDir(pathBuf);

		if (ret < 0)
		{
			printf("print init fail %d %d\n", ret, __LINE__);
			return -1;
		}
	}

	return 0;
}

static int isPrintEnabled(int level, const char *pModule)
{
	int index = 0;
	int ret = 0;

	if (OS_PRINT_LEVEL_FORCE == level ||
		OS_PRINT_LEVEL_ERROR == level)
	{
		return TRUE;
	}
	
	for (index = level; index < OS_PRINT_LEVEL_NUM; index++)
	{
		ret = checkPaths(pModule, l_fileForLevel[index]);
		
		if (0 == ret)
		{
			return TRUE;
		}
	}

	return FALSE;
}

static int isDetailEnabled(int level, const char *pModule)
{
#ifdef INCLUDE_DEBUG_DETAIL_FORCED
	return TRUE;
#else /* !INCLUDE_DEBUG_DETAIL_FORCED */
	int ret = 0;
	int index = 0;
	
	if (level >= OS_PRINT_LEVEL_DEBUG)
	{
		return TRUE;
	}

	ret = checkPaths(pModule, OS_PRINT_SWITCH_DETAIL);
	
	if (0 == ret)
	{
		return TRUE;
	}

	for (index = OS_PRINT_LEVEL_DEBUG; index < OS_PRINT_LEVEL_NUM; index++)
	{
		ret = checkPaths(pModule, l_fileForLevel[index]);
		
		if (0 == ret)
		{
			return TRUE;
		}
	}

	return FALSE;
#endif /* INCLUDE_DEBUG_DETAIL_FORCED */
}

static int fillPrintHead(char *pStrBuf,
								size_t bufLen,
								int level,
								const char *pModule,
								const char *pFile,
								const char *pFunc,
								int line)
{
	unsigned long sec = 0, msec = 0;
	const char *pFileNoDir = NULL;
	int len = 0;
	int ret = 0;

	ret = getTime(&sec, &msec);
	
	if (ret < 0)
	{
		printf("get time fail (%d %d)\n", ret, __LINE__);
		return -1;
	}

	pFileNoDir = strrchr(pFile, '/');

	if (isDetailEnabled(level, pModule))
	{
		ret = snprintf(pStrBuf + len, bufLen - len, 
			"[%lu.%03lu][pid_%lu][tid_%lu][%s][%s][%s] %d", 
			sec,
			msec,
			(long)getpid(),
			(long)gettid(),
			pModule,
			pFileNoDir?pFileNoDir+1:pFile,
			pFunc, 
			line);
	}
	else
	{
		ret = snprintf(pStrBuf + len, bufLen - len, 
			"[%lu.%03lu][%s] %d", 
			sec,
			msec,
			pModule,
			line);
	}
	
	if (ret < 0)
	{
		printf("print fail (%d %d)\n", ret, __LINE__);
		return -1;
	}

	len += ret;

	if (len >= bufLen)
	{
		printf("fill head out of range (%d>=%d). skip messge from [ %s ] %d\n",
			len, bufLen, pFunc, line);
		return -1;
	}

	ret = snprintf(pStrBuf + len, bufLen - len, "%s",
		l_headForLevel[level+1]);

	if (ret < 0)
	{
		printf("print fail (%d %d).\n", ret, __LINE__);
		return -1;
	}
	
	len += ret;

	return len;
}

static int fillPrintTail(char *pStrBuf, size_t bufLen)
{
	int len = 0;
	int ret = 0;

	ret = snprintf(pStrBuf + len, bufLen - len, "\n");
	
	if (ret < 0)
	{
		printf("fill tail fail (%d %d). \n", ret, __LINE__);
		return -1;
	}
	
	len += ret;

	return len;
}

/**************************************************************************************************/
/*                                           PUBLIC_FUNCTIONS                                     */
/**************************************************************************************************/

/* 
 * fn		int os_print_write(int level,
						int headtail,
						const char *pModule,						
						const char *pFile,
						const char *pFunc,
						int line,
						const char *pFormat,
						...)
 * brief	print message from module
 * details	
 *
 * param[in]  level, 0:error, 1:warn, 2:debug, 3:trace
 * param[in]  headtail, add heading and tailling return.
 * param[in]  pModule, module name
 * param[in]  pFunc, caller function name
 * param[in]  line, caller function line number
 * param[in]  pFormat, format string for printf
 * param[out]
 *
 * return	-1:error, or print number of string
 * retval	
 *
 * note
 */
int os_print_write(int level,
						int headtail,
						const char *pModule,						
						const char *pFile,
						const char *pFunc,
						int line,
						const char *pFormat,
						...)
{
	char pStrBuf[OS_PRINT_BUFF_LEN] = {0};
	va_list args;

	int len = 0;
	int ret = 0;

	if (NULL == pModule)
	{
		printf("pModule is null. skip messge.\n");
		return -1;
	}

	if (NULL == pFile)
	{
		printf("pFile is null. skip messge.\n");
		return -1;
	}

	if (NULL == pFunc)
	{
		printf("pFunc is null. skip messge.\n");
		return -1;
	}

	if (level < OS_PRINT_LEVEL_FORCE || level >= OS_PRINT_LEVEL_NUM)
	{
		printf("level(%d) error. skip message.\n", level);
		return -1;
	}
	
	ret = initPrint(pModule);

	if (ret < 0)
	{
		printf("print init fail (%d). skip messge from [ %s ] %d\n",
			ret, pFile, line);
		return -1;
	}

	if (!isPrintEnabled(level, pModule))
	{
		/* printf("skip level %d message\n", level);*/
		return 0;
	}

	if (headtail)
	{
		ret = fillPrintHead(pStrBuf + len,
			OS_PRINT_BUFF_LEN - len,
			level,
			pModule,
			pFile,
			pFunc,
			line);
			
		if (ret < 0)
		{
			printf("print head fail (%d). skip messge from [ %s ] %d\n",
				ret, pFile, line);
			return -1;
		}
		
		len += ret;

		if (len >= OS_PRINT_BUFF_LEN)
		{
			printf("print head out of range (%d>=%d). skip messge from [ %s ] %d\n",
				len, OS_PRINT_BUFF_LEN, pFile, line);
			return -1;
		}
	}
	
	va_start(args, pFormat);
	ret = vsnprintf(pStrBuf + len, OS_PRINT_BUFF_LEN - len, pFormat, args);
	va_end(args);

	if (ret < 0)
	{
		printf("print message fail (%d). skip messge from [ %s ] %d\n",
			ret, pFile, line);
		return -1;
	}

	len += ret;

	if (len >= OS_PRINT_BUFF_LEN)
	{
		printf("print message out of range (%d>=%d). skip messge from [ %s ] %d\n",
			len, OS_PRINT_BUFF_LEN, pFile, line);
		return -1;
	}

	if (headtail)
	{
		ret = fillPrintTail(pStrBuf + len, OS_PRINT_BUFF_LEN - len);
		if (ret < 0)
		{
			printf("print tail fail (%d). skip messge from [ %s ] %d\n",
				ret, pFunc, line);
			return -1;
		}
		len += ret;

		if (len >= OS_PRINT_BUFF_LEN)
		{
			printf("print tail out of range (%d>=%d). skip messge from [ %s ] %d\n",
				len, OS_PRINT_BUFF_LEN, pFile, line);
			return -1;
		}
	}

	if (OS_PRINT_LEVEL_ERROR == level)
	{
		if (fwrite(pStrBuf, 1, (size_t)len, stderr) != (size_t)len)
		{
			printf("print write fail (%d). skip messge from [ %s ] %d\n",
				__LINE__, pFile, line);
			return -1;
		}
		fflush(stderr);
	}
	else
	{
		if (fwrite(pStrBuf, 1, (size_t)len, stdout) != (size_t)len)
		{
			printf("print write fail (%d). skip messge from [ %s ] %d\n",
				__LINE__, pFile, line);
			return -1;
		}
		fflush(stdout);
	}

	return len;
}

/* 
 * fn		int os_print_setModLevel(const char *pModule, int level)
 * brief	set print level while this module code
 * details
 *
 * param[in]  pModule, module name
 * param[in]  level, 0:error, 1:warn, 2:debug, 3:trace
 *
 * return	-1:error, 0:ok
 * retval
 *
 * note
 */
int os_print_setModLevel(const char *pModule, int level)
{
	char pathBuf[OS_PRINT_PATH_LEN] = {0};
	int index = 0;
	int ret = 0;

	if (level < OS_PRINT_LEVEL_ERROR || level >= OS_PRINT_LEVEL_NUM)
	{
		printf("level(%d) error.\n", level);
		return -1;
	}

	ret = initPrint(pModule);

	if (0 != ret)
	{
		printf("print init fail %d %d\n", ret, __LINE__);
		return -1;
	}

	/* remove all debug level files */
	for (index = OS_PRINT_LEVEL_ERROR; index < OS_PRINT_LEVEL_NUM; index++)
	{
		ret = getModPath(pModule, l_fileForLevel[index], pathBuf, sizeof(pathBuf));

		if (ret < 0)
		{
			printf("getModulePath fail %d %d\n", ret, __LINE__);
			return -1;
		}
		
		ret = delFile(pathBuf);
		
		if (0 != ret)
		{
			printf("delFile fail %d %d\n", ret, __LINE__);
			return -1;
		}
	}

	ret = getModPath(pModule, l_fileForLevel[level], pathBuf, sizeof(pathBuf));
	
	if (ret < 0)
	{
		printf("getModulePath fail %d %d\n", ret, __LINE__);
		return -1;
	}

	/* add the debug level */
	ret = addFile(pathBuf);
	
	if (0 != ret)
	{
		printf("addFile fail %d %d\n", ret, __LINE__);
		return -1;
	}

	return 0;
}


/* 
 * fn		int os_print_setPidLevel(const char *pModule, int level)
 * brief	set print level of this process in the module code
 * details
 *
 * param[in]  pModule, module name
 * param[in]  level, 0:error, 1:warn, 2:debug, 3:trace
 *
 * return	-1:error, 0:ok
 * retval
 *
 * note
 */
int os_print_setPidLevel(const char *pModule, int level)
{
	char pathBuf[OS_PRINT_PATH_LEN] = {0};
	int index = 0;
	int ret = 0;

	if (level < OS_PRINT_LEVEL_ERROR || level >= OS_PRINT_LEVEL_NUM)
	{
		printf("level(%d) error.\n", level);
		return -1;
	}

	ret = initPrint(pModule);

	if (0 != ret)
	{
		printf("print init fail %d %d\n", ret, __LINE__);
		return -1;
	}

	/* remove all debug level files */
	for (index = OS_PRINT_LEVEL_ERROR; index < OS_PRINT_LEVEL_NUM; index++)
	{
		ret = getPidPath(pModule, l_fileForLevel[index], pathBuf, sizeof(pathBuf));

		if (ret < 0)
		{
			printf("getModulePath fail %d %d\n", ret, __LINE__);
			return -1;
		}
		
		ret = delFile(pathBuf);
		
		if (0 != ret)
		{
			printf("delFile fail %d %d\n", ret, __LINE__);
			return -1;
		}
	}

	ret = getPidPath(pModule, l_fileForLevel[level], pathBuf, sizeof(pathBuf));
	
	if (ret < 0)
	{
		printf("getModulePath fail %d %d\n", ret, __LINE__);
		return -1;
	}

	/* add the debug level */
	ret = addFile(pathBuf);
	
	if (0 != ret)
	{
		printf("addFile fail %d %d\n", ret, __LINE__);
		return -1;
	}

	return 0;
}


/* 
 * fn		int os_print_setTidLevel(const char *pModule, int level)
 * brief	set print level of this thread in the module code
 * details
 *
 * param[in]  pModule, module name
 * param[in]  level, 0:error, 1:warn, 2:debug, 3:trace
 *
 * return	-1:error, 0:ok
 * retval
 *
 * note
 */
int os_print_setTidLevel(const char *pModule, int level)
{
	char pathBuf[OS_PRINT_PATH_LEN] = {0};
	int index = 0;
	int ret = 0;

	if (level < OS_PRINT_LEVEL_ERROR || level >= OS_PRINT_LEVEL_NUM)
	{
		printf("level(%d) error.\n", level);
		return -1;
	}

	ret = initPrint(pModule);

	if (0 != ret)
	{
		printf("print init fail %d %d\n", ret, __LINE__);
		return -1;
	}

	/* remove all debug level files */
	for (index = OS_PRINT_LEVEL_ERROR; index < OS_PRINT_LEVEL_NUM; index++)
	{
		ret = getTidPath(pModule, l_fileForLevel[index], pathBuf, sizeof(pathBuf));

		if (ret < 0)
		{
			printf("getModulePath fail %d %d\n", ret, __LINE__);
			return -1;
		}
		
		ret = delFile(pathBuf);
		
		if (0 != ret)
		{
			printf("delFile fail %d %d\n", ret, __LINE__);
			return -1;
		}
	}

	ret = getTidPath(pModule, l_fileForLevel[level], pathBuf, sizeof(pathBuf));
	
	if (ret < 0)
	{
		printf("getModulePath fail %d %d\n", ret, __LINE__);
		return -1;
	}

	/* add the debug level */
	ret = addFile(pathBuf);
	
	if (0 != ret)
	{
		printf("addFile fail %d %d\n", ret, __LINE__);
		return -1;
	}

	return 0;
}

/**************************************************************************************************/
/*                                           GLOBAL_FUNCTIONS                                     */
/**************************************************************************************************/

