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

#include <linux/delay.h>
#include <linux/io.h>
#include <linux/of_address.h>
#include <linux/fs.h>
#include <asm/tc3162/tc3162.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <modules/npu/npuMboxAPI.h>
#include <linux/dma-mapping.h>
#include <linux/spinlock_types.h>

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#define NPU_384K_SRAM_BASE      (0x000000)  /*phys:0x1e800000*/
#define NPU_64K_SRAM_BASE       (0x000000)  /*phys:0x1e900000*/

#define NPU_CLUSTER_BASE        (0x308000)  /*phys:0x1eC08000*/
#define CR_CORE_BOOT_TRIGGER    (NPU_CLUSTER_BASE+0x000)
#define CR_CORE_BOOT_CONFIG     (NPU_CLUSTER_BASE+0x004)
#define CR_CORE_BOOT_BASE       (NPU_CLUSTER_BASE+0x020)
#define CR_PC_BASE_ADDR	        (NPU_CLUSTER_BASE+0x800)

#define NPU_MBOX_BASE           (0x30C000)  /*phys:0x1eC0C000*/
#define CR_MBOX_INTR_STATUS     (NPU_MBOX_BASE+0x0)
#define CR_MBOX_INT_MASK0       (NPU_MBOX_BASE+0x004)
#define CR_MBOX_INT_MASK8       (NPU_MBOX_BASE+0x024)
#define CR_MBQ0_CTRL0           (NPU_MBOX_BASE+0x030)
#define CR_MBQ0_CTRL1           (NPU_MBOX_BASE+0x034)
#define CR_MBQ0_CTRL2           (NPU_MBOX_BASE+0x038)
#define CR_MBQ0_CTRL3           (NPU_MBOX_BASE+0x03C)
#define CR_MBQ8_CTRL0           (NPU_MBOX_BASE+0x0b0)
#define CR_MBQ8_CTRL1           (NPU_MBOX_BASE+0x0b4)
#define CR_MBQ8_CTRL2           (NPU_MBOX_BASE+0x0b8)
#define CR_MBQ8_CTRL3           (NPU_MBOX_BASE+0x0bC)
#define CR_NPU_MIB0             (NPU_MBOX_BASE+0x140)
#define CR_NPU_MIB8             (NPU_MBOX_BASE+0x160)
#define CR_NPU_MIB9             (NPU_MBOX_BASE+0x164)
#define CR_NPU_MIB10            (NPU_MBOX_BASE+0x168)
#define CR_NPU_MIB11            (NPU_MBOX_BASE+0x16C)
#define CR_NPU_MIB12            (NPU_MBOX_BASE+0x170)
#define CR_NPU_MIB13            (NPU_MBOX_BASE+0x174)
#define CR_NPU_MIB14            (NPU_MBOX_BASE+0x178)
#define CR_NPU_MIB15            (NPU_MBOX_BASE+0x17C)
#define CR_NPU_MIB16            (NPU_MBOX_BASE+0x180)
#define CR_NPU_MIB19            (NPU_MBOX_BASE+0x18C)
#define CR_NPU_MIB20            (NPU_MBOX_BASE+0x190)

#define NPU_RBUS_BASE           (0x30E000)  /*phys:0x1eC0E000*/
#define CR_NPU_RBUS_DECODER     (NPU_RBUS_BASE+0x1200)

#define NPU_UART_BASE           (0x310000)  /*phys:0x1eC10000*/
#define NPU_TIME0_BASE          (0x310100)  /*phys:0x1eC10100*/
#define NPU_SCU_BASE            (0x311000)  /*phys:0x1eC11000*/

#ifdef TCSUPPORT_NPU_WIFI_OFFLOAD
#define NPU_HOSTADAPTER_BASE    (0x30D000)  /*phys:0x1eC0D000*/
u32 npu_pkt_buf_addr;
char npu_stat = -1;
char forceToCpu = -1;
EXPORT_SYMBOL(npu_stat);
#endif

#define NPU_INT_MAX_NUM         (6)

/************************************************************************
*                  M A C R O S
*************************************************************************
*/
#define S_16K   (0x4000)
#define S_64K   (0x10000)
#define S_128K  (0x20000)
#define S_2M    (0x200000)

#ifdef TCSUPPORT_CPU_EN7523
#define MAX_CORE_NUM    (4) 
#else
#define MAX_CORE_NUM    (8) 
#endif

/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/

struct ecnt_npu {
	struct device *dev;
	void __iomem *base;
	void __iomem *Regbase;
    phys_addr_t resv_paddr;
    void *resv_vaddr;
	int irq[NPU_INT_MAX_NUM];
    int mbox2host_irq;
};

/************************************************************************
*                  STATIC VARIABLE DECLARATIONS
*************************************************************************
*/
struct ecnt_npu *ecnt_npu = NULL;


static const struct of_device_id ecnt_npu_of_id[] = {
    { .compatible = "econet,ecnt-npu"},
    { /* sentinel */}
};
MODULE_DEVICE_TABLE(of, ecnt_npu_of_id);

typedef struct npuMboxObj
{
    unsigned long virtAddr[MAX_MBOX_FUNC];
    unsigned int physAddr[MAX_MBOX_FUNC];
    unsigned short size[MAX_MBOX_FUNC];
    h_mbox_cb_t mbox8_cb[MAX_MBOX_FUNC];
    spinlock_t npuMboxLock;
    spinlock_t mboxBuffLock;

} npuMboxObj_t;

unsigned long g_testWordPat[] = {0x5a5a5a5a, 0xa5a5a5a5, 0xff00ff00, 0x00ff00ff};
unsigned int g_testSize[] = {S_128K, S_16K, 0};

unsigned long npu_384k_sram_size = 0;

static npuMboxObj_t mboxObj[MAX_CORE_NUM];
static struct tasklet_struct mbox_tasklet;


/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/
extern int ecnt_rw_ram_test (unsigned long startAddr, unsigned int size, unsigned long patAdd);
extern unsigned int get_l2c_sram_size(void);

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/

u32 get_npu_384k_sram_data(u32 reg)
{
	if (reg > npu_384k_sram_size)
	{
		printk("ERROR: request addr exceed NPU 384k SRAM size\r\n");
		return 0;
	}
	
    return readl(ecnt_npu->base + reg);
}
EXPORT_SYMBOL(get_npu_384k_sram_data);

void set_npu_384k_sram_data(u32 reg, u32 val)
{
	if (reg > npu_384k_sram_size)
	{
		printk("ERROR: request addr exceed NPU 384k SRAM size\r\n");
		return;
	}
    writel(val, ecnt_npu->base + reg); 
}
EXPORT_SYMBOL(set_npu_384k_sram_data);

/* don't EXPORT this function. Create API for your purpose instead. */
static u32 get_npu_reg_data(u32 reg)
{
	return readl(ecnt_npu->Regbase + reg);
}
/* don't EXPORT this function. Create API for your purpose instead. */
static void set_npu_reg_data(u32 reg, u32 val)
{
	writel(val, ecnt_npu->Regbase + reg);
}
void set_npu_mib_index(u32 reg, u32 val)
{
	if(reg <= 20)
	{
		writel(val, ecnt_npu->Regbase + CR_NPU_MIB0 + (reg <<2)); 
	}
	else
	{
		printk("ERROR: reg should be less than 20.");
	}
}
EXPORT_SYMBOL(set_npu_mib_index);

u32 get_npu_mib_index(u32 reg)
{
	if(reg <= 20)
	{
		return readl(ecnt_npu->Regbase + CR_NPU_MIB0 + (reg <<2));
	}
	else
	{
		printk("ERROR: reg should be less than 20.");
		return 0;
	}
}
EXPORT_SYMBOL(get_npu_mib_index);

void boot_npu_core(int core)
{
    u32 val;


    #ifdef NPU_CODE_IN_SRAM
    set_npu_reg_data(CR_CORE_BOOT_BASE+(core<<2), (u32)0x1e800000);
    #else
    set_npu_reg_data(CR_CORE_BOOT_BASE+(core<<2), (u32)ecnt_npu->resv_paddr);
    #endif
    mdelay(1);

    if (core==0) { /* power on NPU and boot Core0 */
        set_npu_reg_data(CR_CORE_BOOT_CONFIG, 0x1);
        set_npu_reg_data(CR_CORE_BOOT_TRIGGER, 0x1);
    }
    else { /* reboot other Cores */
        val = (get_npu_reg_data(CR_CORE_BOOT_CONFIG)&0xff); /* keep old enabled bits */
        val |= (0x1<<core); /* add new enable bit for CoreX */
        val |= (0x0100<<core); /* add reboot bit for CoreX */
        set_npu_reg_data(CR_CORE_BOOT_CONFIG, val);
        set_npu_reg_data(CR_CORE_BOOT_TRIGGER, 0x2);
    }
    mdelay(100);
}
EXPORT_SYMBOL(boot_npu_core);

u32 get_npu_dbg_csr(int core, const unsigned int csr_ofs)
{
    return get_npu_reg_data((CR_PC_BASE_ADDR+(core<<8))+csr_ofs);
}
EXPORT_SYMBOL(get_npu_dbg_csr);

#ifdef TCSUPPORT_NPU_WIFI_OFFLOAD
int get_npu_irq(int index)
{
    if( index < NPU_INT_MAX_NUM )
        return ecnt_npu->irq[index];
    else
        printk("\n get_npu_irq error, index=%d\n", index);
    return 0;
}
EXPORT_SYMBOL(get_npu_irq);

u32 get_npu_hostadpt_reg(int hadap_ofs)
{
     return get_npu_reg_data(NPU_HOSTADAPTER_BASE+hadap_ofs);
}
EXPORT_SYMBOL(get_npu_hostadpt_reg);

u32 set_npu_hostadpt_reg(int hadap_ofs,int val)
{
     set_npu_reg_data(NPU_HOSTADAPTER_BASE+hadap_ofs,val);
     return;
}
EXPORT_SYMBOL(set_npu_hostadpt_reg);

// Reserve 16M block to save packet, and the address will be got after npu bring up.
void npu_wifi_offload_set_pkt_buf_addr(u32 pkt_buf_addr)
{
	npu_pkt_buf_addr = pkt_buf_addr;
	printk("[%x]npu_pkt_buf_addr=%x:%x\n", __func__, npu_pkt_buf_addr, pkt_buf_addr);
	return;
	
}
u32 npu_wifi_offload_get_pkt_buf_addr(void)
{
	printk("[%x]npu_pkt_buf_addr=%x\n", __func__, npu_pkt_buf_addr);
	return npu_pkt_buf_addr;
	
}
EXPORT_SYMBOL(npu_wifi_offload_get_pkt_buf_addr);

void npu_wifi_offload_set_force_to_cpu_flag(char isForceToCpu)
{
	forceToCpu = isForceToCpu;
	printk("[%x]isForceToCpu=%x:%x\n", __func__, forceToCpu, isForceToCpu);
	return;
	
}
EXPORT_SYMBOL(npu_wifi_offload_set_force_to_cpu_flag);

char npu_wifi_offload_get_force_to_cpu_flag(void)
{
#if defined(TCSUPPORT_RA_HWNAT) && defined(TCSUPPORT_HWNAT_V3)
	return forceToCpu;
#else
	return 1;
#endif
	
}
EXPORT_SYMBOL(npu_wifi_offload_get_force_to_cpu_flag);
#endif


/* 
 * fileType: 0 for npu_init_code (to DRAM), 
 *           1 for npu global data (to NPU_16K_SRAM)
 */
void copy_file_to_ram(struct file *srcf, int bufSize, int fileType)
{
    int i;
    unsigned char *buffer;

    if (fileType==0) {
        #ifdef NPU_CODE_IN_SRAM
        buffer = (unsigned char *)ecnt_npu->base;
        #else
        buffer = (unsigned char *)ecnt_npu->resv_vaddr;
        #endif
    }
    else /* fileType==1 */
        buffer = (unsigned char *)(ecnt_npu->Regbase);

    printk("(%d Bytes) to 0x%lx\n", bufSize, (unsigned long)buffer);

    /* "Unhandled fault: alignment fault" will happen if memset() is used on device node's address */
    for (i=0; i<bufSize; i++)
        buffer[i]=0;
    
    vfs_read(srcf, buffer, bufSize, &srcf->f_pos);

    #ifndef NPU_CODE_IN_SRAM
    if (fileType==0) {
        ecnt_dcache_wback_inv((unsigned long)buffer, bufSize);
    }
    #endif

    #if 0 //dump for debug
    for (i=0; i<bufSize; i++) {
        if ((i&0xf)==0)
            printk("%08x: ", i);
        printk("%02x ", buffer[i]);
        if ((i&0xf)==0xf)
            printk("\n");
        if ((i&0x3fff)==0x3fff)
            msleep(1);
    }
    printk("\n");
    #endif

    return;
}
EXPORT_SYMBOL(copy_file_to_ram);

int test_ram_b4_npu_load(void)
{
    unsigned long startAddr;
    int l;

    for(l=0; g_testSize[l]!=0; l++) {

        if (l==0) {
            #ifdef NPU_CODE_IN_SRAM
            /* for npu_init_code (to NPU 384K SRAM) */
            startAddr = (unsigned long)ecnt_npu->base;
            #else
            /* for npu_init_code (to DRAM) */
            startAddr = (unsigned long)ecnt_npu->resv_vaddr;
            #endif
        }
        else { /* l==1 for npu global data (to NPU_16K_SRAM) */
            startAddr = (unsigned long)(ecnt_npu->Regbase);
        }

        if (ecnt_rw_ram_test(startAddr, g_testSize[l], 0)==-1)
            return -1;
    }

    return 0;
}
EXPORT_SYMBOL(test_ram_b4_npu_load);

int test_npu_384k_sram (unsigned long offset, unsigned int testSize, unsigned long patAdd)
{
    unsigned long startAddr = (unsigned long)(ecnt_npu->base+(unsigned long)NPU_384K_SRAM_BASE+offset);

    #ifdef NPU_CODE_IN_SRAM
    return 0; /* skip 384k sram test because npu code is in it */
    #else
    return ecnt_rw_ram_test(startAddr, testSize, patAdd);
    #endif
}
EXPORT_SYMBOL(test_npu_384k_sram);

void set_npu_test_case(unsigned int tc)
{
    set_npu_reg_data(CR_NPU_MIB8, tc);
    return;
}
EXPORT_SYMBOL(set_npu_test_case);

int get_npu_test_case(void)
{
	return get_npu_reg_data(CR_NPU_MIB8);
}
EXPORT_SYMBOL(get_npu_test_case);

unsigned int npu_test_get_count(int core_id)
{
	u32 val = get_npu_reg_data(CR_NPU_MIB19 + (core_id*4));
	return val;
}
EXPORT_SYMBOL(npu_test_get_count);    /* Used in host_npu_core_on_off_test() */

void host_set_npu_core_on_off(int core, int on) {

	unsigned int tmpVal; 

	tmpVal = get_npu_reg_data(CR_CORE_BOOT_CONFIG);
	if (on) {
		tmpVal = (tmpVal |(1<<core));
	} else {
		tmpVal = (tmpVal & (~(1<<core)));
	}
	set_npu_reg_data(CR_CORE_BOOT_CONFIG, tmpVal);
	set_npu_reg_data(CR_CORE_BOOT_TRIGGER, 0x2);
	mdelay(100);
}
EXPORT_SYMBOL(host_set_npu_core_on_off);

unsigned int read_mbox2host_flag(void)
{
	return get_npu_reg_data(CR_NPU_MIB19);
}
EXPORT_SYMBOL(read_mbox2host_flag);    /* Used in mbox2host_isr() */

void clear_mbox2host_flag(void)
{
	set_npu_reg_data(CR_NPU_MIB19, 0);
}
EXPORT_SYMBOL(clear_mbox2host_flag);

int host_rd_chk_mbox_regs(unsigned int baseVal, unsigned int count)
{
	unsigned int tmpVal, cmpVal;
	int i;

	/* write data */	
	set_npu_reg_data(CR_MBOX_INT_MASK8, baseVal);
	for (i=0; i<8; i++) {
		set_npu_reg_data(CR_MBQ8_CTRL0+(i<<2), baseVal+1+i);
	}
	
	/* confirm data */	
	cmpVal = baseVal&0xffff;
	if ((tmpVal=get_npu_reg_data(CR_MBOX_INT_MASK8)) != cmpVal) {

			printk("[H]Error: CR_MBOX_INT_MASK8:0x%x != (baseVal&0xffff):0x%x at count%d\n", tmpVal, cmpVal, count);
			return 1;
	}
	for (i=0; i<8; i++) {
			cmpVal = baseVal+1+i;
			if (!(i==0 || i==4))
				cmpVal &= 0xffff;
			if ((tmpVal=get_npu_reg_data(CR_MBQ8_CTRL0+(i<<2))) != cmpVal) {
				printk("Error: MBQ8_REG:0x%x != (baseVal+1+%d):0x%x at count%d\n", tmpVal, i, cmpVal, count);
				return 1;
			}
	}

	return 0;
}
EXPORT_SYMBOL(host_rd_chk_mbox_regs);

void clear_mbox_intr_status(void)
{
	set_npu_reg_data(CR_MBOX_INTR_STATUS, 0xffff);	
}
EXPORT_SYMBOL(clear_mbox_intr_status);

int get_npu_irq_mailbox(void)
{
	return ecnt_npu->mbox2host_irq;
}
EXPORT_SYMBOL(get_npu_irq_mailbox);

int npu_wdog_testing(void)
{
	return get_npu_reg_data(CR_NPU_MIB8);
}
EXPORT_SYMBOL(npu_wdog_testing);

int npu_hw_kern_reset_testing(void)
{
	return get_npu_reg_data(CR_NPU_MIB8);
}
EXPORT_SYMBOL(npu_hw_kern_reset_testing);

unsigned int get_hanged_npu_cores(void)
{
	u32 val = get_npu_reg_data(CR_NPU_MIB19);
	return val;
}
EXPORT_SYMBOL(get_hanged_npu_cores);

void release_hanged_core(int core_id)
{
	u32 val = (get_npu_reg_data(CR_NPU_MIB19) & (~(1<<core_id)));
	set_npu_reg_data(CR_NPU_MIB19, val);
}
EXPORT_SYMBOL(release_hanged_core);

void set_npu_needed_info(void)
{
    set_npu_reg_data(CR_NPU_MIB10, ((unsigned int)ecnt_npu->resv_paddr)+0x200000); /*npu_test_area_base*/
    set_npu_reg_data(CR_NPU_MIB11, get_l2c_sram_size()); /*host_l2c_sram_size*/
    set_npu_reg_data(CR_NPU_MIB12, isFPGA); /* FPGA stage */
}
EXPORT_SYMBOL(set_npu_needed_info);

void start_npu_cores_reading_addr(unsigned int addr)
{
    set_npu_reg_data(CR_NPU_MIB10, addr);
    set_npu_reg_data(CR_NPU_MIB11, 1);
}
EXPORT_SYMBOL(start_npu_cores_reading_addr);

void set_npu_core_test_cnt(unsigned int testCnt)
{
    set_npu_reg_data(CR_NPU_MIB12, testCnt);
}
EXPORT_SYMBOL(set_npu_core_test_cnt);

unsigned int get_npu_core_reply(void)
{
    return get_npu_reg_data(CR_NPU_MIB13);
}
EXPORT_SYMBOL(get_npu_core_reply);

void finish_wt_dram_starving_test(void)
{
    set_npu_reg_data(CR_NPU_MIB14, 1);
}
EXPORT_SYMBOL(finish_wt_dram_starving_test);

int test_npu_mib_regs(void)
{
    int i, j;
    unsigned int reg;
    int cpuId = smp_processor_id();
    unsigned int val;

    reg = CR_NPU_MIB20+(cpuId<<3);

    for(j=0; g_testWordPat[j]!=0; j++) {

        val = g_testWordPat[j];

        /* write reg */
        for (i=0; i<2; i++) {
            set_npu_reg_data(reg+(i<<2), val+i);
        }
        
        /* read reg and compare*/
        for (i=0; i<2; i++) {
            if (get_npu_reg_data(reg+(i<<2)) != (val+i)) {
                printk("\nERROR get_npu_reg_data(reg+(i<<2)):0x%x != (val+i):0x%x for reg:0x%x, i:%d, j:%d, val:0x%x, cpu:%d\n", 
                        get_npu_reg_data(reg+(i<<2)), (val+i), reg, i, j, val, cpuId);
                return -1;
            }
        }
    }
    return 0;

}
EXPORT_SYMBOL(test_npu_mib_regs);

void set_npu_dma_config(unsigned int src_phy_addr, unsigned int dst_phy_addr, unsigned int tsize, int bsize)
{
    set_npu_reg_data(CR_NPU_MIB9, 0);
    set_npu_test_case(42);

    set_npu_reg_data(CR_NPU_MIB10, src_phy_addr);
    set_npu_reg_data(CR_NPU_MIB11, dst_phy_addr);
    set_npu_reg_data(CR_NPU_MIB12, tsize);
    set_npu_reg_data(CR_NPU_MIB13, bsize);
}
EXPORT_SYMBOL(set_npu_dma_config);

void enable_npu_dma(void)
{
    set_npu_reg_data(CR_NPU_MIB9, 1);
}
EXPORT_SYMBOL(enable_npu_dma);

int is_npu_dma_done(void)
{
    if (get_npu_reg_data(CR_NPU_MIB9)==0)
        return 1;
    else
        return 0;
}
EXPORT_SYMBOL(is_npu_dma_done);

void enable_npu_rbus_decoder(int enable)
{
    u32 val;

    if (get_l2c_sram_size()==S_128K) {
        
        val = get_npu_reg_data(CR_NPU_RBUS_DECODER);

        if (enable) {
            if (val&0x1) return;
            val |= (0x1);
        }
        else {
            if ((val&0x1)==0) return;
            val &= (~(0x1));
        }

        set_npu_reg_data(CR_NPU_RBUS_DECODER, val);
    }
    return;
}
EXPORT_SYMBOL(enable_npu_rbus_decoder);

unsigned long get_npu_384k_sram_base(void)
{
    return (unsigned long)(ecnt_npu->base+(unsigned long)NPU_384K_SRAM_BASE);
}
EXPORT_SYMBOL(get_npu_384k_sram_base);

struct device* get_npu_dev(void)
{
    if ((ecnt_npu) && (ecnt_npu->dev))
        return ecnt_npu->dev;
    else
        return NULL;
}
EXPORT_SYMBOL(get_npu_dev);

static void host_register_mbox_cb(npuCoreId_t core_id, npuMboxFuncId_t func_id, h_mbox_cb_t cb)
{   
    if ((core_id>=MAX_CORE_NUM) || (func_id>=MAX_MBOX_FUNC)) {
        printk("Error: (core_id:%d >= %d) || (func_id:%d >= %d)\n", core_id, MAX_CORE_NUM, func_id, MAX_MBOX_FUNC);
        return;
    }
    #ifdef MBOX_DBG_ON
    printk("%s: core_id:%d, func_id:%d, cb:%p\n", __func__, core_id, func_id, cb);
    #endif
    mboxObj[core_id].mbox8_cb[func_id] = cb;
    return;
}

/* 
 * Returned Value: 
 *  -- its range is 0~7.  0: error.  1~7: user defined and returned from NPU's callback function.
 *
 * npuMboxInfo_t's members are described as follows:
 *
 * core_id:
 *  -- the NPU core that Host is going to nority.
 *  -- its value is 0,1,2,or 3. corresponding enum "core0","core1","core2","core3" 
 *     are defined in global_inc/modules/npu/npuMboxAPI.h
 *
 * func_id:
 *  -- the app (running on NPU core) that Host is going to nority.
 *  -- its value is defined in "enum npuMboxFuncId" in global_inc/modules/npu/npuMboxAPI.h
 *     user can add his app's func_id if needed. Note: max func_id is deifned as MAX_MBOX_FUNC.
 *
 * virtAddr,physAddr,len:
 *  -- the virtual address, physical address, and size of the memory buffer that Host wants
 *     NPU's (core,app) to handle.
 *
 * flags:
 *  -- its members are defined in npuMboxAPI.h. The following describes its members.
 *    -- isBlockingMode:
 *      -- isBlockingMode==1 means blocking mode. In this mode, host will be blocked until
 *          NPU's (core,app) finishs its corresponding callback function and returns
 *          status back to host.
 *      -- isBlockingMode==0 means non-blocking mode. In this mode, host will leave this
 *          API as soon as NPU's core receives mbox interrupt and returns 1 back to host.
 *          Make sure memory buffer used between Host and NPU can't be released when NPU's
 *          callback is still using it.
 *    -- isStaticBuffInit:
 *      -- only for internal use.
 *      -- This API's "cb" will be registered, and the memory buffer's (phyAddr,len) will
 *          passed to NPU.  Later NPU can notify host of information by the memory buffer.
 *
 * blockTimeout:
 *  -- valid when isBlockingMode==1. 
 *  -- It means the max time that host can be blocked. Its unit is us.
 *
 * cb:
 *  -- should be "NULL" when isBlockingMode==1.
 *  -- when isBlockingMode==0, after NPU finishs its callback, NPU will notify Host to execute 
 *      the "cb". Set "cb" as "NULL" if not needed.
 */
int host_notify_npuMbox(npuMboxInfo_t *mboxInfo)
{
    int reg_shift;
    mboxArg_t arg;
    int res=0;
    int timeoutCnt=300; /* default 30ms */

    if ((mboxInfo->core_id>=MAX_CORE_NUM) || (mboxInfo->func_id>=MAX_MBOX_FUNC)) {
        printk("Error: (core_id:%d >= %d) || (func_id:%d >= %d)\n", 
                mboxInfo->core_id, MAX_CORE_NUM, mboxInfo->func_id, MAX_MBOX_FUNC);
        return 0;
    }

    arg.hWord=0;

    if ((mboxInfo->flags.isStaticBuffInit)||(mboxInfo->flags.isStaticBuffDel)) {
        arg.bits.static_buff=1;
    }

    if (mboxInfo->flags.isBlockingMode) {
        if (mboxInfo->blockTimeout)
            timeoutCnt = mboxInfo->blockTimeout;
        arg.bits.block_mode=1;
    }

    host_register_mbox_cb(mboxInfo->core_id, mboxInfo->func_id, mboxInfo->cb);

    arg.bits.func_id= (mboxInfo->func_id&MASK_FUNC_ID);
    reg_shift = (mboxInfo->core_id<<4);
    if (((mboxInfo->virtAddr)&&(mboxInfo->physAddr)&&(mboxInfo->len)) ||
        (mboxInfo->flags.isStaticBuffDel)) {
        mboxObj[mboxInfo->core_id].virtAddr[mboxInfo->func_id] = mboxInfo->virtAddr;
        mboxObj[mboxInfo->core_id].physAddr[mboxInfo->func_id] = mboxInfo->physAddr;
        mboxObj[mboxInfo->core_id].size[mboxInfo->func_id] = mboxInfo->len;
    }
    else {
        if (mboxObj[mboxInfo->core_id].virtAddr[mboxInfo->func_id]==0) {
            printk("Error(%s): no default (virtAddr,physAddr,len), please assign them!\n", __func__);
            return 0;
        }
    }

    spin_lock_bh(&mboxObj[mboxInfo->core_id].npuMboxLock);

    #ifdef MBOX_DBG_ON
    printk("%s: core_id:%d, func_id:%d, arg:0x%x, timeoutCnt:%d\n", 
            __func__, mboxInfo->core_id, mboxInfo->func_id, arg.hWord, timeoutCnt);
    #endif
    set_npu_reg_data(CR_MBQ0_CTRL0+reg_shift, mboxObj[mboxInfo->core_id].physAddr[mboxInfo->func_id]);
    set_npu_reg_data(CR_MBQ0_CTRL1+reg_shift, mboxObj[mboxInfo->core_id].size[mboxInfo->func_id]);
    set_npu_reg_data(CR_MBQ0_CTRL3+reg_shift, arg.hWord);
    set_npu_reg_data(CR_MBQ0_CTRL2+reg_shift, get_npu_reg_data(CR_MBQ0_CTRL2+reg_shift)+1);

    while (1) 
    {
        arg.hWord = get_npu_reg_data(CR_MBQ0_CTRL3+reg_shift);
        if (arg.bits.done_bit==1) {
            res = (arg.bits.ret_status&MASK_RET_STATUS);
            break;
        }
        
        timeoutCnt--;
        if (timeoutCnt>=0)
            udelay(100);
        else {
            printk("Error: %s timeout for core_id:%d, func_id:%d\n", __func__, mboxInfo->core_id, mboxInfo->func_id);
            break;
        }
    }
    #ifdef MBOX_DBG_ON
    printk("%s exit (res:%d, timeoutCnt:%d)\n", __func__, res, timeoutCnt);
    #endif

    spin_unlock_bh(&mboxObj[mboxInfo->core_id].npuMboxLock);
    
    return res;
}
EXPORT_SYMBOL(host_notify_npuMbox);

#if 0
/*
 * This API is used to get memory buffer's info (virtAddr,physAddr,size) by (core_id,func_id)
 */
int host_get_mboxBuff(
        npuCoreId_t core_id, npuMboxFuncId_t func_id, 
        unsigned long *virtAddr_p, unsigned int *physAddr_p, unsigned short *size_p)
{
    mboxObj[core_id].virtAddr[func_id]==0
    if ((mboxObj[core_id].virtAddr[func_id]==0) ||
        (mboxObj[core_id].physAddr[func_id]==0) ||
        (mboxObj[core_id].size[func_id]==0)) {
        printk("Error(%s): virtAddr:0x%x==0 or physAddr:0x%x==0 or size:0x%x==0\n", 
                __func__, mboxObj[core_id].virtAddr[func_id], 
                mboxObj[core_id].physAddr[func_id], mboxObj[core_id].size[func_id]);
        return -1;
    }

    *virtAddr_p = mboxObj[core_id].virtAddr[func_id];
    *physAddr_p = mboxObj[core_id].physAddr[func_id];
    *size_p = mboxObj[core_id].size[func_id];
    return 0;;
}
EXPORT_SYMBOL(host_get_mboxBuff);
#endif
/*
 * Host allocates uncached memory buffer of "size" bytes, passes the memory buffer's info to NPU, 
 * and registers "cb".
 * Later, NPU can use (core_id,func_id) to notify Host to execute the "cb". Host can use
 * (core_id,func_id) to notify NPU also.
 */
int create_mbox_notify (npuCoreId_t core_id, npuMboxFuncId_t func_id, unsigned short size, h_mbox_cb_t cb)
{
    struct device *dev=NULL;
    dma_addr_t dma_addr;
    unsigned long unc_addr;
    npuMboxInfo_t mboxInfo;

    if (mboxObj[core_id].virtAddr[func_id]) {
        printk("Error(%s): already exist for core_id:%d, func_id:%d\n", __func__, core_id, func_id);
        return -1;
    }

    if ((dev=get_npu_dev())==NULL) {
        printk("%s get_npu_dev failed\n", __func__);
        return -1;
    }

    unc_addr = (unsigned long) dma_alloc_coherent(dev, size, &dma_addr, GFP_KERNEL);
    if (!unc_addr)  {
        printk("%s dma_alloc_coherent failed\n", __func__);
        return -1;
    }

    
    #ifdef MBOX_DBG_ON
    printk("%s core_id:%d, func_id:%d dma_addr:0x%x\n", __func__, core_id, func_id, dma_addr);
    #endif

    memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
    mboxInfo.core_id=core_id;
    mboxInfo.func_id=func_id;
    mboxInfo.virtAddr=unc_addr;
    mboxInfo.physAddr=dma_addr;
    mboxInfo.len=size;
    mboxInfo.flags.isStaticBuffInit=1;
    mboxInfo.cb=cb;
    host_notify_npuMbox(&mboxInfo);

    return 0;
}
EXPORT_SYMBOL(create_mbox_notify);

/*
 * release the resource allocated by create_mbox_notify().
 */
void delete_mbox_notify (npuCoreId_t core_id, npuMboxFuncId_t func_id)
{
    npuMboxInfo_t mboxInfo;

    #ifdef MBOX_DBG_ON
    printk("%s (core_id:%d, func_id:%d)\n", __func__, core_id, func_id);
    #endif

    spin_lock(&mboxObj[core_id].mboxBuffLock);

    dma_free_coherent(ecnt_npu->dev, mboxObj[core_id].size[func_id], 
                    (void*)mboxObj[core_id].virtAddr[func_id], (dma_addr_t)mboxObj[core_id].physAddr[func_id]);

    memset(&mboxInfo, 0, sizeof(npuMboxInfo_t));
    mboxInfo.core_id=core_id;
    mboxInfo.func_id=func_id;
    mboxInfo.flags.isStaticBuffDel=1;
    host_notify_npuMbox(&mboxInfo);

    spin_unlock(&mboxObj[core_id].mboxBuffLock);
    
    return;
}
EXPORT_SYMBOL(delete_mbox_notify);

static void mbox_tasklet_handler(unsigned long data)
{
    unsigned short core_id = get_npu_reg_data(CR_MBQ8_CTRL0)&MASK_CORE_ID;
    unsigned short func_id;
    unsigned short len = get_npu_reg_data(CR_MBQ8_CTRL1);
    int status=0;
    mboxArg_t arg;
    
    arg.hWord = get_npu_reg_data(CR_MBQ8_CTRL3);
    func_id = arg.bits.func_id;

    #ifdef MBOX_DBG_ON
    printk("%s: core_id:%d, func_id:%d, arg:0x%x\n", __func__, core_id, func_id, arg.hWord);
    #endif

    spin_lock(&mboxObj[core_id].mboxBuffLock);
    
    if(mboxObj[core_id].mbox8_cb[func_id]) {
        status = mboxObj[core_id].mbox8_cb[func_id] (mboxObj[core_id].virtAddr[func_id], len);
    }
    
    spin_unlock(&mboxObj[core_id].mboxBuffLock);

    arg.bits.ret_status=(status&MASK_RET_STATUS);
    arg.bits.done_bit=1;

    /* let NPU know that host has done */
    set_npu_reg_data(CR_MBQ8_CTRL3, arg.hWord);
    
    return;
}

static irqreturn_t npuMbox2host_isr(int irq, void *dev_id)
{
	/* clear mbox8 interrupt */
    set_npu_reg_data(CR_MBOX_INTR_STATUS, 1<<MBOX2HOST_IDX);

	tasklet_schedule(&mbox_tasklet);
	
	return IRQ_HANDLED;
}

static int ecnt_npu_drv_probe(struct platform_device *pdev)
{
    struct resource *res = NULL, *res2 = NULL;
    struct device_node *np = NULL;
    struct resource r;
    int rc, ret;
    npuCoreId_t core_id;
#ifdef TCSUPPORT_NPU_WIFI_OFFLOAD
	int irq;
	int irq_idx;
#endif

    if (!pdev->dev.of_node) {
        dev_err(&pdev->dev, "No npu DT node found\n");
        return -EINVAL;
    }

    ecnt_npu = devm_kzalloc(&pdev->dev, sizeof(struct ecnt_npu), GFP_KERNEL);
    if (!ecnt_npu)
        return -ENOMEM;

    platform_set_drvdata(pdev, ecnt_npu);

    /* get NPU base address */
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    ecnt_npu->base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(ecnt_npu->base))
        return PTR_ERR(ecnt_npu->base);

	/* get npu 384k sram size from dts*/
	npu_384k_sram_size = resource_size(res);

    res2 = platform_get_resource(pdev, IORESOURCE_MEM, 1);
    ecnt_npu->Regbase = devm_ioremap_resource(&pdev->dev, res2);
    if (IS_ERR(ecnt_npu->Regbase))
        return PTR_ERR(ecnt_npu->Regbase);
#ifdef TCSUPPORT_NPU_WIFI_OFFLOAD
	for( irq_idx = 0; irq_idx < NPU_INT_MAX_NUM; irq_idx++ ){
		irq = platform_get_irq(pdev, irq_idx);
		if (irq <= 0)
			return irq;
		ecnt_npu->irq[irq_idx] = irq;
	}
#endif


    ecnt_npu->dev = &pdev->dev;

    np = of_parse_phandle(pdev->dev.of_node, "memory-region", 0);
    if (!np) {
        dev_err(&pdev->dev, "No memory-region specified\n");
        return -EINVAL;
    }

    rc = of_address_to_resource(np, 0, &r);
    if (rc) {
        dev_err(&pdev->dev, "No memory address assigned to region\n");
        return -EINVAL;
    }
    
    ecnt_npu->resv_paddr = r.start;
    ecnt_npu->resv_vaddr = memremap(r.start, resource_size(&r), MEMREMAP_WB);

	ecnt_npu->mbox2host_irq = platform_get_irq(pdev, 6);
	if(ecnt_npu->mbox2host_irq <= 0) {
		printk("\n get NPU mbox2host_irq:%d failed\n", ecnt_npu->mbox2host_irq);
		return ecnt_npu->mbox2host_irq;
	}

	ret = request_irq(ecnt_npu->mbox2host_irq, npuMbox2host_isr, 0, "npuMbox2host", ecnt_npu->dev);
	if(ret) {
		printk("\n request_irq() (irq number: %d) failed (ret: %d)\n", ecnt_npu->mbox2host_irq, ret);
		return (ret);
	}

    tasklet_init(&mbox_tasklet, mbox_tasklet_handler, 0);

    for (core_id=0; core_id<MAX_CORE_NUM; core_id++) 
    {
        memset(&mboxObj[core_id], 0, sizeof(npuMboxObj_t));
        spin_lock_init(&mboxObj[core_id].npuMboxLock);
        spin_lock_init(&mboxObj[core_id].mboxBuffLock);
    }

#if 0
	printk("[npu] res->name:%s\n", res->name);
	printk("[npu] res->start:0x%llx ===\n", res->start);
	printk("[npu] res->end:0x%llx ===\n", res->end);
    printk("[npu] ecnt_npu->base:0x%lx\n", (unsigned long)ecnt_npu->base);
    printk("[npu] res2->name:%s\n", res2->name);
	printk("[npu] res2->start:0x%llx ===\n", res2->start);
	printk("[npu] res2->end:0x%llx ===\n", res2->end);
    printk("[npu] ecnt_npu->Regbase:0x%lx\n", (unsigned long)ecnt_npu->Regbase);
    printk("[npu_resv] ecnt_npu->resv_paddr:0x%lx\n", (unsigned long)ecnt_npu->resv_paddr);
    printk("[npu_resv] ecnt_npu->resv_vaddr:0x%lx\n", (unsigned long)ecnt_npu->resv_vaddr);
    printk("[npu_resv] resource_size:0x%lx\n", (unsigned long)resource_size(&r));
    printk("\nrequest_irq for NPU mbox2host_irq:%d done\n", ecnt_npu->mbox2host_irq);
#endif

    return 0;
}

static int ecnt_npu_drv_remove(struct platform_device *pdev)
{
    tasklet_kill(&mbox_tasklet);
    return 0;
}


/************************************************************************
*      P L A T F O R M   D R I V E R S   D E C L A R A T I O N S
*************************************************************************
*/
static struct platform_driver ecnt_npu_driver = {
    .probe = ecnt_npu_drv_probe,
    .remove = ecnt_npu_drv_remove,
    .driver = {
        .name = "ecnt-npu",
        .of_match_table = ecnt_npu_of_id
    },
};
module_platform_driver(ecnt_npu_driver);


MODULE_DESCRIPTION("EcoNet NPU Driver");


