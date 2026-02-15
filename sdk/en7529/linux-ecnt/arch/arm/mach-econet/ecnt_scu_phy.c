#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
#include <linux/module.h>
#include <linux/types.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#else
#include <asm/tc3162.h>
#endif
#include <asm/io.h>
#include <modules/avs/avs.h>


//RG address
#define MUX_TADC       		0x2ec
#define PLLRG_PROTECT      0x264
#define DOUT_TADC      		0x2f0
#define SCU_XTAL_SEL	0x254
//AVS
#define RG_AVS_OUT_EN		0x298
#define RG_DATA_AVS_DAC		0x2e8
#define RG_MUX_TADC			0x2ec

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
extern u32 get_chip_scu_data(u32 reg);
extern void set_chip_scu_data(u32 reg, u32 val);
#else
#define get_chip_scu_data(reg) readl((0x1fa20000 + reg))
#define set_chip_scu_data(reg, val) writel(val, (0x1fa20000 + reg))

#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
//------------------thermal sensor API----------------
void thermal_init(void)
{
	//enable write protect reg, set 0x1fa20264 = 0x80
	set_chip_scu_data(PLLRG_PROTECT,  0x80);
	//set TADC related registers
	//reset TADC, 0x1fa202ec = [7] = 0, 0x1fa202ec = [7] = 1
	set_chip_scu_data(MUX_TADC,  (get_chip_scu_data(MUX_TADC) &(~(0x1 <<7)) ) | (0x0<<7) );
	set_chip_scu_data(MUX_TADC,  (get_chip_scu_data(MUX_TADC) &(~(0x1 <<7)) ) | (0x1<<7) );
	//set TADC mode, 0x1fa202ec[3] = 0; [6] = 1, [5:4] = 3, [2:0] =0.  7'b111  0000
	set_chip_scu_data(MUX_TADC,  (get_chip_scu_data(MUX_TADC) &(~(0x7f <<0)) ) | (0x70<<0) );
}
EXPORT_SYMBOL(thermal_init);

void thermal_TADC_mode(void)
{	
	//set TADC mode, 0x1fa202ec[3] = 0; [6] = 1, [5:4] = 3, [2:0] =0.
	set_chip_scu_data(MUX_TADC,  (get_chip_scu_data(MUX_TADC) &(~(0x7f <<0)) ) | (0x70<<0) );
}
EXPORT_SYMBOL(thermal_TADC_mode);

u32 get_thermal_ADC(void)
{
	return get_chip_scu_data(DOUT_TADC);
}
EXPORT_SYMBOL(get_thermal_ADC);

u32 get_xtal_sel(void)
{
	return ((get_chip_scu_data(SCU_XTAL_SEL)>>19)&0x1);
}
EXPORT_SYMBOL(get_xtal_sel);
#endif

/************************************************************************
*                 AVS API
*************************************************************************
*/
static void AVS_DAC_CHG(void)
{
	u32 tmp_val;
	
	//toggle CHG bit16
	tmp_val = get_chip_scu_data(RG_DATA_AVS_DAC) | 0x10000;
	set_chip_scu_data(RG_DATA_AVS_DAC, tmp_val);
	tmp_val = get_chip_scu_data(RG_DATA_AVS_DAC) & 0xfffeffff;
	set_chip_scu_data(RG_DATA_AVS_DAC, tmp_val);
}

/*get AVS_DAC value & return AVS_OUT_EN value
*/ 
u32 get_avs_DAC(u32 *AVS_DAC)
{
	*AVS_DAC = get_chip_scu_data(RG_DATA_AVS_DAC) & 0x3ff;

	return (get_chip_scu_data(RG_AVS_OUT_EN)&0x1);
}
//EXPORT_SYMBOL(get_avs_DAC);

/*set AVS_DAC value & enable AVS_DAC output
*/ 
void set_avs_DAC(u32 *AVS_DAC)
{
	u32 temp_val=0;
	set_chip_scu_data(PLLRG_PROTECT, 0x80);
	set_chip_scu_data(RG_DATA_AVS_DAC, *AVS_DAC);
	AVS_DAC_CHG();
	//enable output
	temp_val = get_chip_scu_data(RG_AVS_OUT_EN)|0x1;
	set_chip_scu_data(RG_AVS_OUT_EN, temp_val);
	set_chip_scu_data(PLLRG_PROTECT, 0x0);
}
//EXPORT_SYMBOL(set_avs_DAC);


