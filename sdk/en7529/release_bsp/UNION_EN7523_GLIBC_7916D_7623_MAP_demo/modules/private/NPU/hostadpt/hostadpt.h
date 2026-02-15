#ifndef _HOSTADPT_H_
#define _HOSTADPT_H_

/***************************************************************
*						INCLUDE
***************************************************************/
#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/dma-mapping.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <modules/npu/npu_host_adpt.h>


/***************************************************************
*						DEFINES & CONSTANTS
***************************************************************/

/* reg addr */

#define HOSTADPT_RX0_BASE_PTR   (0x180)     /*RX Ring 0 Base pointer*/
#define HOSTADPT_RX0_MAX_CNT    (0x184)
#define HOSTADPT_RX0_DMA_IDX    (0x188)
#define HOSTADPT_RX0_CPU_IDX    (0x18C)

#define HOSTADPT_RX1_BASE_PTR   (0x190)     /*RX Ring 1 Base pointer*/
#define HOSTADPT_RX1_MAX_CNT    (0x194)
#define HOSTADPT_RX1_DMA_IDX    (0x198)
#define HOSTADPT_RX1_CPU_IDX    (0x19C)

/* interrupt */
#define HOSTADPT_TXRXDONE0_INT_MASK       (0x34) /*32bit: Tx0---Tx15,Rx0---Rx15*/
#define HOSTADPT_TXRXDONE1_INT_MASK       (0x38) /*32bit: Tx0---Tx15,Rx0---Rx15*/

#define HOSTADPT_TX0_INT  (1<<0)
#define HOSTADPT_TX1_INT  (1<<1)
#define HOSTADPT_TX2_INT  (1<<2)
#define HOSTADPT_TX3_INT  (1<<3)
#define HOSTADPT_TX4_INT  (1<<4)
#define HOSTADPT_TX5_INT  (1<<5)
#define HOSTADPT_TX6_INT  (1<<6)
#define HOSTADPT_TX7_INT  (1<<7)
#define HOSTADPT_TX8_INT  (1<<8)
#define HOSTADPT_TX9_INT  (1<<9)
#define HOSTADPT_TX10_INT (1<<10)
#define HOSTADPT_TX11_INT (1<<11)
#define HOSTADPT_TX12_INT (1<<12)
#define HOSTADPT_TX13_INT (1<<13)
#define HOSTADPT_TX14_INT (1<<14)
#define HOSTADPT_TX16_INT (1<<15)

#define HOSTADPT_RX0_INT  (1<<0)
#define HOSTADPT_RX1_INT  (1<<1)
#define HOSTADPT_RX2_INT  (1<<2)
#define HOSTADPT_RX3_INT  (1<<3)
#define HOSTADPT_RX4_INT  (1<<4)
#define HOSTADPT_RX5_INT  (1<<5)
#define HOSTADPT_RX6_INT  (1<<6)
#define HOSTADPT_RX7_INT  (1<<7)
#define HOSTADPT_RX8_INT  (1<<8)
#define HOSTADPT_RX9_INT  (1<<9)
#define HOSTADPT_RX10_INT (1<<10)
#define HOSTADPT_RX11_INT (1<<11)
#define HOSTADPT_RX12_INT (1<<12)
#define HOSTADPT_RX13_INT (1<<13)
#define HOSTADPT_RX14_INT (1<<14)
#define HOSTADPT_RX15_INT (1<<15)

#define HOSTADPT_MSG(level, F, B...)	{ \
											if(DBG_LEVEL >= level)	\
												printk("%s: %s [%d]: " F, "host adaptor", strrchr(__FILE__, '/')+1, __LINE__, ##B) ; \
										}



extern struct sk_buff *(*fromHostadptPktHandle_hook)(unsigned int ringIdx, unsigned char* preschedule);
extern void (*hostdapt_enable_int_hook)(unsigned int ringIdx);
extern void (*hostdapt_disable_int_hook)(unsigned int ringIdx);
extern void (*hostdapt_registe_wifitask_hook)(unsigned int ringIdx, void *func);

#endif