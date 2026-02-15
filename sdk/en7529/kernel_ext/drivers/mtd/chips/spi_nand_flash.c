/***************************************************************************************
 *      Copyright(c) 2014 ECONET Incorporation All rights reserved.
 *
 *      This is unpublished proprietary source code of ECONET Networks Incorporation
 *
 *      The copyright notice above does not evidence any actual or intended
 *      publication of such source code.
 ***************************************************************************************
 */

/*======================================================================================
 * MODULE NAME: spi
 * FILE NAME: spi_nand_flash.c
 * DATE: 2014/11/21
 * VERSION: 1.00
 * PURPOSE: To Provide SPI NAND Access interface.
 * NOTES:
 *
 * AUTHOR :          REVIEWED by
 *
 * FUNCTIONS
 *
 *      SPI_NAND_Flash_Init             To provide interface for SPI NAND init. 
 *      SPI_NAND_Flash_Write_Nbyte      To provide interface for Write N Bytes into SPI NAND Flash. 
 *      SPI_NAND_Flash_Read_Byte        To provide interface for read 1 Bytes from SPI NAND Flash. 
 *      SPI_NAND_Flash_Read_DWord       To provide interface for read Double Word from SPI NAND Flash.  
 *      SPI_NAND_Flash_Read_NByte       To provide interface for Read N Bytes from SPI NAND Flash. 
 *      SPI_NAND_Flash_Erase            To provide interface for Erase SPI NAND Flash. 
 * 
 * DEPENDENCIES
 *
 * * $History: $
 * MODIFICTION HISTORY:
 * Version 1.00 - Date 2014/11/21 
 * ** This is the first versoin for creating to support the functions of
 *    current module.
 *
 *======================================================================================
 */

/* INCLUDE FILE DECLARATIONS --------------------------------------------------------- */
#include <linux/version.h>

#if defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL)
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/gen_probe.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/dma-mapping.h>
#include <linux/jiffies.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/time.h>
#include <linux/miscdevice.h>
#include <linux/scatterlist.h>
#include <linux/kthread.h>
#include <linux/math64.h>
#include <linux/random.h>

#include <asm/tc3162/tc3162.h>
#include <asm/string.h>
#include <asm/cacheflush.h>
#include <asm/uaccess.h>
#include <asm/tc3162/tc3162.h>

#if (defined(TCSUPPORT_CPU_EN7516) || defined(TCSUPPORT_CPU_EN7527)) && defined(TCSUPPORT_AUTOBENCH)	
#include <ecnt_hook/ecnt_hook.h>
#include <ecnt_hook/ecnt_hook_spi_nand.h>
#include "flash_test.h"
#endif

#include "bmt.h"
#include "ecnt_utility.h"


#if defined (TCSUPPORT_GPON_DUAL_IMAGE) || defined (TCSUPPORT_EPON_DUAL_IMAGE)
#include "flash_layout/tc_partition.h"
#endif

#define SPI_NAND_FLASH_DEBUG

#else
#include <flashhal.h>
#include <asm/tc3162.h>
#include <asm/system.h>
#endif //#if defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL)

#include <asm/io.h>
#include <linux/types.h>

#if defined(SPI_NAND_FLASH_DEBUG)
#include <stdarg.h>
#endif

#include "spi/spi_nand_flash.h"
#include "spi/spi_controller.h"
#include "spi/spi_nfi.h"
#include "spi/spi_ecc.h"

#if	defined(TCSUPPORT_NAND_BMT)
#if (!defined(LZMA_IMG) && !defined(BOOTROM_EXT)) || defined(TCSUPPORT_BB_256KB)
#include "bmt.h"
#endif
#endif

/* NAMING CONSTANT DECLARATIONS ------------------------------------------------------ */

/* SPI NAND Command Set */
#define _SPI_NAND_OP_GET_FEATURE					0x0F	/* Get Feature */
#define _SPI_NAND_OP_SET_FEATURE					0x1F	/* Set Feature */
#define _SPI_NAND_OP_PAGE_READ						0x13	/* Load page data into cache of SPI NAND chip */
#define _SPI_NAND_OP_READ_FROM_CACHE_SINGLE			0x03	/* Read data from cache of SPI NAND chip, single speed*/
#define _SPI_NAND_OP_READ_FROM_CACHE_DUAL			0x3B	/* Read data from cache of SPI NAND chip, dual speed*/
#define _SPI_NAND_OP_READ_FROM_CACHE_QUAD			0x6B	/* Read data from cache of SPI NAND chip, quad speed*/
#define _SPI_NAND_OP_WRITE_ENABLE					0x06	/* Enable write data to  SPI NAND chip */
#define _SPI_NAND_OP_WRITE_DISABLE					0x04	/* Reseting the Write Enable Latch (WEL) */
#define _SPI_NAND_OP_PROGRAM_LOAD_SINGLE			0x02	/* Write data into cache of SPI NAND chip with cache reset, single speed */
#define _SPI_NAND_OP_PROGRAM_LOAD_QUAD				0x32	/* Write data into cache of SPI NAND chip with cache reset, quad speed */
#define _SPI_NAND_OP_PROGRAM_LOAD_RAMDOM_SINGLE		0x84	/* Write data into cache of SPI NAND chip, single speed */
#define _SPI_NAND_OP_PROGRAM_LOAD_RAMDON_QUAD		0x34	/* Write data into cache of SPI NAND chip, quad speed */

#define _SPI_NAND_OP_PROGRAM_EXECUTE		0x10	/* Write data from cache into SPI NAND chip */
#define _SPI_NAND_OP_READ_ID				0x9F	/* Read Manufacture ID and Device ID */
#define _SPI_NAND_OP_BLOCK_ERASE			0xD8	/* Erase Block */
#define _SPI_NAND_OP_RESET					0xFF	/* Reset */
#define _SPI_NAND_OP_DIE_SELECT				0xC2	/* Die Select */

/* SPI NAND register address of command set */
#define _SPI_NAND_ADDR_MANUFACTURE_ID		0x00	/* Address of Manufacture ID */
#define _SPI_NAND_ADDR_DEVICE_ID			0x01	/* Address of Device ID */

/* SPI NAND value of register address of command set */
#define _SPI_NAND_VAL_DISABLE_PROTECTION	0x0		/* Value for disable write protection */
#define _SPI_NAND_VAL_ENABLE_PROTECTION		0x38	/* Value for enable write protection */
#define _SPI_NAND_VAL_OIP					0x1		/* OIP = Operaton In Progress */
#define _SPI_NAND_VAL_ERASE_FAIL			0x4		/* E_FAIL = Erase Fail */
#define _SPI_NAND_VAL_PROGRAM_FAIL			0x8		/* P_FAIL = Program Fail */

/* Others Define */
#define _SPI_NAND_LEN_ONE_BYTE				(1)
#define _SPI_NAND_LEN_TWO_BYTE				(2)
#define _SPI_NAND_LEN_THREE_BYTE			(3)
#define _SPI_NAND_BLOCK_ROW_ADDRESS_OFFSET	(6)

#define _SPI_NAND_PAGE_SIZE  				4096
#define _SPI_NAND_OOB_SIZE  				256
#define _SPI_NAND_CACHE_SIZE 				(_SPI_NAND_PAGE_SIZE+_SPI_NAND_OOB_SIZE)

#define _SPI_NFI_CHECK_ECC_DONE_MAX_TIMES	(1000000)
#define CACHE_LINE_SIZE						(32)

#ifdef BLOCK_SIZE
#undef BLOCK_SIZE
#endif

#ifdef PAGE_SIZE
#undef PAGE_SIZE
#endif

#define BLOCK_SIZE		(_current_flash_info_t.erase_size)
#define PAGE_SIZE		(_current_flash_info_t.page_size)
#define PAGE_CNT_PER_BLOCK	(BLOCK_SIZE / PAGE_SIZE)

#if defined(TCSUPPORT_CPU_EN7580)
#define _SPI_FREQUENCY_ADJUST_REG			(0xBFA201B8)
#define IOMUX_CONTROL1						(0xBFA20214)
#elif defined(TCSUPPORT_CPU_EN7516)||defined(TCSUPPORT_CPU_EN7527)
#define _SPI_FREQUENCY_ADJUST_REG			(0xBFA2011C)
#define IOMUX_CONTROL1						(0xBFA2015C)
#else
#define _SPI_FREQUENCY_ADJUST_REG			(0xBFA200CC)
#define IOMUX_CONTROL1						(0xBFA20104)
#endif

#define PAGE_SIZE_MAGIC_FLASH_ADDR			(0XFFB0)
#define PAGE_SIZE_MAGIC						(0x50414745)

#define LINUX_USE_OOB_START_OFFSET		(4)
#define MAX_LINUX_USE_OOB_SIZE			(26)
#define MAX_USE_OOB_SIZE				(LINUX_USE_OOB_START_OFFSET + MAX_LINUX_USE_OOB_SIZE + 2)

#define MIN(a,b)			((a) < (b) ? (a) : (b))

#define K0_TO_K1(x)			(((uint32)x) | 0xa0000000)
#define K1_TO_PHY(x)		(((uint32)x) & 0x1fffffff)

#if defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL)
static DEFINE_SPINLOCK(spinandLock);

#define _SPI_NAND_SEMAPHORE_LOCK()			spin_lock_irqsave(&spinandLock, spinand_spinlock_flags)	/* Disable interrupt */
#define _SPI_NAND_SEMAPHORE_UNLOCK()		spin_unlock_irqrestore(&spinandLock, spinand_spinlock_flags)	/* Enable interrupt  */

struct spi_chip_info {
	struct spi_flash_info *flash;
	void (*destroy)(struct spi_chip_info *chip_info);

	u32 (*read)(struct map_info *map, u32 from, u32 to, u32 size);
	u32 (*read_manual)(struct mtd_info *mtd, unsigned long from, unsigned char *buf, unsigned long len);
	u32 (*write)(struct mtd_info *mtd, u32 from, u32 to, u32 size);
	u32 (*erase)(struct mtd_info *mtd, u32 addr);
};

extern unsigned char (*ranand_read_byte)(unsigned long long, SPI_NAND_FLASH_RTN_T *status);
extern unsigned long (*ranand_read_dword)(unsigned long long, SPI_NAND_FLASH_RTN_T *status);

struct mtd_info 	*spi_nand_mtd;

struct _SPI_NAND_FLASH_RW_TEST_T {
	u32 times;
	u32 block_idx;
};

static struct _SPI_NAND_FLASH_RW_TEST_T rw_test_param;
static int _SPI_NAND_WRITE_FAIL_TEST_FLAG = 0;
static int _SPI_NAND_ERASE_FAIL_TEST_FLAG = 0;

#define MAX_BLOCK (2048)

//#define ERASE_WRITE_CNT_LOG

#ifdef ERASE_WRITE_CNT_LOG
typedef enum{
	SPI_NAND_FLASH_ERASE_WRITE_LOG_DISABLE = 0,
	SPI_NAND_FLASH_ERASE_WRITE_LOG_ENABLE,
} ERASE_WRITE_LOG_T;

ERASE_WRITE_LOG_T erase_write_flag;
unsigned long b_erase_cnt[MAX_BLOCK];
unsigned long b_write_total_cnt[MAX_BLOCK];
unsigned long b_write_cnt_per_erase[MAX_BLOCK];
#endif

#define UBIFS_BLANK_PAGE_FIXUP

#ifdef UBIFS_BLANK_PAGE_FIXUP
typedef enum{
	SPI_NAND_FLASH_UBIFS_BLANK_PAGE_ECC_MATCH = 0,
	SPI_NAND_FLASH_UBIFS_BLANK_PAGE_ECC_MISMATCH,
} UBIFS_BLANK_PAGE_ECC_T;

typedef enum{
	SPI_NAND_FLASH_UBIFS_BLANK_PAGE_FIXUP_SUCCESS = 0,
	SPI_NAND_FLASH_UBIFS_BLANK_PAGE_FIXUP_FAIL,
} UBIFS_BLANK_PAGE_FIXUP_T;

#endif

#else
extern void * memcpy4(void * dest, const void *src, size_t count);
#define memcpy	memcpy4

#define _SPI_NAND_SEMAPHORE_LOCK()
#define _SPI_NAND_SEMAPHORE_UNLOCK()

unsigned int print_dot = 0;
int test_write_fail_flag = 0;
int en_oob_write = 0;
#endif

/* FUNCTION DECLARATIONS ------------------------------------------------------ */
extern void * memset(void * s, int c, size_t count);
SPI_NAND_FLASH_RTN_T spi_nand_erase_block(u32 block_index);
static SPI_NAND_FLASH_RTN_T spi_nand_write_page(u32 page_number, 
												u32 data_offset,
											  	const u8  *ptr_data, 
											  	u32 data_len, 
											  	u32 oob_offset,
												u8  *ptr_oob , 
												u32 oob_len,
											    SPI_NAND_FLASH_WRITE_SPEED_MODE_T speed_mode);


/* MACRO DECLARATIONS ---------------------------------------------------------------- */
#define _SPI_NAND_BLOCK_ALIGNED_CHECK( __addr__,__block_size__) ((__addr__) & ( __block_size__ - 1))
#define _SPI_NAND_GET_DEVICE_INFO_PTR		&(_current_flash_info_t)

/* Porting Replacement */
#if !defined(SPI_NAND_FLASH_DEBUG)
	#define _SPI_NAND_PRINTF(args...)
	#define _SPI_NAND_DEBUG_PRINTF(args...)
	#define _SPI_NAND_DEBUG_PRINTF_HEX(args...)
	#define _SPI_NAND_DEBUG_PRINTF_ARRAY(args...)
#else
	#ifdef SPRAM_IMG
		#define _SPI_NAND_PRINTF(fmt, args...)		prom_puts(fmt)		/* Always print information */
	#else
		#if defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL)
		#define _SPI_NAND_PRINTF					printk
		#else
		#define _SPI_NAND_PRINTF					prom_printf			/* Always print information */
		#endif
	#endif
	#if !(defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL))
		#define _SPI_NAND_DEBUG_PRINTF_HEX			spi_nand_flash_debug_printf_hex
	#endif
	#define _SPI_NAND_DEBUG_PRINTF				spi_nand_flash_debug_printf
	#define _SPI_NAND_DEBUG_PRINTF_ARRAY		spi_nand_flash_debug_printf_array
#endif
#define _SPI_NAND_ENABLE_MANUAL_MODE		SPI_CONTROLLER_Enable_Manual_Mode
#define _SPI_NAND_WRITE_ONE_BYTE			SPI_CONTROLLER_Write_One_Byte
#define _SPI_NAND_WRITE_ONE_BYTE_WITH_CMD	SPI_CONTROLLER_Write_One_Byte_With_Cmd
#define _SPI_NAND_WRITE_NBYTE				SPI_CONTROLLER_Write_NByte
#define _SPI_NAND_READ_NBYTE				SPI_CONTROLLER_Read_NByte
#define _SPI_NAND_READ_CHIP_SELECT_HIGH		SPI_CONTROLLER_Chip_Select_High
#define _SPI_NAND_READ_CHIP_SELECT_LOW		SPI_CONTROLLER_Chip_Select_Low

#if !defined(BOOTROM_EXT) || defined(TCSUPPORT_BB_256KB)
struct ra_nand_chip ra;
struct nand_info flashInfo;
flashdev_info devinfo;
#endif

u8 *dma_read_page = NULL;
#if	!defined(LZMA_IMG) || defined(TCSUPPORT_BB_256KB) 
u8 *dma_write_page = NULL;
#endif

#if defined(BOOTROM_EXT)
u8 *tmp_dma_read_page = 0x80000000; //9KB
u8 *tmp_dma_write_page = 0x80002400; //9KB
u8 *_current_cache_page = 0x80004800; //9KB
u8 *_current_cache_page_data = 0x80006C00; //8KB
u8 *_current_cache_page_oob = 0x80008C00; //1KB
u8 *_current_cache_page_oob_mapping = 0x80009000; //1KB
#else
u8 	tmp_dma_read_page[_SPI_NAND_CACHE_SIZE + CACHE_LINE_SIZE];
#if	!defined(LZMA_IMG) || defined(TCSUPPORT_BB_256KB)
u8	tmp_dma_write_page[_SPI_NAND_CACHE_SIZE + CACHE_LINE_SIZE];
#endif
u8	_current_cache_page[_SPI_NAND_CACHE_SIZE];
u8	_current_cache_page_data[_SPI_NAND_PAGE_SIZE];
u8	_current_cache_page_oob[_SPI_NAND_OOB_SIZE];
u8	_current_cache_page_oob_mapping[_SPI_NAND_OOB_SIZE];
#endif

/* TYPE DECLARATIONS ----------------------------------------------------------------- */

/* STATIC VARIABLE DECLARATIONS ------------------------------------------------------ */
#if	defined(TCSUPPORT_NAND_BMT) && (!defined(LZMA_IMG) || defined(TCSUPPORT_BB_256KB))

#if defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL)
extern int nand_logic_size;
#endif

#if !defined(BOOTROM_EXT) || defined(TCSUPPORT_BB_256KB)
static int bmt_pool_size = 0;
static bmt_struct *g_bmt = NULL;
static init_bbt_struct *g_bbt = NULL;
extern int nand_flash_avalable_size;
u32 maximum_bmt_block_count=0;

#define BMT_BAD_BLOCK_INDEX_OFFSET (1)
#define POOL_GOOD_BLOCK_PERCENT 8/100
#define MAX_BMT_SIZE_PERCENTAGE 1/10

#if defined(TCSUPPORT_CT_PON)
#define MAX_BMT_SIZE_PERCENTAGE_CT 1/8
#endif

#endif
#endif

#if defined(SPI_NAND_FLASH_DEBUG)
SPI_NAND_FLASH_DEBUG_LEVEL_T  _SPI_NAND_DEBUG_LEVEL = SPI_NAND_FLASH_DEBUG_LEVEL_0;
#endif

u32	_current_page_num	= 0xFFFFFFFF;
u32 reservearea_size = 0;
unsigned char _ondie_ecc_flag=1;    /* Ondie ECC : [ToDo :  Init this flag base on diffrent chip ?] */
unsigned char _spi_dma_mode=0;
unsigned char _plane_select_bit=0;
unsigned char _die_id = 0;

struct SPI_NAND_FLASH_INFO_T	_current_flash_info_t;			/* Store the current flash information */

/* LOCAL SUBPROGRAM BODIES------------------------------------------------------------ */
#if !(defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL))
static int generic_ffs(int x)
{
	int r = 1;

	if (!x)
		return 0;
	if (!(x & 0xffff)) {
		x >>= 16;
		r += 16;
	}
	if (!(x & 0xff)) {
		x >>= 8;
		r += 8;
	}
	if (!(x & 0xf)) {
		x >>= 4;
		r += 4;
	}
	if (!(x & 3)) {
		x >>= 2;
		r += 2;
	}
	if (!(x & 1)) {
		x >>= 1;
		r += 1;
	}
	return r;
}
#endif

#if defined(SPI_NAND_FLASH_DEBUG)
#if !(defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL))
SPI_NAND_FLASH_DEBUG_LEVEL_T get_debug_level(void)
{
	return _SPI_NAND_DEBUG_LEVEL;
}

void set_debug_level(SPI_NAND_FLASH_DEBUG_LEVEL_T DEBUG_LEVEL)
{
	_SPI_NAND_DEBUG_LEVEL = DEBUG_LEVEL;
}
#endif

static void spi_nand_flash_debug_printf( SPI_NAND_FLASH_DEBUG_LEVEL_T DEBUG_LEVEL, char *fmt, ... )
{
	if( _SPI_NAND_DEBUG_LEVEL >= DEBUG_LEVEL )
	{
#ifdef SPRAM_IMG
		_SPI_NAND_PRINTF(fmt);
#else
		unsigned char 		str_buf[100];	
		va_list 			argptr;
		int 				cnt;		
	
		va_start(argptr, fmt);
		cnt = vsprintf(str_buf, fmt, argptr);
		va_end(argptr);
				
		_SPI_NAND_PRINTF("%s", str_buf);
#endif
	}
}

static void spi_nand_flash_debug_printf_array(SPI_NAND_FLASH_DEBUG_LEVEL_T DEBUG_LEVEL, const char *buf, u32 len)
{
#if defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL)
	u32	idx_for_debug;
	
	if(_SPI_NAND_DEBUG_LEVEL >= DEBUG_LEVEL ) {
		for(idx_for_debug=0; idx_for_debug< len; idx_for_debug++) {				
			if((idx_for_debug % 8) == 0) {
				if((idx_for_debug % 16) == 0) {
					_SPI_NAND_PRINTF("\n%04x:", (idx_for_debug));
				} else {
					_SPI_NAND_PRINTF(": ");
				}
			}
			_SPI_NAND_PRINTF("%02x ", (unsigned char)buf[idx_for_debug]);
		}			
		_SPI_NAND_PRINTF("\n");		
	}
#else
	if( _SPI_NAND_DEBUG_LEVEL >= DEBUG_LEVEL ) {
		show_array(buf, len, "SPI NAND Flash array");	
	}
#endif
}

#if !(defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL))
static void spi_nand_flash_debug_printf_hex(SPI_NAND_FLASH_DEBUG_LEVEL_T DEBUG_LEVEL, unsigned long val, int len)
{
	if( _SPI_NAND_DEBUG_LEVEL >= DEBUG_LEVEL )
	{
#ifdef SPRAM_IMG
		prom_print_hex(val, len);
#else
		_SPI_NAND_PRINTF("%x", val);
#endif
	}
}
#endif
#endif

static SPI_NAND_FLASH_RTN_T spi_nand_protocol_reset( void )
{
	SPI_NAND_FLASH_RTN_T	rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	
	/* 1. Chip Select low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();

	/* 2. Send FFh opcode (Reset) */
	_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_SET_FEATURE );

	/* 5. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();	

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_protocol_reset\n");
	
	return (rtn_status);	
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_set_feature( u8 addr, u8 protection )
 * PURPOSE : To implement the SPI nand protocol for set status register.
 * AUTHOR  : 
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : addr - register address
 *			 data - The variable of this register.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2017/5/26.
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_protocol_set_feature( u8 addr, u8 data )
{
	SPI_NAND_FLASH_RTN_T	rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	
	/* 1. Chip Select low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();

	/* 2. Send 0Fh opcode (Set Feature) */
	_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_SET_FEATURE );
	
	/* 3. Offset addr */
	_SPI_NAND_WRITE_ONE_BYTE( addr );
	
	/* 4. Write new setting */
	_SPI_NAND_WRITE_ONE_BYTE( data );

	/* 5. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();	

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_protocol_set_feature %x: val=0x%x\n", addr, data);
	
	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_get_feature( u8 addr, u8 *ptr_rtn_data )
 * PURPOSE : To implement the SPI nand protocol for get status register.
 * AUTHOR  :
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : addr - register address
 *   OUTPUT: ptr_rtn_protection  - A pointer to the ptr_rtn_protection variable.
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2017/5/26.
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_protocol_get_feature( u8 addr, u8 *ptr_rtn_data )
{
	SPI_NAND_FLASH_RTN_T	rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	
	/* 1. Chip Select low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();

	/* 2. Send 0Fh opcode (Get Feature) */
	_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_GET_FEATURE );
	
	/* 3. Offset addr */
	_SPI_NAND_WRITE_ONE_BYTE_WITH_CMD(_SPI_CONTROLLER_VAL_OP_OS2ID, addr );
	
	/* 4. Read 1 byte data */
	_SPI_NAND_READ_NBYTE( ptr_rtn_data, _SPI_NAND_LEN_ONE_BYTE, SPI_CONTROLLER_SPEED_SINGLE);

	/* 5. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();	

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_protocol_get_feature %x: val=0x%x\n", addr, *ptr_rtn_data);
	
	return (rtn_status);
}

#if !defined(BOOTROM_EXT)
/*------------------------------------------------------------------------------------
 * FUNCTION: SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Get_Flash_Info( struct SPI_NAND_FLASH_INFO_T    *ptr_rtn_into_t )
 * PURPOSE : To get system current flash info.
 * AUTHOR  : 
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : None
 *   OUTPUT: ptr_rtn_into_t  - A pointer to the structure of the ptr_rtn_into_t variable.
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2015/01/14  - The first revision for this function.
 *
 *------------------------------------------------------------------------------------
 */
SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Get_Flash_Info(struct SPI_NAND_FLASH_INFO_T *ptr_rtn_into_t)
{
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T			rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;
	
	memcpy( ptr_rtn_into_t, ptr_dev_info_t, sizeof(struct SPI_NAND_FLASH_INFO_T));
	
	return (rtn_status);	
}

SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Set_Flash_Info( struct SPI_NAND_FLASH_INFO_T *ptr_rtn_into_t)
{
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T			rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;
	
	memcpy(ptr_dev_info_t, ptr_rtn_into_t, sizeof(struct SPI_NAND_FLASH_INFO_T) );
	
	return (rtn_status);	
}
#endif

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_read_id( struct SPI_NAND_FLASH_INFO_T *ptr_rtn_flash_id )
 * PURPOSE : To implement the SPI nand protocol for read id.
 * AUTHOR  : 
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : None
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2014/12/12  - The first revision for this function.
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_protocol_read_id ( struct SPI_NAND_FLASH_INFO_T *ptr_rtn_flash_id )
{
	SPI_NAND_FLASH_RTN_T	rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;	
	
	/* 1. Chip Select Low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();
	
	/* 2. Write op_cmd 0x9F (Read ID) */
	_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_READ_ID );	
	
	/* 3. Write Address Byte (0x00) */
	_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_ADDR_MANUFACTURE_ID );	
	
	/* 4. Read data (Manufacture ID and Device ID) */	
	_SPI_NAND_READ_NBYTE( (u8 *)&(ptr_rtn_flash_id->mfr_id), _SPI_NAND_LEN_ONE_BYTE, SPI_CONTROLLER_SPEED_SINGLE);	
	_SPI_NAND_READ_NBYTE( (u8 *)&(ptr_rtn_flash_id->dev_id), _SPI_NAND_LEN_ONE_BYTE, SPI_CONTROLLER_SPEED_SINGLE);
	
	/* 5. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_protocol_read_id : mfr_id=0x%x, dev_id=0x%x\n", ptr_rtn_flash_id->mfr_id, ptr_rtn_flash_id->dev_id);
	
	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_read_id_2( struct SPI_NAND_FLASH_INFO_T *ptr_rtn_flash_id )
 * PURPOSE : To implement the SPI nand protocol for read id.
 * AUTHOR  : 
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : None
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2014/12/12  - The first revision for this function.
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_protocol_read_id_2 ( struct SPI_NAND_FLASH_INFO_T *ptr_rtn_flash_id )
{
	SPI_NAND_FLASH_RTN_T	rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;	
	
	/* 1. Chip Select Low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();
	
	/* 2. Write op_cmd 0x9F (Read ID) */
	_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_READ_ID );	
	
	/* 3. Read data (Manufacture ID and Device ID) */	
	_SPI_NAND_READ_NBYTE( (u8 *)&(ptr_rtn_flash_id->mfr_id), _SPI_NAND_LEN_ONE_BYTE, SPI_CONTROLLER_SPEED_SINGLE);	
	_SPI_NAND_READ_NBYTE( (u8 *)&(ptr_rtn_flash_id->dev_id), _SPI_NAND_LEN_ONE_BYTE, SPI_CONTROLLER_SPEED_SINGLE);
	
	/* 4. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_protocol_read_id_2 : mfr_id=0x%x, dev_id=0x%x\n", ptr_rtn_flash_id->mfr_id, ptr_rtn_flash_id->dev_id);
	
	return (rtn_status);
}

static SPI_NAND_FLASH_RTN_T spi_nand_protocol_die_select_1( u8 die_id)
{
	SPI_NAND_FLASH_RTN_T	rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	/* 1. Chip Select low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();

	/* 2. Send C2h opcode (Die Select) */
	_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_DIE_SELECT );

	/* 3. Send Die ID */
	_SPI_NAND_WRITE_ONE_BYTE( die_id );

	/* 5. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_protocol_die_select_1\n");

	return (rtn_status);
}

static SPI_NAND_FLASH_RTN_T spi_nand_protocol_die_select_2( u8 die_id)
{
	SPI_NAND_FLASH_RTN_T	rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	u8 feature;

	rtn_status = spi_nand_protocol_get_feature(_SPI_NAND_ADDR_FEATURE_4, &feature);
	if(rtn_status != SPI_NAND_FLASH_RTN_NO_ERROR) {
		_SPI_NAND_PRINTF("spi_nand_protocol_die_select_2 get die select fail.\n");
		return (rtn_status);
	}
	
	if(die_id == 0) {
		feature &= ~(0x40);
	} else {
		feature |= 0x40;
	}
	rtn_status = spi_nand_protocol_set_feature(_SPI_NAND_ADDR_FEATURE_4, feature);

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_protocol_die_select_2\n");

	return (rtn_status);
}

static void spi_nand_select_die (u32 page_number)
{
	struct SPI_NAND_FLASH_INFO_T *ptr_dev_info_t;
	u8 die_id = 0;

	ptr_dev_info_t = _SPI_NAND_GET_DEVICE_INFO_PTR;

	if(ptr_dev_info_t->feature & SPI_NAND_FLASH_DIE_SELECT_1_HAVE) {
		/* single die = 1024blocks * 64pages */
		die_id = ((page_number >> 16) & 0xff);

		if (_die_id != die_id) {
			_die_id = die_id;
			spi_nand_protocol_die_select_1(die_id);

			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spi_nand_protocol_die_select_1: die_id=0x%x\n", die_id);
		}
	} else if(ptr_dev_info_t->feature & SPI_NAND_FLASH_DIE_SELECT_2_HAVE) {
		/* single die = 2plans * 1024blocks * 64pages */
		die_id = ((page_number >> 17) & 0xff);

		if (_die_id != die_id) {
			_die_id = die_id;
			spi_nand_protocol_die_select_2(die_id);

			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spi_nand_protocol_die_select_2: die_id=0x%x\n", die_id);
		}
	}
}

static void spi_nand_direct_select_die(u32 die)
{
	struct SPI_NAND_FLASH_INFO_T *ptr_dev_info_t;

	ptr_dev_info_t = _SPI_NAND_GET_DEVICE_INFO_PTR;

	if(ptr_dev_info_t->feature & SPI_NAND_FLASH_DIE_SELECT_1_HAVE) {
		if (_die_id != die) {
			_die_id = die;
			spi_nand_protocol_die_select_1(die);

			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spi_nand_protocol_die_select_1: die_id=0x%x\n", die);
		}
	} else if(ptr_dev_info_t->feature & SPI_NAND_FLASH_DIE_SELECT_2_HAVE) {
		if (_die_id != die) {
			_die_id = die;
			spi_nand_protocol_die_select_2(die);

			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spi_nand_protocol_die_select_2: die_id=0x%x\n", die);
		}
	}
}

/*------------------------------------------------------------------------------------
 * FUNCTION: spi_nand_set_clock_speed( u32 clk)
 * PURPOSE : To set SPI clock. 
 *                 clock_factor = 0
 *                   EN7512:
 *                     SPI clock = 500MHz / 40 = 12.5MHz
 *                   EN7526FC, EN7516:
 *                     SPI clock = 400MHz / 40 = 10MHz
 *                 clock_factor > 0
 *                   EN7512:
 *                     SPI clock = 500MHz / (clock_factor * 2)
 *                   EN7526FC, EN7516:
 *                     SPI clock = 400MHz / (clock_factor * 2)
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : clock_factor - The SPI clock divider.
 * RETURN  : NONE.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
void spi_nand_set_clock_speed(u32 clk)
{
	u32 val;
	u32 dividend;
	u32 clock_factor;

	if(!isFPGA) {
		if(isEN7526c || isEN751627|| isEN7580) {
			dividend = 400;
		} else {
			dividend = 500;
		}

		clock_factor = (dividend / (clk * 2));
		
		val  = VPint(_SPI_FREQUENCY_ADJUST_REG);
		val &= 0xffff0000;
		VPint(_SPI_FREQUENCY_ADJUST_REG) = val;
		
		val |= (((clock_factor) << 8)|1);
		VPint(_SPI_FREQUENCY_ADJUST_REG) = val;
		_SPI_NAND_PRINTF("Set SPI Clock to %u Mhz\n", (dividend / (clock_factor * 2)));
	} else {
		_SPI_NAND_PRINTF("FPGA can not Set SPI Clock, FPGA SPI Clock is fixed to 25 Mhz\n");
	}
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_block_aligned_check( u32   addr,
 *                                                                     u32   len  )
 * PURPOSE : To check block align.
 * AUTHOR  : 
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : addr - The addr variable of this function.
 *           len  - The len variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2014/12/15  - The first revision for this function.
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_block_aligned_check( u32 addr, 
														  u32 len )
{
	struct SPI_NAND_FLASH_INFO_T		*ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T				rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;	
	
	
	ptr_dev_info_t = _SPI_NAND_GET_DEVICE_INFO_PTR ;
	
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "SPI_NAND_BLOCK_ALIGNED_CHECK_check: addr=0x%x, len=0x%x, block size =0x%x \n", addr, len, (ptr_dev_info_t->erase_size));	

	if (_SPI_NAND_BLOCK_ALIGNED_CHECK(len, (ptr_dev_info_t->erase_size))) 
	{
		len = ( (len/ptr_dev_info_t->erase_size) + 1) * (ptr_dev_info_t->erase_size);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "SPI_NAND_BLOCK_ALIGNED_CHECK_check: erase block aligned first check OK, addr:%x len:%x\n", addr, len, (ptr_dev_info_t->erase_size));
	}

	if (_SPI_NAND_BLOCK_ALIGNED_CHECK(addr, (ptr_dev_info_t->erase_size)) || _SPI_NAND_BLOCK_ALIGNED_CHECK(len, (ptr_dev_info_t->erase_size)) ) 
	{
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "SPI_NAND_BLOCK_ALIGNED_CHECK_check: erase block not aligned, addr:0x%x len:0x%x, blocksize:0x%x\n", addr, len, (ptr_dev_info_t->erase_size));		
		rtn_status = SPI_NAND_FLASH_RTN_ALIGNED_CHECK_FAIL;
	}	
	
	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: void SPI_NAND_Flash_Clear_Read_Cache_Data( void )
 * PURPOSE : To clear the cache data for read. 
 *           (The next time to read data will get data from flash chip certainly.)
 * AUTHOR  : 
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : None
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2015/01/21  - The first revision for this function.
 *
 *------------------------------------------------------------------------------------
 */
void SPI_NAND_Flash_Clear_Read_Cache_Data( void )
{
	_current_page_num	= 0xFFFFFFFF;
}

void SPI_NAND_Flash_Set_DmaMode( u32 input )
{
	_spi_dma_mode = input;
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "SPI_NAND_Flash_Set_DmaMode : dma_mode =%d\n", _spi_dma_mode);
}

void SPI_NAND_Flash_Get_DmaMode( u32 *val )
{
	*val = _spi_dma_mode;
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "SPI_NAND_Flash_Get_DmaMode : dma_mode =%d\n", _spi_dma_mode);
}

SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Enable_OnDie_ECC( void )
{
	unsigned char feature;
	struct SPI_NAND_FLASH_INFO_T *ptr_dev_info_t;
	u8 die_num;
	u32 die_page_num;

	ptr_dev_info_t = _SPI_NAND_GET_DEVICE_INFO_PTR;

	if(((ptr_dev_info_t->feature) & SPI_NAND_FLASH_DIE_SELECT_1_HAVE) ||
	   ((ptr_dev_info_t->feature) & SPI_NAND_FLASH_DIE_SELECT_2_HAVE)) {
		for(die_num = 0; die_num < ptr_dev_info_t->die_num; die_num++) {
			die_page_num = die_num * (ptr_dev_info_t->device_size / ptr_dev_info_t->die_num) / ptr_dev_info_t->page_size;
			spi_nand_select_die(die_page_num);

			spi_nand_protocol_get_feature(ptr_dev_info_t->ecc_en.ecc_en_addr, &feature);
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "before setting : SPI_NAND_Flash_Enable_OnDie_ECC, status reg=0x%x\n", feature);	
			
			if((feature & ptr_dev_info_t->ecc_en.ecc_en_mask) != ptr_dev_info_t->ecc_en.ecc_en_value) {
				feature = (feature & ~(ptr_dev_info_t->ecc_en.ecc_en_mask));
				feature |= (ptr_dev_info_t->ecc_en.ecc_en_value & ptr_dev_info_t->ecc_en.ecc_en_mask);

				spi_nand_protocol_set_feature(ptr_dev_info_t->ecc_en.ecc_en_addr, feature);
			
				/* Value check*/
				spi_nand_protocol_get_feature(ptr_dev_info_t->ecc_en.ecc_en_addr, &feature);
			}
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "after setting : SPI_NAND_Flash_Enable_OnDie_ECC, status reg=0x%x\n", feature);	
		}
	} else {
		spi_nand_protocol_get_feature(ptr_dev_info_t->ecc_en.ecc_en_addr, &feature);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "before setting : SPI_NAND_Flash_Enable_OnDie_ECC, status reg=0x%x\n", feature);	
		
		if((feature & ptr_dev_info_t->ecc_en.ecc_en_mask) != ptr_dev_info_t->ecc_en.ecc_en_value) {
			feature = (feature & ~(ptr_dev_info_t->ecc_en.ecc_en_mask));
			feature |= (ptr_dev_info_t->ecc_en.ecc_en_value & ptr_dev_info_t->ecc_en.ecc_en_mask);
			
			spi_nand_protocol_set_feature(ptr_dev_info_t->ecc_en.ecc_en_addr, feature);

			/* Value check*/
			spi_nand_protocol_get_feature(ptr_dev_info_t->ecc_en.ecc_en_addr, &feature);
		}
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "after setting : SPI_NAND_Flash_Enable_OnDie_ECC, status reg=0x%x\n", feature);	
	}

	_ondie_ecc_flag = 1;

	return (SPI_NAND_FLASH_RTN_NO_ERROR);
}

SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Disable_OnDie_ECC( void )
{
	unsigned char feature;
	struct SPI_NAND_FLASH_INFO_T *ptr_dev_info_t;
	u8 die_num;
	u32 die_page_num;

	ptr_dev_info_t = _SPI_NAND_GET_DEVICE_INFO_PTR;

	if(((ptr_dev_info_t->feature) & SPI_NAND_FLASH_DIE_SELECT_1_HAVE) ||
	   ((ptr_dev_info_t->feature) & SPI_NAND_FLASH_DIE_SELECT_2_HAVE)) {
		for(die_num = 0; die_num < ptr_dev_info_t->die_num; die_num++) {
			die_page_num = die_num * (ptr_dev_info_t->device_size / ptr_dev_info_t->die_num) / ptr_dev_info_t->page_size;
			spi_nand_select_die(die_page_num);

			spi_nand_protocol_get_feature(ptr_dev_info_t->ecc_en.ecc_en_addr, &feature);

			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "before setting : SPI_NAND_Flash_Disable_OnDie_ECC, status reg=0x%x\n", feature);	

			if((feature & ptr_dev_info_t->ecc_en.ecc_en_mask) == ptr_dev_info_t->ecc_en.ecc_en_value) {
				feature = (feature & ~(ptr_dev_info_t->ecc_en.ecc_en_mask));
				feature |= ((~(ptr_dev_info_t->ecc_en.ecc_en_value)) & ptr_dev_info_t->ecc_en.ecc_en_mask);
			
				spi_nand_protocol_set_feature(ptr_dev_info_t->ecc_en.ecc_en_addr, feature);

				/* Value check*/
				spi_nand_protocol_get_feature(ptr_dev_info_t->ecc_en.ecc_en_addr, &feature);
			}
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "after setting : SPI_NAND_Flash_Disable_OnDie_ECC, status reg=0x%x\n", feature);
		}
	} else {
		spi_nand_protocol_get_feature(ptr_dev_info_t->ecc_en.ecc_en_addr, &feature);

		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "before setting : SPI_NAND_Flash_Disable_OnDie_ECC, status reg=0x%x\n", feature);	

		if((feature & ptr_dev_info_t->ecc_en.ecc_en_mask) == ptr_dev_info_t->ecc_en.ecc_en_value) {
			feature = (feature & ~(ptr_dev_info_t->ecc_en.ecc_en_mask));
			feature |= ((~(ptr_dev_info_t->ecc_en.ecc_en_value)) & ptr_dev_info_t->ecc_en.ecc_en_mask);
			
			spi_nand_protocol_set_feature(ptr_dev_info_t->ecc_en.ecc_en_addr, feature);

			/* Value check*/
			spi_nand_protocol_get_feature(ptr_dev_info_t->ecc_en.ecc_en_addr, &feature);
		}
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "after setting : SPI_NAND_Flash_Disable_OnDie_ECC, status reg=0x%x\n", feature);
	}

	_ondie_ecc_flag = 0;

	return (SPI_NAND_FLASH_RTN_NO_ERROR);
}

void set_spi_quad_mode_shared_pin(void)
{
	if(!isFPGA) {
		if(isEN7526c) {
			VPint(IOMUX_CONTROL1) |= (1 << 19);
			VPint(IOMUX_CONTROL1) &= ~((1 << 18) | (1 << 11) | (1 << 8) | (1 << 7) | (1 << 3));
		} else if(isEN751627) {
			VPint(IOMUX_CONTROL1) |= (1 << 27);
			VPint(IOMUX_CONTROL1) &= ~((1 << 24) | (1 << 17) | (1 << 14));
		} else if(isEN7580) {
			VPint(IOMUX_CONTROL1) |= (1 << 4);
		} else {
			_SPI_NAND_PRINTF("can not config share pin.\n");
			while(1);
		}
	}
}

void enable_quad(void)
{
	unsigned char					feature;
	unsigned char					die;
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	
	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1,"SPI NAND Chip Init : Unlock all block and Enable Quad Mode\n"); 

	for(die = 0; die < ptr_dev_info_t->die_num; die++) {
		spi_nand_direct_select_die(die);

		if(((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_WINBOND) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_W25N01GV)) ||
		   ((ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_WINBOND) && (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_W25M02GV)))
		{
			/* Enable to modify the status regsiter 1 */
			spi_nand_protocol_get_feature(_SPI_NAND_ADDR_FEATURE, &feature);
			feature |= 0x8;
			spi_nand_protocol_set_feature(_SPI_NAND_ADDR_FEATURE, feature);
		}

		/* Enable Qual mode */
		if(ptr_dev_info_t->quad_en.quad_en_mask != 0x00) {
			spi_nand_protocol_get_feature(_SPI_NAND_ADDR_FEATURE, &feature);
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "Before enable qual mode setup, the status register2 =0x%x\n", feature);

			feature = (feature & ~(ptr_dev_info_t->quad_en.quad_en_mask));
			feature |= ptr_dev_info_t->quad_en.quad_en_value;
			spi_nand_protocol_set_feature(_SPI_NAND_ADDR_FEATURE, feature);
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "enable qual mode setup, the feature = 0x%x\n", feature);

			spi_nand_protocol_get_feature(_SPI_NAND_ADDR_FEATURE, &feature);
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "After enable qual mode setup, the status register2 =0x%x\n", feature);
		} else {
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "No need enable qual mode setup.\n");
		}
	}

	/* modify share pin for quad mode */
	set_spi_quad_mode_shared_pin();
	
	_SPI_NAND_PRINTF("enable SPI Quad mode\n");
}

#if READ_AREA
#endif

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_page_read( u32    page_number )
 * PURPOSE : To implement the SPI nand protocol for page read.
 * AUTHOR  : 
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : page_number - The page_number variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2014/12/17  - The first revision for this function.
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_protocol_page_read ( u32 page_number )
{
	SPI_NAND_FLASH_RTN_T	rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	
	/* 1. Chip Select low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();
	
	/* 2. Send 13h opcode */
	_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_PAGE_READ );
	
	/* 3. Send page number */
	_SPI_NAND_WRITE_ONE_BYTE( ((page_number >> 16 ) & 0xff) );
	_SPI_NAND_WRITE_ONE_BYTE( ((page_number >> 8  ) & 0xff) );
	_SPI_NAND_WRITE_ONE_BYTE( ((page_number)        & 0xff) );
	
	/* 4. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();	
	
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_load_page_into_cache: value=0x%x\n", page_number);	
	
	return (rtn_status);
	
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_read_from_cache( u32  data_offset,
 *                                                                          u32  len,
 *                                                                          u8   *ptr_rtn_buf,
 *																			u32	 read_mode,
 *																			SPI_NAND_FLASH_READ_DUMMY_BYTE_T dummy_mode)
 * PURPOSE : To implement the SPI nand protocol for read from cache with single speed.
 * AUTHOR  : 
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : data_offset  - The data_offset variable of this function.
 *           len          - The len variable of this function.
 *   OUTPUT: ptr_rtn_buf  - A pointer to the ptr_rtn_buf variable.
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_protocol_read_from_cache( u32 data_offset, 
																u32 len, 
																u8 *ptr_rtn_buf, 
																u32 read_mode,
																SPI_NAND_FLASH_READ_DUMMY_BYTE_T dummy_mode )
{			

	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T	rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	
	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;	
	
	/* 1. Chip Select low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();
	
	/* 2. Send opcode */
	switch (read_mode)
	{
		/* 03h */
		case SPI_NAND_FLASH_READ_SPEED_MODE_SINGLE:
			_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_READ_FROM_CACHE_SINGLE );
			break;

		/* 3Bh */
		case SPI_NAND_FLASH_READ_SPEED_MODE_DUAL:
			_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_READ_FROM_CACHE_DUAL );
			break;										

		/* 6Bh */
		case SPI_NAND_FLASH_READ_SPEED_MODE_QUAD:
			_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_READ_FROM_CACHE_QUAD );
			break;

		default:
			break;
	}
	
	/* 3. Send data_offset addr */
	if( dummy_mode == SPI_NAND_FLASH_READ_DUMMY_BYTE_PREPEND )
	{
		_SPI_NAND_WRITE_ONE_BYTE(0xff);		/* dummy byte */
	}
	
	if( ((ptr_dev_info_t->feature) & SPI_NAND_FLASH_PLANE_SELECT_HAVE) )
	{
		if( _plane_select_bit == 0)
		{
			_SPI_NAND_WRITE_ONE_BYTE( ((data_offset >> 8 ) &(0xef)) );
		}
		if( _plane_select_bit == 1)
		{
			_SPI_NAND_WRITE_ONE_BYTE( ((data_offset >> 8 ) | (0x10)) );
		}				
	}	
	else
	{
		_SPI_NAND_WRITE_ONE_BYTE( ((data_offset >> 8 ) &(0xff)) );
	}

	if((dummy_mode == SPI_NAND_FLASH_READ_DUMMY_BYTE_PREPEND) && 
	  (read_mode == SPI_NAND_FLASH_READ_SPEED_MODE_SINGLE)) {
		_SPI_NAND_WRITE_ONE_BYTE_WITH_CMD(_SPI_CONTROLLER_VAL_OP_OS2IS, ((data_offset      ) &(0xff)) );	
	} else {
		_SPI_NAND_WRITE_ONE_BYTE( ((data_offset      ) &(0xff)) );	
	}
	
	if( dummy_mode == SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND )
	{
		/* dummy byte */
		switch(read_mode)
		{
			case SPI_NAND_FLASH_READ_SPEED_MODE_SINGLE:
				_SPI_NAND_WRITE_ONE_BYTE_WITH_CMD(_SPI_CONTROLLER_VAL_OP_OS2IS, 0xff);	
				break;

			case SPI_NAND_FLASH_READ_SPEED_MODE_DUAL:
				_SPI_NAND_WRITE_ONE_BYTE_WITH_CMD(_SPI_CONTROLLER_VAL_OP_OS2ID, 0xff);	
				break;										

			case SPI_NAND_FLASH_READ_SPEED_MODE_QUAD:
				_SPI_NAND_WRITE_ONE_BYTE_WITH_CMD(_SPI_CONTROLLER_VAL_OP_OS2IQ, 0xff);	
				break;

			default:
				break;
		}
	}

	if(dummy_mode == SPI_NAND_FLASH_READ_DUMMY_BYTE_PREPEND)
	{
		/* dummy byte */
		switch(read_mode)
		{
			case SPI_NAND_FLASH_READ_SPEED_MODE_DUAL:
				_SPI_NAND_WRITE_ONE_BYTE_WITH_CMD(_SPI_CONTROLLER_VAL_OP_OS2ID, 0xff);	
				break;										

			case SPI_NAND_FLASH_READ_SPEED_MODE_QUAD:
				_SPI_NAND_WRITE_ONE_BYTE_WITH_CMD(_SPI_CONTROLLER_VAL_OP_OS2IQ, 0xff);	
				break;

			default:
				break;
		}
	}
	
	/* 4. Read n byte (len) data */
	switch (read_mode)
	{
		case SPI_NAND_FLASH_READ_SPEED_MODE_SINGLE:
			_SPI_NAND_READ_NBYTE( ptr_rtn_buf, len, SPI_CONTROLLER_SPEED_SINGLE);
			break;

		case SPI_NAND_FLASH_READ_SPEED_MODE_DUAL:
			_SPI_NAND_READ_NBYTE( ptr_rtn_buf, len, SPI_CONTROLLER_SPEED_DUAL);
			break;										

		case SPI_NAND_FLASH_READ_SPEED_MODE_QUAD:
			_SPI_NAND_READ_NBYTE( ptr_rtn_buf, len, SPI_CONTROLLER_SPEED_QUAD);
			break;

		default:
			break;
	}
		
	/* 5. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();
	
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_protocol_read_from_cache : data_offset=0x%x, buf=0x%x\n", data_offset, ptr_rtn_buf);	
	
	return (rtn_status);
}

static SPI_NAND_FLASH_RTN_T ecc_fail_check( u32 page_number )
{
	u8								status;
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T			rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	
	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;

	spi_nand_protocol_get_feature(_SPI_NAND_ADDR_STATUS, &status);

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_ecc_fail_check]: status=0x%x\n", status);

	if((ptr_dev_info_t->feature & SPI_NAND_FLASH_NO_ECC_STATUS_HAVE) == 0) {
		if((status & ptr_dev_info_t->ecc_fail_check_info.ecc_check_mask) == ptr_dev_info_t->ecc_fail_check_info.ecc_uncorrected_value) {
			rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
			_SPI_NAND_PRINTF("[spinand_ecc_fail_check] : ECC cannot recover detected !, page=0x%x\n", page_number);
		}
	}

	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_load_page_into_cache( long  page_number )
 * PURPOSE : To load page into SPI NAND chip.
 * AUTHOR  : 
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : page_number - The page_number variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2014/12/16  - The first revision for this function.
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_load_page_into_cache( u32 page_number)
{
	u8						status;
	SPI_NAND_FLASH_RTN_T	rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_load_page_into_cache: page number =0x%x\n", page_number);
		
	if(_current_page_num == page_number) {
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_load_page_into_cache: page number == _current_page_num\n");
	} else {
		spi_nand_select_die(page_number);

		spi_nand_protocol_page_read(page_number);

		/*  Checking status for load page/erase/program complete */
		do{
			spi_nand_protocol_get_feature(_SPI_NAND_ADDR_STATUS, &status);
		} while(status & _SPI_NAND_VAL_OIP) ;

		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_load_page_into_cache : status =0x%x\n", status);

		if(!isSpiNandAndCtrlECC)
		{
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_load_page_into_cache: check flash ecc status.\n");
			rtn_status = ecc_fail_check(page_number);
		}
	}
	
	return (rtn_status);
}

static SPI_NAND_FLASH_RTN_T spi_nand_read_page (u32 page_number, SPI_NAND_FLASH_READ_SPEED_MODE_T speed_mode)
{
	u32			 					idx=0;
	u32								i, j;
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T			rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	SPI_NFI_RTN_T					nfi_status;
	struct spi_nand_flash_oobfree 	*ptr_oob_entry_idx;	
	u16 							read_addr;
	SPI_NFI_MISC_SPEDD_CONTROL_T	dma_speed_mode;
	u32								read_cmd;

	SPI_NFI_CONF_T			spi_nfi_conf_t;
	SPI_ECC_DECODE_CONF_T	spi_ecc_decode_conf_t;
	u32 					check_cnt;

	SPI_ECC_DECODE_STATUS_T decode_status_t;
	SPI_CONTROLLER_CONF_T	spi_conf_t;
	u32 					offset1, offset2, offset3, dma_sec_size;

	if(_current_page_num != page_number) {
		ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;

		/* read from read_addr index in the page */
		read_addr = 0;

		/* Switch to manual mode*/
		_SPI_NAND_ENABLE_MANUAL_MODE();

		/* 1. Load Page into cache of NAND Flash Chip */
		if(spi_nand_load_page_into_cache(page_number) == SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK) {
			_SPI_NAND_PRINTF("spi_nand_read_page: Bad Block, ECC cannot recovery detecte, page=0x%x\n", page_number);
			rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
		}

		/* 2. Read whole data from cache of NAND Flash Chip */
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_read_page: curren_page_num=0x%x, page_number=0x%x\n", _current_page_num, page_number);

		/* No matter what status, we must read the cache data to dram */
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spi_nand_read_page: before read, _current_cache_page:\n");
		if(_spi_dma_mode == 0) {
			_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &_current_cache_page[0], ptr_dev_info_t->page_size + ptr_dev_info_t->oob_size);
		} else {
			_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &dma_read_page[0], ptr_dev_info_t->page_size + ptr_dev_info_t->oob_size);
		}
		
		if(ptr_dev_info_t->feature & SPI_NAND_FLASH_PLANE_SELECT_HAVE) {
			_plane_select_bit = ((page_number >> 6)& (0x1));

			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1,"spi_nand_read_page: plane select = 0x%x\n",  _plane_select_bit);			
		}
		
		if(_spi_dma_mode ==1 ) {	
			SPI_CONTROLLER_Get_Configure(&spi_conf_t);

			spi_conf_t.dummy_byte_num = 0;

			switch (speed_mode)
			{
				case SPI_NAND_FLASH_READ_SPEED_MODE_SINGLE:
					dma_speed_mode = SPI_NFI_MISC_CONTROL_X1;
					read_cmd = _SPI_NAND_OP_READ_FROM_CACHE_SINGLE;
					break;

				case SPI_NAND_FLASH_READ_SPEED_MODE_DUAL:
					dma_speed_mode = SPI_NFI_MISC_CONTROL_X2;
					read_cmd = _SPI_NAND_OP_READ_FROM_CACHE_DUAL;
					if(ptr_dev_info_t->dummy_mode == SPI_NAND_FLASH_READ_DUMMY_BYTE_PREPEND) {
						spi_conf_t.dummy_byte_num = 1;
					}
					break;										
					
				case SPI_NAND_FLASH_READ_SPEED_MODE_QUAD:
					dma_speed_mode = SPI_NFI_MISC_CONTROL_X4;
					read_cmd = _SPI_NAND_OP_READ_FROM_CACHE_QUAD;
					if(ptr_dev_info_t->dummy_mode == SPI_NAND_FLASH_READ_DUMMY_BYTE_PREPEND) {
						spi_conf_t.dummy_byte_num = 1;
					}
					break;

				default:
					_SPI_NAND_PRINTF("[Error] Read DMA : read speed setting error:%d!\n", speed_mode);
					dma_speed_mode = SPI_NFI_MISC_CONTROL_X1;
					read_cmd = _SPI_NAND_OP_READ_FROM_CACHE_SINGLE;
					break;
			}
			
			spi_conf_t.mode = SPI_CONTROLLER_MODE_DMA;
			SPI_CONTROLLER_Set_Configure(&spi_conf_t);

			/* Reset NFI statemachie is neccessary */
			SPI_NFI_Reset();
			
			SPI_NFI_Get_Configure(&spi_nfi_conf_t);
			SPI_NFI_Set_Configure(&spi_nfi_conf_t);

			SPI_ECC_Decode_Get_Configure(&spi_ecc_decode_conf_t);
			SPI_ECC_Decode_Set_Configure(&spi_ecc_decode_conf_t);
			
			if(spi_nfi_conf_t.hw_ecc_t == SPI_NFI_CON_HW_ECC_Enable) {
				SPI_ECC_Decode_Get_Configure(&spi_ecc_decode_conf_t);
				
				if(spi_ecc_decode_conf_t.decode_en == SPI_ECC_DECODE_ENABLE) {
					spi_ecc_decode_conf_t.decode_block_size = ((spi_nfi_conf_t.fdm_ecc_num + 512) * 8) + (spi_ecc_decode_conf_t.decode_ecc_abiliry * 13);

					SPI_ECC_Decode_Set_Configure(&spi_ecc_decode_conf_t);
					
					_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_read_page: decode block size=0x%x, ecc_num=0x%x, ecc_ab=0x%x\n", VPint(0xBFA12104), (spi_nfi_conf_t.fdm_ecc_num), (spi_ecc_decode_conf_t.decode_ecc_abiliry));
					_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_read_page : SPI_ECC_Decode_Enable \n");

					SPI_ECC_Decode_Disable();
					SPI_ECC_Encode_Disable();					
					SPI_ECC_Decode_Enable();
				}			
			}						

			/* Set plane select */
			if(ptr_dev_info_t->feature & SPI_NAND_FLASH_PLANE_SELECT_HAVE) {
				if(_plane_select_bit == 0) {
					read_addr &= ~(0x1000);
				} else if(_plane_select_bit == 1) {
					read_addr |= (0x1000);
				}				
			}

			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_read_page: dma_speed_mode:%d, read_cmd:%x\n", dma_speed_mode, read_cmd);

			/* Invalid dma_read_page */
#if defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL)
			dma_cache_inv((unsigned long)dma_read_page, _SPI_NAND_CACHE_SIZE);
#else
			flush_dcache_range((unsigned long)dma_read_page, (unsigned long)(dma_read_page + _SPI_NAND_CACHE_SIZE));
#endif

			mb();
			nfi_status = SPI_NFI_Read_SPI_NAND_Page(dma_speed_mode, read_cmd, read_addr, (u32 *)K1_TO_PHY(&dma_read_page[0]));
			if(nfi_status != SPI_NFI_RTN_NO_ERROR) {
				rtn_status = SPI_NAND_FLASH_RTN_NFI_FAIL;
			}
			
			if(spi_nfi_conf_t.hw_ecc_t == SPI_NFI_CON_HW_ECC_Enable) {
				if(spi_ecc_decode_conf_t.decode_en == SPI_ECC_DECODE_ENABLE) {
					/* Check Decode done or not */
					for(check_cnt = 0 ; check_cnt < _SPI_NFI_CHECK_ECC_DONE_MAX_TIMES; check_cnt++) {
						SPI_ECC_Decode_Check_Done(&decode_status_t);
						
						if(decode_status_t == SPI_ECC_DECODE_STATUS_DONE) {
							break;
						}
					}
					if(check_cnt == _SPI_NFI_CHECK_ECC_DONE_MAX_TIMES) {		
						_SPI_NAND_PRINTF("[Error] Read ECC : Check Decode Done Timeout ! \n");

						SPI_NAND_Flash_Clear_Read_Cache_Data();

						/*  return  somthing ? */
						rtn_status = SPI_NAND_FLASH_RTN_ECC_DECODE_FAIL;
					}

					if(SPI_ECC_DECODE_Check_Correction_Status() == SPI_ECC_RTN_CORRECTION_ERROR) {
						_SPI_NAND_PRINTF("[Error] Read ECC : ECC Fail! page:0x%x\n", page_number);
						SPI_NAND_Flash_Clear_Read_Cache_Data();

						/* Switch to manual mode*/
						_SPI_NAND_ENABLE_MANUAL_MODE();
						
						/*  return somthing ?*/
						rtn_status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
					}
				}
			}

			/* Switch to manual mode*/
			_SPI_NAND_ENABLE_MANUAL_MODE();
		} else {
			spi_nand_protocol_read_from_cache(read_addr, ((ptr_dev_info_t->page_size)+(ptr_dev_info_t->oob_size)), &_current_cache_page[0], speed_mode, ptr_dev_info_t->dummy_mode );
		}

		/* Divide read page into data segment and oob segment  */
		if((_spi_dma_mode == 0) || 
		  ((_spi_dma_mode == 1) && (!isSpiNandAndCtrlECC) && (spi_nfi_conf_t.auto_fdm_t == SPI_NFI_CON_AUTO_FDM_Disable) && (spi_nfi_conf_t.hw_ecc_t == SPI_NFI_CON_HW_ECC_Disable)))
		{
			
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spi_nand_read_page: after read, _current_cache_page:\n");

			if(_spi_dma_mode == 0) {
				_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &_current_cache_page[0], ptr_dev_info_t->page_size + ptr_dev_info_t->oob_size);
				memcpy(&_current_cache_page_data[0], &_current_cache_page[0], ptr_dev_info_t->page_size);
				memcpy(&_current_cache_page_oob[0],  &_current_cache_page[ptr_dev_info_t->page_size], ptr_dev_info_t->oob_size);
			} else {
				_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &dma_read_page[0], ptr_dev_info_t->page_size + ptr_dev_info_t->oob_size);
				memcpy(&_current_cache_page_data[0], &dma_read_page[0], ptr_dev_info_t->page_size);
				memcpy(&_current_cache_page_oob[0],  &dma_read_page[ptr_dev_info_t->page_size], ptr_dev_info_t->oob_size);
			}
			
			idx = 0;
			ptr_oob_entry_idx = (struct spi_nand_flash_oobfree*) &((ptr_dev_info_t->oob_free_layout)->oobfree);

			if(_ondie_ecc_flag == 1)   /*  When OnDie ecc is enable,  mapping oob area is neccessary */
			{
				/* Transter oob area from physical offset into logical offset */
				for( i=0; (i<SPI_NAND_FLASH_OOB_FREE_ENTRY_MAX) && (ptr_oob_entry_idx[i].len) && (idx< ((ptr_dev_info_t->oob_free_layout)->oobsize)) ; i++)
				{
					for(j=0; (j< (ptr_oob_entry_idx[i].len)) && (idx<(ptr_dev_info_t->oob_free_layout->oobsize)) ; j++)
					{
						/* _SPI_NAND_PRINTF("i=%d , j=%d, len=%d, idx=%d, size=%d\n", i, j,(ptr_oob_entry_idx[i].len), idx, (ptr_dev_info_t->oob_free_layout->oobsize) ); */
						_current_cache_page_oob_mapping[idx] = _current_cache_page_oob[(ptr_oob_entry_idx[i].offset)+j];
						idx++;
					}
				}	
			}
			else
			{
				memcpy( &_current_cache_page_oob_mapping[0],  &_current_cache_page_oob[0], (ptr_dev_info_t->oob_size) );	
			}
		}
		else
		{	
			
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spi_nand_read_page: after read, _current_cache_page:\n");
			_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &dma_read_page[0], ptr_dev_info_t->page_size + ptr_dev_info_t->oob_size);

			if(spi_nfi_conf_t.auto_fdm_t == SPI_NFI_CON_AUTO_FDM_Disable)  
			{
				offset1 = 0;
				offset2 = 0;
				offset3 = 0;
				dma_sec_size = ((spi_nfi_conf_t.page_size_t)/ (spi_nfi_conf_t.sec_num));
				for(i = 0; i < spi_nfi_conf_t.sec_num; i++) {
					memcpy( &_current_cache_page_data[offset1], &dma_read_page[offset2], dma_sec_size);
					memcpy( &_current_cache_page_oob[offset3], &dma_read_page[offset2+dma_sec_size], (spi_nfi_conf_t.spare_size_t) );
					offset1 += dma_sec_size;
					offset2 += (dma_sec_size+ (spi_nfi_conf_t.spare_size_t));
					offset3 += (spi_nfi_conf_t.spare_size_t);					
				}

				offset1 = 0;
				offset2 = 0;
				for(i = 0; i < spi_nfi_conf_t.sec_num; i++) {
					memcpy( &_current_cache_page_oob_mapping[offset1], &_current_cache_page_oob[offset2], spi_nfi_conf_t.fdm_num);
					offset1 += spi_nfi_conf_t.fdm_num;
					offset2 += spi_nfi_conf_t.spare_size_t;
				}
			}
			else /* Auto FDM enable : Data and oob alternate ,  Data inside DRAM , oob inside NFI register */  
			{
				memcpy( &_current_cache_page_data[0], &dma_read_page[0], (ptr_dev_info_t->page_size) );
				SPI_NFI_Read_SPI_NAND_FDM(&_current_cache_page_oob[0], (ptr_dev_info_t->oob_size));
				memcpy( &_current_cache_page_oob_mapping[0], &_current_cache_page_oob[0], ptr_dev_info_t->oob_size);
			}
		}
		
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spi_nand_read_page: _current_cache_page:\n");
		if(_spi_dma_mode == 0) {
			_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &_current_cache_page[0], ptr_dev_info_t->page_size + ptr_dev_info_t->oob_size);
		} else {
			_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &dma_read_page[0], ptr_dev_info_t->page_size + ptr_dev_info_t->oob_size);
		}
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spi_nand_read_page: _current_cache_page_data:\n");
		_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &_current_cache_page_data[0], ptr_dev_info_t->page_size);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spi_nand_read_page: _current_cache_page_oob:\n");
		_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &_current_cache_page_oob[0], ptr_dev_info_t->oob_size);		
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spi_nand_read_page: _current_cache_page_oob_mapping:\n");
		_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &_current_cache_page_oob_mapping[0], (ptr_dev_info_t->oob_free_layout)->oobsize);
		
		_current_page_num = page_number; 
	}	

	return rtn_status;
}

#if !defined(BOOTROM_EXT)
/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_read_internal( u32     addr,
 *                                                               u32     len,
 *                                                               u8      *ptr_rtn_buf )
 * PURPOSE : To read flash internally.
 * AUTHOR  : 
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : addr         - The addr variable of this function.
 *           len          - The len variable of this function.
 *   OUTPUT: ptr_rtn_buf  - A pointer to the ptr_rtn_buf variable.
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2014/12/19  - The first revision for this function.
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_read_internal(
												#if defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL)
													u64 addr,
												#else
													u32 addr,
												#endif
													u32 len, 
													u8 *ptr_rtn_buf, 
													SPI_NAND_FLASH_READ_SPEED_MODE_T speed_mode,
													SPI_NAND_FLASH_RTN_T *status)
{
	u32			 					page_number, data_offset;
#if defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL)
	/* for exceed 64bits address */
	u64								read_addr, physical_read_addr;
#else
	u32								read_addr, physical_read_addr;
#endif
	u32			 					remain_len, logical_block, physical_block;
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T			rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	SPI_ECC_RTN_T					ecc_status = SPI_ECC_RTN_NO_ERROR;
	unsigned long					spinand_spinlock_flags;
	SPI_NFI_CONF_T					spi_nfi_conf_t;
	SPI_ECC_DECODE_CONF_T			spi_ecc_decode_conf_t;
	
#if	defined(TCSUPPORT_NAND_BMT) && !defined(LZMA_IMG) && !defined(BOOTROM_EXT)
    unsigned short phy_block_bbt;
	unsigned long  addr_offset_in_block;
#endif
	
	if((0xbc000000 <= addr) && (addr <= 0xbfffffff)) {		/* Reserver address area for system */
		if( (addr & 0xbfc00000) == 0xbfc00000) {
			addr &= 0x003fffff;
		} else {
			addr &= 0x03ffffff;
		}
	}

	SPI_NFI_Get_Configure(&spi_nfi_conf_t);
	SPI_ECC_Decode_Get_Configure(&spi_ecc_decode_conf_t);
		
	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;			
	read_addr  		= addr;
	remain_len 		= len;

#if defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL)
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "\nspi_nand_read_internal : addr=0x%llx, len=0x%x\n", addr, len );
#else
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "\nspi_nand_read_internal : addr=0x%lx, len=0x%x\n", addr, len );
#endif

	_SPI_NAND_SEMAPHORE_LOCK();

	*status = SPI_NAND_FLASH_RTN_NO_ERROR;

	while(remain_len > 0) {
		physical_read_addr = read_addr;

#if	defined(TCSUPPORT_NAND_BMT) && !defined(LZMA_IMG) && !defined(BOOTROM_EXT)
		addr_offset_in_block = (read_addr % ptr_dev_info_t->erase_size);
		logical_block = (read_addr / (ptr_dev_info_t->erase_size));
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1,"read_addr=0x%x, erase size =0x%x, logical_block =0x%x\n", read_addr, (ptr_dev_info_t->erase_size), logical_block);
		physical_block = get_mapping_block_index(logical_block, &phy_block_bbt);
		physical_read_addr = (physical_block * ptr_dev_info_t->erase_size) + addr_offset_in_block;
		if(physical_block != logical_block) {			
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "Bad Block Mapping, from %d block to %d block\n", logical_block, physical_block);
		}
#endif					
		/* Caculate page number */
		data_offset = (physical_read_addr % (ptr_dev_info_t->page_size));
		page_number = (physical_read_addr / (ptr_dev_info_t->page_size));

		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_read_internal: read_addr=0x%x, page_number=0x%x, data_offset=0x%x\n", physical_read_addr, page_number, data_offset);

		if(len == 1 && _current_page_num == page_number) {
			ptr_rtn_buf[0] = _current_cache_page_data[data_offset];
			_SPI_NAND_SEMAPHORE_UNLOCK();
			return rtn_status;
		}

		rtn_status = spi_nand_read_page(page_number, speed_mode);
		if(_spi_dma_mode == 1) {
			if((spi_nfi_conf_t.hw_ecc_t == SPI_NFI_CON_HW_ECC_Enable) &&
			   (spi_ecc_decode_conf_t.decode_en == SPI_ECC_DECODE_ENABLE)) {
				ecc_status = SPI_ECC_DECODE_Check_Correction_Status();
				if(ecc_status == SPI_ECC_RTN_NO_ERROR ) {
					*status = SPI_NAND_FLASH_RTN_NO_ERROR;
				} else {
					*status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
					_SPI_NAND_SEMAPHORE_UNLOCK();
					return (rtn_status);
				}
			} else {
				if(rtn_status == SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK) {
					*status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
					_SPI_NAND_SEMAPHORE_UNLOCK();
					return (rtn_status);
				}
			}
		} else {
			if(rtn_status == SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK) {
				*status = SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK;
				_SPI_NAND_SEMAPHORE_UNLOCK();
				return (rtn_status);
			}
		}

		/* 3. Retrieve the request data */
		if((data_offset + remain_len) < ptr_dev_info_t->page_size) {
			memcpy( &ptr_rtn_buf[len - remain_len], &_current_cache_page_data[data_offset], remain_len);
			remain_len =0;
		} else {
			memcpy( &ptr_rtn_buf[len - remain_len], &_current_cache_page_data[data_offset], ptr_dev_info_t->page_size - data_offset);
			remain_len -= (ptr_dev_info_t->page_size - data_offset);
			read_addr += (ptr_dev_info_t->page_size - data_offset);
		}
	}
	
	_SPI_NAND_SEMAPHORE_UNLOCK();

	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: int SPI_NAND_Flash_Read_NByte( long     addr,
 *                                          long     len,
 *                                          long     *retlen,
 *                                          char     *buf    )
 * PURPOSE : To provide interface for Read N Bytes from SPI NAND Flash.
 * AUTHOR  : 
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : addr    - The addr variable of this function.
 *           len     - The len variable of this function.
 *           retlen  - The retlen variable of this function.
 *           buf     - The buf variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2014/12/12  - The first revision for this function.
 *
 *------------------------------------------------------------------------------------
 */
u32 SPI_NAND_Flash_Read_NByte(	
							#if defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL)
								u64 							addr,
							#else
								u32 							addr,
							#endif
								u32								 len, 
								u32								 *retlen, 
								u8								 *buf, 
								SPI_NAND_FLASH_READ_SPEED_MODE_T speed_mode,
								SPI_NAND_FLASH_RTN_T *status)
{
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "SPI_NAND_Flash_Read_NByte\n");
	
	return spi_nand_read_internal(addr, len, buf, speed_mode, status);	
}

/*------------------------------------------------------------------------------------
 * FUNCTION: char SPI_NAND_Flash_Read_Byte( long     addr )
 * PURPOSE : To provide interface for read 1 Bytes from SPI NAND Flash.
 * AUTHOR  : 
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : addr - The addr variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2014/12/12  - The first revision for this function.
 *
 *------------------------------------------------------------------------------------
 */
unsigned char SPI_NAND_Flash_Read_Byte(
									#if defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL)
										u64 addr,
									#else
										u32 addr,
									#endif
										SPI_NAND_FLASH_RTN_T *status)
{
	unsigned char 					ch;
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	int 							ret = 0;
	size_t 							retlen;

	ptr_dev_info_t  = _SPI_NAND_GET_DEVICE_INFO_PTR;

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "SPI_NAND_Flash_Read_Byte\n");

#ifdef TCSUPPORT_BB_256KB
	/* The mi.conf has new offset(0x3FE00) in 256KB tcboot.bin,
	 * The mi.conf offset is 0xFF00 in 128KB tcboot.bin.
	 */
	if(addr >= 0xFF00 && addr <= 0xFFFF) {
		addr += 0x2FF00;
	}
#endif

#if !(defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL))
	spi_nand_read_internal(addr, 1, &ch, ptr_dev_info_t->read_mode, status);
#elif defined(TCSUPPORT_2_6_36_KERNEL)
	ret = spi_nand_mtd->read(spi_nand_mtd, (loff_t)addr, 1, &retlen, &ch);
#else
	ret = spi_nand_mtd->_read(spi_nand_mtd, (loff_t)addr, 1, &retlen, &ch);
#endif

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "SPI_NAND_Flash_Read_Byte : buf=0x%x\n", ch);
	
	return ch;
}

/*------------------------------------------------------------------------------------
 * FUNCTION: long SPI_NAND_Flash_Read_DWord( long    addr )
 * PURPOSE : To provide interface for read Double Word from SPI NAND Flash.
 * AUTHOR  : 
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : addr - The addr variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2014/12/12  - The first revision for this function.
 *
 *------------------------------------------------------------------------------------
 */
unsigned long SPI_NAND_Flash_Read_DWord(
									#if defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL)
										u64 addr,
									#else
										u32 addr,
									#endif
										SPI_NAND_FLASH_RTN_T *status)
{
	u32		 ret_val=0;
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	size_t							retlen;
	int								ret = 0;

	ptr_dev_info_t  = _SPI_NAND_GET_DEVICE_INFO_PTR;
	
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "SPI_NAND_Flash_Read_DWord\n");

#if !(defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL))
	spi_nand_read_internal(addr, 4, &ret_val, ptr_dev_info_t->read_mode, status);
#elif defined(TCSUPPORT_2_6_36_KERNEL)
	ret = spi_nand_mtd->read(spi_nand_mtd, (loff_t)addr, 4, &retlen, &ret_val);
#else
	ret = spi_nand_mtd->_read(spi_nand_mtd, (loff_t)addr, 4, &retlen, &ret_val);
#endif

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "SPI_NAND_Flash_Read_DWord : ret_val=0x%x\n", ret_val);
	
	return ret_val;
}

int nandflash_read(unsigned long from, unsigned long len, u32 *retlen, unsigned char *buf, SPI_NAND_FLASH_RTN_T *status)
{
	struct SPI_NAND_FLASH_INFO_T	 *ptr_dev_info_t;
	 
	ptr_dev_info_t  = _SPI_NAND_GET_DEVICE_INFO_PTR;

	if( SPI_NAND_Flash_Read_NByte(from, len, retlen, buf, ptr_dev_info_t->read_mode, status) == SPI_NAND_FLASH_RTN_NO_ERROR) {
		return 0;
	} else {
		return -1;
	}	
}
#endif

#if WRITE_AREA
#endif

#if !defined(LZMA_IMG) || defined(TCSUPPORT_BB_256KB)
/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_write_enable( void )
 * PURPOSE : To implement the SPI nand protocol for write enable.
 * AUTHOR  : 
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : None
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2014/12/17  - The first revision for this function.
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_protocol_write_enable( void )
{
	SPI_NAND_FLASH_RTN_T	rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	
	/* 1. Chip Select Low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();
	
	/* 2. Write op_cmd 0x06 (Write Enable (WREN)*/
	_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_WRITE_ENABLE );
	
	/* 3. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();
	
	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_write_disable( void )
 * PURPOSE : To implement the SPI nand protocol for write disable.
 * AUTHOR  : 
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : None
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2014/12/17  - The first revision for this function.
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_protocol_write_disable( void )
{
	SPI_NAND_FLASH_RTN_T	rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;	
	
	/* 1. Chip Select Low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();
	
	/* 2. Write op_cmd 0x04 (Write Disable (WRDI)*/
	_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_WRITE_DISABLE );
	
	/* 3. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();
	
	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_program_load( u32     addr,
 *                                                                       u8      *ptr_data,
 *                                                                       u32     len,
 *																		 u32 write_mode)
 * PURPOSE : To implement the SPI nand protocol for program load, with single speed.
 * AUTHOR  : 
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : addr      - The addr variable of this function.
 *           ptr_data  - A pointer to the ptr_data variable.
 *           len       - The len variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_protocol_program_load ( u32 addr, 
																		  u8 *ptr_data, 
																		  u32 len,
																		  u32 write_mode)
{

	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T	rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	
	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;
	
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_protocol_program_load: addr=0x%x, len=0x%x\n", addr, len );
	
	/* 1. Chip Select low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();
	
	/* 2. Send opcode */
	switch (write_mode)
	{
		/* 02h */
		case SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE:
			_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_PROGRAM_LOAD_SINGLE );
			break;

		/* 32h */
		case SPI_NAND_FLASH_WRITE_SPEED_MODE_QUAD:
			_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_PROGRAM_LOAD_QUAD );
			break;										

		default:
			break;
	}
	
	/* 3. Send address offset */
	if( ((ptr_dev_info_t->feature) & SPI_NAND_FLASH_PLANE_SELECT_HAVE) )
	{
		if( _plane_select_bit == 0)
		{
			_SPI_NAND_WRITE_ONE_BYTE( ((addr >> 8 ) & (0xef)) );
		}
		if( _plane_select_bit == 1)
		{
			_SPI_NAND_WRITE_ONE_BYTE( ((addr >> 8 ) | (0x10)) );
		}				
	}	
	else
	{
		_SPI_NAND_WRITE_ONE_BYTE( ((addr >> 8 ) & (0xff)) );
	}

	_SPI_NAND_WRITE_ONE_BYTE( ((addr)        & (0xff)) );
	
	/* 4. Send data */
	switch (write_mode)
	{
		case SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE:
			_SPI_NAND_WRITE_NBYTE( ptr_data, len, SPI_CONTROLLER_SPEED_SINGLE);
			break;

		case SPI_NAND_FLASH_WRITE_SPEED_MODE_QUAD:
			_SPI_NAND_WRITE_NBYTE( ptr_data, len, SPI_CONTROLLER_SPEED_QUAD);
			break;										

		default:
			break;
	}
	
	/* 5. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();	
	
	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_program_load_random( u32     addr,
 *                                                                       u8      *ptr_data,
 *                                                                       u32     len,
 *																		 u32 write_mode)
 * PURPOSE : To implement the SPI nand protocol for program load, with single speed.
 * AUTHOR  : 
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : addr      - The addr variable of this function.
 *           ptr_data  - A pointer to the ptr_data variable.
 *           len       - The len variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2014/12/17  - The first revision for this function.
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_protocol_program_load_random ( u32 addr, 
																  u8 *ptr_data, 
																  u32 len,
																  u32 write_mode)
{
	SPI_NAND_FLASH_RTN_T	rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_protocol_program_load_random: addr=0x%x, len=0x%x\n", addr, len );
	
	/* 1. Chip Select low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();
	
	/* 2. Send opcode */
	switch (write_mode)
	{
		/* 84 */
		case SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE:
			_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_PROGRAM_LOAD_RAMDOM_SINGLE );
			break;

		/* 34h */
		case SPI_NAND_FLASH_WRITE_SPEED_MODE_QUAD:
			_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_PROGRAM_LOAD_RAMDON_QUAD );
			break;										

		default:
			break;
	}
	
	/* 3. Send address offset */
	_SPI_NAND_WRITE_ONE_BYTE( ((addr >> 8  ) & 0xff) );
	_SPI_NAND_WRITE_ONE_BYTE( ((addr)        & 0xff) );
	
	/* 4. Send data */
	switch (write_mode)
	{
		case SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE:
			_SPI_NAND_WRITE_NBYTE( ptr_data, len, SPI_CONTROLLER_SPEED_SINGLE);
			break;

		case SPI_NAND_FLASH_WRITE_SPEED_MODE_QUAD:
			_SPI_NAND_WRITE_NBYTE( ptr_data, len, SPI_CONTROLLER_SPEED_QUAD);
			break;										

		default:
			break;
	}
	
	/* 5. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();	
	
	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_program_execute( u32  addr )
 * PURPOSE : To implement the SPI nand protocol for program execute.
 * AUTHOR  : 
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : addr - The addr variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2014/12/17  - The first revision for this function.
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_protocol_program_execute ( u32 addr )
{
	SPI_NAND_FLASH_RTN_T	rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_protocol_program_execute: addr=0x%x\n", addr);	
	
	/* 1. Chip Select low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();
	
	/* 2. Send 10h opcode */
	_SPI_NAND_WRITE_ONE_BYTE(_SPI_NAND_OP_PROGRAM_EXECUTE);
	
	/* 3. Send address offset */
	_SPI_NAND_WRITE_ONE_BYTE( ((addr >> 16) & 0xff) );
	_SPI_NAND_WRITE_ONE_BYTE( ((addr >> 8 ) & 0xff) );
	_SPI_NAND_WRITE_ONE_BYTE( ((addr)       & 0xff) );
	
	/* 4. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();	
	
	return (rtn_status);
}

static SPI_NAND_FLASH_RTN_T spi_nand_dma_program_load(u32 addr, u32 oob_len, SPI_NAND_FLASH_WRITE_SPEED_MODE_T speed_mode)
{
	SPI_CONTROLLER_CONF_T	spi_conf_t; 	
	SPI_NFI_CONF_T			spi_nfi_conf_t;
	SPI_ECC_ENCODE_CONF_T	spi_ecc_encode_conf_t;
	SPI_NAND_FLASH_RTN_T	rtn_status;
	u16 					write_addr;
	u32						write_cmd;
	SPI_NFI_MISC_SPEDD_CONTROL_T	dma_speed_mode;
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;

	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;	

	/* Set plane select */
	write_addr = addr;
	if(ptr_dev_info_t->feature & SPI_NAND_FLASH_PLANE_SELECT_HAVE) {
		if(_plane_select_bit == 0) {
			write_addr &= ~(0x1000);
		} else if(_plane_select_bit == 1) {
			write_addr |= (0x1000);
		}				
	}

	switch (speed_mode)
	{
		case SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE:
			dma_speed_mode = SPI_NFI_MISC_CONTROL_X1;
			write_cmd = _SPI_NAND_OP_PROGRAM_LOAD_SINGLE;
			break;
						
		case SPI_NAND_FLASH_WRITE_SPEED_MODE_QUAD:
			dma_speed_mode = SPI_NFI_MISC_CONTROL_X4;
			write_cmd = _SPI_NAND_OP_PROGRAM_LOAD_QUAD;
			break;

		default:
			_SPI_NAND_PRINTF("[Error] Write DMA : write speed setting error:%d!\n", speed_mode);
			dma_speed_mode = SPI_NFI_MISC_CONTROL_X1;
			write_cmd = _SPI_NAND_OP_PROGRAM_LOAD_SINGLE;
			break;
	}

	SPI_CONTROLLER_Get_Configure(&spi_conf_t);
	spi_conf_t.dummy_byte_num = 0 ;
	spi_conf_t.mode = SPI_CONTROLLER_MODE_DMA;
	SPI_CONTROLLER_Set_Configure(&spi_conf_t);

	/* Reset NFI statemachie is neccessary */
	SPI_NFI_Reset();

	SPI_NFI_Get_Configure(&spi_nfi_conf_t);
	SPI_NFI_Set_Configure(&spi_nfi_conf_t);

	SPI_ECC_Encode_Get_Configure(&spi_ecc_encode_conf_t);
	SPI_ECC_Encode_Set_Configure(&spi_ecc_encode_conf_t);
	
	if(spi_nfi_conf_t.hw_ecc_t == SPI_NFI_CON_HW_ECC_Enable) {
		SPI_ECC_Encode_Get_Configure(&spi_ecc_encode_conf_t);
		
		if(spi_ecc_encode_conf_t.encode_en == SPI_ECC_ENCODE_ENABLE) {
			spi_ecc_encode_conf_t.encode_block_size = (spi_nfi_conf_t.fdm_ecc_num + 512) ;
			SPI_ECC_Encode_Set_Configure(&spi_ecc_encode_conf_t);

			SPI_ECC_Encode_Disable();
			SPI_ECC_Decode_Disable();
			SPI_ECC_Encode_Enable();
		}
	}

	if(spi_nfi_conf_t.auto_fdm_t == SPI_NFI_CON_AUTO_FDM_Enable) {	/* Data and oob alternate */
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_dma_program_load: set fdm\n");
		SPI_NFI_Write_SPI_NAND_FDM(&_current_cache_page_oob_mapping[0], oob_len);
	}

	rtn_status = SPI_NFI_Write_SPI_NAND_page(dma_speed_mode, write_cmd, write_addr, (u32 *)K1_TO_PHY(&dma_write_page[0]));

	/* Switch to manual mode*/
	_SPI_NAND_ENABLE_MANUAL_MODE();	

	return rtn_status;
}

#ifdef UBIFS_BLANK_PAGE_FIXUP
UBIFS_BLANK_PAGE_ECC_T check_blank_page(u32 page_number)
{
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	u32 block, page_per_block;
	u8 i = 0, sec_idx;
	u8 ecc_parity_0[8] = {0};
	u8 ctlerECC_blank_ecc_4[] = {0x26, 0x20, 0x98, 0x1b, 0x87, 0x6e, 0xfc, 0xff};
	SPI_NFI_CONF_T spi_nfi_conf_t;
	int cmpVal;
	SPI_NAND_FLASH_DEBUG_LEVEL_T ubiDbgLv = SPI_NAND_FLASH_DEBUG_LEVEL_2;
	
	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;

	page_per_block = (ptr_dev_info_t->erase_size / ptr_dev_info_t->page_size);
	block = page_number / page_per_block;

	if ((!isSpiNandAndCtrlECC) && (ptr_dev_info_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_WINBOND)) {
		if((ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_W25N01GV) ||
		   (ptr_dev_info_t->dev_id == _SPI_NAND_DEVICE_ID_W25M02GV)) {
		   /* for winbond, if data has been written blank,
		    * the ECC parity is all 0.
		    */
			memset(ecc_parity_0, 0x0, sizeof(ecc_parity_0));

			for(i = 0; i < 4; i++) {
				cmpVal= memcmp(ecc_parity_0, &_current_cache_page_oob[i * 16 + 8], sizeof(ecc_parity_0));
				if(cmpVal == 0) {
					_SPI_NAND_DEBUG_PRINTF(ubiDbgLv, "page 0x%x detected ECC parity 0 at block:0x%x page offset:0x%x.\n", page_number, block, page_number % page_per_block);
					if(_spi_dma_mode == 1) {
						_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &dma_read_page[0], ((ptr_dev_info_t->page_size)+(ptr_dev_info_t->oob_size)));
					} else {
						_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &_current_cache_page[0], ((ptr_dev_info_t->page_size)+(ptr_dev_info_t->oob_size)));
					}
					return SPI_NAND_FLASH_UBIFS_BLANK_PAGE_ECC_MISMATCH; /* blank sector*/
				}
			}
		}
		
		return SPI_NAND_FLASH_UBIFS_BLANK_PAGE_ECC_MATCH; /* Good Page*/
	} else if(isEN7526c && isSpiNandAndCtrlECC) {
		SPI_NFI_Get_Configure(&spi_nfi_conf_t);
		
		for (sec_idx = 0; sec_idx < spi_nfi_conf_t.sec_num; sec_idx++) {
			cmpVal = memcmp(ctlerECC_blank_ecc_4, &_current_cache_page_oob[sec_idx * spi_nfi_conf_t.spare_size_t + spi_nfi_conf_t.fdm_num], sizeof(ctlerECC_blank_ecc_4));
			if(cmpVal == 0) {
				_SPI_NAND_DEBUG_PRINTF(ubiDbgLv, "page 0x%x detected ECC parity 0 at block:0x%x page offset:0x%x.\n", page_number, block, page_number % page_per_block);
				_SPI_NAND_DEBUG_PRINTF(ubiDbgLv, "oob:\n");
				_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &_current_cache_page_oob[0], ((ptr_dev_info_t->oob_size)));
				_SPI_NAND_DEBUG_PRINTF(ubiDbgLv, "page:\n");
				_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &dma_read_page[0], ((ptr_dev_info_t->page_size)+(ptr_dev_info_t->oob_size)));
				return SPI_NAND_FLASH_UBIFS_BLANK_PAGE_ECC_MISMATCH; /* blank sector*/
			}
		}
	} else {
		return SPI_NAND_FLASH_UBIFS_BLANK_PAGE_ECC_MATCH; /* Good Page*/
	}
}

SPI_NAND_FLASH_RTN_T store_block(u32 block, u8 *block_buf)
{
	struct SPI_NAND_FLASH_INFO_T *ptr_dev_info_t;
	int i;
	u32 page_per_block;
	u32 start_page;
	SPI_NAND_FLASH_RTN_T rtn_status;
	SPI_NFI_CONF_T spi_nfi_conf_t;
	SPI_NAND_FLASH_DEBUG_LEVEL_T ubiDbgLv = SPI_NAND_FLASH_DEBUG_LEVEL_2;

	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;

	page_per_block = (ptr_dev_info_t->erase_size / ptr_dev_info_t->page_size);
	start_page = block * page_per_block;

	// read all pages in the block
	for(i = 0; i < page_per_block; i++) {
		rtn_status = spi_nand_read_page(i + start_page, SPI_NAND_FLASH_READ_SPEED_MODE_SINGLE);
		if(rtn_status == SPI_NAND_FLASH_RTN_NO_ERROR) {
			_SPI_NAND_DEBUG_PRINTF(ubiDbgLv, "store_block: block:0x%x page offset:0x%x\n", block, i);
			
			if(isEN7526c && isSpiNandAndCtrlECC) {
				memcpy(block_buf + i * (ptr_dev_info_t->page_size + ptr_dev_info_t->oob_size), 
					   _current_cache_page_data, 
					   ptr_dev_info_t->page_size);
				memcpy(block_buf + i * (ptr_dev_info_t->page_size + ptr_dev_info_t->oob_size) + ptr_dev_info_t->page_size, 
					   _current_cache_page_oob, 
					   ptr_dev_info_t->oob_size);
				_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &_current_cache_page_data[0], (ptr_dev_info_t->page_size));
				_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &_current_cache_page_oob[0], (ptr_dev_info_t->oob_size));
			} else {
				if(_spi_dma_mode == 1) {
					memcpy(block_buf + i * (ptr_dev_info_t->page_size + ptr_dev_info_t->oob_size), 
						   dma_read_page, 
						   ptr_dev_info_t->page_size + ptr_dev_info_t->oob_size);
				} else {
					memcpy(block_buf + i * (ptr_dev_info_t->page_size + ptr_dev_info_t->oob_size), 
						   _current_cache_page, 
						   ptr_dev_info_t->page_size + ptr_dev_info_t->oob_size);
				}
				_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &_current_cache_page[0], ((ptr_dev_info_t->page_size)+(ptr_dev_info_t->oob_size)));
			}
		} else {
			_SPI_NAND_PRINTF("%s: fix blank page 0x%x read error\n",__func__, start_page+i);
			break;
		}
	}

	return rtn_status;
}

SPI_NAND_FLASH_RTN_T restore_block(u32 block, u8 *block_buf, u32 page_number)
{
	struct SPI_NAND_FLASH_INFO_T *ptr_dev_info_t;
	u32 i, j, k, idx;
	u32 page_per_block;
	u32 start_page;
	SPI_NAND_FLASH_RTN_T rtn_status;
	int isBlankData, isBlankOOB;
	u8 *page_buf = NULL;
	u8 oob_mapping[_SPI_NAND_OOB_SIZE];
	struct spi_nand_flash_oobfree 	*ptr_oob_entry_idx;	
	SPI_NAND_FLASH_DEBUG_LEVEL_T ubiDbgLv = SPI_NAND_FLASH_DEBUG_LEVEL_2;

	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;

	page_per_block = (ptr_dev_info_t->erase_size / ptr_dev_info_t->page_size);
	start_page = block * page_per_block;
	
	// read all pages in the block
	for(i = 0; i < page_per_block; i++) {
		if((i + start_page) == page_number) {
			_SPI_NAND_DEBUG_PRINTF(ubiDbgLv, "restore_block: skip source page block:0x%x page offset:0x%x\n", block, i);
			continue;
		}

		page_buf = (block_buf + i * (ptr_dev_info_t->page_size + ptr_dev_info_t->oob_size));

		isBlankData = 1;
		for(idx = 0; idx < ptr_dev_info_t->page_size; idx++) {
			if(page_buf[idx] != 0xFF) {
				isBlankData = 0;
				break;
			}
		}

		if(isBlankData == 1) {
			isBlankOOB = 1;
			ptr_oob_entry_idx = (struct spi_nand_flash_oobfree*) &( ptr_dev_info_t->oob_free_layout->oobfree );

			idx = 0;
			for(k = 0; (k < SPI_NAND_FLASH_OOB_FREE_ENTRY_MAX) && (ptr_oob_entry_idx[k].len) && (idx < ptr_dev_info_t->oob_free_layout->oobsize); k++)
			{
				for(j = 0; (j < ptr_oob_entry_idx[k].len) && idx < (ptr_dev_info_t->oob_free_layout->oobsize); j++)
				{
					if(page_buf[ptr_dev_info_t->page_size + (ptr_oob_entry_idx[k].offset) + j] != 0xFF) {
						isBlankOOB = 0;
						k = SPI_NAND_FLASH_OOB_FREE_ENTRY_MAX;
						break;
					}
					idx++;
				}
			}
		}

		if((isBlankData == 1) && (isBlankOOB == 1)) {
			_SPI_NAND_DEBUG_PRINTF(ubiDbgLv, "restore_block: skip blank page block:0x%x page offset:0x%x\n", block, i);
			continue;
		} else {
			ptr_oob_entry_idx = (struct spi_nand_flash_oobfree*) &( ptr_dev_info_t->oob_free_layout->oobfree );
			idx = 0;

			memset(oob_mapping, 0xFF, _SPI_NAND_OOB_SIZE);
			for(k = 0; (k < SPI_NAND_FLASH_OOB_FREE_ENTRY_MAX) && (ptr_oob_entry_idx[k].len) && (idx < ptr_dev_info_t->oob_free_layout->oobsize); k++)
			{
				for(j = 0; (j < ptr_oob_entry_idx[k].len) && (idx < ptr_dev_info_t->oob_free_layout->oobsize); j++)
				{
					oob_mapping[idx] = page_buf[ptr_dev_info_t->page_size + (ptr_oob_entry_idx[k].offset) + j];
					idx++;
				}
			}
		}

		rtn_status = spi_nand_write_page(i + start_page, 
										 0, 
										 (block_buf + i * (ptr_dev_info_t->page_size + ptr_dev_info_t->oob_size)), 
										 ptr_dev_info_t->page_size, 
										 0, 
										 oob_mapping, 
										 ptr_dev_info_t->oob_size, 
										 SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE);

		_SPI_NAND_DEBUG_PRINTF(ubiDbgLv, "restore_block: block:0x%x page offset:0x%x\n", block, i);
		
		if(rtn_status == SPI_NAND_FLASH_RTN_NO_ERROR) {
			_SPI_NAND_DEBUG_PRINTF(ubiDbgLv, "fixed page %x\n", start_page + i);
		} else {
			_SPI_NAND_PRINTF("%s: fix_ecc_0 0x%x write error \n",__func__, start_page+i);
			return rtn_status;
		}
	}

	return SPI_NAND_FLASH_RTN_NO_ERROR;
}


UBIFS_BLANK_PAGE_FIXUP_T fix_blank_page(u32 page_number)
{
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	u8 *block_buf;
	u32 page_per_block;
	u32 block;
	SPI_NAND_FLASH_RTN_T rtn_status;

	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;

	page_per_block = (ptr_dev_info_t->erase_size / ptr_dev_info_t->page_size);
	block = page_number / page_per_block;
	
	block_buf = (u8 *) kmalloc((ptr_dev_info_t->page_size + ptr_dev_info_t->oob_size) * page_per_block, GFP_KERNEL);
	if(!block_buf) {
		_SPI_NAND_PRINTF("%s:can not allocate buffer\n", __func__);
		return SPI_NAND_FLASH_UBIFS_BLANK_PAGE_FIXUP_FAIL;
	}
	memset(block_buf, 0xff, (ptr_dev_info_t->page_size + ptr_dev_info_t->oob_size) * page_per_block);

	/* store block */
	rtn_status = store_block(block, block_buf);
	if(rtn_status != SPI_NAND_FLASH_RTN_NO_ERROR) {
		kfree(block_buf);
		return SPI_NAND_FLASH_UBIFS_BLANK_PAGE_FIXUP_FAIL;
	}

	/* erase block */
	rtn_status = spi_nand_erase_block(block);
	if(rtn_status != SPI_NAND_FLASH_RTN_NO_ERROR) {
		kfree(block_buf);
		return SPI_NAND_FLASH_UBIFS_BLANK_PAGE_FIXUP_FAIL;
	}

	/* restore block except page_number */
	rtn_status = restore_block(block, block_buf, page_number);
	if(rtn_status != SPI_NAND_FLASH_RTN_NO_ERROR) {
		kfree(block_buf);
		return SPI_NAND_FLASH_UBIFS_BLANK_PAGE_FIXUP_FAIL;
	}

	kfree(block_buf);
	return SPI_NAND_FLASH_UBIFS_BLANK_PAGE_FIXUP_SUCCESS;
}
#endif

static SPI_NAND_FLASH_RTN_T spi_nand_write_page(u32 page_number, 
												u32 data_offset,
											  	const u8  *ptr_data, 
											  	u32 data_len, 
											  	u32 oob_offset,
												u8  *ptr_oob , 
												u32 oob_len,
											    SPI_NAND_FLASH_WRITE_SPEED_MODE_T speed_mode)
{
	u8		status, status_2;
	u32		i = 0, j = 0, idx = 0;
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	struct spi_nand_flash_oobfree 	*ptr_oob_entry_idx;	
	SPI_NAND_FLASH_RTN_T			rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	u16 							write_addr;
	SPI_NFI_CONF_T					spi_nfi_conf_t;
	u32								offset1, offset2, offset3, dma_sec_size;
	SPI_NAND_FLASH_DEBUG_LEVEL_T	ubiDbgLv = SPI_NAND_FLASH_DEBUG_LEVEL_2;
	static int						isUbifsBlankPageFix = 0;
	unsigned long					block_idx;

	/* write to write_addr index in the page */
	write_addr = 0;

	SPI_NFI_Get_Configure(&spi_nfi_conf_t);
	
	/* Switch to manual mode*/
	_SPI_NAND_ENABLE_MANUAL_MODE();

	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;

#ifdef ERASE_WRITE_CNT_LOG
	block_idx = page_number / PAGE_CNT_PER_BLOCK;
	if(erase_write_flag == SPI_NAND_FLASH_ERASE_WRITE_LOG_ENABLE) {
		b_write_cnt_per_erase[block_idx]++;
		b_write_total_cnt[block_idx]++;
	}
#endif

#ifdef UBIFS_BLANK_PAGE_FIXUP
	if(isUbifsBlankPageFix == 0) {
		/* Read Current page data to software cache buffer */
		if(isEN7526c && isSpiNandAndCtrlECC) {
			spi_nfi_conf_t.auto_fdm_t = SPI_NFI_CON_AUTO_FDM_Disable;
			SPI_NFI_Set_Configure(&spi_nfi_conf_t);
		}

		spi_nand_read_page(page_number, speed_mode);

		if(isEN7526c && isSpiNandAndCtrlECC) {
			spi_nfi_conf_t.auto_fdm_t = SPI_NFI_CON_AUTO_FDM_Enable;
			SPI_NFI_Set_Configure(&spi_nfi_conf_t);
		}
		
		if(check_blank_page(page_number) == SPI_NAND_FLASH_UBIFS_BLANK_PAGE_ECC_MISMATCH) {
			isUbifsBlankPageFix = 1;
			_SPI_NAND_DEBUG_PRINTF(ubiDbgLv, "UBIFS_BLANK_PAGE_FIXUP, page:0x%x\n", page_number);
			fix_blank_page(page_number);
			isUbifsBlankPageFix = 0;

			/* Read Current page data to software cache buffer */
			spi_nand_read_page(page_number, speed_mode);
		}
	}
#else
	/* Read Current page data to software cache buffer */
	spi_nand_read_page(page_number, speed_mode);
#endif

#if defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL)
	dma_cache_inv((unsigned long)dma_write_page, _SPI_NAND_CACHE_SIZE);
#else
	flush_dcache_range((unsigned long)dma_write_page, (unsigned long)(dma_write_page + _SPI_NAND_CACHE_SIZE));
#endif


	/* Write data & OOB */
	if((_spi_dma_mode == 0) ||
	  ((_spi_dma_mode == 1) && (!isSpiNandAndCtrlECC) && (spi_nfi_conf_t.auto_fdm_t == SPI_NFI_CON_AUTO_FDM_Disable) && (spi_nfi_conf_t.hw_ecc_t == SPI_NFI_CON_HW_ECC_Disable)))
	{
		if(data_len > 0) {
			memcpy( &_current_cache_page_data[data_offset], &ptr_data[0], data_len);
		}
		
		if(_spi_dma_mode == 0) {
			memcpy(&_current_cache_page[data_offset], &_current_cache_page_data[0], PAGE_SIZE);
		} else {
			memcpy((u8 *)K0_TO_K1(&dma_write_page[data_offset]), &_current_cache_page_data[0], PAGE_SIZE);
		}
		
		if(oob_len > 0) {
			if(_ondie_ecc_flag == 1) {	/*  When OnDie ecc is enable,  mapping oob area is neccessary */
				ptr_oob_entry_idx = (struct spi_nand_flash_oobfree*) &( ptr_dev_info_t->oob_free_layout->oobfree );
						
				for(i = 0; (i < SPI_NAND_FLASH_OOB_FREE_ENTRY_MAX) && (ptr_oob_entry_idx[i].len) && ((idx < ptr_dev_info_t->oob_free_layout->oobsize) && (idx < oob_len)); i++) {
					for(j = 0; (j < ptr_oob_entry_idx[i].len) && (idx < ptr_dev_info_t->oob_free_layout->oobsize) && ((idx < ptr_dev_info_t->oob_free_layout->oobsize) && (idx < oob_len)) ;j++) {
						_current_cache_page_oob[(ptr_oob_entry_idx[i].offset)+j] &= ptr_oob[idx];
						idx++;
					}
				}			
			} else {
				memcpy( &_current_cache_page_oob[0], &ptr_oob[0], oob_len);
			}
		}
		
		if(_spi_dma_mode == 0) {
			memcpy( &_current_cache_page[ptr_dev_info_t->page_size], &_current_cache_page_oob[0], ptr_dev_info_t->oob_size);
		} else {
			memcpy((u8 *)K0_TO_K1(&dma_write_page[ptr_dev_info_t->page_size]), &_current_cache_page_oob[0], ptr_dev_info_t->oob_size);
		}
	} else {
		if(data_len > 0) {	
			memcpy(&_current_cache_page_data[data_offset], &ptr_data[0], data_len);
		}
		
		if(spi_nfi_conf_t.auto_fdm_t== SPI_NFI_CON_AUTO_FDM_Disable) {	/* Data and oob alternate */
			if(oob_len > 0) {
				offset1 = 0;
				offset2 = 0;
				for(i = 0; i < spi_nfi_conf_t.sec_num; i++) {
					memcpy( &_current_cache_page_oob[offset1], &ptr_oob[offset2], spi_nfi_conf_t.spare_size_t);
					offset1 += spi_nfi_conf_t.spare_size_t;
					offset2 += spi_nfi_conf_t.spare_size_t;
				}
			}

			offset1 = 0;
			offset2 = 0;
			offset3 = 0;
			dma_sec_size = (spi_nfi_conf_t.page_size_t / spi_nfi_conf_t.sec_num);
			for(i = 0 ; i < spi_nfi_conf_t.sec_num; i++) {													
				memcpy((u8 *)K0_TO_K1(&dma_write_page[offset2]), &_current_cache_page_data[offset1],	dma_sec_size );
				memcpy((u8 *)K0_TO_K1(&dma_write_page[offset2+dma_sec_size]), &_current_cache_page_oob[offset3] , (spi_nfi_conf_t.spare_size_t) );
				offset1 += dma_sec_size;
				offset2 += (dma_sec_size+ (spi_nfi_conf_t.spare_size_t));
				offset3 += (spi_nfi_conf_t.spare_size_t);					
			}
		} else {  /* Data inside DRAM , oob inside NFI register */
			if(oob_len > 0) {
				memcpy( &_current_cache_page_oob_mapping[0], &ptr_oob[0], oob_len);
			}
			/* Set data */
			memcpy((u8 *)K0_TO_K1(&dma_write_page[0]), &_current_cache_page_data[0], ptr_dev_info_t->page_size);
		}
	}

	if(_spi_dma_mode == 1) {	
		mb();
	}
	
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spi_nand_write_page: page=0x%x, data_offset=0x%x, data_len=0x%x, oob_offset=0x%x, oob_len=0x%x\n", page_number, data_offset, data_len, oob_offset, oob_len);
	if(_spi_dma_mode == 0) {
		_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &_current_cache_page[0], ((ptr_dev_info_t->page_size) + (ptr_dev_info_t->oob_size)));
	} else {
		_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, K0_TO_K1(&dma_write_page[0]), ptr_dev_info_t->page_size + ptr_dev_info_t->oob_size);
	}

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spi_nand_write_page: _current_cache_page_data:\n");
	_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &_current_cache_page_data[0], ptr_dev_info_t->page_size);
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spi_nand_write_page: _current_cache_page_oob:\n");
	_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &_current_cache_page_oob[0], ptr_dev_info_t->oob_size);		
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spi_nand_write_page: _current_cache_page_oob_mapping:\n");
	_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &_current_cache_page_oob_mapping[0], (ptr_dev_info_t->oob_free_layout)->oobsize);

	if(ptr_dev_info_t->feature & SPI_NAND_FLASH_PLANE_SELECT_HAVE) {
		_plane_select_bit = ((page_number >> 6) & (0x1));

		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spi_nand_write_page: _plane_select_bit=0x%x\n", _plane_select_bit );
	}

	spi_nand_select_die ( page_number );

	/* Different Manafacture have different prgoram flow and setting */
	if(ptr_dev_info_t->write_en_type == SPI_NAND_FLASH_WRITE_LOAD_FIRST) {
		if(_spi_dma_mode == 1) {	
			rtn_status = spi_nand_dma_program_load(write_addr, oob_len, speed_mode);
			if(rtn_status != SPI_NAND_FLASH_RTN_NO_ERROR) {
				SPI_NAND_Flash_Clear_Read_Cache_Data();
				/* Switch to manual mode*/
				_SPI_NAND_ENABLE_MANUAL_MODE();
				return (rtn_status);
			}
		} else {
			spi_nand_protocol_program_load(write_addr, &_current_cache_page[0], ((ptr_dev_info_t->page_size) + (ptr_dev_info_t->oob_size)), speed_mode);
		}
					
		/*	Enable write_to flash */
		spi_nand_protocol_write_enable();					
	} else {
		/*	Enable write_to flash */
		spi_nand_protocol_write_enable();		
		
		if(_spi_dma_mode == 1) {	
			rtn_status = spi_nand_dma_program_load(write_addr, oob_len, speed_mode);
			if(rtn_status != SPI_NAND_FLASH_RTN_NO_ERROR) {
				SPI_NAND_Flash_Clear_Read_Cache_Data();
				/* Switch to manual mode*/
				_SPI_NAND_ENABLE_MANUAL_MODE();
				return (rtn_status);
			}
		} else {	
			/* Proram data into buffer of SPI NAND chip */
			spi_nand_protocol_program_load(write_addr, &_current_cache_page[0], ((ptr_dev_info_t->page_size) + (ptr_dev_info_t->oob_size)), speed_mode);
		}
	}

	/*	Execute program data into SPI NAND chip  */
	spi_nand_protocol_program_execute ( page_number );

	/*	Checking status for erase complete */
	do {
		spi_nand_protocol_get_feature(_SPI_NAND_ADDR_STATUS, &status);
	} while(status & _SPI_NAND_VAL_OIP) ;
				
	/*. Disable write_flash */
	spi_nand_protocol_write_disable();

	spi_nand_protocol_get_feature(_SPI_NAND_ADDR_PROTECTION, &status_2);

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spi_nand_write_page]: status 1 = 0x%x, status 3 =0x%x\n", status_2, status);

#if !(defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL))
	print_dot++;
	if((print_dot % 20) == 0 ) {
		_SPI_NAND_PRINTF(".");
	}
#endif

	/*	Check Program Fail Bit */
	if(status & _SPI_NAND_VAL_PROGRAM_FAIL) {
		_SPI_NAND_PRINTF("spi_nand_write_page : Program Fail at addr_offset =0x%x, page_number=0x%x, status=0x%x\n", data_offset, page_number, status);
		rtn_status = SPI_NAND_FLASH_RTN_PROGRAM_FAIL;
	}		

	SPI_NAND_Flash_Clear_Read_Cache_Data();
	
	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_write_internal( u32    dst_addr,
 *                                                                u32    len,
 *                                                                u32    *ptr_rtn_len,
 *                                                                u8*    ptr_buf      )
 * PURPOSE : To write flash internally.
 * AUTHOR  : 
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : dst_addr     - The dst_addr variable of this function.
 *           len          - The len variable of this function.
 *           ptr_buf      - A pointer to the ptr_buf variable.
 *   OUTPUT: ptr_rtn_len  - A pointer to the ptr_rtn_len variable.
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2014/12/19  - The first revision for this function.
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_write_internal( u32 dst_addr, 
													 u32 len, 
													 u32 *ptr_rtn_len, 
													 u8* ptr_buf, 
													 SPI_NAND_FLASH_WRITE_SPEED_MODE_T speed_mode )
{
	u32					 			remain_len, write_addr, data_len, page_number, physical_dst_addr;
	u32					 			addr_offset;
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T			rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	unsigned long					spinand_spinlock_flags;
		
#if	defined(TCSUPPORT_NAND_BMT) && !defined(LZMA_IMG) && !defined(BOOTROM_EXT)
    unsigned short	phy_block_bbt;
	unsigned long	addr_offset_in_block;
	u32				logical_block, physical_block;
	u8				oob_buf[_SPI_NAND_OOB_SIZE]={0xff};
#endif

	*ptr_rtn_len 	= 0;
	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;	
	remain_len 		= len;
	write_addr 		= dst_addr;
	
	_SPI_NAND_SEMAPHORE_LOCK();	

	SPI_NAND_Flash_Clear_Read_Cache_Data();

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_write_internal: remain_len =0x%x\n", remain_len);
	
	while(remain_len > 0)	
	{
		physical_dst_addr = write_addr;

#if	defined(TCSUPPORT_NAND_BMT) && !defined(LZMA_IMG) && !defined(BOOTROM_EXT)
		memset(oob_buf, 0xff, _SPI_NAND_OOB_SIZE);
		addr_offset_in_block = (write_addr %(ptr_dev_info_t->erase_size) );
		logical_block = (write_addr / (ptr_dev_info_t->erase_size));
		physical_block = get_mapping_block_index(logical_block, &phy_block_bbt);
		physical_dst_addr = (physical_block * (ptr_dev_info_t->erase_size))+ addr_offset_in_block;

		if(physical_block != logical_block) {			
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "Bad Block Mapping, from %d block to %d block\n", logical_block, physical_block);
		}
#endif
	
		/* Caculate page number */
		addr_offset = (physical_dst_addr % (ptr_dev_info_t->page_size));
		page_number = (physical_dst_addr / (ptr_dev_info_t->page_size));		
		
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "\nspi_nand_write_internal: addr_offset =0x%x, page_number=0x%x, remain_len=0x%x, page_size=0x%x\n", addr_offset, page_number, remain_len,(ptr_dev_info_t->page_size) );		
		if(((addr_offset + remain_len) > ptr_dev_info_t->page_size)) {  /* data cross over than 1 page range */
			data_len = ((ptr_dev_info_t->page_size) - addr_offset);			
		} else {
			data_len = remain_len;
		}

#if	defined(TCSUPPORT_NAND_BMT) && !defined(LZMA_IMG) && !defined(BOOTROM_EXT)
		if(block_is_in_bmt_region(physical_block)) {
			memcpy(oob_buf + OOB_INDEX_OFFSET, &phy_block_bbt, OOB_INDEX_SIZE);
		}

#if defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL)
		if(_SPI_NAND_WRITE_FAIL_TEST_FLAG == 0) {
#else
		if(test_write_fail_flag == 0) {
#endif
			rtn_status = spi_nand_write_page(page_number, addr_offset, &(ptr_buf[len - remain_len]), data_len, 0, &oob_buf[0], ptr_dev_info_t->oob_size , speed_mode);
		} else {
			rtn_status = SPI_NAND_FLASH_RTN_PROGRAM_FAIL;
		}

		if(rtn_status != SPI_NAND_FLASH_RTN_NO_ERROR) {
		    _SPI_NAND_PRINTF("write fail at page: %d \n", page_number);
            if(update_bmt(page_number * ptr_dev_info_t->page_size, UPDATE_WRITE_FAIL, &(ptr_buf[len - remain_len]), oob_buf)) {
                _SPI_NAND_PRINTF("Update BMT success\n");
				rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
           
            } else {
                _SPI_NAND_PRINTF("Update BMT fail\n");
				_SPI_NAND_SEMAPHORE_UNLOCK();	
                return -1;
            }		
		}
#else
		rtn_status = spi_nand_write_page(page_number, addr_offset, &(ptr_buf[len - remain_len]), data_len, 0, NULL, 0 , speed_mode);
#endif
		
		/* 8. Write remain data if neccessary */
		write_addr	+= data_len;
		remain_len  -= data_len;				
		ptr_rtn_len += data_len;			
	}
	
	_SPI_NAND_SEMAPHORE_UNLOCK();
		
	return (rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Write_Nbyte( u32    dst_addr,
 *                                                            u32    len,
 *                                                            u32    *ptr_rtn_len,
 *                                                            u8*    ptr_buf      )
 * PURPOSE : To provide interface for Write N Bytes into SPI NAND Flash.
 * AUTHOR  : 
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : dst_addr - The dst_addr variable of this function.
 *           len      - The len variable of this function.
 *           buf      - The buf variable of this function.
 *   OUTPUT: rtn_len  - The rtn_len variable of this function.
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2014/12/15  - The first revision for this function.
 *
 *------------------------------------------------------------------------------------
 */
SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Write_Nbyte( u32						dst_addr, 
												 u32								len, 
												 u32								*ptr_rtn_len, 
												 u8									*ptr_buf, 
												 SPI_NAND_FLASH_WRITE_SPEED_MODE_T	speed_node )
{		
	SPI_NAND_FLASH_RTN_T	rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;	
	
	rtn_status = spi_nand_write_internal(dst_addr, len, ptr_rtn_len, ptr_buf, speed_node);
	
	*ptr_rtn_len = len ;  /* , tmp modify */
	
	return (rtn_status);
}

int nandflash_write(unsigned long to, unsigned long len, u32 *retlen, unsigned char *buf)
{
	struct SPI_NAND_FLASH_INFO_T	 *ptr_dev_info_t;
	 
	ptr_dev_info_t  = _SPI_NAND_GET_DEVICE_INFO_PTR;
	
	if( SPI_NAND_Flash_Write_Nbyte(to, len, retlen, buf, ptr_dev_info_t->write_mode) == SPI_NAND_FLASH_RTN_NO_ERROR )
	{
		return 0;
	}
	else
	{
		return -1;
	}	
}
#endif

#if ERASE_AREA
#endif

#if !defined(LZMA_IMG) || defined(TCSUPPORT_BB_256KB)
/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_protocol_block_erase( u32   block_idx )
 * PURPOSE : To implement the SPI nand protocol for block erase.
 * AUTHOR  : 
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : block_idx - The block_idx variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2014/12/17  - The first revision for this function.
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_protocol_block_erase( u32 block_idx )
{
	SPI_NAND_FLASH_RTN_T	rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	
	/* 1. Chip Select Low */
	_SPI_NAND_READ_CHIP_SELECT_LOW();
	
	/* 2. Write op_cmd 0xD8 (Block Erase) */
	_SPI_NAND_WRITE_ONE_BYTE( _SPI_NAND_OP_BLOCK_ERASE );

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_protocol_block_erase : block idx =0x%x\n", block_idx);
	
	/* 3. Write block number */
	block_idx = block_idx << _SPI_NAND_BLOCK_ROW_ADDRESS_OFFSET; 	/*Row Address format in SPI NAND chip */
	
	_SPI_NAND_WRITE_ONE_BYTE( (block_idx >> 16) & 0xff );		/* dummy byte */
	_SPI_NAND_WRITE_ONE_BYTE( (block_idx >> 8)  & 0xff );
	_SPI_NAND_WRITE_ONE_BYTE(  block_idx & 0xff );
	
	/* 4. Chip Select High */
	_SPI_NAND_READ_CHIP_SELECT_HIGH();
			
	return (rtn_status);
}

SPI_NAND_FLASH_RTN_T spi_nand_erase_block(u32 block_index)
{
	u8								status;
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T			rtn_status	= SPI_NAND_FLASH_RTN_NO_ERROR;

#ifdef ERASE_WRITE_CNT_LOG
	if(erase_write_flag == SPI_NAND_FLASH_ERASE_WRITE_LOG_ENABLE) {
		b_erase_cnt[block_index]++;
		b_write_cnt_per_erase[block_index] = 0;
	}
#endif

	ptr_dev_info_t = _SPI_NAND_GET_DEVICE_INFO_PTR ;

	spi_nand_select_die (block_index * (ptr_dev_info_t->erase_size / ptr_dev_info_t->page_size));

	/* 2.2 Enable write_to flash */
	spi_nand_protocol_write_enable();		

	/* 2.3 Erasing one block */
	spi_nand_protocol_block_erase(block_index);

	/* 2.4 Checking status for erase complete */
	do {
		spi_nand_protocol_get_feature(_SPI_NAND_ADDR_STATUS, &status);
	} while(status & _SPI_NAND_VAL_OIP) ;
				
	/* 2.5 Disable write_flash */
	spi_nand_protocol_write_disable();

	/* 2.6 Check Erase Fail Bit */
	if(status & _SPI_NAND_VAL_ERASE_FAIL) {
		_SPI_NAND_PRINTF("spi_nand_erase_block : erase block fail, block=0x%x, status=0x%x\n", block_index, status);
		rtn_status = SPI_NAND_FLASH_RTN_ERASE_FAIL;
	}

	return rtn_status;
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_erase_internal( u32     addr,
 *                                               				  u32     len )
 * PURPOSE : To erase flash internally.
 * AUTHOR  : 
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : addr - The addr variable of this function.
 *           len - The size variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2014/12/15  - The first revision for this function.
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_erase_internal( u32 addr, 
													 u32 len )
{		
	u32						block_index = 0;
	u32						erase_len	= 0;
	SPI_NAND_FLASH_RTN_T	rtn_status  = SPI_NAND_FLASH_RTN_NO_ERROR;
	unsigned long			spinand_spinlock_flags;
				
#if	defined(TCSUPPORT_NAND_BMT) && !defined(LZMA_IMG) && !defined(BOOTROM_EXT)
    unsigned short	phy_block_bbt;
	u32				logical_block, physical_block;
#endif	
		
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "\nspi_nand_erase_internal (in): addr=0x%x, len=0x%x\n", addr, len );		
		
	_SPI_NAND_SEMAPHORE_LOCK();	
	
	/* Switch to manual mode*/
	_SPI_NAND_ENABLE_MANUAL_MODE();

	SPI_NAND_Flash_Clear_Read_Cache_Data();
	
	/* 1. Check the address and len must aligned to NAND Flash block size */
	if( spi_nand_block_aligned_check( addr, len) == SPI_NAND_FLASH_RTN_NO_ERROR) {
		/* 2. Erase block one by one */
		while( erase_len < len ) {
			/* 2.1 Caculate Block index */
			block_index = (addr/(_current_flash_info_t.erase_size));
#if	defined(TCSUPPORT_NAND_BMT) && !defined(LZMA_IMG) && !defined(BOOTROM_EXT)
			logical_block = block_index;
			physical_block = get_mapping_block_index(logical_block, &phy_block_bbt);		
			if( physical_block != logical_block) {			
				_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "Bad Block Mapping, from %d block to %d block\n", logical_block, physical_block);
			}
			block_index = physical_block;
#endif							
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_erase_internal: addr=0x%x, len=0x%x, block_idx=0x%x\n", addr, len, block_index );

#if defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL)
			if(_SPI_NAND_ERASE_FAIL_TEST_FLAG == 0) {
				rtn_status = spi_nand_erase_block(block_index);
			} else {
				rtn_status = SPI_NAND_FLASH_RTN_ERASE_FAIL;
			}
#else
			rtn_status = spi_nand_erase_block(block_index);
#endif

			/* 2.6 Check Erase Fail Bit */
			if(rtn_status != SPI_NAND_FLASH_RTN_NO_ERROR) {
#if	defined(TCSUPPORT_NAND_BMT) && !defined(LZMA_IMG) && !defined(BOOTROM_EXT)
				if (update_bmt((block_index * BLOCK_SIZE),UPDATE_ERASE_FAIL, NULL, NULL)) {
					_SPI_NAND_PRINTF("Erase fail at block: %d, update BMT success\n", addr/(_current_flash_info_t.erase_size));
					rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
				} else {
					_SPI_NAND_PRINTF("Erase fail at block: %d, update BMT fail\n", addr/(_current_flash_info_t.erase_size));
					rtn_status = SPI_NAND_FLASH_RTN_ERASE_FAIL;
					break;
				}
#else
				_SPI_NAND_PRINTF("spi_nand_erase_internal : Erase Fail at addr=0x%x, len=0x%x, block_idx=0x%x\n", addr, len, block_index);
				rtn_status = SPI_NAND_FLASH_RTN_ERASE_FAIL;
				break;
#endif
			}
			
			/* 2.7 Erase next block if needed */
			addr		+= _current_flash_info_t.erase_size;
			erase_len	+= _current_flash_info_t.erase_size;
		}		
	} else {
		rtn_status = SPI_NAND_FLASH_RTN_ALIGNED_CHECK_FAIL;
	}	
	
	_SPI_NAND_SEMAPHORE_UNLOCK();
		
	return 	(rtn_status);
}

/*------------------------------------------------------------------------------------
 * FUNCTION: SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Erase( u32  dst_addr,
 *                                                      u32  len      )
 * PURPOSE : To provide interface for Erase SPI NAND Flash.
 * AUTHOR  : 
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : dst_addr - The dst_addr variable of this function.
 *           len      - The len variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2014/12/17  - The first revision for this function.
 *
 *------------------------------------------------------------------------------------
 */
SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Erase(u32 dst_addr, u32 len)
{
	SPI_NAND_FLASH_RTN_T rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	
	rtn_status = spi_nand_erase_internal(dst_addr, len);
	
	return (rtn_status);
}

int nandflash_erase(unsigned long offset, unsigned long len)
{
	if(SPI_NAND_Flash_Erase(offset, len) == SPI_NAND_FLASH_RTN_NO_ERROR) {
		return 0;
	} else {
		return -1;
	}
}
#endif

#if	defined(TCSUPPORT_NAND_BMT)
#if (!defined(LZMA_IMG) && !defined(BOOTROM_EXT)) || defined(TCSUPPORT_BB_256KB)
int en7512_nand_exec_read_page(u32 page, u8* date, u8* oob)
{	
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T			rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	unsigned long					spinand_spinlock_flags;

	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;

	rtn_status = spi_nand_read_page(page, ptr_dev_info_t->read_mode);
	
	if(rtn_status == SPI_NAND_FLASH_RTN_NO_ERROR) {
		/* Get data segment and oob segment  */
		memcpy( date, &_current_cache_page_data[0], ptr_dev_info_t->page_size );
		memcpy( oob,  &_current_cache_page_oob_mapping[0], ptr_dev_info_t->oob_size );
		
		return 0;
	} else {
		 _SPI_NAND_PRINTF( "en7512_nand_exec_read_page: read error, page=0x%x\n", page);
		return -1;
	}
}
int en7512_nand_check_block_bad(u32 offset, u32 bmt_block)
{
	u32								page_number;
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	u8								bad_block_indicator[28];
	unsigned long					spinand_spinlock_flags;
	SPI_NAND_FLASH_RTN_T			rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	ptr_dev_info_t  = _SPI_NAND_GET_DEVICE_INFO_PTR;	

	if((0xbc000000 <= offset) && (offset <= 0xbfffffff)) {		/* Reserver address area for system */
		if( (offset & 0xbfc00000) == 0xbfc00000) {
			offset &= 0x003fffff;
		} else {
			offset &= 0x03ffffff;
		}
	}	 

	/* Caculate page number */
	page_number = (offset / (ptr_dev_info_t->page_size));		 

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "en7512_nand_check_block_bad: read_addr=0x%x, page_number=0x%x\n", offset, page_number);

	SPI_NAND_Flash_Clear_Read_Cache_Data();

	rtn_status = spi_nand_read_page(page_number, ptr_dev_info_t->read_mode);
	memcpy(bad_block_indicator, _current_cache_page_oob_mapping, 2);
#if 0 /* No matter what status of spi_nand_read_page, we must check badblock by oob. */
      /* Otherwise, the bmt pool size will be changed */
	if (rtn_status != SPI_NAND_FLASH_RTN_NO_ERROR) {
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "en7512_nand_check_block_bad return error, block:%d\n", offset/ptr_dev_info_t->erase_size);
		return 1;
	}
#endif
	if(bmt_block) {
		if(bad_block_indicator[BMT_BAD_BLOCK_INDEX_OFFSET] != 0xff) {
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "Bad block detected at page_addr 0x%x, oob_buf[%d] is 0x%x\n", page_number, BMT_BAD_BLOCK_INDEX_OFFSET,_current_cache_page_oob_mapping[BMT_BAD_BLOCK_INDEX_OFFSET]);
			return 1;
		}
	}
	else {
		if(bad_block_indicator[0] != 0xff) {
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "Bad block detected at page_addr 0x%x, oob_buf[0] is 0x%x\n", page_number, _current_cache_page_oob_mapping[0]);
			return 1;
		}
	}
#if 0
u8 def_bad_block_ECC[28];
#ifdef TCSUPPORT_SPI_CONTROLLER_ECC
	if(isSpiNandAndCtrlECC) {
		/* This is for check default bad block.
		 * When DMA read with all ECC parity equal to 0xFF,
		 * this will not generate read ECC error. So, it must 
		 * close DMA to check first OOB byte.
		 */
		_SPI_NAND_SEMAPHORE_LOCK();
		SPI_NAND_Flash_Set_DmaMode(0);
		SPI_NAND_Flash_Clear_Read_Cache_Data();
		rtn_status = spi_nand_read_page(page_number, ptr_dev_info_t->read_mode);
		SPI_NAND_Flash_Set_DmaMode(1);
		SPI_NAND_Flash_Clear_Read_Cache_Data();
		_SPI_NAND_SEMAPHORE_UNLOCK();
		
		if (rtn_status != SPI_NAND_FLASH_RTN_NO_ERROR) {
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "en7512_nand_check_block_bad return error, block:%d\n", offset/ptr_dev_info_t->erase_size);
			SPI_NAND_Flash_Set_DmaMode(1);
			_SPI_NAND_SEMAPHORE_UNLOCK();
			return 1;
		}
		SPI_NAND_Flash_Set_DmaMode(1);
		SPI_NAND_Flash_Clear_Read_Cache_Data();
		_SPI_NAND_SEMAPHORE_UNLOCK();

		memset(def_bad_block_ECC, 0xFF, sizeof(def_bad_block_ECC));
		SPI_NFI_Get_Configure(&spi_nfi_conf_t);

		if(memcmp(def_bad_block_ECC, &(bad_block_indicator[spi_nfi_conf_t.fdm_num]), (spi_nfi_conf_t.spare_size_t - spi_nfi_conf_t.fdm_num)) == 0) {
			if(bmt_block) {
				if(bad_block_indicator[BMT_BAD_BLOCK_INDEX_OFFSET] != 0xff) {
					_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "DMA closed, Bad block detected at page_addr 0x%x, oob_buf[%d] is 0x%x\n", page_number, BMT_BAD_BLOCK_INDEX_OFFSET,_current_cache_page_oob_mapping[BMT_BAD_BLOCK_INDEX_OFFSET]);
					return 1;
				}
			} else {
				if(bad_block_indicator[0] != 0xff) {
					_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "DMA closed, Bad block detected at page_addr 0x%x, oob_buf[0] is 0x%x\n", page_number, _current_cache_page_oob_mapping[0]);
					return 1;
				}
			}
		}
	}
#endif
#endif
	return 0;  /* Good Block*/
}

int en7512_nand_erase(u32 offset)
{
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T			rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	unsigned long					spinand_spinlock_flags;
	
	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;   

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "en7512_nand_erase: offset =0x%x, erase_size=0x%x\n", offset, (ptr_dev_info_t->erase_size));

	SPI_NAND_Flash_Clear_Read_Cache_Data();
	
	rtn_status = spi_nand_erase_block((offset / ptr_dev_info_t->erase_size));
	
	if(rtn_status == SPI_NAND_FLASH_RTN_NO_ERROR)
	{
		return 0;		
	}
	else
	{
		_SPI_NAND_PRINTF("en7512_nand_erase : Fail \n");
		return -1;
	}
}

int en7512_nand_mark_badblock(u32 offset, u32 bmt_block)
{
	u32 							page_number;
	u8 buf[8];
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T			rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	unsigned long					spinand_spinlock_flags;

	ptr_dev_info_t  = _SPI_NAND_GET_DEVICE_INFO_PTR;	

	/* Caculate page number */
	page_number = (((offset / BLOCK_SIZE) * BLOCK_SIZE) / PAGE_SIZE);	

	memset(buf, 0xFF, 8);
	if(bmt_block)
	{
		buf[BMT_BAD_BLOCK_INDEX_OFFSET] = 0;
	}
	else
	{
		buf[0] = 0;
	}

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "en7512_nand_mark_badblock: buf info:\n");
	_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &buf[0], 8);

	_SPI_NAND_PRINTF("en7512_nand_mark_badblock: page_num=0x%x\n", page_number);

	rtn_status = spi_nand_write_page(page_number, 0, NULL, 0, 0, &buf[0], 8, ptr_dev_info_t->write_mode);
	
	if( rtn_status == SPI_NAND_FLASH_RTN_NO_ERROR)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}
int en7512_nand_exec_write_page(u32 page, u8 *dat, u8 *oob)
{

	SPI_NAND_FLASH_RTN_T			rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	unsigned long					spinand_spinlock_flags;

	ptr_dev_info_t  = _SPI_NAND_GET_DEVICE_INFO_PTR;

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "en7512_nand_exec_write_page: page=0x%x\n", page);

	rtn_status = spi_nand_write_page(page, 0, dat, ptr_dev_info_t->page_size, 0, oob, ptr_dev_info_t->oob_size, ptr_dev_info_t->write_mode);

	if( rtn_status == SPI_NAND_FLASH_RTN_NO_ERROR)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

#if defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL)
int calc_bmt_pool_size(struct mtd_info *mtd)
#else
int calc_bmt_pool_size(struct ra_nand_chip *ra)
#endif
{
#if defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL)
	struct nand_chip *nand = mtd->priv;
    int chip_size = nand->chipsize;
    int block_size = 1 << nand->phys_erase_shift;
#else
    int chip_size = 1 << ra->flash->chip_shift;
    int block_size = 1 << ra->flash->erase_shift;
#endif
    int total_block = chip_size / block_size;
    int last_block = total_block - 1;
    u16 valid_block_num = 0;
    u16 need_valid_block_num = total_block * POOL_GOOD_BLOCK_PERCENT;

#if defined(TCSUPPORT_CT_PON)
	maximum_bmt_block_count = total_block * MAX_BMT_SIZE_PERCENTAGE_CT;
#else
	maximum_bmt_block_count = total_block * MAX_BMT_SIZE_PERCENTAGE;
#endif

	nand_flash_avalable_size = chip_size - (maximum_bmt_block_count * block_size);

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "\r\navalable block = %d\n", nand_flash_avalable_size / block_size);

    for(;last_block > 0; --last_block) {
        if(en7512_nand_check_block_bad((last_block*block_size), BAD_BLOCK_RAW)) {
            continue;  
        } else {
            valid_block_num++;
            if(valid_block_num == need_valid_block_num) {
                break;
            }
        }
    }

    return (total_block - last_block);   
}
#endif
#endif //defined(TCSUPPORT_NAND_BMT)

#if !(defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL))
#if !defined(BOOTROM_EXT)
#if defined(TCSUPPORT_NAND_BMT) && !defined(LZMA_IMG)
bmt_struct *get_g_bmt(void)
{
	return g_bmt;
}

init_bbt_struct *get_g_bbt(void)
{
	return g_bbt;
}

u32 get_maximum_bmt_block_count(void)
{
	return maximum_bmt_block_count;
}

void set_dma_write_addr(u8 *addr)
{
	dma_write_page = addr;
}

void reset_dma_write_addr()
{
	set_dma_write_addr(tmp_dma_write_page + (CACHE_LINE_SIZE - (((u32)tmp_dma_write_page) % CACHE_LINE_SIZE)));
}
#endif //#if defined(TCSUPPORT_NAND_BMT) && !defined(LZMA_IMG)

SPI_NAND_FLASH_RTN_T get_spi_nand_protocol_read_id (struct SPI_NAND_FLASH_INFO_T *ptr_rtn_flash_id)
{
	return spi_nand_protocol_read_id(ptr_rtn_flash_id);
}

SPI_NAND_FLASH_RTN_T get_spi_nand_protocol_read_id_2 (struct SPI_NAND_FLASH_INFO_T *ptr_rtn_flash_id)
{
	return spi_nand_protocol_read_id_2(ptr_rtn_flash_id);
}

void set_test_write_fail_flag(u32 val)
{
	test_write_fail_flag = val;
}

void set_dma_read_addr(u8 *addr)
{
	dma_read_page = addr;
}

void reset_dma_read_addr()
{
	set_dma_read_addr(tmp_dma_read_page + (CACHE_LINE_SIZE - (((u32)tmp_dma_read_page) % CACHE_LINE_SIZE)));
}

SPI_NAND_FLASH_RTN_T set_spi_nand_load_page_into_cache(u32 page_number)
{
	return spi_nand_load_page_into_cache(page_number);
}
#endif //#if !defined(BOOTROM_EXT)
#endif //#if !(defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL))

#if init_AREA
#endif

/*------------------------------------------------------------------------------------
 * FUNCTION: static void spi_nand_manufacute_init( struct SPI_NAND_FLASH_INFO_T *ptr_device_t )
 * PURPOSE : To init SPI NAND Flash chip
 * AUTHOR  : 
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : None
 *   OUTPUT: None.
 * RETURN  : None.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2015/05/15  - The first revision for this function.
 *
 *------------------------------------------------------------------------------------
 */
static void spi_nand_manufacute_init(struct SPI_NAND_FLASH_INFO_T *ptr_device_t)
{
	unsigned char	feature;
	unsigned char	die;

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1,"spi_nand_manufacute_init: Unlock all block and Enable Quad Mode\n"); 

	for(die = 0; die < ptr_device_t->die_num; die++) {
		spi_nand_direct_select_die(die);

		if(((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_WINBOND) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_W25N01GV)) ||
		   ((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_WINBOND) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_W25M02GV)))
		{
			/* Enable to modify the status regsiter 1 */
			feature = 0x58;
			spi_nand_protocol_set_feature(_SPI_NAND_ADDR_FEATURE, feature);
		}

		/* 1. Unlock All block */
		if(ptr_device_t->unlock_block_info.unlock_block_mask != 0x00) {	
			spi_nand_protocol_get_feature(_SPI_NAND_ADDR_PROTECTION, &feature);
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "Before Unlock all block setup, the status register1 = 0x%x\n", feature);
			
			feature = (feature & ~(ptr_device_t->unlock_block_info.unlock_block_mask));
			feature |= (ptr_device_t->unlock_block_info.unlock_block_value & ptr_device_t->unlock_block_info.unlock_block_mask);
			spi_nand_protocol_set_feature(_SPI_NAND_ADDR_PROTECTION, feature);
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "Unlock all block setup, the feature = 0x%x\n", feature);

			spi_nand_protocol_get_feature(_SPI_NAND_ADDR_PROTECTION, &feature);
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "After Unlock all block setup, the status register1 = 0x%x\n", feature);
		} else {
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "No need Unlock all block setup.\n");
		}

		/* 2. Enable Qual mode */
		if((ptr_device_t->quad_en.quad_en_mask != 0x00) &&
		   ((ptr_device_t->read_mode == SPI_NAND_FLASH_READ_SPEED_MODE_QUAD) || (ptr_device_t->write_mode == SPI_NAND_FLASH_WRITE_SPEED_MODE_QUAD))) {
			spi_nand_protocol_get_feature(_SPI_NAND_ADDR_FEATURE, &feature);
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "Before enable qual mode setup, the status register2 =0x%x\n", feature);

			feature = (feature & ~(ptr_device_t->quad_en.quad_en_mask));
			feature |= (ptr_device_t->quad_en.quad_en_value & ptr_device_t->quad_en.quad_en_mask);
			spi_nand_protocol_set_feature(_SPI_NAND_ADDR_FEATURE, feature);
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "enable qual mode setup, the feature = 0x%x\n", feature);

			spi_nand_protocol_get_feature(_SPI_NAND_ADDR_FEATURE, &feature);
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "After enable qual mode setup, the status register2 =0x%x\n", feature);
		} else {
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "No need enable qual mode setup.\n");
		}

		if(((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_WINBOND) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_W25N01GV)) ||
		   ((ptr_device_t->mfr_id == _SPI_NAND_MANUFACTURER_ID_WINBOND) && (ptr_device_t->dev_id == _SPI_NAND_DEVICE_ID_W25M02GV)))
		{
			/* Disable to modify the status regsiter 1 */
			feature = 0x18;
			spi_nand_protocol_set_feature(_SPI_NAND_ADDR_FEATURE, feature);
		}
	}
}

/*------------------------------------------------------------------------------------
 * FUNCTION: static SPI_NAND_FLASH_RTN_T spi_nand_probe( struct SPI_NAND_FLASH_INFO_T  *ptr_rtn_device_t )
 * PURPOSE : To probe SPI NAND flash id.
 * AUTHOR  : 
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : None
 *   OUTPUT: rtn_index  - The rtn_index variable of this function.
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2014/12/12  - The first revision for this function.
 *
 *------------------------------------------------------------------------------------
 */
static SPI_NAND_FLASH_RTN_T spi_nand_probe( struct SPI_NAND_FLASH_INFO_T *ptr_rtn_device_t )
{
	SPI_NAND_FLASH_RTN_T	rtn_status = SPI_NAND_FLASH_RTN_PROBE_ERROR;
	unsigned long			spinand_spinlock_flags;

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_probe: start \n");

	/* Protocol for read id */
	_SPI_NAND_SEMAPHORE_LOCK();	
	spi_nand_protocol_read_id(ptr_rtn_device_t );
	_SPI_NAND_SEMAPHORE_UNLOCK();	

	rtn_status = scan_spi_nand_table(ptr_rtn_device_t);

	if ( rtn_status != SPI_NAND_FLASH_RTN_NO_ERROR )
	{
		/* Another protocol for read id  (For example, the GigaDevice SPI NADN chip for Type C */
		_SPI_NAND_SEMAPHORE_LOCK();	
		spi_nand_protocol_read_id_2( (struct SPI_NAND_FLASH_INFO_T *)ptr_rtn_device_t );
		_SPI_NAND_SEMAPHORE_UNLOCK();	

		rtn_status = scan_spi_nand_table(ptr_rtn_device_t);
	}

	_SPI_NAND_PRINTF("spi_nand_probe: mfr_id=0x%x, dev_id=0x%x\n", ptr_rtn_device_t->mfr_id, ptr_rtn_device_t->dev_id);

	if(rtn_status == SPI_NAND_FLASH_RTN_NO_ERROR)
	{
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_probe: device size:0x%x \n", ptr_rtn_device_t->device_size);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_probe: erase size :0x%x \n", ptr_rtn_device_t->erase_size);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_probe: page size  :0x%x \n", ptr_rtn_device_t->page_size);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_probe: oob size   :%d \n", ptr_rtn_device_t->oob_size);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_probe: dummy mode :%d \n", ptr_rtn_device_t->dummy_mode);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_probe: read mode  :%d \n", ptr_rtn_device_t->read_mode);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_probe: write mode :%d \n", ptr_rtn_device_t->write_mode);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_probe: feature    :0x%x \n", ptr_rtn_device_t->feature);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_probe: die num    :%d \n", ptr_rtn_device_t->die_num);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_probe: write type    :%d \n", ptr_rtn_device_t->write_en_type);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_probe: ecc check info mask     :%x \n", ptr_rtn_device_t->ecc_fail_check_info.ecc_check_mask);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_probe: ecc check info value    :%x \n", ptr_rtn_device_t->ecc_fail_check_info.ecc_uncorrected_value);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_probe: unlock block info mask  :%x \n", ptr_rtn_device_t->unlock_block_info.unlock_block_mask);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_probe: unlock block info value :%x \n", ptr_rtn_device_t->unlock_block_info.unlock_block_value);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_probe: quad en info mask       :%x \n", ptr_rtn_device_t->quad_en.quad_en_mask);
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_probe: quad en info value      :%x \n", ptr_rtn_device_t->quad_en.quad_en_value);
		
		_SPI_NAND_SEMAPHORE_LOCK();
		spi_nand_manufacute_init(ptr_rtn_device_t);		
		_SPI_NAND_SEMAPHORE_UNLOCK();	

		if((ptr_rtn_device_t->write_mode == SPI_NAND_FLASH_WRITE_SPEED_MODE_QUAD) || 
			(ptr_rtn_device_t->read_mode == SPI_NAND_FLASH_READ_SPEED_MODE_QUAD)) {
			enable_quad();
		}
	}	
	
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spi_nand_probe: end \n");
	
	return (rtn_status);
}

int enable_dma(SPI_NFI_CONF_T *spi_nfi_conf_t,
				SPI_ECC_ENCODE_CONF_T *encode_conf_t,
				SPI_ECC_DECODE_CONF_T *decode_conf_t)
{
	unsigned long spinand_spinlock_flags;
	int i;
	
	_SPI_NAND_SEMAPHORE_LOCK();

	if(isSpiNandAndCtrlECC) {
		_current_flash_info_t.oob_free_layout = &ooblayout_spi_controller_ecc;
		_current_flash_info_t.oob_free_layout->oobsize = spi_nfi_conf_t->sec_num * 8;
		
		for(i = 0; i < spi_nfi_conf_t->sec_num; i++) {
			_current_flash_info_t.oob_free_layout->oobfree[i].offset = i * spi_nfi_conf_t->spare_size_t;
			_current_flash_info_t.oob_free_layout->oobfree[i].len= 8;
		}
	}

	/* Set controller to DMA mode */
	SPI_NAND_Flash_Set_DmaMode(1);

	/* Init DMA */
	SPI_NFI_Init();

	/* config DMA */
	SPI_NFI_Set_Configure(spi_nfi_conf_t);

	/* config ECC */
	if(spi_nfi_conf_t->hw_ecc_t == SPI_NFI_CON_HW_ECC_Enable) {
		if(isEN751627|| isEN7580) {
			SPI_NFI_ECC_DATA_SOURCE_INV(SPI_NFI_CONF_ECC_DATA_SOURCE_INV_ENABLE);
		}
		
		/* Init Decode, Encode */
#if !defined(LZMA_IMG) || defined(TCSUPPORT_BB_256KB)
		SPI_ECC_Encode_Init();
#endif
		SPI_ECC_Decode_Init();
			
		/* Setup Encode */
#if !defined(LZMA_IMG) || defined(TCSUPPORT_BB_256KB)
		SPI_ECC_Encode_Set_Configure(encode_conf_t);
#endif	
		/* Setup Decode */
		SPI_ECC_Decode_Set_Configure(decode_conf_t);
	}

	_SPI_NAND_SEMAPHORE_UNLOCK();
	return 0;
}

#if 0
int spinand_ctrlEcc_calibration(SPI_NFI_CONF_T	*spi_nfi_conf_t, 
						 SPI_ECC_ENCODE_CONF_T	*encode_conf_t, 
						 SPI_ECC_DECODE_CONF_T	*decode_conf_t)
{
	unsigned long auto_sizing_magic;
	int auto_ecc_detect_idx;
	int auto_spare_size_idx;
	SPI_NAND_FLASH_DEBUG_LEVEL_T dbg_lv = SPI_NAND_FLASH_DEBUG_LEVEL_1;
	SPI_NAND_FLASH_RTN_T status;
	u32 ecc_ability_array[] = {SPI_ECC_DECODE_ABILITY_4BITS, SPI_ECC_DECODE_ABILITY_6BITS, SPI_ECC_DECODE_ABILITY_8BITS, SPI_ECC_DECODE_ABILITY_10BITS,
							   SPI_ECC_DECODE_ABILITY_12BITS, SPI_ECC_DECODE_ABILITY_14BITS, SPI_ECC_DECODE_ABILITY_16BITS};
	u32 spare_size_array[] = {SPI_NFI_CONF_SPARE_SIZE_16BYTE, SPI_NFI_CONF_SPARE_SIZE_26BYTE, SPI_NFI_CONF_SPARE_SIZE_27BYTE, SPI_NFI_CONF_SPARE_SIZE_28BYTE};
	
	/* auto-detecting ECC ability */
	for(auto_ecc_detect_idx = 0; auto_ecc_detect_idx < (sizeof(ecc_ability_array) / sizeof(ecc_ability_array[0])); auto_ecc_detect_idx++) {
		for(auto_spare_size_idx = 0; auto_spare_size_idx < (sizeof(spare_size_array) / sizeof(spare_size_array[0])); auto_spare_size_idx++) {
			/* Check spare area is enough to store parity */
			if(((spare_size_array[auto_spare_size_idx] - spi_nfi_conf_t->fdm_num) * 8) < (13 * ecc_ability_array[auto_ecc_detect_idx])) {
				_SPI_NAND_DEBUG_PRINTF(dbg_lv, "skip\n");
				_SPI_NAND_DEBUG_PRINTF(dbg_lv, "  ecc  :%02d\n", ecc_ability_array[auto_ecc_detect_idx]);
				_SPI_NAND_DEBUG_PRINTF(dbg_lv, "  spare:%02d\n", spare_size_array[auto_spare_size_idx]);
				continue;
			}

			/* Enable Manual Mode */
			_SPI_NAND_ENABLE_MANUAL_MODE();	

			/* set NFI */
			spi_nfi_conf_t->spare_size_t 		= spare_size_array[auto_spare_size_idx];

			/* Setup Encode */
#if !defined(LZMA_IMG)
			encode_conf_t->encode_ecc_abiliry	= SPI_ECC_ENCODE_ABILITY_4BITS;
#endif

			/* Setup Decode */
			decode_conf_t->decode_ecc_abiliry	= ecc_ability_array[auto_ecc_detect_idx];
			decode_conf_t->decode_block_size 	= (((spi_nfi_conf_t->fdm_ecc_num) + 512) * 8) + ((decode_conf_t->decode_ecc_abiliry) * 13);

			_SPI_NAND_DEBUG_PRINTF(dbg_lv, "testing...\n");
			_SPI_NAND_DEBUG_PRINTF(dbg_lv, "  ecc  :%02d\n", decode_conf_t->decode_ecc_abiliry);
			_SPI_NAND_DEBUG_PRINTF(dbg_lv, "  spare:%02d\n", spi_nfi_conf_t->spare_size_t);

			/* enable DMA */
			enable_dma(spi_nfi_conf_t, encode_conf_t, decode_conf_t);

			SPI_NAND_Flash_Clear_Read_Cache_Data();
			auto_sizing_magic = SPI_NAND_Flash_Read_DWord(PAGE_SIZE_MAGIC_FLASH_ADDR, &status);

			if(status == SPI_NAND_FLASH_RTN_NO_ERROR ) {
				_SPI_NAND_DEBUG_PRINTF(dbg_lv, "magic:0x%x\n", auto_sizing_magic);

				if(auto_sizing_magic == PAGE_SIZE_MAGIC) {
					_SPI_NAND_DEBUG_PRINTF(dbg_lv, "detected:\n");
					_SPI_NAND_DEBUG_PRINTF(dbg_lv, "  ecc  :%02d\n", decode_conf_t->decode_ecc_abiliry);
					_SPI_NAND_DEBUG_PRINTF(dbg_lv, "  spare:%02d\n", spi_nfi_conf_t->spare_size_t);
					return 0;
				}
			}
		} //for auto_spare_size_idx
	} //for auto_ecc_detect_idx

	/* can not find magic number, use default value */
	/* set ECC ability = 4bits */
	/* set spare size = 16B */

	/* set NFI */
	spi_nfi_conf_t->spare_size_t 		= SPI_NFI_CONF_SPARE_SIZE_16BYTE;

	/* Setup Encode */
#if !defined(LZMA_IMG)
	encode_conf_t->encode_ecc_abiliry	= SPI_ECC_ENCODE_ABILITY_4BITS;
#endif
	
	/* Setup Decode */
	decode_conf_t->decode_ecc_abiliry	= SPI_ECC_DECODE_ABILITY_4BITS;
	
	return -1;
}
#endif

#if defined(TCSUPPORT_NAND_BMT)
#if (!defined(LZMA_IMG) && !defined(BOOTROM_EXT)) || defined(TCSUPPORT_BB_256KB)
#if defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL)
int init_bmt_bbt(struct mtd_info *mtd)
#else
int init_bmt_bbt(struct ra_nand_chip *ra)
#endif
{
#if defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL)
	bmt_pool_size = calc_bmt_pool_size(mtd);
#else
	bmt_pool_size = calc_bmt_pool_size(ra);
#endif

	if(bmt_pool_size > maximum_bmt_block_count) {
		_SPI_NAND_PRINTF("Error : bmt pool size: %d > maximum size %d\n", bmt_pool_size, maximum_bmt_block_count);
		_SPI_NAND_PRINTF("Error: init bmt failed \n");
		return -1;
	}

	if(bmt_pool_size > MAX_BMT_SIZE) {
		bmt_pool_size = MAX_BMT_SIZE;
	}

	_SPI_NAND_PRINTF("bmt pool size: %d \n", bmt_pool_size);
	
	if(!g_bmt) {
#if defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL)
		if (!(g_bmt = init_bmt(mtd, bmt_pool_size)))
#else
		if (!(g_bmt = init_bmt(ra, bmt_pool_size)))
#endif
		{
			_SPI_NAND_PRINTF("Error: init bmt failed \n");
			return -1;
		}
	}

	if(!g_bbt) {
		if (!(g_bbt = start_init_bbt())) {
			_SPI_NAND_PRINTF("Error: init bbt failed \n");
			return -1;
		}
	}

	if(write_bbt_or_bmt_to_flash() != 0) {				
		_SPI_NAND_PRINTF("Error: save bbt or bmt to nand failed \n");
		return -1;
	}
	
	if(create_badblock_table_by_bbt()) {
		_SPI_NAND_PRINTF("Error: create bad block table failed \n");
		return -1;
	}

	_SPI_NAND_PRINTF("BMT & BBT Init Success \n");

	return 0;
}
#endif
#endif

/*------------------------------------------------------------------------------------
 * FUNCTION: SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Init( long  rom_base )
 * PURPOSE : To provide interface for SPI NAND init.
 * AUTHOR  : 
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : rom_base - The rom_base variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2014/12/12  - The first revision for this function.
 *
 *------------------------------------------------------------------------------------
 */
SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Init(u32 rom_base)
{
	SPI_NFI_CONF_T			spi_nfi_conf_t;
	SPI_ECC_ENCODE_CONF_T	encode_conf_t;
	SPI_ECC_DECODE_CONF_T	decode_conf_t;
	int						sec_num;
	unsigned long 			spi_nand_info;
	SPI_ECC_ENCODE_ABILITY_T	encode_ecc_abiliry;
	SPI_ECC_DECODE_ABILITY_T	decode_ecc_abiliry;
	SPI_NFI_CONF_SPARE_SIZE_T   spare_size_t;
	
	SPI_NAND_FLASH_RTN_T	rtn_status = SPI_NAND_FLASH_RTN_PROBE_ERROR;	

	/* 1. set SFC Clock to 50MHZ  */
	spi_nand_set_clock_speed(50);
	
	/* 2. Enable Manual Mode */
	_SPI_NAND_ENABLE_MANUAL_MODE();	

 	/* 3. Probe flash information */
	if ( spi_nand_probe(  &_current_flash_info_t) != SPI_NAND_FLASH_RTN_NO_ERROR ) {
		_SPI_NAND_PRINTF("SPI NAND Flash Detected Error !\n");
	} else {
#ifdef ERASE_WRITE_CNT_LOG
		/* init */
		memset(b_erase_cnt, 0, sizeof(b_erase_cnt));
		memset(b_write_total_cnt, 0, sizeof(b_write_total_cnt));
		memset(b_write_cnt_per_erase, 0, sizeof(b_write_cnt_per_erase));
		erase_write_flag = SPI_NAND_FLASH_ERASE_WRITE_LOG_DISABLE;
#endif

		/* for 32bytes alignment */
		if(dma_read_page == NULL) {
			dma_read_page = tmp_dma_read_page + (CACHE_LINE_SIZE - (((u32)tmp_dma_read_page) % CACHE_LINE_SIZE));
			/* flush cache_page */
#if defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL)
			dma_cache_inv((unsigned long)dma_read_page, _SPI_NAND_CACHE_SIZE);
#else
			flush_dcache_range((unsigned long)dma_read_page, (unsigned long)(dma_read_page + _SPI_NAND_CACHE_SIZE));
#endif
		}
		
		/* for 32bytes alignment */
#if	!defined(LZMA_IMG) || defined(TCSUPPORT_BB_256KB)
		if(dma_write_page == NULL) {
			dma_write_page = tmp_dma_write_page + (CACHE_LINE_SIZE - (((u32)tmp_dma_write_page) % CACHE_LINE_SIZE));
			/* flush cache_page */
#if defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL)
			dma_cache_inv((unsigned long)dma_write_page, _SPI_NAND_CACHE_SIZE);
#else
			flush_dcache_range((unsigned long)dma_write_page, (unsigned long)(dma_write_page + _SPI_NAND_CACHE_SIZE));
#endif
		}
#endif

		if(_current_flash_info_t.page_size == _SPI_NAND_PAGE_SIZE_2KBYTE) {
			sec_num = 4;
		} else if(_current_flash_info_t.page_size == _SPI_NAND_PAGE_SIZE_4KBYTE) {
			sec_num = 8;
		} else {
			sec_num = 1;
		}

		if(isSpiNandAndCtrlECC) {
			/* Disable OnDie ECC */
			SPI_NAND_Flash_Disable_OnDie_ECC();

			if(!isEN7526c) {
				/* en7516, en7580... */
				spi_nand_info		= VPint(SPI_NAND_CONTROLLER_ECC_INFO_SRAM_ADDR_INFO);
				spare_size_t		= (spi_nand_info & 0xFFFF);
				encode_ecc_abiliry	= ((spi_nand_info >> 16) & 0xFFFF);
				decode_ecc_abiliry	= ((spi_nand_info >> 16) & 0xFFFF);
			} else {
				/* en7526c */
				spare_size_t		= SPI_NFI_CONF_SPARE_SIZE_16BYTE;
				encode_ecc_abiliry	= SPI_ECC_ENCODE_ABILITY_4BITS;
				decode_ecc_abiliry	= SPI_ECC_DECODE_ABILITY_4BITS;
			}

			_SPI_NAND_PRINTF("Using SPI controller ECC.\n");
			_SPI_NAND_PRINTF("  ECC ability:%d\n", encode_ecc_abiliry);
			_SPI_NAND_PRINTF("  NFI spare area size:%d\n", spare_size_t);

			/* Setup NFI */
			spi_nfi_conf_t.auto_fdm_t			= SPI_NFI_CON_AUTO_FDM_Enable;
			spi_nfi_conf_t.hw_ecc_t 			= SPI_NFI_CON_HW_ECC_Enable;
			spi_nfi_conf_t.dma_burst_t			= SPI_NFI_CON_DMA_BURST_Enable;
			spi_nfi_conf_t.fdm_num				= 8;
			spi_nfi_conf_t.fdm_ecc_num			= 8;
			spi_nfi_conf_t.page_size_t			= _current_flash_info_t.page_size;
			spi_nfi_conf_t.sec_num				= sec_num;
			spi_nfi_conf_t.cus_sec_size_en_t	= SPI_NFI_CONF_CUS_SEC_SIZE_Disable;
			spi_nfi_conf_t.sec_size 			= 0;
			spi_nfi_conf_t.spare_size_t 		= spare_size_t;

			/* Setup Encode */
#if !defined(LZMA_IMG) || defined(TCSUPPORT_BB_256KB)
			encode_conf_t.encode_en 			= SPI_ECC_ENCODE_ENABLE;
			encode_conf_t.encode_ecc_abiliry	= encode_ecc_abiliry;
			encode_conf_t.encode_block_size 	= 512 + spi_nfi_conf_t.fdm_ecc_num;
#endif
					
			/* Setup Decode */
			decode_conf_t.decode_en 			= SPI_ECC_DECODE_ENABLE;
			decode_conf_t.decode_ecc_abiliry	= decode_ecc_abiliry;
			decode_conf_t.decode_block_size 	= (((spi_nfi_conf_t.fdm_ecc_num) + 512) * 8) + ((decode_conf_t.decode_ecc_abiliry) * 13);

			/* enable DMA */
			enable_dma(&spi_nfi_conf_t, &encode_conf_t, &decode_conf_t);
		} else {
			_SPI_NAND_PRINTF("Using Flash ECC.\n");
			SPI_NAND_Flash_Enable_OnDie_ECC();
#ifdef TCSUPPORT_SPI_NAND_FLASH_ECC_DMA
			if(isEN7526c || isEN751627|| isEN7580) {
				/* Setup NFI */
				spi_nfi_conf_t.auto_fdm_t			= SPI_NFI_CON_AUTO_FDM_Disable;
				spi_nfi_conf_t.hw_ecc_t 			= SPI_NFI_CON_HW_ECC_Disable;
				spi_nfi_conf_t.dma_burst_t			= SPI_NFI_CON_DMA_BURST_Enable;
				spi_nfi_conf_t.spare_size_t 		= SPI_NFI_CONF_SPARE_SIZE_16BYTE;
				spi_nfi_conf_t.page_size_t			= _current_flash_info_t.page_size;
				spi_nfi_conf_t.sec_num				= sec_num;
				spi_nfi_conf_t.cus_sec_size_en_t	= SPI_NFI_CONF_CUS_SEC_SIZE_Enable;
				spi_nfi_conf_t.sec_size 			= (_current_flash_info_t.page_size + _current_flash_info_t.oob_size) / sec_num;

				/* Setup Encode */
#if !defined(LZMA_IMG) || defined(TCSUPPORT_BB_256KB)
				encode_conf_t.encode_en 			= SPI_ECC_ENCODE_DISABLE;
#endif
						
				/* Setup Decode */
				decode_conf_t.decode_en 			= SPI_ECC_DECODE_DISABLE;

				/* enable DMA */
				enable_dma(&spi_nfi_conf_t, &encode_conf_t, &decode_conf_t);
			}
#endif
		}

		_SPI_NAND_PRINTF("Detected SPI NAND Flash : %s, Flash Size=0x%x\n", _current_flash_info_t.ptr_name,  _current_flash_info_t.device_size);

		rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
		
#if !(defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL))
#if !defined(BOOTROM_EXT) || defined(TCSUPPORT_BB_256KB)
		devinfo.blocksize = (_current_flash_info_t.erase_size)/1024;
		devinfo.totalsize = ((_current_flash_info_t.device_size)/(1024*1024));

		/*  For bootloader to caculate flash size information */
		memset((void*) &ra, 0, sizeof(struct ra_nand_chip));
		memset((void*) &flashInfo, 0, sizeof(struct nand_info));
		
		flashInfo.chip_shift = generic_ffs(_current_flash_info_t.device_size) - 1;
		flashInfo.erase_shift = generic_ffs(_current_flash_info_t.erase_size) - 1;
		flashInfo.page_shift = generic_ffs(_current_flash_info_t.page_size) - 1;
		flashInfo.oob_shift = generic_ffs(MAX_LINUX_USE_OOB_SIZE) - 1;
		ra.flash = &flashInfo;					

#if defined(TCSUPPORT_CT_PON)
reservearea_size = 0x40000;
#else
reservearea_size = _current_flash_info_t.erase_size;
#endif

#if	defined(TCSUPPORT_NAND_BMT)
#if (!defined(LZMA_IMG) && !defined(BOOTROM_EXT)) || defined(TCSUPPORT_BB_256KB)
		if(init_bmt_bbt(&ra) == -1) {
			return -1;
		}
#endif
#endif //#if defined(TCSUPPORT_NAND_BMT) && !defined(LZMA_IMG) && !defined(BOOTROM_EXT)
#endif //#if !(defined(BOOTROM_EXT) && (defined(TCSUPPORT_CPU_EN7516)||defined(TCSUPPORT_CPU_EN7527)))
#endif //
	}

   return (rtn_status);
}

int nandflash_init(int rom_base)
{	
	if(SPI_NAND_Flash_Init(rom_base) == SPI_NAND_FLASH_RTN_NO_ERROR) {
		return 0;
	} else {
		return -1;
	}
}

/***********************************************************************************/
/***********************************************************************************/
/***** Modify for SPI NAND linux kernel driver below *******************************/
/***********************************************************************************/
/***********************************************************************************/
/***********************************************************************************/
#if defined(TCSUPPORT_2_6_36_KERNEL) || defined(TCSUPPORT_3_18_21_KERNEL)

/* feature/ status reg */
#define REG_BLOCK_LOCK                  0xa0
#define REG_OTP                         0xb0
#define REG_STATUS                      0xc0/* timing */

/* status */
#define STATUS_OIP_MASK                 0x01
#define STATUS_READY                    (0 << 0)
#define STATUS_BUSY                     (1 << 0)

#define STATUS_E_FAIL_MASK              0x04
#define STATUS_E_FAIL                   (1 << 2)

#define STATUS_P_FAIL_MASK              0x08
#define STATUS_P_FAIL                   (1 << 3)

#define STATUS_ECC_MASK                 0x30
#define STATUS_ECC_1BIT_CORRECTED       (1 << 4)
#define STATUS_ECC_ERROR                (2 << 4)
#define STATUS_ECC_RESERVED             (3 << 4)

/*ECC enable defines*/
#define OTP_ECC_MASK                    0x10
#define OTP_ECC_OFF                     0
#define OTP_ECC_ON                      1

#define ECC_DISABLED
#define ECC_IN_NAND
#define ECC_SOFT

#define SPI_NAND_PROCNAME				"driver/spi_nand_debug"
#define SPI_NAND_TEST					"driver/spi_nand_test"

#define BUFSIZE (2 * 2048)

#define CONFIG_MTD_SPINAND_ONDIEECC		1

#define MAX_WAIT_JIFFIES  (40 * HZ)

#ifdef CONFIG_MTD_SPINAND_ONDIEECC
static struct nand_ecclayout spinand_oob_64 = {
        .eccbytes = 0,
        .eccpos = {},
        .oobavail = MAX_LINUX_USE_OOB_SIZE,
        .oobfree = {
                {.offset = 0, 
				 .length = MAX_LINUX_USE_OOB_SIZE}
        }
};
#endif


/* NAND driver */
int ra_nand_init(void)
{
	return 0;
}

void ra_nand_remove(void)
{

}

static inline struct spinand_state *mtd_to_state(struct mtd_info *mtd)
{
    struct nand_chip *chip = (struct nand_chip *)mtd->priv;
    struct spinand_info *info = (struct spinand_info *)chip->priv;
    struct spinand_state *state = (struct spinand_state *)info->priv;

    return state;
}

/*
 * spinand_read_id- Read SPI Nand ID
 * Description:
 *    Read ID: read two ID bytes from the SPI Nand device
 */
static int spinand_read_id(struct spi_device *spi_nand, u8 *id)
{
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	ptr_dev_info_t		= 	_SPI_NAND_GET_DEVICE_INFO_PTR;	

	id[0] = ptr_dev_info_t->mfr_id;
	id[1] = ptr_dev_info_t->dev_id;
	
	return 0;
}

/*
 * spinand_read_status- send command 0xf to the SPI Nand status register
 * Description:
 *    After read, write, or erase, the Nand device is expected to set the
 *    busy status.
 *    This function is to allow reading the status of the command: read,
 *    write, and erase.
 *    Once the status turns to be ready, the other status bits also are
 *    valid status bits.
 */
static int spinand_read_status(struct spi_device *spi_nand, uint8_t *status)
{
	return spi_nand_protocol_get_feature(_SPI_NAND_ADDR_STATUS, status);
}

static int wait_till_ready(struct spi_device *spi_nand)
{
	unsigned long deadline;
	int retval;
	u8 stat = 0;

	deadline = jiffies + MAX_WAIT_JIFFIES;
	do {
		retval = spinand_read_status(spi_nand, &stat);
		if (retval < 0)
			return -1;
		else if (!(stat & 0x1))
			break;

		cond_resched();
	} while (!time_after_eq(jiffies, deadline));

	if ((stat & 0x1) == 0)
		return 0;

	return -1;
}

/**
 * spinand_get_otp- send command 0xf to read the SPI Nand OTP register
 * Description:
 *   There is one bit( bit 0x10 ) to set or to clear the internal ECC.
 *   Enable chip internal ECC, set the bit to 1
 *   Disable chip internal ECC, clear the bit to 0
 */
static int spinand_get_otp(struct spi_device *spi_nand, u8 *otp)
{
	struct SPI_NAND_FLASH_INFO_T *ptr_dev_info_t = _SPI_NAND_GET_DEVICE_INFO_PTR;

	return spi_nand_protocol_get_feature(ptr_dev_info_t->ecc_en.ecc_en_addr, otp);
}

/**
 * spinand_set_otp- send command 0x1f to write the SPI Nand OTP register
 * Description:
 *   There is one bit( bit 0x10 ) to set or to clear the internal ECC.
 *   Enable chip internal ECC, set the bit to 1
 *   Disable chip internal ECC, clear the bit to 0
 */
static int spinand_set_otp(struct spi_device *spi_nand, u8 *otp)
{
	struct SPI_NAND_FLASH_INFO_T *ptr_dev_info_t =  _SPI_NAND_GET_DEVICE_INFO_PTR;

	return spi_nand_protocol_set_feature(ptr_dev_info_t->ecc_en.ecc_en_addr, *otp);
}

static SPI_NAND_FLASH_RTN_T spi_nand_write_page_internal(u32 page_number, 
														u32 data_offset,
										  				const u8  *ptr_data, 
													  	u32 data_len, 
													  	u32 oob_offset,
														u8  *ptr_oob , 
														u32 oob_len,
													    SPI_NAND_FLASH_WRITE_SPEED_MODE_T speed_mode )
{
	u32					 			addr_offset;
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T			rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	unsigned long					spinand_spinlock_flags;
		
#if	defined(TCSUPPORT_NAND_BMT)
    unsigned short phy_block_bbt;
	u32			   logical_block, physical_block;
	u32			   page_offset_in_block;
#endif

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spi_nand_write_page_internal]: enter\n");
	
	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;	
	
	_SPI_NAND_SEMAPHORE_LOCK();	
	
	SPI_NAND_Flash_Clear_Read_Cache_Data();
	
#if	defined(TCSUPPORT_NAND_BMT)
		page_offset_in_block = ((page_number * (ptr_dev_info_t->page_size))%(ptr_dev_info_t->erase_size))/(ptr_dev_info_t->page_size);
		logical_block = ((page_number * (ptr_dev_info_t->page_size))/(ptr_dev_info_t->erase_size)) ;
		physical_block = get_mapping_block_index(logical_block, &phy_block_bbt);
		if( physical_block != logical_block) {			
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "Bad Block Mapping, from %d block to %d block\n", logical_block, physical_block);
		}
		page_number = (page_offset_in_block)+((physical_block*(ptr_dev_info_t->erase_size))/(ptr_dev_info_t->page_size));
#endif
		
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spi_nand_write_page_internal]: page_number = 0x%x\n", page_number);
		
#if	defined(TCSUPPORT_NAND_BMT)
		if(block_is_in_bmt_region(physical_block)) {						
			memcpy(ptr_oob + OOB_INDEX_OFFSET, &phy_block_bbt, OOB_INDEX_SIZE);
		}
		
       if(_SPI_NAND_WRITE_FAIL_TEST_FLAG == 0) {
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spi_nand_write_page_internal: data_offset=0x%x, date_len=0x%x, oob_offset=0x%x, oob_len=0x%x\n", data_offset, data_len, oob_offset, oob_len);
			_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &ptr_oob[0], oob_len);

			rtn_status = spi_nand_write_page(page_number, addr_offset, ptr_data, data_len, 0, ptr_oob, MAX_USE_OOB_SIZE , speed_mode);
       } else {
	   		rtn_status = SPI_NAND_FLASH_RTN_PROGRAM_FAIL;
	   }
					
		if(rtn_status != SPI_NAND_FLASH_RTN_NO_ERROR) {
		    _SPI_NAND_PRINTF("write fail at page: 0x%x \n", page_number);
            if (update_bmt(page_number * (ptr_dev_info_t->page_size), UPDATE_WRITE_FAIL, ptr_data, ptr_oob)) {
                _SPI_NAND_PRINTF("Update BMT success\n");
				rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
            } else {
                _SPI_NAND_PRINTF("Update BMT fail\n");
            }		
		}
#else
		rtn_status = spi_nand_write_page(page_number, addr_offset, ptr_data, data_len, 0, ptr_oob, MAX_USE_OOB_SIZE , speed_mode);
#endif

	SPI_NAND_Flash_Clear_Read_Cache_Data();
	
	_SPI_NAND_SEMAPHORE_UNLOCK();
	
	return (rtn_status);
}

static SPI_NAND_FLASH_RTN_T spi_nand_read_page_internal(u32 page_number, 
														SPI_NAND_FLASH_READ_SPEED_MODE_T speed_mode)
{
	u32 							logical_block, physical_block;
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T			rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	unsigned long					spinand_spinlock_flags;

#if	defined(TCSUPPORT_NAND_BMT)
	unsigned short phy_block_bbt;
	u32 		   page_offset_in_block;
#endif

	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;			

	_SPI_NAND_SEMAPHORE_LOCK(); 	
	
#if	defined(TCSUPPORT_NAND_BMT)
	page_offset_in_block = (((page_number * (ptr_dev_info_t->page_size))%(ptr_dev_info_t->erase_size))/ (ptr_dev_info_t->page_size));
	logical_block = ((page_number * (ptr_dev_info_t->page_size))/(ptr_dev_info_t->erase_size)) ;
	physical_block = get_mapping_block_index(logical_block, &phy_block_bbt);
	if( physical_block != logical_block) {			
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "Bad Block Mapping, from %d block to %d block\n", logical_block, physical_block);
	}
	page_number = (page_offset_in_block)+ ((physical_block*(ptr_dev_info_t->erase_size))/(ptr_dev_info_t->page_size));
#endif				

	rtn_status = spi_nand_read_page(page_number, speed_mode);

	_SPI_NAND_SEMAPHORE_UNLOCK();
			
	return (rtn_status);
}

/**
 * spinand_write_enable- send command 0x06 to enable write or erase the
 * Nand cells
 * Description:
 *   Before write and erase the Nand cells, the write enable has to be set.
 *   After the write or erase, the write enable bit is automatically
 *   cleared (status register bit 2)
 *   Set the bit 2 of the status register has the same effect
 */
static int spinand_write_enable(struct spi_device *spi_nand)
{
	return spi_nand_protocol_write_enable();
}

static int spinand_read_page_to_cache(struct spi_device *spi_nand, u32 page_id)
{
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "spinand_read_page_to_cache: page_idx=0x%x\n", page_id);
	return spi_nand_protocol_page_read((u32)page_id);
}

/*
 * spinand_read_from_cache- send command 0x03 to read out the data from the
 * cache register(2112 bytes max)
 * Description:
 *   The read can specify 1 to 2112 bytes of data read at the corresponding
 *   locations.
 *   No tRd delay.
 */
static int spinand_read_from_cache(struct spi_device *spi_nand, u32 page_id,
                u32 byte_id, u32 len, u8 *rbuf)
{
	unsigned int					ret;
	u8								status;
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;

	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;
		
	spi_nand_protocol_read_from_cache(byte_id, len, rbuf, ptr_dev_info_t->read_mode, ptr_dev_info_t->dummy_mode);
      
	while (1) {
		ret = spinand_read_status(spi_nand, &status);
		if (ret < 0) {
			_SPI_NAND_PRINTF("err %d read status register\n", ret);
			return ret;
		}

		if ((status & STATUS_OIP_MASK) == STATUS_READY) {
			break;
		}
	}            

	return 0;  /* tmp return success any way */
}

/*
 * spinand_read_page-to read a page with:
 * @page_id: the physical page number
 * @offset:  the location from 0 to 2111
 * @len:     number of bytes to read
 * @rbuf:    read buffer to hold @len bytes
 *
 * Description:
 *   The read includes two commands to the Nand: 0x13 and 0x03 commands
 *   Poll to read status to wait for tRD time.
 */
static int spinand_read_page(struct spi_device *spi_nand, u32 page_id,
                u32 offset, u32 len, u8 *rbuf)
{
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T			rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	SPI_NAND_FLASH_RTN_T 			*status;

	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;

	rtn_status = spi_nand_read_internal((ptr_dev_info_t->page_size * page_id) + offset, len, rbuf, ptr_dev_info_t->read_mode, status);

	if(rtn_status == SPI_NAND_FLASH_RTN_NO_ERROR) {
		return 0;
	} else {
		_SPI_NAND_PRINTF("spinand_read_page, error\n");
		return -1;
	}
}

/*
 * spinand_program_data_to_cache--to write a page to cache with:
 * @byte_id: the location to write to the cache
 * @len:     number of bytes to write
 * @rbuf:    read buffer to hold @len bytes
 *
 * Description:
 *   The write command used here is 0x84--indicating that the cache is
 *   not cleared first.
 *   Since it is writing the data to cache, there is no tPROG time.
 */
static int spinand_program_data_to_cache(struct spi_device *spi_nand,
                u32 page_id, u32 byte_id, u32 len, u8 *wbuf)
{
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;

	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;
	
	return spi_nand_protocol_program_load(byte_id, wbuf, len, ptr_dev_info_t->write_mode);
}

/**
 * spinand_program_execute--to write a page from cache to the Nand array with
 * @page_id: the physical page location to write the page.
 *
 * Description:
 *   The write command used here is 0x10--indicating the cache is writing to
 *   the Nand array.
 *   Need to wait for tPROG time to finish the transaction.
 */
static int spinand_program_execute(struct spi_device *spi_nand, u32 page_id)
{
	return spi_nand_protocol_program_execute(page_id);
}

/**
 * spinand_program_page--to write a page with:
 * @page_id: the physical page location to write the page.
 * @offset:  the location from the cache starting from 0 to 2111
 * @len:     the number of bytes to write
 * @wbuf:    the buffer to hold the number of bytes
 *
 * Description:
 *   The commands used here are 0x06, 0x84, and 0x10--indicating that
 *   the write enable is first sent, the write cache command, and the
 *   write execute command.
 *   Poll to wait for the tPROG time to finish the transaction.
 */
static int spinand_program_page(struct mtd_info *mtd,
                u32 page_id, u32 offset, u32 len, u8 *buf)
{
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	struct spinand_state *state = mtd_to_state(mtd);			
	SPI_NAND_FLASH_RTN_T			rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_program_page]: enter, page=0x%x\n", page_id);
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spinand_program_page: _current_cache_page_oob_mapping: state->oob_buf_len=0x%x, state->oob_buf=\n", (state->oob_buf_len));
	_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &(state->oob_buf[0]), (state->oob_buf_len));
	
	rtn_status = spi_nand_write_page_internal(page_id, (state->buf_idx), &state->buf[(state->buf_idx)], (state->buf_len),  0, (&state->oob_buf[0]), (state->oob_buf_len), ptr_dev_info_t->write_mode);

	if( rtn_status == SPI_NAND_FLASH_RTN_NO_ERROR) {
		return 0;
	} else {
		_SPI_NAND_PRINTF("spinand_program_page, error\n");
		return -1;
	}
}

/**
 * spinand_erase_block_erase--to erase a page with:
 * @block_id: the physical block location to erase.
 *
 * Description:
 *   The command used here is 0xd8--indicating an erase command to erase
 *   one block--64 pages
 *   Need to wait for tERS.
 */
static int spinand_erase_block_erase(struct spi_device *spi_nand, u32 block_id)
{
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1,"[spinand_erase_block_erase]: enter, block id=0x%x \n", block_id);
	return spi_nand_protocol_block_erase(block_id);
}

/**
 * spinand_erase_block--to erase a page with:
 * @block_id: the physical block location to erase.
 *
 * Description:
 *   The commands used here are 0x06 and 0xd8--indicating an erase
 *   command to erase one block--64 pages
 *   It will first to enable the write enable bit (0x06 command),
 *   and then send the 0xd8 erase command
 *   Poll to wait for the tERS time to complete the tranaction.
 */
static int spinand_erase_block(struct spi_device *spi_nand, u32 block_id)
{
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T			rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;

	
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1,"[spinand_erase_block]: enter, block id=0x%x \n", block_id);
	
	rtn_status = spi_nand_erase_internal(block_id * ptr_dev_info_t->erase_size, ptr_dev_info_t->erase_size);

	if(rtn_status == SPI_NAND_FLASH_RTN_NO_ERROR) {
		return 0;
	} else {
		_SPI_NAND_PRINTF("spinand_erase_block, error\n");
		return -1;
	}
}

void spinand_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	int		min_oob_size;
    struct spinand_state *state = mtd_to_state(mtd);

	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_write_buf]: enter \n");

	if(state->col >= mtd->writesize) {	/* Write OOB area */
		min_oob_size = MIN(len, (MAX_LINUX_USE_OOB_SIZE - ((state->col) - (mtd->writesize))));
		memcpy( &(state->oob_buf)[((state->col)-(mtd->writesize))+LINUX_USE_OOB_START_OFFSET], buf, min_oob_size);
		state->col += min_oob_size;
		state->oob_buf_len = min_oob_size;
	} else {							/* Write Data area */
		memcpy( &(state->buf)[state->buf_idx], buf, len);
	    state->col += len;
		state->buf_len += len;
	}
}

void spinand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
    struct spinand_state *state = mtd_to_state(mtd);
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_read_buf]: enter, len=0x%x, offset=0x%x\n", len, state->buf_idx);	

	if(	(state->command == NAND_CMD_READID) ||
		(state->command == NAND_CMD_STATUS) ) {
		memcpy(buf, &state->buf, len);
	} else {
		if( (state->buf_idx) < ( ptr_dev_info_t->page_size )) {		/* Read data area */
        	memcpy(buf, &_current_cache_page_data[state->buf_idx], len);
		} else {													/* Read oob area */
			/* dump_stack(); */
			memcpy(buf, &_current_cache_page_oob_mapping[ ((state->buf_idx)-(ptr_dev_info_t->page_size))+ LINUX_USE_OOB_START_OFFSET], len);			
		}	        
	}	

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spinand_read_buf : idx=0x%x, len=0x%x\n", (state->buf_idx), len);
	_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &buf[0], len);
	
	state->buf_idx += len;
}

#ifdef CONFIG_MTD_SPINAND_ONDIEECC
/**
 * spinand_enable_ecc- send command 0x1f to write the SPI Nand OTP register
 * Description:
 *   There is one bit( bit 0x10 ) to set or to clear the internal ECC.
 *   Enable chip internal ECC, set the bit to 1
 *   Disable chip internal ECC, clear the bit to 0
 */
static int spinand_enable_ecc(struct spi_device *spi_nand)
{
    u8 otp = 0;

	SPI_NAND_Flash_Enable_OnDie_ECC();
	
	return spinand_get_otp(spi_nand, &otp);
}

static int spinand_disable_ecc(struct spi_device *spi_nand)
{
	SPI_NAND_Flash_Disable_OnDie_ECC();
		
	return 0;
}

static 
#if defined(TCSUPPORT_3_18_21_KERNEL)
int
#else
void
#endif
spinand_write_page_hwecc(struct mtd_info *mtd,
                struct nand_chip *chip, const uint8_t *buf, int oob_required)
{
	
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_write_page_hwecc]: enter \n");

	spinand_write_buf(mtd, buf, mtd->writesize);
	spinand_write_buf(mtd, chip->oob_poi, mtd->oobsize);

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spinand_write_page_hwecc: data=\n");
	_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &buf[0], mtd->writesize);
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spinand_write_page_hwecc: oob=\n");
	_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &chip->oob_poi[0], mtd->oobsize);	

#if defined(TCSUPPORT_3_18_21_KERNEL)
	return 0;
#endif
}

static int spinand_read_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip,
                uint8_t *buf, 
#if defined(TCSUPPORT_3_18_21_KERNEL)
				int oob_required,
#endif
                int page)
{
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	unsigned long					spinand_spinlock_flags;

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_read_page_hwecc]: enter, page=0x%x \n", page);	
	
	_SPI_NAND_SEMAPHORE_LOCK();

	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;	

	if(buf == NULL) {
		_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_read_page_hwecc]buf is NULL\n");
	}
	
	memcpy(buf, &_current_cache_page_data[0], (ptr_dev_info_t->page_size));
	memcpy((chip->oob_poi), &_current_cache_page_oob_mapping[LINUX_USE_OOB_START_OFFSET], MAX_LINUX_USE_OOB_SIZE);

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spinand_read_page_hwecc: data:\n");
	_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &buf[0], mtd->writesize);
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spinand_read_page_hwecc: oob:\n");
	_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &chip->oob_poi[0], mtd->oobsize);	
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_2, "spinand_read_page_hwecc: _current_cache_page_oob_mapping:\n");
	_SPI_NAND_DEBUG_PRINTF_ARRAY(SPI_NAND_FLASH_DEBUG_LEVEL_2, &_current_cache_page_oob_mapping[0], mtd->oobsize);			

	_SPI_NAND_SEMAPHORE_UNLOCK();

	return 0;
}
#endif

static int spinand_write_page(struct mtd_info *mtd, struct nand_chip *chip, 
        const u8 *buf, int page, int cached, int raw)
{
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	struct spinand_state *state = mtd_to_state(mtd);			
	SPI_NAND_FLASH_RTN_T			rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;

	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_write_page]: enter, page=0x%x \n", page);	

	memset((state->oob_buf), 0xff, MAX_USE_OOB_SIZE);
	memcpy(&(state->oob_buf)[LINUX_USE_OOB_START_OFFSET], chip->oob_poi, MAX_LINUX_USE_OOB_SIZE);
	
	rtn_status = spi_nand_write_page_internal(page, 0, buf, (ptr_dev_info_t->page_size),  0, (state->oob_buf), MAX_LINUX_USE_OOB_SIZE, ptr_dev_info_t->write_mode);
	if(rtn_status == SPI_NAND_FLASH_RTN_NO_ERROR) {
		return 0;
	} else {
		return -EIO;
	}
}

static int spinand_write_oob(struct mtd_info *mtd, struct nand_chip *chip, int page)
{
	struct spinand_state *state = mtd_to_state(mtd);			
	SPI_NAND_FLASH_RTN_T			rtn_status = SPI_NAND_FLASH_RTN_NO_ERROR;
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;

	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;
	
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_write_oob]: enter, page=0x%x \n", page);	

	memset(state->oob_buf, 0xff, MAX_USE_OOB_SIZE);
	memcpy(&(state->oob_buf)[LINUX_USE_OOB_START_OFFSET], chip->oob_poi, MAX_LINUX_USE_OOB_SIZE);

	rtn_status = spi_nand_write_page_internal(page, 0, NULL, 0,  0, (&state->oob_buf[0]), MAX_LINUX_USE_OOB_SIZE, ptr_dev_info_t->write_mode);

	if(rtn_status == SPI_NAND_FLASH_RTN_NO_ERROR)  {
		return 0;
	} else {
		return -EIO;
	}
}

static int spinand_read_oob(struct mtd_info *mtd,struct nand_chip *chip, int page, int sndcmd)
{
	return 0;
}

static int spinand_block_markbad(struct mtd_info *mtd, loff_t offset)
{
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_block_markbad]: enter , offset=0x%x\n", offset);

	return 0;
}


static int spinand_block_bad(struct mtd_info *mtd, loff_t ofs, int getchip)
{
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_block_bad]: enter \n");
	
	return 0;
}

static void spinand_select_chip(struct mtd_info *mtd, int dev)
{
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_select_chip]: enter \n");	
}

static int spinand_dev_ready(struct mtd_info *mtd)
{	
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_dev_ready]: enter \n");
	return 1;
}

static void spinand_enable_hwecc(struct mtd_info *mtd, int mode)
{
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_enable_hwecc]: enter \n");
}

static int spinand_correct_data(struct mtd_info *mtd, u_char *dat,
				     u_char *read_ecc, u_char *calc_ecc)
{
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_correct_data]: enter \n");
	return 0;
}

static int spinand_calculate_ecc(struct mtd_info *mtd, const u_char *dat, u_char *ecc_code)
{
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_calculate_ecc]: enter \n");
	return 0;
}

static uint8_t spinand_read_byte(struct mtd_info *mtd)
{
	struct spinand_state *state = mtd_to_state(mtd);
	u8 data;

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_read_byte]: enter \n");		

	data = state->buf[state->buf_idx];
	state->buf_idx++;
	return data;
}

static int spinand_wait(struct mtd_info *mtd, struct nand_chip *chip)
{
	struct spinand_info *info = (struct spinand_info *)chip->priv;

	unsigned long timeo = jiffies;
	int retval, state = chip->state;
	u8 status;

	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_wait]: enter \n");		

	if (state == FL_ERASING)
		timeo += (HZ * 400) / 1000;
	else
		timeo += (HZ * 20) / 1000;

	while (time_before(jiffies, timeo)) {
		retval = spinand_read_status(info->spi, &status);
		if ((status & STATUS_OIP_MASK) == STATUS_READY)
			return 0;

		cond_resched();
	}
	
	return 0;
}

/*
 * spinand_reset- send RESET command "0xff" to the Nand device.
 */
static void spinand_reset(struct spi_device *spi_nand)
{
	_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_reset]: enter \n");

	spi_nand_protocol_reset();
}

static void spinand_cmdfunc(struct mtd_info *mtd, unsigned int command, int column, int page)
{
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	
    struct nand_chip *chip = (struct nand_chip *)mtd->priv;
    struct spinand_info *info = (struct spinand_info *)chip->priv;
    struct spinand_state *state = (struct spinand_state *)info->priv;
    u16		block_id;

	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;

	state->command = command;
	
	switch (command) {
	/*
	 * READ0 - read in first  0x800 bytes
	 */
	case NAND_CMD_READ1:
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_cmdfunc]: NAND_CMD_READ1 \n");
	case NAND_CMD_READ0:
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_cmdfunc]: NAND_CMD_READ0 \n");	

			state->buf_idx = column;
			spi_nand_read_page_internal(page, ptr_dev_info_t->read_mode);
			
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1,"[spinand_cmdfunc]: NAND_CMD_READ0/1, End\n");

	        break;
	/* READOOB reads only the OOB because no ECC is performed. */
	case NAND_CMD_READOOB:
 			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_cmdfunc]: NAND_CMD_READOOB, page=0x%x \n", page);
	        state->buf_idx = column + (ptr_dev_info_t->page_size);
	        spi_nand_read_page_internal(page, ptr_dev_info_t->read_mode);
			
	        break;
	case NAND_CMD_RNDOUT:
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_cmdfunc]: NAND_CMD_RNDOUT \n");
	        state->buf_idx = column;
	        break;
	case NAND_CMD_READID:
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_cmdfunc]: NAND_CMD_READID \n");
	        state->buf_idx = 0;
	        spinand_read_id(info->spi, (u8 *)state->buf);
	        break;
	/* ERASE1 stores the block and page address */
	case NAND_CMD_ERASE1:
			block_id = page /((mtd->erasesize)/(mtd->writesize));

			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_cmdfunc]: NAND_CMD_ERASE1 \n");			
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "erasesize=0x%x, writesiez=0x%x, page=0x%x, block_idx=0x%x\n", (mtd->erasesize), (mtd->writesize), page, block_id);
	        spinand_erase_block(info->spi, block_id);
	        break;
	/* ERASE2 uses the block and page address from ERASE1 */
	case NAND_CMD_ERASE2:
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1,"[spinand_cmdfunc]: NAND_CMD_ERASE2 \n");
	        break;
	/* SEQIN sets up the addr buffer and all registers except the length */
	case NAND_CMD_SEQIN:
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_cmdfunc]: NAND_CMD_SEQIN \n");			
	        state->col = column;
	        state->row = page;
	        state->buf_idx = column;
			state->buf_len = 0;			
			state->oob_buf_len = 0 ;
			memset(state->buf, 0xff, BUFSIZE);
			memset(state->oob_buf, 0xff, MAX_USE_OOB_SIZE);
	        break;
	/* PAGEPROG reuses all of the setup from SEQIN and adds the length */
	case NAND_CMD_PAGEPROG:
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_cmdfunc]: NAND_CMD_PAGEPROG \n");			
	        spinand_program_page(mtd, state->row, state->col,
	                        state->buf_idx, state->buf);
	        break;
	case NAND_CMD_STATUS:
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_cmdfunc]: NAND_CMD_STATUS \n");			
	        spinand_get_otp(info->spi, state->buf);
	        if (!(state->buf[0] & 0x80))
	                state->buf[0] = 0x80;
	        state->buf_idx = 0;
	        break;
	/* RESET command */
	case NAND_CMD_RESET:
			_SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1, "[spinand_cmdfunc]: NAND_CMD_RESET \n");			
	        if (wait_till_ready(info->spi))
	                printk("WAIT timedout!!!\n");
	        /* a minimum of 250us must elapse before issuing RESET cmd*/
	        udelay(250);
	        spinand_reset(info->spi);
	        break;
	default:
	        _SPI_NAND_DEBUG_PRINTF(SPI_NAND_FLASH_DEBUG_LEVEL_1,"[spinand_cmdfunc]: Unknown CMD: 0x%x\n", command);
	}
}

struct nand_flash_dev spi_nand_flash_ids[] = {
	{NULL,	0, 0, 0, 0, 0},
	{NULL,}
};

static void free_allcate_memory(struct mtd_info *mtd)
{
	_SPI_NAND_PRINTF("SPI NAND : free_allcate_memory");
	
	if(((struct spinand_info *)(((struct nand_chip *)(mtd->priv))->priv))->spi) {
		kfree( ((struct spinand_info *)(((struct nand_chip *)(mtd->priv))->priv))->spi );
	}

	if(((struct spinand_info *)(((struct nand_chip *)(mtd->priv))->priv))) {
		kfree( ((struct spinand_info *)(((struct nand_chip *)(mtd->priv))->priv)) );
	}

	if((((struct nand_chip *)(mtd->priv))->priv))	{
		kfree ( (((struct nand_chip *)(mtd->priv))->priv) ) ;
	}

	if((mtd->priv)) {
		kfree((mtd->priv));
	}

	if(mtd) {
		kfree(mtd);
	}	
}

static int spi_nand_setup(u32 *ptr_rtn_mtd_address)
{
    struct mtd_info 		*mtd;
    struct nand_chip 		*chip;
    struct spinand_info 	*info;
    struct spinand_state	*state;    
    struct spi_device		*spi_nand;
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	int ret;

	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;

	_SPI_NAND_PRINTF("[spi_nand_setup] : Enter \n");
    
	/* 1. Allocate neccessary struct memory ,and assigned related pointer */
    info  = kzalloc(sizeof(struct spinand_info),GFP_KERNEL);
    if (!info) {
    	_SPI_NAND_PRINTF("spi_nand_setup: allocate info structure error! \n");
		return -ENOMEM;            
    }
            	            
	/* , temp assign. Other function will pass it, but we wil not use it in functions. */	            
	spi_nand = kzalloc(sizeof(struct spinand_info),GFP_KERNEL);	
    if (!spi_nand) {	
	    _SPI_NAND_PRINTF("spi_nand_setup: allocate spi_nand structure error! \n");
		return -ENOMEM;
    }
		
    info->spi = spi_nand;		

    state = kzalloc(sizeof(struct spinand_state),GFP_KERNEL);    
    if (!state) {	
	    _SPI_NAND_PRINTF("spi_nand_setup: allocate state structure error! \n");
		return -ENOMEM;
    }

    info->priv      = state;
    state->buf_idx  = 0;
    state->buf      = kzalloc( BUFSIZE, GFP_KERNEL);			/* Data buffer */
	state->oob_buf	= kzalloc( MAX_USE_OOB_SIZE, GFP_KERNEL);	/* OOB buffer */
    if (!state->buf) {
    	_SPI_NAND_PRINTF("spi_nand_setup: allocate data buf error! \n");
		return -ENOMEM;
    }
    if (!state->oob_buf) {
    	_SPI_NAND_PRINTF("spi_nand_setup: allocate oob buf error! \n");
		return -ENOMEM;
    }	

    chip = kzalloc(sizeof(struct nand_chip),GFP_KERNEL);
    if (!chip) {
	    _SPI_NAND_PRINTF("spi_nand_setup: allocate chip structure error! \n");
    	return -ENOMEM;
    }

	chip->priv				= info;
	chip->read_byte 		= spinand_read_byte;
	chip->read_buf			= spinand_read_buf;
	chip->write_buf 		= spinand_write_buf;
	chip->waitfunc			= spinand_wait;
	chip->options			|= NAND_CACHEPRG;
	chip->select_chip		= spinand_select_chip;
	chip->dev_ready 		= spinand_dev_ready;
	chip->cmdfunc			= spinand_cmdfunc;
#ifdef CONFIG_MTD_SPINAND_ONDIEECC
    chip->ecc.mode  		= NAND_ECC_HW;
    chip->ecc.size  		= 0x200;
    chip->ecc.bytes 		= 0x4;
    chip->ecc.steps 		= 0x4;
	chip->ecc.total 		= chip->ecc.steps * chip->ecc.bytes;
#if 0  
    chip->ecc.strength		= 1;
#endif     
    chip->ecc.layout 		= &spinand_oob_64;
    chip->ecc.read_page		= spinand_read_page_hwecc;
    chip->ecc.write_page 	= spinand_write_page_hwecc;
	if(isSpiNandAndCtrlECC) {
		/* Disable OnDie ECC */
		if (spinand_disable_ecc(spi_nand) < 0)
			pr_info("%s: disable ecc failed!\n", __func__);
	} else {
		/* Enable OnDie ECC */
		if (spinand_enable_ecc(spi_nand) < 0)
			pr_info("%s: enable ecc failed!\n", __func__);
	}
#else
    chip->ecc.mode  = NAND_ECC_SOFT;
    if (spinand_disable_ecc(spi_nand) < 0)
		pr_info("%s: disable ecc failed!\n", __func__);
#endif

	chip->options	|= NAND_NO_SUBPAGE_WRITE;	/* Chip does not allow subpage writes. */
    chip->options	|= NAND_SKIP_BBTSCAN;   	/*To skips the bbt scan during initialization.  */
    /*  For BMT, we need to revise driver architecture */
#if 0	
    //chip->write_page		= spinand_write_page;
    //chip->ecc.read_oob		= spinand_read_oob;
#endif
    chip->ecc.write_oob		= spinand_write_oob;
    chip->block_markbad		= spinand_block_markbad;   /* tmp null */
    chip->block_bad			= spinand_block_bad;		/* tmp null */
	chip->ecc.calculate		= spinand_calculate_ecc;
	chip->ecc.correct		= spinand_correct_data;
	chip->ecc.hwctl			= spinand_enable_hwecc;

    mtd = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
    if (!mtd) {
		_SPI_NAND_PRINTF("spi_nand_setup: allocate mtd error! \n");
		return -ENOMEM;
    }

	spi_nand_mtd = mtd;
    mtd->priv = chip;
    mtd->name = "EN7512-SPI_NAND";
    mtd->owner = THIS_MODULE;
    mtd->oobsize = MAX_LINUX_USE_OOB_SIZE;

	spi_nand_flash_ids[0].name 		= ptr_dev_info_t->ptr_name;
#if defined(TCSUPPORT_3_18_21_KERNEL)
	spi_nand_flash_ids[0].dev_id   		= ptr_dev_info_t->dev_id; 
#else
	spi_nand_flash_ids[0].id   		= ptr_dev_info_t->dev_id; 
#endif
	spi_nand_flash_ids[0].pagesize  = ptr_dev_info_t->page_size; 
	spi_nand_flash_ids[0].chipsize  = ((ptr_dev_info_t->device_size)>>20); 
	spi_nand_flash_ids[0].erasesize = ptr_dev_info_t->erase_size; 
	spi_nand_flash_ids[0].options   = 0; 
	
	ret = nand_scan_ident(mtd, 1, spi_nand_flash_ids);
	if (!ret) {
		_SPI_NAND_PRINTF("nand_scan_ident ok\n");
		ret = nand_scan_tail(mtd);
		_SPI_NAND_PRINTF("[spi_nand_setup]: chip size =  0x%llx, erase_shift=0x%x\n", chip->chipsize, chip->phys_erase_shift);
	} else {
		_SPI_NAND_PRINTF("nand_scan_ident fail\n");
		return -ENOMEM;
	}           

#if	defined(TCSUPPORT_NAND_BMT)
	if(init_bmt_bbt(mtd) == -1) {
		return -1;
	}

#ifdef TCSUPPORT_CT_PON
	mtd->size = nand_flash_avalable_size;
#else					
	mtd->size = nand_logic_size;					
#endif							
				
#endif
	ranand_read_byte  = SPI_NAND_Flash_Read_Byte;
	ranand_read_dword = SPI_NAND_Flash_Read_DWord;

	*ptr_rtn_mtd_address = mtd;
	
	return 0;
}

static void dump_bmt(bmt_struct *bmt)
{
    int i;
    
    _SPI_NAND_PRINTF("BMT v%d.", bmt->version);
	_SPI_NAND_PRINTF("bad count:%d\n", g_bmt->bad_count);
    _SPI_NAND_PRINTF("total %d mapping\n", bmt->mapped_count);
    for (i = 0; i < bmt->mapped_count; i++)
    {
        _SPI_NAND_PRINTF("0x%x -> 0x%x \n", bmt->table[i].bad_index, bmt->table[i].mapped_index);
    }
}

static int spi_nand_proc_read(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len;

	if (off > 0) {
		return 0;
	}

	len = sprintf(page, "SPI NAND DEBUG LEVEL=%d, _SPI_NAND_WRITE_FAIL_TEST_FLAG=%d, _SPI_NAND_ERASE_FAIL_TEST_FLAG=%d\n", _SPI_NAND_DEBUG_LEVEL, _SPI_NAND_WRITE_FAIL_TEST_FLAG, _SPI_NAND_ERASE_FAIL_TEST_FLAG);

	return len;
}

static int spi_nand_proc_write(struct file* file, const char* buffer,
	unsigned long count, void *data)
{
	char buf[16];

	int len = count;

	if (copy_from_user(buf, buffer, len)) {
		return -EFAULT;
	}	

	buf[len] = '\0';

	_SPI_NAND_PRINTF("len = 0x%x, buf[0]=%c, buf[1]=%c, buf[2]=%c\n", len , buf[0], buf[1], buf[2]);

	if (buf[0] == '0') {
		_SPI_NAND_PRINTF("Set SPI NAND DEBUG LEVLE to %d\n", SPI_NAND_FLASH_DEBUG_LEVEL_0);
		_SPI_NAND_DEBUG_LEVEL = SPI_NAND_FLASH_DEBUG_LEVEL_0;
	} else if (buf[0] == '1') {
		_SPI_NAND_PRINTF("Set SPI NAND DEBUG LEVLE to %d\n", SPI_NAND_FLASH_DEBUG_LEVEL_1);
		_SPI_NAND_DEBUG_LEVEL = SPI_NAND_FLASH_DEBUG_LEVEL_1;
	} else if (buf[0] == '2') {
		_SPI_NAND_PRINTF("Set SPI NAND DEBUG LEVLE to %d\n", SPI_NAND_FLASH_DEBUG_LEVEL_2);
		_SPI_NAND_DEBUG_LEVEL = SPI_NAND_FLASH_DEBUG_LEVEL_2;
	} else {
		_SPI_NAND_PRINTF("DEBUG LEVEL only up to %d\n", (SPI_NAND_FLASH_DEBUG_LEVEL_DEF_NO -1 ));
	}

	if(buf[1] == '0')
	{
		_SPI_NAND_WRITE_FAIL_TEST_FLAG = 0;
		_SPI_NAND_PRINTF("Set _SPI_NAND_WRITE_FAIL_TEST_FLAG to %d\n", _SPI_NAND_WRITE_FAIL_TEST_FLAG);
	}
	if(buf[1] == '1')
	{
		_SPI_NAND_WRITE_FAIL_TEST_FLAG = 1;
		_SPI_NAND_PRINTF("Set _SPI_NAND_WRITE_FAIL_TEST_FLAG to %d\n", _SPI_NAND_WRITE_FAIL_TEST_FLAG);
	}	
	if(buf[2] == '0')
	{
		_SPI_NAND_ERASE_FAIL_TEST_FLAG = 0;
		_SPI_NAND_PRINTF("Set _SPI_NAND_ERASE_FAIL_TEST_FLAG to %d\n", _SPI_NAND_ERASE_FAIL_TEST_FLAG);
	}	
	if(buf[2] == '1')
	{
		_SPI_NAND_ERASE_FAIL_TEST_FLAG = 1;
		_SPI_NAND_PRINTF("Set _SPI_NAND_ERASE_FAIL_TEST_FLAG to %d\n", _SPI_NAND_ERASE_FAIL_TEST_FLAG);
	}

	return len;
}

static int write_test(void *arg)
{
	struct _SPI_NAND_FLASH_RW_TEST_T param;
	struct SPI_NAND_FLASH_INFO_T	 *ptr_dev_info_t;
	u32								 ptr_rtn_len;
	u8 buf[64], read_buf[64];
	SPI_NAND_FLASH_RTN_T status;
#if defined(CONFIG_MIPS_MT_SMP) || defined(CONFIG_MIPS_MT_SMTC)
	int cpu = smp_processor_id();
	int vpe = cpu_data[cpu].vpe_id;
#else
	int cpu = 0;
	int vpe = 0;
#endif

	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;

	memcpy(&param, arg, sizeof(struct _SPI_NAND_FLASH_RW_TEST_T));
	_SPI_NAND_PRINTF("write_test: run at vpe:%d, cpu:%d\n", vpe, cpu);
	_SPI_NAND_PRINTF("write_test: times=%d, block_idx=%d\n", param.times, param.block_idx);

	while (!kthread_should_stop() && param.times > 0) {
		if(param.times % 10 == 0)
			printk("write_test:%d\n", param.times);
		msleep(1);
		param.times--;
		get_random_bytes(buf, sizeof(buf));
		SPI_NAND_Flash_Erase(param.block_idx * ptr_dev_info_t->erase_size, sizeof(buf));
    	SPI_NAND_Flash_Write_Nbyte(param.block_idx * ptr_dev_info_t->erase_size, sizeof(buf), &ptr_rtn_len, buf, ptr_dev_info_t->write_mode);
		SPI_NAND_Flash_Read_NByte(param.block_idx * ptr_dev_info_t->erase_size, sizeof(read_buf), &ptr_rtn_len, read_buf, ptr_dev_info_t->read_mode, &status);

		if(memcmp(buf, read_buf, sizeof(buf)) != 0) {
			_SPI_NAND_PRINTF("write fail\n");
			return -1;
		}
	}

	_SPI_NAND_PRINTF("write done\n");

	return 0;
}

static int read_test(void *arg)
{
	struct _SPI_NAND_FLASH_RW_TEST_T param;
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	u32								 ptr_rtn_len;
	u8 buf[64];
	SPI_NAND_FLASH_RTN_T status;
#if defined(CONFIG_MIPS_MT_SMP) || defined(CONFIG_MIPS_MT_SMTC)
	int cpu = smp_processor_id();
	int vpe = cpu_data[cpu].vpe_id;
#else
	int cpu = 0;
	int vpe = 0;
#endif

	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;

	memcpy(&param, arg, sizeof(struct _SPI_NAND_FLASH_RW_TEST_T));
	_SPI_NAND_PRINTF("read_test: run at vpe:%d, cpu:%d\n", vpe, cpu);
	_SPI_NAND_PRINTF("read_test: times=%d, block_idx=%d\n", param.times, param.block_idx);

	memset(buf, 0xaa, sizeof(buf));

	while (!kthread_should_stop() && param.times > 0) {
		if(param.times % 10 == 0)
			_SPI_NAND_PRINTF("read_test:%d\n", param.times);
		msleep(1);
		param.times--;
		SPI_NAND_Flash_Read_NByte(param.block_idx * ptr_dev_info_t->erase_size, sizeof(buf), &ptr_rtn_len, buf, ptr_dev_info_t->read_mode, &status);
	}

	_SPI_NAND_PRINTF("read done\n");

	return 0;
}

static int spi_nand_proc_test_write(struct file* file, const char* buffer,
	unsigned long count, void *data)
{
	char 							buf[64], cmd[32];
	u32 							arg1, arg2;
	struct task_struct 				*thread;
	u32								ptr_rtn_len;
	u32								idx;
	struct SPI_NAND_FLASH_INFO_T	*ptr_dev_info_t;
	SPI_NAND_FLASH_RTN_T status;
	u8 feature;

	ptr_dev_info_t	= _SPI_NAND_GET_DEVICE_INFO_PTR;

	if (copy_from_user(buf, buffer, count)) {
		return -EFAULT;
	}	

	buf[count] = '\0';

	sscanf(buf, "%s %x %x", cmd, &arg1, &arg2) ;

	_SPI_NAND_PRINTF("cmd:%s, arg1=%u, arg2=%u\n", cmd, arg1, arg2);
	
	if (!strcmp(cmd, "rw_test")) {
		rw_test_param.times = arg1;
		rw_test_param.block_idx = arg2;

		thread = kthread_create(write_test, (void *)&rw_test_param, "write_test");
#if defined(CONFIG_MIPS_MT_SMP)
		kthread_bind(thread, 0);
#else
		kthread_bind(thread, 3);
#endif
		wake_up_process(thread);
		thread = kthread_create(read_test, (void *)&rw_test_param, "read_test");
#if defined(CONFIG_MIPS_MT_SMP)
		kthread_bind(thread, 1);
#else
		kthread_bind(thread, 2);
#endif

		wake_up_process(thread);
 	} else if (!strcmp(cmd, "read")) {
 		SPI_NAND_Flash_Clear_Read_Cache_Data();
 		spi_nand_read_page(arg1, SPI_NAND_FLASH_READ_SPEED_MODE_SINGLE); 
	} else if (!strcmp(cmd, "erase")) {
 		printk("erase block:0x%x\n", arg1);
		spi_nand_erase_block(arg1);
 	} else if (!strcmp(cmd, "getFeature")) {
		spi_nand_protocol_get_feature(arg1, &feature);
		printk("get feature:0x%x=0x%x\n", arg1, feature);
 	} else if (!strcmp(cmd, "dumpBmt")) {
		dump_bmt(g_bmt);
 	} else if (!strcmp(cmd, "clrCache")) {
		_current_page_num = 0xFFFFFFFF;
		printk("cache has beed cleared, _current_page_num:0x%x\n", _current_page_num);
#ifdef ERASE_WRITE_CNT_LOG
	} else if(!strcmp(cmd, "dbg_erase_write")) {
		if(arg1 == 0 || arg1 == 1) {
			/* set debug flag */
			erase_write_flag = (ERASE_WRITE_LOG_T)arg1;
			_SPI_NAND_PRINTF("erase_write_flag = %d\n\n", erase_write_flag);
		} else {
			_SPI_NAND_PRINTF("arg1 error, arg1 must 0 or 1\n");
		}
	} else if(!strcmp(cmd, "dbg_erase_write_clean")) {
		for(idx = 0; idx < MAX_BLOCK; idx++) {
			b_erase_cnt[idx] = 0;
			b_write_total_cnt[idx] = 0;
			b_write_cnt_per_erase[idx] = 0;
		}
	} else if(!strcmp(cmd, "dbg_erase_write_dump")) {
		_SPI_NAND_PRINTF("Erase total count:\n");
		for(idx = 0; idx < MAX_BLOCK; idx++) {
			if(b_erase_cnt[idx] != 0) {
				_SPI_NAND_PRINTF("block[0x%04x] = 0x%x\n", idx, b_erase_cnt[idx]);
			}
		}

		_SPI_NAND_PRINTF("Write total count:\n");
		for(idx = 0; idx < MAX_BLOCK; idx++) {
			if(b_write_total_cnt[idx] != 0) {
				_SPI_NAND_PRINTF("block[0x%04x] = 0x%x\n", idx, b_write_total_cnt[idx]);
			}
		}

		_SPI_NAND_PRINTF("Write per erase count:\n");
		for(idx = 0; idx < MAX_BLOCK; idx++) {
			if(b_write_cnt_per_erase[idx] != 0) {
				_SPI_NAND_PRINTF("block[0x%04x] = 0x%x\n", idx, b_write_cnt_per_erase[idx]);
			}
		}

		_SPI_NAND_PRINTF("dbg_erase_write_dump finish\n");
#endif

 	} else {
		_SPI_NAND_PRINTF("input not defined.\n");
	}

	return count;
}


static struct mtd_info *spi_nand_probe_kernel(struct map_info *map)
{
	u32	mtd_address;
	int	rtn_status;

	_SPI_NAND_PRINTF("EN7512 mtd init: spi nand probe enter\n");		

	rtn_status = spi_nand_setup(&mtd_address);

	if(rtn_status == 0) {  /* Probe without error */
		return ((struct mtd_info * )(mtd_address));
	} else {
		free_allcate_memory((struct mtd_info * )(mtd_address));			
		_SPI_NAND_PRINTF("[spi_nand_probe_kernel] probe fail !\n");
		return NULL;
	}
}

static void spi_nand_destroy_kernel(struct mtd_info *mtd)
{
	free_allcate_memory(mtd);	
}

static struct mtd_chip_driver spi_nand_chipdrv = {
	.probe	 = spi_nand_probe_kernel,
	.destroy = spi_nand_destroy_kernel,
	.name	 = "nandflash_probe",
	.module	 = THIS_MODULE
};

static int __init linux_spi_nand_flash_init(void)
{
	struct proc_dir_entry *entry;

	_SPI_NAND_PRINTF("IS_SPIFLASH=0x%x, IS_NANDFLASH=0x%x, (0xBFA10114)=0x%lx)\n", (unsigned int)IS_SPIFLASH, (unsigned int)IS_NANDFLASH, VPint(0xBFA10114));
	
	if(IS_SPIFLASH) { 	/* For boot from SPI NOR, then mount NAND as a MTD partition */
		_SPI_NAND_PRINTF("[linux_spi_nand_flash_init] spi nor flash\n");
		return -1;
	} else {				
		SPI_NAND_Flash_Init(0);
		
		_SPI_NAND_PRINTF("spi nand flash\n");
		register_mtd_chip_driver(&spi_nand_chipdrv);

		entry = create_proc_entry(SPI_NAND_PROCNAME, 0666, NULL);
		if (entry == NULL) {
			_SPI_NAND_PRINTF("SPI NAND  unable to create /proc entry\n");
			return -ENOMEM;
		}
		entry->read_proc = spi_nand_proc_read;
		entry->write_proc = spi_nand_proc_write;

		entry = create_proc_entry(SPI_NAND_TEST, 0666, NULL);
		if (entry == NULL) {
			_SPI_NAND_PRINTF("SPI NAND  unable to create /proc entry\n");
			return -ENOMEM;
		}
		entry->write_proc = spi_nand_proc_test_write;

#if (defined(TCSUPPORT_CPU_EN7516)||defined(TCSUPPORT_CPU_EN7527)) && defined(TCSUPPORT_AUTOBENCH)	
		if(ecnt_register_hook(&ecnt_spi_nand_op)) {
			printk("ecnt_flash_op register fail\n");
			return -ENODEV ;
		}
#endif
		
		return 0;
	}
}

static void __exit linux_spi_nand_flash_exit(void)
{
	if(IS_SPIFLASH) {
	} else {
#if (defined(TCSUPPORT_CPU_EN7516)||defined(TCSUPPORT_CPU_EN7527)) && defined(TCSUPPORT_AUTOBENCH)
		ecnt_unregister_hook(&ecnt_spi_nand_op);
#endif
		unregister_mtd_chip_driver(&spi_nand_chipdrv);

		remove_proc_entry(SPI_NAND_PROCNAME, NULL);
		remove_proc_entry(SPI_NAND_TEST, NULL);
	}	
}

module_init(linux_spi_nand_flash_init);
module_exit(linux_spi_nand_flash_exit);

#endif

/* End of [spi_nand_flash.c] package */
