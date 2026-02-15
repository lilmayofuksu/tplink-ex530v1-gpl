/*  Copyright(c) 2009-2011 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 * file		os_linux.c
 * brief		
 * details	
 *
 * author	Wu Zhiqin
 * version	
 * date		04May11
 *
 *
 * history \arg	
 */

#if defined(__LINUX_OS_FC__)

#include <string.h>
#include <stdlib.h>
#ifdef INCLUDE_DEBUG_LOCK
#include <sys/stat.h>
#include <sys/time.h>
#include <stdarg.h>
#endif
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/sysinfo.h>
#include <sys/prctl.h>
#include <semaphore.h>
#include <stdio.h> 
#include <time.h> 
#include <assert.h>
#if defined(INCLUDE_DECT)
#include <linux/sched.h>
#include <sys/syscall.h>
#endif

#include <os_lib.h>
#include <limits.h>

/**************************************************************************************************/
/*                                      DEFINES                                                   */
/**************************************************************************************************/
#define MAX_TIMER_NUM	(32)
#define MAX_GET_SEM_COUNT	(30)
#define MAX_SYSTEM_UP_TIME	(200)

#define MAX_TIMER_NUM	(32)
#define OS_BUFLEN_32	(32)
#define OS_BUFLEN_256	(256)
#define OS_BUFLEN_32	(32)


/* 
 * brief ARP proc path	
 */
#define _ARP_TABLE_NAME		"/proc/net/arp"


/**************************************************************************************************/
/*                                      TYPES                                                     */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                      EXTERN_PROTOTYPES                                         */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                      LOCAL_PROTOTYPES                                          */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                      VARIABLES                                                 */
/**************************************************************************************************/

/* 
 * brief for ftok id.
 */
static int queues = 0;

/* 
 * brief	for timer
 */
timer_t timers[MAX_TIMER_NUM] = {0};


OS_THREAD_POOL_ATTR l_defaultThreadPoolAttr = 
{
	.stackSize = 1024 *1024,
	.highPrio = PTHREAD_PRIO_HIGH,
	.lowPrio = PTHREAD_PRIO_LOW,
	.highPrioThreadCount = 1
};

const char *l_anoymousWorkName = "Anonymous";

/**************************************************************************************************/
/*                                      LOCAL_FUNCTIONS                                           */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                      PUBLIC_FUNCTIONS                                          */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                      GLOBAL_FUNCTIONS                                          */
/**************************************************************************************************/

/* 
 * fn		int os_semCreate(int initialCount, OS_SEM_OPTIONS opt, OS_SEM *pSem)
 * brief	create semaphore.
 * details	opt is for vxWorks OS. opt value may be SEM_Q_FIFO_K or SEM_Q_PRIORITY_K
 *
 * param [in]	initialCount - initial value for semaphore.
 * param [in]	opt	- SEM_Q_FIFO_K or SEM_Q_PRIORITY_K, used in vxWorks OS.
 * param[out]	pSem - to keep the semaphore ID.
 *
 * return	0 if success, -1 otherwise.	
 */
int os_semCreate(int initialCount, OS_SEM_OPTIONS opt, OS_SEM *pSem)
{
    int ret;

	assert(pSem != NULL);
	
    ret = sem_init(pSem, 0, initialCount);
    if (ret != 0)
    {
        switch(errno)
        {
            case EINVAL:
                printf("Semaphore initialization failed: value exceeds SEM_VALUE_MAX.");
                break;
            case ENOSYS:
                printf("Semaphore initialization failed: pshared is non-zero, but the system "
							"does  not  support  process shared semaphores (see sem_overview.)");
                break;
            default:
                printf("Semaphore initialization failed: unknown fault.");
                break;
        }
        return -1;
    }

    return 0;
}



/* 
 * fn		int os_semDestroy  (OS_SEM *pSem)
 * brief	delete semaphore.
 * details	
 *
 * param [in]	sem - semaphore ID to delete
 * param[out]	
 *
 * return	0 if success, -1 otherwise.		
 */
int os_semDestroy  (OS_SEM *pSem)
{
    if (0 != sem_destroy(pSem))
    {
        printf("Semaphore destroy failed.");
        return -1;
    }

    return 0;
}



/* 
 * fn		int os_semTake(OS_SEM *pSem, unsigned int timeout)
 * brief	take a semaphore
 * details	This routine performs the take operation on a specified semaphore. For vxWorks OS, 
 *			A timeout in ticks may be specified. If a task times out, os_semTake( ) will return 
 *			ERROR. Timeouts of WAIT_FOREVER (-1) and NO_WAIT (0) indicate to wait indefinitely or 
 *			not to wait at all. 
 *
 * param [in]	sem - semaphore ID to take 
 * param [in]	timeout - for vxWorks, timeout in ticks 
 * param[out]	
 *
 * return	0 if success, -1 otherwise.	
 */
int os_semTake(OS_SEM *pSem, unsigned int timeout)
{
    if (0 != sem_wait(pSem))
    {
		perror("sem_wait");
        printf("Semaphore wait failed.");
        return -1;
    }

    return 0;
}



/* 
 * fn		int os_semGive(OS_SEM *pSem)
 * brief	give a semaphore
 * details	This routine performs the give operation on a specified semaphore.
 *
 * param [in]	sem - semaphore ID to give 
 * param[out]	
 *
 * return	0 if success, -1 otherwise.		
 */
int os_semGive(OS_SEM *pSem)
{
    if (0 != sem_post(pSem))
    {
		perror("sem_post");
        printf("Semaphore post failed.");
        return -1;
    }

    return 0;
}

/* 
 * fn		int os_semGetValue(OS_SEM *pSem, int * val)
 * brief	get value of a semaphore
 * details	This routine performs the value get on a specified semaphore.
 *
 * param [in]	sem - semaphore ID to operate 
 * param[out]	
 *
 * return	0 if success, -1 otherwise.		
 */
int os_semGetValue(OS_SEM *pSem, int * val)
{
    if (0 != sem_getvalue(pSem, val))
    {
		perror("sem_getvalue");
        printf("Semaphore get value failed.\n");
        return -1;
    }

    return 0;
}

/* 
 * fn		    int os_semVCreate(unsigned int key, int initialValue, OS_V_SEM *pSem) 
 * brief	    Create a new system v semaphore.for process synchronous.
 * details	    
 *
 * param [in]	key			 - a none zero(IPC_PRIVATE) key is associated with a unique 
 *							   semaphore set.
 * param [in]	initialValue - initial value of the system v semaphore. can be 0 or 1
 * param[out]	pSem		 - to keep the system v semaphore ID.
 *
 * return	    0 if success, -1 if failed
 * retval	    
 *
 * note		  	In Linux, this routine does not create a systen v semaphore set, it only create
 *				a system v semaphore.  
 */
int os_semVCreate(unsigned int key, int initialValue, OS_V_SEM *pSem)
{
	int flags;

	assert((pSem != NULL) && (initialValue > -1 && initialValue < 2));

	flags = IPC_CREAT | 0666;

	if ((*pSem = semget((key_t)key, 1, flags)) == -1)
	{	
		printf("semget failed, errno=%d", errno);

		return -1;
	}

	/*
	 * We are creating new semaphore, so initialize semaphore to 1
	 */
	if(semctl(*pSem, 0, SETVAL, initialValue) == -1)
	{
		printf("setctl setval 1 failed, errno=%d", errno);

		return -1;
	}

	return 0;
}



/* 
 * fn		    int os_semVGet(unsigned int key, OS_V_SEM *pSem) 
 * brief	    Get a existed system v semaphore referenced by key.
 * details	    
 *
 * param [in]	key	 - a none zero(IPC_PRIVATE) key is associated with a unique 
 *					   semaphore set.
 * param[out]	pSem - to keep the system v semaphore ID.
 *
 * return	    0 if success, -1 if failed	    
 */
int os_semVGet(unsigned int key, OS_V_SEM *pSem)
{
	int flags = IPC_CREAT | 0666;
	
	if (key == 0)
	{
		printf("use key IPC_PRIVATE(0) will aways create a new v semaphore.");
		return -1;
	}

	assert(pSem != NULL);

	/* NOTE: int semget(key_t key, int nsems, int semflg);
	**       param [in] nsems - 0 : get a exist sem; other : set num of sem will be created.
	*/
	if ((*pSem = semget((key_t)key, 0, flags)) == -1)
	{
		DEBUG_PRINT("v semaphore with key(%x) is not exitst, please create the semaphore.", key);
		return -1;
	}

	return 0;
}


/*
 * fn           OS_V_SEM os_semVGetSafety(int callerLine, const unsigned int key, OS_V_SEM *pSemIdShm)
 *
 * brief        Get an existing System V semaphore referenced by key.
 *
 * details      This function attempts to retrieve an existing System V semaphore
 *              associated with the provided key. If the system uptime exceeds
 *              a predefined maximum value, the function returns the provided 
 *              semId directly. Otherwise, it retries to get the semaphore up 
 *              to a maximum number of attempts if the semaphore is not found 
 *              initially. If successful within the allowed attempts, it returns
 *              the valid semId; otherwise, it returns -1.
 *
 * param [in]   callerLine  - the caller function line 
 *
 * param [in]   key  - a non-zero (not IPC_PRIVATE) key associated with a unique semaphore set.
 *
 * param [in]   pSemIdShm - the shared memory semID pointer.
 *
 * return       semId(>=0) if success, -1 if failed
 */
OS_V_SEM os_semVGetSafety(int callerLine, const unsigned int key, OS_V_SEM *pSemIdShm)
{
	unsigned int count = 0;
	OS_V_SEM semIdSys = -1;
	int pidSys = 0;
	int pidShm = 0;
	int pidSelf = 0;
	int semIdShm = 0;

#if 0 
	unsigned int upTime = 0;
#endif

	if (NULL == pSemIdShm)
	{
		printf("NULL pointer(%p), caller(%d)\n", pSemIdShm, callerLine);
		return -1;
	}

#if 0
	os_getSysUpTime(&upTime);
	if (upTime > MAX_SYSTEM_UP_TIME)
	{
		/* Only do check in boot time <  MAX_SYSTEM_UP_TIME.
		** why do we this? Because we want to reduce the risk.
		*/
		return *pSemIdShm;
	}
#endif

	for (count = 0; count < MAX_GET_SEM_COUNT; count++)
	{
		/* Another thread will init shm and create sem.
		** we should update semIdShm and semIdSys after 1 second.
		*/
		semIdShm = *pSemIdShm;
		os_semVGet(key, &semIdSys);

		if ((semIdSys < 0) || (semIdShm != semIdSys))
		{
			pidSelf = getpid();

			/* exception happened which maybe semaphore not created.
			** So here wait a while try again.
			*/
			printf("== WARNING ==: PID(%d, %d), errno(%d) sem(%x), IDshm(%d) IDSys(%d), count(%d/%d)\n",
					pidSelf, callerLine, errno, key, semIdShm, semIdSys, count, MAX_GET_SEM_COUNT);
			sleep(1);
		}
		else
		{
			/* Successfully obtained semaphore with key 
			** also semaphore ID is same as system ID.
			*/
			return semIdShm;
		}
	}

	/* To determine why semIdSys differs from semIdShm,
	** we should print caller pid, the last-used semIdShm pid, and the last-used semIdSys pid.
	*/
	pidSys = semctl(semIdSys, 0, GETPID, 0);
	pidShm = semctl(semIdShm, 0, GETPID, 0);
	printf("== ERROR ==: pidSelf(%d), sem(%x), IDshm(%d, %d), IDSys(%d, %d)\n",
			pidSelf, key, semIdShm, pidShm, semIdSys, pidSys);

	return -1;

}


/* 
 * fn		    int os_semVDestory(OS_V_SEM sem) 
 * brief	    Destory a system v semaphore.
 * details	    
 *
 * param [in]	sem - system v semaphore ID.
 * param[out]	
 *
 * return	    0 if success, -1 if failed		    
 */
int os_semVDestory(OS_V_SEM sem)
{
	if (sem == -1)
	{
		return 0;
	}

	if (semctl(sem, 0, IPC_RMID) < 0)
	{
		printf("delete v semaphore %d failed, errno=%d", sem, errno);
		return -1;
	}
	
	DEBUG_PRINT("v semaphore %d deleted.", sem);

	return 0;
}

#if defined(INCLUDE_VOIP) && defined(INCLUDE_DECT)
int os_msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg)
{
	int ret;
	do
	{
		ret = msgsnd(msqid, msgp, msgsz, msgflg);
	} while (-1 == ret && EINTR == errno);

	return ret;
}

ssize_t os_msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg)
{
	int ret;
	static int i = 0;
	do
	{
		i ++;
		ret = msgrcv(msqid, msgp, msgsz, msgtyp, msgflg);
	} while (-1 == ret && EINTR == errno && i < 99999);
	i = 0;
	return (ssize_t)ret;
}
#endif

#ifdef INCLUDE_DEBUG_LOCK

/* 
 * fn		    int os_writeDebugFile(char fileName[], int fileMaxSize, const char* fmt, ...)
 * brief	    input debug info into file.
 * details	    
 *
 * param [in]	fileName - file name to record the debug info .
 * param [in]	fileMaxSize - file's max size to record debug info, if debug info is more than fileMaxSize, the file will be cleared.
 * param [in]	fmt - the debug info you want to write to file.
 *
 * return	  -1 if fail, 0 if success  	    
 */
int os_writeDebugFile(char fileName[], int fileMaxSize, const char* fmt, ...)
{
	char info[DBG_INFO_LEN] = {0};
	int len = 0;
	va_list args;
	struct stat statbuf;
	int fileSize = 0;
	FILE* fp = NULL;
	time_t timesec;
	if(fileName == NULL || fileMaxSize == 0)
		return -1;
	if(stat(fileName, &statbuf) == 0)
	{
		fileSize =  statbuf.st_size;
	}
	if((fp = fopen(fileName, (fileSize > fileMaxSize)?"w":"a")) == NULL)
	{
		printf("%s open file %s failed\n", __FUNCTION__, fileName);
		return -1;
	}
	else
	{
		timesec = time(NULL);
		len = strftime(info, DBG_INFO_LEN, "%Y-%m-%d %X ", localtime(&timesec));
		va_start(args, fmt);
		vsnprintf(info + len, DBG_INFO_LEN - len, fmt, args);
		va_end(args);
	
		fputs(info, fp);
		fflush(fp);
		fclose(fp);
		return 0;
	}
}
#endif

/* 
 * fn		    int os_semVTake(OS_V_SEM sem) 
 * brief	    Take a system v semaphore (P operate)
 * details	    
 *
 * param [in]	sem - system v semaphore ID.
 * param[out]	
 *
 * return	    0 if success, -1 if failed		    
 */
#ifdef INCLUDE_DEBUG_LOCK
int os_semVTakeDebug(OS_V_SEM sem, const char func[])
#else
int os_semVTake(OS_V_SEM sem)
#endif
{
	struct sembuf semBuf;
	semBuf.sem_num = 0;
	semBuf.sem_op = -1;
	semBuf.sem_flg = SEM_UNDO;

	if (semop(sem, &semBuf, 1) == -1)
	{
		printf("Take semaphore failed, errno=%d", errno);
		return -1;
	}
#ifdef INCLUDE_DEBUG_LOCK
	char fileName[FILE_NAME_MAX_SIZE];
	snprintf(fileName, FILE_NAME_MAX_SIZE, "/var/run/debug_lock_%d.txt", (int)sem);
	os_writeDebugFile(fileName, LOCK_DUBUG_FILE_MAX_SIZE, "pid:%d %s(called by %s)\n", getpid(), __FUNCTION__, func);
#endif


	return 0;
}



/* 
 * fn		    int os_semVGive(OS_V_SEM sem) 
 * brief	    Give a system v semaphore (V operate)
 * details	    
 *
 * param [in]	sem - system v semaphore ID.
 * param[out]	
 *
 * return	    0 if success, -1 if failed		    
 */
 #ifdef INCLUDE_DEBUG_LOCK
 int os_semVGiveDebug(OS_V_SEM sem, const char func[]) 
 #else
 int os_semVGive(OS_V_SEM sem)
 #endif
{
	struct sembuf semBuf;
	semBuf.sem_num = 0;
	semBuf.sem_op = 1;
	semBuf.sem_flg = SEM_UNDO;

	if (semop(sem, &semBuf, 1) == -1)
	{
		printf("Give semaphore failed, errno=%d", errno);
		return -1;
	}
#ifdef INCLUDE_DEBUG_LOCK
	char fileName[FILE_NAME_MAX_SIZE];
	snprintf(fileName, FILE_NAME_MAX_SIZE, "/var/run/debug_lock_%d.txt", (int)sem);
	os_writeDebugFile(fileName, LOCK_DUBUG_FILE_MAX_SIZE, "pid:%d %s(called by %s)\n", getpid(), __FUNCTION__, func);
#endif

	return 0;
}



/* 
 * fn		int os_threadCreate(char * pName,  
 *								int priority, 
 *								int stack_size, 
 *								FUNCPTR pFunc, 
 *								void * pArg, 				
 *								OS_THREAD * pTid)
 * brief	create a Linux thread or spawn a vxWorks task
 * details	
 *
 * param [in]	pName - for vxWorks, name of new  task.
 * param [in]	priority - for vxWorks, priority of new task
 * param [in]	stackSize - for vxWorks, size (bytes) of stack needed plus name 
 * param [in]	pFunc - entry point of new task or thread.
 * param [in]	pArg - task args to pass to func 
 * param[out]	pTid - to keep the thread id or task id
 *
 * return	0 if success, -1 otherwise.	
 */
int os_threadCreate(char * pName,  
					int priority, 
					int stackSize, 
					FUNCPTR pFunc, 
					void * pArg, 				
					OS_THREAD * pTid)
{
    int ret = -1;
    pthread_attr_t thread_attr;

	assert((pFunc != NULL) && (pTid != NULL));

    //init thread attr
    if (pthread_attr_init(&thread_attr) == 0) 
    {
        DEBUG_PRINT( "pthread_attr_init success.");

        //set thread attr
        if (pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED) == 0) 
        {
            DEBUG_PRINT("pthread_attr_setdetachstate success.");

			/* set stack size */
			if (stackSize < PTHREAD_STACK_MIN)
			{
				stackSize = PTHREAD_STACK_MIN;
			}
			pthread_attr_setstacksize(&thread_attr, stackSize);
			/* set priority for special thread */
			if (priority < 100)
			{
				struct sched_param sch_param;
				pthread_attr_setinheritsched (&thread_attr, PTHREAD_EXPLICIT_SCHED);
				pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO);
				sch_param.sched_priority = priority;
				pthread_attr_setschedparam(&thread_attr, &sch_param);
			}
            //Creat thread
            if (pthread_create(pTid, &thread_attr, pFunc, pArg) == 0) 
            {
                DEBUG_PRINT("Init thread %s successful.", pName);
                ret = 0;
            } 
            else 
            {
                switch(errno) 
                {
                    case EAGAIN:
                        printf("pthread_create(%s) failed: too much thread numbers.", pName);
                        break;
                    case EINVAL:
                        printf("pthread_create(%s) failed: thread id illegality.",pName);
                        break;
                    default :
                        printf("pthread_create(%s) failed: unknown.",pName);
                        break;
                }
            }
        }
        else
        {
            printf("pthread_attr_setdetachstate Failed.");
        }

        (void)pthread_attr_destroy(&thread_attr);
    }
    else
    {
        printf("pthread_attr_init failed .");
    }

    return ret;
}


#if defined(INCLUDE_DECT)
/* 
 * fn		int os_DectthreadCreate(char * pName,  
 *								int priority, 
 *								int stack_size, 
 *								FUNCPTR pFunc, 
 *								void * pArg, 				
 *								OS_THREAD * pTid)
 * brief	create a Linux thread or spawn a vxWorks task
 * details	
 *
 * param [in]	pName - for vxWorks, name of new  task.
 * param [in]	priority - for vxWorks, priority of new task
 * param [in]	stackSize - for vxWorks, size (bytes) of stack needed plus name 
 * param [in]	pFunc - entry point of new task or thread.
 * param [in]	pArg - task args to pass to func 
 * param[out]	pTid - to keep the thread id or task id
 *
 * return	0 if success, -1 otherwise.	
 */
int os_DectthreadCreate(char * pName,  
					int priority, 
					int stackSize, 
					FUNCPTR pFunc, 
					void * pArg, 				
					OS_THREAD * pTid)
{
    int ret = -1;
	int policy = SCHED_NORMAL;
    pthread_attr_t thread_attr;
	OS_PARAMS params;

	assert((pFunc != NULL) && (pTid != NULL));

    //init thread attr
    if (pthread_attr_init(&thread_attr) == 0) 
    {
        DEBUG_PRINT( "pthread_attr_init success.");

        //set thread attr
        if ((priority & PTHREAD_JOINABLE) != 0 ||
			pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED) == 0)
        {
			priority &= ~PTHREAD_JOINABLE;
            DEBUG_PRINT("pthread_attr_setdetachstate success.");

			/* set stack size */
			if (stackSize < PTHREAD_STACK_MIN)
			{
				stackSize = PTHREAD_STACK_MIN;
			}
			pthread_attr_setstacksize(&thread_attr, stackSize);
			/* set priority for special thread */
			if ((priority & PTHREAD_SCHED_FIFO) != 0)
			{
				priority &= ~PTHREAD_SCHED_FIFO;
				policy = SCHED_FIFO;
			}
			if ((priority & PTHREAD_SCHED_RR) != 0)
			{
				priority &= ~PTHREAD_SCHED_RR;
				policy = SCHED_RR;
			}
			pthread_attr_setinheritsched (&thread_attr, PTHREAD_EXPLICIT_SCHED);
			if (policy != SCHED_NORMAL)
			{
				struct sched_param sch_param;
				if (priority < sched_get_priority_min(policy) || priority > sched_get_priority_max(policy))
				{
					priority = sched_get_priority_min(policy);
				}
				pthread_attr_setschedpolicy(&thread_attr, policy);
				sch_param.sched_priority = priority;
				pthread_attr_setschedparam(&thread_attr, &sch_param);
			}
            //Creat thread
            memset(&params, 0, sizeof(OS_PARAMS));
			if (pName != NULL)
			{
				strncpy(params.name, pName, OS_NAME_LEN - 1);
			}
			params.arg = pArg;
			params.priority = priority;
			if ( sem_init(&params.sem, 0, 0) )
			{
				printf("sem for thread %s created failed!\n", params.name);
				return -1;
			}
            if (pthread_create(pTid, &thread_attr, (void *)pFunc, (void *)&params) == 0) 
            {
				/* params is a local variable,we MUST wait until the thread does not use it any more */
		        while (sem_wait(&params.sem) == -1 && EINTR == errno);			
                DEBUG_PRINT("Init thread %s successful.", pName);
                ret = 0;
            } 
            else 
            {
                switch(errno) 
                {
                    case EAGAIN:
                        printf("pthread_create(%s) failed: too much thread numbers.", pName);
                        break;
                    case EINVAL:
                        printf("pthread_create(%s) failed: thread id illegality.",pName);
                        break;
                    default :
                        printf("pthread_create(%s) failed: unknown.",pName);
                        break;
                }
            }
        }
        else
        {
            printf("pthread_attr_setdetachstate Failed.");
        }

        (void)pthread_attr_destroy(&thread_attr);
    }
    else
    {
        printf("pthread_attr_init failed .");
    }

    return ret;
}



/* 
 * fn		void os_threadSetNamePrioFinal(OS_PARAMS *pArg)
 * brief	set name and priority for thread,and notify the caller to continue
 *
 * param[in]	pArg : inlcudes the thread name,priority or other arguments the thread required
 *
 * note		the thread MUST NOT use the argument after 100ms or this function has been called
 */
void os_threadSetNamePrioFinal(OS_PARAMS *pArg)
{
	prctl(PR_SET_NAME,	pArg->name, 0, 0, 0);
	if (PTHREAD_PRIO_OTHER == pArg->priority)
		setpriority(PRIO_PROCESS, syscall(__NR_gettid), 0);
	sem_post(&pArg->sem);
}
#endif

/* 
 * fn		int os_threadExit(OS_THREAD tid)
 * brief	exit a thread. 
 * details	in vxWorks, this routine do nothing.
 *
 * param [in]	tid - thread id or task id.
 * param[out]	
 *
 * return	0 if success, -1 otherwise.	
 */
int os_threadExit(OS_THREAD tid)
{
    pthread_exit(NULL);

    return 0;
}



/* 
 * fn		int	os_threadDelete(OS_THREAD tid)
 * brief	cancel a thread or delete a task
 * details	
 *
 * param [in]	tid - thread id to cancel or task ID of task to delete 
 * param[out]	
 *
 * return	0 if success, -1 otherwise.		
 */
int	os_threadDelete(OS_THREAD tid)
{
	if (0 != pthread_cancel (tid))
	{
		printf("thread delete failed .");
		return -1;
	}

	return 0;
}

/*
 * fn		void os_threadPoolFreeWork(OS_THREAD_POOL_WORK *work)
 * brief	free work struct
 *
 * param[in]	work	work struct to free
 *
 * return	0
 */
static int os_threadPoolFreeWork(OS_THREAD_POOL_WORK *work)
{
	if (!work)
	{
		return 0;
	}

	if (work->arg1)
	{
		free(work->arg1);
		work->arg1 = NULL;
	}
	if (work->arg2)
	{
		free(work->arg2);
		work->arg2 = NULL;
	}
#ifdef OS_THREAD_POOL_DEBUG
	if (work->name && work->name != l_anoymousWorkName)
	{
		free(work->name);
		work->name = NULL;
	}
#endif /* OS_THREAD_POOL_DEBUG */
	free(work);
	work = NULL;

	return 0;
}

/*
 * fn		void *os_threadPoolBase(void *pArg)
 * brief	Base func for pthread to get and run works
 *
 * param[in]	pArg	OS_THREAD_POOL_PTHREAD struct of this pthread
 *
 * return	N/A
 */
static void *os_threadPoolBase(void *pArg)
{
	OS_THREAD_POOL_PTHREAD *pthread = (OS_THREAD_POOL_PTHREAD *)pArg;
	OS_THREAD_POOL_HANDLE *handle = pthread->handle;
	OS_THREAD_POOL_WORK *work = NULL;
	char pthreadName[16] = {0}; /* TASK_COMM_LEN */

	snprintf(pthreadName, sizeof(pthreadName), "tp%d%c", 
			pthread->index + 1, 
			(pthread->index < handle->attr.highPrioThreadCount) ? 'h' : 'l');
	prctl(PR_SET_NAME, pthreadName);

	pthread_mutex_lock(&handle->mutex);
	while (1)
	{
		pthread->status = PTHREAD_STATUS_IDLE;

		if (pthread->highPrio)
		{
			pthread_cond_wait(&handle->highPrioCond, &handle->mutex);
		}
		else
		{
			pthread_cond_wait(&handle->lowPrioCond, &handle->mutex);
		}
#ifdef OS_THREAD_POOL_DEBUG
		fprintf(stderr, "Thread \"%s\" awake\n", pthreadName);
#endif /* OS_THREAD_POOL_DEBUG */		

		pthread->status = PTHREAD_STATUS_RUNNING;

		while (1)
		{
			work = NULL;
			if (pthread->highPrio)
			{
				if (handle->highPrioQueue)
				{
					work = handle->highPrioQueue;
					handle->highPrioQueue = work->next;
				}
			}
			else
			{
				/* high prio queue delayed, low prio thread do high prio works instead */
				if (handle->highPrioQueue && handle->highPrioQueue->next)
				{
					work = handle->highPrioQueue;
					handle->highPrioQueue = work->next;
				}
				else if (handle->lowPrioQueue)
				{
					work = handle->lowPrioQueue;
					handle->lowPrioQueue = work->next;
				}
			}

			if (work)
			{
#ifdef OS_THREAD_POOL_DEBUG
				struct timespec currTime;
				float diff = 0;

				clock_gettime(CLOCK_MONOTONIC, &currTime);
				diff = (float)(currTime.tv_nsec - work->startTime.tv_nsec) / 1000000000;
				diff += (currTime.tv_sec - work->startTime.tv_sec);
				memcpy(&work->startTime, &currTime, sizeof(currTime));
#endif /* OS_THREAD_POOL_DEBUG */

				pthread->work = work;
				pthread_mutex_unlock(&handle->mutex);

#ifdef OS_THREAD_POOL_DEBUG
				fprintf(stderr, "Thread %s work \"%s\" start, wait time %.6f\n", pthreadName, work->name, diff);
#endif /* OS_THREAD_POOL_DEBUG */

				work->func(work->arg1, work->arg2);

#ifdef OS_THREAD_POOL_DEBUG
				clock_gettime(CLOCK_MONOTONIC, &currTime);
				diff = (float)(currTime.tv_nsec - work->startTime.tv_nsec) / 1000000000;
				diff += (currTime.tv_sec - work->startTime.tv_sec);
				fprintf(stderr, "Thread %s work \"%s\" end, work time %.6f\n", pthreadName, work->name, diff);
#endif /* OS_THREAD_POOL_DEBUG */

				pthread_mutex_lock(&handle->mutex);
				pthread->work = NULL;

				os_threadPoolFreeWork(work);
				work = NULL;
			}
			else
			{
				break;
			}
		}
	}

	return NULL;
}

/*
 * fn		int os_threadPoolGetDefaultAttr(OS_THREAD_POOL_ATTR *attr)
 * brief	get default attr of thread pool
 *
 * param[out]	attr	return the default attrs
 *
 * return	0
 */
int os_threadPoolGetDefaultAttr(OS_THREAD_POOL_ATTR *attr)
{
	memcpy(attr, &l_defaultThreadPoolAttr, sizeof(OS_THREAD_POOL_ATTR));
	return 0;
}

/*
 * fn		OS_THREAD_POOL_HANDLE *os_threadPoolCreate(int pthreadCount, OS_THREAD_POOL_ATTR *attr)
 * brief	Create thread pool to run works parallel
 *
 * param[in]	pthreadCount	num of pthreads in thread pool
 * param[in]	attr			thread pool attrs. NULL to use default attr
 *
 * return	OS_THREAD_POOL_HANDLE*	handle of this thread pool. used to add works to this thread pool
 * retval	NULL	Create thread pool failed
 *			Other	handle struct of this thread pool
 *
 * note	
 */
OS_THREAD_POOL_HANDLE *os_threadPoolCreate(int pthreadCount, OS_THREAD_POOL_ATTR *attr)
{
	int i = 0;
	OS_THREAD_POOL_HANDLE *handle = NULL;
	OS_THREAD_POOL_PTHREAD *pthread = NULL;

	if (pthreadCount < OS_THREAD_POOL_COUNT_MIN || pthreadCount > OS_THREAD_POOL_COUNT_MAX)
	{
		fprintf(stderr, "pthread count not available\n");
		return NULL;
	}

	handle = malloc(sizeof(OS_THREAD_POOL_HANDLE));
	if (NULL == handle)
	{
		fprintf(stderr, "malloc for OS_THREAD_POOL_HANDLE failed\n");
		return NULL;
	}
	memset(handle, 0, sizeof(OS_THREAD_POOL_HANDLE));

	if (NULL == attr)
	{
		attr = &l_defaultThreadPoolAttr;
	}
	memcpy(&handle->attr, attr, sizeof(OS_THREAD_POOL_ATTR));
	handle->_pthreadCount = pthreadCount;

	if (0 != pthread_mutex_init(&handle->mutex, NULL)
		|| 0 != pthread_cond_init(&handle->highPrioCond, NULL)
		|| 0 != pthread_cond_init(&handle->lowPrioCond, NULL))
	{
		fprintf(stderr, "mutex/cond init failed\n");
		goto err;
	}

	pthread_mutex_lock(&handle->mutex);
	for (i = 0; i < pthreadCount; i++)
	{
		pthread = malloc(sizeof(OS_THREAD_POOL_PTHREAD));
		if (NULL == pthread)
		{
			fprintf(stderr, "malloc for OS_THREAD_POOL_PTHREAD failed\n");
			goto free_pthread;
		}

		pthread->index = i;
		pthread->status = PTHREAD_STATUS_IDLE;
		pthread->highPrio = (i < attr->highPrioThreadCount) ? 1 : 0;
		pthread->handle = handle;
		pthread->work = NULL;

		pthread->next = handle->pthreadList;
		handle->pthreadList = pthread;

		os_threadCreate("tp",
						(i < attr->highPrioThreadCount) ? attr->highPrio : attr->lowPrio,
						attr->stackSize,
						os_threadPoolBase,
						(void *)pthread,
						&(pthread->tid));
	}

	pthread_mutex_unlock(&handle->mutex);
	return handle;

free_pthread:
	while (handle->pthreadList)
	{
		pthread = handle->pthreadList;
		handle->pthreadList = pthread->next;
		pthread_cancel(pthread->tid);
		free(pthread);
		pthread = NULL;
	}
	pthread_mutex_unlock(&handle->mutex);

err:
	free(handle);
	handle = NULL;
	return NULL;
}

/*
 * fn		int os_threadPoolFree(OS_THREAD_POOL_HANDLE *handle)
 * brief	Free created thread pool
 *
 * param[in]	handle	handle struct of this thread pool
 *
 * return	0
 *
 * note		Do not add any work to the thread pool freeing or freed!!!
 */
int os_threadPoolFree(OS_THREAD_POOL_HANDLE *handle)
{
	int i = 0;
	OS_THREAD_POOL_WORK *work = NULL;
	OS_THREAD_POOL_PTHREAD *pthread = NULL;

	for (i = 0; i < handle->_pthreadCount; i++)
	{
		os_threadPoolAddWork(handle, "free", (OS_THREAD_POOL_WORK_FUNC)pthread_exit, 
					(i < handle->attr.highPrioThreadCount) ? 1 : 0, 1, NULL, 0, NULL, 0);
	}

	while (handle->pthreadList)
	{
		pthread = handle->pthreadList;
		handle->pthreadList = pthread->next;

		pthread_join(pthread->tid, NULL);
		os_threadPoolFreeWork(pthread->work);

		free(pthread);
		pthread = NULL;
	}

	/* pthread exited, no need to lock */
	while (handle->highPrioQueue)
	{
		work = handle->highPrioQueue;
		handle->highPrioQueue = work->next;
		os_threadPoolFreeWork(work);
		work = NULL;
	}

	while (handle->lowPrioQueue)
	{
		work = handle->lowPrioQueue;
		handle->lowPrioQueue = work->next;
		os_threadPoolFreeWork(work);
		work = NULL;
	}

	free(handle);
	handle = NULL;

	return 0;
}

/*
 * fn		int os_threadPoolAddWork(OS_THREAD_POOL_HANDLE *handle, const char *name, OS_THREAD_POOL_WORK_FUNC func, 
 *						int highPrio, int duplicate, void *pArg1, int arg1Len, void *pArg2, int arg2Len)
 * brief	Add work to thread pool. Thread pool will run works added parallel
 *
 * param[in]	handle		handle struct of the thread pool, that work will add to
 * param[in]	name		work name, only used when DEBUG opened
 * param[in]	func		function of the work
 * param[in]	highPrio	whether high priority work
 * param[in]	duplicate	whether allow duplicated works
 * param[in]	pArg1		arg1 of the work function
 * param[in]	arg1Len		arg1 length
 * param[in]	pArg2		arg2 of the work function
 * param[in]	arg2Len		arg2 length
 *
 * return	result of add work to thread pool
 * retval	0	success
 *			-1	fail
 *
 * note	
 */
int os_threadPoolAddWork(OS_THREAD_POOL_HANDLE *handle, const char *name, OS_THREAD_POOL_WORK_FUNC func, 
						int highPrio, int duplicate, void *pArg1, int arg1Len, void *pArg2, int arg2Len)
{
	OS_THREAD_POOL_WORK *work = NULL;
	OS_THREAD_POOL_WORK **pNext = NULL;
	OS_THREAD_POOL_PTHREAD *pthread = NULL;
	int found = 0;
	int lowPrioThreadCount = 0;
	int count = 0;

#ifdef OS_THREAD_POOL_DEBUG
	struct timespec currTime;
	float diff = 0;
#endif /* OS_THREAD_POOL_DEBUG */

	if (!handle || !func)
	{
		return 0;
	}

#ifdef OS_THREAD_POOL_DEBUG
	fprintf(stderr, "###############################################################################\n");
	fprintf(stderr, "Add work: %s\n", name);
	pthread_mutex_lock(&handle->mutex);
	fprintf(stderr, "Thread Pool status:\n");
	for (pthread = handle->pthreadList; pthread; pthread = pthread->next)
	{
		if (pthread->work)
		{
			work = pthread->work;
			fprintf(stderr, "%s", work->name);
			clock_gettime(CLOCK_MONOTONIC, &currTime);
			diff = (float)(currTime.tv_nsec - work->startTime.tv_nsec) / 1000000000;
			diff += (currTime.tv_sec - work->startTime.tv_sec);
			fprintf(stderr, "\t%.6f", diff);
		}
		else
		{
			fprintf(stderr, "IDLE");
		}
		fprintf(stderr, pthread->highPrio ? "\tHIGH\n" : "\tLOW\n");
	}
	if (handle->highPrioQueue)
	{
		fprintf(stderr, "High Prio delayed works:\n");
		for (work = handle->highPrioQueue; work; work = work->next)
		{
			fprintf(stderr, "\t%s", work->name);
		}
		fprintf(stderr, "\n");
	}
	if (handle->lowPrioQueue)
	{
		fprintf(stderr, "Low Prio delayed works:\n");
		for (work = handle->lowPrioQueue; work; work = work->next)
		{
			fprintf(stderr, "\t%s", work->name);
		}
		fprintf(stderr, "\n");
	}
	pthread_mutex_unlock(&handle->mutex);
	fprintf(stderr, "###############################################################################\n");
#endif /* OS_THREAD_POOL_DEBUG */

	if (0 == duplicate)
	{
		pthread_mutex_lock(&handle->mutex);
/*
 * brief	maybe should disable this to make work continous
 * By	wangwenhao, 25Jan22
 */
#if 1
		for (pthread = handle->pthreadList; pthread; pthread = pthread->next)
		{
			if (pthread->work && pthread->work->func)
			{
				if (pthread->work->func == func)
				{
					found = 1;
					goto check_end;
				}
			}
		}
#endif

		if (highPrio)
		{
			work = handle->highPrioQueue;
		}
		else
		{
			work = handle->lowPrioQueue;
		}

		for (; work; work = work->next)
		{
			if (work->func == func)
			{
				found = 1;
				goto check_end;
			}
		}

check_end:
		pthread_mutex_unlock(&handle->mutex);

		if (found)
		{
#ifdef OS_THREAD_POOL_DEBUG
			fprintf(stderr, "Ignore duplicated work: %s, \n", name);
#endif /* OS_THREAD_POOL_DEBUG */
			return 0;
		}
	}

	work = malloc(sizeof(OS_THREAD_POOL_WORK));
	if (NULL == work)
	{
		fprintf(stderr, "malloc for OS_THREAD_POOL_WORK failed\n");
		return -1;
	}
	memset(work, 0, sizeof(OS_THREAD_POOL_WORK));

	if (pArg1 && arg1Len)
	{
		work->arg1 = malloc(arg1Len);
		if (NULL == work->arg1)
		{
			fprintf(stderr, "malloc for arg1 failed\n");
			goto err;
		}
		memcpy(work->arg1, pArg1, arg1Len);
	}

	if (pArg2 && arg2Len)
	{
		work->arg2 = malloc(arg2Len);
		if (NULL == work->arg2)
		{
			fprintf(stderr, "malloc for arg2 failed\n");
			goto err;
		}
		memcpy(work->arg2, pArg2, arg2Len);
	}

#ifdef OS_THREAD_POOL_DEBUG
	if (name)
	{
		work->name = strdup(name);
		if (NULL == work->name)
		{
			fprintf(stderr, "strdup for work name failed\n");
			goto err;
		}
	}
	else
	{
		work->name = l_anoymousWorkName;
	}
	clock_gettime(CLOCK_MONOTONIC, &work->startTime);
#endif /* OS_THREAD_POOL_DEBUG */

	work->func = func;
	work->next = NULL;

	if (highPrio)
	{
#if 1	/* FIFO */
		while (1)
		{
			pthread_mutex_lock(&handle->mutex);
			for (pNext = &handle->highPrioQueue, count = 0; *pNext; pNext = &(*pNext)->next, count++);
			/* loop end */
			if (count > handle->attr.highPrioThreadCount * OS_THREAD_POOL_WAIT_MULTI)
			{
				pthread_mutex_unlock(&handle->mutex);
				/* wait until queue reduced */
				sleep(1);
			}
			else
			{
				if (count > handle->attr.highPrioThreadCount * OS_THREAD_POOL_WARN_MULTI)
				{
					fprintf(stderr, "\033[1;33;40mHigh prio queue %d works delayed, "
									"consider increasing highPrioThreadCount\033[m\n", 
									count);
				}
				*pNext = work;
				pthread_mutex_unlock(&handle->mutex);
				break;
			}
		}
#else
		pthread_mutex_lock(&handle->mutex);
		work->next = handle->highPrioQueue;
		handle->highPrioQueue = work;
		pthread_mutex_unlock(&handle->mutex);
#endif
	}
	else
	{
#if 1	/* FIFO */
		while (1)
		{
			lowPrioThreadCount = handle->_pthreadCount - handle->attr.highPrioThreadCount;
			pthread_mutex_lock(&handle->mutex);
			for (pNext = &handle->lowPrioQueue, count = 0; *pNext; pNext = &(*pNext)->next, count++);
			/* loop end */
			if (count > lowPrioThreadCount * OS_THREAD_POOL_WAIT_MULTI)
			{
				pthread_mutex_unlock(&handle->mutex);
				/* wait until queue reduced */
				sleep(1);
			}
			else
			{
				if (count > lowPrioThreadCount * OS_THREAD_POOL_WARN_MULTI)
				{
					fprintf(stderr, "\033[1;33;40mLow prio queue %d works delayed, "
									"consider increasing pthreadCount\033[m\n",
									count);
				}
				*pNext = work;
				pthread_mutex_unlock(&handle->mutex);
				break;
			}
		}
#else
		pthread_mutex_lock(&handle->mutex);
		work->next = handle->lowPrioQueue;
		handle->lowPrioQueue = work;
		pthread_mutex_unlock(&handle->mutex);
#endif
	}

	if (highPrio)
	{
		pthread_cond_signal(&handle->highPrioCond);
	}
	else
	{
		pthread_cond_signal(&handle->lowPrioCond);
	}

	return 0;

err:
	os_threadPoolFreeWork(work);
	work = NULL;

	return -1;
}

/* 
 * fn		    OS_THREAD os_getTid() 
 * brief	    get current thread's ID
 * details	    
 *
 * return	    current thread's ID	    
 */
OS_THREAD os_getTid()
{
	return pthread_self();
}



/* 
 * fn		int os_queueCreate  (int maxMsg, int msgSize, OS_MSG_OPTIONS opt, OS_MSGQ *pQid)
 * brief	create a message queue
 * details	
 *
 * param [in]	maxMsg - for vxWorks, max messages that can be queued 
 * param [in]	msgSize - for vxWorks, max bytes in a message 
 * param [in]	opt - for vxWorks, message queue options 
 * param[out]	pQid - to keep the queue id.
 *
 * return	0 if success, -1 otherwise.		
 */
int os_queueCreate  (int maxMsg, int msgSize, OS_MSG_OPTIONS opt, OS_MSGQ *pQid)
{
    int flag;
	key_t key;

	assert(pQid != NULL);

	if (-1 == (key = ftok("/etc/services", queues++)))
	{
		printf("ftok error: no key generated");
        return -1;
	}
	
    flag = IPC_CREAT | 0666;
    *pQid = msgget(key, flag);
	
    if (*pQid == -1)
	{
        printf("msgget failed.");
        return -1;
    }
    return 0;
}



/* 
 * fn		int os_queueDelete  (OS_MSGQ qid)
 * brief	delete a message queue
 * details	
 *
 * param [in]	qid - message queue to delete 
 * param[out]	
 *
 * return	0 if success, -1 otherwise.		
 */
int os_queueDelete  (OS_MSGQ qid)
{
    if (-1 == msgctl(qid, IPC_RMID, NULL))
    {
       printf("msgget failed.");
       return -1;
    }

    return 0;
}



/* 
 * fn		int os_queueReceive(OS_MSGQ qid, long type, int timeout, int bufSize, char *pBuf)
 * brief	receive data from a message queue
 * details	This routine receives a message from the message queue qid. The received message is 
 *			copied into the specified buffer, which is bufSize in length. If the message is 
 *			longer than bufSize, the remainder of the message is discarded .
 *
 * param [in]	qid - message queue from which to receive  
 * param [in]	type - for Linux, message type to revice.
 * param [in]	timeout - for vxWorks, ticks to wait
 * param [in]	bufSize - size of buffer 
 * param[out]	pBuf - buffer to receive message 
 *
 * return	The number of bytes copied to buffer, or -1	
 */
int os_queueReceive(OS_MSGQ qid, long type, int timeout, int bufSize, char *pBuf)
{
    int ret = 0;
    int flag = 0;
    struct msgbuf {
        long mtype;
        char *text;
    }msg; 

	assert(pBuf != NULL);
	
	msg.text = pBuf;

    ret = msgrcv(qid, &msg, bufSize, type, flag); 

    if (ret == -1) {
        printf("msgrcv failed.\n");
        return -1;
    }
    strcpy(pBuf, msg.text);
    
    return ret;
}



/* 
 * fn		int os_queueSend(OS_MSGQ qid, int type, int timeout, int priority, char *pBuf)
 * brief	send data to a message queue
 * details	
 *
 * param [in]	qid - message queue on which to send
 * param [in]	type - for Linux, message type to send.
 * param [in]	timeout - for vxWorks, ticks to wait
 * param [in]	priority - for vxWorks,  MSG_PRI_NORMAL or MSG_PRI_URGENT 
 * param [in]	pBuf - message to send 
 * param[out]	
 *
 * return	0 if success, -1 otherwise.		
 */
int os_queueSend(OS_MSGQ qid, long type, int timeout, int priority, char *pBuf)
{
    int ret = 0;
    struct msgbuf {
        long mtype;
        char *text;
    }msg;

	assert(pBuf != NULL);

    msg.mtype = type;
    msg.text = pBuf;
    
    ret = msgsnd(qid, &msg, strlen(msg.text) + 1, 0);
    if(ret == -1) {
        printf("msgsend failed.");
        return -1;
    }
    
    return 0;
}



/* 
 * fn		int os_mutexCreate (OS_MUTEX *pMutex)
 * brief	create a mutex
 * details	
 *
 * param [in]	
 * param[out]	pMutex - to keep the mutex id.
 *
 * return	0 if success, -1 otherwise.		
 */
int os_mutexCreate (OS_MUTEX *pMutex)
{
	assert(pMutex != NULL);
	
    if (0 != pthread_mutex_init(pMutex,NULL))
    {
        printf("pthread mutex init failed.");
        return -1;
    }

    return 0;
}



/* 
 * fn		int os_mutexDestroy (OS_MUTEX *pMutex)
 * brief	delete a mutex
 * details	
 *
 * param [in]	pMutex - mutex to delete
 * param[out]	
 *
 * return	0 if success, -1 otherwise.		
 */
int os_mutexDestroy (OS_MUTEX *pMutex)
{
	assert(pMutex != NULL);
	
    if (0 != pthread_mutex_destroy(pMutex))
    {
        printf("pthread mutex destroy failed.");
        return -1;
    }

    return 0;
}



/* 
 * fn		int os_mutexLock(OS_MUTEX *pMutex)
 * brief	take a mutex
 * details	
 *
 * param [in]	pMutex - the mutex to take.
 * param[out]	
 *
 * return	0 if success, -1 otherwise.		
 */
int os_mutexLock(OS_MUTEX *pMutex)
{
	assert(pMutex != NULL);
	
    return pthread_mutex_lock(pMutex);
}



/* 
 * fn		int os_mutexUnlock(OS_MUTEX *pMutex)
 * brief	give a mutex
 * details	
 *
 * param [in]	pMutex - the mutex to give
 * param[out]	
 *
 * return	0 if success, -1 otherwise.	
 */
int os_mutexUnlock(OS_MUTEX *pMutex)
{
	assert(pMutex != NULL);
	
    return pthread_mutex_unlock(pMutex);
}



/* 
 * fn		int	os_shmGet(int key, size_t size, int shmFlg, void** ppShmAddr)
 * brief	Create the shared memory region.
 * details	For Linux, this routine is used to create a shared memory region. For vxWorks, this 
 *			routine is used to malloc a buffer.
 *
 * param [in]	key - key to get the shared memory region.
 * param [in]	size - shared memory size.
 * param [in]	shmFlg - flag to get the shared memory region.
 *
 * return	0 if success, -1 otherwise.	
 *
 * note		if key == 0, it will always create a new shared memory.
 */
int	os_shmGet(int key, size_t size, int shmFlg)
{
	return shmget((key_t)key, size, shmFlg);
}



/* 
 * fn		void *os_shmAt(int shmId, const void* pShmAddr, int shmFlg)
 * brief	attach to the shared memory.
 * details	For Linux, this routine, attach to the shared memory shmId, and return the address that
 *			the process attach to. For vxWorks, this routine just increase the counter of the shared
 *			memory.
 *
 * param [in]	shmId - for Linux, shared memory id.
 * param [in]	pShmAddr - In Linux, this address may be NULL or the fixed address that you want to
 *						   attath to. In vxWorks, this address is the address that return in 
 *						   os_shmGet.
 * param [in]	shmFlg - for Linux, attach flag.
 * param[out]	
 *
 * return	0 if success, -1 otherwise.	
 */
void *os_shmAt(int shmId, const void* pShmAddr, int shmFlg)
{
	return shmat(shmId, pShmAddr, shmFlg);
}



/* 
 * fn		int	os_shmDt(const void *pShmAddr)
 * brief	deattach the shared memory.
 * details	
 *
 * param [in]	pShmAddr - address of the shared memory.
 * param[out]	
 *
 * return	0 if success, -1 otherwise.		
 */
int	os_shmDt(const void *pShmAddr)
{
	return shmdt(pShmAddr);
}



/* 
 * fn		int os_shmDel(int shmId, void *pShmAddr)
 * brief	delete the shared memory.
 * details	In Linux, this routine delete the shared memory, if the attach number of the shared is 
 *			less than 2, otherwise just deattch the memory. In vxWorks, this routine free the shared 
 *			memory buffer, if the attach number of the shared is less than 2, otherwise juct deattch
 *			the memory.
 *
 * param [in]	shmId - for Linux.
 * param [in]	pShmAddr - shared memory addredd.
 * param[out]	
 *
 * return	0 if success, -1 otherwise.			
 */
int os_shmDel(int shmId, void *pShmAddr)
{
	struct shmid_ds shmbuf;

	/*
	* stat the shared memory to see how many processes are attached.
	*/
	memset(&shmbuf, 0, sizeof(shmbuf));
	if (shmctl(shmId, IPC_STAT, &shmbuf) < 0)
	{
		DEBUG_PRINT("shmId = %d, shmAddr = 0x%x", shmId, pShmAddr);
		printf("shmctl IPC_STAT failed");
		return -1;
	}
	else
	{
		DEBUG_PRINT("nattached=%d", (int)shmbuf.shm_nattch);
	}

	if (shmbuf.shm_nattch > 1 && pShmAddr != NULL)
	{
		/* other proceeses are still attached, just detach myself and return now. */
		if (shmdt(pShmAddr) != 0)
		{
			printf("shmdt of shmAddr=0x%x failed", pShmAddr);
		}
		else
		{
			DEBUG_PRINT("detached shmAddr=0x%x", pShmAddr);
		}

		return 0;
	}

	if (shmdt(pShmAddr) != 0)
	{
		printf("shmdt of shmAddr=0x%x failed", pShmAddr);
	}
	else
	{
		DEBUG_PRINT("detached shmAddr=0x%x", pShmAddr);
	}

	memset(&shmbuf, 0, sizeof(shmbuf));
	if (shmctl(shmId, IPC_RMID, &shmbuf) < 0)
	{
		printf("shm destory of shmId=%d failed.", shmId);
	}
	else
	{
		DEBUG_PRINT("shared mem (shmId=%d) destroyed.", shmId);
	}

	return 0;
}



/* 
 * fn		    int os_inet_aton(const char * pString, struct in_addr * pInetAddr) 
 * brief	    convert a network address from dot notation, store in a structure
 * details	    
 *
 * param [in]	pString - string containing address, dot notation
 * param[out]	pInetAddr - struct in which to store address
 *
 * return	    0 if success, -1, otherwise.	    
 */
int os_inet_aton(const char * pString, struct in_addr * pInetAddr)
{
	if (!inet_aton(pString, pInetAddr))	
	{
		return -1;
	}
	else
	{
		return 0;
	}
}


/* 
 * fn		int os_taskCreate(char * pName,  
 *							  int priority, 
 *							  int stackSize, 
 *							  FUNCPTR pFunc, 
 *							  void * pArg)
 * brief	create a Linux process or spawn a vxWorks task
 * details	
 *
 * param [in]	pName - for vxWorks, name of new  task.
 * param [in]	priority - for vxWorks, priority of new task
 * param [in]	stackSize - for vxWorks, size (bytes) of stack needed plus name 
 * param [in]	pFunc - entry point of new task or thread.
 * param [in]	pArg - task args to pass to func 
 *
 * return	0 if success, -1 otherwise.		
 */
int os_taskCreate(char *pName,  
				  int priority, 
				  int stackSize, 
				  FUNCPTR pFunc, 
				  void * pArg)
{
	if (NULL != strchr(pName, ';'))
	{
		printf("Parameter contains illegal character!");
		return -1;
	}

	system(pName);
	
	return 0;
}

/* 
 * fn 		int os_getMacByIp(char *pIpAddr, char *pMacStr)
 * brief 	Get mac addr by ip in system ARP table.
 *
 * param [in] pIpAddr Internet address of target;
 * param[out] pMacAddr where to return the H/W address string("00:e0:ec:69:35:d4");
 *
 * return 0 if success, -1 if error.
 *
 * note		Only for IPv4
 */
int os_getMacByIp(char *pIpAddr, char *pMacStr)
{
#define OS_IP_STRING_LEN	16
#define OS_MAC_STRING_LEN	18

	FILE *fp = NULL;
	char buff[OS_BUFLEN_256] = {0};
	int mark = 0;
	char *p = NULL, *q = NULL;
	int len = 0;

	fp =  fopen(_ARP_TABLE_NAME, "r");
	if (NULL == fp)
	{
		perror("getMacByIp");
		
		memset(pMacStr, 0 , OS_MAC_STRING_LEN);
		printf("Open arp proc error when get %s ip's mac\n", pIpAddr);

		return -1;
	}
	
	/* append whitespace to the IP string, otherwise, if we search "192.168.1.1" in 
	 * a string which contains "192.168.1.1xx", it return FOUND but not correct.
	 */
	len = strlen(pIpAddr);
	if (len < OS_IP_STRING_LEN - 1)
	{
		pIpAddr[len] = ' ';
		pIpAddr[len+1] = 0;
	}

	while (fgets(buff, OS_BUFLEN_256, fp) != NULL)
	{
		if ((p = strstr(buff, pIpAddr)) != NULL)
		{
			q = p;
			while (++q && q < buff + OS_BUFLEN_256 - 1)
			{
				if (*q == ' ' && mark == 0)
				{
					p = q + 1;
				}
				
				if (*q == ':')
				{
					*q = '-';
					mark = 1;
				}
				
				if (*q == ' ' && mark == 1)
				{
					strncpy(pMacStr, p, OS_MAC_STRING_LEN - 1);
					pMacStr[OS_MAC_STRING_LEN - 1] = 0;

					/* Restoration pIpAddr */
					if (len < OS_IP_STRING_LEN - 1)
					{
						pIpAddr[len] = 0;
					}
					fclose(fp);
					
					return 0;
				}
			}
		}
	};

	/* Restoration pIpAddr */
	if (len < OS_IP_STRING_LEN - 1)
	{
		pIpAddr[len] = 0;
	}
	fclose(fp);

#undef OS_IP_STRING_LEN
#undef OS_MAC_STRING_LEN

	return -1;
}

/* 
 * fn		timer_t os_timerCreate(int signum, sighandler handler_func)
 * brief	creates a interval timer, which will deliver signal to the thread each interval,
 *			and register a function to the signal, then function will be excuted each interval
 *
 * param[in]	signum			signum of signal to deliver 
 * param[in]	handler_func	function registered to signal
 * param[out]	N/A
 *
 * return	timer id or error
 * retval	tid		timer id for later control
 *			< 0		error
 *
 * note		
 */
timer_t os_timerCreate(int signum, sighandler handler_func)
{
	timer_t tid;
	struct sigevent se;
	int index = signum - SIGNUM_MIN;

	if (signum < SIGNUM_MIN || signum > SIGNUM_MAX)
	{
		printf("tpCreateTimer: signum is too small\n");
		return (timer_t)-1;
	}

	/* signal already registered? */
	if (timers[index])
	{
		printf("tpCreateTimer: signal %d has been used\n", signum);
		return (timer_t)-2;
	}

	memset (&se, 0, sizeof (se));
	
	signal(signum, handler_func);
	
	se.sigev_notify = SIGEV_SIGNAL;
	se.sigev_signo = signum;
	se.sigev_notify_function = handler_func;	
	//se.sigev_value.sival_int = signum;	
	//se.sigev_value.sival_ptr = (void *) &tid;
	
	if (timer_create(CLOCK_MONOTONIC, &se, &tid) < 0)
	{
		perror("timer_creat");
		return (timer_t)-3; 
	}

	timers[index] = tid;

	return tid;
}

/* 
 * fn		timer_t os_timerSet(timer_t timer_id, int firstRun, int interval)
 * brief	start the created timer
 *
 * param[in]	timer_id	timer id of timer to start
 * param[in]	firstRun	time waiting before first run (second)
 * param[in]	interval	interval after the first run (second), if 0, will not run
 *							any more after first run. WARNING: if 0, the timer wil not
 *							NOT be deleted automatically, and can be set to run again
 * param[out]	N/A
 *
 * return	result of start timer
 * retval	timer id	start OK
 *			-1	start ERROR
 *
 * note		WARNING: see param[in]	interval
 */
timer_t os_timerSet(timer_t timer_id, int firstRun, int interval)
{
	struct itimerspec ts, ots;
	
	ts.it_value.tv_sec = firstRun;
	ts.it_value.tv_nsec =  0;
	ts.it_interval.tv_sec = interval;
	ts.it_interval.tv_nsec = 0;

	/* we need a firstRun delay, so TIMER_ABSTIME cant be set -- lsz 081215 
	 *if (timer_settime(timer_id, TIMER_ABSTIME, &ts, &ots) < 0) */
	 
	if (timer_settime(timer_id, 0, &ts, &ots) < 0)
	{
		perror ("tpSetTimer");
		return (timer_t)(-1);
	}

	return timer_id;
}

/* 
 * fn		int os_timerDelete(timer_t timer_id)
 * brief	delete a created timer
 *
 * param[in]	timer_id	timer id of timer to delete
 * param[out]	N/A
 *
 * return	result of delete
 * retval	0		delete OK
 *			other	delete ERROR
 *
 * note		
 */
int os_timerDelete(timer_t timer_id)
{
	int i;

	for (i = 0; i < MAX_TIMER_NUM; i ++)
	{
		if (timers[i] == timer_id)
		{
			timers[i] = 0;
			return timer_delete(timer_id);
		}
	}

	printf("delete timer id %d failed: not found\n", timer_id);
	return -1;
}

/* 
 * fn		timer_t os_timerDelayRun(sighandler func, int nDelay)
 * brief	just run a function after a period of time
 *
 * param[in]	func	function to run later]
 * param[in]	nDelay	time to wait before running the function (second)
 * param[out]	N/A
 *
 * return	result of create and start the timer
 * retval	timer id		all OK
 *			other	ERROR ocurred
 *
 * note		
 */
timer_t os_timerDelayRun(sighandler func, int nDelay)
{
	return os_timerPeriodRun(func, nDelay, 0);
}

/* 
 * fn		timer_t os_timerPeriodRun(sighandler func, int nDelay, int nInterval)
 * brief	run a function repeatedly with appointed first wait and interval 
 *
 * param[in]	func		function to run repeatedly
 * param[in]	nDelay		time to wait before the first run (second)
 * param[in]	nInterval	interval time between each running after the first run (second)
 * param[out]	N/A
 *
 * return	result of create and start the timer
 * retval	timer id	all OK
 *			other	ERROR ocurred
 *
 * note		
 */
timer_t os_timerPeriodRun(sighandler func, int nDelay, int nInterval)
{
	timer_t tid = 0;
	int sig;
	for (sig = SIGNUM_DYNAMIC_MIN; sig < SIGNUM_MAX; sig++)
	{
		if (!timers[sig - SIGNUM_MIN])
		{
			tid = os_timerCreate(sig, func);

			if (tid < 0)
			{
				printf("Call tpCreateTimer failed\n");
				return (timer_t)-1;
			}
			
			return os_timerSet(tid, nDelay, nInterval);
		}
	}

	return (timer_t)-1;
}


/* 
 * fn		int os_getSysUpTime(unsigned int *upTime)
 * brief	Get syetem up time	
 * param[out]	upTime - return system up time	
 *
 * return	0 means OK;-1 means error	
 */
int os_getSysUpTime(unsigned int *upTime)
{
	struct sysinfo info;

	if (-1 == sysinfo(&info))
	{
		perror("Get system up time error:");
		
		*upTime = 0;
		return -1;
	}
	
	/* NOTICE:info.uptime is "long" data type */
	*upTime = (unsigned int)info.uptime;

	return 0;
}

/* 
 * fn				int os_getCpuInfo(CPU_USAGE_INFO *pCpuUsageInfo)
 * brief			Get CPU info
 * param[in/out]	pCpuUsageInfo - cpu	info struct
 *
 * return	0 means OK;-1 means error	
 */
int os_getCpuInfo(CPU_USAGE_INFO *pCpuUsageInfo)
{
	char *pChar = NULL;
	char line[OS_BUFLEN_256], buf[OS_BUFLEN_256];
	char dummy[OS_BUFLEN_32];
	char cpuUser[OS_BUFLEN_32];
	char cpuNice[OS_BUFLEN_32];
	char cpuIdle[OS_BUFLEN_32];
	char cpuSystem[OS_BUFLEN_32];
	char cpuIowait[OS_BUFLEN_32];
	char cpuIrq[OS_BUFLEN_32];
	char cpuSoftirq[OS_BUFLEN_32];
	
	/* read the cpu stats */
	FILE* fs = fopen("/proc/stat", "r");
	if ( fs == NULL ) 
	{
		printf("can't open /proc/stat");
		return -1;
	}

	if (NULL == fgets(line, sizeof(line), fs))
	{
		printf("read /proc/stat error");
		fclose(fs);
		return -1;
	}
		
	
	/*
	 * normally this file's content as follow:
	 * cpu  2749 0 10363 1917604 80 18332 446 0 0
	 * cpu0 675 0 9730 952849 63 11048 423 0 0
	 * cpu1 2073 0 632 964755 17 7283 22 0 0
	 * ...
	 * we just need the first line.
	 */
	pChar = strchr(line, ' ');
	if ( pChar != NULL )
	{
		pChar++;
	}
	if ( pChar != NULL && ('0' <= (pChar) && (pChar) <= '9') ) 
	{
		strcpy(buf, pChar);
		*pChar = ' ';
		strcpy(++pChar, buf);
	}
	sscanf(line, "%s  %s %s %s %s %s %s %s",
			   dummy, cpuUser, cpuNice, cpuSystem,
			   cpuIdle, cpuIowait, cpuIrq, cpuSoftirq);
	pCpuUsageInfo->user = strtoul(cpuUser, NULL, 10);
	pCpuUsageInfo->nice = strtoul(cpuNice, NULL, 10);
	pCpuUsageInfo->system = strtoul(cpuSystem, NULL, 10);
	pCpuUsageInfo->idle = strtoul(cpuIdle, NULL, 10);
	pCpuUsageInfo->iowait = strtoul(cpuIowait, NULL, 10);
	pCpuUsageInfo->irq = strtoul(cpuIrq, NULL, 10);
	pCpuUsageInfo->softirq = strtoul(cpuSoftirq, NULL, 10);

	pCpuUsageInfo->total = pCpuUsageInfo->user + pCpuUsageInfo->nice + 
							pCpuUsageInfo->system + pCpuUsageInfo->idle + 
				   			pCpuUsageInfo->iowait + pCpuUsageInfo->irq + 
				   			pCpuUsageInfo->softirq;

	fclose(fs);
	return 0;
}

/* 
 * fn				int os_getMemInfo(unsigned int *pTotal, unsigned int *pFree)
 * brief			Get memory info
 *
 * param[in/out]	pTotal - total memory
 * param[in/out]	pFree  - free memory
 *
 * return	0 means OK;-1 means error	
 */
int os_getMemInfo(unsigned int *pTotal, unsigned int *pFree)
{
	char line[OS_BUFLEN_256];
	unsigned int memTotal = 0;
	unsigned int memFree = 0;
	unsigned int memBuffers = 0;
	unsigned int memCached = 0;
	int ret = -1;
	char *pChar = NULL;

	/* read the memory stats */
	FILE* fs = fopen("/proc/meminfo", "r");
	if ( fs == NULL ) 
	{
		printf("can't open /proc/meminfo");
		return ret;
	}

	while ( 0 != ret &&
		fgets(line, sizeof(line), fs) )
	{
		/* search for MemTotal */
		if (strncmp(line, "MemTotal", 8) == 0 &&
			(pChar = strstr(line, ":")) != NULL &&
			(pChar + 1) != NULL)
		{
			/* pChar+1: read pass ":" */
			memTotal = (unsigned int) strtoul(pChar+1, (char **)NULL, 10);
		}

		/* search for MemFree */
		if (strncmp(line, "MemFree", 7) == 0 &&
			(pChar = strstr(line, ":")) != NULL &&
			(pChar + 1) != NULL)
		{
			/* pChar+1: read pass ":" */
			memFree = (unsigned int) strtoul(pChar+1, (char **)NULL, 10);
		}

		/* search for Buffers */
		if (strncmp(line, "Buffers", 7) == 0 &&
			(pChar = strstr(line, ":")) != NULL &&
			(pChar + 1) != NULL)
		{
			/* pChar+1: read pass ":" */
			memBuffers = (unsigned int) strtoul(pChar+1, (char **)NULL, 10);
		}

		/* search for Cached */
		if (strncmp(line, "Cached", 6) == 0 &&
			(pChar = strstr(line, ":")) != NULL &&
			(pChar + 1) != NULL)
		{
			/* pChar+1: read pass ":" */
			memCached = (unsigned int) strtoul(pChar+1, (char **)NULL, 10);
			ret = 0;
		}
	} /* while */

	fclose(fs);

	*pTotal = memTotal;
	*pFree = memFree + memBuffers + memCached;
	return ret;
}


#endif  /* __LINUX_OS_FC__ */

