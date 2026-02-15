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
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/mtd/rt_flash.h>
#include "ecnt_event_global/ecnt_event_system.h"

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#define BOOTARGS_TCLINUX_INFO_STR	("tclinux_info")
	
#define BOOTARGS_MAC_SIZE			(6)
#define BOOTARGS_MAC_STR			("ethaddr")

#define BOOTARGS_ONU_TYPE_STR		("onu_type")
#define BOOTARGS_QDMA_INIT_STR		("qdma_init")

#define BOOTARGS_GPON_SN_SIZE		(20)
#define BOOTARGS_GPON_SN_STR		("gpon_sn")

#define BOOTARGS_BOOTFLAG_STR		("bootflag")

#define BOOTARGS_SERDES_SEL_STR		("serdes_sel")

/************************************************************************
*                  M A C R O S
*************************************************************************
*/

/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/


/************************************************************************
*                  STATIC VARIABLE DECLARATIONS
*************************************************************************
*/
uint64_t tclinux_info[] = {TCLINUX_INFO_UNKNOW_VAL,	/* 0: tclinux.bin size */
						TCLINUX_INFO_UNKNOW_VAL,	/* 1: master kernel offset */
						TCLINUX_INFO_UNKNOW_VAL,	/* 2. master kernel size */
						TCLINUX_INFO_UNKNOW_VAL,	/* 3. master rootfs offset */
						TCLINUX_INFO_UNKNOW_VAL,	/* 4. master rootfs size */
						TCLINUX_INFO_UNKNOW_VAL,	/* 5: slave tclinux.bin size */
						TCLINUX_INFO_UNKNOW_VAL,	/* 6. slave kernel offset */
						TCLINUX_INFO_UNKNOW_VAL,	/* 7. slave kernel size */
						TCLINUX_INFO_UNKNOW_VAL,	/* 8. slave rootfs offset */
						TCLINUX_INFO_UNKNOW_VAL};	/* 9. slave rootfs size */

unsigned char tclinux_mac[BOOTARGS_MAC_SIZE] = {0x00, 0xAA, 0xBB, 0x01, 0x23, 0x40};
char onutype = 0;
char qdmainit = 0;
unsigned char gpon_sn[BOOTARGS_GPON_SN_SIZE];
char bootflag = 0;
char serdes_sel = 0;

/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
static int __init early_tclinux_info(char *p)
{

	unsigned long start, size;
	char *endp;
	int i;

	tclinux_info[0] = memparse(p, &endp);
	for(i = 1; *endp == ','; i++) {
		tclinux_info[i] = memparse(endp + 1, &endp);
	}

	return 0;
}
early_param(BOOTARGS_TCLINUX_INFO_STR, early_tclinux_info);

static int __init early_tclinux_mac(char *p)
{

	int ret;

	ret = sscanf(p, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
								&tclinux_mac[0],
								&tclinux_mac[1],
								&tclinux_mac[2],
								&tclinux_mac[3],
								&tclinux_mac[4],
								&tclinux_mac[5]);

	if(ret != BOOTARGS_MAC_SIZE) {
		printk("parse ethaddr error:%d\n", ret);
		return -1;
	}

	return 0;
}
early_param(BOOTARGS_MAC_STR, early_tclinux_mac);

int get_partition_info(unsigned int index, uint64_t *val)
{
	*val = tclinux_info[index];
	
	if(TCLINUX_INFO_UNKNOW_VAL == *val) {
		printk("Get partition error, index:%d!\n", index);
		return -1;
	}

	return 0;
}
EXPORT_SYMBOL(get_partition_info);

int get_ethaddr(unsigned char *ethaddr, int len)
{
	if(len != 6) {
		printk("Error get ethaddr length!\n");
		return -1;
	}

	if(ethaddr == NULL) {
		printk("Error! ethaddr is NULL.\n");
		return -1;
	}

	memcpy(ethaddr, tclinux_mac, len);

	return 0;
}
EXPORT_SYMBOL(get_ethaddr);

static int __init early_onutype(char *p)
{

	int ret;

	ret = sscanf(p, "%hhx", &onutype);

	if(ret != 1) {
		printk("parse onutype error:%d\n", ret);
		return -1;
	}

	return 0;
}
early_param(BOOTARGS_ONU_TYPE_STR, early_onutype);

char get_onutype(void)
{
	return onutype;
}
EXPORT_SYMBOL(get_onutype);

static int __init early_qdmainit(char *p)
{

	int ret;

	ret = sscanf(p, "%hhx", &qdmainit);
	if(ret != 1) {
		printk("parse qdma_init error:%d\n", ret);
		return -1;
	}

	return 0;
}
early_param(BOOTARGS_QDMA_INIT_STR, early_qdmainit);

char get_qdmainit(void)
{
	return qdmainit;
}
EXPORT_SYMBOL(get_qdmainit);

static int __init early_bootflag(char *p)
{

	int ret;

	ret = sscanf(p, "%hhx", &bootflag);
	if(ret != 1) {
		printk("parse bootflag error:%d\n", ret);
		return -1;
	}

	return 0;
}
early_param(BOOTARGS_BOOTFLAG_STR, early_bootflag);

char get_bootflag(void)
{
	return bootflag;
}
EXPORT_SYMBOL(get_bootflag);

static int __init early_serdes_sel(char *p)
{

	int ret;

	ret = sscanf(p, "%hhx", &serdes_sel);
	if(ret != 1) {
		printk("parse %s error:%d\n", BOOTARGS_SERDES_SEL_STR, ret);
		return -1;
	}

	return 0;
}
early_param(BOOTARGS_SERDES_SEL_STR, early_serdes_sel);

int get_serdes_sel(ECNT_EVENT_SYSTEM_SERDES_SEL_t sel)
{
	int bit_idx = (int)sel;
	
	if(sel >= ECNT_EVENT_SERDES_SEL_MAX) {
		return -1;
	}

	return ((serdes_sel & (0x1 << bit_idx)) >> bit_idx);
}
EXPORT_SYMBOL(get_serdes_sel);

