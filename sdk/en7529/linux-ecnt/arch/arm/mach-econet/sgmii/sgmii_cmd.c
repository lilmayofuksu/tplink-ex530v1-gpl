
/************************************************************************
*               I N C L U D E S
*************************************************************************
*/
#include <linux/types.h>
#include <asm/io.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include "ecnt_sgmii.h"
#include <asm/string.h>
#include "sgmii_reg_pon.h"

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
extern u32 sgmii_cmd_ro(sgmii_port_type port_id, sgmii_reg_type type, u32 reg);
extern void sgmii_cmd_wo(sgmii_port_type port_id, sgmii_reg_type type, u32 reg, u32 val);

extern void SET_SCU_RST_RG(u32 val);
extern void SET_SSR3(u32 val);
extern void SET_WAN_CONF(u32 val);

extern void set_pon_phy_data(u32 reg, u32 val);
extern void get_pon_phy_data(u32 reg);

extern u32 get_pon_pcs2_data(u32 reg);
extern void set_pon_an_data(u32 reg, u32 val);
extern u32 get_pon_an_data(u32 reg);


const char* sgmii_port_name[] = {"PORT_PCIE0", "PORT_PCIE1", "PORT_USB0","PORT_UNKNOW"};
const char* sgmii_speed_name[] = {"SPEED_2500M", "SPEED_1000M", "SPEED_100M","SPEED_10M","SPEED_UNKNOW"};
/************************************************************************
*                  M A C R O S
*************************************************************************
*/

/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/


/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/


/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
int sgmii_api1(int argc, char *argv[], void *p);
int sgmii_api_mode(sgmii_api_method_type method, sgmii_port_type port, sgmii_speed_type speed);
int sgmii_api_info(sgmii_api_method_type method, sgmii_port_type port);
static int sgmii_test(unsigned long *reg, unsigned long *value);

static sgmii_port_type _covert_port(char* port_str);
static sgmii_speed_type _covert_speed(char* speed_str);
static sgmii_api_method_type _covert_method(char* method_str);
static sgmii_api_type _covert_api(char* api_str);

static int _sgmii_get_link(sgmii_port_type port);
static int _sgmii_set_speed(sgmii_port_type port, sgmii_speed_type speed);
static sgmii_speed_type _sgmii_get_speed(sgmii_port_type port);
/************************************************************************
*                  P R I V A T E   D A T A
*************************************************************************
*/
/*API -------------------------- */ 
static int sgmii_test(unsigned long *reg, unsigned long *value)
{
	printk("sgmii_test: \n");
	printk("%lx:%lx\n",*reg, *value);
	u32 ro = sgmii_cmd_ro(SGMII_PORT_PCIE0, SGMII_REG_PCS2, (u32)*reg);
	printk("ro:%lx\n",ro);
	return 0;
}


int sgmii_api_pcie1_force_hsgmii(void)
{
	//printk("sgmii_api_pcie1_force_hsgmii: inint \n");
	u32 default_val = 0;
	u32 mask = 0;
	default_val = GET_SSR3();
	mask = 0xBFFFFFFF;
	SET_SSR3(default_val & mask); //0xa0002820 [30]=0
	//printk("sgmii aaa\n");
	sgmii_cmd_wo(SGMII_PORT_PCIE1, SGMII_REG_PHYA, SGMII_REG_PHYA_11, 0x14817);
	//printk("sgmii aaa2\n");
	sgmii_cmd_wo(SGMII_PORT_PCIE1, SGMII_REG_RATEADAPT, SGMII_RG_RATE_ADAPT_CTRL_0, 0x0c000c11);
	//printk("sgmii aaa3\n");
	sgmii_cmd_wo(SGMII_PORT_PCIE1, SGMII_REG_AN, SGMII_REG_AN0, 0x00000140);
	//printk("sgmii aaa4\n");
	sgmii_cmd_wo(SGMII_PORT_PCIE1, SGMII_REG_PCS2, SGMII_REG_AN_5, 0x3);
	//printk("sgmii aaa5\n");
	
	default_val = GET_SCU_RST_RG();
	//printk("sgmii aaa6\n");
	SET_SCU_RST_RG(default_val | 0x24000);
	//printk("sgmii aaa7\n");
	SET_SCU_RST_RG(default_val & 0xFFFDBFFF);
	//printk("sgmii_api_pcie1_force_hsgmii: exit \n");
	return 0;
}


int sgmii_api_pcie0_force_hsgmii(void)
{
	//printk("sgmii_api_pcie1_force_hsgmii: inint \n");
	u32 default_val = 0;
	u32 mask = 0;
	default_val = GET_SSR3();
	mask = 0x7FFFFFFF;
	SET_SSR3(default_val & mask); //0x60002820 [31]=0
	
	//printk("sgmii aaa\n");
	sgmii_cmd_wo(SGMII_PORT_PCIE0, SGMII_REG_PHYA, SGMII_REG_PHYA_11, 0x14817);
	//printk("sgmii aaa2\n");
	sgmii_cmd_wo(SGMII_PORT_PCIE0, SGMII_REG_RATEADAPT, SGMII_RG_RATE_ADAPT_CTRL_0, 0x0c000c11);
	//printk("sgmii aaa3\n");
	sgmii_cmd_wo(SGMII_PORT_PCIE0, SGMII_REG_AN, SGMII_REG_AN0, 0x00000140);
	//printk("sgmii aaa4\n");
	sgmii_cmd_wo(SGMII_PORT_PCIE0, SGMII_REG_PCS2, SGMII_REG_AN_5, 0x3);
	//printk("sgmii aaa5\n");
	
	default_val = GET_SCU_RST_RG();
	//printk("sgmii aaa6\n");
	SET_SCU_RST_RG(default_val | 0x12000);
	//printk("sgmii aaa7\n");
	SET_SCU_RST_RG(default_val & 0xFFFEDFFF);
	//printk("sgmii_api_pcie1_force_hsgmii: exit \n");
	return 0;
}



int sgmii_api_usb0_force_hsgmii(void)
{
	//printk("sgmii_api_pcie1_force_hsgmii: inint \n");
	u32 default_val = 0;
	u32 mask = 0;
	default_val = GET_SSR3();
	mask = 0xDFFFFFFF;
	SET_SSR3(default_val & mask); //0xc0002820 [29]=0
	
	//printk("sgmii aaa\n");
	sgmii_cmd_wo(SGMII_PORT_USB0, SGMII_REG_PHYA, SGMII_REG_PHYA_11, 0x14817);
	//printk("sgmii aaa2\n");
	sgmii_cmd_wo(SGMII_PORT_USB0, SGMII_REG_RATEADAPT, SGMII_RG_RATE_ADAPT_CTRL_0, 0x0c000c11);
	//printk("sgmii aaa3\n");
	sgmii_cmd_wo(SGMII_PORT_USB0, SGMII_REG_AN, SGMII_REG_AN0, 0x00000140);
	//printk("sgmii aaa4\n");
	sgmii_cmd_wo(SGMII_PORT_USB0, SGMII_REG_PCS2, SGMII_REG_AN_5, 0x3);
	//printk("sgmii aaa5\n");
	
	default_val = GET_SCU_RST_RG();
	//printk("sgmii aaa6\n");
	SET_SCU_RST_RG(default_val | 0x48000);
	//printk("sgmii aaa7\n");
	SET_SCU_RST_RG(default_val & 0xFFFB7FFF);
	//printk("sgmii_api_pcie1_force_hsgmii: exit \n");
	return 0;
}

/**/
int sgmii_api_pon0_force_hsgmii(void)
{
	/* XTAL SET ------------------------------------------------------------------ */
	//RGS_ECC_SEL[19]
	//RGS_XTAL_FREQ =1'b1 internal XTAL 25Mhz; =1'b0 internal XTAL 20Mhz
	//uint32 reg;
	//reg = mmio_read_32(0x1fa20254); 
	//reg = (reg >> 19) & 0x1;
	set_pon_phy_data(SS_LCPLL_TDC_FLT_2_ADDR, 0x64000000 ); //25Mhz
	//============================================================
	u32 default_val = 0;
	default_val = GET_SSR3();
	SET_SSR3((default_val & 0xFFFF9FFF) | 0x00004000); //0xe0004820 [14:13]=0
    SET_WAN_CONF(0xe0000011); //0x1fb00070 6 --> 11 , wan_sel as hsgmii
	
	
	
    set_pon_an_data(0x00, 0x140); //1140 -> 140 AN DIS
    //IO_SPHYA_REG_BITS( PON_SERDES_CTRL_10_ADDR, 8 , 8,  0x0);  //4228 rg_tx_bit_polarity_tmp
    set_pon_phy_data(PON_SERDES_CTRL_10_ADDR, 0x103 ); //0x3 => E2
    //IO_SPHYA_REG_BITS( FRQ_CTRL_2_ADDR , 11, 10, 0x0);  //4364 rg_hg_rx_freq_det_mux_tmp rg_hg_tx_freq_det_mux_tmp 
    set_pon_phy_data(FRQ_CTRL_2_ADDR, 0x3003 ); //3c03 -> 3003 
    //IO_SPHYA_REG_BITS( FRQ_CTRL_4_ADDR , 11,  0, 0x618);  //4364   rg_hg_tx_freq_cnt_tmp
    //IO_SPHYA_REG_BITS( FRQ_CTRL_4_ADDR , 27, 16, 0x618);  //4364   rg_hg_rx_freq_cnt_tmp
    set_pon_phy_data(FRQ_CTRL_4_ADDR, 0x15601560 ); //6180618 => E2
    //IO_SPHYA_REG_BITS( RG_SSUSB_LN0_CDR_PD_DIV_BYPASS_ADDR , 12, 12, 0x1);  //3028 RG_SSUSB_LN0_CDR_EPEN_tmp
    set_pon_phy_data(RG_SSUSB_LN0_CDR_PD_DIV_BYPASS_ADDR, 0x18001722 ); //18000722 -> 18001722
    //IO_SPHYA_REG_BITS( PON_SYS_CTRL_0_ADDR , 9,  4, 0x3c);  //4608 ck_en
    //IO_SPHYA_REG_BITS( PON_SYS_CTRL_0_ADDR , 29, 25, 0xa);  //4608 ck_sel
    set_pon_phy_data(PON_SYS_CTRL_0_ADDR, 0x250003c0 ); //150003f0 => E2
    //IO_SPHYA_REG_BITS( HG_RST_CTRL_0_ADDR , 5, 0, 0x3f);  // rst
    set_pon_phy_data(HG_RST_CTRL_0_ADDR, 0x3f ); //0 -> 3f
    //IO_SPHYA_REG_BITS( PON_DA_CTRL_2_ADDR , 25, 25, 0x0);  //RG_SSUSB_CDR_PI_PWD_tmp
    set_pon_phy_data(PON_DA_CTRL_2_ADDR, 0x54101801 ); //56101801 -> 54101801
    //IO_SPHYA_REG_BITS( PON_RXFEDIG_CTRL_12_ADDR , 22, 22, 0x0);  //RG_SSUSB_EQ_REV_tmp
    set_pon_phy_data(PON_RXFEDIG_CTRL_12_ADDR, 0x380013 ); //780013 -> 380013
    //IO_SPHYA_REG_BITS( PON_OSR_SEL_CTRL_ADDR , 1, 0, 0x1);  //
    //IO_SPHYA_REG_BITS( PON_OSR_SEL_CTRL_ADDR , 4, 4, 0x1);  //
    set_pon_phy_data(PON_OSR_SEL_CTRL_ADDR, 0x11 ); //0 -> 11
    //IO_SPHYA_REG_BITS( HG_MODE_CTRL_0_ADDR   , 0, 0, 0x1);  // rg_serdes_mode_tmp
    set_pon_phy_data(HG_MODE_CTRL_0_ADDR, 0x1 ); //0 -> 1
    //IO_SPHYA_REG_BITS( PON_SYS_CTRL_0_ADDR   ,24,24, 0x0);  // rg_rx_pma_clk_div_sel_tmp
    set_pon_phy_data(PON_SYS_CTRL_0_ADDR, 0x240003c0 ); //140003f0 => E2
	/* ---------------------------------------------------------------------------*/
    //IO_SPHYA_REG_BITS( SS_LCPLL_TDC_FLT_5_ADDR   ,24,24, 0);  // 413c
    set_pon_phy_data(SS_LCPLL_TDC_FLT_5_ADDR, 0x10100 ); //101_0100 --> 10100
    //IO_SPHYA_REG_BITS( SS_LCPLL_TDC_FLT_3_ADDR   ,8,8, 1);  // 4134
    set_pon_phy_data(SS_LCPLL_TDC_FLT_3_ADDR, 0x20000100 ); //20000000 --> 2000_0100
    //IO_SPHYA_REG_BITS( SS_LCPLL_TDC_FLT_5_ADDR   ,24,24, 1);  // 413c
    set_pon_phy_data(SS_LCPLL_TDC_FLT_5_ADDR, 0x1010100 ); //101_0100 --> 10100
    //IO_SPHYA_REG_BITS( PON_SERDES_CTRL_0_ADDR  ,0,0, 1);  // 4200
    set_pon_phy_data(PON_SERDES_CTRL_0_ADDR, 0x100381 ); //100380 -> 100381
    //udelay(500);
	return 0;
}



/**/
int sgmii_api_force_mode(sgmii_port_type port, sgmii_speed_type speed)
{
	/*ex. ---------------------------------------------------
	int port = 0; //{pcie0=0, pcie1, usb0, pon0};
	int speed = 0; //{HSGMII=0, SGMII, RA100M, RA10M};
	int ret = sgmii_api_force_mode(0, argc, argv);
	if(ret == 1 && sync ==1)
		printk("set_done!\n");
	else
		printk("fail!\n");
	// ---------------------------------------------------*/ 
	
	
	//unknow format, now only hsgmii = SGMII_SPEED_2500M = 0
	if(speed >= SGMII_SPEED_UNKNOW || speed <0)
	{	
		return 0;
	}
	switch(port)
	{
		case SGMII_PORT_PCIE0:
			sgmii_api_pcie0_force_hsgmii();
			break;
		case SGMII_PORT_PCIE1:
			sgmii_api_pcie1_force_hsgmii();
			break;
		case SGMII_PORT_USB0:
			sgmii_api_usb0_force_hsgmii();
			break;
		case SGMII_PORT_PON0:
			sgmii_api_pon0_force_hsgmii();
			break;
		default: //unknow format
			return 0;
			break;
		
	}
	
	return 1;
}
EXPORT_SYMBOL(sgmii_api_force_mode);

/**/
int sgmii_api_get_info(sgmii_port_type port, int *argc, int *argv[])
{
	 
	/*ex. ---------------------------------------------------
	int argc=3, sync, an, dump;
	int *argv[] = {&sync, &an, &dump};
	int port = 0; //{pcie0=0, pcie1, usb0, pon0};
	int ret = sgmii_api_get_info(0, &argc, argv);
	if(ret == 1 && sync ==1)
		printk("linkup\n");
	else
		printf("linkdown\n");
	// ---------------------------------------------------*/ 
	
	//unknow format, now only hsgmii = SGMII_SPEED_2500M = 0
	if(port >= SGMII_PORT_UNKNOW || port <0 || *argc == NULL || *argc <2)
	{	
		return 0;
	}
	
	uint32 link = 0;
	*argc = 3;
	if(port == SGMII_PORT_PON0)
	{
		link = get_pon_pcs2_data(0x104); //0xb04 pcs2_base
		*argv[0] = link>>5 & 0x1;		//[5]sync
		*argv[1] = link>>0 & 0x1; 		//[0]an
		*argv[2] = 0x0000;				//dump
	}
	else
	{
		link = _sgmii_get_link(port);
		//ro_rxdump <<4 + ro_an_done<<1 + ro_sync<<0;
		*argv[0] = link>>0 & 0x1;		//[5]sync
		*argv[1] = link>>1 & 0x1; 		//[0]an
		*argv[2] = link>>4 & 0xf;		//dump
	}
	
	
	return 1;
}
EXPORT_SYMBOL(sgmii_api_get_info);


int sgmii_api_mode(sgmii_api_method_type method, sgmii_port_type port, sgmii_speed_type speed)
{
	//sgmii mode set pcie0 0/1/2/3
	
	
	if(port == SGMII_PORT_UNKNOW || method == SGMII_API_METHOD_UNKNOW || (method == SGMII_API_METHOD_SET  && speed == SGMII_SPEED_UNKNOW)){
		printk("SGMII_API_MODE:UNKNOW\n");
		return 0;
	}
	
	//printk("sgmii_api_mode: \n");
	//printk("method:%d, port:%d, speed:%d\n",method, port, speed);
	
	
	switch(method){
		case SGMII_API_METHOD_GET:
			printk("sgmii_mode_get:\n");
			break;
		case SGMII_API_METHOD_SET:
			_sgmii_set_speed(port, speed);
			break;
	}
	
	_sgmii_get_speed(port);
	
	return 0;
}
EXPORT_SYMBOL(sgmii_api_mode);

int sgmii_api_info(sgmii_api_method_type method, sgmii_port_type port)
{
	//sgmii info get pcie0
	if(port == SGMII_PORT_UNKNOW || method == SGMII_API_METHOD_UNKNOW){
		printk("SGMII_API_INFO:UNKNOW\n");
		return 0;
	}
	
	//printk("sgmii_api_mode: \n");
	//printk("method:%d, port:%d\n",method, port);
	
	
	switch(method){
		case SGMII_API_METHOD_GET:
			printk("sgmii_mode_get:\n");
			_sgmii_get_speed(port);
			_sgmii_get_link(port);
			break;
		case SGMII_API_METHOD_SET:
			printk("sgmii_mode_set: only get\n");
			break;
	}
	
	return 0;
}
EXPORT_SYMBOL(sgmii_api_info);

/*INTERNAL -------------------------- */ 
static int _sgmii_get_link(sgmii_port_type port)
{
	HAL_RG_TOP rg;
	uint32 data_t;
	uint32 ro_an_done;
	uint32 ro_sync;
	uint32 ro_rxdump;
	uint32 ro_txdump;
	
	//RO
	data_t = sgmii_cmd_ro(port, SGMII_REG_PCS2, SGMII_RG_HSGMII_PCS_STATE_2);
	rg.pcs2.rg_hsgmii_pcs_state_2 = &data_t;
	ro_sync    = rg.pcs2.rg_hsgmii_pcs_state_2->ro_rx_sync;
	ro_an_done = rg.pcs2.rg_hsgmii_pcs_state_2->ro_an_done;
	
	data_t = sgmii_cmd_ro(port, SGMII_REG_AN, SGMII_REG_AN_5);
	ro_rxdump    = data_t;
	
	data_t = sgmii_cmd_ro(port, SGMII_REG_AN, SGMII_REG_AN_4);
	ro_txdump    = data_t;
	printk("PORT:%s, SYNC:%d, AN:%d, TXAN:%x, RXAN:%x\n",sgmii_port_name[port], ro_sync, ro_an_done, ro_txdump, ro_rxdump);
	
	int v_reg = (ro_rxdump <<4) + (ro_an_done<<1) + (ro_sync<<0);
	
	return v_reg;
}


static int _sgmii_set_speed(sgmii_port_type port, sgmii_speed_type speed)
{
	HAL_RG_TOP rg;
	uint32 data_t;
	uint32 wo_speed;
	uint32 wo_rateadapt;
	uint32 wo_ra_bypass;
	uint32 wo_an_en;
	uint32 wo_ra_en;
	switch(speed){
		case SGMII_SPEED_2500M:
			wo_speed     = 1;
			wo_rateadapt = 0;
			wo_ra_bypass = 1;
			wo_an_en     = 0;
			wo_ra_en     = 0;
			break;
		case SGMII_SPEED_1000M:
			wo_speed 	 = 0;
			wo_rateadapt = 0;
			wo_ra_bypass = 1;
			wo_an_en     = 1;
			wo_ra_en     = 0;
			break;
		case SGMII_SPEED_100M:
			wo_speed     = 0;
			wo_rateadapt = 1;
			wo_ra_bypass = 0;
			wo_an_en     = 1;
			wo_ra_en     = 1;
			break;
		case SGMII_SPEED_10M:
			wo_speed     = 0;
			wo_rateadapt = 2;
			wo_ra_bypass = 0;
			wo_an_en     = 1;
			wo_ra_en     = 1;
			break;			
		default:
			printk("sgmii_set_speed:fail\n");
			return 0;	
		
	}
	
	//RW
	//data_t = sgmii_cmd_ro(port, SGMII_REG_PHYA, SGMII_REG_PHYA_11);
	//rg.phya.sgmii_reg_phya_11 = &data_t;
	//rg.phya.sgmii_reg_phya_11->rg_tphy_speed = wo_speed;
	//sgmii_cmd_wo(port, SGMII_REG_PHYA, SGMII_REG_PHYA_11, data_t);
	
	data_t = sgmii_cmd_ro(port, SGMII_REG_PCS2, SGMII_RG_HSGMII_PCS_CTROL_6);
	rg.pcs2.rg_hsgmii_pcs_ctrol_6 = &data_t;
	rg.pcs2.rg_hsgmii_pcs_ctrol_6->rg_sgmii_force_rateadapt_value= wo_rateadapt;
	sgmii_cmd_wo(port, SGMII_REG_PCS2, SGMII_RG_HSGMII_PCS_CTROL_6, data_t);
	
	data_t = sgmii_cmd_ro(port, SGMII_REG_RATEADAPT, SGMII_RG_RATE_ADAPT_CTRL_0);
	rg.ra.rg_rate_adapt_ctrl_0 = &data_t;
	rg.ra.rg_rate_adapt_ctrl_0->rg_rate_adapt_tx_bypass= wo_ra_bypass;
	rg.ra.rg_rate_adapt_ctrl_0->rg_rate_adapt_rx_bypass= wo_ra_bypass;
	rg.ra.rg_rate_adapt_ctrl_0-> rg_rate_adapt_tx_en= wo_ra_en;
	rg.ra.rg_rate_adapt_ctrl_0->rg_rate_adapt_rx_en= wo_ra_en;
	sgmii_cmd_wo(port, SGMII_REG_RATEADAPT, SGMII_RG_RATE_ADAPT_CTRL_0, data_t);
	
	data_t = sgmii_cmd_ro(port, SGMII_REG_PCS2, SGMII_RG_HSGMII_PCS_CTROL_1);
	rg.pcs2.rg_hsgmii_pcs_ctrol_1 = &data_t;
	rg.pcs2.rg_hsgmii_pcs_ctrol_1->rg_an_enable= wo_an_en;
	sgmii_cmd_wo(port, SGMII_REG_PCS2, SGMII_RG_HSGMII_PCS_CTROL_1, data_t);
	
	
	return 0;
}


static sgmii_speed_type _sgmii_get_speed(sgmii_port_type port)
{
	HAL_RG_TOP rg;
	uint32 data_t;
	uint32 ro_speed_lsb=0;
	uint32 ro_rateadapt;
	
	//RO
	//data_t = sgmii_cmd_ro(port, SGMII_REG_PHYA, SGMII_REG_PHYA_11);
	//rg.phya.sgmii_reg_phya_11 = &data_t;
	//ro_speed_lsb = rg.phya.sgmii_reg_phya_11->rg_tphy_speed;
	
	data_t = sgmii_cmd_ro(port, SGMII_REG_PCS2, SGMII_RG_HSGMII_PCS_CTROL_6);
	rg.pcs2.rg_hsgmii_pcs_ctrol_6 = &data_t;
	ro_rateadapt = rg.pcs2.rg_hsgmii_pcs_ctrol_6->rg_sgmii_force_rateadapt_value;
	
	if(ro_speed_lsb == 1){
		printk("PORT:%s, SPEED:%s\n",sgmii_port_name[port], sgmii_speed_name[SGMII_SPEED_2500M]);
		return SGMII_SPEED_2500M;
	}
	else if(ro_speed_lsb == 0 && ro_rateadapt == 0){
		printk("PORT:%s, SPEED:%s\n",sgmii_port_name[port], sgmii_speed_name[SGMII_SPEED_1000M]);
		return SGMII_SPEED_1000M;
	}
	else if(ro_speed_lsb == 0 && ro_rateadapt == 1){
		printk("PORT:%s, SPEED:%s\n",sgmii_port_name[port], sgmii_speed_name[SGMII_SPEED_100M]);
		return SGMII_SPEED_100M;
	}
	else if(ro_speed_lsb == 0 && ro_rateadapt == 2){
		printk("PORT:%s, SPEED:%s\n",sgmii_port_name[port], sgmii_speed_name[SGMII_SPEED_10M]);
		return SGMII_SPEED_10M;
	}
	else{
		printk("sgmii_api_mode:ERROR!\n");
		printk("PORT:%s, SPEED:%s\n",ro_speed_lsb, ro_rateadapt);
	}
}


static sgmii_api_method_type _covert_method(char* method_str)
{
	sgmii_api_method_type method;
	
	if (strncmp(method_str, "get",3)==0)
		method = SGMII_API_METHOD_GET;
	else if (strncmp(method_str, "set",3)==0)
		method = SGMII_API_METHOD_SET;
	else
		method = SGMII_API_METHOD_UNKNOW;
	
	return method;
}

static sgmii_port_type _covert_port(char* port_str)
{
	sgmii_port_type port;
	
	if (strncmp(port_str, "pcie0",5)==0)
		port = SGMII_PORT_PCIE0;
	else if (strncmp(port_str, "pcie1",5)==0)
		port = SGMII_PORT_PCIE1;
	else if (strncmp(port_str, "usb0",4)==0)
		port = SGMII_PORT_USB0;
	else
		port = SGMII_PORT_UNKNOW;
	
	return port;
}

static sgmii_speed_type _covert_speed(char* speed_str)
{
	sgmii_speed_type speed;
	
	if (strncmp(speed_str, "2500",4)==0)
		speed = SGMII_SPEED_2500M;
	else if (strncmp(speed_str, "1000",4)==0)
		speed = SGMII_SPEED_1000M;
	else if (strncmp(speed_str, "100",3)==0)
		speed = SGMII_SPEED_100M;
	else if (strncmp(speed_str, "10",2)==0)
		speed = SGMII_SPEED_10M;
	else
		speed = SGMII_SPEED_UNKNOW;
	return speed;
}


static sgmii_api_type _covert_api(char* api_str)
{
	sgmii_api_type api;
	//printk("api_str\n");
	if (strncmp(api_str, "mode",4)==0)
		api = SGMII_API_MODE;
	else if (strncmp(api_str, "info",4)==0)
		api = SGMII_API_INFO;
	else if (strncmp(api_str, "test",4)==0)
		api = SGMII_API_TEST;
	else
		api = SGMII_API_UNKNOW;
	//printk("api:%d\n",api);
	return api;
}

/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/
//int sgmii_api(unsigned long *reg, unsigned long *value)
int sgmii_api(int argc, char *argv[], void *p)
{
	//sgmii mode set pcie0 0/1/2/3
	//sgmii mode get pcie0
	//sgmii info get pcie0
	//
	//printk("sgmii_api:inin\n");
	
	
	unsigned long reg=0;
	unsigned long value=0;
	sgmii_port_type port;
	sgmii_speed_type speed;
	sgmii_api_method_type method;
	sgmii_api_type api;
	int flag;
	if(argc<1)
	{
		printk("sgmii_api:error: input:mode/info \n");
		return 0;
	}
	api = _covert_api(argv[1]);
	//printk("top_api:%d\n",api);
	switch(api)
	{
		case SGMII_API_TEST:
				printk("SGMII_API_TEST\n");
				
				sgmii_api_pon0_force_hsgmii();
				
				//
				//reg = (unsigned long)simple_strtoul(argv[1], NULL, 16);
				//value = (unsigned long)simple_strtoul(argv[2], NULL, 16);
				//printk("%lx:%lx\n",reg, value);
				//flag = sgmii_test(&reg, &value);
			break;
		case SGMII_API_MODE:
				//printk("SGMII_API_MODE\n");
				if(argc<4){
					printk("sgmii_api:error: ex. sgmii mode set pcie0 2500M \n");return 0;
				}
				else if(argc<5){
					speed = SGMII_SPEED_UNKNOW;
					printk("sgmii_api:speed_empty.\n");
				}
				else{
					speed = _covert_speed(argv[4]);
				}
				method = _covert_method(argv[2]);
				port = _covert_port(argv[3]);
				flag = sgmii_api_mode(method, port, speed);
			break;
		case SGMII_API_INFO:
		
				//printk("SGMII_API_INFO\n");
				if(argc<3){
					printk("sgmii_api:error: ex. sgmii info get pcie0\n");return 0;
				}
				method = _covert_method(argv[2]);
				port = _covert_port(argv[3]);
				flag = sgmii_api_info(method, port);
			break;
		default:
			printk("sgmii_api:UNKNOW?? \n");
			break;
		
	}
	
	
	//printk("sgmii_api:exit\n");
	return 0;
}


EXPORT_SYMBOL(sgmii_api);

