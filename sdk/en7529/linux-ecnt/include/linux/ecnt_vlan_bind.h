#ifndef __VLAN_BIND_H
#define __VLAN_BIND_H
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/net.h>
#include <linux/netdevice.h>
#include <linux/types.h>
#include <uapi/linux/in.h>
#include <linux/if_vlan.h>
#if defined(TCSUPPORT_MULTI_USER_ITF) ||  defined(TCSUPPORT_MULTI_SWITCH_EXT)
#include <lan_port/lan_port_info.h>
#endif

#if defined(TCSUPPORT_CT_VLAN_BIND)

#if !defined(TCSUPPORT_MULTI_USER_ITF)
#if defined(TCSUPPORT_WLAN_AC)
#define MAX_LAN_PORT_NUM     14
#else
#define MAX_LAN_PORT_NUM    8
#endif
#endif

#define MAX_VLAN_GROUP 		20
typedef struct vlanBind {
	u16 lVlanId;
	u16 wVlanId;
} vlanBind_t;
extern vlanBind_t vBindArray[MAX_LAN_PORT_NUM][MAX_VLAN_GROUP];

int check_vlan_bind(struct sk_buff *skb, struct net_device *out_dev);
int check_vbind_lanvlan(struct sk_buff *skb, unsigned int mark);
void ecnt_vlan_bind_proc_init(void);
void ecnt_vlan_bind_proc_deinit(void);

#define 	VTAG_VID_MASK   		0xfff
#define 	VTAG_GET_VID(tci)   	( (tci) & VLAN_VID_MASK )
#define		ROUTING_MODE_VLAN_PACKET	(VLAN_PACKET | ROUTING_MODE_PACKET)
#endif
#endif
