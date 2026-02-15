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
#define CR_FE_PPE_PHY_BASE		(0x1fb50000)
#define CR_FE_PPE_PHY_RANGE		(0x2600)
#define CR_FE_PPE_PHY_END		(CR_FE_PPE_PHY_BASE + CR_FE_PPE_PHY_RANGE)

#define CR_QDMA_PHY_BASE		(0x1fb54000)
#define CR_QDMA_PHY_RANGE		(0x4000)
#define CR_QDMA_PHY_END			(CR_QDMA_PHY_BASE + CR_QDMA_PHY_RANGE)

#define CR_SWITCH_PHY_BASE		(0x1fb58000)
#define CR_SWITCH_PHY_RANGE		(0x8000)
#define CR_SWITCH_PHY_END		(CR_SWITCH_PHY_BASE + CR_SWITCH_PHY_RANGE)

#define QDMA_LAN_INT_MAX_NUM		(4)
#define QDMA_WAN_INT_MAX_NUM		(4)
#define QDMA_ALL_INT_MAX_NUM		(8)

#define FE_ALL_INT_MAX_NUM			(1)

#define PDMA_ALL_INT_MAX_NUM		(1)

#define FRAME_ENGINE_INT_MAX_NUM	(10)

/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/

struct ecnt_frame_engine_str {
	struct device *dev;
	void __iomem *fe_ppe_base;
	void __iomem *qdma_base;
	void __iomem *switch_base;

	int qdma_irq[QDMA_ALL_INT_MAX_NUM];
	int fe_irq;
	int pdma_irq;
};

/************************************************************************
*                  STATIC VARIABLE DECLARATIONS
*************************************************************************
*/
phys_addr_t qdmalan_buffer_base = 0;
phys_addr_t qdmawan_buffer_base = 0;

struct ecnt_frame_engine_str *ecnt_frame_engine = NULL;


static const struct of_device_id ecnt_frame_engine_of_id[] = {
    { .compatible = "econet,ecnt-frame_engine"},
    { /* sentinel */}
};
MODULE_DEVICE_TABLE(of, ecnt_frame_engine_of_id);

/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
/* don't EXPORT this function. Create API for your purpose instead. */
u32 get_fe_ppe_data(u32 reg)
{
    return readl(ecnt_frame_engine->fe_ppe_base + reg);
}

/* don't EXPORT this function. Create API for your purpose instead. */
void set_fe_ppe_data(u32 reg, u32 val)
{
    writel(val, ecnt_frame_engine->fe_ppe_base + reg); 
}

/* don't EXPORT this function. Create API for your purpose instead. */
u32 get_qdma_data(u32 reg)
{
    return readl(ecnt_frame_engine->qdma_base + reg);
}

/* don't EXPORT this function. Create API for your purpose instead. */
void set_qdma_data(u32 reg, u32 val)
{
    writel(val, ecnt_frame_engine->qdma_base + reg); 
}

/* don't EXPORT this function. Create API for your purpose instead. */
u32 get_switch_data(u32 reg)
{
    return readl(ecnt_frame_engine->switch_base + reg);
}

/* don't EXPORT this function. Create API for your purpose instead. */
void set_switch_data(u32 reg, u32 val)
{
    writel(val, ecnt_frame_engine->switch_base + reg); 
}

/* APIs */
u32 get_frame_engine_data(u32 reg)
{
	u32 reg_phy = 0;
	u32 reg_offset = 0;

	/* translate addr to physical addr */
	if( reg > 0xa0000000)
		reg_phy = (reg & 0x1fffffff);
	else
		reg_phy = reg;
	
	reg_offset = reg_phy % 4;
	if(reg_offset != 0){
		printk("\nDatapath(%s) get reg error, reg=0x%08X\n", __func__, reg);
		return 0;
	}
	
	if( (CR_FE_PPE_PHY_BASE <= reg_phy) && (reg_phy < CR_FE_PPE_PHY_END) )
    	return get_fe_ppe_data(reg_phy - CR_FE_PPE_PHY_BASE);
	else if( (CR_QDMA_PHY_BASE <= reg_phy) && (reg_phy < CR_QDMA_PHY_END) )
    	return get_qdma_data(reg_phy - CR_QDMA_PHY_BASE);
	else if( (CR_SWITCH_PHY_BASE <= reg_phy) && (reg_phy < CR_SWITCH_PHY_END) )
    	return get_switch_data(reg_phy - CR_SWITCH_PHY_BASE);
	else
		printk("\nDatapath(%s) get reg error, reg=0x%08X\n", __func__, reg);

	return 0;
}
EXPORT_SYMBOL(get_frame_engine_data);

void set_frame_engine_data(u32 reg, u32 val)
{
	u32 reg_phy = 0;
	u32 reg_offset = 0;

	/* translate addr to physical addr */
	if( reg > 0xa0000000)
		reg_phy = (reg & 0x1fffffff);
	else
		reg_phy = reg;

	reg_offset = reg_phy % 4;
	if(reg_offset != 0){
		printk("\nDatapath(%s) set reg error, reg=0x%08X\n", __func__, reg);
		return 0;
	}

	if( (CR_FE_PPE_PHY_BASE <= reg_phy) && (reg_phy < CR_FE_PPE_PHY_END) )
    	set_fe_ppe_data(reg_phy - CR_FE_PPE_PHY_BASE, val); 
	else if( (CR_QDMA_PHY_BASE <= reg_phy) && (reg_phy < CR_QDMA_PHY_END) )
    	set_qdma_data(reg_phy - CR_QDMA_PHY_BASE, val); 
	else if( (CR_SWITCH_PHY_BASE <= reg_phy) && (reg_phy < CR_SWITCH_PHY_END) )
    	set_switch_data(reg_phy - CR_SWITCH_PHY_BASE, val); 
	else	
		printk("\nDatapath(%s) set reg error, reg=0x%08X\n", __func__, reg);

}
EXPORT_SYMBOL(set_frame_engine_data);

struct device* get_frame_engine_dev(void)
{
	if( (ecnt_frame_engine) && (ecnt_frame_engine->dev) )
		return ecnt_frame_engine->dev;
	else
		return NULL;
}
EXPORT_SYMBOL(get_frame_engine_dev);

int get_qdma_lan_irq(int index)
{
	if( index < QDMA_LAN_INT_MAX_NUM )
		return ecnt_frame_engine->qdma_irq[index];
	else
		printk("\nDatapath(%s) get_qdma_lan_irq error, index=%d\n", __func__, index);

	return 0;
}
EXPORT_SYMBOL(get_qdma_lan_irq);

int get_qdma_wan_irq(int index)
{
	if( index < QDMA_WAN_INT_MAX_NUM )
		return ecnt_frame_engine->qdma_irq[index + QDMA_LAN_INT_MAX_NUM];
	else
		printk("\nDatapath(%s) get_qdma_lan_irq error, index=%d\n", __func__, index);

	return 0;
}
EXPORT_SYMBOL(get_qdma_wan_irq);

int get_fe_irq(void)
{
	return ecnt_frame_engine->fe_irq;
}
EXPORT_SYMBOL(get_fe_irq);

int get_pdma_irq(void)
{
	return ecnt_frame_engine->pdma_irq;
}
EXPORT_SYMBOL(get_pdma_irq);

/* ------------------------- For QDMA buffer init, start ------------------------- */
unsigned int ecnt_qdma_lan_get_buffer_size(void)
{
	unsigned int buffer_size = 0;
	char qdma_init_value = get_qdmainit();
	/*--16K*2048, 16K*1024, 16K*512, 16K*256--*/
	unsigned int qdma_lan_buffer_size[4] = {32, 16, 8, 4};

	buffer_size = qdma_lan_buffer_size[qdma_init_value & 0x3] << 20;

	return buffer_size;
}
EXPORT_SYMBOL(ecnt_qdma_lan_get_buffer_size);

unsigned int ecnt_qdma_wan_get_buffer_size(void)
{
	unsigned int buffer_size = 0;
	char qdma_init_value = get_qdmainit();
	/*--16K*2048, 16K*1024, 16K*512, 16K*256--*/
	unsigned int qdma_wan_buffer_size[4] = {32, 16, 8, 4};

	buffer_size = qdma_wan_buffer_size[(qdma_init_value >> 4) & 0x3] << 20;

	return buffer_size;
}
EXPORT_SYMBOL(ecnt_qdma_wan_get_buffer_size);

void ecnt_qdma_lan_set_buffer_base(phys_addr_t buffer_base)
{
	qdmalan_buffer_base = buffer_base;
}
EXPORT_SYMBOL(ecnt_qdma_lan_set_buffer_base);

unsigned int ecnt_qdma_lan_get_buffer_base(void)
{
	return (unsigned int)qdmalan_buffer_base;
}
EXPORT_SYMBOL(ecnt_qdma_lan_get_buffer_base);

void ecnt_qdma_wan_set_buffer_base(phys_addr_t buffer_base)
{
	qdmawan_buffer_base = buffer_base;
}
EXPORT_SYMBOL(ecnt_qdma_wan_set_buffer_base);

unsigned int ecnt_qdma_wan_get_buffer_base(void)
{
	return (unsigned int)qdmawan_buffer_base;
}
EXPORT_SYMBOL(ecnt_qdma_wan_get_buffer_base);
/* ------------------------- For QDMA buffer init, end ------------------------- */

static int ecnt_frame_engine_probe(struct platform_device *pdev)
{
    struct resource *res = NULL;
	int irq = 0;
	int irq_idx = 0;

    if (!pdev->dev.of_node) {
        dev_err(&pdev->dev, "No frame engine DT node found");
        return -EINVAL;
    }

    ecnt_frame_engine = devm_kzalloc(&pdev->dev, sizeof(struct ecnt_frame_engine_str), GFP_KERNEL);
    if (!ecnt_frame_engine)
        return -ENOMEM;

    platform_set_drvdata(pdev, ecnt_frame_engine);

    /* get FE+PPE base address */
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    ecnt_frame_engine->fe_ppe_base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(ecnt_frame_engine->fe_ppe_base))
        return PTR_ERR(ecnt_frame_engine->fe_ppe_base);

	/* get QDMA base address */
    res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
    ecnt_frame_engine->qdma_base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(ecnt_frame_engine->qdma_base))
        return PTR_ERR(ecnt_frame_engine->qdma_base);

	/* get switch base address */
    res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
    ecnt_frame_engine->switch_base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(ecnt_frame_engine->switch_base))
        return PTR_ERR(ecnt_frame_engine->switch_base);

	/* get irq num */
	for( irq_idx = 0; irq_idx < FRAME_ENGINE_INT_MAX_NUM; irq_idx++ ){
		irq = platform_get_irq(pdev, irq_idx);
		if (irq <= 0)
			return irq;

		if( irq_idx < QDMA_ALL_INT_MAX_NUM )
			ecnt_frame_engine->qdma_irq[irq_idx] = irq;
		else if( irq_idx < (QDMA_ALL_INT_MAX_NUM + FE_ALL_INT_MAX_NUM))
			ecnt_frame_engine->fe_irq = irq;
		else
			ecnt_frame_engine->pdma_irq = irq;
	}

    ecnt_frame_engine->dev = &pdev->dev;

    return 0;
}

static int ecnt_frame_engine_remove(struct platform_device *pdev)
{
    return 0;
}


/************************************************************************
*      P L A T F O R M   D R I V E R S   D E C L A R A T I O N S
*************************************************************************
*/

static struct platform_driver ecnt_frame_engine_driver = {
    .probe = ecnt_frame_engine_probe,
    .remove = ecnt_frame_engine_remove,
    .driver = {
        .name = "ecnt-frame_engine",
        .of_match_table = ecnt_frame_engine_of_id
    },
};
module_platform_driver(ecnt_frame_engine_driver);


MODULE_DESCRIPTION("EcoNet Frame_Engine Driver");


