/***************************************************************
Copyright Statement:

This software/firmware and related documentation (EcoNet Software) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (EcoNet) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (ECONET SOFTWARE) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN AS IS 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVERS SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVERS SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/
#ifndef _ECNT_HOOK_FE_TYPE_H_
#define _ECNT_HOOK_FE_TYPE_H_

/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/
#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/in6.h>
#else
#include <netinet/in.h>
#endif

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
	
/************************************************************************
*                  M A C R O S
*************************************************************************
*/
#define FE_WAN_RX_UC_RATE 	0
#define FE_WAN_RX_BC_RATE 	1
#define FE_WAN_RX_MC_RATE 	2
#define FE_WAN_RX_OC_RATE 	3
#define FE_WAN_TX_RATE_LIMIT	4
#define FE_WAN_RX_MAC_FILTER_RATE_LIMIT	5
	
#define CDM_TX_ENCODING_SHIFT					(0)
#define CDM_TX_ENCODING_MASK					(0xF<<CDM_TX_ENCODING_SHIFT)
#define CDM_RX_DECODING_SHIFT					(4)
#define CDM_RX_DECODING_MASK					(0xF<<CDM_RX_DECODING_SHIFT)
#define GDM_TX_ENCODING_SHIFT					(8)
#define GDM_TX_ENCODING_MASK					(0xF<<GDM_TX_ENCODING_SHIFT)
#define GDM_RX_DECODING_SHIFT					(12)
#define GDM_RX_DECODING_MASK					(0xF<<GDM_RX_DECODING_SHIFT)

#define MTR_MASK        (0x3F)
#define MTR_MASK_LEN    (6)
#define ACNT0_OFFSET    MTR_MASK_LEN
#define ACNT0_MASK      (0x3F<<ACNT0_OFFSET)
#define ACNT0_MASK_LEN  (6)
#define ACNT1_OFFSET    (MTR_MASK_LEN + ACNT0_MASK_LEN)
#define ACNT1_MASK      (0x1F<<ACNT1_OFFSET)
#define ACNT1_MASK_LEN  (5)
#define ACNT2_OFFSET    (MTR_MASK_LEN + ACNT0_MASK_LEN + ACNT1_MASK_LEN)
#define ACNT2_MASK      (0x7F<<ACNT2_OFFSET)
#define ACNT2_MASK_LEN  (7)

#define ETHER_WAN_OFFSET (MTR_MASK_LEN + ACNT0_MASK_LEN + ACNT1_MASK_LEN + ACNT2_MASK_LEN)
#define ETHER_WAN_MASK  (1<<ETHER_WAN_OFFSET)

#define DFT_ITF_OFFSET   (MTR_MASK_LEN + ACNT0_MASK_LEN + ACNT1_MASK_LEN + ACNT2_MASK_LEN)
#define DFT_ITF_MASK     (0x1<<DFT_ITF_OFFSET)
#define DFT_ITF_MASK_LEN (1)

#define FE_AC_MTR_GROUP_MAX     (63)
#define ACNT_GRP_WAN_NUM		(8)
#define UP_DOWN_WAN_ITF_NUM		(ACNT_GRP_WAN_NUM*2)
#define UP_DOWN_QUEUE_NUM		(16)

#define WAN_ITF_AND_DEV_NUM				(23)
#define UP_DOWN_WAN_ITF_AND_DEV_NUM		(WAN_ITF_AND_DEV_NUM*2)
#define WAN_ITF_ONLY_BASE				(UP_DOWN_WAN_ITF_AND_DEV_NUM)
#define WAN_ITF_ONLY_NUM				(3)
#define UP_DOWN_WAN_ITF_ONLY_NUM		(WAN_ITF_ONLY_NUM*2)
#define MULTICAST_DOWNSTREAM_BASE		(WAN_ITF_ONLY_BASE + UP_DOWN_WAN_ITF_ONLY_NUM)
#define MULTICAST_DOWNSTREAM_NUM		(11)

#define WAN_ITF_HAS_FLOW_ACNT_NUM		(8)/*one itf need one*/

#define FE_TRFC_SVID	(1 << 0)
#define FE_TRFC_SPBIT	(1 << 1)
#define FE_TRFC_CVID	(1 << 2)
#define FE_TRFC_CPBIT	(1 << 3)
#define FE_TRFC_TOS		(1 << 4)
#define FE_TRFC_TC		(1 << 5)
#define FE_TRFC_ETYPE	(1 << 6)
#define FE_TRFC_DPORT	(1 << 7)
#define FE_TRFC_SPORT	(1 << 8)
#define FE_TRFC_DIPV6	(1 << 9)
#define FE_TRFC_SIPV6	(1 << 10)
#define FE_TRFC_DIPV4	(1 << 11)
#define FE_TRFC_SIPV4	(1 << 12)
#define FE_TRFC_DMAC	(1 << 13)
#define FE_TRFC_SMAC	(1 << 14)

#define FE_TRFC_CVLAN	(FE_TRFC_CPBIT|FE_TRFC_CVID)
#define FE_TRFC_SVLAN	(FE_TRFC_SPBIT|FE_TRFC_SVID)
#define FE_TRFC_VLAN	(FE_TRFC_CVLAN|FE_TRFC_SVLAN)

#define FE_MC_VLAN_TABLE_MAX     (64)
#define FE_MC_VLAN_PORT_MAX    	 (16)

/************************************************************************
*                  D A T A	 T Y P E S
*************************************************************************
*/

typedef enum {
	ECNT_FE_API,
}FE_Api_SubType_t;

typedef enum {
	FE_GDM_SEL_GDMA1 = 0 ,
	FE_GDM_SEL_GDMA2,
	FE_GDM_SEL_GDMA3
} FE_Gdma_Sel_t;

typedef enum {
	FE_OFF = 0 ,
	FE_ON = 1
} FE_Status_Sel_t;

typedef enum {
	FE_MAC_SET_LAN = 0 ,
	FE_MAC_SET_WAN
} FE_MacSet_Sel_t;

typedef enum {
	FE_MAC_GPON = 0 ,
	FE_MAC_EPON
} FE_PonMode_Sel_t;

typedef enum {
	FE_WANPORT_PSE_P2 = 2 ,
#if defined(TCSUPPORT_CPU_EN7580)
	FE_WANPORT_GSW_P0 = 16,
#else
	FE_WANPORT_GSW_P0 = 8,
#endif
	FE_WANPORT_GSW_P1,
	FE_WANPORT_GSW_P2,
	FE_WANPORT_GSW_P3,
	FE_WANPORT_GSW_P4,
	FE_WANPORT_GSW_P5
} FE_WanPort_Sel_t;

typedef enum {
	FE_CDM_SEL_CDMA1 = 0 ,
	FE_CDM_SEL_CDMA2
} FE_Cdma_Sel_t;

typedef enum {
	FE_RED_DROP_Q0 = 0 ,
	FE_RED_DROP_Q1L,
	FE_RED_DROP_Q1H
} FE_RedDropQ_Sel_t;

typedef enum {
	FE_RATE_OR_FULL_DROP = 0 ,
	FE_RATE_OR_THRL_DROP,
	FE_RATE_AND_FULL_DROP,
	FE_RATE_AND_THRL_DROP
} FE_RedDropMode_Sel_t;

typedef enum {
	FE_GDM_SEL_TX = 0 ,
	FE_GDM_SEL_RX 
} FE_TXRX_Sel_t;

typedef enum {
	FE_DISABLE = 0 ,
	FE_ENABLE 
} FE_Enable_t;	

typedef enum {
	FE_NOT_PPPOE = 0 ,
	FE_IS_PPPOE 
} FE_PPPOE_t;	

typedef enum {
	ETH_TYPE = 0 ,
	PPPOE_TYPE,
	IP_TYPE,
	TCP_TYPE,
	UDP_TYPE,
} FE_Patn_type;	

typedef enum {
	FE_KEY0 = 0 ,
	FE_KEY1 
} FE_L2U_KEY_t;	

 typedef enum {
	 FE_LINKUP = 0,
	 FE_LINKDOWN
 } FE_Linkup_t;

typedef enum {
	FE_PCP_MODE_DISABLE = 0,
	FE_PCP_MODE_8B0D = 1,
	FE_PCP_MODE_7B1D = 2, 
	FE_PCP_MODE_6B2D = 4,
	FE_PCP_MODE_5B3D = 8
} FE_PcpMode_t;

typedef enum {
	FE_DP_QDMA1_CPU = 0,
	FE_DP_GDM1,
	FE_DP_GDM2,
	FE_DP_QDMA1_HWF,
	FE_DP_GDMA3_HWF = 3,
	FE_DP_PPE,
	FE_DP_QDMA2_CPU,
	FE_DP_QDMA2_HWF,
	FE_DP_DISCARD
} FE_Frame_dp_t;

typedef enum {
	FE_TYPE_OC = 0,
	FE_TYPE_MC,
	FE_TYPE_BC,
	FE_TYPE_UC,
} FE_Frame_type_t ;

typedef enum {
	FE_ERR_CRC = 0,
	FE_ERR_LONG,
	FE_ERR_RUNT,
} FE_Err_type_t ;

typedef enum{
	FE_ADD_TLS_PORT=0,
	FE_DEL_TLS_PORT,
	FE_SET_TLS_VLAN,
	FE_UNSET_TLS_VLAN,
	FE_ADD_GROUP_TLS_ENTRY,
	FE_DEL_GROUP_TLS_ENTRY,
	FE_SHOW_GROUP_TLS_ENTRY,
}FE_Tls_cmd_t;

typedef enum{
	FE_ADD_TLS_ENTRY=0,
	FE_DEL_TLS_ENTRY,
	FE_SHOW_TLS_ENTRY,	
}FE_Tls_Trunk_Mode_Cmd_t;

typedef enum{
	FE_TLS_FORWARD_UPSTREAM=0,
	FE_TLS_FORWARD_DOWNSTREAM,
}FE_Tls_forward_direction_t;

typedef struct
{
	unsigned char uni_port_id;/*0~3*/
	unsigned short uni_vlan;/*2~4094,4095 means not care*/
	unsigned short gem_port;/*gemport id*/
	unsigned short ani_vlan;/*2~4094,4095 means not care*/
}FE_Tls_info_t;

typedef enum{
	FE_SCU_RESET = 0,
	FE_CORE_RESET,
}FE_Reset_mode_t;

enum BindDir {
	UPSTREAM_ONLY = 0,	/* only speed up upstream flow */
	DOWNSTREAM_ONLY = 1,	/* only speed up downstream flow */
	BIDIRECTION = 2		/* speed up bi-direction flow */
};

typedef enum{
	FIX_MODE = 0,
	RANDOM_MODE,
}FE_Random_mode_t;

typedef enum{
    METER_BYTE_MODE = 0,
    METER_PACKET_MODE,
}FE_Packet_Mode_t;

typedef enum{
    FAST_TICK = 0,
    SLOW_TICK,
}FE_TickSel_t;


typedef enum{
    DOWN_STREAM = 0,
    UP_STREAM,
}FE_Dir_t;

typedef enum{
    CAR_QOS_MODE = 0,/*CT and cmcc */
    IP_BIND_MODE,
    IF_BIND_MODE,
    OLT_CTL_MODE,
    TRAFFIC_QOS_MODE,/*used for 8 queue traffic qos */
    DEV_MAC_MODE,
    FORCEBW_WRR_MODE,
    WAN_MODE,
    NULL_BIND_MODE,
}FE_Meter_Mode_t;

typedef enum{
	TRAFFIC_FLOW_MODE = 0, /*dafault set , traffic mode, could be exend  if need */
	DEV_MODE,
	WAN_ITF_MODE,
	MULTICAST_MODE,
	WAN_ITF_AND_TRAFFIC_FLOW_MODE,
	DEFAULT_NULL_MODE,
}FE_ACNT_Mode_t;


struct  Fe_Ratelimit_IP_Info   
{
    unsigned char index ;			/*rue index*/
    unsigned char enable ;			/*this rule if active?*/
    unsigned int rate ;

    unsigned char ip_version; 		/*0: IPv4 , 1: IPv6*/

    unsigned char v4_pair_l[4] ; 	/*IPv4 range*/
    unsigned char v4_pair_h[4] ;

    unsigned char v6_pair_l[16] ;	/*IPv6 range*/
    unsigned char v6_pair_h[16] ;
};

struct  Fe_Ratelimit_Itf_Info
{
    unsigned char index ;			/*rule index*/
    unsigned char enable ;			/*this rule if active?*/
    unsigned int rate ;
};


typedef struct
{
    struct sk_buff *skb;
    uint               rate;/*ratelimit value.*/
    FE_Enable_t        enable;
    FE_Packet_Mode_t   pkt_mode;
    FE_TickSel_t       tick_sel;
    unchar             idx;/*idx of mapping array*/
    FE_Dir_t           dir;
    FE_Meter_Mode_t    mode;
    FE_ACNT_Mode_t	   acnt_mode;
    uint               bucket_size;  
    unchar             meter_id;
    unchar             acnt2_id;
    unchar             acnt1_id;
    unchar             acnt0_id;
    uint               byteCnt_L;
    uint               byteCnt_H;
    uint               pktCnt;
    unchar 			   mac[6];
    char               itf_name[16];
    struct  Fe_Ratelimit_IP_Info ip_info;
    struct  Fe_Ratelimit_Itf_Info if_info;
}Meter_cfg_t;

typedef enum{
	ADD_MODE = 0,
	SUBTRACT_MODE,
}FE_Rate_minus_t;

typedef enum {
	FE_MC_VLAN_KEEP = 0 ,
	FE_MC_VLAN_REPLACE,
	FE_MC_VLAN_ADD,
	FE_MC_VLAN_REMOVE
} FE_McVlan_OP_t;

typedef enum {
	FE_MC_VPM_KEEP = 0 ,
	FE_MC_VPM_8100,
	FE_MC_VPM_88a8,
	FE_MC_VPM_CSR
} FE_McVlan_VPM_t;

typedef enum {
	XSI_PHY_LINK_DOWN,
	XSI_PHY_LINK_UP,
} XFI_LINK_STATUS_t ;

typedef enum {
	FE_RLE_PRE_PACKET = 0 ,
	FE_RLE_PRE_MBI_TRANSACTION
} FE_RLS_MODE_t;

typedef enum {
	FE_SET_PACKET_LENGTH = 0,
	FE_SET_CHANNEL_ENABLE,
	FE_SET_MAC_ADDR,
	FE_SET_HWFWD_CHANNEL,
	FE_SET_CHANNEL_RETIRE,
	FE_SET_CRC_STRIP,
	FE_SET_PADDING,
	FE_GET_EXT_TPID,
	FE_SET_EXT_TPID,
	FE_GET_FW_CFG,
	FE_SET_FW_CFG,
	FE_SET_DROP_UDP_CHKSUM_ERR_ENABLE,
	FE_SET_DROP_TCP_CHKSUM_ERR_ENABLE,
	FE_SET_DROP_IP_CHKSUM_ERR_ENABLE,
	FE_SET_DROP_CRC_ERR_ENABLE,
	FE_SET_DROP_RUNT_ENABLE,
	FE_SET_DROP_LONG_ENABLE,
	FE_SET_VLAN_CHECK,
	FE_GET_OK_CNT,
	FE_GET_RX_CRC_ERR_CNT,
	FE_GET_RX_DROP_FIFO_CNT,
	FE_GET_RX_DROP_ERR_CNT,
	FE_GET_OK_BYTE_CNT,
	FE_GET_TX_GET_CNT,
	FE_GET_TX_DROP_CNT,
	FE_GET_TIEM_STAMP,
	FE_SET_TIEM_STAMP,
	FE_SET_INS_VLAN_TPID,
	FE_SET_VLAN_ENABLE,
	FE_SET_BLACK_LIST,
	FE_SET_ETHER_TYEP,
	FE_SET_L2U_KEY,
	FE_GET_AC_GROUP_PKT_CNT,
	FE_GET_AC_GROUP_BYTE_CNT,
	FE_CLR_AC_GROUP_PKT_CNT,
	FE_CLR_AC_GROUP_BYTE_CNT,
	FE_SET_METER_GROUP,
	FE_GET_METER_GROUP,
	FE_SET_GDM_PCP_CODING,
	FE_SET_CDM_PCP_CODING,
	FE_SET_VIP_ENABLE,
	FE_GET_ETH_RX_CNT,
	FE_GET_ETH_TX_CNT,
	FE_GET_ETH_FRAME_CNT,
	FE_GET_ETH_ERR_CNT,
	FE_SET_CLEAR_MIB,
	FE_SET_CDM_RX_RED_DROP,
	FE_GET_CDM_RX_RED_DROP,
	FE_SET_CHANNEL_RETIRE_ALL,
	FE_SET_CHANNEL_RETIRE_ONE,	
	FE_SET_TX_RATE,
	FE_SET_RXUC_RATE,
	FE_SET_RXBC_RATE,
	FE_SET_RXMC_RATE,
	FE_SET_RXOC_RATE,
	FE_ADD_VIP_ETHER,
	FE_ADD_VIP_PPP,
	FE_ADD_VIP_IP,
	FE_ADD_VIP_TCP,
	FE_ADD_VIP_UDP,
	FE_DEL_VIP_ETHER,
	FE_DEL_VIP_PPP,
	FE_DEL_VIP_IP,
	FE_DEL_VIP_TCP,
	FE_DEL_VIP_UDP,
	FE_ADD_L2LU_VLAN_DSCP,
	FE_ADD_L2LU_VLAN_TRFC,
	FE_DEL_L2LU_VLAN_DSCP,
	FE_DEL_L2LU_VLAN_TRFC,	
	FE_ADD_TRAFFIC_CLASS,
	FE_DEL_TRAFFIC_CLASS,
	FE_SET_TX_FAVOR_OAM_ENABLE,
	FE_SET_TLS_CFG,
	FE_TLS_FORWARD,
	FE_DO_RESET,
	FE_SET_MAC_ADDR_7516,
	FE_SET_WAN_PORT_7516,
	FE_SET_LOOPBACK_ENABLE,
	FE_SET_LOOPBACK_MODE,
	FE_GET_UNKNOWN_MUL_PKT,
#if 1
    FE_SET_METER_RATELIMIT,
    FE_GET_METER_RATELIMIT,
    FE_GET_METER_IDX,
    FE_GET_ACNT1_IDX,
    FE_GET_ACNT0_IDX,
    FE_INIT_RESOURCE_MANAGE,
    FE_DEINIT_RESOURCE_MANAGE,
    FE_GET_RX_RATELIMIT_MODE,
    FE_SET_RX_RATELIMIT_RULE,
    FE_SET_RX_RATELIMIT_MODE,
    FE_SET_METER_CTL_BY_OLT,
    FE_GET_FLOW_CNT,
    FE_CLEAR_FLOW_CNT,
    FE_GET_ACNT0_MODE,
    FE_GET_ACNT1_MODE,
    FE_SET_ACNT0_MODE,
    FE_SET_ACNT1_MODE,
    FE_GET_METER_ENABLE,
    FE_GET_DEV_MAC_INDEX,
    FE_SET_PSE_OQ_THR,
    FE_GET_ACNT2_IDX,
    FE_SET_ACNT2_MODE,
    FE_GET_WAN_ITF_INDEX,
#endif
	FE_SET_GLO_RATE_BYTE,
	FE_GET_PPPOE_INFO,
	FE_SET_PPPOE_INFO_CLEAN,
	FE_GET_TX_TRAFFIC,
	FE_GET_RX_TRAFFIC,
	FE_GET_TX_RATE,
	FE_GET_RX_RATE,
	FE_GET_TX_OCTETS,
	FE_GET_RX_OCTETS,
	FE_GET_RX_DISCARD_COUNTER,
	FE_GET_TX_DISCARD_COUNTER,
	FE_GET_RX_ERROR_COUNTER,
	FE_GET_TX_ERROR_COUNTER,
	FE_ADD_DEV_TO_TOTAL_ACCOUNT,
	FE_ADD_STB_SRC_IP,
	FE_DEL_STB_SRC_IP,
	FE_SET_RATELIMIT_FOR_PKT_FORMATE,
	FE_SET_MC_VLAN_GLOBAL,
	FE_GET_MC_VLAN_GLOBAL,
	FE_SET_MC_VLAN_TABLE_CFG,
	FE_GET_MC_VLAN_TABLE_CFG,
	FE_SET_MC_VLAN_ACTION_CFG,
	FE_GET_MC_VLAN_ACTION_CFG,
	FE_SET_MC_VLAN_CLEAR_ALL,
	FE_SET_RX_MAC_FILTER,
	FE_SET_RX_MAC_FILTER_RATE,
	FE_XFI_PHY_LINK_CHANGE,
	FE_SET_GDMA_RELEASE_MODE,
	FE_GET_HSGMII_LAN_RX_CNT,
	FE_GET_HSGMII_LAN_TX_CNT,
	FE_SET_AEWAN_FWDFQ,
	FE_SET_AEWAN_IFCDISABLE,
	FE_SET_GMD2_SPTAG_FOR_LOOPBACK,
	FE_SET_QDMAWAN_TXQ_CNGST_ENABLE,
	FE_FUNCTION_MAX_NUM,
} FE_HookFunctionID_t ;

typedef struct FE_TxCnt_s{
	unsigned long long txFrameCnt;
	unsigned long long txFrameLen;
	unsigned long long txDropCnt;
	unsigned long long txBroadcastCnt;
	unsigned long long txMulticastCnt;
	unsigned long long txLess64Cnt;
	unsigned long long txMore1518Cnt;
	unsigned long long txEq64Cnt;
	unsigned long long txFrom65To127Cnt;
	unsigned long long txFrom128To255Cnt;
	unsigned long long txFrom256To511Cnt;
	unsigned long long txFrom512To1023Cnt;
	unsigned long long txFrom1024To1518Cnt;
}FE_TxCnt_t;

typedef struct FE_RxCnt_s{
    unsigned long long rxFCDropCnt;
    unsigned long long rxRCDropCnt;
    unsigned long long rxOVDropCnt;
    unsigned long long rxERRDropCnt;
    unsigned long long rxOKPktCnt;
    unsigned long long rxOKByteCnt;
    unsigned long long rxOversizeCnt;
    unsigned long long rxUnderSizeCnt;
    unsigned long long rxFrameCnt;
    unsigned long long rxFrameLen;
    unsigned long long rxDropCnt;
    unsigned long long rxBroadcastCnt;
    unsigned long long rxMulticastCnt;
    unsigned long long rxCrcCnt;
    unsigned long long rxFragFameCnt;
    unsigned long long rxJabberFameCnt;
    unsigned long long rxLess64Cnt;
    unsigned long long rxMore1518Cnt;
    unsigned long long rxEq64Cnt;
    unsigned long long rxFrom65To127Cnt;
    unsigned long long rxFrom128To255Cnt;
    unsigned long long rxFrom256To511Cnt;
    unsigned long long rxFrom512To1023Cnt;
    unsigned long long rxFrom1024To1518Cnt;
}FE_RxCnt_t;

typedef struct
{
	unchar valid;
	int timeout;
	unsigned char mac[6];
	int wan_itf;
}WanItfAndDevBandwidth_t;

typedef struct
{
	unchar valid;
	int timeout;
	int wan_itf;
}WanItf_t;

typedef struct
{
	unchar valid;
	int timeout;
	int wan_itf;
	int group_ip_type;  /* 0 means ipv4, 1 means ipv6 */
	union {
		unsigned int ip4;
		struct in6_addr ip6;
	} group_ip;
}Multicast_t;

#ifdef __KERNEL__
typedef struct {
    unsigned char mac[6];
    unsigned int upstream_byte_lo;
    unsigned int upstream_byte_hi;
    unsigned int upstream_packet;
    unsigned int downstream_byte_lo;
    unsigned int downstream_byte_hi;
    unsigned int downstream_packet;
    struct list_head account_node;
}dev_bandwidth_account_t;
#endif

typedef struct FE_TrfcCfg_s
{
	unsigned char smac[6];
	unsigned char dmac[6];
	unsigned int sip;
	unsigned int dip;
	unsigned short sipv6[8];
	unsigned short dipv6[8];
	unsigned short sport;
	unsigned short dport;
	unsigned short eth_type;
	unsigned char tc_min;
	unsigned char tc_max;
	unsigned char tos_min;
	unsigned char tos_max;
	unsigned char cpbit_min;
	unsigned char cpbit_max;
	unsigned short cvid_min;
	unsigned short cvid_max;
	unsigned char spbit_min;
	unsigned char spbit_max;
	unsigned short svid_min;
	unsigned short svid_max;
	/* each bit means if need to care the key above or not */
	unsigned int mask; 
	unsigned char field_num;
}FE_TrfcCfg_t;

typedef struct FE_PppoeInfo_s
{
	int valid;
	char ifname[16];
	unsigned char smac_add[6];
	unsigned char dmac_add[6];
	unsigned int sessionID;
	unsigned int src_ip;
	unsigned int dst_ip;
	int dns_valid;
	unsigned int pri_dns;
	unsigned int snd_dns;
}FE_PppoeInfo_t;

typedef struct FE_McVlanTableCfg_s
{
	unsigned char   mc_table_id;

	FE_Enable_t     enable;
	unsigned short  ppe_entry_id;
}FE_McVlanTableCfg_t;

typedef struct FE_McVlanActionCfg_s
{
	unsigned char   mc_table_id;
	unsigned char   mc_chnl_id;

	FE_McVlan_OP_t  mc_vlan_op;
	FE_McVlan_VPM_t mc_vlan_vpm;
	unsigned short  mc_vlan;
}FE_McVlanActionCfg_t;

typedef struct FE_RxMacFilter_s
{
	unsigned char cmd_id;
	unsigned char gdm_sel;
	unsigned char pkt_type;
	unsigned char mac_sel;
	unsigned char action;
	unsigned char list_type;
	unsigned char mac[6];
}FE_RxMacFilter_t;

struct ecnt_fe_data {
	FE_HookFunctionID_t function_id;
	FE_Gdma_Sel_t gdm_sel;
	FE_MacSet_Sel_t macSet_sel;
    FE_Cdma_Sel_t cdm_sel;
    FE_RedDropQ_Sel_t dropQ_sel;
    FE_RedDropMode_Sel_t dropMode_sel;
	FE_TXRX_Sel_t txrx_sel;
	FE_Err_type_t err_type;
	uint 	channel;
	uint 	index;
	uint 	reg_val;
	uint 	cnt;
	uint 	cnt_hi;
    uint 	timeStamp;
	union {
		FE_Enable_t enable;
		FE_Linkup_t link_mode;
		FE_PcpMode_t coding_mode;
		uint meter_rate;
		uint unknown_mul_pkt;
		FE_Reset_mode_t reset_mode;
		FE_PonMode_Sel_t pon_mode;
		struct {
			uint byteCnt_L;
			uint byteCnt_H;
		}byteCnt;
		struct {
			uint length_long;
			uint length_short;
		} pkt_len;
		struct {
			unchar *mac;
			ushort mask;
		} mac_addr;
		struct {
			uint mac_h;
			uint mac_lmin;
			uint mac_lmax;
		} mac_addr_7516;
		struct {
			FE_Enable_t wan1_en;
			FE_WanPort_Sel_t wan1_port;
			FE_WanPort_Sel_t wan0_port;
		} wan_port_7516;
		struct {
			FE_Frame_type_t dp_sel;
			FE_Frame_dp_t dp_val;
		} fw_cfg;
		struct {
			FE_Enable_t enable;
			FE_PPPOE_t is_pppoe;
			uint value;
		} eth_cfg;
		struct {
			FE_L2U_KEY_t key_sel;
			uint key0;
			uint key1;
		} l2u_key;
		struct {
			FE_Enable_t enable;
			FE_Patn_type patten_type;
			uint patten;
		} vip_cfg;
		struct {
			uint rate;
			uint mode;
			uint maxBkSize;
			uint tick;
		}rate_cfg;
		struct {
			ushort type;
			ushort src;
			ushort dst;
			unchar mode;
		}vip;
		struct {
			unchar mask;
			unchar dscp;
			ushort svlan;
			ushort cvlan;
		}l2lu;
		struct{
			unchar cmd;
			unchar tls_port;
			ushort tls_vlan;			
			FE_Tls_info_t tls_info;
		}tls_cfg;
		FE_RxCnt_t FE_RxCnt;
		FE_TxCnt_t FE_TxCnt;
		struct{
			void* skb;
			FE_Tls_forward_direction_t dir;
		}tls_forward;
		struct{
			FE_Random_mode_t channel_mode;
			FE_Random_mode_t length_mode;
			FE_Random_mode_t gap_mode;
		}lpbp_mode;
        Meter_cfg_t meter_cfg;
		FE_TrfcCfg_t fe_trfc_cfg;
		struct{
			FE_Rate_minus_t rate_minus;
			unchar rate_byte;
		}glo_rate_byte;
		FE_PppoeInfo_t pppoe_info;
		struct{
			int lan_port;
			union {
				uint rate;
				uint frameByteCnt;
			};
		} fe_cnt_str;
		struct {
			unsigned char mac[6];
		}dev_cfg;
		struct {
			int ip_type;  /* 0 means ipv4, 1 means ipv6 */
			union {
				unsigned int ip4;
				struct in6_addr ip6;
			};
		}stb_ip_cfg;
		struct{
			unsigned char pkt_formate;
			unsigned char mode;
		}pkt_ratelimit;
		FE_McVlanTableCfg_t fe_mcvlan_table_cfg;
		FE_McVlanActionCfg_t fe_mcvlan_action_cfg;
		FE_RxMacFilter_t macfilter_cfg;
		XFI_LINK_STATUS_t status;
		FE_RLS_MODE_t fe_rls_mode;
	} api_data;
	int retValue;
};

	
/************************************************************************
*                  D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N    D E C L A R A T I O N S
#                  I N L I N E    F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

#endif /* _ECNT_HOOK_QDMA_TYPE_H_ */

