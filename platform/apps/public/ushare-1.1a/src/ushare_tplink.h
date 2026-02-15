/*  Copyright(c) 2009-2011 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 * file		ushare_tplink.h
 * brief	tplink ˾صһЩʵ.	
 * details	
 *
 * author	LI CHENGLONG(port from HouXB)
 * version	1.0.0
 * date		24Nov11
 *
 * history 	\arg  1.0.0, 24Nov11, LI CHENGLONG, Create the file.	
 */

#ifndef __USHARE_TPLINK_H__
#define __USHARE_TPLINK_H__

/**************************************************************************************************/
/*                                          INCLUDES                                              */
/**************************************************************************************************/
#include <os_msg.h>
#include "list_template.h"
#include "content.h"
#include "ushare.h"
/**************************************************************************************************/
/*                                           DEFINES                                              */
/**************************************************************************************************/

#ifndef TRUE
#define TRUE 1
#endif
 
#ifndef FALSE
#define FALSE 0
#endif

#define BUFLEN_4        4
#define BUFLEN_6		6
#define BUFLEN_8        8
#define BUFLEN_16       16
#define BUFLEN_18       18
#define BUFLEN_20		20
#define BUFLEN_24       24
#define BUFLEN_32       32
#define BUFLEN_40       40
#define BUFLEN_48       48
#define BUFLEN_64       64
#define BUFLEN_128      128
#define BUFLEN_256      256
#define BUFLEN_264      264
#define BUFLEN_512      512
#define BUFLEN_1024     1024

#define DEVELOP_DEBUG	0
#define US_SCAN_DFINTERVAL 3600			/*1 hour*/
#define US_DEF_NAME	"MediaShare"


/* 
 * brief	Add ":1" to the end of ushare servername make our dut compatible to xbox.	
 *			Add by zengdongbiao, 29May15.
 */
#define USHARE_SERVERNAME_SUFFIX	":1"


#if  DEVELOP_DEBUG == 1
#define USHARE_DEBUG(format, args ...)	 do{printf("[%s:%d]"format, __FUNCTION__, __LINE__, ##args);}while(0)
#else
#define USHARE_DEBUG(format, args ...)
#endif /*DEVELOP_DEBUG == 1*/

#define USHARE_ERROR(format, args ...)	 do{printf("[%s:%d]"format, __FUNCTION__, __LINE__, ##args);fflush(stdout);}while(0)
/**************************************************************************************************/
/*                                           TYPES                                                */
/**************************************************************************************************/
typedef	unsigned char	UBOOL8;

typedef signed char		SINT8;
typedef signed short	SINT16;
typedef signed int		SINT32;

typedef unsigned char	UINT8;
typedef unsigned short	UINT16;
typedef unsigned int	UINT32;

/**************************************************************************************************/
/*                                           VARIABLES                                            */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                           FUNCTIONS                                            */
/**************************************************************************************************/

/* 
 *--------------------------------------------------------------------------------------------------
 *һЩpublicʵ,ushareطõ.
 *--------------------------------------------------------------------------------------------------
 */

/* 
 * fn		int ushareProcessInitInfo()
 * brief	ushare ̽ճʼϢ.	
 *
 * return		BOOL		ֵ
 * retval		TRUE		ɹ
 *				FALSE		ʧ	
 *
 * note	written by  24Nov11, LI CHENGLONG	
 */
BOOL ushareProcessInitInfo();

/* 
 * fn		int ushareProcessUserRequest()
 * brief	ushare ̼еû,д.	
 *
 * return	BOOL		ֵ
 * retval	FALSE		ʧ
 *			TRUE		ɹ
 *
 * note	written by  24Nov11, LI CHENGLONG	
 */
BOOL ushareProcessUserRequest();

/* 
 *--------------------------------------------------------------------------------------------------
 *Ŀ¼صһЩʵ.
 *--------------------------------------------------------------------------------------------------
 */
/* 
 * fn		BOOL ushare_rebuild_all(int flag) 
 * brief	յֶɨ,Զɨ,/var/usbdisk¸ıʱ͵øú.	
 * param[in]	flag	rebuildͬʱǷֹͣ0 Ϊֹͣ1Ϊֹͣ.
 * details	úͷcontent_list,ͷmetatree,ͷredblack tree;	
 *
 * return	BOOL	ֵ		
 * retval	0		ɹ
 *			-1		ʧ
 *
 * note	written by  18Dec11, LI CHENGLONG	
 */
BOOL ushare_rebuild_all(int flag);


/* 
 * fn		content_list *ushare_sync_mtree(content_list *list)
 * brief	content_list ,ýϢͺ.	
 *
 * param[in]	list		contentͷ.
 *
 * return		content_list*
 * retval		ºcontentͷ
 *
 * note	written by  18Dec11, LI CHENGLONG	
 */
content_list *ushare_sync_mtree(content_list *list);

/* 
 * fn		BOOL ushareRebuildMetaList()
 * brief	ɨ蹲Ŀ¼, ½ýϢ.	
 *
 * return		BOOL
 * retval		TRUE	ɹ
 *				FALSE	ʧ
 *
 * note	written by  24Nov11, LI CHENGLONG	
 */
BOOL ushareRebuildMetaList();

/* 
 * fn		int ushare_entry_add(int index)
 * brief	content_list±ΪindexcontentӵϢredblack.	
 *
 * param[in]	list	content_listͷ.
 * param[in]	index	contnetcontent_listе±.
 *
 * return		BOOL	ֵ
 * retval		TRUE	ʧ.
 *				FALSE	ӳɹ.
 *
 * note	written by  18Dec11, LI CHENGLONG	
 */
BOOL ushare_entry_add(content_list *list, int index);

/* 
 * fn		int ushare_entry_del(const char *fullpath)
 * brief	ͷrootentryµĳĿ¼ýϢռõڴ.	
 *
 * param[in]	fullpath	ýĿ¼·
 *
 * return		BOOL		ֵ
 * retval		TRUE		ͷųɹ
 *				FALSE		ͷʧ
 *
 * note	written by  18Dec11, LI CHENGLONG	
 */
BOOL ushare_entry_del(const char *fullpath);

/* 
 * fn		int build_content_list(ushare_t *ut)
 * brief	ôcontent_list.	
 *
 * param[in/out]	ut		ushareȫָ.
 *
 * return			BOOL		ֵ
 * retval			TRUE		ɹ
 *					FALSE		ʧ
 *
 * note	written by  18Dec11, LI CHENGLONG	
 */
BOOL build_content_list(ushare_t *ut);

/* 
 * fn		content_list *content_priv_del(content_list *list, const char *item)
 * brief	listɾ·ΪitemĹĿ¼.	
 * details	content.cвδʵͨ·ɾĿ¼ķ,ͨװṩһĽӿ.	
 *
 * param[in]	list		content
 * param[in]	item		·ַ	
 *
 * return		content_list*	
 * retval		ظºͷ.
 *
 * note	written by  18Dec11, LI CHENGLONG	
 */
content_list *content_priv_del(content_list *list, const char *item);

/* 
 * fn 		content_list *ushare_content_add(content_list *list, 
 											 const char *item, 
 											 const char *name, 
 											 const char *pFolderUuid,
 											 char flag)
 * brief	itemnameһ·.	
 * details	/var/usbdiskĿ¼ɨдڵĹ·,ӵУҪUUIDƥ䡣
 *
 * param[in]	list			Ŀ¼.
 * param[in]	shareAll		to indicate whether to share all the volumes or share the folders set by user
 * param[in]	item			Ŀ¼·.
 * param[in]	name			Ŀ¼.
 * param[in]	pFolderUuid		Ŀ¼ڷuuid.
 * param[in]	flag			ǷҪýϢ.
 *
 * return		content_list*
 * retval		ظºͷ.
 *
 * note	written by  16Dec11, LI CHENGLONG	
 *	modified by LY to add shareAll flag to work with newUI,in 2014.09.10
 */
content_list *ushare_content_add(content_list *list, int shareAll, const char *item, 
										const char *name, const char *pFolderUuid, char flag);

/* 
 * fn		content_list *ushare_content_del(content_list *list, 
 											 const char *item, 
 											 const char *name, 
 											 char flag)
 * brief	itemnameɾһ·.	
 * details	/var/usbdiskĿ¼ɨдڵĹ·,ӹɾ·.
 *
 * param[in]	list			Ŀ¼.
 * param[in]	item			Ŀ¼·.
 * param[in]	name			Ŀ¼.
 * param[in]	flag			ǷҪýϢ.
 *
 * return		content_list*
 * retval		ظºͷ.
 *
 * note	written by  16Dec11, LI CHENGLONG	
 */
content_list *ushare_content_del(content_list *list, const char *item, const char *name, char flag);

/* 
 * fn		BOOL ushareStartAutoScanWorker()
 * brief	Ushare ʱwoker. ڶʱ.	
 *
 * return	BOOL	ֵ	
 * retval	FALSE	ʧ
 *			TRUE	ɹ
 *
 * note	written by  25Nov11, LI CHENGLONG	
 */
BOOL ushareStartAutoScanWorker();

/* 
 *--------------------------------------------------------------------------------------------------
 *뿪رصһЩ.
 *--------------------------------------------------------------------------------------------------
 */

/* 
 * fn	 	BOOL ushareSwitchOn() 
 * brief	media server.
 *
 * return	BOOL	ֵ
 * retval	TRUE	ɹ	
 *			FALSE	ʧ
 *
 * note	written by  24Nov11, LI CHENGLONG	
 */
BOOL ushareSwitchOn();

/* 
 * fn	 	BOOL ushareSwitchOff() 
 * brief	رmedia server.	
 *
 * return	BOOL	ֵ
 * retval	TRUE	رճɹ
 *			FALSE 	رʧ	
 *
 * note	written by  24Nov11, LI CHENGLONG	
 */
BOOL ushareSwitchOff();

/* 
 * fn		BOOL ushareSwitchOnOff(BOOL onOff)
 * brief	رMediaServer.	
 *
 * param[in]	onOff	رMediaServer.
 *
 * return		BOOL	ֵ
 * retval		TRUE	óɹ
 *				FALSE	ʧ
 *
 * note	written by  24Nov11, LI CHENGLONG	
 */
BOOL ushareSwitchOnOff(BOOL onOff);

/* 
 * fn		BOOL ushareParseConfig(int argc, char **argv) 
 * brief	ushare̽,, ļ, ʼϢ.	
 * details	һ,еöutṹ,о͸ut.	
 *
 * param[in]	argc		ֱmainĲ
 * param[in]	argv		ֱmainĲ	
 *
 * return		BOOL		ֵ
 * retval		TRUE		ɹ
 *				FALSE		гִ
 *
 * note	written by  25Nov11, LI CHENGLONG	
 */
BOOL ushareParseConfig(int argc, char **argv);

/* 
 * fn		BOOL ushareInitUPnPEnv() 
 * brief	ushare̳ʼupnpĻ,upnpinitõ,ushareֻʼһ.	
 *
 * return		BOOL	ֵ
 * retval		TRUE	ʼɹ
 *				FALSE	ʼʧ
 *
 * note	written by  25Nov11, LI CHENGLONG	
 */
BOOL ushareInitUPnPEnv();

/* 
 * fn		int daemonize(int nochdir, int noclose)
 * brief	ǰ̷ŵ̨.	
 *
 * param[in]	nochdir		Ƿı̵̨ĹĿ¼
 * param[in]	noclode		Ƿı̵̨ı׼	
 *
 * return		int		
 * retval		-1		ʧ
 *				0		ɹ
 *
 * note	written by  24Nov11, LI CHENGLONG	
 */
int daemonize(int nochdir, int noclose);

#endif	/* __USHARE_TPLINK_H__ */
