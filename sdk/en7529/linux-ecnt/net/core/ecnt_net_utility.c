/***************************************************************
Copyright Statement:

This software/firmware and related documentation (��EcoNet Software��) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (��EcoNet��) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (��ECONET SOFTWARE��) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ��AS IS�� 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER��S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER��S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/

/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,4,90)
#include <linux/proc_fs.h>
#endif
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>
#include <linux/if_pppox.h>
#include <linux/ip.h>
#include <net/ip.h>
#ifdef TCSUPPORT_IPV6
#include <linux/ipv6.h>
#endif
#if defined(TCSUPPORT_CT_VOIP_QOS)
#include <linux/ecnt_voip_proc.h>
#endif
#include <linux/ecnt_vlan_bind.h>
#include <net/addrconf.h>
#include "ecnt_net_core.h"
#include "../ipv6/ecnt_net_ipv6.h"
#if defined(TCSUPPORT_CT_JOYME2)
#include <linux/time.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
#include <spi/spi_nand_flash.h>
#endif
#include <flash_layout/tc_partition.h>
#endif
#include <ecnt_hook/ecnt_hook_fe.h>

#ifdef TCSUPPORT_CPU_ARMV8
#include <asm-generic/irq_regs.h>
#endif

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#define PPP_IP		0x21	/* Internet Protocol */
#define PPP_IPV6	0x57	/* Internet Protocol Version 6 */

/************************************************************************
*                  M A C R O S
*************************************************************************
*/
#define MAX_DEV_BANDWIDTH_NUM 64

/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/
#ifdef TCSUPPORT_DOWNSTREAM_QOS
/*use for set voip rx port in application, shnwind add 20110215.*/
unsigned short int voip_rx_port[VOIP_RX_PORT_NUM] = {0};
EXPORT_SYMBOL(voip_rx_port);
char downstream_qos_enable = 0;
EXPORT_SYMBOL(downstream_qos_enable);
#endif

int remove_proc_flag = 0;

typedef struct devBandwidth_s{
	unsigned char mac[6];
	unsigned long long upBytes;
	unsigned long long downBytes;
	int valid;
}devBandwidth_t;	
typedef struct devBandwidthList_s{	
	int enable;
	int portMultiMacEn;
	struct devBandwidth_s bandwidthList[MAX_DEV_BANDWIDTH_NUM];
}devBandwidthList_t;


/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/


/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************
*/
struct devBandwidthList_s *gHwBandwidthList = NULL;
EXPORT_SYMBOL(gHwBandwidthList);
struct devBandwidthList_s *gDevBandwidthList = NULL;
EXPORT_SYMBOL(gDevBandwidthList);
int wlanledsta = 0;
int wlan11acledsta = 0;
EXPORT_SYMBOL(wlanledsta);
EXPORT_SYMBOL(wlan11acledsta);

int (*match_multicast_vtag_check)
(struct sk_buff *skb, struct net_device *vdev);
EXPORT_SYMBOL(match_multicast_vtag_check);
#if !defined(TCSUPPORT_CT_VLAN_TAG)
int (*match_multicast_vtag)(struct sk_buff *skb, struct net_device *vdev);
EXPORT_SYMBOL(match_multicast_vtag);
#endif
#if defined(TCSUPPORT_CT_VLAN_BIND)
vlanBind_t vBindArray[MAX_LAN_PORT_NUM][MAX_VLAN_GROUP];
#endif
int (*vlanbind_check_group_hook)(struct sk_buff *skb);
EXPORT_SYMBOL(vlanbind_check_group_hook);
int (*wifi_eth_fast_tx_hook)(struct sk_buff *skb);
EXPORT_SYMBOL(wifi_eth_fast_tx_hook);
int (*offload_eth_fast_tx_hook)(struct sk_buff *skb, int channel);
EXPORT_SYMBOL(offload_eth_fast_tx_hook);
int (*xsi_fast_tx_hook)(struct sk_buff *skb) = NULL;
EXPORT_SYMBOL(xsi_fast_tx_hook);
#ifdef TCSUPPORT_TEST_VWTEST
int (*wifi_eth_fast_tx_ecnt_hook)(struct sk_buff *skb)= NULL;
EXPORT_SYMBOL(wifi_eth_fast_tx_ecnt_hook);
#endif
void (*wlan_counter_print_hook)(struct file *fp) = NULL;
EXPORT_SYMBOL(wlan_counter_print_hook);
void (*qdma_err_drop_counters_hook)(unsigned short *qdma_tx_err, unsigned short *qdma_rx_err) = NULL;
EXPORT_SYMBOL(qdma_err_drop_counters_hook);

#if defined(TCSUPPORT_CT_DS_LIMIT)
int (*dslimit_remarkQueue_hook)( struct sk_buff *skb, int up_dw );
EXPORT_SYMBOL(dslimit_remarkQueue_hook);
#endif


int (*fe_resource_mark_meter_hook)( struct sk_buff *skb, int dir );
EXPORT_SYMBOL(fe_resource_mark_meter_hook);
int (*fe_resource_mark_acnt_hook)( struct sk_buff *skb, int dir );
EXPORT_SYMBOL(fe_resource_mark_acnt_hook);

int (*fe_resource_mark_wan_idx_hook)( struct sk_buff *skb, u8 wan_index, u8 dir) = NULL;
EXPORT_SYMBOL(fe_resource_mark_wan_idx_hook);

void (*wlan_to_lan_hook)(struct sk_buff *skb, u8 local) = NULL;
EXPORT_SYMBOL(wlan_to_lan_hook);

/*-------------------sw_rps_for_wifi---------------------------*/
//#ifdef TCSUPPORT_WLAN_SW_RPS
int (*fromWlan5GPktRpsHandle_hook)(void* pRxPacket) = NULL;
EXPORT_SYMBOL(fromWlan5GPktRpsHandle_hook);

int (*toWlan5GPktRpsHandle_hook)(struct sk_buff *skb) = NULL;
EXPORT_SYMBOL(toWlan5GPktRpsHandle_hook);

void (*rps5GQueueDrop_hook)(unsigned int queue, unsigned int *count) = NULL;
EXPORT_SYMBOL(rps5GQueueDrop_hook);

int (*qdma_to_wifi_fast_tx_hook)(struct sk_buff *skb, int index) = NULL;
EXPORT_SYMBOL(qdma_to_wifi_fast_tx_hook);

int (*qdma_to_wifi2g_fast_tx_hook)(struct sk_buff *skb, int index) = NULL;
EXPORT_SYMBOL(qdma_to_wifi2g_fast_tx_hook);

void (*clear_detect_rx_tx_info_hook)(void) = NULL;
EXPORT_SYMBOL(clear_detect_rx_tx_info_hook);

int (*toWlan2GPktRpsHandle_hook)(struct sk_buff *skb) = NULL;
EXPORT_SYMBOL(toWlan2GPktRpsHandle_hook);

int (*fromWlan2GPktRpsHandle_hook)(void* pRxPacket) = NULL;
EXPORT_SYMBOL(fromWlan2GPktRpsHandle_hook);
int (*ecnt_wifi_rx_rps_2g_hook)(struct sk_buff * skb) = NULL;
EXPORT_SYMBOL(ecnt_wifi_rx_rps_2g_hook);

unsigned int (*ecnt_7603_get_ampdu_pkt_hook)(void) = NULL;
EXPORT_SYMBOL(ecnt_7603_get_ampdu_pkt_hook);
unsigned int (*ecnt_7603_get_rx_pkt_hook)(void) = NULL;
EXPORT_SYMBOL(ecnt_7603_get_rx_pkt_hook);

int (*ecnt_wifi_rx_rps_hook)(struct sk_buff * skb) = NULL;
EXPORT_SYMBOL(ecnt_wifi_rx_rps_hook);

int (*ecnt_wifi_tx_rps_hook)(struct sk_buff * skb) = NULL;
EXPORT_SYMBOL(ecnt_wifi_tx_rps_hook);

int (*get_WifitolanRps_hook)(void) = NULL;
EXPORT_SYMBOL(get_WifitolanRps_hook);
int (*get_Wifi2GtolanRps_hook)(void) = NULL;
EXPORT_SYMBOL(get_Wifi2GtolanRps_hook);

int (*get_LantoWifiRps_hook)(void) = NULL;
EXPORT_SYMBOL(get_LantoWifiRps_hook);

int (*ecnt_set_2Gwifi_rps_hook)(int RxOn, int WLanCPU, int TxOn_2G, int LanCPU) = NULL;
EXPORT_SYMBOL(ecnt_set_2Gwifi_rps_hook);

int (*ecnt_set_wifi_rps_hook)(int RxOn, int WLanCPU, int TxOn, int LanCPU) = NULL;
EXPORT_SYMBOL(ecnt_set_wifi_rps_hook);

int (*traffic_process_hook)(struct sk_buff *skb, struct sock *sk, unsigned int *res);
EXPORT_SYMBOL(traffic_process_hook);
int (*dev_bandwidth_hook_get_cnt) (dev_bandwidth_account_t *dev_account) = NULL;
EXPORT_SYMBOL(dev_bandwidth_hook_get_cnt);
int (*dev_bandwidth_hook_clear_cnt) (void) = NULL;
EXPORT_SYMBOL(dev_bandwidth_hook_clear_cnt);

int (*vxlan_left_to_right_handle_hook)(char *data_ptr, unsigned short *data_len, unsigned short foe_index, int *data_offset, unsigned short *VirIfIdx) = NULL;
EXPORT_SYMBOL(vxlan_left_to_right_handle_hook);

int (*vxlan_left_to_right_xmit_hook) (char *data_ptr, unsigned short *data_len, unsigned short foe_index, struct net_device *dev) = NULL;
EXPORT_SYMBOL(vxlan_left_to_right_xmit_hook);

//#endif
/*-------------------sw_rps_for_wifi---------------------------*/


/*-------------------npu_wifi_offload---------------------------*/
#ifdef TCSUPPORT_NPU_WIFI_OFFLOAD
struct sk_buff *(*fromHostadptPktHandle_hook)(unsigned int ringIdx) = NULL;
EXPORT_SYMBOL(fromHostadptPktHandle_hook);
void (*hostdapt_enable_int_hook)(unsigned int ringIdx) = NULL;
EXPORT_SYMBOL(hostdapt_enable_int_hook);
void (*hostdapt_disable_int_hook)(unsigned int ringIdx) = NULL;
EXPORT_SYMBOL(hostdapt_disable_int_hook);
void (*hostdapt_registe_wifitask_hook)(unsigned int ringIdx, void *func) = NULL;
EXPORT_SYMBOL(hostdapt_registe_wifitask_hook);

#endif
/*-------------------npu_wifi_offload---------------------------*/


/*for ALG switch*/
/*0 means switch off; 1 means switch on; 2 means switch not set*/
int nf_conntrack_ftp_enable  __read_mostly = 1;
EXPORT_SYMBOL_GPL(nf_conntrack_ftp_enable);
int nf_conntrack_sip_enable  __read_mostly = 1;
EXPORT_SYMBOL_GPL(nf_conntrack_sip_enable);
int nf_conntrack_h323_enable  __read_mostly = 1;
EXPORT_SYMBOL_GPL(nf_conntrack_h323_enable);
int nf_conntrack_rtsp_enable  __read_mostly = 1;
EXPORT_SYMBOL_GPL(nf_conntrack_rtsp_enable);
int nf_conntrack_l2tp_enable __read_mostly = 2;
EXPORT_SYMBOL_GPL(nf_conntrack_l2tp_enable);
int nf_conntrack_ipsec_enable __read_mostly = 2;
EXPORT_SYMBOL_GPL(nf_conntrack_ipsec_enable);
int nf_conntrack_pptp_enable __read_mostly = 1;
EXPORT_SYMBOL_GPL(nf_conntrack_pptp_enable);
int nf_conntrack_portscan_enable __read_mostly = 0;
EXPORT_SYMBOL_GPL(nf_conntrack_portscan_enable);
int nf_conntrack_ftp_port __read_mostly = 21;
EXPORT_SYMBOL_GPL(nf_conntrack_ftp_port);
int nf_conntrack_esp_timeout __read_mostly = 30;
EXPORT_SYMBOL_GPL(nf_conntrack_esp_timeout);
int nf_conntrack_rtcp_enable  __read_mostly = 1;
EXPORT_SYMBOL_GPL(nf_conntrack_rtcp_enable);
unsigned int nf_conntrack_rtsp_src_ip4_mask __read_mostly = 0xFFFFFFFF;
EXPORT_SYMBOL_GPL(nf_conntrack_rtsp_src_ip4_mask);
int nf_conntrack_tcp_max_session __read_mostly = -1;
EXPORT_SYMBOL_GPL(nf_conntrack_tcp_max_session);
atomic_t nf_conntrack_tcp_session_num;
EXPORT_SYMBOL_GPL(nf_conntrack_tcp_session_num);

struct net_device* (*portbind_get_outdev_by_indev_ct_hook)(unsigned char* indev_name);
EXPORT_SYMBOL(portbind_get_outdev_by_indev_ct_hook);


int (*soft_ratelimit_enqueue_hook) (struct sk_buff * skb,unsigned int queue_idx) = NULL;
EXPORT_SYMBOL(soft_ratelimit_enqueue_hook);

int (*soft_ratelimit_set_queue_hook) (unsigned int queue,unsigned int rate,int(* func)(struct sk_buff * skb)) = NULL;
EXPORT_SYMBOL(soft_ratelimit_set_queue_hook);

int (*soft_ratelimit_allocatequeue_hook)( unsigned int num) = NULL;
EXPORT_SYMBOL(soft_ratelimit_allocatequeue_hook);

int (*soft_ratelimit_recyclequeue_hook)( unsigned int start,unsigned num) = NULL;
EXPORT_SYMBOL(soft_ratelimit_recyclequeue_hook);

int (*soft_cirpir_queue_check_hook) (struct sk_buff * skb,unsigned int queue_idx) = NULL;
EXPORT_SYMBOL(soft_cirpir_queue_check_hook);

int (*soft_cirpir_rate_check_hook) (struct sk_buff * skb,unsigned int queue_idx,int *ret_queue) = NULL;
EXPORT_SYMBOL(soft_cirpir_rate_check_hook);
int (*soft_ratelimit_cds_qdma_hook) (unsigned int idx, struct sk_buff *skb) = NULL;
EXPORT_SYMBOL(soft_ratelimit_cds_qdma_hook);

int (*soft_cirpir_set_cir_queue_hook)( unsigned int queue_idx, unsigned int cirqueue_idx) = NULL;
EXPORT_SYMBOL(soft_cirpir_set_cir_queue_hook);
#if defined(TCSUPPORT_TEST_SAMBA_SHORTCUT)
int SmbdTxSpeedOn = 1;
EXPORT_SYMBOL(SmbdTxSpeedOn);

int SmbdTxSpeedCnt0 = 0;
EXPORT_SYMBOL(SmbdTxSpeedCnt0);

int SmbdTxSpeedCnt1 = 0;
EXPORT_SYMBOL(SmbdTxSpeedCnt1);
#endif
int ShortCutTxSpeedCnt0 = 0;
EXPORT_SYMBOL(ShortCutTxSpeedCnt0);

int ShortCutTxSpeedCnt1 = 0;
EXPORT_SYMBOL(ShortCutTxSpeedCnt1);

unsigned int wlan_thread_cpu_bind_info = 1;
EXPORT_SYMBOL(wlan_thread_cpu_bind_info);

#if defined(TCSUPPORT_CT_JOYME4)
int g_capable_user_root_switch = 0;
EXPORT_SYMBOL(g_capable_user_root_switch);
#endif

int (*soft_qdma_ratelimit_enqueue_hook) (struct sk_buff * skb) = NULL;
EXPORT_SYMBOL(soft_qdma_ratelimit_enqueue_hook);

int (*soft_qdma_ratelimit_dequeue_hook) (struct sk_buff * skb) = NULL;
EXPORT_SYMBOL(soft_qdma_ratelimit_dequeue_hook);

u8 def_mac_addr[6] = {0x00, 0x00, 0xaa, 0xbb, 0xcc, 0xff};
EXPORT_SYMBOL(def_mac_addr);

void (*macSend_hook)(u32 chanId, struct sk_buff *skb) = NULL;
EXPORT_SYMBOL(macSend_hook);

#if defined(TCSUPPORT_TEST_VWTEST) || defined(TCSUPPORT_CMCC_ENTERPRISE)
unsigned int ixia_tx_counter0[3] = {0, 0, 0};
unsigned int ixia_tx_counter1[3] = {0, 0, 0};
int g_Multi_To_One = 0;

EXPORT_SYMBOL(ixia_tx_counter0);
EXPORT_SYMBOL(ixia_tx_counter1);
EXPORT_SYMBOL(g_Multi_To_One);

int forTx2G = 0;
int forTx5G = 0;
int forRx5G = 0;
int Tx_shortcut_level = 5;
int shortGIEnable = 0;
int trafficSkbLen = 1518;
int RxMapLen_now = 0;

EXPORT_SYMBOL(forTx2G);
EXPORT_SYMBOL(forTx5G);
EXPORT_SYMBOL(forRx5G);
EXPORT_SYMBOL(Tx_shortcut_level);
EXPORT_SYMBOL(shortGIEnable);
EXPORT_SYMBOL(trafficSkbLen);
EXPORT_SYMBOL(RxMapLen_now);
unsigned int ecnt_shortcut_cnt[64];
EXPORT_SYMBOL(ecnt_shortcut_cnt);

unsigned char g_Global_MAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
int g_Global_index = -1;
int g_Global_skbLen = 1500;
EXPORT_SYMBOL(g_Global_MAC);
EXPORT_SYMBOL(g_Global_index);
EXPORT_SYMBOL(g_Global_skbLen);

int qmTxLockEnable = 0;
int qmTxLockTryCounter = 0;
EXPORT_SYMBOL(qmTxLockEnable);
EXPORT_SYMBOL(qmTxLockTryCounter);

int drop_KA_Enable = 0;
EXPORT_SYMBOL(drop_KA_Enable);

unsigned int keep_alive_pkt_cnt = 0;
EXPORT_SYMBOL(keep_alive_pkt_cnt);

unsigned int keep_alive_pkt_cnt1[3] = {0, 0, 0};
EXPORT_SYMBOL(keep_alive_pkt_cnt1);

int ecnt_tx_flag = 0;
EXPORT_SYMBOL(ecnt_tx_flag);

struct net_device *rai0_dev = NULL;
EXPORT_SYMBOL(rai0_dev);

struct net_device *ra0_dev = NULL;
EXPORT_SYMBOL(ra0_dev);

int detectRxTxEnable = 0;
EXPORT_SYMBOL(detectRxTxEnable);
#endif

#ifdef TCSUPPORT_CT_DUALWLAN_LED
int isWLanUp = 0;
EXPORT_SYMBOL(isWLanUp);
int isWLan11acUp = 0;
EXPORT_SYMBOL(isWLan11acUp);
int hasWLanClient = 0;
EXPORT_SYMBOL(hasWLanClient);
int hasWLan11acClient = 0;
EXPORT_SYMBOL(hasWLan11acClient);

void (*update_wifi_led_status)(void);
EXPORT_SYMBOL(update_wifi_led_status);
#endif

void* foe_ext_export = NULL;
EXPORT_SYMBOL(foe_ext_export);

/* TCSUPPORT_FORWARD_LEFT_TO_RIGHT */
int aggressive_offload_short_cut_mode = 0;
EXPORT_SYMBOL(aggressive_offload_short_cut_mode);

/* TCSUPPORT_FORWARD_LEFT_TO_RIGHT */
/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

/****************************************************************************
**function name
	 __vlan_proto
**description:
	get protocol via skb
**return 
	eth_type
**parameter:
	skb: the packet information
****************************************************************************/
static inline __be16 __vlan_proto(const struct sk_buff *skb)
{
	return vlan_eth_hdr(skb)->h_vlan_encapsulated_proto;
}

/****************************************************************************
**function name
	 check_ppp_udp_multicast
**description:
	check multicast packet in downstream
**return 
	0:	check ok or ignore
	-1:	fail
**parameter:
	skb: the packet information
	vdev: virtual net device
****************************************************************************/
int check_ppp_udp_multicast
(struct sk_buff *skb, struct net_device *vdev)
{
#if defined(TCSUPPORT_CT_PON_SC)
	struct pppoe_hdr *ppph = NULL;
	struct iphdr *iph = NULL;
	__be16 ppp_proto = 0;
	unsigned char *pppoe_h = NULL, dest_addr[16] = {0};;
	struct ipv6hdr *ip6h = NULL;
	u8 nexthdr = 0;
	int offset = 0, skip_start = 0;
#define VTAG_SUCCESS 0
	__be16 frag_off;

	if ( htons(ETH_P_PPP_SES) == skb->protocol )
		ppph = pppoe_hdr(skb);
	else if( ( htons(ETH_P_8021Q) == skb->protocol )
			&& ( htons(ETH_P_PPP_SES) == __vlan_proto(skb) ) )
		ppph = (struct pppoe_hdr *)(skb_mac_header(skb) + VLAN_ETH_HLEN);
	else
		return 0;

	pppoe_h = (ppph + 1);
	ppp_proto = *(__be16*)(pppoe_h);
	pppoe_h += 2; /* skip ppp protocol */
	memset(dest_addr, 0, sizeof(dest_addr));
	/* IPv4 */
	if ( PPP_IP == ppp_proto )
	{
		iph = (struct iphdr* )(pppoe_h);
		if ( IPPROTO_UDP != iph->protocol )
			return 0;
		memcpy(dest_addr, (unsigned char*)&iph->daddr, 4);
		if ( 0xe0 != (dest_addr[0] & 0xf0) )
			return 0;

		/* drop it when multicast vlanid isn't cofigured correctly. */
		if ( match_multicast_vtag_check
			&& VTAG_SUCCESS != match_multicast_vtag_check(skb, vdev) )
			return -1;
	}
#ifdef TCSUPPORT_IPV6
	else if ( PPP_IPV6 == ppp_proto ) /* IPv6 */
	{
		ip6h = (struct ipv6hdr* )(pppoe_h);
		nexthdr = ip6h->nexthdr;
		offset = ipv6_skip_exthdr(skb, skip_start, &nexthdr, &frag_off);
		if ( offset >= 0 && IPPROTO_UDP == nexthdr )
		{
			memcpy(dest_addr, ip6h->daddr.s6_addr, 16);
	 		if ( 0xff != dest_addr[0] )
				return 0;

			/* drop it when multicast vlanid isn't cofigured correctly. */
			if ( match_multicast_vtag_check
				&& VTAG_SUCCESS != match_multicast_vtag_check(skb, vdev) )
				return -1;
		}
	}
#endif
#endif

	return 0; /* check ok or ignore. */
}
EXPORT_SYMBOL(check_ppp_udp_multicast);

/****************************************************************************
**function name
	 __is_ip_udp
**description:
	check whether packet is IP udp packets.
**return 
	0:	check ok or ignore
	-1:	fail
**parameter:
	skb: the packet information
	vdev: virtual net device
****************************************************************************/
int __is_ip_udp(struct sk_buff *skb)
{
#if defined(TCSUPPORT_CT_PON_SC) || defined(TCSUPPORT_CT)
	struct iphdr *iph = NULL;
#ifdef TCSUPPORT_IPV6
	struct ipv6hdr *ip6h = NULL;
#endif
	u8 nexthdr = 0;
	int offset = 0, skip_start = 0;
	__be16 frag_off;

	if ( htons(ETH_P_IP) == skb->protocol )
		iph = ip_hdr(skb);
	else if( ( htons(ETH_P_8021Q) == skb->protocol )
			&& ( htons(ETH_P_IP) == __vlan_proto(skb) ) )
		iph = (struct iphdr *)(skb_mac_header(skb) + VLAN_ETH_HLEN);
#ifdef TCSUPPORT_IPV6	
	else if ( skb->protocol == htons(ETH_P_IPV6) )
		ip6h = ipv6_hdr(skb);	
	else if( (skb->protocol == htons(ETH_P_8021Q))
			&& (__vlan_proto(skb) == htons(ETH_P_IPV6)) )
		ip6h = (struct iphdr *)(skb_mac_header(skb) + VLAN_ETH_HLEN);
#endif	
	else
		return 0;

	if ( iph && IPPROTO_UDP == iph->protocol )
		return 1;
#ifdef TCSUPPORT_IPV6
	else if (ip6h )
	{
		nexthdr = ip6h->nexthdr;
		offset = ipv6_skip_exthdr(skb, skip_start, &nexthdr, &frag_off);
		if ( offset >= 0 && IPPROTO_UDP == nexthdr )
			return 1;
	}
#endif
	else
		return 0;
#else
	return 0;
#endif
}
EXPORT_SYMBOL(__is_ip_udp);

/****************************************************************************
**function name
	 tr143RxShortCut
**description:
	tr143 test shortcut data path
**return 
	0:	receive succeed.
	-1:	fail
**parameter:
	skb: the packet information
	vlanLayer: vlan layer counts
	ifaceidx: interface index
	iptype: ip type, 1: IP, 2:PPP, 3:dslite+IP, 4:dslite+PPP 
****************************************************************************/
int tr143RxShortCut(int enable
, struct sk_buff *skb
, int vlanLayer
, int ifaceidx
, int iptype
)
{
#if defined(TCSUPPORT_CT_PON_GDV20) || defined(TCSUPPORT_IS_FH_PON)
	struct iphdr *iph = NULL;
	struct net_device *dev = NULL;
	int isPPP = 0, isDslite = 0, isTag = 0;
	char devName[IFNAMSIZ] = {0};
#define IP6_NETWORK_HLEN	40
#define MAX_SMUX_NUM 8


	switch ( iptype )
	{
		case 1: /* IP */
		case 3: /* dslite+IP */
			isDslite = (3 == iptype ? 1 : 0 );
			snprintf(devName, sizeof(devName) - 1, "%snas%d_%d"
				, ( 1 == isDslite ? "ds." : "")
				, ifaceidx / MAX_SMUX_NUM
				, ifaceidx % MAX_SMUX_NUM );
			break;
		case 2: /* PPP */
		case 4: /* dslite+PPP */
			isDslite = (4 == iptype ? 1 : 0 );
			snprintf(devName, sizeof(devName) - 1, "%sppp%d"
				, ( 1 == isDslite ? "ds." : "")
				, ifaceidx );
			isPPP = 1;
			break;
		default:
			return -2;
	}
	dev = dev_get_by_name(&init_net, devName);
	if ( !dev )
		return -3;

	skb->pkt_type = PACKET_HOST;
	skb->protocol = htons(ETH_P_IP);
	skb->ip_summed = CHECKSUM_UNNECESSARY ;

	/* remove vlan tag */
	if ( vlanLayer != 0 )
		skb_pull(skb, 4*vlanLayer);
	skb_reset_network_header(skb);
	skb_reset_transport_header(skb);
	skb->mac_len = skb->network_header - skb->mac_header;

	skb->dev = dev;
	if ( !skb->skb_iif )
		skb->skb_iif = skb->dev->ifindex;

	if ( 2 == enable ) /* debug on */
	{
		printk("\nTR143 shortcut dev=[%s], iptype=[%d] \n", devName, iptype);
	}

	/* remove ppp header */
	if ( isPPP )
	{
		skb_pull(skb, PPPOE_SES_HLEN);
		skb_reset_network_header(skb);
	}

	/* remove ipv6 header */
	if ( isDslite )
	{
		skb_pull(skb, IP6_NETWORK_HLEN);
		skb_reset_network_header(skb);

	}

	/* remove ip header */
	skb_pull(skb, ip_hdrlen(skb));
	skb_reset_transport_header(skb);
	iph = ip_hdr(skb);
	if ( NULL == skb_dst(skb) )
	{
		ip_route_input_noref(skb, iph->daddr, iph->saddr,
								   iph->tos, skb->dev);
	}

	/* Point into the IP datagram, just past the header. */
	tcp_v4_rcv(skb);
	dev_put(dev);
#endif

	return 0;
}
EXPORT_SYMBOL(tr143RxShortCut);

#if (defined(TCSUPPORT_XPON_MAPPING) || defined(TCSUPPORT_PON_VLAN) || defined(TCSUPPORT_XPON_IGMP)) && defined(TCSUPPORT_PON_IP_HOST)
#define br_port_get_rcu(dev) \
	 ((struct net_bridge_port *) rcu_dereference(dev->rx_handler_data))
#define PON_IP_HOST_WANIF_PATH "tc3162/pon_wanIf"

char voip_wanIf[8] = "NULL";
char tr069_wanIf[8] = "nas0_0";

static int pon_wanIf_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	 int len = 0;

	 len = sprintf(buf,"voip %s\n tr069 %s", voip_wanIf, tr069_wanIf );

	 if (len < off + count)
		 *eof = 1;
 
	 len -= off;
	 *start = buf + off;
	 if(len > count)
		 len = count;
	  if(len < 0)
		 len = 0;

	return len;
}
 
static int pon_wanIf_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char wan_type[8] = {0};
	char wan_name[8] = {0};
	char temp[16] = {0};
	
	if (count > 15)
		return -EFAULT;

	if (copy_from_user(temp, buffer, count))
		return -EFAULT;

	sscanf(temp, "%s %s", wan_type, wan_name);
	
	if(memcmp(wan_name, "nas", 3) != 0)
		return -EFAULT;
	
	if(strcmp(wan_type, "voip") == 0)
		strcpy(voip_wanIf, wan_name);
	else if(strcmp(wan_type, "tr069") == 0)
		strcpy(tr069_wanIf, wan_name);
	else
		return -EFAULT;
		
	return count;
}

int isBridgeWan(struct net_device *dev)
{
	 struct net_bridge_port *port;
 
	 if (dev == NULL) 
		 return 0;
 
	 if ((port = br_port_get_rcu(dev)) == NULL)
		 return 0;
	 
	 return 1;
}


/****************************************************************************
**function name
	ecnt_wanIf_proc_init
**description:
	wanIf proc init
**return 
**parameter:
****************************************************************************/
void ecnt_wanIf_proc_init(void)
{
	struct proc_dir_entry *voip_wanIf_proc = NULL;

	voip_wanIf_proc = create_proc_entry(PON_IP_HOST_WANIF_PATH, 0, NULL);
	voip_wanIf_proc->read_proc = pon_wanIf_read_proc;
	voip_wanIf_proc->write_proc = pon_wanIf_write_proc;

	return;
}
/****************************************************************************
**function name
	ecnt_wanIf_proc_deinit
**description:
	wanIf proc destroy
**return 
**parameter:
****************************************************************************/
void ecnt_wanIf_proc_deinit(void)
{
	remove_proc_entry(PON_IP_HOST_WANIF_PATH, NULL);
}
#endif

int isVoipWan(struct net_device *dev){
#if (defined(TCSUPPORT_XPON_MAPPING) || defined(TCSUPPORT_PON_VLAN) || defined(TCSUPPORT_XPON_IGMP)) && defined(TCSUPPORT_PON_IP_HOST)
	if(dev == NULL || isBridgeWan(dev))
		return 0;

#if defined(TCSUPPORT_CT)
	if(dev->name[0] == 'n' && dev->name[3] == voip_wanIf[3] && dev->name[5] == voip_wanIf[5])
#else
	if(dev->name[0] == 'n' && dev->name[3] == voip_wanIf[3])
#endif
		return 1;
#endif	
	return 0;
}

EXPORT_SYMBOL(isVoipWan);

#if defined(TCSUPPORT_CMCC)
int isTR069Wan(struct net_device *dev){
#if (defined(TCSUPPORT_XPON_MAPPING) || defined(TCSUPPORT_PON_VLAN) || defined(TCSUPPORT_XPON_IGMP)) && defined(TCSUPPORT_PON_IP_HOST)
	if(dev == NULL || isBridgeWan(dev))
		return 0;

#if defined(TCSUPPORT_CT)
	if(dev->name[0] == 'n' && dev->name[3] == tr069_wanIf[3] && dev->name[5] == tr069_wanIf[5])
#else
	if(dev->name[0] == 'n' && dev->name[3] == tr069_wanIf[3])
#endif
		return 1;
#endif
	return 0;
}
#endif
#if defined(TCSUPPORT_CT_VLAN_BIND)
int check_vbind_lanvlan(struct sk_buff *skb, unsigned int mark)
{	
	int j = 0;	
	for (j=0; j<MAX_VLAN_GROUP; j++) 
	{		
		if (vBindArray[mark-1][j].lVlanId == 4096) 
		{			
			break;		
		}	   

		if ( VTAG_GET_VID(skb->lan_vlan_tci) == vBindArray[mark-1][j].lVlanId ) 
        {			
			return 1;	   	
		}	
	}	
	return 0;
}

/****************************************************************************
**function name
	 check_vlan_bind
**description:
	check bridge out packets if vlan bind
**return 
	0:	check fail, drop packets.
	1:	check ok
**parameter:
	skb: the packet information
	out_dev: virtual net device
****************************************************************************/
int check_vlan_bind(struct sk_buff *skb, struct net_device *out_dev)
{
	unsigned int mark = 0;
	int pvc_mark = 0, pvc_index = 0, entry_index = 0;
	char ifName[10] = {0};
	int i;

	mark = (skb->mark & 0x7F0000) >> 16;
	if ( mark == 0 )
	{
		/* If no mark, let it go with the internet bridge.*/
		if ((out_dev->bind_type & IF_TYPE_INTERNET) == IF_TYPE_INTERNET)
		{
			return 1;
		}
	}
	else
	{
		pvc_mark = (mark - 1)/MAX_PVC_NUM;
		sscanf(out_dev->name, "nas%d_%d", &pvc_index, &entry_index); 
#if defined(TCSUPPORT_CMCC)
		if ( (mark - 1) == (pvc_index * MAX_SMUX_NUM + entry_index) )
#else
		if ( pvc_mark == pvc_index )
#endif
		{
			return 1;
		}
	}
	return 0;
}
/****************************************************************************
**function name
	vbind_entry_array_read_proc
**description:
	vlan bind data info proc read
**return 
**parameter:
****************************************************************************/
static int vbind_entry_array_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len = 0;
	int i = 0, j = 0;

	for (i = 0; i < MAX_LAN_PORT_NUM; i ++) {
		if(vBindArray[i][0].lVlanId != 4096)
			len += sprintf(page+len,"LAN PORT %d: \n", i+1);
		for (j = 0; j < MAX_VLAN_GROUP; j ++) {
			if(vBindArray[i][j].lVlanId != 4096){
				len += sprintf(page+len,"%d/%d  ", 
					(int)(vBindArray[i][j].lVlanId),
					(int)(vBindArray[i][j].wVlanId));
			}
		}
		len += sprintf(page+len,"\n");
	}

	len -= off;
	*start = page + off;

	if (len > count)
		len = count;
	else
		*eof = 1;

	if (len < 0)
		len = 0;

	return len;
}
/****************************************************************************
**function name
	vbind_entry_array_read_proc
**description:
	vlan bind data info proc write
**return 
**parameter:
****************************************************************************/
static int vbind_entry_array_write_proc(struct file *file
, const char *buffer, unsigned long count, void *data)
{
	char val_string[64] = {0};
	int i = 0, j = 0;
	int lVlanId = 0, wVlanId = 0;
	vlanBind_t vlanBind = {4096,4096};

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';
	if (strstr(val_string, "reset")) {
		for ( i=0; i<MAX_LAN_PORT_NUM; i++) {
			for ( j=0; j<MAX_VLAN_GROUP; j++) {
				vBindArray[i][j] = vlanBind;
			}
		}
		return count;
	}

	sscanf(val_string, "%d %d %d %d", &i, &j, &lVlanId, &wVlanId);
	vlanBind.lVlanId = (u16)lVlanId;
	vlanBind.wVlanId = (u16)wVlanId;
	vBindArray[i][j] = vlanBind;
	return count;
}
/****************************************************************************
**function name
	ecnt_vlan_bind_proc_init
**description:
	vlan bind data info proc init
**return 
**parameter:
****************************************************************************/
void ecnt_vlan_bind_proc_init(void)
{
	struct proc_dir_entry *vbind_proc = NULL;

	vbind_proc = create_proc_entry(VBIND_ENTRY_ARRAY_PATH, 0, NULL);
	vbind_proc->read_proc = vbind_entry_array_read_proc;
	vbind_proc->write_proc = vbind_entry_array_write_proc;	

	return;
}
/****************************************************************************
**function name
	ecnt_vlan_bind_proc_deinit
**description:
	vlan bind data info proc destroy
**return 
**parameter:
****************************************************************************/
void ecnt_vlan_bind_proc_deinit(void)
{
	remove_proc_entry(VBIND_ENTRY_ARRAY_PATH, NULL);
}
#endif

#if defined(TCSUPPORT_TEST_SAMBA_SHORTCUT)
/****************************************************************************
**function name
	samba_shortcut_read_proc
**description:
	samba_shortcut_read_proc
**return 
**parameter:
****************************************************************************/
static int samba_shortcut_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len = 0;

	len += sprintf(page+len,"SmbdTxSpeedOn: %d \n", SmbdTxSpeedOn);
	len += sprintf(page+len,"SmbdTxSpeedCnt0: %d \n", SmbdTxSpeedCnt0);
	len += sprintf(page+len,"SmbdTxSpeedCnt1: %d \n", SmbdTxSpeedCnt1);

	len -= off;
	*start = page + off;

	if (len > count)
		len = count;
	else
		*eof = 1;

	if (len < 0)
		len = 0;

	return len;
}
/****************************************************************************
**function name
	samba_shortcut_write_proc
**description:
	samba_shortcut_write_proc
**return 
**parameter:
****************************************************************************/
static int samba_shortcut_write_proc(struct file *file
, const char *buffer, unsigned long count, void *data)
{
	char val_string[64] = {0};
	int enable_tmp = 0;

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';
	if (strstr(val_string, "reset")) {
		SmbdTxSpeedCnt0 = 0;
		SmbdTxSpeedCnt1 = 0;
		return count;
	}

	sscanf(val_string, "%d", &enable_tmp);
	if((enable_tmp == 0) || (enable_tmp == 1))
		SmbdTxSpeedOn = enable_tmp;
	else
		printk("\nerr value\n");
	
	return count;
}
/****************************************************************************
**function name
	ecnt_samba_shortcut_proc_init
**description:
	ecnt_samba_shortcut_proc_init
**return 
**parameter:
****************************************************************************/
void ecnt_samba_shortcut_proc_init(void)
{
	struct proc_dir_entry *samba_shortcut_proc = NULL;

	samba_shortcut_proc = create_proc_entry(SAMBA_SHORTCUT_PATH, 0, NULL);
	samba_shortcut_proc->read_proc = samba_shortcut_read_proc;
	samba_shortcut_proc->write_proc = samba_shortcut_write_proc;	

	return;
}
/****************************************************************************
**function name
	ecnt_samba_shortcut_proc_deinit
**description:
	ecnt_samba_shortcut_proc_deinit
**return 
**parameter:
****************************************************************************/
void ecnt_samba_shortcut_proc_deinit(void)
{
	remove_proc_entry(SAMBA_SHORTCUT_PATH, NULL);
}
#endif

char ecnt_shortcut_apps[APP_SHORTCUT_MAX_NUM][32] = {0};

/****************************************************************************
**function name
	ecnt_shortcut_app_list_check
**description:
	ecnt_shortcut_app_list_check
**return
**parameter:
****************************************************************************/

int ecnt_shortcut_app_list_check(char * name)
{
    int i;

    for (i = 0; i < APP_SHORTCUT_MAX_NUM; i++)
    {
        if (!strcmp(ecnt_shortcut_apps[i], name))
            break;
    }

    if (i < APP_SHORTCUT_MAX_NUM)
    {
        return 1;
    }

    return 0;
}

/****************************************************************************
**function name
	ecnt_shortcut_app_list_add
**description:
	ecnt_shortcut_app_list_add
**return
**parameter:
****************************************************************************/

int ecnt_shortcut_app_list_add(char * name)
{
    int i, pos = APP_SHORTCUT_MAX_NUM;

    for (i = 0; i < APP_SHORTCUT_MAX_NUM; i++)
    {
        if (!strcmp(ecnt_shortcut_apps[i], name))
        {
            return 1;
        }
		if ((ecnt_shortcut_apps[i][0] == '\0') && (pos == APP_SHORTCUT_MAX_NUM))
        {
            pos = i;
			break;
        }
    }

    if (pos < APP_SHORTCUT_MAX_NUM)
    {
        strncpy(ecnt_shortcut_apps[pos], name, 31);
        ecnt_shortcut_apps[pos][31] = '\0';
        return 1;
    }

    return 0;
}

/****************************************************************************
**function name
	ecnt_shortcut_app_list_del
**description:
	ecnt_shortcut_app_list_del
**return
**parameter:
****************************************************************************/

int ecnt_shortcut_app_list_del(char * name)
{
    int i;

    for (i = 0; i < APP_SHORTCUT_MAX_NUM; i++)
    {
        if (!strcmp(ecnt_shortcut_apps[i], name))
            break;
    }

    if (i < APP_SHORTCUT_MAX_NUM)
    {
        ecnt_shortcut_apps[i][0] = '\0';
        return 1;
    }

    return 0;
}

/****************************************************************************
**function name
	ecnt_app_shortcut_read_proc
**description:
	ecnt_app_shortcut_read_proc
**return
**parameter:
****************************************************************************/

static int ecnt_app_shortcut_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0, i;

	len += sprintf(page+len,"ShortCutApps: ");
    for (i = 0; i < APP_SHORTCUT_MAX_NUM; i++)
    {
        len += sprintf(page+len,"%s ", ecnt_shortcut_apps[i]);
    }
	len += sprintf(page+len,"\n");
	len += sprintf(page+len,"ShortCutTxSpeedCnt0: %d \n", ShortCutTxSpeedCnt0);
	len += sprintf(page+len,"ShortCutTxSpeedCnt1: %d \n", ShortCutTxSpeedCnt1);

	len -= off;
	*start = page + off;

	if (len > count)
		len = count;
	else
		*eof = 1;

	if (len < 0)
		len = 0;

	return len;
}

/****************************************************************************
**function name
	ecnt_app_shortcut_write_proc
**description:
	ecnt_app_shortcut_write_proc
**return
**parameter:
****************************************************************************/

static int ecnt_app_shortcut_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char val_string[32] = {0};
	char name[32] = {0};
	int flag = 0;

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';
	if (strstr(val_string, "reset"))
    {
		ShortCutTxSpeedCnt0 = 0;
		ShortCutTxSpeedCnt1 = 0;
		return count;
	}

	sscanf(val_string, "%d %s", &flag, name);
    if (flag)
    {
        ecnt_shortcut_app_list_add(name);
    }
    else
    {
        ecnt_shortcut_app_list_del(name);
    }
	return count;
}

/****************************************************************************
**function name
	ecnt_app_shortcut_proc_init
**description:
	ecnt_app_shortcut_proc_init
**return
**parameter:
****************************************************************************/

void ecnt_app_shortcut_proc_init(void)
{
	struct proc_dir_entry *app_shortcut_proc = NULL;

	app_shortcut_proc = create_proc_entry(APP_SHORTCUT_PATH, 0, NULL);
	app_shortcut_proc->read_proc = ecnt_app_shortcut_read_proc;
	app_shortcut_proc->write_proc = ecnt_app_shortcut_write_proc;

	return;
}

/****************************************************************************
**function name
	ecnt_app_shortcut_proc_deinit
**description:
	ecnt_app_shortcut_proc_deinit
**return
**parameter:
****************************************************************************/

void ecnt_app_shortcut_proc_deinit(void)
{
	remove_proc_entry(APP_SHORTCUT_PATH, NULL);
}



/****************************************************************************
**function name
	ecnt_wlan_cpu_bind_read_proc
**description:
	ecnt_wlan_cpu_bind_read_proc
**return
**parameter:
****************************************************************************/

static int ecnt_wlan_cpu_bind_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0, i;

	len += sprintf(page+len,"WlanCpuBind: ");
	len += sprintf(page+len,"%d ", wlan_thread_cpu_bind_info);
	len += sprintf(page+len,"\n");

	len -= off;
	*start = page + off;

	if (len > count)
		len = count;
	else
		*eof = 1;

	if (len < 0)
		len = 0;

	return len;
}

/****************************************************************************
**function name
	ecnt_wlan_cpu_bind_write_proc
**description:
	ecnt_wlan_cpu_bind_write_proc
**return
**parameter:
****************************************************************************/

static int ecnt_wlan_cpu_bind_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char val_string[32] = {0};
	char name[32] = {0};
	int flag = 0;

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';

	sscanf(val_string, "%d", &wlan_thread_cpu_bind_info);

	return count;
}

/****************************************************************************
**function name
	ecnt_wlan_cpu_bind_proc_init
**description:
	ecnt_wlan_cpu_bind_proc_init
**return
**parameter:
****************************************************************************/

void ecnt_wlan_cpu_bind_proc_init(void)
{
	struct proc_dir_entry *app_shortcut_proc = NULL;

	app_shortcut_proc = create_proc_entry(WLAN_CPU_BIND_PATH, 0, NULL);
	app_shortcut_proc->read_proc = ecnt_wlan_cpu_bind_read_proc;
	app_shortcut_proc->write_proc = ecnt_wlan_cpu_bind_write_proc;

	return;
}

/****************************************************************************
**function name
	ecnt_wlan_cpu_bind_proc_deinit
**description:
	ecnt_wlan_cpu_bind_proc_deinit
**return
**parameter:
****************************************************************************/

void ecnt_wlan_cpu_bind_proc_deinit(void)
{
	remove_proc_entry(WLAN_CPU_BIND_PATH, NULL);
}

#if defined(TCSUPPORT_CT_JOYME4)
/****************************************************************************
**function name
	capable_user_root_switch_read_proc
**description:
	capable_user_root_switch_read_proc
**return
**parameter:
****************************************************************************/
static int capable_user_root_switch_read_proc(char *buf, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len = 0;

	len += sprintf(buf+len, "%d\n", g_capable_user_root_switch);

	len -= off;
	*start = buf + off;

	if (len > count)
		len = count;
	else
		*eof = 1;

	if (len < 0)
		len = 0;

	return len;
}
/****************************************************************************
**function name
	capable_user_root_switch_write_proc
**description:
	capable_user_root_switch_write_proc
**return
**parameter:
****************************************************************************/
static int capable_user_root_switch_write_proc(struct file *file
, const char *buffer, unsigned long count, void *data)
{
	int len;
	char get_buf[32];

	/* do a range checking, don't overflow buffers in kernel modules */
	if(count > 32)
		len = 32;
	else
		len = count;
	
	if(copy_from_user(get_buf, buffer, len))
		return -EFAULT;
	
	get_buf[len]='\0';
	
	sscanf(get_buf, "%d", &g_capable_user_root_switch);
	
	return len;

}
/****************************************************************************
**function name
	ecnt_capable_user_root_switch_proc_init
**description:
	ecnt_capable_user_root_switch_proc_init
**return
**parameter:
****************************************************************************/
void ecnt_capable_user_root_switch_proc_init(void)
{
	struct proc_dir_entry *capable_user_root_switch_proc = NULL;

	capable_user_root_switch_proc = create_proc_entry(CAPABLE_USER_ROOT_SWITCH_PATH, 0, NULL);
	capable_user_root_switch_proc->read_proc = capable_user_root_switch_read_proc;
	capable_user_root_switch_proc->write_proc = capable_user_root_switch_write_proc;

	return;
}
/****************************************************************************
**function name
	ecnt_capable_user_root_switch_proc_deinit
**description:
	ecnt_capable_user_root_switch_proc_deinit
**return
**parameter:
****************************************************************************/
void ecnt_capable_user_root_switch_proc_deinit(void)
{
	remove_proc_entry(CAPABLE_USER_ROOT_SWITCH_PATH, NULL);
}
#endif


/****************************************************************************
**function name
	dbg_msg_read_proc
**description:
	dbg_msg_read_proc
**return 
**parameter:
****************************************************************************/
static int dbg_msg_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	return 0;
}
/****************************************************************************
**function name
	dbg_msg_write_proc
**description:
	dbg_msg_write_proc
**return 
**parameter:
****************************************************************************/
static int dbg_msg_write_proc(struct file *file
, const char *buffer, unsigned long count, void *data)
{
	char val_string[1024] = {0};

	if (count > sizeof(val_string) - 1) 
	{
		return -EINVAL;
	}

	if (copy_from_user(val_string, buffer, count)) 
	{
		return -EFAULT;
	}

	val_string[count] = '\0';
	printk(val_string);

	return count;
}
/****************************************************************************
**function name
	ecnt_dbg_msg_proc_init
**description:
	ecnt_dbg_msg_proc_init
**return 
**parameter:
****************************************************************************/
void ecnt_dbg_msg_proc_init(void)
{
	struct proc_dir_entry *dbg_msg_proc = NULL;

	dbg_msg_proc = create_proc_entry(DBG_MSG_PATH, 0, NULL);
	dbg_msg_proc->read_proc = dbg_msg_read_proc;
	dbg_msg_proc->write_proc = dbg_msg_write_proc;	

	return;
}
/****************************************************************************
**function name
	ecnt_dbg_msg_proc_deinit
**description:
	ecnt_dbg_msg_proc_deinit
**return 
**parameter:
****************************************************************************/
void ecnt_dbg_msg_proc_deinit(void)
{
	remove_proc_entry(DBG_MSG_PATH, NULL);
}

void ecnt_netdev_init_hook(void)
{
	if (remove_proc_flag == 1)
		return;
#if defined(TCSUPPORT_CT_VOIP_QOS)
	ecnt_voip_qos_proc_init();
#endif

#if defined(TCSUPPORT_CT_VLAN_BIND)
	ecnt_vlan_bind_proc_init();
#endif

#if defined(TCSUPPORT_TEST_SAMBA_SHORTCUT)
	ecnt_samba_shortcut_proc_init();
#endif
	ecnt_app_shortcut_proc_init();
	ecnt_wlan_cpu_bind_proc_init();
#if defined(TCSUPPORT_CT_JOYME4)
	ecnt_capable_user_root_switch_proc_init();
#endif

#if (defined(TCSUPPORT_XPON_MAPPING) || defined(TCSUPPORT_PON_VLAN) || defined(TCSUPPORT_XPON_IGMP)) && defined(TCSUPPORT_PON_IP_HOST)
	ecnt_wanIf_proc_init();
#endif

	ecnt_dbg_msg_proc_init();

	remove_proc_flag = 1;
}


void ecnt_netdev_dest_hook(void)
{
	/* only remove one time */
	if (remove_proc_flag == 0)
		return;
#if defined(TCSUPPORT_CT_VOIP_QOS)
	ecnt_voip_qos_proc_dest();
#endif

#if defined(TCSUPPORT_CT_VLAN_BIND)
	ecnt_vlan_bind_proc_deinit();
#endif

#if defined(TCSUPPORT_TEST_SAMBA_SHORTCUT)
	ecnt_samba_shortcut_proc_deinit();
#endif
	ecnt_app_shortcut_proc_deinit();
	ecnt_wlan_cpu_bind_proc_deinit();
#if defined(TCSUPPORT_CT_JOYME4)
	ecnt_capable_user_root_switch_proc_deinit();
#endif

#if (defined(TCSUPPORT_XPON_MAPPING) || defined(TCSUPPORT_PON_VLAN) || defined(TCSUPPORT_XPON_IGMP)) && defined(TCSUPPORT_PON_IP_HOST)
	ecnt_wanIf_proc_deinit();
#endif
	ecnt_dbg_msg_proc_deinit();

	remove_proc_flag = 0;
}

#if defined(TCSUPPORT_CT_WAN_CHILD_PREFIX)
#define IN6ADDRSZ sizeof(struct in6_addr)
#define INADDRSZ 4
#define INT16SZ 2

/* int
 * inet_pton6(src, dst)
 *  convert presentation level address to network order binary form.
 * return:
 *  1 if `src' is a valid [RFC1884 2.2] address, else 0.
 * notice:
 *  (1) does not touch `dst' unless it's returning 1.
 *  (2) :: in a full address is silently ignored.
 * credit:
 *  inspired by Mark Andrews.
 * author:
 *  Paul Vixie, 1996.
 */
int
inet_pton6(
    const char *src,
    u_char *dst
)
{
    static const char xdigits_l[] = "0123456789abcdef",
              xdigits_u[] = "0123456789ABCDEF";
    u_char tmp[IN6ADDRSZ], *tp, *endp, *colonp;
    const char *xdigits, *curtok;
    int ch, saw_xdigit;
    unsigned val;
	
    memset((tp = tmp), 0, IN6ADDRSZ);
    endp = tp + IN6ADDRSZ;
    colonp = NULL;
    /* Leading :: requires some special handling. */
    if (*src == ':')
        if (*++src != ':')
            return (0);
    curtok = src;
    saw_xdigit = 0;
    val = 0;
    while ((ch = *src++) != '\0') {
        const char *pch;
 
        if ((pch = (char *) strchr((xdigits = xdigits_l), ch)) == NULL)
            pch = (char *) strchr((xdigits = xdigits_u), ch);
        if (pch != NULL) {
            val <<= 4;
            val |= (pch - xdigits);
            if (val > 0xffff)
                return (0);
            saw_xdigit = 1;
            continue;
        }
        if (ch == ':') {
            curtok = src;
            if (!saw_xdigit) {
                if (colonp)
                    return (0);
                colonp = tp;
                continue;
            }
            if (tp + INT16SZ > endp)
                return (0);
            *tp++ = (u_char) (val >> 8) & 0xff;
            *tp++ = (u_char) val & 0xff;
            saw_xdigit = 0;
            val = 0;
            continue;
        }
	#if 0
        if (ch == '.' && ((tp + INADDRSZ) <= endp) &&
            inet_pton4(curtok, tp) > 0) {
            tp += INADDRSZ;
            saw_xdigit = 0;
            break;  /* '\0' was seen by inet_pton4(). */
        }
	#endif
        return (0);
    }
    if (saw_xdigit) {
        if (tp + INT16SZ > endp)
            return (0);
        *tp++ = (u_char) (val >> 8) & 0xff;
        *tp++ = (u_char) val & 0xff;
    }
    if (colonp != NULL) {
        /*
         * Since some memmove()'s erroneously fail to handle
         * overlapping regions, we'll do the shift by hand.
         */
        const int n = tp - colonp;
        int i;
 
        for (i = 1; i <= n; i++) {
            endp[- i] = colonp[n - i];
            colonp[n - i] = 0;
        }
        tp = endp;
    }
    if (tp != endp)
        return (0);
    /* bcopy(tmp, dst, IN6ADDRSZ); */
    memcpy(dst, tmp, IN6ADDRSZ);

	return (1);
}

/*******************************************************************************************
**function name
	generate_prefix
**description:
	according the parent PD and childprefixbits in tr069, generate new prefix.
 **retrun 
 	0:success
 	-1:failure
**parameter:
	skb: packet buffer
	pinfo_ptr: prefix infor pointer
********************************************************************************************/
int generate_prefix(struct sk_buff *skb ,struct prefix_info * pinfo_ptr){
	int ret = -1;
	struct inet6_dev *in6_dev = NULL;
	struct in6_addr		parent_prefix;
	struct in6_addr		child_prefix;
	u8	temp_u8 = 0;
	int start_mask = 0, end_mask = 0, mask_range = 0;
	int in6_addr_len = sizeof(struct in6_addr);
	int i = 0, j = 0, k = 0;
	char * tmp_p=NULL;
	char * tmp2_p=NULL;
	char * tmp3_p=NULL;
	char tmpLen_str[40] = {0};
	char parent_prefix_str[64] = {0};
	char child_prefix_str[64] = {0};
	int child_prefix_len = 0, parent_prefix_len = 0;
	
	/*init*/
	if(skb == NULL || pinfo_ptr == NULL){
		goto end;
	}
	in6_dev = in6_dev_get(skb->dev);
	//in6_dev = __in6_dev_get(skb->dev);
	
	if (in6_dev == NULL || in6_dev->dev == NULL) {		
		goto end;
	}
	if(!is_wan_dev(in6_dev->dev)){	
		goto end;
	}
	memset(&(pinfo_ptr->prefix), 0, in6_addr_len);
	pinfo_ptr->prefix_len = 0 ;
	memset(&(parent_prefix), 0, in6_addr_len);
	memset(&(child_prefix), 0, in6_addr_len);
	memcpy(parent_prefix_str, in6_dev->cnf.parent_pd_prefix, sizeof(parent_prefix_str));
	memcpy(child_prefix_str, in6_dev->cnf.child_prefix, sizeof(child_prefix_str));
	if(in6_dev->cnf.child_prefix_orign != PREFIX_ORIGN_SLLA){
		goto end;
	}

	if(child_prefix_str[0] == '\0'){
		goto end;
	}
	/*get child prefix len*/
	tmp_p = strstr(child_prefix_str, "/");
	if(tmp_p == NULL){
		goto end;
	}
	
	tmp2_p = tmp_p+1;
	child_prefix_len = simple_strtoul(tmp2_p, NULL, 10);
	*tmp_p = '\0';

	if(parent_prefix_str[0] == '\0'){
		goto end;
	}
	tmp_p = strstr(parent_prefix_str, "/");
	if(tmp_p == NULL){
		goto end;
	}
	
	tmp2_p = tmp_p+1;
	parent_prefix_len = simple_strtoul(tmp2_p, NULL, 10);
	*tmp_p = '\0';
	if(parent_prefix_len >= MAX_PD_PREFIX){
		goto end;
	}
	/*convert ipv6 string to in6_addr type*/
	if(inet_pton6(parent_prefix_str, parent_prefix.s6_addr) != 1){
		goto end;
	}
	if(inet_pton6(child_prefix_str, child_prefix.s6_addr) != 1){
		goto end;
	}

	start_mask = parent_prefix_len;
	end_mask = child_prefix_len;
	mask_range = end_mask - start_mask;
	if(mask_range <= 0 ){
		if(mask_range == 0){
			memcpy(&(pinfo_ptr->prefix), &(parent_prefix),in6_addr_len);
			pinfo_ptr->prefix_len = parent_prefix_len;
			ret = 0;
		}
		goto end;
	}
	/*calculate new prefix based on childprefix and parentprefix*/
	for(i=start_mask; i<end_mask; i++){
		j = i/8;
		k = i%8;
		temp_u8 = (1<<k);
		temp_u8 &= child_prefix.s6_addr[j];
		parent_prefix.s6_addr[j] |= temp_u8;
	}

	memcpy(&(pinfo_ptr->prefix), &(parent_prefix),in6_addr_len);	
	pinfo_ptr->prefix_len = child_prefix_len;
	
	ret = 0;
	
end:
	if(ret == 0){
		if(pinfo_ptr->prefix_len>64){
			pinfo_ptr->prefix_len = 64;
		}
	}
	return ret;
}
#endif

/****************************************************************************
**function name
	 __is_igmp
**description:
	check whether packet is igmp packets.
**return 
	0:	match fail
	1:	match ok
**parameter:
	skb: the packet information
	vdev: virtual net device
****************************************************************************/
int __is_igmp(struct sk_buff *skb)
{
#if defined(TCSUPPORT_CMCC)
	struct iphdr *iph = NULL;
#ifdef TCSUPPORT_IPV6
	struct ipv6hdr *ip6h = NULL;
#endif
	struct pppoe_hdr *ppph = NULL;
	u8 nexthdr = 0;
	int offset = 0, skip_start = 0;
	unsigned char *pppoe_h = NULL;
	__be16 ppp_proto = 0;
	__be16 frag_off;

	if ( !skb )
		return 0;

	if ( htons(ETH_P_IP) == skb->protocol )
		iph = ip_hdr(skb);
	else if( ( htons(ETH_P_8021Q) == skb->protocol )
			&& ( htons(ETH_P_IP) == __vlan_proto(skb) ) )
		iph = (struct iphdr *)(skb_mac_header(skb) + VLAN_ETH_HLEN);
#ifdef TCSUPPORT_IPV6	
	else if ( skb->protocol == htons(ETH_P_IPV6) )
	{   
		ip6h = ipv6_hdr(skb);	
		skip_start = sizeof(*ip6h) + ETH_HLEN;
	}
	else if( (skb->protocol == htons(ETH_P_8021Q))
			&& (__vlan_proto(skb) == htons(ETH_P_IPV6)) )
	{   
		ip6h = (struct iphdr *)(skb_mac_header(skb) + VLAN_ETH_HLEN);
		skip_start = sizeof(*ip6h) + VLAN_HLEN + ETH_HLEN;
	}
#endif
	else if ( htons(ETH_P_PPP_SES) == skb->protocol )
		ppph = pppoe_hdr(skb);
	else if( ( htons(ETH_P_8021Q) == skb->protocol )
			&& ( htons(ETH_P_PPP_SES) == __vlan_proto(skb) ) )
		ppph = (struct pppoe_hdr *)(skb_mac_header(skb) + VLAN_ETH_HLEN);
	else
		return 0;

	if ( iph && IPPROTO_IGMP == iph->protocol )
		return 1;
#ifdef TCSUPPORT_IPV6
	else if (ip6h )
	{
		nexthdr = ip6h->nexthdr;
		offset = ipv6_skip_exthdr(skb, skip_start, &nexthdr, &frag_off);
		if ( (offset >= 0) && (IPPROTO_ICMPV6 == nexthdr) )
			return 1;
	}
#endif
	else if ( ppph )
	{
		pppoe_h = (ppph + 1);
		ppp_proto = *(__be16*)(pppoe_h);
		pppoe_h += 2; /* skip ppp protocol */
		/* IPv4 */
		if ( IPPROTO_IGMP == ppp_proto )
		{
			return 1;
		}
#ifdef TCSUPPORT_IPV6
		else if ( PPP_IPV6 == ppp_proto ) /* IPv6 */
		{
			ip6h = (struct ipv6hdr* )(pppoe_h);
			nexthdr = ip6h->nexthdr;
			offset = ipv6_skip_exthdr(skb, skip_start, &nexthdr, &frag_off);
			if ( offset >= 0 && IPPROTO_ICMPV6 == nexthdr )
			{
				return 1;
			}
		}
#endif
	}
	else
		return 0;
#endif
	return 0;

}
EXPORT_SYMBOL(__is_igmp);

/****************************************************************************
**function name
	 __is_udp_multicast
**description:
	check multicast packet in downstream
**return 
	0:	match fail
	1:	match ok
**parameter:
	skb: the packet information
	vdev: virtual net device
****************************************************************************/
int __is_udp_multicast(struct sk_buff *skb)
{
#if defined(TCSUPPORT_CMCC)
	struct pppoe_hdr *ppph = NULL;
	struct iphdr *iph = NULL;
	__be16 ppp_proto = 0;
	unsigned char *pppoe_h = NULL, dest_addr[16] = {0};;
#ifdef TCSUPPORT_IPV6
	struct ipv6hdr *ip6h = NULL;
#endif
	u8 nexthdr = 0;
	int offset = 0, skip_start = 0;
	unsigned char *dstAddr = NULL;
	__be16 frag_off;

	if ( !skb )
		return 0;

	dstAddr = eth_hdr(skb)->h_dest;

	if ( dstAddr && 
		(htons(ETH_P_IP) == skb->protocol
		|| ((htons(ETH_P_8021Q) == skb->protocol)
			&& ( htons(ETH_P_IP) == __vlan_proto(skb)))
#ifdef TCSUPPORT_IPV6	
			|| (htons(ETH_P_IPV6) == skb->protocol)
			|| ((htons(ETH_P_8021Q) == skb->protocol)
				&& (htons(ETH_P_IPV6) == __vlan_proto(skb)))
#endif
			)
		&& ((dstAddr[0] & 1)
		&& ((dstAddr[0] & dstAddr[1] & dstAddr[2] 
		& dstAddr[3] & dstAddr[4] & dstAddr[5]) != 0xff)) )
	{
		return 1;
	}
	else if ( htons(ETH_P_PPP_SES) == skb->protocol )
		ppph = pppoe_hdr(skb);
	else if( ( htons(ETH_P_8021Q) == skb->protocol )
			&& ( htons(ETH_P_PPP_SES) == __vlan_proto(skb) ) )
		ppph = (struct pppoe_hdr *)(skb_mac_header(skb) + VLAN_ETH_HLEN);
	else
		return 0;

	if ( ppph )
	{
		pppoe_h = (ppph + 1);
		ppp_proto = *(__be16*)(pppoe_h);
		pppoe_h += 2; /* skip ppp protocol */
		memset(dest_addr, 0, sizeof(dest_addr));
		/* IPv4 */
		if ( PPP_IP == ppp_proto )
		{
			iph = (struct iphdr* )(pppoe_h);
			if ( IPPROTO_UDP != iph->protocol )
				return 0;
			memcpy(dest_addr, (unsigned char*)&iph->daddr, 4);
			if ( 0xe0 != (dest_addr[0] & 0xf0) )
				return 0;
	
			return 1;
		}
#ifdef TCSUPPORT_IPV6
		else if ( PPP_IPV6 == ppp_proto ) /* IPv6 */
		{
			ip6h = (struct ipv6hdr* )(pppoe_h);
			nexthdr = ip6h->nexthdr;
			offset = ipv6_skip_exthdr(skb, skip_start, &nexthdr, &frag_off);
			if ( offset >= 0 && IPPROTO_UDP == nexthdr )
			{
				memcpy(dest_addr, ip6h->daddr.s6_addr, 16);
				if ( 0xff != dest_addr[0] )
					return 0;

				return 1;
			}
		}
#endif
	}
#endif

	return 0;
}
EXPORT_SYMBOL(__is_udp_multicast);

#if defined(TCSUPPORT_FH_JOYMEV2_PON) || defined(TCSUPPORT_CT_JOYME_BANDWIDTH)
int (*wifi_bandwidth_hook_tx) (struct sk_buff * skb) = NULL;
int  (*wifi_bandwidth_hook_rx)(unsigned char* srcMac, int length) = NULL;

EXPORT_SYMBOL(wifi_bandwidth_hook_tx);
EXPORT_SYMBOL(wifi_bandwidth_hook_rx);
#endif



int localInShortCut(struct sk_buff *skb, int protocol)
{
	struct iphdr *iph = NULL;

    skb->pkt_type = PACKET_HOST;
	skb->ip_summed = CHECKSUM_UNNECESSARY ;

	skb_reset_network_header(skb);
	skb->mac_len = skb->network_header - skb->mac_header;
	iph = ip_hdr(skb);
	pskb_trim_rcsum(skb, ntohs(iph->tot_len));
	
	if ( !skb->skb_iif )
		skb->skb_iif = skb->dev->ifindex;

	/* remove ip header */
	skb_pull(skb, ip_hdrlen(skb));
	skb_reset_transport_header(skb);

	if (NULL == skb_dst(skb))
	{
		ip_route_input_noref(skb, iph->daddr, iph->saddr,
			iph->tos, skb->dev);
	}

    if (protocol)
    {
        if (!skb_steal_sock(skb) && !skb_rtable(skb))
        {
            kfree_skb(skb);
            return 0;
        }

        udp_rcv(skb);
    }
    else
    {   
        tcp_v4_rcv(skb);
    }

	return 0;
}
EXPORT_SYMBOL(localInShortCut);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
void smp_call_function_single_ext(int cpuid, struct call_single_data *data)
#else
void smp_call_function_single_ext(int cpuid, call_single_data_t *data)
#endif
{
    smp_call_function_single_async(cpuid, data);

    return;
}
EXPORT_SYMBOL(smp_call_function_single_ext);

#if defined(TCSUPPORT_CT_JOYME2)
#define PANIC_BUF_LEN	(12*1024)
int panic_log_on = 0;
int panic_len = 0;
char panic_buf[PANIC_BUF_LEN];

int get_panic_log_flag(void)
{
	return panic_log_on;
}

int set_panic_log_flag(int panic_flag)
{
	panic_log_on = panic_flag;
	return 0;
}

int get_panic_log_buffer_len(void)
{
	return panic_len;
}

int set_panic_log_buffer_len(int len)
{
	panic_len = len;
	return 0;
}

int config_reboot_info_exception(void)
{
	struct file *fp_info, *fp_src, *fp_tz;
	int ret = -1, len = 0;
	mm_segment_t fs;
	struct timeval nowtime;
    struct rtc_time tm;
	char format_time[20] = {0};
	char buf[256] = {0};
	char *p = NULL;
	char tmp1[8] = {0}, tmp2[8] = {0};
	int tz_hour = 0, tz_min = 0, minus = 1;

	const char* json_format = "{\"Time\":\"%s\","
							  "\"Source\":\"%s\"}";

	fs = get_fs();
	set_fs(KERNEL_DS);
	
	/* get local TZ */
	fp_tz = filp_open("/etc/TZ", O_RDONLY, 0);
	if (IS_ERR(fp_tz)) {
		printk("open TZ fail\n");
		goto out;
	}
	else if ( !ecnt_kernel_fs_read_check(fp_tz) )
	{
		filp_close(fp_tz, NULL);
		printk("cann't read TZ file\n");
		goto out;
	}
	else {
		ret = ecnt_kernel_fs_read(fp_tz, buf, sizeof(buf) - 1, &fp_tz->f_pos);
		printk("TZ:%s\n", buf);
		if ( (p = strstr(buf, "+")) || (p = strstr(buf, "-")) ) {
			memset(tmp1, 0, sizeof(tmp1));
			strncpy(tmp1, p, 3);
			if ('-' == tmp1[0])
				minus = -1;
			memset(tmp2, 0, sizeof(tmp2));
			strncpy(tmp2, &tmp1[1], 2);
			sscanf(tmp2, "%d", &tz_hour);
		}
		else {
			tz_hour = 8;
			minus = -1;
		}
		filp_close(fp_tz, NULL);
	}
	
	fp_info = filp_open("/opt/upt/apps/info/reboot_info", O_CREAT | O_WRONLY, 0666);
	if (IS_ERR(fp_info)) {
		printk("failed open file\n");
		goto out;
	}
	if( !ecnt_kernel_fs_write_check(fp_info) )
	{
		printk("cann't write\n");
		filp_close(fp_info, NULL);
		goto out;
	}

	/* get nowtime */
    #if LINUX_VERSION_CODE < KERNEL_VERSION(5,4,0)
	do_gettimeofday(&nowtime);
    #endif
    /* parse to local time with current TZ */
    nowtime.tv_sec -= minus * tz_hour * 60 * 60;
    /* parse to tm */
    rtc_time_to_tm(nowtime.tv_sec,&tm);
	snprintf(format_time
			, sizeof(format_time)
			, "%04d-%02d-%02d %02d:%02d:%02d"
			, tm.tm_year + 1900
			, tm.tm_mon + 1
			, tm.tm_mday
			, tm.tm_hour
			, tm.tm_min
			, tm.tm_sec);

	len = snprintf(buf, sizeof(buf), json_format, format_time, "Exception");

	ret = ecnt_kernel_fs_write(fp_info, buf, len, &fp_info->f_pos);
	filp_close(fp_info, NULL);

	/* create reboot_source file */
	fp_src = filp_open("/opt/upt/apps/info/reboot_source", O_CREAT | O_WRONLY, 0666);
	if (IS_ERR(fp_src)) {
		printk("failed open file\n");
		goto out;
	}
	
	if( !ecnt_kernel_fs_write_check(fp_src) )
	{
		printk("cann't write\n");
		filp_close(fp_src, NULL);
		goto out;
	}
	ret = ecnt_kernel_fs_write(fp_src, buf, len, &fp_src->f_pos);
	filp_close(fp_src, NULL);

out:
	set_fs(fs);
	return 0;
}

extern int nand_flash_avalable_size;
#ifndef TCSUPPORT_CPU_ARMV8
extern int iswatchDogReset;
#endif
int panic_write(void){	
	struct file *fp = NULL;
	int ret = -1, len = 0;
	mm_segment_t fs;
	int info_size = 0;
	unsigned int defaultromfile_flag_addr = 0;

	if ((len = get_panic_log_buffer_len()) <= 0)
		goto out;

/*------------------------------------------------------------------------
	Use the defaultromfile sector to save crash_info, because the reservearea is 
	last sector and the reservearea's size is 0x1c0000, so the defaultromfile_flag_addr
	= nand_flash_avalable_size - 0x1c0000 + 0x40000
	|sector 	name				cover area				note
	|1			backupromfile			0~0x3ffff					256k
	|2			defaultromfile			0x40000~0x7ffff 		256k
	#define RESERVEAREA_TOTAL_SIZE RESERVEAREA_ERASE_SIZE*7
	#define DEFAULTROMFILE_RA_OFFSET (BACKUPROMFILE_RA_OFFSET+BACKUPROMFILE_RA_SIZE)
------------------------------------------------------------------------*/
	defaultromfile_flag_addr = nand_flash_avalable_size - RESERVEAREA_TOTAL_SIZE + DEFAULTROMFILE_RA_OFFSET;
	/*erase defaultromfile*/
	nandflash_erase(defaultromfile_flag_addr, 0x10000);
	/*write crash_info into defaultromfile sector*/
	nandflash_write(defaultromfile_flag_addr, len + 4, &info_size, panic_buf);

#ifndef TCSUPPORT_CPU_ARMV8
	iswatchDogReset = 1;
#endif
	fp = filp_open("/opt/upt/apps/info/crash_info", O_CREAT | O_WRONLY, 0666);
	if (IS_ERR(fp)) {
		printk("failed open file\n");
		return ret;
	}
	
	if( !ecnt_kernel_fs_write_check(fp) )
	{
		printk("cann't write\n");
		goto out;
	}

	fs = get_fs();
	set_fs(KERNEL_DS);
	ret = ecnt_kernel_fs_write(fp, panic_buf, len + 4, &fp->f_pos);
	set_fs(fs);

out:
	if (NULL != fp)
	{	
		filp_close(fp, NULL);
	}
	config_reboot_info_exception();
	sys_sync();
	return ret;
}

int panic_write2(const char *buf)
{
	struct pt_regs *regs;
	int ret = 0, panic_flag = 0;

	panic_flag = get_panic_log_flag();
	if (panic_flag) 
		return 0;
	set_panic_log_flag(1);
	printk("%s\n", buf);
 	regs = get_irq_regs();
	if (regs) 
		show_regs(regs);
	dump_stack();
	ret = panic_write();

	return ret;
}

void set_panic_log_buffer(va_list *args, const char* fmt){
	int panic_flag = 0, len = 0;

	panic_flag = get_panic_log_flag();
	len = get_panic_log_buffer_len();
		
	if (panic_flag && len < PANIC_BUF_LEN) {
		if (len == 0)
		{
			panic_buf[0] = 'e';
			panic_buf[1] = 'c';
			panic_buf[2] = 'o';
			panic_buf[3] = '\n';
			len += vsnprintf(&panic_buf[len + 4], PANIC_BUF_LEN - 4 - len, fmt, *args);
		}
		else
			len += vsnprintf(&panic_buf[len], PANIC_BUF_LEN - 4 - len, fmt, *args);
	}
	set_panic_log_buffer_len(len);
}
#endif

int itf_start_idx = 1;
EXPORT_SYMBOL(itf_start_idx);

int isLANInterface(struct net_device *dev)
{
    return ((dev != NULL) && (dev->name[0] == 'e') && \
        (dev->name[1] == 't')  && (dev->name[2] == 'h') && \
        (dev->name[3] == '0') && (dev->name[4] == '.'));
}
EXPORT_SYMBOL(isLANInterface);

int is24GWiFiInterface(struct net_device *dev)
{
#ifdef TCSUPPORT_WLAN_MT76_MAC80211
    return ((dev != NULL) && (strlen(dev->name) == 7) && \
        (strncmp(dev->name,"wlan0", 5) == 0));
#else
    return ((dev != NULL) && (strlen(dev->name) == 3) && \
        (dev->name[0] == 'r') && (dev->name[1] == 'a'));
#endif
}
EXPORT_SYMBOL(is24GWiFiInterface);

int is24GWDSInterface(struct net_device *dev)
{
    return ((dev != NULL) && (strlen(dev->name) == 4)&& (dev->name[0] == 'w') && \
        (dev->name[1] == 'd') && (dev->name[2] == 's'));
}
EXPORT_SYMBOL(is24GWDSInterface);

int is5GWDSInterface(struct net_device *dev)
{
    return ((dev != NULL) && (strlen(dev->name) == 5)&& (dev->name[0] == 'w') && \
        (dev->name[1] == 'd') && (dev->name[2] == 's') && (dev->name[3] == 'i'));
}
EXPORT_SYMBOL(is5GWDSInterface);

int is24GAPCLIInterface(struct net_device *dev)
{
      return ((dev != NULL) && (strlen(dev->name) == 6)&& (dev->name[0] == 'a') && \
        (dev->name[1] == 'p') && (dev->name[2] == 'c') && (dev->name[3] == 'l') && (dev->name[4] == 'i'));
}
EXPORT_SYMBOL(is24GAPCLIInterface);

int is5GAPCLIInterface(struct net_device *dev)
{
    return ((dev != NULL) && (strlen(dev->name) == 7)&& (dev->name[0] == 'a') && \
        (dev->name[1] == 'p') && (dev->name[2] == 'c') && (dev->name[3] == 'l') && (dev->name[4] == 'i') && (dev->name[5] == 'i'));
}
EXPORT_SYMBOL(is5GAPCLIInterface);

int is5GWiFiInterface(struct net_device *dev)
{
#ifdef TCSUPPORT_WLAN_MT76_MAC80211
    return ((dev != NULL) && (strlen(dev->name) == 7) && \
        (strncmp(dev->name,"wlan1", 5) == 0));
#else
    return ((dev != NULL) && (strlen(dev->name) == 4) && \
        (dev->name[0] == 'r') && (dev->name[1] == 'a') && (dev->name[2] == 'i'));
#endif
}
EXPORT_SYMBOL(is5GWiFiInterface);

int isWiFiInterface(struct net_device *dev)
{
#ifdef TCSUPPORT_WLAN_MT76_MAC80211
    return ((dev != NULL) && \
        (strncmp(dev->name,"wlan", 4) == 0));
#else
    return ((dev != NULL) && (dev->name[0] == 'r') && \
        (dev->name[1] == 'a'));
#endif
}
EXPORT_SYMBOL(isWiFiInterface);

int isUSBInterface(struct net_device *dev)
{
    return ((dev != NULL) && (dev->name[0] == 'u') && \
        (dev->name[1] == 's') && (dev->name[2] == 'b'));
}
EXPORT_SYMBOL(isUSBInterface);

int isXSIInterface(struct net_device *dev)
{
    return ((dev != NULL) && (dev->name[0] == 'e') && \
        (dev->name[1] == 't') && (dev->name[2] == 'h') && (dev->name[3] == '1'));
}
EXPORT_SYMBOL(isXSIInterface);

int isPONInterface(struct net_device *dev)
{
    return ((dev != NULL) && (dev->name[0] == 'p') && \
        (dev->name[1] == 'o') && (dev->name[2] == 'n'));
}
EXPORT_SYMBOL(isPONInterface);

int isWANInterface(struct net_device *dev)
{
    return ((dev != NULL) && (dev->name[0] == 'n') && \
        (dev->name[1] == 'a')&& (dev->name[2] == 's'));
}
EXPORT_SYMBOL(isWANInterface);

int isBridgeInterface(struct net_device *dev)
{
    return ((dev != NULL) && (dev->name[0] == 'b') && 
        (dev->name[1] == 'r'));
}
EXPORT_SYMBOL(isBridgeInterface);


int getLANIndex(struct net_device *dev)
{
    return (dev->name[5] - '0' - itf_start_idx);
}
EXPORT_SYMBOL(getLANIndex);

int getLANIndexByName(char *pname)
{
    return (pname[5] - '0' - itf_start_idx);
}
EXPORT_SYMBOL(getLANIndexByName);

char lanPortNamePre[] = "eth0.";
char *lanNamePre(void)
{
    return lanPortNamePre;
}
EXPORT_SYMBOL(lanNamePre);

int get24GWDSIndex(struct net_device *dev)
{
    return (dev->name[3] - '0');
}
EXPORT_SYMBOL(get24GWDSIndex);

int get5GWDSIndex(struct net_device *dev)
{
    return (dev->name[4] - '0');
}
EXPORT_SYMBOL(get5GWDSIndex);

int get24GWifiIndex(struct net_device *dev)
{
#ifdef TCSUPPORT_WLAN_MT76_MAC80211
    return (dev->name[6] - '0');
#else
    return (dev->name[2] - '0');
#endif
}
EXPORT_SYMBOL(get24GWifiIndex);

int get5GWifiIndex(struct net_device *dev)
{
#ifdef TCSUPPORT_WLAN_MT76_MAC80211
    return (dev->name[6] - '0');
#else
    return (dev->name[3] - '0');
#endif
}
EXPORT_SYMBOL(get5GWifiIndex);

int getUSBIndex(struct net_device *dev)
{
    return (dev->name[3] - '0');
}
EXPORT_SYMBOL(getUSBIndex);

/*After calling this interface to get net_dev, we need to call dev_put to release it*/
struct net_device *get24GWifiName(int index)
{
	struct net_device dev;
#ifdef TCSUPPORT_WLAN_OPENWRT_MT76_MAC80211_SOFT_MAC
	dev.name[0] = 'w';
	dev.name[1] = 'l';
	dev.name[3] = 'a';
	dev.name[4] = 'n';
	dev.name[5] = '0';
	dev.name[6] = '-';
	dev.name[7] = '0'+index;
	dev.name[8] = '\0';
#else
	dev.name[0] = 'r';
	dev.name[1] = 'a';
	dev.name[2] = '0'+index;
	dev.name[3] = '\0';
#endif
    return dev_get_by_name(&init_net, dev.name);
}
EXPORT_SYMBOL(get24GWifiName);


#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,4,0)
struct class	*hnat_class1;
struct device *ecnt_device_create(int result)
{
	hnat_class1 = class_create(THIS_MODULE, "hwnat0");
	
	return device_create(hnat_class1,NULL,MKDEV(result,0),NULL,"hwnat0");
}
EXPORT_SYMBOL(ecnt_device_create);

void ecnt_device_delete(int result)
{
	device_destroy(hnat_class1,MKDEV(result,0));
	class_destroy(hnat_class1);
}
EXPORT_SYMBOL(ecnt_device_delete);
#endif
