#ifndef __MSG_EXT_H
#define __MSG_EXT_H

#define EVT_WAN_INTERNAL                "wan.internal"
#define EVT_WAN_EXTERNAL				"wan.external"
#define EVT_BUF_LENGTH					(64)
#define EVT_TYPE_LENGTH					(32)

enum _evt_type_
{
	EVT_WAN_INTERNAL_TYPE = 1,
	EVT_WAN_EXTERNAL_TYPE = 2,
};

typedef struct _wan_evt_t
{
	char buf[EVT_BUF_LENGTH];

}wan_evt_t, *pt_wan_evt;

enum _evt_id_
{
	/*WAN Internal event*/
	EVT_CFG_WAN_ENTRY_DELETE    = 0x1,
	EVT_CFG_WAN_ENTRY_UPDATE    = 0x2,
	EVT_WAN_CONN_GETV4          = 0x3,
	EVT_WAN_CONN_LOSTV4         = 0x4,
	EVT_WAN_CONN_GETV6          = 0x5,
	EVT_WAN_CONN_LOSTV6         = 0x6,
	
	/*WAN external event*/
	EVT_WAN_ENTRY_DELETE 	    = 0x10,  //16
	EVT_WAN_ENTRY_UPDATE 	    = 0x11,  //17
	EVT_WAN_IPV4_UP 		    = 0x12,  //18
	EVT_WAN_IPV4_DOWN 		    = 0x13,  //19
	EVT_WAN_IPV6_UP 		    = 0x14,  //20
	EVT_WAN_IPV6_DOWN 		    = 0x15,  //21
	EVT_WAN_BRIDGE_UP 		    = 0x16,  //22
	EVT_WAN_BRIDGE_DOWN		    = 0x17,  //23
	EVT_WAN_UPDATE_CONNREQ_PORT = 0x18,  //24
	
	/*xpon evt*/
	EVT_XPON_UP 	 		    = 0x20,  //32
	EVT_XPON_DOWN 	 		    = 0x21,  //33

	/*WAN_RELATED Internal event*/
	EVT_CFG_WAN_RELATED_FIREWALL_UPDATE 	= 0x30,
	EVT_CFG_WAN_RELATED_ACL_UPDATE			= 0x31,
	EVT_CFG_WAN_RELATED_TIMEZONE_UPDATE		= 0x32,
	EVT_CFG_WAN_RELATED_APPFILTER_UPDATE	= 0x33,

	/*WAN_RELATED external event*/

	/*debug*/
	EVT_DUMP_WAN_SRV_ENTRY_DEV     = 0x200, //512
	EVT_DUMP_WAN_SRV_ENTRY_PATH	   = 0x201, //512
	EVT_DUMP_WAN_SRV_ALL_ENTRY     = 0x202, //514
	EVT_WAN_SRV_READY_IPV6_GATEWAY = 0x203, //515
	EVT_WAN_SRV_DEBUG_LEVEL        = 0x204, //516
};

#endif
