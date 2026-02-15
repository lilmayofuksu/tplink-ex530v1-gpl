
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#ifndef TCSUPPORT_CPU_ARMV8 
#include <asm/addrspace.h>
#endif
#include <asm/tc3162/tc3162.h>
#include <linux/dma-mapping.h>
#include <linux/random.h>

#include "npu/npu_test_common.h"

#define CR_NPU_MIB8 (0xbec0c160)
#define CR_NPU_MIB9 (0xbec0c164)
#define CR_NPU_MIB19    (0xbec0c18c)
#define NPU_ALL_TESTS   (99)

#ifdef TCSUPPORT_CPU_ARMV8
#define NPU_SRAM_CPU_TEST_OFF   (0x40000)
#define NPU_SRAM_CPU_TEST_SIZE  (0x20000)
#define L2C_SRAM_CPU_TEST_OFF   (0x14000) //80K
#define L2C_SRAM_CPU_TEST_SIZE  (0xC000)   //48K
#define L2C_SRAM_PBUS_BASE		(0x1EFC0000)
#define NPU_384K_SRAM_ADDR 		(0x1E800000)
#define NPU_384KSRAM_TEST_SIZE	(0x10000)
#endif

#define HOST_DRAM_TEST_SIZE (0x400000)
#define MBOX_BASE_UNC_ADDR  (0xbec0c000)
#define MBOX_INTR_STATUS    (MBOX_BASE_UNC_ADDR+0x000)
#define MBOX_INT_MASK8      (MBOX_BASE_UNC_ADDR+0x024)
#define MBQ8_CTRL0          (MBOX_BASE_UNC_ADDR+0x0b0)

#define NPU_CORE_OFF_THRESH	(0x30)
#define REG_CORE_BOOT_TRIGGER	(0xbec08000)
#define REG_CORE_BOOT_CONFIG	(0xbec08004)
#define DYNAMIC_CLK_STRESS_TEST	0
#define RBUS_STRESS_TEST	1
spinlock_t cpu0AccessLock;
#define CHECK_TIMES 5
#define SIZE_4K             (0x1000)
#define wtMemW(addr,val)        *(volatile unsigned int*)(addr)=(unsigned int)(val)
#define rdMeml(addr)                    *(volatile unsigned long*)(addr)
#define wtMeml(addr,val)        *(volatile unsigned long*)(addr)=(unsigned long)(val)


unsigned int testPatterns[] = {0x55aa55aa, 0xaa55aa55, 0x5a5a5a5a, 0xa5a5a5a5, 0xff00ff00, 0x00ff00ff, 0x12345678, 0x87654321};
#ifndef TCSUPPORT_CPU_ARMV8 	
unsigned int mbox2host_flag[3] = {0};
#else
unsigned int mbox2host_flag[6] = {0};
#endif

enum coreState {
    WORKING=0,
    STOPPED, 
    REBOOTING,
    REBOOTED 
};

enum testResult {
    NOT_YET_FINISHED=0,
    TEST_FAILED, 
    TEST_PASSED
};

enum npuIrqNum {
    WDOG_IRQ0=0,
    WDOG_IRQ1, 
    WDOG_IRQ2,
    WDOG_IRQ3,
    MBOX_IRQ
};

extern void host_dump_npu_csr(int core);
#ifdef TCSUPPORT_CPU_ARMV8
extern struct device* get_gdmpSram_dev(void);
extern void enable_npu_rbus_decoder(int enable);
extern int get_npu_test_case(void);
extern int test_npu_384k_sram (unsigned long offset, unsigned int testSize, unsigned long patAdd);
extern int l2c_sram_test(unsigned long offset, unsigned int t_size, unsigned long patAdd);
extern void set_npu_mib_index(u32 reg,u32 val);
extern u32 get_npu_mib_index(u32 reg);
extern void SET_RCU_CPU_STALL_SUPRESS(void);
extern void SET_GDMA_CONFIG(u32 channel, u32 sa, u32 da, u32 ct0, u32 ct1);
extern u32 IS_GDMA_DONE(u32 channel);
extern void CLEAR_GDMA_DONE(u32 channel);
extern int l2c_sram_read_test (unsigned int l2cSram_off, unsigned int size, unsigned long patAdd);
extern int l2c_sram_write_test (unsigned int l2cSram_off, unsigned int size, unsigned long patAdd);
extern u32 get_npu_384k_sram_data(u32 reg);
extern void set_npu_384k_sram_data(u32 reg, u32 val);
#endif

#ifndef TCSUPPORT_CPU_ARMV8 /* Host CPUs access test will be implemented in tcci's cpu_bus_test.c */
extern int npuRamTest(int test_case);

static void host_access_test (int isCached)
{
    unsigned int pattern = 0xa5a5a5a5;
    unsigned int round=0;
    int testCase;

    if (isCached)
        testCase = 2;
    else
        testCase = 1;

    while (1) {

        /* access NPU Reg test */
    
        regWrite32(CR_NPU_MIB19, pattern);
        
        if (regRead32(CR_NPU_MIB19) != pattern) {
            printk("\n[H]ERROR: regRead32(CR_NPU_MIB19):0x%x != pattern:0x%x\n", regRead32(CR_NPU_MIB19), pattern);
            break;
        }

        if (pattern==ALL_FF) 
            pattern=0;
        else
            pattern++;

        /* access NPU 4k/64K/384K SRAM test */

        if ((npuRamTest(testCase)==-1) || (regRead32(CR_NPU_MIB8)==0)) /* if NPU Core failed, CR_NPU_MIB8 will be set to 0*/
            break;

        if ((round&0xf) == 0)
            printk("[H%d]\n", round);

        if (round==ALL_FF)
            round=0;
        else
            round++;
    }

    return;
}
#endif

static irqreturn_t mbox2host_isr(int irq, void *dev_id)
{
#ifndef TCSUPPORT_CPU_ARMV8 	
    unsigned int regVal = regRead32(CR_NPU_MIB19);


	if (regVal==1) { /* for mBox0~7 intr enable test */
			mbox2host_flag[0]++;
	}
	else if (regVal==2){ /* for mBox8~15 intr enable test */
			mbox2host_flag[1]++;
	}
	else if (regVal==3){ /* for mBox0~7 & 8~15 intr disable test */
			mbox2host_flag[2]++;
	}
	else {
	}
	
	/* before npu_core0 clear mbox0's status, host may enter this isr several times,
	 * so set CR_NPU_MIB19 as 0 before leaving, otherwise, mbox2host_flag[] may be updated too many times */
	regWrite32(CR_NPU_MIB19, 0);
#else
	unsigned int regVal = read_mbox2host_flag();

	printk("[H]Read MIB19<0x%x>\n", regVal);
	if ((regVal > 0) && (regVal < 7))
		mbox2host_flag[regVal-1]++;

	clear_mbox2host_flag();
#endif

    return IRQ_HANDLED;
}

/*
 * 1. register mbox2host_isr to Host CPU.
 * 2. check if NPU's Mbox can send interrupts to Host CPU.
 */
static void host_mboxIrq_test(void)
{
    int i;

    for (i=0; i<3; i++) {
        mbox2host_flag[i]=0;
    }
#ifndef TCSUPPORT_CPU_ARMV8	
    request_irq(DMT_INT, mbox2host_isr, 0, "Mbox2Host", NULL);
#else
	i = request_irq(get_npu_irq_num(MBOX_IRQ), mbox2host_isr, 0, "Mbox2Host", NULL);
	if (i < 0)
		printk("[H] request mbox irq fail\n");
	printk("request_irq<%d>\n", i);
#endif
    /* wait until all 8 Cores finish sending mbox2host_irq.
     * Note: mbox2host_flag[] will be updated in mbox2host_isr */
    while(1) {
#ifndef TCSUPPORT_CPU_ARMV8	
        if ((mbox2host_flag[0]==MAX_CORE_NUM) && (mbox2host_flag[1]==MAX_CORE_NUM))
#else
		if ((mbox2host_flag[0]==MAX_CORE_NUM) && (mbox2host_flag[1]==MAX_CORE_NUM) 
			&& (mbox2host_flag[2]==MAX_CORE_NUM) && (mbox2host_flag[3]==MAX_CORE_NUM))
#endif
            break;
        mdelay(1000);
    }
#ifndef TCSUPPORT_CPU_ARMV8	
    free_irq(DMT_INT, NULL) ;

	if (mbox2host_flag[2]==0)
		printk("[H] done\n");
	else
		printk("[H] fail%d\n", (int)mbox2host_flag[2]);
#else
	free_irq(get_npu_irq_num(), NULL);

	if ((mbox2host_flag[4]==0) || (mbox2host_flag[5]==0)) {
		printk("[H] done\n");
	} else {
		if (mbox2host_flag[4] != 0)
			printk("[H] fail%d\n", (int)mbox2host_flag[4]);
		else
			printk("[H] fail%d\n", (int)mbox2host_flag[5]);
	}
#endif


    return;
}

/*
 * R/W test on its part of NPU's MBox registers for 500000 times.
 */
static void host_rw_mbox_regs_test(void)
{
    unsigned int baseVal, tmpVal, cmpVal;
    int i;
    unsigned int count=0;

    mdelay(250); /* make sure NPU Core0 is finishing mbox_reset */

    while(1) {

        baseVal = random32();
#ifndef TCSUPPORT_CPU_ARMV8	
        /* write data */

        regWrite32(MBOX_INT_MASK8, baseVal);
        for (i=0; i<8; i++) {
            regWrite32(MBQ8_CTRL0+(i<<2), baseVal+1+i);
        }

        /* confirm data */

        cmpVal = baseVal&0xffff;
        if ((tmpVal=regRead32(MBOX_INT_MASK8)) != cmpVal) {
            printk("[H]Error: MBOX_INT_MASK8:0x%x != (baseVal&0xffff):0x%x at count%d\n", tmpVal, cmpVal, count);
            return;
        }
        for (i=0; i<8; i++) {
            cmpVal = baseVal+1+i;
            if (!(i==0 || i==4))
                cmpVal &= 0xffff;
            if ((tmpVal=regRead32(MBQ8_CTRL0+(i<<2))) != cmpVal) {
                printk("Error: MBQ8_REG:0x%x != (baseVal+1+%d):0x%x at count%d\n", tmpVal, i, cmpVal, count);
                return;
            }
        }
#else
	/* write and confirm data */
	if (host_rd_chk_mbox_regs(baseVal, count))
		return;
#endif
        count++;
        if (count==500000)
            break;
    }
#ifndef TCSUPPORT_CPU_ARMV8	
    /* clear all MBox Status */
    regWrite32(MBOX_INTR_STATUS, 0xffff);
#else
    clear_mbox_intr_status();
#endif
    printk("[H] done\n");
    return;
}

#if 0
static void host_mdelay_test(void)
{
    int i;

    regWrite32(CR_NPU_MIB19, 0);
    __asm__ volatile ("sync");

    /* make sure that NPU Core is read */
    mdelay(2);

    /* trigger the test */
    regWrite32(CR_NPU_MIB19, 1);
    __asm__ volatile ("sync");
    
    mdelay(MDELAY_TEST_CNT);
    
    regWrite32(CR_NPU_MIB19, MDELAY_TEST_CNT);
    __asm__ volatile ("sync");

    return;
}
#endif

#ifndef TCSUPPORT_CPU_ARMV8	
static void host_set_npu_core_on_off(int core, int on) {

	unsigned int tmpVal; 

	tmpVal = regRead32(REG_CORE_BOOT_CONFIG);
	if (on) {
		tmpVal = (tmpVal |(1<<core));
	} else {
		tmpVal = (tmpVal & (~(1<<core)));
	}
	regWrite32(REG_CORE_BOOT_CONFIG, tmpVal);
	regWrite32(REG_CORE_BOOT_TRIGGER, 0x2);
	mdelay(100);
}
#endif

static void host_reset_all_npu_cores(void) {
	int i;

	for (i = 0; i < MAX_CORE_NUM; i++) {
		host_set_npu_core_on_off(i, 0);
	}
	
	for (i = 0; i < MAX_CORE_NUM; i++) {
		host_set_npu_core_on_off(i, 1);
	}

	return;
}

/* NPU core will trigger IRQ and Host should be able to recieve IRQ */
static irqreturn_t npu_wdog_isr(int irq, void *dev_id)
{
	int i;
	/* Let npu cores print ISR content. */
	mdelay(100);
	printk("Recieve irq from npu watchdog, irq_num<%d>\n", irq);
	host_dump_npu_csr(irq-7);
	
	return IRQ_HANDLED;
}

static void host_npu_wdog_test(void) 
{
	int i, status;

	for (i = 0; i <= WDOG_IRQ3; i++) {
		status = request_irq(get_npu_irq_num(i), npu_wdog_isr, 0, "NPU_WDOG", NULL);
	}

	while(npu_wdog_testing())
		mdelay(1000);
	host_reset_all_npu_cores();
	
	return;
}

static void host_npu_core_on_off_test(void)
{
	unsigned int cntAddr;
	unsigned int preVal[MAX_CORE_NUM] = {0};
	unsigned int curVal[MAX_CORE_NUM] = {0};
	unsigned int coreStates[MAX_CORE_NUM] = {0};
	int core_id, target, check, result;

	target = check = result = 0;

	mdelay(500);
	/* Wait NPU cores start to count */
	for (core_id = 0; core_id < MAX_CORE_NUM; core_id++) {
		preVal[core_id] = 0;
		curVal[core_id] = 0;
	}

	core_id = 0;
	while (result == NOT_YET_FINISHED) {
#ifndef TCSUPPORT_CPU_ARMV8
		cntAddr = CR_NPU_MIB19 + (core_id*4); 
		/* Each core will keep counting and write to rigisters from MIB19. */
		preVal[core_id] = curVal[core_id];
		curVal[core_id] = regRead32(cntAddr);
#else
		preVal[core_id] = curVal[core_id];
		curVal[core_id] = npu_test_get_count(core_id);
#endif
		if (target == core_id) {
			/* [target]The target core to be turned off/on in this round. */
			switch (coreStates[target]) {
				case WORKING:
					host_set_npu_core_on_off(target, 0);
					coreStates[target] = STOPPED;
					check = 0;
					printk("[H][C%d] Try to stop\n", core_id);
					break;

				case STOPPED:
					if (curVal[core_id] == preVal[core_id]) { 
						printk("[H][C%d] Stop check round<%d>\n", core_id, check+1);
						check++;
					} else {
						printk("[H][C%d] Not stopped!TEST FAILED!\n", core_id);
						result = TEST_FAILED;
						break;
					}
					if (check == CHECK_TIMES) {
						host_set_npu_core_on_off(target, 1);	
						check = 0;
						coreStates[target] = REBOOTING;
					}
					break;

				case REBOOTING:
					if (curVal[target] != preVal[target]) {
						coreStates[target] = REBOOTED;
						printk("[H][C%d] Rebooted\n", core_id);
						target++;
						if (target == MAX_CORE_NUM) {
							printk("\n[H] All NPU TEST PASSED!\n");
							result = TEST_PASSED;
						}
					} else {
						check++;
						if (check == CHECK_TIMES) {
							printk("[H][C%d] Reboot failed! TEST FAILED!\n", core_id);
							result = TEST_FAILED;
							} else {
							printk("[H][C%d] Still rebooting\n", core_id);
						}
					}
					break;
					
				default:
					printk("[H][C%d] In unknown state. TEST FAILED!\n", core_id);
					result = TEST_FAILED;
					break;
			}
		}else 	if (curVal[core_id] != preVal[core_id]) {
			printk("[H][C%d] CNT<%x>, PRE<%x>\n", core_id, curVal[core_id], preVal[core_id]);
			/* If the core is not target, print out the coutners. */
		} else {
			printk("[H][C%d] Unexpectedly Stopped. TEST FAILED!\n", core_id);
			result = TEST_FAILED;
		}
	
		core_id=(core_id+1)%MAX_CORE_NUM;
		if (core_id == 0) {
			printk("\n\n");
			mdelay(200); 		
		}
	}

#ifndef TCSUPPORT_CPU_ARMV8	
	regWrite32(CR_NPU_MIB8, 0x0);
#else
	set_npu_test_case(0);
#endif
	host_reset_all_npu_cores();
	if (result == TEST_PASSED)
		printk("[H] host_hang_core_csr_test done\n\n");

	return;
}

static void host_hang_core_csr_test(void)
{
	unsigned int tmpVal;
	int i;

	mdelay(500); 
	/* Wait npu cores get into infinite loop. */
#ifndef TCSUPPORT_CPU_ARMV8
	tmpVal = regRead32(CR_NPU_MIB19);
#else
	tmpVal = get_hanged_npu_cores();
#endif
	for (i = MAX_CORE_NUM-1; i >= 0; i--){
		if ((tmpVal &(1<<i)) > 0){
			host_dump_npu_csr(i);
#ifndef TCSUPPORT_CPU_ARMV8
			tmpVal = ( regRead32(CR_NPU_MIB19)& (~(1<<i)));
			regWrite32(CR_NPU_MIB19, tmpVal);
			tmpVal = regRead32(CR_NPU_MIB19);
#else
			release_hanged_core(i);
			tmpVal = get_hanged_npu_cores();
#endif
			printk("[H] tmpVal = 0x%lx\n", tmpVal);
			mdelay(500);
		}
	}

	printk("[H] host_hang_core_csr_test done\n");

	return;
}

static void host_dead_core_csr_test(void)
{
	  int i;

	  mdelay(500); 
	/* Wait npu cores get into dead status. */
	for (i = MAX_CORE_NUM-1; i >= 0; i--){
		host_dump_npu_csr(i);
		mdelay(500);
	}

	printk("[H] host_dead_core_csr_test done\n");
#ifndef TCSUPPORT_CPU_ARMV8	
	regWrite32(CR_NPU_MIB8, 0x0);
#else
	set_npu_test_case(0);
#endif
	host_reset_all_npu_cores();

	return;
}

static void host_npu_module_reset_test(int test_case)
{
	int i;

	for (i = 0; i<3; i++){
		printk("[H] Wait for %s ROUND %d\n", __func__, i+1);
#ifndef TCSUPPORT_CPU_ARMV8	
		while(regRead32(CR_NPU_MIB8) != 0) {
#else
		while(get_npu_test_case() != 0) {
#endif
			mdelay(500);
		}
		if (i ==2)
			break;
		mdelay(500);
		printk("\n\n[H] Set %s ROUND %d\n", __func__,  i+2);
#ifndef TCSUPPORT_CPU_ARMV8
		regWrite32(CR_NPU_MIB8, test_case);
#else
		set_npu_test_case(test_case);
#endif
	}
	
	printk("[H] %s done\n", __func__);
	return;
}

static void host_npu_hw_kernel_reset_test(int testCase)
{
	while (npu_hw_kern_reset_testing());
	mdelay(500);
	host_reset_all_npu_cores();
}

static void host_write_dram_starving_test(void)
{
    unsigned int *unc_addr;
    struct device *dev=NULL;
    dma_addr_t phy_addr;
    unsigned int totalCnt=(SIZE_4K/sizeof(unsigned int));
    unsigned int testCnt=1, reply=0;

    #ifdef TCSUPPORT_CPU_ARMV8
    if ((dev=get_gdmpSram_dev())==NULL) {
        printk("\nget_gdmpSram_dev failed\n");
		return;
    }
    #endif
    
    unc_addr = (unsigned int *)dma_alloc_coherent(dev, SIZE_4K, &phy_addr, GFP_KERNEL);
    if (unc_addr == NULL) {
        printk("\nError(%s):dma_alloc_coherent failed\n", __func__);
        return;
    }

    printk("\nunc_addr:0x%lx, phy_addr:0x%x\n", unc_addr, (unsigned int)phy_addr);

    start_npu_cores_reading_addr((unsigned int)phy_addr);
    mdelay(100);

    while (1) {

        wtMemW(unc_addr, testPatterns[testCnt&0x7]);
        mdelay(100);
        set_npu_core_test_cnt(testCnt);
        mdelay(100);
        if ((reply=get_npu_core_reply())!=(testCnt)) {
            printk("\nError: reply:0x%x != testCnt:0x%x when unc_addr:0x%lx\n", reply, testCnt, unc_addr);
            goto host_write_dram_starving_out;
        }

        unc_addr++;
        testCnt++;
        if (testCnt==totalCnt)
            break;

        if ((testCnt&0x3f)==0) {
            printk("testCnt:0x%x OK\n", testCnt);
            msleep(100);
        }
    }

    printk("\n%s done\n", __func__);
host_write_dram_starving_out:
    finish_wt_dram_starving_test();
    dma_free_coherent(dev, SIZE_4K, unc_addr, phy_addr);
    return;
}

void cpuTest_locking(spinlock_t *lock, unsigned long *flags)
{    
    
    spin_lock_init(lock);
    spin_lock_irqsave(lock, (*flags)) ;
    return;
}

void cpuTest_unlocking(spinlock_t *lock, unsigned long *flags)
{    

    spin_unlock_irqrestore(lock, (*flags)) ;
    return;
}

static void host_npu_l2c_sram_test(void)
{
	unsigned int testCnt=0;
    struct device *dev=NULL;
    unsigned long *dram_unc_addr;
    dma_addr_t dram_phy_addr;
	unsigned long flags;
    int i;
    unsigned long tmpAddr, tmpWord;
    unsigned int wordLen = sizeof(unsigned long);
    unsigned int testWordNum = L2C_SRAM_CPU_TEST_SIZE/wordLen;
	unsigned long data;
    unsigned int l2cSram_pbus_phy_addr = L2C_SRAM_PBUS_BASE+L2C_SRAM_CPU_TEST_OFF;;


    if ((dev = get_gdmpSram_dev())==NULL) {
        printk("(%s) get_gdmpSram_dev is NULL\n", __func__);
        return;
    }

    /* 
     * npu_rbus_decode disable test:  
     * When npu_rbus_decode is disabled, CPU can access NPU 384K SRAM but can't access L2C_SRAM via NPU_RBUS 
     */
    enable_npu_rbus_decoder(0);

    if (test_npu_384k_sram(NPU_SRAM_CPU_TEST_OFF, NPU_SRAM_CPU_TEST_SIZE, 0)==-1) {
        printk("\n[H] ERROR: test_npu_384k_sram failed for npu_rbus_decode disable test\n");
		return;
    }

    if (l2c_sram_test(L2C_SRAM_CPU_TEST_OFF, L2C_SRAM_CPU_TEST_SIZE, 0)==0){ /* When npu_rbus_decode is disabled, the access should fail */
        printk("\n[H] ERROR: l2c_sram_test failed for npu_rbus_decode disable test\n");
		return;
    }
    else
        printk("[H] l2c_sram_test OK for npu_rbus_decode disable test\n");

    /* trigger NPU Cores to do npu_rbus_decode disable test */
    set_npu_mib_index(9, 1);
    /* wait until the test is done */
	while(get_npu_mib_index(9)!= (MAX_CORE_NUM+1))
		mdelay(10);


    /* 
     * npu_rbus_decode enable test for overnight: 
     */
    enable_npu_rbus_decoder(1);
	SET_RCU_CPU_STALL_SUPRESS();
	
	dram_unc_addr = (unsigned long *) dma_alloc_coherent(dev, L2C_SRAM_CPU_TEST_SIZE, &dram_phy_addr, GFP_KERNEL);
	if ((dram_unc_addr==NULL)) {
		printk("(%s) dma_alloc_coherent failed\n", __func__);
		return;
	}
	printk("dma_alloc_coherent addr = %lx, dram_phy_addr = %x\n", (unsigned long)dram_unc_addr,dram_phy_addr);

	/* Prepare Stress Test:
	 * During kernel will check is the CPU trapped in some process 
	 * Add lock for avoid kernel call trace
	 */
	   
	cpuTest_locking(&cpu0AccessLock, &flags);

	while(1)
	{
		if ((testCnt&0x1f)==0)
			printk("[H%x]\n", testCnt);
		if (test_npu_384k_sram(NPU_SRAM_CPU_TEST_OFF, NPU_SRAM_CPU_TEST_SIZE, testCnt)==-1) {
            printk("\n[H] ERROR: test_npu_384k_sram failed on round %x\n", testCnt);
    		goto err_handle;
        }

        if (l2c_sram_test(L2C_SRAM_CPU_TEST_OFF, L2C_SRAM_CPU_TEST_SIZE, testCnt)==-1){
            printk("\n[H] ERROR: l2c_sram_test failed on round %x\n", testCnt);
    		goto err_handle;
        }

        /* CPU writes words to DRAM */
        tmpAddr = (unsigned long)dram_unc_addr;
        tmpWord= testPatterns[testCnt%8];
        for (i=0; i<testWordNum; i++) {
            wtMeml(tmpAddr, (tmpWord+i));
            tmpAddr += wordLen;
        }
		/* configure and enable GDMA */
		SET_GDMA_CONFIG(0, dram_phy_addr, l2cSram_pbus_phy_addr, ((L2C_SRAM_CPU_TEST_SIZE&0xffff)<<16)|(1<<3)|(1<<1)|(1<<0), 0x4);
		/* wait until GDMA is done */
		while(!IS_GDMA_DONE(0));
		CLEAR_GDMA_DONE(0); /* clear done bit */
		
		if (l2c_sram_read_test(L2C_SRAM_CPU_TEST_OFF, L2C_SRAM_CPU_TEST_SIZE, tmpWord)==-1){
            printk("\n[H] ERROR: l2c_sram gdma test failed on round %x\n", testCnt);
    		goto err_handle;	
		}
		
		l2c_sram_write_test(L2C_SRAM_CPU_TEST_OFF,L2C_SRAM_CPU_TEST_SIZE, tmpWord);
		/* configure and enable GDMA */
		SET_GDMA_CONFIG(0, l2cSram_pbus_phy_addr, dram_phy_addr, ((L2C_SRAM_CPU_TEST_SIZE&0xffff)<<16)|(1<<3)|(1<<1)|(1<<0), 0x4);
		
		/* wait until GDMA is done */
		while(!IS_GDMA_DONE(0));
		CLEAR_GDMA_DONE(0); /* clear done bit */		

		tmpAddr = (unsigned long)dram_unc_addr;

		/* CPU read words from DRAM*/
		for (i=0; i<testWordNum; i++) {
			data = rdMeml(tmpAddr);
			if (data != (tmpWord+i)) {
				printk("\nERROR: data:0x%lx != tmpWord:0x%lx at word:%d\n", 
					 data, tmpWord, i);
				goto err_handle;
			}
			tmpAddr += wordLen;
		}


		
		if (testCnt==0xffffffff) testCnt=0;
        else testCnt++;
	}


err_handle:
	dma_free_coherent(dev, L2C_SRAM_CPU_TEST_SIZE, (void *)dram_unc_addr, dram_phy_addr);
 	cpuTest_unlocking(&cpu0AccessLock, &flags);   
	return;
}

static void host_rbus_test(void )
{
	int i=0;
	int j=0;
	unsigned int test_loop=0;
	unsigned long flags = 0;
    unsigned long * cached_addr=0;
	unsigned long * uncached_addr=0;
    unsigned long tmpAddr, tmpWord;
    unsigned int wordLen = sizeof(unsigned long);
    unsigned int testWordNum = HOST_DRAM_TEST_SIZE/wordLen;
	unsigned long data;
	unsigned long test_addr[2] = {0};
	

    dma_addr_t phy_addr;
	struct device *dev=NULL;

    if ((dev=get_gdmpSram_dev())==NULL) {
        printk("\nget_gdmpSram_dev failed\n");
		return;
    }
    

    cached_addr = (unsigned long *) kmalloc(HOST_DRAM_TEST_SIZE, GFP_KERNEL);
	
    if (cached_addr==NULL) {
        printk("\nERROR(%s) kmalloc fail\n",__func__);
        return;
    }
    uncached_addr = (unsigned long*) dma_alloc_coherent(dev, HOST_DRAM_TEST_SIZE, &phy_addr, GFP_KERNEL);
    if ((uncached_addr==NULL)) {
		kfree(cached_addr);
        printk("(%s) dma_alloc_coherent failed\n", __func__);
        return;
    }
	test_addr[0] = (unsigned long) cached_addr;
	test_addr[1] = (unsigned long) uncached_addr;

	printk("uncached addr = %lx, cached addr = %lx\n",test_addr[1],test_addr[0]);

	cpuTest_locking(&cpu0AccessLock, &flags);
	SET_RCU_CPU_STALL_SUPRESS();

	while(1)
	{
		/* reduce printk frequency*/
		if ((test_loop & 0xfffff) == 0)
		{
			printk("\n\n[H} %s RBUS STRESS test loop reaches %x times\n", __func__, test_loop);
		}

		for (j = 0 ; j < 2; j++)
		{
	        /* CPU writes words to DRAM */
	        tmpAddr = test_addr[j];
	        tmpWord= testPatterns[test_loop%8];
	        for (i=0; i<testWordNum; i++) {
	            wtMeml(tmpAddr, (tmpWord+i));
	            tmpAddr += wordLen;
	        }

			tmpAddr = test_addr[j];
			/* CPU read words from DRAM*/
			for (i=0; i<testWordNum; i++) {
				data = rdMeml(tmpAddr);
				if (data != (tmpWord+i)) {
					printk("\nERROR: data:0x%lx != tmpWord:0x%lx at word:%d\n", 
						 data, tmpWord, i);
					printk("\n\n[H] %s test_host_dram fail in start addr is %lx on round %x\n",__func__,test_addr[j], test_loop);
					goto err_handle;
				}
				tmpAddr += wordLen;
			}
		}
		if (test_loop == 0xffffffff) test_loop = 0;
		else test_loop++;

	}
	
err_handle:
	
	kfree(cached_addr);
	dma_free_coherent(dev, HOST_DRAM_TEST_SIZE, (void *)uncached_addr, phy_addr);
	cpuTest_unlocking(&cpu0AccessLock, &flags);
	return;

	
}

static void host_npu_cpu_access_test(void)
{
	unsigned long * uncached_addr=0;
	unsigned int npu_test_src[2] = {0};
	unsigned long test_src[2] = {0};
	unsigned long test_dst = 0;
	dma_addr_t phy_addr;
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int dram_test_size = (HOST_DRAM_TEST_SIZE>>1); //dram_test_size is half of total alloc memory
    unsigned int wordLen = sizeof(unsigned long);
    unsigned int testWordNum[2] = {(dram_test_size/wordLen),(NPU_384KSRAM_TEST_SIZE/wordLen)};
	unsigned long tmpAddr = 0;
	int npu_core = 4;
	struct device *dev=NULL;

    if ((dev=get_gdmpSram_dev())==NULL) {
        printk("\nget_gdmpSram_dev failed\n");
		return;
    }
	/* wait for NPU ready*/
	while (get_npu_mib_index(8) != 0)
		mdelay(10);

	/* alloc memory for test*/
    uncached_addr = (unsigned long*) dma_alloc_coherent(dev, HOST_DRAM_TEST_SIZE, &phy_addr, GFP_KERNEL);
    if ((uncached_addr==NULL)) {
        printk("(%s) dma_alloc_coherent failed\n", __func__);
        return;
    }	
	printk("CPU alloc dma phy addr : %x\r\n",phy_addr);
	npu_test_src[0] = (unsigned int)phy_addr;
	npu_test_src[1] = NPU_384K_SRAM_ADDR;
	test_src[0] = (unsigned long)uncached_addr;
	test_dst = test_src[j] + dram_test_size;

	for (npu_core = 0 ; npu_core < (MAX_CORE_NUM); npu_core++)
	{
		for(j = 0 ; j < 2 ; j++)
		{
			set_npu_mib_index(9, npu_test_src[j]);
			/* wait NPU prepare src data*/
			while (get_npu_mib_index(9) != 0)
				mdelay(10);

			printk("CPU start copy data\r\n");

			tmpAddr = test_src[j];
			/*copy src data to dst */
			for (i = 0 ; i < (testWordNum[j]);i++)
			{
				if (j==0)
				{
					wtMeml(tmpAddr + dram_test_size, rdMeml (tmpAddr)); 
					
				}
				else /* copy data from 0x1e800000 to 0x1e810000*/
				{
					set_npu_384k_sram_data(tmpAddr + NPU_384KSRAM_TEST_SIZE, get_npu_384k_sram_data(tmpAddr));
				}
				tmpAddr += wordLen;
			}
			if (j==0)
				printk("CPU copy DRAM data from %lx to %lx DONE\r\n",test_src[j], test_dst);
			else
				printk("CPU copy NPU SRAM data DONE\r\n");

			set_npu_mib_index(9, 1);

			/*check if the test failed*/
			while(get_npu_mib_index(9)==1)
				mdelay(10);
			if (get_npu_mib_index(9)== 0xff)
			{
				printk("CPU NPU access test Fail !\r\n");
				goto testDone;
			}

		}
	}

	printk("CPU NPU access test Pass !\r\n");

testDone:
	dma_free_coherent(dev, HOST_DRAM_TEST_SIZE, (void *)uncached_addr, phy_addr);
	return;
}

static void host_test_case(int testCase)
{
#ifndef TCSUPPORT_CPU_ARMV8 
    if (testCase==TEST_UNC_ACCESS_RAMS) {
    
        host_access_test(0);
    }
    else if (testCase==TEST_CAC_ACCESS_RAMS) {
        
        host_access_test(1);
    }
    else if (testCase==TEST_MBOX_INTR) {
#else
    if (testCase==TEST_MBOX_INTR) {
#endif        
        host_mboxIrq_test();
    }
    else if (testCase==TEST_MBOX_RW_REGS) {
        
        host_rw_mbox_regs_test();
    }
#if 0
    else if (testCase==TEST_NPU_TIMER) {
        
        host_mdelay_test();
    }
#endif
    else if (testCase==TEST_NPU_WDOG) {
		
        host_npu_wdog_test();
    }
    else if (testCase==TEST_CORE_ON_OFF) {
        
        host_npu_core_on_off_test();
		
    }
    else if (testCase==TEST_HANG_CORE_CSR) {
        
        host_hang_core_csr_test();
	}
    else if (testCase==TEST_DEAD_CORE_CSR) {
        
        host_dead_core_csr_test();
    }	
    else if ((testCase==TEST_PLIC_RESET) || (testCase==TEST_HW_KERN_RESET)) {
        
        host_npu_module_reset_test(testCase);
    }	
    else if (testCase==TEST_HW_KERN_RESET_2) {
		
        host_npu_module_reset_test(testCase);
    }
    else if (testCase==TEST_WT_DRAM_STARVING) {
		
        host_write_dram_starving_test();
    }
	else if (testCase == TEST_NPU_RW_L2C_SRAM) {
		host_npu_l2c_sram_test();
	}
	else if (testCase == TEST_NPU_RBUS_STRESS) {
		host_rbus_test();
	}
	else if (testCase == TEST_NPU_CPU_ACCESS) {
		host_npu_cpu_access_test();
	}

    return;
}

static void do_npu_all_tests(void)
{
    int i;
#if 0 //CLKNEEDFIX
    for (i=1; i<TEST_LAST_ITEM; i++) {

        /* skip overnight tests and wdog_reset test */
        if ((i==TEST_UNC_ACCESS_RAMS)||(i==TEST_CAC_ACCESS_RAMS)||(i==TEST_NPU_WDOG))
            continue;

        regWrite32(CR_NPU_MIB9, 0);

        /* issue test case "i" to NPU Cores */
        regWrite32(CR_NPU_MIB8, i);

        /* Host does its part of test */
        host_test_case(i);

        /* wait until all NPU Cores are done */
        while (regRead32(CR_NPU_MIB9)==0)
            mdelay(100);
    }
#endif
    return;
}

static int npu_test_cases_write_proc(struct file *file, const char *buffer,
					  unsigned long count, void *data)
{
	char valString[16];
        unsigned int testCases=0;
    
         
	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EFAULT;

	valString[count] = '\0';

    /* test case is 10-digit based*/
	sscanf(valString, "%d", &testCases);

    printk("\nNPU TestCase:%d\n", testCases);
	#ifndef TCSUPPORT_CPU_ARMV8 
    regWrite32(CR_NPU_MIB8, testCases);
	#else
	set_npu_test_case(testCases);
	#endif


    if (testCases==NPU_ALL_TESTS)    
        do_npu_all_tests();
    else
        host_test_case(testCases);

	return count;
}

void npu_test_proc_create(void)
{
    struct proc_dir_entry *npu_proc=NULL;

    npu_proc = create_proc_entry("npu_test_cases", 0, NULL);
    npu_proc->write_proc = npu_test_cases_write_proc;

    return;
}

void npu_test_proc_remove(void)
{
    remove_proc_entry("npu_test_cases", NULL);

    return;
}

