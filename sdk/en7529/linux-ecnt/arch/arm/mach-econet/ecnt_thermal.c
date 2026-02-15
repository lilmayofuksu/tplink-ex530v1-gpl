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
#include <asm/tc3162/tc3162.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>

#include <asm/io.h>


/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#define DUMMY_REG 0xf20 //PTPSPARE0 debug 0x1efb d000

/************************************************************************
*                  M A C R O S
*************************************************************************
*/


/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/
struct ecnt_thermal_phy {
	struct device *dev;
	//void __iomem *thermal_base;
	void __iomem *ptp_base;
};

/************************************************************************
*                  STATIC VARIABLE DECLARATIONS
*************************************************************************
*/
struct ecnt_thermal_phy *thermal_phy = NULL;

static const struct of_device_id ecnt_thermal_phy_of_id[] = {
    { .compatible = "econet,ecnt-thermal_phy"},
    { /* sentinel */}
};
MODULE_DEVICE_TABLE(of, ecnt_thermal_phy_of_id);
/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
/*****************************************************************
 ****  Reg   a c c e s s ********************************
 ******************************************************************/
// u32 get_efuse_data(u32 addr)
//{
//	return 0;
//}


u32 get_ptp_dummy(void)
{
	return readl(thermal_phy->ptp_base + (DUMMY_REG));
}

EXPORT_SYMBOL(get_ptp_dummy);


/* APIs */

static int thermal_phy_drv_probe(struct platform_device *pdev)
{
	struct resource *res = NULL;
	int ret = 0;
	
    if (!pdev->dev.of_node) {
        dev_err(&pdev->dev, "No thermal_phy DT node found");
        return -EINVAL;
    }

    thermal_phy = devm_kzalloc(&pdev->dev, sizeof(struct ecnt_thermal_phy), GFP_KERNEL);
    if (!thermal_phy)
        return -ENOMEM;

    platform_set_drvdata(pdev, thermal_phy);

    /* get uphy base address */
/*	
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    thermal_phy->thermal_base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(thermal_phy->thermal_base))
        return PTR_ERR(thermal_phy->thermal_base);
*/
 res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    thermal_phy->ptp_base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(thermal_phy->ptp_base))
        return PTR_ERR(thermal_phy->ptp_base);

    thermal_phy->dev = &pdev->dev;	
	
	if(isEN7523)
	{		
		printk("7523 Thermal Sensor address probe done\n");
	}
	
    return 0;
}


static int thermal_phy_drv_remove(struct platform_device *pdev)
{
	if(thermal_phy)  {
  		printk("thermal_phy_drv_remove\n");
	}
    return 0;
}

/************************************************************************
*      P L A T F O R M   D R I V E R S   D E C L A R A T I O N S
*************************************************************************
*/
static struct platform_driver thermal_phy_driver = {
    .probe = thermal_phy_drv_probe,
    .remove = thermal_phy_drv_remove,
    .driver = {
	    .name = "ecnt-thermal_phy",
	    .of_match_table = ecnt_thermal_phy_of_id
    },
};

//builtin_platform_driver(thermal_phy_driver);
module_platform_driver(thermal_phy_driver); 

MODULE_DESCRIPTION("EcoNet thermal Driver");


