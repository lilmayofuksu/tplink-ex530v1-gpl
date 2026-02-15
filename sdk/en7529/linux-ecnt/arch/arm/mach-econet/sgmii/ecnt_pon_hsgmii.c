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
#include <linux/interrupt.h>
#include <linux/proc_fs.h>

#include <asm/io.h>
#include "ecnt_event_global/ecnt_event_system.h"

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

/************************************************************************
*                  M A C R O S
*************************************************************************
*/
#define PON_PCS1_BASE_OFFSET 0x100
#define PON_INT_EN 0x5f0
#define PON_INT_STA_CLR 0x5f4
#define PON_INT_STA 0x5f8

#define  HSGMII_ETHER_WAN_PROC "tc3162/wan_2_5"

/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/
struct ecnt_hsgmii {
	struct device *dev;
	void __iomem *pcs1_base;
	void __iomem *pcs2_base;
	void __iomem *an_base;
	void __iomem *ra_base;
	int irq;
};

/************************************************************************
*                  STATIC VARIABLE DECLARATIONS
*************************************************************************
*/
struct ecnt_hsgmii *ecnt_pon_hsgmii = NULL;

static const struct of_device_id ecnt_pon_hsgmii_of_id[] = {
    { .compatible = "econet,ecnt-pon_hsgmii"},
    { /* sentinel */}
};
MODULE_DEVICE_TABLE(of, ecnt_pon_hsgmii_of_id);

static int wanEtherSupported = 0;

/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
/*****************************************************************
 ****  N P    S C U    a c c e s s ********************************
 ******************************************************************/
 
u32 get_pon_pcs1_data(u32 reg)
{
	return readl(ecnt_pon_hsgmii->pcs1_base + (reg - PON_PCS1_BASE_OFFSET));
}

void set_pon_pcs1_data(u32 reg, u32 val)
{
	writel(val, (ecnt_pon_hsgmii->pcs1_base + (reg - PON_PCS1_BASE_OFFSET))); 
}

u32 get_pon_pcs2_data(u32 reg)
{
    return readl(ecnt_pon_hsgmii->pcs2_base + reg);
}
EXPORT_SYMBOL(get_pon_pcs2_data);

void set_pon_an_data(u32 reg, u32 val)
{
	writel(val, ecnt_pon_hsgmii->an_base + reg); 
}
EXPORT_SYMBOL(set_pon_an_data);

u32 get_pon_an_data(u32 reg)
{
	return readl(ecnt_pon_hsgmii->an_base + reg);
}
EXPORT_SYMBOL(get_pon_an_data);

u32 get_pon_ra_data(u32 reg)
{
    return readl(ecnt_pon_hsgmii->ra_base + reg);
}

/* APIs */
static void pon_hsgmii_int_init(void)
{
	//enable lof & phyrdy int
	set_pon_pcs1_data(PON_INT_EN, 0x22);
}

static irqreturn_t pon_hsgmii_interrupt(int irq, void *dev_id)
{
	u32 pcs1_int_sts=0;

	/* read to clear interrupt */
	pcs1_int_sts = get_pon_pcs1_data(PON_INT_STA);
	set_pon_pcs1_data(PON_INT_STA_CLR, 0x3ff);

	printk("pon hsgmii interrupt status=0x%x\n", pcs1_int_sts);
	
	return IRQ_HANDLED;
}

#if defined(TCSUPPORT_CPU_EN7523)
static int hsgmii_ether_wan_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int index = 0;
	
	index = sprintf(buf, "%d\n", wanEtherSupported);

	index -= off;
	*start = buf + off;

	if (index > count)
		index = count;
	else
		*eof = 1;

	if (index < 0)
		index = 0;

	return index;
}
#endif

static int ecnt_pon_hsgmii_drv_probe(struct platform_device *pdev)
{
    struct resource *res = NULL;
	int ret = 0;
	
    if (!pdev->dev.of_node) {
        dev_err(&pdev->dev, "No pon_hsgmii DT node found");
        return -EINVAL;
    }

    ecnt_pon_hsgmii = devm_kzalloc(&pdev->dev, sizeof(struct ecnt_hsgmii), GFP_KERNEL);
    if (!ecnt_pon_hsgmii)
        return -ENOMEM;

    platform_set_drvdata(pdev, ecnt_pon_hsgmii);

    /* get pcs1 base address */
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    ecnt_pon_hsgmii->pcs1_base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(ecnt_pon_hsgmii->pcs1_base))
        return PTR_ERR(ecnt_pon_hsgmii->pcs1_base);
	
    /* get pcs2 base address */	
	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
   	ecnt_pon_hsgmii->pcs2_base = devm_ioremap_resource(&pdev->dev, res);
   	if (IS_ERR(ecnt_pon_hsgmii->pcs2_base))
	   return PTR_ERR(ecnt_pon_hsgmii->pcs2_base);

	/* get AN base address */	
	res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
   	ecnt_pon_hsgmii->an_base = devm_ioremap_resource(&pdev->dev, res);
   	if (IS_ERR(ecnt_pon_hsgmii->an_base))
	   return PTR_ERR(ecnt_pon_hsgmii->an_base);

	/* get RA base address */	
	res = platform_get_resource(pdev, IORESOURCE_MEM, 3);
   	ecnt_pon_hsgmii->ra_base = devm_ioremap_resource(&pdev->dev, res);
   	if (IS_ERR(ecnt_pon_hsgmii->ra_base))
	   return PTR_ERR(ecnt_pon_hsgmii->ra_base);

    ecnt_pon_hsgmii->dev = &pdev->dev;

	/* get irq num */
	ecnt_pon_hsgmii->irq = platform_get_irq(pdev, 0);
	if(ecnt_pon_hsgmii->irq <= 0) {
		printk("\n get pon hsgmii irq number failed\n");
		return ecnt_pon_hsgmii->irq;
	}

	//pon_hsgmii_int_init();
	
	/* request irq */
	/*ret = request_irq(ecnt_pon_hsgmii->irq, pon_hsgmii_interrupt, 0, "PonHsgmiiInt", ecnt_pon_hsgmii->dev);
	if(ret) {
		printk("\n request_irq() (irq number: %d) failed (ret: %d)\n", ecnt_pon_hsgmii->irq, ret);
		return (ret);
	}
    */

	wanEtherSupported = (((int)get_serdes_sel(ECNT_EVENT_PON_HSGMII)) == 1) ? 1 : 0;
	
	if(wanEtherSupported) {
		create_proc_read_entry(HSGMII_ETHER_WAN_PROC, 0, NULL, hsgmii_ether_wan_read_proc, NULL);
	}
    return 0;
}
static int ecnt_pon_hsgmii_drv_remove(struct platform_device *pdev)
{
	if(wanEtherSupported) {
		remove_proc_entry(HSGMII_ETHER_WAN_PROC, 0);
	}
    return 0;
}

/************************************************************************
*      P L A T F O R M   D R I V E R S   D E C L A R A T I O N S
*************************************************************************
*/
static struct platform_driver ecnt_pon_hsgmii_driver = {
    .probe = ecnt_pon_hsgmii_drv_probe,
    .remove = ecnt_pon_hsgmii_drv_remove,
    .driver = {
	    .name = "ecnt-pon_hsgmii",
	    .of_match_table = ecnt_pon_hsgmii_of_id
    },
};
module_platform_driver(ecnt_pon_hsgmii_driver);


MODULE_DESCRIPTION("EcoNet pon HSGMII Driver");

