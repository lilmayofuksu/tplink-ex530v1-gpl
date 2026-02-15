#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <asm/io.h>
#include <asm/tc3162/tc3162.h>
#include <asm/tc3162/ledcetrl.h>
#include <linux/mtd/rt_flash.h>
#include <ecnt_hook/ecnt_hook.h>
#include <ecnt_hook/ecnt_hook_spi_nand.h>
#include "../../../../modules/private/auto_bench/751627/autobench.h"

#define SCRIPT_CMD "/usr/script/autobench_flash.sh"
#define ERASE_CHECK_FILE "/tmp/flash_erase_check"
#define WRITE_CHECK_FILE "/tmp/flash_write_check"
#define WRITE_FILE "/userfs/bin/mtd"
#define ENV_PARAM1 "HOME=/"
#define ENV_PARAM2 "TERM=vt100"
#define ENV_PARAM3 "PATH=/sbin:/usr/sbin:/bin:/usr/bin:/userfs/bin"
#define DBG_EN 0
#define dbg_printf(fmt, val...) {if(DBG_EN){printk("[DBG(%d) %s:]",__LINE__,__func__);printk(fmt, ##val);printk("\n");}}
#define TEST_SIZE 2048

u8 buf[TEST_SIZE], buf2[TEST_SIZE];

void (*flash_callusermodehelper)(int*cmdSeq, int wait);
EXPORT_SYMBOL(flash_callusermodehelper);

int flash_test(void){
	int i;
	struct file *file = NULL;
	mm_segment_t old_fs;

	
	int cmdSeq[]={FLASH_TEST,CMD_NULL};
	if(flash_callusermodehelper != NULL) {
		flash_callusermodehelper(cmdSeq, UMH_WAIT_PROC);
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	/* Erase check */
	file = filp_open(ERASE_CHECK_FILE, O_RDONLY, 0);
	if(IS_ERR(file)) {
		printk("\r\nwhen open file %s error!",ERASE_CHECK_FILE);
		return -1;
	} else {
		memset(buf, 0x00, TEST_SIZE);
		file->f_op->read(file, buf, TEST_SIZE, &file->f_pos);
		filp_close(file,NULL);
	}
	for(i = 0; i < TEST_SIZE; i++) { /* Erase check fail */
		if(buf[i] != 0xFF) {
			return -1;
		}
	}

	/* Write check */
	file = filp_open(WRITE_FILE, O_RDONLY, 0);
	if(IS_ERR(file)) {
		printk("\r\nwhen open file %s error!",WRITE_FILE);
		return -1;
	} else {
		memset(buf, 0x00, TEST_SIZE);
		file->f_op->read(file, buf, TEST_SIZE, &file->f_pos);
		filp_close(file,NULL);
	}
	file = filp_open(WRITE_CHECK_FILE, O_RDONLY, 0);
	if(IS_ERR(file)) {
		printk("\r\nwhen open file %s error!",WRITE_CHECK_FILE);
		return -1;
	} else {
		memset(buf2, 0x00, TEST_SIZE);
		file->f_op->read(file, buf2, TEST_SIZE, &file->f_pos);
		filp_close(file,NULL);
	}
	if(memcmp(buf, buf2, TEST_SIZE) != 0) { /* Write check fail */
		return -1;
	}
	
	set_fs(old_fs);
	return 0;
}

spi_nand_op_t ecnt_spi_nand_operation[] =
{
	/* autobench */
	flash_test,
};

ecnt_ret_val ecnt_spi_nand_hook(struct ecnt_data *in_data)
{
	struct ECNT_SPI_NAND_DATA *spi_nand_data = (struct ECNT_SPI_NAND_DATA *)in_data ;
	
	if(spi_nand_data->function_id >= SPI_NAND_FUNCTION_MAX_NUM) {
		printk("spi_nand_data->function_id is %d, exceed max number: %d", spi_nand_data->function_id, SPI_NAND_FUNCTION_MAX_NUM);
		return ECNT_HOOK_ERROR;
	}

	spi_nand_data->retValue = ecnt_spi_nand_operation[spi_nand_data->function_id](spi_nand_data) ;
	
	return ECNT_CONTINUE;
}

