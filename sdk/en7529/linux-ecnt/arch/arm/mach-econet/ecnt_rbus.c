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
#include <linux/module.h>
#include <linux/types.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>

#include <asm/io.h>

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#ifdef TCSUPPORT_CPU_ARMV8
#define RBUS_CORE_BASE              (0x00000000)
#else
#define RBUS_CORE_BASE              (0xBFA00000)
#endif

#define CR_BUS_UTI_CTL             (RBUS_CORE_BASE+0xa0)
#define CR_BUS_UTI_STS             (RBUS_CORE_BASE+0xa4)
#define CR_BUS_UTI_VLD             (RBUS_CORE_BASE+0xa8)
#define CR_BUS_UTI_IDEL            (RBUS_CORE_BASE+0xb0)

#define CR_TIMEOUT_STS0		(RBUS_CORE_BASE + 0xd0)
#define CR_TIMEOUT_STS1		(RBUS_CORE_BASE + 0xd4)
#define CR_TIMEOUT_CFG0		(RBUS_CORE_BASE + 0xd8)
#define CR_TIMEOUT_CFG1		(RBUS_CORE_BASE + 0xdc)
#define CR_TIMEOUT_CFG2		(RBUS_CORE_BASE + 0xe0)
#define EMI_CLK_DOMAIN      (0x14D00000) /*3333 MHz*/

#define CR_BLOCK_EN     (RBUS_CORE_BASE+0x0ec)
#define CR_BLOCK_MASK   (RBUS_CORE_BASE+0x0f0)
#define CR_BLOCK_CNT0   (RBUS_CORE_BASE+0x0f4)  /* gdma,hsdma,woe,wdma,crypto,... */
#define CR_BLOCK_CNT1   (RBUS_CORE_BASE+0x0f8)  /* ppe */
#define CR_BLOCK_CNT2   (RBUS_CORE_BASE+0x0fc)  /* qdma_lan */
#define CR_BLOCK_CNT3   (RBUS_CORE_BASE+0x100)  /* qdma_wan */
#define CR_BLOCK_CNT4   (RBUS_CORE_BASE+0x104)  /* tdma */
#define CR_BLOCK_CNT5   (RBUS_CORE_BASE+0x108)  /* npu */
#define MAX_BLOCK_CNT   (6)

/************************************************************************
*                  M A C R O S
*************************************************************************
*/


/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/

struct ecnt_rbus {
	struct device *dev;
	void __iomem *base;
};


/************************************************************************
*                  STATIC VARIABLE DECLARATIONS
*************************************************************************
*/
struct ecnt_rbus *ecnt_rbus = NULL;


static const struct of_device_id ecnt_rbus_of_id[] = {
    { .compatible = "econet,ecnt-rbus"},
    { /* sentinel */}
};
MODULE_DEVICE_TABLE(of, ecnt_rbus_of_id);

/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
/* don't EXPORT this function. Create API for your purpose instead. */
static u32 get_rbus_data(u32 reg)
{
    return readl(ecnt_rbus->base + reg);
}

/* don't EXPORT this function. Create API for your purpose instead. */
static void set_rbus_data(u32 reg, u32 val)
{
    writel(val, ecnt_rbus->base + reg); 
}

void ENABLE_BUS_UTIL(void)
{
    set_rbus_data(CR_BUS_UTI_CTL, 0x1);
    return;
}
EXPORT_SYMBOL(ENABLE_BUS_UTIL);

void DISABLE_BUS_UTIL(void)
{
    set_rbus_data(CR_BUS_UTI_CTL, 0x2);
    return;
}
EXPORT_SYMBOL(DISABLE_BUS_UTIL);

int IS_BUS_UTIL_DONE(void)
{
    return (get_rbus_data(CR_BUS_UTI_STS)&0x1);
}
EXPORT_SYMBOL(IS_BUS_UTIL_DONE);

void GET_BUS_UTIL_CNT(u32 *validCnt, u32 *idleCnt)
{
    *validCnt = get_rbus_data(CR_BUS_UTI_VLD);
    *idleCnt = get_rbus_data(CR_BUS_UTI_IDEL);
    return;
}
EXPORT_SYMBOL(GET_BUS_UTIL_CNT);

void enable_block_dma_mechanism(unsigned int enable)
{
    u32 val = get_rbus_data(CR_BLOCK_EN);

    if(enable)
        val |= 0x1;
    else /* disable */
        val &= (~0x1);

    set_rbus_data(CR_BLOCK_EN, val);
}
EXPORT_SYMBOL(enable_block_dma_mechanism);

/* Note: 
 * if mask range is 64 bytes, parameter "mask" will be 0x3f and register CR_BLOCK_MASK will be 0xffffffc0
 * if mask range is 128 bytes, parameter "mask" will be 0x7f and register CR_BLOCK_MASK will be 0xffffff80 
 */
void set_block_dma_mask(unsigned int mask)
{
    unsigned int old_mask = get_rbus_data(CR_BLOCK_MASK);
    unsigned int val= 0xffffffff;

    val &= (~mask);
    set_rbus_data(CR_BLOCK_MASK, val);

    printk("old_blk_mask:0x%x, new_blk_mask:0x%x\n", old_mask, get_rbus_data(CR_BLOCK_MASK));
}
EXPORT_SYMBOL(set_block_dma_mask);

unsigned int get_block_dma_mask(void)
{
    return get_rbus_data(CR_BLOCK_MASK);
}
EXPORT_SYMBOL(get_block_dma_mask);

unsigned int get_block_dma_counter(int cnt_no)
{
    if ((cnt_no>=MAX_BLOCK_CNT) || (cnt_no<0)) {
        printk("cnt_no:%d is wrong! Correct value: 0~5\n", cnt_no);
        return 0;
    }

    return get_rbus_data(CR_BLOCK_CNT0+(cnt_no<<2));
}
EXPORT_SYMBOL(get_block_dma_counter);

static char* blk_dbg_reg_name [] = {
        "Block_en",     "Block_Mask",   "Block_Cnt0",   "Block_Cnt1", 
        "Block_Cnt2",   "Block_Cnt3",   "Block_Cnt4",   "Block_Cnt5",
        "Block_Cnt6",   "WtBuff0",      "WtBuff1",      "WtBuff2",
        "WtBuff3",      "WtBuff4",      "WtBuff5",      "WtBuff6",
        "WtBuff7",      "WB_ValidBits", "DMA_Port0",    "DMA_Port1",
        "DMA_Port2",    "DMA_Port3",    "DMA_Port4",    "DMA_Port5",
        "DMA_Port6"
};

void show_block_dbg_regs(void)
{
    unsigned int offset;
    int i=0;

    printk("%s: (base_addr: 0x1fa00000)\n", __func__);
    for (offset=0x0ec; offset<=0x14c; ) {
        printk("\t0x%x: 0x%x (%s)\n", offset, get_rbus_data(RBUS_CORE_BASE+offset), blk_dbg_reg_name[i]);
        offset+=4;
        i++;
    }
    
    return;
}
EXPORT_SYMBOL(show_block_dbg_regs);

static void rbus_timeout_init(void)
{
    /* set cmd/wdata/rdata timeout_cnt as 100 ms */
    set_rbus_data(CR_TIMEOUT_CFG0, EMI_CLK_DOMAIN/10);
    set_rbus_data(CR_TIMEOUT_CFG1, EMI_CLK_DOMAIN/10); 
    set_rbus_data(CR_TIMEOUT_CFG2, EMI_CLK_DOMAIN/10); 

    /* enable timeout.
     * Note: ISR won't work due to data/bss section in DMEM */
    set_rbus_data(CR_TIMEOUT_STS0, 0x80000000); 
    
    return;
}

static int ecnt_rbus_drv_probe(struct platform_device *pdev)
{
    struct resource *res = NULL;

    if (!pdev->dev.of_node) {
        dev_err(&pdev->dev, "No rbus DT node found");
        return -EINVAL;
    }

    ecnt_rbus = devm_kzalloc(&pdev->dev, sizeof(struct ecnt_rbus), GFP_KERNEL);
    if (!ecnt_rbus)
        return -ENOMEM;

    platform_set_drvdata(pdev, ecnt_rbus);

    /* get RBUS base address */
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    ecnt_rbus->base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(ecnt_rbus->base))
        return PTR_ERR(ecnt_rbus->base);

    ecnt_rbus->dev = &pdev->dev;

    rbus_timeout_init();

#if 0
	printk("[rbus] res->name:%s\n", res->name);
	printk("[rbus] res->start:0x%llx ===\n", res->start);
	printk("[rbus] res->end:0x%llx ===\n", res->end);
	printk("[rbus] ecnt_rbus->base:0x%lx\n", (unsigned long)ecnt_rbus->base);
#endif

    return 0;
}

static int ecnt_rbus_drv_remove(struct platform_device *pdev)
{
    return 0;
}


/************************************************************************
*      P L A T F O R M   D R I V E R S   D E C L A R A T I O N S
*************************************************************************
*/
static struct platform_driver ecnt_rbus_driver = {
    .probe = ecnt_rbus_drv_probe,
    .remove = ecnt_rbus_drv_remove,
    .driver = {
        .name = "ecnt-rbus",
        .of_match_table = ecnt_rbus_of_id
    },
};
module_platform_driver(ecnt_rbus_driver);


MODULE_DESCRIPTION("EcoNet RBUS Driver");


