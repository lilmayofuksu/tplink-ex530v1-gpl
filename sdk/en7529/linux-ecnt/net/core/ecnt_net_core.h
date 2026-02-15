#ifndef _LINUX_ECNT_NET_CORE_H
#define _LINUX_ECNT_NET_CORE_H
#include <linux/version.h>
#include <linux/kernel.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,4,0)
#include <linux/kmemcheck.h>
#endif
#include <linux/compiler.h>
#include <linux/time.h>
#include <linux/bug.h>
#include <linux/cache.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,4,90)
#include <linux/proc_fs.h>
#endif
#include <linux/atomic.h>
#include <asm/types.h>
#include <linux/spinlock.h>
#include <linux/net.h>
#include <linux/textsearch.h>
#include <net/checksum.h>
#include <linux/rcupdate.h>
#include <linux/hrtimer.h>
#include <linux/dma-mapping.h>
#include <linux/netdev_features.h>
#include <linux/sched.h>
#include <linux/socket.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,90)
#include <net/flow_keys.h>
#endif
#include <ecnt_hook/ecnt_hook.h>
#include <ecnt_hook/ecnt_hook_fe.h>
#include <ecnt_hook/ecnt_hook_net.h>
#include <uapi/linux/ecnt_in.h>
#include <linux/ecnt_vlan_bind.h>

#ifdef TCSUPPORT_IPV6_ENHANCEMENT
#include <net/neighbour.h>
#include <linux/sysctl.h>
#endif
#include <linux/qos_type.h>
#include <asm/tc3162/tc3162.h>

extern int (*smux_pkt_recv_hook)(struct sk_buff *skb, 
                  struct net_device *dev,
                  struct net_device *rdev);

#ifdef TCSUPPORT_PORTBIND
#if defined(TCSUPPORT_CT)
#define ADD_GROUP		1
#define DEL_GROUP		2
extern int (*portbind_sw_hook)(void);
extern void (*portbind_update_hook)(struct net_device *dev, int type);
extern int (*portbind_check_hook)(struct net_device *in_dev, struct net_device *out_dev, struct sk_buff *skb);
#else
extern int (*portbind_sw_hook)(void);
/*TCSUPPORT_ROUTEPOLICY_PRIOR_PORTBIND*/
extern int (*portbind_sw_prior_hook)(struct sk_buff *skb);
extern struct net_device* (*portbind_get_outdev_by_indev_hook)(unsigned char* indev_name);
/*END TCSUPPORT_ROUTEPOLICY_PRIOR_PORTBIND*/
extern int (*portbind_check_hook)(char *inIf, char *outIf);
#endif
#endif

#if defined(TCSUPPORT_CT)
#if defined(TCSUPPORT_CT_VLAN_BIND)
extern int (*vlanbind_active_hook)(void);
extern int (*vlanbind_entry_active_hook)(int i);
#if defined(TCSUPPORT_CMCC)
extern int (*vlanbind_check_group_hook)(struct sk_buff *skb);
#endif
#endif
#endif

#if defined(TCSUPPORT_CT_VLAN_BIND) || defined(TCSUPPORT_CMCC) || defined(TCSUPPORT_PORTBIND)
extern struct net_bridge_fdb_entry *get_fdb_by_skb(struct sk_buff *skb);
#endif


#if defined(TCSUPPORT_CFG_NG_UNION)
#if defined(TCSUPPORT_CT_VLAN_TAG)
extern int (*check_vtag_ct_hook)(void);
extern int (*check_vtag_restore_ct_hook)(void);
extern int (*restore_vtag_ct_hook)(struct sk_buff **pskb);
#endif
#endif

#if defined(TCSUPPORT_CT_DS_LIMIT)
extern int (*dslimit_remarkQueue_hook)( struct sk_buff *skb, int up_dw );
#endif

#if defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7527)
extern int (*fe_resource_mark_meter_hook)(struct sk_buff *skb, int dir) ;   
extern int (*fe_resource_mark_acnt_hook)(struct sk_buff *skb, int dir) ;    
#endif

extern void (*get_net_device_interface_hw_stats_hook)(struct net_device *dev, struct rtnl_link_stats64 *storage);

static inline int ecnt_vlan_untag_inline_hook(struct sk_buff *skb, __be16 vlan_tci)
{
	if(vlan_tci&VLAN_CFI_MASK)
		skb->vlan_tag_flag |= VLAN_TAG_FOR_CFI;
	return 0;
}

#if defined(TCSUPPORT_PORTBIND) || defined(TCSUPPORT_CT_VLAN_BIND)
static int isDhcpv6ResponsePacket(struct sk_buff* skb)
{
	unsigned char *p = skb->data;
	int eth_type = 0;
	unsigned char next_header = 0;
	unsigned char msg_type = 0;
	
	if(skb == NULL)
		return 0;

	p = p + 12;//point to ethernet type
	eth_type = *(unsigned short *)p;
	if(eth_type == htons(0x8100)){
		p = p + 4;
		eth_type = *(unsigned short *)p;
		if(eth_type == htons(0x8100)){
			p = p + 4;
			eth_type = *(unsigned short *)p;
		}
	}
	
	/*ETH_P_IPV6 is 0x86dd*/
	if (eth_type != htons(ETH_P_IPV6)){
		return 0;
	}
	
	p = p + 2;//point to ip header
	next_header = *(unsigned char *)(p + 6);
	if(next_header != 0x11) {//not udp
		return 0;
	}
	p = p + 40;//pointer to udp
	p = p + 8; //pointer to dhcpv6
	msg_type = *(unsigned char *)p;
	if(msg_type >= 0x01 && msg_type <= 0x0d)//dhcpv6 msg type(1-13)
	{
		return 1;
	}
	return 0;
}

#define PROTOCOL_ICMPV6  0x3a
#define NDISC_ROUTER_ADVERTISEMENT	134

static int isRadvdResponsePacket(struct sk_buff* skb)
{
	unsigned char *p = skb->data;
	int eth_type = 0;
	unsigned char next_header = 0;
	unsigned char msg_type = 0;
	
	if(skb == NULL)
		return 0;

	p = p + 12;//point to ethernet type
	eth_type = *(unsigned short *)p;
	if(eth_type == htons(0x8100)){
		p = p + 4;
		eth_type = *(unsigned short *)p;
		if(eth_type == htons(0x8100)){
			p = p + 4;
			eth_type = *(unsigned short *)p;
		}
	}
	
	/*ETH_P_IPV6 is 0x86dd*/
	if (eth_type != htons(ETH_P_IPV6)){
		return 0;
	}
	
	p = p + 2;//point to ip header
	next_header = *(unsigned char *)(p + 6);
	if(next_header != PROTOCOL_ICMPV6) {//not icmpv6
		return 0;
	}

	p = p + 40;//pointer to icmpv6
	msg_type = *(unsigned char *)p;
	if(msg_type == NDISC_ROUTER_ADVERTISEMENT)
		return 1;
	return 0;
}
#endif

#ifndef TCSUPPORT_PON_VLAN
static inline int ecnt_skb_original_dev_inline_hook(struct sk_buff **skbp)
{
	struct sk_buff *skb = *skbp;
	if(skb->dev->name[0] != 'b')
		skb->original_dev = skb->dev;
		
	return ECNT_CONTINUE;
}
#endif
#if defined(TCSUPPORT_PON_VLAN) 
extern int (*pon_insert_tag_hook)(struct sk_buff **pskb);
extern int (*pon_check_vlan_hook)(struct net_device *dev, struct sk_buff *skb);
extern int (*pon_vlan_get_mode_hook)(void);

#ifdef TCSUPPORT_PON_SFU_HGU_HYBRID
extern int (*pon_hybrid_sfu_lan_check_hook)(struct sk_buff **pskb);
extern int (*pon_hybrid_sfu_wan_check_hook)(struct sk_buff **pskb);
#endif


static inline int ecnt_ponvlan_rx_inline_hook(struct sk_buff **skbp,struct net_device *orig_dev)
{	
/*!!!If pskb is used as the input parameter of a function, the skb must be reassigned after the function call is finished*/
		int vlan_mode = MODE_HGU;
		struct sk_buff *skb = *skbp;
		if(pon_vlan_get_mode_hook)
			vlan_mode = pon_vlan_get_mode_hook();

		int retval = ECNT_CONTINUE;
		if(orig_dev->name[0] == 'r' || orig_dev->name[0] == 'u' || orig_dev->name[0] == 'e')
		{
			if(vlan_mode == MODE_SFU)
							skb->pon_vlan_flag |= PON_PKT_FROM_LAN;

#ifdef TCSUPPORT_PON_SFU_HGU_HYBRID
			else if(vlan_mode == MODE_HGU){
				if(pon_hybrid_sfu_lan_check_hook && pon_hybrid_sfu_lan_check_hook(skbp))
					skb->pon_vlan_flag |= PON_PKT_FROM_HYBRID_PPTP;
				else
					skb->pon_vlan_flag |= PON_PKT_FROM_HYBRID_VEIP;
			}
#endif
		}
		else if(strncmp(orig_dev->name,"pon", 3) == 0 && vlan_mode == MODE_SFU)
		{
			skb->pon_vlan_flag |= PON_PKT_FROM_WAN;
		}
#ifdef TCSUPPORT_PON_SFU_HGU_HYBRID
		else if(strncmp(orig_dev->name,"pon", 3) == 0 && 
                        (pon_hybrid_sfu_wan_check_hook && pon_hybrid_sfu_wan_check_hook(skbp)))
		{
			skb->pon_vlan_flag |= PON_PKT_FROM_HYBRID_SFU_WAN; //hybrid mode, sfu port downstream
		}
#endif

		if(skb->dev->name[0] != 'b')
		skb->original_dev = skb->dev;
		if(pon_insert_tag_hook && vlan_mode == MODE_HGU)
		{
			if(strcmp(orig_dev->name,"pon") == 0)
				skb->pon_vlan_flag |= PON_VLAN_RX_CALL_HOOK;

#if defined(TCSUPPORT_CMCC)
			if(skb->pon_vlan_flag & PON_PKT_DROP_FLAG){
				if(!isTR069Wan(orig_dev)){
					kfree_skb(skb);
					return ECNT_RETURN_DROP;
				}
			}
#endif
/*!!!If pskb is used as the input parameter of a function, the skb must be reassigned after the function call is finished*/
			if(pon_insert_tag_hook)
			{
				retval = pon_insert_tag_hook(skbp);
				if(retval == -1)
				{
					kfree_skb(skb);
					return ECNT_RETURN_DROP;
				}
				if(retval == -2)
				{
					return ECNT_RETURN;
				}
				skb = *skbp;
			}

		}
#ifdef TCSUPPORT_PON_IP_HOST
		else if(pon_insert_tag_hook && vlan_mode == MODE_SFU){
			if(isVoipWan(orig_dev)){
				skb->pon_vlan_flag |= PON_PKT_VOIP_RX;
				if(pon_insert_tag_hook(&skb) == -1)
				{
					kfree_skb(skb);
					return ECNT_RETURN_DROP;
				}
			}
		}
#endif
	return ECNT_CONTINUE;
}

static inline int ecnt_ponvlan_xmit_inline_hook(struct sk_buff **skbp, struct net_device *dev)
{
/*!!!If pskb is used as the input parameter of a function, the skb must be reassigned after the function call is finished*/
	struct sk_buff *skb = *skbp;
	int vlan_mode = MODE_HGU;
	int retval = ECNT_CONTINUE;
	if(pon_vlan_get_mode_hook)
		vlan_mode = pon_vlan_get_mode_hook();

    if(vlan_mode == MODE_SFU)
	{
		/* packet from cpe */
        if (skb->original_dev == NULL) 
		{
			skb->pon_vlan_flag |= PON_PKT_FROM_CPE;
        }
	}
	else
	{
		if(strncmp(skb->dev->name,"pon", 3) == 0)		
			skb->pon_vlan_flag |= PON_VLAN_TX_CALL_HOOK;
	}
/*!!!If pskb is used as the input parameter of a function, the skb must be reassigned after the function call is finished*/
	if(pon_insert_tag_hook)
	{	
		retval = pon_insert_tag_hook(skbp);
		if(retval == -1)
		{
			kfree_skb(*skbp);
			return ECNT_RETURN_DROP;
        }
		if(retval == -2)
			return ECNT_RETURN;
		skb = *skbp;
	}
    if(pon_check_vlan_hook)
	{
		retval = pon_check_vlan_hook(dev,skb);
		if(retval != 1)
			return ECNT_RETURN;
	}
	return ECNT_CONTINUE;
}
#endif


#if defined(TCSUPPORT_CT)
#if defined(TCSUPPORT_PORTBIND) 
static inline void ecnt_portbind_rx_ct(struct sk_buff *skb,struct net_device *orig_dev){	

		if((orig_dev->name[0] == 'o') || !strcmp(orig_dev->name, "pon")
#if defined(TCSUPPORT_CT_DSL_EX) 
			|| strstr(orig_dev->name, "ptm")
#endif
			)
			return;
		 /*we only check OSMUX interface and other interface*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0) 
		if((orig_dev->name[0] != 'n') || (orig_dev->priv_flags & IFF_OSMUX)){
#else
		if((orig_dev->name[0] != 'n') || (orig_dev->dev_flags & IFF_OSMUX)){
#endif
			if( ((skb->portbind_mark & MASK_ORIGIN_DEV) == 0)){
				skb->portbind_mark |= MASK_ORIGIN_DEV;
				skb->orig_dev = skb->dev;
			}		
			if (orig_dev->name[0] == 'p') {
				skb->orig_dev = skb->dev;
			}
		}
#if defined(TCSUPPORT_VXLAN)
		if ( skb->orig_dev && 0 == strncmp(orig_dev->name, "vxlan", 5) )
			skb->orig_dev = skb->dev;
#endif
}
#endif
#endif

#ifdef TCSUPPORT_PORTBIND
#if !defined(TCSUPPORT_CT)
static inline void ecnt_portbind_rx(struct sk_buff *skb,struct net_device *orig_dev){	
#if defined(TCSUPPORT_FTP_THROUGHPUT)
		if (portbind_sw_hook) {
#endif
	/*
#if defined(TCSUPPORT_ROUTEPOLICY_PRIOR_PORTBIND)
		if ((portbind_sw_prior_hook) && (portbind_sw_prior_hook(skb) == 1)) {
#else
		if (portbind_sw_hook && (portbind_sw_hook() == 1)) {
#endif
	*/
	
	
#ifdef CONFIG_SMUX
		 /*we only check OSMUX interface and other interface*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0)
		if( (orig_dev->priv_flags & IFF_OSMUX) || (skb->portbind_mark & MASK_ORIGIN_DEV) == 0)
#else
		if( (orig_dev->dev_flags & IFF_OSMUX) || (skb->portbind_mark & MASK_ORIGIN_DEV) == 0)
#endif
		{
			skb->portbind_mark |= MASK_ORIGIN_DEV;
			memcpy(skb->orig_dev_name, orig_dev->name, IFNAMSIZ);
		}
#else
		if( (skb->portbind_mark & MASK_ORIGIN_DEV) == 0)
		{
			skb->portbind_mark |= MASK_ORIGIN_DEV;
			memcpy(skb->orig_dev_name, orig_dev->name, IFNAMSIZ);
		}
#endif
#if defined(TCSUPPORT_FTP_THROUGHPUT)
		}
#endif
}
#endif
#endif

#if defined(TCSUPPORT_VLAN_TAG)
static inline int ecnt_insert_vlan_tag_inline_hook(struct sk_buff **skbp, struct net_device *dev)
{
/*!!!If pskb is used as the input parameter of a function, the skb must be reassigned after the function call is finished*/
	struct sk_buff *skb = *skbp;
#if defined(TCSUPPORT_PON_VLAN)
	int vlan_mode = MODE_HGU;
#endif

#ifdef TCSUPPORT_PON_VLAN
	if(pon_vlan_get_mode_hook)
		vlan_mode = pon_vlan_get_mode_hook();
	
	if(vlan_mode == MODE_HGU)
#endif	
	{
	if (check_vtag_hook && (check_vtag_hook()) == 1)
	{
/*!!!If pskb is used as the input parameter of a function, the skb must be reassigned after the function call is finished*/
		if (insert_vtag_hook && (-1 == insert_vtag_hook(skbp)))
		{
			return ECNT_RETURN_DROP;
		}
		skb = *skbp;
	}
	}
	return ECNT_CONTINUE; 

}

static inline int ecnt_remark_vlan_pbit_inline_hook(struct sk_buff * skb)
{
	if (NULL != skb->sk && skb->sk->lPbit <= 7 )
	{
		skb->vlan_tag_flag |= VLAN_TAG_PBIT_REMARK_ENABLE;
		skb->vlan_tag_flag &= ~(0x7<<16);
		skb->vlan_tag_flag |= (skb->sk->lPbit << 16);
	}

	return ECNT_CONTINUE;
}
#endif

#if defined(TCSUPPORT_PON_MAC_FILTER)
extern int (*pon_check_mac_hook)(struct sk_buff *skb);
extern int (*pon_mac_filter_get_mode_hook)(void);

static inline int ecnt_pon_macfilter_rx_inline_hook(struct sk_buff **skbp,struct net_device *orig_dev)
{	
	int mac_filter_mode = MODE_HGU;
	struct sk_buff *skb = *skbp;
	int retval = ECNT_CONTINUE;
	
	if(pon_mac_filter_get_mode_hook)
		mac_filter_mode = pon_mac_filter_get_mode_hook();

	if(pon_check_mac_hook){
		if(mac_filter_mode == MODE_HGU)
		{
			if(strcmp(orig_dev->name,"pon") == 0)
				skb->pon_mac_filter_flag |= PON_MAC_FILTER_RX_CALL_HOOK;

			retval = pon_check_mac_hook(*skbp);
			if(retval == -1)
			{				
				kfree_skb(*skbp);
				return ECNT_RETURN_DROP;
			}
		} 
#ifdef TCSUPPORT_CHS
		else if(mac_filter_mode == MODE_SFU)
		{
			if (orig_dev->name[0] == 'e'|| orig_dev->name[0] == 'r'|| orig_dev->name[0] == 'u')
				skb->pon_mac_filter_flag |= PON_MAC_FILTER_RX_CALL_HOOK;

			retval = pon_check_mac_hook(*skbp);
			if(retval == -1)
			{				
				kfree_skb(*skbp);
				return ECNT_RETURN_DROP;
			}
		}
#endif
	}
		
	return ECNT_CONTINUE; 
}


static inline int ecnt_pon_macfilter_xmit_inline_hook(struct sk_buff **skbp)
{
	struct sk_buff *skb = *skbp;	
	int mac_filter_mode = MODE_HGU;
	int retval = ECNT_CONTINUE;

	if(pon_mac_filter_get_mode_hook)
		mac_filter_mode = pon_mac_filter_get_mode_hook();


	if(pon_check_mac_hook)
	{
		if(mac_filter_mode == MODE_SFU 
			&& (skb->dev->name[0] == 'e' || skb->dev->name[0] == 'r' || skb->dev->name[0] == 'u'))
			skb->pon_mac_filter_flag |= PON_MAC_FILTER_TX_CALL_HOOK;

		retval = pon_check_mac_hook(*skbp);
		if(retval == -1)
		{		
			//printk("[%s:%d]pon_check_mac_hook fail:kfree,orig_dev->name=%s\n",__FUNCTION__,__LINE__, skb->dev->name);
			kfree_skb(*skbp);
			return ECNT_RETURN_DROP;
		}
	}

	return ECNT_CONTINUE;
}
#endif

#if defined(TCSUPPORT_GPON_DOWNSTREAM_MAPPING)
extern int (*gpon_downstream_mapping_hook)(struct sk_buff *skb);
static inline int ecnt_downstream_mapping_inline_hook(struct sk_buff *skb)
{
	if(gpon_downstream_mapping_hook && (-1 == gpon_downstream_mapping_hook(skb)))
		return ECNT_RETURN_DROP;

	return ECNT_CONTINUE;
}
#endif
static inline int ecnt_netif_recv_inline_hook(struct sk_buff **pskb)
{
/*!!!If pskb is used as the input parameter of a function, the skb must be reassigned after the function call is finished*/
	struct sk_buff *skb = *pskb;
	int ret;
	struct net_device *orig_dev;
	orig_dev = skb->dev;
#if defined(TCSUPPORT_PON_VLAN)
	int vlan_mode = MODE_HGU;
#endif

	ECNT_CORE_DEV_HOOK(ECNT_NETIF_RCV_SKB, skb);
#ifndef TCSUPPORT_PON_VLAN
	ecnt_skb_original_dev_inline_hook(pskb);
	skb = *pskb;
#endif
#if defined(TCSUPPORT_PON_VLAN) 
/*!!!If pskb is used as the input parameter of a function, the skb must be reassigned after the function call is finished*/
	ret = ecnt_ponvlan_rx_inline_hook(pskb, orig_dev);
	if(ret != ECNT_CONTINUE)
		return ECNT_RETURN;
	skb = *pskb;
#endif

#ifdef TCSUPPORT_PON_VLAN
	if(pon_vlan_get_mode_hook)
		vlan_mode = pon_vlan_get_mode_hook();

	if(vlan_mode == MODE_HGU)
#endif
	{
#ifdef TCSUPPORT_VLAN_TAG
		  if (check_vtag_hook && (check_vtag_hook() == 1))
		  {
			  if (get_vtag_hook)
				  if (-1 == get_vtag_hook(orig_dev, skb)) {
					  kfree_skb(skb);
					  return ECNT_RETURN_DROP;
				  }
		  }
#endif	
	}

#if defined(TCSUPPORT_PON_MAC_FILTER) 
/*!!!If pskb is used as the input parameter of a function, the skb must be reassigned after the function call is finished*/
	ret = ecnt_pon_macfilter_rx_inline_hook(pskb, orig_dev);
	if(ret != ECNT_CONTINUE){
		return ECNT_RETURN;
	}
	skb = *pskb;
#endif

#if defined(TCSUPPORT_VLAN_ACCESS_TRUNK) 
	if (orig_dev && 
		(!strncmp(orig_dev->name, "nas", 3) || !strncmp(orig_dev->name, "ppp", 3)))
	{
		skb->wan_dev = orig_dev;
	}
#endif

#if defined(TCSUPPORT_CT)
#if defined(TCSUPPORT_PORTBIND) 
	ecnt_portbind_rx_ct(skb, orig_dev);
#endif
#endif

#ifdef TCSUPPORT_PORTBIND
#if !defined(TCSUPPORT_CT)
	ecnt_portbind_rx(skb, orig_dev);
#endif
#endif

#ifdef CONFIG_SMUX
	if((orig_dev->priv_flags & IFF_RSMUX) && smux_pkt_recv_hook
#ifdef TCSUPPORT_XPON_HAL_API_EXT
		&&!(skb->mark&DOWNSTREAM_SIMULATION_MASK)
#endif
	){
		ret = smux_pkt_recv_hook(skb, skb->dev, orig_dev);		  
		return ECNT_RETURN;
	}
#endif

	return ECNT_CONTINUE;
}

#if defined(TCSUPPORT_CT)
#ifdef TCSUPPORT_PORTBIND
#if defined(TCSUPPORT_CT_VLAN_BIND)
static inline int ecnt_vbind_portbindxmit_inline_hook_ct
(struct sk_buff *skb)
{
	struct net_device *dev = skb->dev;
	int port_idx = 0;

	/* vlan bind is ON. */
	if ( vlanbind_active_hook 
		&& (0 != vlanbind_active_hook()) )
	{
#if defined(TCSUPPORT_MULTI_USER_ITF) || defined(TCSUPPORT_MULTI_SWITCH_EXT)
		port_idx = GET_LAN_ITF_MARK(skb->mark);
#else
		port_idx = (skb->mark & 0xF0000000) >> 28;
#endif
		/*packets From ont to lan, return continue*/
		if((skb->orig_dev == NULL) && (skb->dev->name[0] == 'e'))
			return ECNT_CONTINUE;
		/* packets FROM LAN port and it is vlan binded. */
		if ( vlanbind_entry_active_hook
		&& (0 != vlanbind_entry_active_hook(port_idx)) )
		{
			/* check vlan bind for bridge wan. */
			if ( ( 'n' == skb->dev->name[0] ) 
				&& ( IF_TYPE_WAN_BRIDE == 
				(dev->bind_type & IF_TYPE_WAN_BRIDE)) )
			{
				if ( 0 == check_vlan_bind(skb, dev) )
				{
					kfree_skb(skb);
					return ECNT_RETURN_DROP;
				}
			}

#if defined(TCSUPPORT_CMCC)
			if (vlanbind_check_group_hook && (vlanbind_check_group_hook(skb) == 0)) 
			{
				kfree_skb(skb);
				return ECNT_RETURN_DROP;
			}
#endif
			if(skb->dev->name[0] == 'n' && (skb->dev->vlan_mode == 3 || skb->dev->vlan_mode == 4) && skb->lan_vlan_tci_valid == 1)
			{			
				if(check_vbind_lanvlan(skb, port_idx) == 0)				
				{
					kfree_skb(skb);
					return ECNT_RETURN_DROP;
				}	
			}
			return ECNT_RETURN;
		}
	}

	return ECNT_CONTINUE;
}

static inline int ecnt_get_vlan_and_bindtype_from_skb_inline_hook_ct(struct sk_buff *skb, u8 * bind_type, u16 * vlan)
{
	char dev_name[IFNAMSIZ] = {0};
	struct net_device *dev = NULL;
	
	snprintf(dev_name,sizeof(dev_name),"nas%d_%d",(skb->mark >> 24)/8,(skb->mark >> 24)%8);
	dev = dev_get_by_name(&init_net,dev_name);
	if(dev == NULL)
	{
		return -1;
	}
	*bind_type = dev->bind_type;	
	*vlan = dev->tci;
	dev_put(dev);
	return 0;
}

static inline int ecnt_vbind_xmit_inline_hook_ct(struct sk_buff *skb)
{
	int port_idx = 0, idx = 0, j = 0;
	u16 lVlanId = VBIND_INVALID_VLANID;
	u8	bind_type = 0;
	u16 vlan = VBIND_INVALID_VLANID;
	struct net_bridge_fdb_entry *fdb = NULL;
#ifdef TCSUPPORT_XSI_ENABLE
    if((skb->original_dev) && ('x' == skb->original_dev->name[0])){
        return ECNT_CONTINUE;
    }
#endif

	/* vlan bind is ON. */
	if ( vlanbind_active_hook 
		&& ( 0 != vlanbind_active_hook() ) )
	{
#if defined(TCSUPPORT_MULTI_USER_ITF) || defined(TCSUPPORT_MULTI_SWITCH_EXT)
		port_idx = devname_convert_port(skb->dev->name);
#else	
		if (isLANInterface(skb->dev))
			port_idx = getLANIndex(skb->dev) + 1;
#ifdef TCSUPPORT_WLAN_AC
		else if ( 'r' == skb->dev->name[0]
			&& 'i' == skb->dev->name[2] )
			port_idx = skb->dev->name[3] - '0' + 11;
#endif
		else if ( 'r' == skb->dev->name[0] )
			port_idx = skb->dev->name[2] - '0' + 5;
#endif
		/* packets TO LAN port and it is vlan binded. */
		if ( vlanbind_entry_active_hook
			&& 0 != vlanbind_entry_active_hook(port_idx) )
		{
			/* dns packets */
			if ( (VLAN_TAG_FOR_DNS == 
				(skb->vlan_tag_flag & VLAN_TAG_FOR_DNS))
				&& (skb->vlan_tags[0] < VBIND_INVALID_VLANID) )
			{
				skb = __vlan_put_tag(skb, skb->vlan_tags[0]);
				if ( !skb )
					return ECNT_RETURN;
			}
			/* wan route mode */
			else if ( skb->nfct && skb->orig_dev 
				&& (IF_TYPE_WAN_ROUTE == 
				(skb->orig_dev->bind_type & IF_TYPE_WAN_ROUTE)) )
			{
				if ( skb->nfct->lVlanId < VBIND_INVALID_VLANID )
				{
#if defined(TCSUPPORT_CMCC)
					for(j=0;j<MAX_VLAN_GROUP;j++){
						if(vBindArray[port_idx-1][j].lVlanId == (skb->nfct->lVlanId & 0xfff))
						{
#endif
					skb = __vlan_put_tag(skb, vBindArray[port_idx-1][j].lVlanId);
					if ( !skb )
						return ECNT_RETURN;
#if defined(TCSUPPORT_CMCC)
					break;
					}
					else if(htons(ETH_P_IPV6) == skb->protocol)
					{							
						if(vBindArray[port_idx-1][j].wVlanId == skb->vlan_tags[0])
						{
							skb = __vlan_put_tag(skb, vBindArray[port_idx-1][j].lVlanId);
							if ( !skb )
								return ECNT_RETURN;
							break;
						
						}
					}
				}
#endif
				}
			}
			/* wan bridge mode */
			else if ( skb->orig_dev && ( 'n' == skb->orig_dev->name[0])
#if defined(TCSUPPORT_CMCC)
				&& (IF_TYPE_WAN_BRIDE == (skb->orig_dev->bind_type & IF_TYPE_WAN_BRIDE)))
#else
				&& (IF_TYPE_OTHER_WAN_BRIDE ==
				(skb->orig_dev->bind_type & IF_TYPE_OTHER_WAN_BRIDE)) )
#endif
			{
				for ( idx = 0; idx < MAX_VLAN_GROUP; idx ++ )
				{
					if ( VBIND_INVALID_VLANID
						== vBindArray[port_idx-1][idx].lVlanId )
							break;

					if ( VTAG_GET_VID(skb->orig_dev->tci) 
						== vBindArray[port_idx-1][idx].wVlanId )
					{
						lVlanId = vBindArray[port_idx-1][idx].lVlanId;
						break;
					}
				}
				if ( lVlanId < VBIND_INVALID_VLANID )
				{
#if defined(TCSUPPORT_CMCC)
#if defined(TCSUPPORT_LAN_VLAN)
					struct net_bridge_fdb_entry *dst_fdb = NULL;
					dst_fdb = get_fdb_by_skb(skb);
					if((dst_fdb != NULL) && (dst_fdb->vlan_layer != 0))
					{
						if((dst_fdb->vlan_id & 0xfff) == lVlanId){
#endif
#endif
					lVlanId |= (((skb->vlan_tag_flag&(0xF00))>>8) << 12);
					skb = __vlan_put_tag(skb, lVlanId);
					if ( !skb )
						return ECNT_RETURN;
#if defined(TCSUPPORT_CMCC)
#if defined(TCSUPPORT_LAN_VLAN)
						}
					}
#endif
#endif
				}
				else
					{
						kfree_skb(skb);
						return ECNT_RETURN_DROP;
					}
			}								
#if defined(TCSUPPORT_CT_MULTI_LAN_PD)
			/*ont's dhcp6s/radad server send packet to lan*/
			else if((skb->orig_dev == NULL) && (skb->dev->name[0] == 'e'))
			{
				fdb = get_fdb_by_skb(skb);
				if(fdb == NULL)
				{
					/*
					only check Dhcpv6 response or Router Advertise Packet
					other packets such as IPv4 ARP broadcast  should be  ECNT_CONTINUE (ignore)
					*/
					if ( isDhcpv6ResponsePacket(skb) || isRadvdResponsePacket(skb) )
					{
						if ( 0 != ecnt_get_vlan_and_bindtype_from_skb_inline_hook_ct(skb,&bind_type,&vlan) )
						{
							kfree_skb(skb);
							return ECNT_RETURN_DROP;
						}
							
						for ( idx = 0; idx < MAX_VLAN_GROUP; idx ++ )
						{
							/*not support mulpile lan vlan ->one wan vlan*/
							if(vlan == vBindArray[port_idx-1][idx].wVlanId)
							{
								__vlan_put_tag(skb, vBindArray[port_idx-1][idx].lVlanId);
								return ECNT_CONTINUE;
							}
						}

						if ( bind_type & IF_TYPE_INTERNET )
						{
							return ECNT_CONTINUE;
						}
	
						kfree_skb(skb);
						return ECNT_RETURN_DROP;
					}

					return ECNT_CONTINUE;
				}

				if(fdb->vlan != 0)
					__vlan_put_tag(skb, fdb->vlan);
				if(isDhcpv6ResponsePacket(skb) || isRadvdResponsePacket(skb))
				{
					if(0 != ecnt_get_vlan_and_bindtype_from_skb_inline_hook_ct(skb,&bind_type,&vlan))
					{
						kfree_skb(skb);
						return ECNT_RETURN_DROP;
					}
					/*untag & internet wan*/
					if((fdb->vlan == 0) && (bind_type & IF_TYPE_INTERNET))
					{
						return ECNT_CONTINUE;
					}
					else if(fdb->vlan == 0)
					{
						kfree_skb(skb);
						return ECNT_RETURN_DROP;
					}
					for ( idx = 0; idx < MAX_VLAN_GROUP; idx ++ )
					{
						if ((fdb->vlan == vBindArray[port_idx-1][idx].lVlanId) && \
							(vlan == vBindArray[port_idx-1][idx].wVlanId))
						{
							break;
						}
					}
					if(idx == 0) /*no vlan bind on port,should pass*/
					{
						return ECNT_CONTINUE;
					}
					
					if(idx == MAX_VLAN_GROUP)/*have vlan bind on port, but no match,should drop*/
					{
						kfree_skb(skb);
						return ECNT_RETURN_DROP;
					}
					/*have vlan bind on port and match, pass*/
				}
			}
#endif
		}
	}

	return ECNT_CONTINUE;
}
#endif
static inline int ecnt_portbind_xmit_ct(struct sk_buff *skb)
{
	int ret = 0;
	char dev_name[IFNAMSIZ] = {0};
	struct net_device *dev = NULL;
#if defined(TCSUPPORT_CT_L2TP_VPN) && defined(TCSUPPORT_CMCCV2)
	int ppp_vpn = 0;
#endif
#ifdef TCSUPPORT_XSI_ENABLE
    if((skb->original_dev) && ('x' == skb->original_dev->name[0])){
        return ECNT_CONTINUE;
    }
#endif
#if defined(TCSUPPORT_CT_VLAN_BIND)
	ret = ecnt_vbind_portbindxmit_inline_hook_ct(skb);
	switch ( ret )
	{
		/* vlanband hook, will not go port bind. */
		case ECNT_RETURN:
			return ECNT_CONTINUE;
		/* vlanband hook, drop skb. */
		case ECNT_RETURN_DROP:
			return ECNT_RETURN_DROP;
		default:
			break;
	}
#endif

#if defined(TCSUPPORT_CT_L2TP_VPN) && defined(TCSUPPORT_CMCCV2)
		if ( skb->dev && 'p' == skb->dev->name[0]
			&& 'p' == skb->dev->name[1]
			&& 'p' == skb->dev->name[2]
			&& '1' == skb->dev->name[3]
			&& '0' == skb->dev->name[4]
			&& '0' == skb->dev->name[5] )
			ppp_vpn = 1;
		else if ( skb->orig_dev && 'p' == skb->orig_dev->name[0]
			&& 'p' == skb->orig_dev->name[1]
			&& 'p' == skb->orig_dev->name[2]
			&& '1' == skb->orig_dev->name[3]
			&& '0' == skb->orig_dev->name[4]
			&& '0' == skb->orig_dev->name[5] )
			ppp_vpn = 1;
#endif

	/* packet from cpe, arp etc. let it pass. */
	if (skb->orig_dev == NULL){
		skb->portbind_mark |= MASK_OUT_DEV;
#if defined(TCSUPPORT_CT_MULTI_LAN_PD)
		if(skb->dev->name[0] == 'e' || skb->dev->name[0] == 'r')
		{
			if(isDhcpv6ResponsePacket(skb) || isRadvdResponsePacket(skb))
			{
				snprintf(dev_name,sizeof(dev_name),"nas%d_%d",(skb->mark >> 24)/8,(skb->mark >> 24)%8);
				dev = dev_get_by_name(&init_net,dev_name);
				if(dev == NULL)
				{
					return ECNT_CONTINUE;
				}
				skb->orig_dev = dev;
				if ((portbind_check_hook) && (portbind_check_hook(skb->orig_dev, skb->dev, skb) == 0)){
					dev_put(dev);
					kfree_skb(skb);
					return ECNT_RETURN_DROP;
				}
				skb->orig_dev = NULL;
				dev_put(dev);
			}
		}
#endif
	}
	/*
	** for out interface, if its br0 do not check and let it pass
	** because only smux interface(eg. nas1_0) or 
	** br2684 interface(eg. nas2) or
	** pppx device in bind group
	*/
	else if (skb->dev->name[0] != 'b'
#if defined(TCSUPPORT_CT_DSLITE)
		&& ( '\0' != skb->dev->name[0] && 'd' != skb->dev->name[0] && 's' != skb->dev->name[1] )
#endif
#if defined(TCSUPPORT_CT_L2TP_VPN) && defined(TCSUPPORT_CMCCV2)
		&& ( 1 != ppp_vpn ) /* PPP100 is reserved for L2TP VPN. */
#endif
	){
		if (((skb->portbind_mark & MASK_OUT_DEV) == 0) && (portbind_sw_hook) && (portbind_sw_hook() == 1)) {
			if ((portbind_check_hook) && (portbind_check_hook(skb->orig_dev, skb->dev, skb) == 0)){
				kfree_skb(skb);
				return ECNT_RETURN_DROP;
			}
			else {
				skb->portbind_mark |= MASK_OUT_DEV;
			}
		}
	}	
	return ECNT_CONTINUE;
}
#endif
#endif

#ifdef TCSUPPORT_PORTBIND
#if !defined(TCSUPPORT_CT)
static inline int ecnt_portbind_xmit_inline_hook(struct sk_buff *skb)
{
#ifdef TCSUPPORT_XSI_ENABLE
    if((skb->original_dev) && ('x' == skb->original_dev->name[0])){
        return ECNT_CONTINUE;
    }
#endif
	#if defined(TCSUPPORT_FTP_THROUGHPUT)
			if ((portbind_sw_hook) && ((skb->portbind_mark & MASK_OUT_DEV) == 0)) {
	#else
#if defined(TCSUPPORT_ROUTEPOLICY_PRIOR_PORTBIND)
			if ((portbind_sw_prior_hook) && (portbind_sw_prior_hook(skb) == 1) && ((skb->portbind_mark & MASK_OUT_DEV) == 0)) {
#else
			if ((portbind_sw_hook) && (portbind_sw_hook() == 1) && ((skb->portbind_mark & MASK_OUT_DEV) == 0)) {
	#endif
#endif
			if (portbind_check_hook) {
				/* only need check once. shnwind 20110407 */
				int portbind_ret = 0;
				portbind_ret = portbind_check_hook(skb->orig_dev_name, skb->dev->name);
				if(portbind_ret == 0){
					kfree_skb(skb);
					return ECNT_RETURN_DROP;
				}else if(portbind_ret == 1){
					skb->portbind_mark |= MASK_OUT_DEV; 
				}
#if 0	
				if (portbind_check_hook(skb->orig_dev_name, skb->dev->name) == 0) {
					kfree_skb(skb);
					return ECNT_RETURN_DROP;
				}else if (portbind_check_hook(skb->orig_dev_name, skb->dev->name) == 1) 
					skb->portbind_mark |= MASK_OUT_DEV;
#endif				
				/* else check again */
			}
		}
	

	return ECNT_CONTINUE;


}
#endif
#endif	

#if defined(TCSUPPORT_TEST_SAMBA_SHORTCUT)
extern int SmbdTxSpeedOn;
#endif

static inline int ecnt_dev_queue_xmit_inline_hook(struct sk_buff **pskb)
{
/*!!!If pskb is used as the input parameter of a function, the skb must be reassigned after the function call is finished*/
	struct sk_buff *skb = *pskb; 
	int ret;

#if defined(TCSUPPORT_TEST_SAMBA_SHORTCUT)
	if(SmbdTxSpeedOn && skb->smbd_on_speed){
		return ECNT_CONTINUE;
	}
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,115)
    if(skb->shortcut_on_speed)
    {
        return ECNT_CONTINUE;
    }

	if(!isEN751221)
	{
	    if(skb->sk && skb->sk->sk_shortcut_info.shortcut_sk 
	        && skb->dev && ((!strncmp(skb->dev->name, "eth0.", 5)) || (!strncmp(skb->dev->name, "ra", 2))))
	    {   
	        skb->sk->sk_shortcut_info.out_dev = skb->dev;
	        memcpy(skb->sk->sk_shortcut_info.mac_header, skb->data, ETH_HLEN);
	        memcpy(skb->sk->sk_shortcut_info.ip_header, skb->data + ETH_HLEN, sizeof(struct iphdr));
	        skb->sk->sk_shortcut_info.shortcut_speed = 1;
	    }
	}
#endif
	ECNT_CORE_DEV_HOOK(ECNT_DEV_QUE_XMIT, skb);

#if defined(TCSUPPORT_CT)
#ifdef TCSUPPORT_PORTBIND
	ret = ecnt_portbind_xmit_ct(skb);
	if(ret != ECNT_CONTINUE)
		return ret;

#if defined(TCSUPPORT_CT_VLAN_BIND)
	ret = ecnt_vbind_xmit_inline_hook_ct(skb);
	if(ret != ECNT_CONTINUE)
		return ret;
#endif
#endif
#endif

#if defined(TCSUPPORT_PORTBIND)
#if !defined(TCSUPPORT_CT)
		ret = ecnt_portbind_xmit_inline_hook(skb);
		if(ret != ECNT_CONTINUE)
		{
			return ret;
		}
#endif
#endif

#if defined(TCSUPPORT_PON_VLAN)
/*!!!If pskb is used as the input parameter of a function, the skb must be reassigned after the function call is finished*/
	ret = ecnt_ponvlan_xmit_inline_hook(pskb, skb->dev);
	if(ret != ECNT_CONTINUE)
	{
		return ret;
	}
	skb = *pskb;
#endif

#if defined(TCSUPPORT_VLAN_TAG)
/*!!!If pskb is used as the input parameter of a function, the skb must be reassigned after the function call is finished*/
		ecnt_remark_vlan_pbit_inline_hook(skb);

		ret = ecnt_insert_vlan_tag_inline_hook(pskb, skb->dev);
		if(ret != ECNT_CONTINUE)
		{
			return ret;
		}
		skb = *pskb;
#endif

#if defined(TCSUPPORT_CFG_NG_UNION)
#if defined(TCSUPPORT_CT_VLAN_TAG)
if (check_vtag_ct_hook && (check_vtag_ct_hook() == 1)){
	if((check_vtag_restore_ct_hook && (check_vtag_restore_ct_hook() == 1))
		&&(skb != NULL) && (skb->dev != NULL)
			&& (skb->dev->name[0] == 'e')
			&& restore_vtag_ct_hook){ 	
			if(restore_vtag_ct_hook(&skb) != 0){
				if(skb != NULL){
					kfree_skb(skb);			
				}
				return 0;
			}else{
				*pskb = skb;
			}
		}
}
#endif
#endif

#if defined(TCSUPPORT_CT_VLAN_TAG)
#ifdef TR143
		if (NULL != skb->sk && skb->sk->lPbit <= 7 )
		{
			
			skb->vlan_tag_flag &= (~(0x0F00));
			if ( 0 == skb->sk->lPbit )
				skb->vlan_tag_flag |= (0x0F << 8);
			else {
				skb->vlan_tag_flag |= (skb->sk->lPbit << 8);
			}
		}

#endif
#endif


#if defined(TCSUPPORT_PON_MAC_FILTER)
/*!!!If pskb is used as the input parameter of a function, the skb must be reassigned after the function call is finished*/	
	ret = ecnt_pon_macfilter_xmit_inline_hook(pskb);
	if(ret != ECNT_CONTINUE)
	{
		return ret;
	}
	skb = *pskb;
#endif

#if defined(TCSUPPORT_GPON_DOWNSTREAM_MAPPING)
	ret = ecnt_downstream_mapping_inline_hook(skb);
	if(ret != ECNT_CONTINUE)
	{
		return ret;
	}
#endif

#if defined(TCSUPPORT_CT_DS_LIMIT)
	if ( dslimit_remarkQueue_hook )
	{
		dslimit_remarkQueue_hook(skb, DSLIMIT_DW);
	}	
#endif

#if defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7527)
	if (fe_resource_mark_meter_hook )
	{
		fe_resource_mark_meter_hook(skb, DOWN_STREAM);
	}
	if (fe_resource_mark_acnt_hook )
	{
		fe_resource_mark_acnt_hook(skb, DOWN_STREAM);
	}      
#endif

#if defined(TCSUPPORT_CT_QOS)
	ret = qos_wan_interface_hook(pskb);
	if(ret != ECNT_CONTINUE)
		return ret;
	skb = *pskb;
#endif

	return ECNT_CONTINUE;
}

static inline int ecnt_register_netdevice_inline_hook(struct net_device *dev)
{	
#if defined(TCSUPPORT_CT)
#ifdef TCSUPPORT_PORTBIND
	if (portbind_update_hook){
		portbind_update_hook(dev, ADD_GROUP);
	}
#endif
#endif
	
#if defined(TCSUPPORT_TSO_ENABLE)
	if(!strncmp(dev->name, "eth", 3) ){
		dev->features |= NETIF_F_SG;
		dev->features |= NETIF_F_GSO;
		dev->hw_features |= NETIF_F_SG;
		dev->hw_features |= NETIF_F_GSO;
		dev->wanted_features |= NETIF_F_SG;
		dev->wanted_features |= NETIF_F_GSO;
	}
	if(!strncmp(dev->name, "nas", 3) || !strncmp(dev->name, "pon", 3) || !strncmp(dev->name, "ppp", 3)){
		dev->hw_features |= NETIF_F_SG;
		dev->hw_features |= NETIF_F_GSO;
	}
#endif

	return ECNT_CONTINUE;
}

static inline int ecnt_unregister_netdevice_queue_inline_hook(struct net_device *dev)
{
#if defined(TCSUPPORT_CT)
#ifdef TCSUPPORT_PORTBIND
	if (portbind_update_hook){
		portbind_update_hook(dev, DEL_GROUP);
	}
#endif
#endif

	return ECNT_CONTINUE;
}

static inline int ecnt_neighbour_sysctl_register_inline_hook(struct ctl_table *neigh_var, struct neigh_parms *p, struct net_device *dev)
{
#ifdef TCSUPPORT_IPV6_ENHANCEMENT
	neigh_var = neigh_var+NEIGH_VAR_DEFAULT_ROUTE;
	neigh_var->data = &p->dlf_route[0];	
	neigh_var->extra1 = dev;
	neigh_var->extra2 = p;
#endif
        return ECNT_CONTINUE;
}


static inline int ecnt_netif_hw_stats_inline_hook(struct net_device *dev, struct rtnl_link_stats64 *storage)
{
	if(get_net_device_interface_hw_stats_hook)
	{
		get_net_device_interface_hw_stats_hook(dev, storage);
	}

	return ECNT_CONTINUE;
}
static inline int ecnt_sock_init_data_inline_hook(struct socket *sock, struct sock *sk)
{
#if defined(TCSUPPORT_CT_VLAN_TAG)
#ifdef TR143
			sk->lPbit = 8;
#endif
#endif
return ECNT_CONTINUE;
}

static inline int ecnt_skb_scrub_packet_inline_hook(struct sk_buff *skb)
{
#if defined(TCSUPPORT_VLAN_ACCESS_TRUNK)
	if(skb->dev && (0 == strncmp(skb->dev->name, "veth", 4))){
		return ECNT_RETURN;
	}
#endif

	return ECNT_CONTINUE;
}

static inline int ecnt_sock_setsockopt_inline_hook
(struct sock *sk, int optname, int val, int valbool, int *ret)
{
	switch ( optname )
	{
		case SO_TYPE_TRAFFIC:
			if (!capable(CAP_NET_ADMIN))
				*ret = -EPERM;
			else
			{
				if ( val )
					sk->lPbit |= ( (val & 0x7) << 8 );
				else
					sk->lPbit &= (~TRAFFIC_PROCESS_MASK);
				*ret = 0;
			}
			break;
		case SO_TYPE_COPY_SKB_MARK:
			if (!capable(CAP_NET_ADMIN))
				*ret = -EPERM;
			else
			{
				if ( val )
					sk->lPbit |= ( SOCK_TYPE_COPY_MARK );
				else
					sk->lPbit &= ( ~SOCK_TYPE_COPY_MARK );
				*ret = 0;
			}

			break;
		default:
			break;
	}

	return ECNT_CONTINUE;
}

static inline int ecnt_ppp_flags_init_inline_hook(struct net_device *dev)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,90)
#if defined(TCSUPPORT_CT)	
#if defined(TCSUPPORT_CT_PPP_ONDEMAND)
	if ( NULL != dev )
		dev->ppp_flags = 0;
#endif
#endif
#endif

	return ECNT_CONTINUE;
}

#endif

