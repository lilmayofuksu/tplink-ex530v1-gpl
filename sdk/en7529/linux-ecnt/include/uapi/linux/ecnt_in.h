#ifndef _LINUX_ECNT_IN_H
#define _LINUX_ECNT_IN_H

#include <linux/version.h>

/*
#define IP_MINTTL       21
#define IP_NODEFRAG     22
*/
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,9,263)	
#define	IP_CUSTOM_DEFINE_START  25
#elif LINUX_VERSION_CODE > KERNEL_VERSION(3,19,8)	
#define	IP_CUSTOM_DEFINE_START  24 
#else
#define	IP_CUSTOM_DEFINE_START  22 
#endif
#define	IP_SKB_MARK_FLAG	IP_CUSTOM_DEFINE_START+1
#define	IP_SKB_VLAN_ID_FLAG	IP_SKB_MARK_FLAG+1


/*
#define IP_MULTICAST_ALL		49
#define IP_UNICAST_IF			50
*/
#define	IP_SKB_MARK			51
#define	IP_SKB_VLAN_ID		52

#define IP_SKB_PBIT 		54


/* interface type */
#define 	IF_TYPE_WAN_ROUTE			(1<<0)
#define		IF_TYPE_WAN_BRIDE			(1<<1)
#define		IF_TYPE_LAN					(1<<2)
#define		IF_TYPE_INTERNET			(1<<3)
#define		IF_TYPE_OTHER				(1<<4)
#define 	IF_TYPE_HAS_REGISTER		(1<<5)
#define 	IF_TYPE_LAN_BIND_INTERNET	(1<<6)
#define 	IF_TYPE_OTHER_WAN_BRIDE		(IF_TYPE_WAN_BRIDE | IF_TYPE_OTHER)

#define 	MAX_PVC_NUM 			8
#define		MAX_SMUX_NUM			8

#define 	WLAN_CPU_BIND_PATH    "tc3162/wlan_cpubind"

#if defined(TCSUPPORT_CT_VLAN_BIND)
#define 	VBIND_ENTRY_ARRAY_PATH "tc3162/vbind_entry_array"
#define		VBIND_INVALID_VLANID	4096
#define		VBIND_DNSPORT			53
#endif

#if defined(TCSUPPORT_TEST_SAMBA_SHORTCUT)
#define 	SAMBA_SHORTCUT_PATH "tc3162/samba_shortcut"
#endif
#define 	APP_SHORTCUT_PATH    "tc3162/app_shortcut"
#define 	APP_SHORTCUT_MAX_NUM 8

#if defined(TCSUPPORT_CT_JOYME4)
#define 	CAPABLE_USER_ROOT_SWITCH_PATH  "tc3162/capable_user_root_switch"
#endif
#define 	DBG_MSG_PATH "tc3162/dbg_msg"

#endif /* _LINUX_ECNT_IN_H */
