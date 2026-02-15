/*  Copyright(c) 2010-2011 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 * file	os_msgLinux.c
 * brief	Linux general IPC method. 
 *
 * author	Yang Xv
 * version	1.0.0
 * date	28Apr11
 *
 * history 	\arg 1.0.0, 28Apr11, Yang Xv, Create the file.
 */
#ifdef __LINUX_OS_FC__

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <os_lib.h>
#include <os_msg.h>
#include <os_log.h>


/**************************************************************************************************/
/*                                           DEFINES                                              */
/**************************************************************************************************/

/* 
 * brief Different OS have different path prefix
 */
//#define OS_PATH_PREFIX "/var/run/"
#define OS_PATH_PREFIX "/var/tmp/"



/* 
 * brief For VOIP	
 */
#ifdef INCLUDE_VOIP
#define CM_IS_STARTING         "/var/tmp/voip_starting"
#endif /* INCLUDE_VOIP */

/**************************************************************************************************/
/*                                           TYPES                                                */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                           LOCAL_PROTOTYPES                                     */
/**************************************************************************************************/
#ifdef INCLUDE_TEST_STRACE_CMM_MSG
static int msg_getInodeFromUnixSocketPath(const char *path, unsigned long *inode);
static int msg_getPidAndProgNameFromUnixSocketPath(const char *path, int *pid, char *name);
static int msg_getProgNameFromPid(const int pid, char *name, int size);
static int msg_getStracePidWithAttachPid(const int target_pid, int *strace_pid);
#endif /* INCLUDE_TEST_STRACE_CMM_MSG */

/**************************************************************************************************/
/*                                           LOCAL_FUNCTIONS                                      */
/**************************************************************************************************/
#ifdef INCLUDE_TEST_STRACE_CMM_MSG
static int msg_getInodeFromUnixSocketPath(const char *path, unsigned long *inode)
{
	FILE *fp = NULL;
	char line[256] = {0};
	char rdpath[64] = {0};
	char *ptr = NULL;
	unsigned long rdinode = 0;

	if (NULL == path || NULL == inode)
	{
		printf("%s[%d]: Error: invalid paramter\n",
			__FUNCTION__, __LINE__);
		return -1;
	}

	if (NULL == (fp = fopen("/proc/net/unix", "r")))
	{
		printf("%s[%d]: Error: can't open /proc/net/unix to read\n",
			__FUNCTION__, __LINE__);
		return -1;
	}

	/* every single line in /proc/net/unix will less than 128 chs */
	while(NULL != fgets(line, 256, fp))
	{
		/* remove '\n' */
		line[strlen(line) - 1] = '\0';
		if (NULL != (ptr = strstr(line, path)))
		{
			memset(rdpath, 0, 64);
			/*format: Num: RefCount Protocol Flags Type St Inode Path */
			if (sscanf(line, "%*p: %*X %*X %*X %*X %*X %lu %s", &rdinode, rdpath) < 2)
			{
				printf("%s[%d]: Error: can't read inode in line [%s]\n",
					__FUNCTION__, __LINE__, line);
				fclose(fp);
				return -1;
			}
			if (0 == strcmp(path, rdpath) && 0 != rdinode)
			{
				*inode = rdinode;
				fclose(fp);
				return 0;
			}
		}
	}

	fclose(fp);
	return -1;
}

static int msg_getPidAndProgNameFromUnixSocketPath(const char *path, int *pid, char *name)
{
	DIR *dir = NULL;
	DIR *fddir = NULL;
	struct dirent *next = NULL;
	struct dirent *fdnext = NULL;
	char dirname[128] = {0};
	char buf[256] = {0};
	char rdname[64] = {0};
	char *ptr = NULL;
	unsigned long inode = 0;
	int tmppid = -1;

	if (NULL == path || NULL == pid || NULL == name)
	{
		printf("%s[%d]: Error: invalid paramter\n",
			__FUNCTION__, __LINE__);
		return -1;
	}

	/* get inode from /proc/net/unix with unix socket path */
	if (0 != msg_getInodeFromUnixSocketPath(path, &inode))
	{
		printf("%s[%d]: Error: can't find inode with socket path [%s]\n",
			__FUNCTION__, __LINE__, path);
		return -1;
	}

	/* iterate pid in /proc */
	sprintf(dirname, "/proc");
	if (NULL == (dir = opendir(dirname)))
	{
		printf("%s[%d]: Error: can't open dir[%s]\n",
			__FUNCTION__, __LINE__, dirname);
		return -1;
	}

	while (NULL != (next = readdir(dir)))
	{
		/* skip /proc entries which aren't processes */
		if (isdigit(next->d_name[0]))
		{
			tmppid = strtoul(next->d_name, NULL, 10);
			if (0 != msg_getProgNameFromPid(tmppid, rdname, sizeof(rdname) - 1))
			{
				continue;
			}

			/* iterate fd in /proc/pid */
			sprintf(dirname, "/proc/%d/fd", tmppid);
			if (NULL != (fddir = opendir(dirname)))
			{
				while (NULL != (fdnext = readdir(fddir)))
				{
					memset(buf, 0, 256);
					sprintf(dirname, "/proc/%d/fd/%s", tmppid, fdnext->d_name);
					readlink(dirname, buf, 256);
					/* format: socket:[inode] or [0000]:inode */
					if (NULL != strstr(buf, "socket:"))
					{
						ptr = buf + strlen("socket:[");
						ptr[strlen(ptr) - 1] = '\0';
					}
					else if (NULL != strstr(buf, "[0000]:"))
					{
						ptr = buf + strlen("[0000]:");
					}
					else
					{
						ptr = NULL;
					}
					if (NULL != ptr && inode == strtoul(ptr, NULL, 10))
					{
						*pid = tmppid;
						strncpy(name, rdname, 64 - 1);
						closedir(fddir);
						closedir(dir);
						return 0;
					}
				}
				closedir(fddir);
			}
		}
	}

	closedir(dir);

	return -1;
}

static int msg_getProgNameFromPid(const int pid, char *name, int size)
{
	FILE *fp = NULL;
	char filename[128] = {0};
	char buf[256] = {0};

	if (pid <= 0 || NULL == name || size <= 0)
	{
		printf("%s[%d]: Error: invalid paramter\n",
			__FUNCTION__, __LINE__);
		return -1;
	}

	sprintf(filename, "/proc/%d/cmdline", pid);
	if (NULL == (fp = fopen(filename, "r")))
	{
		printf("%s[%d]: Error: can't open file[%s]\n",
			__FUNCTION__, __LINE__, filename);
		return -1;
	}

	/* cmdline format: program\0paramter1\0paramter2\0... */
	memset(buf, 0, 256);
	fgets(buf, 256, fp);
	fclose(fp);

	if (0 == strlen(buf))
	{
		return -1;
	}

	/* save program name */
	strncpy(name, buf, size);

	return 0;
}

static int msg_getStracePidWithAttachPid(const int target_pid, int *strace_pid)
{
	DIR *dir = NULL;
	struct dirent *next = NULL;
	FILE *fp = NULL;
	char dirname[128] = {0};
	char filename[128] = {0};
	char buf[256] = {0};
	char rdname[64] = {0};
	char *ptr = NULL;
	int tmppid = -1;

	if (target_pid <= 0 || NULL == strace_pid)
	{
		printf("%s[%d]: Error: invalid paramter\n",
			__FUNCTION__, __LINE__);
		return -1;
	}

	/* iterate pid in /proc */
	sprintf(dirname, "/proc");
	if (NULL == (dir = opendir(dirname)))
	{
		printf("%s[%d]: Error: can't open dir[%s]\n",
			__FUNCTION__, __LINE__, dirname);
		return -1;
	}

	while (NULL != (next = readdir(dir)))
	{
		/* skip /proc entries which aren't processes */
		if (isdigit(next->d_name[0]))
		{
			tmppid = strtoul(next->d_name, NULL, 10);
			sprintf(filename, "/proc/%d/cmdline", tmppid);
			if (NULL == (fp = fopen(filename, "r")))
			{
				printf("%s[%d]: Error: can't open file[%s]\n",
					__FUNCTION__, __LINE__, filename);
				closedir(dir);
				return -1;
			}
			/* cmdline format: program\0paramter1\0paramter2\0... */
			memset(buf, 0, 256);
			fgets(buf, 256, fp);
			fclose(fp);

			if (0 == strlen(buf))
			{
				continue;
			}
			else if(0 == strcmp(buf, "strace"))
			{
				/* format: strace -p pid & */
				ptr = buf + strlen("strace") + 1;
				if (NULL != ptr && 0 == strcmp(ptr, "-p"))
				{
					ptr = ptr + strlen("-p") + 1;
					if ('&' == ptr[strlen(ptr) - 1])
					{
						ptr[strlen(ptr) - 1] = '\0';
					}
					if (NULL != ptr && target_pid == strtoul(ptr, NULL, 10))
					{
						*strace_pid = tmppid;
						closedir(dir);
						return 0;
					}
				}
			}
		}
	}

	closedir(dir);

	return -1;

}
#endif /* INCLUDE_TEST_STRACE_CMM_MSG */

/**************************************************************************************************/
/*                                           GLOBAL_FUNCTIONS                                     */
/**************************************************************************************************/

/* 
 * fn		int msg_init(CMSG_FD *pMsgFd)
 * brief	Create an endpoint for msg
 *	
 * param[out]	pMsgFd - return msg descriptor that has been create	
 *
 * return	-1 is returned if an error occurs, otherwise is 0
 *
 * note 	Need call msg_cleanup() when you no longer use this msg which is created by msg_init()
 */
int msg_init(CMSG_FD *pMsgFd)
{
	assert(pMsgFd != NULL);

	if (-1 == (pMsgFd->fd = socket(AF_LOCAL, SOCK_DGRAM, 0))) 	
	{		
		perror("socket");
		printf("line(%d) pid(%d) errno(%d)\n", __LINE__, getpid(), errno);
		return -1;	
	}
	
	/* set FD_CLOEXEC */
	if (-1 == fcntl(pMsgFd->fd, F_SETFD, 1))
	{
		perror("msg fcntl");
		printf("line(%d) pid(%d) errno(%d)\n", __LINE__, getpid(), errno);
		close(pMsgFd->fd);
		return -1;
	}
	
	return 0;
}


/* 
 * fn		int msg_srvInit(CMSG_ID msgId, CMSG_FD *pMsgFd)
 * brief	Init an endpoint as a server and bind a name to this endpoint msg	
 *
 * param[in]	msgId - server name	
 * param[in]	pMsgFd - server endpoint msg fd
 *
 * return	-1 is returned if an error occurs, otherwise is 0	
 */
int msg_srvInit(CMSG_ID msgId, CMSG_FD *pMsgFd)
/* 本来应该是传入一个name，之后拼接成path，但是如果由开发者传入name，因为随意命名，可能会出现冲突
 * 因此这里直接传入ID，用ID做文件名
 */
{
	char path[MAX_PATH_LEN + 1] = {0};

	assert(pMsgFd != NULL);

	/* prepare domain path */
	snprintf(path, MAX_PATH_LEN, "%s%d", OS_PATH_PREFIX, msgId);
	DEBUG_PRINT("Server path is %s\n", path);
	unlink(path);
	
	/* bind socket to specify path */
	memset(&(pMsgFd->_localAddr), 0, sizeof(pMsgFd->_localAddr));
	pMsgFd->_localAddr.sun_family = AF_LOCAL;	
	strncpy(pMsgFd->_localAddr.sun_path, path, sizeof(pMsgFd->_localAddr.sun_path));	

	if (-1 == bind(pMsgFd->fd, (struct sockaddr *)(&(pMsgFd->_localAddr)), 
		SUN_LEN(&(pMsgFd->_localAddr))))
	{		
		perror("bind");
		printf("line(%d) pid(%d) errno(%d)\n", __LINE__, getpid(), errno);
		return -1;	
	}
	
	return 0;
}



/* 
 * fn		int msg_connSrv(CMSG_ID msgId, CMSG_FD *pMsgFd)
 * brief	Init an endpoint as a client and specify a server name	
 *
 * param[in]		msgId - server name that we want to connect	
 * param[in/out]	pMsgFd - client endpoint msg fd	
 *
 * return	-1 is returned if an error occurs, otherwise is 0
 */
int msg_connSrv(CMSG_ID msgId, CMSG_FD *pMsgFd)
{
	char path[MAX_PATH_LEN + 1] = {0};

	assert(pMsgFd != NULL);

	/* prepare domain path */
	snprintf(path, MAX_PATH_LEN, "%s%d", OS_PATH_PREFIX, msgId);
	DEBUG_PRINT("Client connect server path is %s\n", path);
	
	/* bind socket to specify path */
	memset(&(pMsgFd->_remoteAddr), 0, sizeof(pMsgFd->_remoteAddr));
	pMsgFd->_remoteAddr.sun_family = AF_LOCAL;	
	strncpy(pMsgFd->_remoteAddr.sun_path, path, sizeof(pMsgFd->_remoteAddr.sun_path));

	return 0;
}


/* 
 * fn		int msg_recv(const CMSG_FD *pMsgFd, CMSG_BUFF *pMsgBuff)
 * brief	Receive a message form a msg	
 *
 * param[in]	pMsgFd - msg fd that we want to receive message
 * param[out]	pMsgBuff - return recived message
 *
 * return	-1 is returned if an error occurs, otherwise is 0
 *
 * note		we will clear msg buffer before recv
 */
int msg_recv(const CMSG_FD *pMsgFd, CMSG_BUFF *pMsgBuff)
/* 这里使用_remoteAddr是为了后续可能使用msg_send的情况(msg_sendAndGetReplyWithTimeout()) */
{
	int len = sizeof(pMsgFd->_remoteAddr);
	int ret = 0;
	
	assert((pMsgFd != NULL) && (pMsgBuff != NULL));

	memset(pMsgBuff, 0, sizeof(CMSG_BUFF));

	/*
	* 
	* 使用于"慢系统调用"的基本规则:当阻塞于某个慢系统调用的进程捕获某个信号且相应信号
	* 处理函数返回时。该系统调用可能返回一个EINTR错误。有些内核自动重启某些被中断的系统调用，
	* 但是更加保险的方式是当我们遇到这种错误时，手动重启该函数。
	*  recvfrom属于慢系统调用。目前看来某些system shell调用，不会导致其调用失败并返回错误码EINTR.
	* 
	*/
	do {
		ret = recvfrom(pMsgFd->fd, pMsgBuff, sizeof(CMSG_BUFF), 0, (struct sockaddr *)(&(pMsgFd->_remoteAddr)), (socklen_t *)&len);
		if (ret <= 0 && (ENOENT == errno || EINTR == errno))
		{
			perror("recvfrom");
			printf("line(%d) Msg.type(%d) pid(%d) errno(%d) ret(%d)\n", __LINE__, pMsgBuff->type, getpid(), errno, ret);
			usleep(10000);
		}
	} while (ret <= 0 && (EINTR == errno || EAGAIN == errno));

	if (ret <= 0)
	{
		perror("recvfrom");
		printf("line(%d) Msg.type(%d) pid(%d) errno(%d) ret(%d)\n", __LINE__, pMsgBuff->type, getpid(), errno, ret);
		MSG_DEBUG_START_STRACE(pMsgFd);
		return -1;
	}

	return 0;
}

#ifdef INCLUDE_MSG_DEBUG
#define MSG_DEBUG_FILE "/var/run/debug_msg.txt"
#define MSG_DEBUG_FILE_MAX_SIZE (32*1024)
#endif
/* 
 * fn		int msg_send(const CMSG_FD *pMsgFd, const CMSG_BUFF *pMsgBuff)
 * brief	Send a message from a msg	
 *
 * param[in]	pMsgFd - msg fd that we want to send message	
 * param[in]	pMsgBuff - msg that we wnat to send
 *
 * return	-1 is returned if an error occurs, otherwise is 0
 *
 * note 	This function will while call sendto() if sendto() return ENOENT error
 */
int msg_send(const CMSG_FD *pMsgFd, const CMSG_BUFF *pMsgBuff)
/* 如果sendto()返回的是ENOENT，那么我们将继续调用sendto，直到成功或者返回其他错误为止
 * 主要目的是用来处理服务器还未启动，但是客户端已经启动的情况(主要是系统初始化的时候)
 * 这里有一种可能，就是这个域套接字的路径是存在的，但是服务器已经关闭，暂时不处理
 */
{
	int ret = 0;
	int flag = 0;
	char httpd_path[8];
	int is_http;
	int is_tmp;
	char *p_slash;
	char *sockname;
	
	assert((pMsgFd != NULL) && (pMsgBuff != NULL));
#ifdef INCLUDE_MSG_DEBUG
	if(pMsgBuff->type != CMSG_CWMP_TIMER)
	{
		//fd: descriptor; pid: the process id which send msg;remoteaddr:/var/tmp/%d, %d is value of 'CMSG_ID'; type: msg type of 'CMSG_TYPE'
		os_writeDebugFile(MSG_DEBUG_FILE, MSG_DEBUG_FILE_MAX_SIZE, 
		"msg_send fd:%d, pid:%d, remoteaddr:%s, type:%d\n", pMsgFd->fd, getpid(), pMsgFd->_remoteAddr.sun_path, pMsgBuff->type);
	}
	
#endif

	sockname = (char *)&pMsgFd->_remoteAddr.sun_path;
	do
	{
		p_slash = strchr(sockname, '/');
		if (p_slash != NULL)
		{
			sockname = ++p_slash;
		}
	} while (p_slash != NULL);
	snprintf(httpd_path, sizeof(httpd_path), "%d", CMSG_ID_HTTP);
	is_http = (strcmp(httpd_path, sockname) == 0) ? 1 : 0;
	/* someone has called msg_sendAndGetReplyWithTimeout and exit after timeout,the socket has been deleted */
	is_tmp = (strncmp("tmp_", sockname, 4) == 0) ? 1 : 0;
	
#ifdef INCLUDE_VOIP	
	char cmd[MAX_PATH_LEN];
	/* 
	 * brief	NOTE !! : These messages will lead to the restart of CM, so CMM MUST remember it
	 *					and MUST NOT try to get the CM status if any flash writing is to be issued.
	 *					Otherwise, CMM will lock the DM, send request to CM and wait reply from CM.
	 *					unfortunately, the CM is starting and it want to access the DM but the DM
	 *					is locked, so CMM can not get the replay from CM and it will delete the
	 *					socket and return after timeout. After the DM is unlocked, CM will enter
	 *					into dead loop to send replay to CMM because of the socket error.
	 */
	if (pMsgBuff->type >= CMSG_VOIP_WAN_STS_CHANGED && pMsgBuff->type <= CMSG_VOIP_RESTART_CALLMGR)
	{
		sprintf(cmd, "echo 1 > %s", CM_IS_STARTING);
		do
		{
			system(cmd);
			usleep(1 * 1000);
		} while (access(CM_IS_STARTING, F_OK) == -1);
	}
#endif /* INCLUDE_VOIP */
	
	/* Don't wait log message when queue is full 
	 * added by yangxv,2012.9.29
	 */
	if (pMsgBuff->type == CMSG_LOG)
	{
		flag = MSG_DONTWAIT;
	}

	do
	{
		if (pMsgBuff->type == CMSG_LOG)
		{
			ret = sendto(pMsgFd->fd, pMsgBuff, LOG_MSG_SIZE, flag, 
						(struct sockaddr *)(&(pMsgFd->_remoteAddr)), SUN_LEN(&(pMsgFd->_remoteAddr)));
		}
		else
		{
			ret = sendto(pMsgFd->fd, pMsgBuff, sizeof(CMSG_BUFF), flag, 
				(struct sockaddr *)(&(pMsgFd->_remoteAddr)), SUN_LEN(&(pMsgFd->_remoteAddr)));
		}
		
		/* consider msg_sendAndGetReplyWithTimeout(),no need always send for CMSG_REPLY */
		if (pMsgBuff->type == CMSG_REPLY)
		{
			break;
		}

		if ((ret <= 0) && 0 == is_tmp && (ENOENT == errno || EINTR == errno ||
							(ECONNREFUSED == errno && is_http)))
		{
			perror("sendto");
			printf("line(%d) Msg.type(%d) pid(%d) errno(%d) ret(%d)\n", __LINE__, pMsgBuff->type, getpid(), errno, ret);
			usleep(10000);
		}
	}while((ret <= 0) && 0 == is_tmp && (ENOENT == errno || EINTR == errno || EAGAIN == errno ||
							(ECONNREFUSED == errno && is_http)));

	if (ret <= 0)
	{
#ifdef INCLUDE_VOIP
		if (pMsgBuff->type >= CMSG_VOIP_WAN_STS_CHANGED && pMsgBuff->type <= CMSG_VOIP_RESTART_CALLMGR)
		{
			sprintf(cmd, "rm -f %s", CM_IS_STARTING);
			do
			{
				system(cmd);
				usleep(1 * 1000);
			} while(access(CM_IS_STARTING, F_OK) == 0);
		}
#endif /* INCLUDE_VOIP */	
		perror("sendto");
		printf("line(%d) Msg.type(%d) pid(%d) errno(%d) ret(%d)\n", __LINE__, pMsgBuff->type, getpid(), errno, ret);
		MSG_DEBUG_START_STRACE(pMsgFd);
		return -1;
	}
#ifdef INCLUDE_VOIP	
	if (pMsgBuff->type >= CMSG_VOIP_WAN_STS_CHANGED && pMsgBuff->type <= CMSG_VOIP_RESTART_CALLMGR)
	{
		/* 
		 * brief	let CM delete the temporary file in time, so CMM can send msg to CM as normal
		 */
		usleep(500 * 1000);
	}
#endif /* INCLUDE_VOIP */	
	
	return 0;
}


/* 
 * fn		int msg_cleanup(CMSG_FD *pMsgFd)
 * brief	Close a message fd
 * details	
 *
 * param[in]	pMsgFd - message fd that we want to close		
 *
 * return	-1 is returned if an error occurs, otherwise is 0		
 */
int msg_cleanup(CMSG_FD *pMsgFd)
{
	assert(pMsgFd != NULL);

	if (-1 == close(pMsgFd->fd))
	{
		printf("line(%d) pid(%d) errno(%d)\n", __LINE__, getpid(), errno);
		perror("close");		
		return -1;		
	}

	/* TODO:
	 * 这里可以考虑增加删除域套接字的路径，不过由于服务器都是启动之后就不再退出，
	 * 因此暂时可以不考虑
	 */

	return 0;
}


/* 
 * fn		int msg_connCliAndSend(CMSG_ID msgId, CMSG_FD *pMsgFd, CMSG_BUFF *pMsgBuff)
 * brief	init a client msg and send msg to server which is specified by msgId	
 *
 * param[in]	msgId -	server ID that we want to send
 * param[in]	pMsgFd - message fd that we want to send
 * param[in]	pMsgBuff - msg that we wnat to send
 *
 * return	-1 is returned if an error occurs, otherwise is 0	
 */
int msg_connCliAndSend(CMSG_ID msgId, CMSG_FD *pMsgFd, CMSG_BUFF *pMsgBuff)
/* 将向server发送消息的四步操作封装起来，方便使用 */
{
	assert((pMsgFd != NULL) && (pMsgBuff != NULL));
	
	if (msg_init(pMsgFd) < 0)
	{
		printf("line(%d) Init %d msg client error\n", __LINE__, msgId);
		return -1;
	}
	
	if (msg_connSrv(msgId, pMsgFd) < 0)
	{
		printf("line(%d) Connect %d msg client error\n", __LINE__, msgId);
		msg_cleanup(pMsgFd);
		
		return -1;
	}

	msg_send(pMsgFd, pMsgBuff);

	msg_cleanup(pMsgFd);

	return 0;
}


/* 
 * fn		int msg_sendAndGetReply(CMSG_FD *pMsgFd, CMSG_BUFF *pMsgBuff)
 * brief	
 *
 * param[in]	pMsgFd - msg fd that we want to use
 * param[in/out]pMsgBuff - send msg and get reply
 * param[in]	timeSeconds - timeout in second
 *
 * return	-1 is returned if an error occurs, otherwise is 0	
 */
int msg_sendAndGetReplyWithTimeout(CMSG_FD *pMsgFd, CMSG_BUFF *pMsgBuff, int timeSeconds)
{
	int fd = 0;
	int ret = 0;
	struct sockaddr_un tmpAddr;
	char tempFile[64];
	struct timeval tv;
	int errHappen = 0;
	time_t start, end;

	assert((pMsgFd != NULL) && (pMsgBuff != NULL));

	memset(tempFile, 0, 64);
	sprintf(tempFile, "/var/tmp_%d_%d_XXXXXX", getpid(), pMsgBuff->type);

	if (-1 == (fd = mkstemp(tempFile)))
	{
		printf("line(%d) tempFile(%s)\n", __LINE__, tempFile);
		perror("mkstemp");
		return -1;
	}
	
	memset(&tmpAddr, 0, sizeof(tmpAddr));
	tmpAddr.sun_family = AF_LOCAL;
	strncpy(tmpAddr.sun_path, tempFile, sizeof(tmpAddr.sun_path));
	unlink(tmpAddr.sun_path);

	/* bind socket to a temp path for recive */
	if (-1 == bind(pMsgFd->fd, (struct sockaddr *)&tmpAddr, SUN_LEN(&tmpAddr))) 	
	{		
		perror("bind");		
		printf("line(%d) pid(%d) errno(%d)\n", __LINE__, getpid(), errno);
		return -1;	
	}

	do
	{
		ret = sendto(pMsgFd->fd, pMsgBuff, sizeof(CMSG_BUFF), 0, 
				(struct sockaddr *)(&(pMsgFd->_remoteAddr)), SUN_LEN(&(pMsgFd->_remoteAddr)));

		if ((ret <= 0) && (ENOENT == errno || EINTR == errno || EAGAIN == errno))
		{
			perror("sendto");
			printf("line(%d) Msg.type(%d) pid(%d) errno(%d) ret(%d) tempFile(%s)\n", 
				__LINE__, pMsgBuff->type, getpid(), errno, ret, tempFile);
			usleep(10000);
		}
	}while((ret <= 0) && (ENOENT == errno || EINTR == errno || EAGAIN == errno));

	if (ret <= 0)
	{
		perror("sendto");
		printf("line(%d) Msg.type(%d) pid(%d) errno(%d) ret(%d) tempFile(%s)\n", 
			__LINE__, pMsgBuff->type, getpid(), errno, ret, tempFile);
		errHappen = 1;
		goto error;		
	}

	/* set recvfrom timeout */
	tv.tv_sec = timeSeconds;
	tv.tv_usec = 0;
	
	if (-1 == setsockopt(pMsgFd->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)))
	{
		perror("setsockopt");
		errHappen = 1;
		goto error;
	}

	/*
	* 
	* 使用于"慢系统调用"的基本规则:当阻塞于某个慢系统调用的进程捕获某个信号且相应信号
	* 处理函数返回时。该系统调用可能返回一个EINTR错误。有些内核自动重启某些被中断的系统调用，
	* 但是更加保险的方式是当我们遇到这种错误时，手动重启该函数。
	*  recvfrom属于慢系统调用。目前看来某些system shell调用，不会导致其调用失败并返回错误码EINTR.
	* 
	*/
	/*
	if (os_threadCreate("test", 100, 16*1024, (FUNCPTR)system_call, NULL, &pid))
	{
		perror("os_threadCreate");
	}
	*/

	time(&start);
	do {
		ret = recvfrom(pMsgFd->fd, pMsgBuff, sizeof(CMSG_BUFF), 0, NULL, NULL);
		if (ret <= 0)
		{
			if (ENOENT == errno || EINTR ==errno || EAGAIN == errno)
			{
				perror("recvfrom");
				printf("line(%d) Msg.type(%d) pid(%d) errno(%d) ret(%d) tempFile(%s)\n", 
					__LINE__, pMsgBuff->type, getpid(), errno, ret, tempFile);

			}
			/*
			*	Because calling setsockopt to set the timeout for recvfrom, 
			*	recvfrom returns - 1 after timeout, and errno is EAGAIN, 
			*	so we need to set the timeout judgment ourselves
			*/
			usleep(10000);
			if (EAGAIN == errno)
			{
				time(&end);
				if ((end - start) >= timeSeconds)
				{
					printf("line(%d) Msg.type(%d) pid(%d) errno(%d) ret(%d) tempFile(%s) total time = %ld, timeSeconds = %d\n", 
						__LINE__, pMsgBuff->type, getpid(), errno, ret, tempFile, (end - start), timeSeconds);
					break;
				}
			}			

		}
	} while((ret <= 0) && (EINTR == errno || EAGAIN == errno));

	if (ret <= 0)
	{
		perror("recvfrom");
		printf("line(%d) Msg.type(%d) pid(%d) errno(%d) ret(%d) tempFile(%s)\n", 
			__LINE__, pMsgBuff->type, getpid(), errno, ret, tempFile);
		errHappen = 1;
		goto error;
	}

error:

	/* set recvfrom no timeout */
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	
	if (-1 == setsockopt(pMsgFd->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)))
	{
		perror("setsockopt");
	}

	/* delete file after get reply */
	close(fd);
	unlink(tmpAddr.sun_path);

	/* rebind method not found,so close and create socket again, ugly I know... */
	close(pMsgFd->fd);

	if (-1 == (pMsgFd->fd = socket(AF_LOCAL, SOCK_DGRAM, 0))) 	
	{		
		perror("new socket");		
		return -1;	
	}

	if (1 == errHappen)
	{
		MSG_DEBUG_START_STRACE(pMsgFd);
		return -1;
	}
	else
	{
		return 0;
	}
}

/* 
 * fn		int msg_reply_send(const CMSG_FD *pMsgFd, const CMSG_BUFF *pMsgBuff)
 * brief	Send a reply(2011) from a msg	
 *
 * param[in]	pMsgFd - msg fd that we want to send message	
 * param[in]	pMsgBuff - msg that we wnat to send
 *
 * return	-1 is returned if an error occurs, otherwise is 0
 *
 * note     This function is copied from msg_send()
 *          This function is only used for msg_sendAndGetReplyWithTimeout reply(2011)
 *          and remove the voip
 */
int msg_reply_send(const CMSG_FD *pMsgFd, const CMSG_BUFF *pMsgBuff)
{
	int ret = 0;
	int flag = 0;
	struct sockaddr_un	peer_sock;
	int len;
	int temp_err;
	
	assert((pMsgFd != NULL) && (pMsgBuff != NULL));

	/* Don't wait log message when queue is full 
	 * added by yangxv,2012.9.29
	 */
	if (pMsgBuff->type == CMSG_LOG)
	{
		flag = MSG_DONTWAIT;
	}
	
	do
	{
		if (pMsgBuff->type == CMSG_LOG)
		{
			ret = sendto(pMsgFd->fd, pMsgBuff, LOG_MSG_SIZE, flag, 
						(struct sockaddr *)(&(pMsgFd->_remoteAddr)), SUN_LEN(&(pMsgFd->_remoteAddr)));
		}
		else
		{
			ret = sendto(pMsgFd->fd, pMsgBuff, sizeof(CMSG_BUFF), flag, 
					(struct sockaddr *)(&(pMsgFd->_remoteAddr)), SUN_LEN(&(pMsgFd->_remoteAddr)));
		}

		if ((ret <= 0) && (ENOENT == errno || EINTR == errno))
		{
			perror("sendto");
			printf("line(%d) Msg.type(%d) pid(%d) errno(%d) ret(%d) tempFile(%s)\n", 
				__LINE__, pMsgBuff->type, getpid(), errno, ret, pMsgFd->_remoteAddr.sun_path);
			usleep(10000);
		}
		/*
		*	Compared with the send function, we cancel the loop when errno is equal to ENOENT,
		*	Because of some unknown reasons, the domain socket file is deleted peer end, 
		*	and it will be in a dead loop
		*	so sendto() will not be sent successfully
		*/
	}while((ret <= 0) && (EINTR == errno || EAGAIN == errno));
	
	temp_err = errno;

	len = sizeof(peer_sock);
	if (getsockname(pMsgFd->fd, (struct sockaddr *) &peer_sock, &len) == -1)
	{
		printf("line(%d) fd=%d\n", __LINE__, pMsgFd->fd);
		perror("getsockname");
	}

	if (ret <= 0)
	{
		perror("sendto");
		printf("line(%d) Msg.type(%d) pid(%d) errno(%d) ret(%d) tempFile(%s), peer_sock_file(%s)\n", 
			__LINE__, pMsgBuff->type, getpid(), temp_err, ret, pMsgFd->_remoteAddr.sun_path, peer_sock.sun_path);
		MSG_DEBUG_START_STRACE(pMsgFd);
		return -1;
	}
	
	return 0;
}

#ifdef INCLUDE_PON
/* 
 * fn		int msg_send_no_loop(const CMSG_FD *pMsgFd, const CMSG_BUFF *pMsgBuff)
 * brief	Send a message from a msg	
 *
 * param[in]	pMsgFd - msg fd that we want to send message	
 * param[in]	pMsgBuff - msg that we wnat to send
 *
 * return	-1 is returned if an error occurs, otherwise is 0
 *
 * note 	Added by YeZuopou @ 19Nov13.
 */
int msg_send_no_loop(const CMSG_FD *pMsgFd, const CMSG_BUFF *pMsgBuff)
{
	int ret = 0;
	int flag = 0;

    int count = 100;
	
	assert((pMsgFd != NULL) && (pMsgBuff != NULL));

	do
	{
		ret = sendto(pMsgFd->fd, pMsgBuff, sizeof(CMSG_BUFF), flag, 
				(struct sockaddr *)(&(pMsgFd->_remoteAddr)), SUN_LEN(&(pMsgFd->_remoteAddr)));

		if ((-1 == ret) && (ENOENT == errno) && (1 == count))
		{
			usleep(10000);
			perror("sendto");
			printf("pid %d send %d error\n", getpid(), pMsgBuff->type);
		}
		
	}while((-1 == ret) && (ENOENT == errno || EINTR == errno) && (--count));

	if (-1 == ret)
	{
		perror("sendto");
		printf("pid %d send %d error, errno is %d\n", getpid(), pMsgBuff->type, errno);
		MSG_DEBUG_START_STRACE(pMsgFd);
		return -1;
	}
	
	return 0;
}
#endif /* INCLUDE_PON */

#ifdef INCLUDE_TEST_STRACE_CMM_MSG
/*
 * fn		int msg_startStrace(const CMSG_FD *pMsgFd)
 * brief	start strace to monitor program with specific CMSG_FD
 *
 * param[in]	pMsgFd - msg fd that we want to monitor
 *
 * return	-1 is returned if an error occurs, otherwise is 0
 */
int msg_startStrace(const CMSG_FD *pMsgFd)
{
	int self_pid = -1;
	int target_pid = -1;
	char self_name[64] = {0};
	char target_name[64] = {0};
	char cmd[64] = {0};

	if (NULL == pMsgFd)
	{
		printf("%s[%d]: Error: pMsgFd is null pointer\n",
			__FUNCTION__, __LINE__);
		return -1;
	}

	/* Get target pid and program name */
	if (0 != msg_getPidAndProgNameFromUnixSocketPath(pMsgFd->_remoteAddr.sun_path, &target_pid, target_name))
	{
		printf("%s[%d]: Error: can't find target pid and program name from path %s\n",
			__FUNCTION__, __LINE__, pMsgFd->_remoteAddr.sun_path);
		return -1;
	}

	if (-1 == target_pid || 0 == strlen(target_name))
	{
		printf("%s[%d]: Error: invalid target pid[%d] or program name[%s]\n",
			__FUNCTION__, __LINE__, target_pid, target_name);
		return -1;
	}

	/* Get self pid and program name */
	self_pid = getpid();
	if (0 != msg_getProgNameFromPid(self_pid, self_name, sizeof(self_name) - 1))
	{
		printf("%s[%d]: Error: can't find self program name with pid[%d]\n",
			__FUNCTION__, __LINE__, self_pid);
		return -1;
	}

	sprintf(cmd, "strace -p %d &", target_pid);
	printf("%s[%d]: Start strace for %s[%d] while %s[%d] can't communicate with it\n", 
		__FUNCTION__, __LINE__, target_name, target_pid, self_name, self_pid);

	/* start strace:
	 * only one strace for the specific pid will be activated.
	 */
	system(cmd);

	return 0;
}

/*
 * fn		int msg_stopStrace(const CMSG_FD *pMsgFd)
 * brief	stop monitor program with specific CMSG_FD
 *
 * param[in]	pMsgFd - msg fd that we want to monitor
 *
 * return	-1 is returned if an error occurs, otherwise is 0
 */
int msg_stopStrace(const CMSG_FD *pMsgFd)
{
	int self_pid = -1;
	int target_pid = -1;
	int strace_pid = -1;
	char self_name[64] = {0};
	char target_name[64] = {0};
	char cmd[64] = {0};

	if (NULL == pMsgFd)
	{
		printf("%s[%d]: Error: pMsgFd is null pointer\n",
			__FUNCTION__, __LINE__);
		return -1;
	}

	/* Get target pid and program name */
	if (0 != msg_getPidAndProgNameFromUnixSocketPath(pMsgFd->_remoteAddr.sun_path, &target_pid, target_name))
	{
		printf("%s[%d]: Error: can't find target pid and program name from path %s\n",
			__FUNCTION__, __LINE__, pMsgFd->_remoteAddr.sun_path);
		return -1;
	}

	if (-1 == target_pid || 0 == strlen(target_name))
	{
		printf("%s[%d]: Error: invalid target pid[%d] or program name[%s]\n",
			__FUNCTION__, __LINE__, target_pid, target_name);
		return -1;
	}

	/* Get self pid and program name */
	self_pid = getpid();
	if (0 != msg_getProgNameFromPid(self_pid, self_name, sizeof(self_name) - 1))
	{
		printf("%s[%d]: Error: can't find self program name with pid[%d]\n",
			__FUNCTION__, __LINE__, self_pid);
		return -1;
	}

	if (0 != msg_getStracePidWithAttachPid(target_pid, &strace_pid))
	{
#if 0
		printf("%s[%d]: Error: can't find strace with pid[%d]\n",
			__FUNCTION__, __LINE__, target_pid);
#endif
		return -1;
	}

	sprintf(cmd, "kill -9 %d", strace_pid);
	printf("%s[%d]: Stop strace for %s[%d] because communication with %s[%d] is alive\n", 
		__FUNCTION__, __LINE__, target_name, target_pid, self_name, self_pid);

	system(cmd);

	return 0;
}
#endif /* INCLUDE_TEST_STRACE_CMM_MSG */


#endif /* __LINUX_OS_FC__ */
