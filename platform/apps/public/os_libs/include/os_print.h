/*  Copyright(c) 2009-2022 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 * file		os_print.h
 * brief		
 * details	
 *
 * author	Huang Qingjia
 * version	
 * date		2020.04.19
 *
 * history 	\arg	
 */

#ifndef __OS_PRINT_H__
#define __OS_PRINT_H__

/**************************************************************************************************/
/*                                           DEFINES                                              */
/**************************************************************************************************/

#define OS_PRINT_LEVEL_FORCE	(-1)
#define OS_PRINT_LEVEL_ERROR    (0)
#define OS_PRINT_LEVEL_WARN     (1)
#define OS_PRINT_LEVEL_DEBUG    (2)
#define OS_PRINT_LEVEL_TRACE    (3)
#define OS_PRINT_LEVEL_NUM		(4)

#define COLOR(clr_code)     clr_code

/* White background */
#define CLRr                COLOR("\e[0;31m")       /* red              */
#define CLRg                COLOR("\e[0;32m")       /* green            */
#define CLRy                COLOR("\e[0;33m")       /* yellow           */
#define CLRb                COLOR("\e[0;34m")       /* blue             */
#define CLRm                COLOR("\e[0;35m")       /* magenta          */
#define CLRc                COLOR("\e[0;36m")       /* cyan             */

/* blacK "inverted" background */
#define CLRrk               COLOR("\e[0;31;40m")    /* red     on blacK */
#define CLRgk               COLOR("\e[0;32;40m")    /* green   on blacK */
#define CLRyk               COLOR("\e[0;33;40m")    /* yellow  on blacK */
#define CLRmk               COLOR("\e[0;35;40m")    /* magenta on blacK */
#define CLRck               COLOR("\e[0;36;40m")    /* cyan    on blacK */

/* Colored background */
#define CLRcb               COLOR("\e[0;36;44m")    /* cyan    on blue  */
#define CLRyr               COLOR("\e[0;33;41m")    /* yellow  on red   */
#define CLRym               COLOR("\e[0;33;45m")    /* yellow  on magen */

/* Generic foreground colors */
#define CLRhigh            CLRm                     /* Highlight color  */
#define CLRmkBlink          COLOR("\e[5;35;40m")    /* Blink magenta on black */
#define CLRbold             CLRcb                   /* Bold      color  */
#define CLRbold2            CLRym                   /* Bold2     color  */
#define CLRerr              CLRyr                   /* Error     color  */
#define CLRBerr             COLOR ("\e[5;33;41m")   /* Blink Error color  */
#define CLRnorm             COLOR("\e[0m")          /* Normal    color  */
#define CLRnl               CLRnorm "\n"            /* Normal + newline */

/**************************************************************************************************/
/*                                           VARIABLES                                            */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                           FUNCTIONS                                            */
/**************************************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

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
 * param[in]  module, module name
 * param[in]  func, caller function name
 * param[in]  line, caller function line number
 * param[in]  fmt, format string for printf
 * param[out]
 *
 * return	-1:error, or print number of string
 * retval	
 *
 * note
 *    We add "__attribute__((format(printf,7,8))" to check the corresponding relationship of 7th and 8th argument.
 *    For example, the code below will cause GCC compile error,
 *
 *        int test(void)
 *        {
 *           int ret = 0;
 *           CWMP_PRINT("ret = %s", ret);
 *           return ret;
 *        }
 *        GCC: error: format ‘%s’ expects argument of type ‘char *’, but argument 8 has type ‘int’ [-Werror=format]
 *
 */
int os_print_write(int level,
						int headtail,
						const char *pModule,						
						const char *pFile,
						const char *pFunc,
						int line,
						const char *pFormat,
						...) __attribute__((format(printf,7,8)));

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
int os_print_setModLevel(const char *pModule, int level);

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
int os_print_setPidLevel(const char *pModule, int level);

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
int os_print_setTidLevel(const char *pModule, int level);

/* forbid to use CDBG_XXX */
void cdbg_printf(int dbgLevel, const char *call, int line, const char *fmt, ...) __attribute__((format(printf,4,5))) __attribute__ ((deprecated));
void cdbg_perror(const char *call, int line, int errCode) __attribute__ ((deprecated));

#if defined(__cplusplus)
}
#endif /* __cplusplus */


#endif	/* __OS_PRINT_H__ */

