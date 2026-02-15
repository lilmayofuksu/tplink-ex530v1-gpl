#ifndef _NPU_HOST_ADPT_H_
#define _NPU_HOST_ADPT_H_

#define dma_addr_t  unsigned int
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int

#define HOSTADPT_RX_DSCP_WORD_LENS    4
#define HOSTADPT_RX_RING_NUM          2
#define HOSTADPT_RX0_DSCP_NUM         512
#define HOSTADPT_RX1_DSCP_NUM         512
#define HOSTADPT_RX_TOTAL_DSCP_NUM    (HOSTADPT_RX0_DSCP_NUM + HOSTADPT_RX1_DSCP_NUM)
#define HOSTAPD_BUFFER_LEN		3500
#define HOSTADPT_RX_RING_0            0
#define HOSTADPT_RX_RING_1            1

//#define HOSTADPT_TEST

#define DBG_OFF     0
#define DBG_ERR     1
#define DBG_WRN     2
#define DBG_LOG     3

#define DBG_LEVEL   DBG_OFF


typedef struct {
    
    struct {
		u32 done               : 1 ;
		u32 cur_len             : 13;
		u32 len                : 16;
		u32 lastflag           : 1;
		u32 resv1	           : 1 ;
    } pkt_ctrl ;
    struct {
		u32 foe_number         : 15;
		u32 crsn               : 5 ;
		u32 src_port		   : 5 ;
		u32 is_last              : 7;
    } pkt_info;
    u32 	pkt_addr ;
	u32    *skb_p;
} HOSTADPT_DMA_DSCP_T;
#endif
