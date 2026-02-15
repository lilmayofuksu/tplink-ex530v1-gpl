《BBA平台DEBUG接口使用说明》
					by Huangqingjia@tp-link.com.cn 2022.4.25

一. 格式输出

	1. 简略格式：
		[时间戳][模块] – 打印类别: 自定义信息\n
		
		譬如：
		[1910.010][cwmp] - ERROR: ...
		[1910.010][cwmp] - INFOs: ...
		
	2. 复杂格式：
		[时间戳][进程PID][模块][文件名][函数] 行号 – 打印类别: 自定义信息\n

		譬如：
		[1910.010][PID_752][cwmp][cwmp_core.c][cwmp_process] 1535 - ERROR: ...
		[1910.010][PID_752][cwmp][cwmp_core.c][cwmp_process] 1536 - INFOs: ...

二. 使用方法

	1.	修改menu/sysdeps/Debug.in，添加配置选项INCLUDE_DEBUG_XXX_PRINT（其中XXX是模块名称）
		譬如：
		config INCLUDE_DEBUG_CWMP_PRINT
			int "CWMP print level ( 1:info, 2:debug, 3:trace )"
			range 1,3
			default 1
			help
					you can shutdown message at runtime by deleting file in /var/tmp/os_print/cwmp_xxx.
					cwmp_info for info message, cwmp_debug for debug message, cwmp_trace for trace message.

		注意：default值必须是1，表示默认只编译/打印ERROR和INFO等级的信息。
		
	2.	模块中增加xxx_print.h（其中xxx是模块名称，具体参考附件），定义宏大概如下：
		譬如：
		------------------------------------------------------------------------
		#include "os_print.h"

		#define CWMP_MODULE_NAME "cwmp"

		#define CWMP_PRINT(type, headtail, args...) \
			os_print_write(OS_PRINT_LEVEL_##type, \
				headtail, \
				CWMP_MODULE_NAME, \
				__FILE__, \
				__FUNCTION__, \
				__LINE__, \
				args)

		#define CWMP_ERROR(args...) CWMP_PRINT(ERROR, TRUE, args)
		#define CWMP_ERROR_D(args...) CWMP_PRINT(ERROR, FALSE, args)

		#define CWMP_INFO(args...) CWMP_PRINT(INFO, TRUE, args)
		#define CWMP_INFO_D(args...) CWMP_PRINT(INFO, FALSE, args)

		#if INCLUDE_DEBUG_CWMP_PRINT >= OS_PRINT_LEVEL_DEBUG
			#define CWMP_DEBUG(args...) CWMP_PRINT(DEBUG, TRUE, args)
			#define CWMP_DEBUG_D(args...) CWMP_PRINT(DEBUG, FALSE, args)	
		#else /* INCLUDE_DEBUG_CWMP_PRINT ... */
			#define CWMP_DEBUG(args...)
			#define CWMP_DEBUG_D(args...)
		#endif /* INCLUDE_DEBUG_CWMP_PRINT ...*/

		#if INCLUDE_DEBUG_CWMP_PRINT >= OS_PRINT_LEVEL_TRACE
			#define CWMP_TRACE(args...) CWMP_PRINT(TRACE, TRUE, args)
			#define CWMP_TRACE_D(args...) CWMP_PRINT(TRACE, FALSE, args)
		#else
			#define CWMP_TRACE(args...)
			#define CWMP_TRACE_D(args...)
		#endif /* INCLUDE_DEBUG_CWMP_PRINT ...*/
		------------------------------------------------------------------------
		
	3.	修改模块Makefile，链接platform/apps/public/os_libs动态库（大部分app都已经链接，已链接了的省略这个步骤）

		LDFLAGS += -L$(OS_LIB_PATH)/ -los
		
	4.	模块代码中使用引用步骤2定义的宏打印自定义信息
		譬如：
		------------------------------------------------------------------------
		……
		#include “xxx_print.h”

		int module_function(...)
		{
			XXX_ERROR("This is fatal message");
			……
			XXX_INFO("This is normal message");
			……
			XXX_DEBUG("This is debug message");
			……
			XXX_TRACE("This is trace message");
			……
		}
		------------------------------------------------------------------------
	
		注意：XXX_ERROR/XXX_INFO/XXX_DEBUG/XXX_TRACE将会自动添加前缀（具体见格式输出说明）以及换行，
		如果不希望自动添加前缀和换行，请使用XXX_ERROR_D/XXX_INFO_D/XXX_DEBUG_D/XXX_TRACE_D
		譬如：
		------------------------------------------------------------------------
		……
		#include “xxx_print.h”

		int module_function(...)
		{
			……
			XXX_INFO_D("Device mac is ");
			for(i = 0; i< MAC_LEN; i++)
			{
				XXX_INFO_D("%02x ", mac[i]);
			}
			XXX_INFO_D("\n");
			……
		}
		------------------------------------------------------------------------
		
三. 使用技巧

	1.	运行时打印信息管理
		1.1 分为全部模块和特定模块两个等级控制管理
			1.1.1 进入/var/tmp/os_print/all，对所有模块的打印信息进行管理，影响所有模块打印。
			1.1.2 进入/var/tmp/os_print/xxx，对特定模块xxx进行管理，只影响特定模块打印。
			
		1.2  支持对不同类别打印信息进行屏蔽和打开操作。
			删除1_info，将会屏蔽模块INFO类别打印信息，反之创建文件会重新打开打印信息。
			删除2_debug，将会屏蔽模块DEBUG类别打印信息，反之创建文件会重新打开打印信息。
			删除3_trace，将会屏蔽模块TRACE类别打印信息，反之创建文件会重新打开打印信息。
			
			注意：ERROR类别信息无法屏蔽。
			
		1.3  输出格式切换。
			删除detail文件，ERROR和INFO的打印格式，将由复杂格式变为简略格式。反之，
			创建detail文件，RROR和INFO的打印格式，将由简略格式变为复杂格式。
			
			注意1：ERROR和INFO类别信息，初始默认是简略格式输出。
			注意2：DEBUG和TRACE类别信息不会受影响，将持续保持复杂格式输出。
		
		1.4 LOG收集
			进入特定模块目录下，创建log文件，则对应特定模块的打印信息将会在串口中显示的同时，
			也添加到该log文件中。
		
	2.  打印信息中添加颜色控制
		打印信息默认没有颜色控制，可以根据实际情景需要，添加颜色控制，达到提示的效果。
		譬如：
			XXX_INFO("this is " CLRr "red" CLRnorm " color.");  /* 其中 red字串将在窗口中输出为红色。*/
			XXX_INFO("this is " CLRg "green" CLRnorm " color.");  /* 其中 green字串将在窗口中输出为绿色。*/
			
			注意：CLRnorm的作用是将后续字符串颜色修正为常规颜色。这个是必须的，否则后续字符串的颜色都将受影响。
			提示：更多颜色定义，可以参考platform/apps/public/os_libs/include/os_print.h
	
