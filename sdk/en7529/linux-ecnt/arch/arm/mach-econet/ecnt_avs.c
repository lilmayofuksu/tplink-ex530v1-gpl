#include <linux/version.h>
#include <asm/io.h>
#include <modules/avs/avs.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
#else
#include <asm/tc3162.h>
#endif
//#include "avs/avs.h"

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#define mux_T	0
#define mux_AVSMON	1
#define mux_VCORE	2
#define mux_AVSDAC	3
	
	//#define AVS_DAC_step		12		//unit: 0.1mV
#define AVS_DAC_step_HV		12		//unit: 0.1mV, experiment value for Vcore accuracy
#define AVS_DAC_step_LV		9		//unit: 0.1mV, experiment value for Vcore accuracy
	//#define TADC_step 		31	//unit: uV
#define TADC_4_step			12		//unit: 0.1mV
#define AVS_high_thd		13000	//unit: 0.1mV
#define AVS_low_thd			10300	//unit: 0.1mV
	//#define AVS_DAC_NV			0x290	//experiment value for Vcore = 1.15V
#define AVS_DAC_offset		0xc	//experiment value for Vcore accuracy
	
#define Temp_high_thd_ADC	0x9000
#define BUCK_high_limit		19200	//unit: 0.1mV
#define	BUCK_default_V		11500	//unit: 0.1mV
#define	AVS_turn_point		12500	//unit: 0.1mV


/************************************************************************
*                  M A C R O S
*************************************************************************
*/
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
#else
#define printk printf
#endif

/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/
/* APIs */
//kernel version
extern u32 get_avs_DAC(u32 *AVS_DAC);
extern void set_avs_DAC(u32 *AVS_DAC);
#if 0	//ADC read not ready
static u32 AVS_Read_ADC(u32 mux_num)
{
	u32 tmp_val;
	set_avs_reg(0x80, RG_PLLRG_PROTECT);
	set_avs_reg(0x70, RG_MUX_TADC);	//config mode
	set_avs_reg(0xf0, RG_MUX_TADC);	//reset TADC
	tmp_val = (get_avs_reg(RG_MUX_TADC) & 0xfffffff8) | (mux_num & 0x3);
	set_avs_reg(tmp_val, RG_MUX_TADC);
	mdelay(1);
	tmp_val = get_avs_reg(RG_DOUT_TADC);
	set_avs_reg(0x0, RG_PLLRG_PROTECT);
	return tmp_val;
}
#endif
 
/*target V unit = 0.01V 
  V_ADC variable = (real_VADC)/4 to have same voltage scale with AVS_DAC
  E1 IC change algorithm due to ADC read invalid*/     
AVS_STATUS_T AVS_Set(u32 target_V)
{
	//u32 target_V_ADC = target_V*100/TADC_4_step;
	//u32 current_V_ADC, temp_ADC, FB_V_ADC;
	//u32 target_DAC;
	//u32 RG_AVS_OUT_EN_val, AVS_DAC;
	//u32 ii;
	
	if((target_V*100) > AVS_high_thd || (target_V*100) < AVS_low_thd)
	{
		printk("AVS OUT_RANGE_FAIL\r\n");
		return OUT_RANGE_FAIL;
	}
#if 0	//ADC read not ready
	temp_ADC = AVS_Read_ADC(mux_T);
	current_V_ADC = AVS_Read_ADC(mux_VCORE)>>2;

	printk("temp_ADC = 0x%x\n", temp_ADC);
	printk("current_Vcore_ADC = 0x%x\n", current_V_ADC);

	if(temp_ADC > Temp_high_thd_ADC && target_V_ADC > current_V_ADC)
	{
		printk("AVS TEMP_HIGH_FAIL\n");
		return TEMP_HIGH_FAIL;
	}
	
	FB_V_ADC = AVS_Read_ADC(mux_AVSMON)>>2;
	printk("AVSMON_ADC = 0x%x\n", current_V_ADC);


	printk("AVS_Set\r\n");
	
	RG_AVS_OUT_EN_val = get_avs_DAC(&AVS_DAC);
	
	if((RG_AVS_OUT_EN_val&0x1) == 0)
	{
		//AVS_DAC = (FB_V_ADC*TADC_4_step)/AVS_DAC_step;	//ADC read not ready
		AVS_DAC = ((BUCK_high_limit - AVS_turn_point)/AVS_DAC_step_HV) + ((AVS_turn_point - BUCK_default_V)/AVS_DAC_step_LV);
		set_avs_DAC(&AVS_DAC);
		printk("AVS enable\r\n");
	}

#if 0	//ADC read not ready
	while(current_V_ADC != target_V_ADC)
	{
		if(current_V_ADC > target_V_ADC)
		{
			current_V_ADC--;
			AVS_DAC++;
			set_avs_reg(AVS_DAC, RG_DATA_AVS_DAC);
			AVS_DAC_CHG();
		}
		else
		{
			current_V_ADC++;
			AVS_DAC--;
			set_avs_reg(AVS_DAC, RG_DATA_AVS_DAC);
			AVS_DAC_CHG();
		}
	}
#else	//use ECNT Buck transfer formula
	
	target_DAC = ((target_V*100 < AVS_turn_point)
					? (((BUCK_high_limit - AVS_turn_point)/AVS_DAC_step_HV) + ((AVS_turn_point - target_V*100)/AVS_DAC_step_LV))
					: ((BUCK_high_limit - target_V*100)/AVS_DAC_step_HV)) - AVS_DAC_offset;
	
	while(AVS_DAC != target_DAC)
	{
		if(AVS_DAC > target_DAC)
		{
			AVS_DAC--;
			set_avs_DAC(&AVS_DAC);
		}
		else
		{
			AVS_DAC++;
			set_avs_DAC(&AVS_DAC);
		}
	}
#endif
	
	printk("AVS set done.\r\n");
#endif	
	return AVS_OK;
}
//EXPORT_SYMBOL(AVS_Set);

//return value unit = 0.01V 
u32 AVS_Get_Vcore(void)
{
	u32 avs_oen, AVS_DAC, DAC_turn_point;

	avs_oen = get_avs_DAC(&AVS_DAC);

	if(avs_oen == 0)
	{
		//AVS not enable, return BUCK default voltage
		return	(BUCK_default_V/100);
	}
	
	AVS_DAC += AVS_DAC_offset;
	DAC_turn_point = ((BUCK_high_limit - AVS_turn_point)/AVS_DAC_step_HV);

	return ((AVS_DAC < DAC_turn_point)
				?((BUCK_high_limit - AVS_DAC*AVS_DAC_step_HV)/100)
				:((AVS_turn_point - (AVS_DAC - DAC_turn_point)*AVS_DAC_step_LV)/100));
}
//EXPORT_SYMBOL(AVS_Get_Vcore);


