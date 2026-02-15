/************************************************************************
 *
 *	Copyright (C) 2010 Trendchip Technologies, Corp.
 *	All Rights Reserved.
 *
 * Trendchip Confidential; Need to Know only.
 * Protected as an unpublished work.
 *
 * The computer program listings, specifications and documentation
 * herein are the property of Trendchip Technologies, Co. and shall
 * not be reproduced, copied, disclosed, or used in whole or in part
 * for any reason without the prior express written permission of
 * Trendchip Technologeis, Co.
 *
 *************************************************************************/
#ifndef TC_PARTITION_H
#define TC_PARTITION_H
 #ifdef TCSUPPORT_MTD_ENCHANCEMENT
/*
note:		
	1.the read base address of reserve area need to compute by (flash total size -flash erase size * reverse block num) 
		-the base addrass can be obtained by mtd ioctl
		-for example :flash total size is 32 m,flash erase size is 64k,reverse block num is 1,so the read base address of 
		reserve area is 0x3fff0000
	2.reverse block num is according to compile option TCSUPPORT_RESERVEAREA_BLOCK
	TCSUPPORT_RESERVEAREA_BLOCK==1 --------reverse block num is 1
	TCSUPPORT_RESERVEAREA_BLOCK==2 --------reverse block num is 2
	TCSUPPORT_RESERVEAREA_BLOCK==3 --------reverse block num is 3
	TCSUPPORT_RESERVEAREA_BLOCK==4 --------reverse block num is 4
	3.if you modify the size or offset of the certain sector or you add a new sector,you must modify the reserve area table in order to let
	other people be clear.
*/

//********************************
//	operation of  reserve area 	       //
//********************************
#if defined(TCSUPPORT_CPU_EN7512) || defined(TCSUPPORT_CPU_EN7521)
#ifndef __KERNEL__
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <mtd/mtd-user.h>
#endif
#endif


#define TC_FLASH_READ_CMD		"/userfs/bin/mtd readflash %s %lu %lu %s"
#define TC_FLASH_WRITE_CMD	"/userfs/bin/mtd writeflash %s %lu %lu %s"

#define TC_FLASH_ERASE_SECTOR_CMD "/userfs/bin/mtd erasesector %lu %s"

#define RESERVEAREA_NAME "reservearea"

#ifdef TCSUPPORT_MT7570
#if defined (TCSUPPORT_WLAN_MT7615_11N) || defined (TCSUPPORT_DUAL_WLAN_MT7615E) || defined (TCSUPPORT_WLAN_MT7615D)
#ifdef TCSUPPORT_NAND_BMT
#define BOB_RA_OFFSET_OLD_MODE  656896
#else 
#define BOB_RA_OFFSET_OLD_MODE  329216
#endif
#else 
#if (TCSUPPORT_RESERVEAREA_BLOCK==4)
#define BOB_RA_OFFSET_OLD_MODE 198144
#else
#define BOB_RA_OFFSET_OLD_MODE  1536
#endif
#endif
#endif

#if defined(TCSUPPORT_CPU_EN7512) || defined(TCSUPPORT_CPU_EN7521)
#ifndef __KERNEL__
static inline int mtd_reservearea_open(const char *mtd, int flags)
{
	FILE *fp = NULL;
	char dev[128];
	int i;

	if ((fp = fopen("/proc/mtd", "r"))) {
		while (fgets(dev, sizeof(dev), fp)) {
			if (sscanf(dev, "mtd%d:", &i) && strstr(dev, mtd)) {
				snprintf(dev, sizeof(dev), "/dev/mtd%d", i);
				fclose(fp);
				return open(dev, flags);
			}
		}
		fclose(fp);
	}

	return -1;
}
#endif

/* the original NAND_FLASH_BLOCK_SIZE is 0x20000 */
#define ORIGINAL_NAND_FLASH_BLOCK_SIZE (0x20000)
static inline unsigned int runtime_flash_block_size(void)
{
#ifndef __KERNEL__
	int fd, ret;
	mtd_info_t mtd_info;
	

	fd = mtd_reservearea_open(RESERVEAREA_NAME, O_RDONLY);
	if(fd < 0) {
		fprintf(stderr, "Couldn't open mtd:%s!\n", RESERVEAREA_NAME);
		return ORIGINAL_NAND_FLASH_BLOCK_SIZE;
	}

	ret = ioctl(fd, MEMGETINFO, &mtd_info);
	close(fd);
	
	if(ret == 0) {
		return mtd_info.erasesize;
	} else {
		fprintf(stderr, "Couldn't ioctl to %s!\n", RESERVEAREA_NAME);
		return ORIGINAL_NAND_FLASH_BLOCK_SIZE;
	}
#else /* __KERNEL__ */
	return ORIGINAL_NAND_FLASH_BLOCK_SIZE;
#endif /* __KERNEL__ */
}
#define NAND_FLASH_BLOCK_SIZE runtime_flash_block_size()  //this define can be changed baccording to type of nandflash (0x10000/0x20000/0x40000)
#else /* defined(TCSUPPORT_CPU_EN7512) || defined(TCSUPPORT_CPU_EN7521) */
#define NAND_FLASH_BLOCK_SIZE (0x20000)  //this define can be changed baccording to type of nandflash (0x10000/0x20000/0x40000)
#endif /* defined(TCSUPPORT_CPU_EN7512) || defined(TCSUPPORT_CPU_EN7521) */

#if defined(TCSUPPORT_CT)
//********************************
//	sectors define of reserve area 	//
//********************************
#define EEPROM_RA_AC_SIZE 			512
#define EEPROM_RA_AC_RESERVE_SIZE	512
#ifdef TCSUPPORT_NAND_FLASH
#define RESERVEAREA_ERASE_SIZE 0x40000 //this define should be changed baccording to  flash erase size
#else
#define RESERVEAREA_ERASE_SIZE 0x10000 //this define should be changed baccording to  flash erase size
#endif
#define RESERVEAREA_BLOCK_BASE 0

//#if defined( TCSUPPORT_RESERVEAREA_1_BLOCK)

//#elif defined( TCSUPPORT_RESERVEAREA_2_BLOCK)

//#elif defined( TCSUPPORT_RESERVEAREA_3_BLOCK)

#if (TCSUPPORT_RESERVEAREA_BLOCK==1)
/*
---------------------------------------------------------------------------------
@reserve area table@
|sector		name				cover area				note		
------------------------------------------------------------------------------------
*/
#elif (TCSUPPORT_RESERVEAREA_BLOCK==2)
/*
---------------------------------------------------------------------------------
@reserve area table@
|sector		name				cover area				note			
------------------------------------------------------------------------------------
*/
#elif (TCSUPPORT_RESERVEAREA_BLOCK==3)
/*
---------------------------------------------------------------------------------
@reserve area table@
|sector		name				cover area				note		
------------------------------------------------------------------------------------
*/
#elif (TCSUPPORT_RESERVEAREA_BLOCK==4)
//defined( TCSUPPORT_RESERVEAREA_BLOCK == 4)
#if defined(TCSUPPORT_CT_E8B_ADSL) && defined(TCSUPPORT_CPU_MT7505)
/*
---------------------------------------------------------------------------------
@reserve area table@
|sector		name				cover area				note
|0			backupromfile			0~0xffff					64k
|1			temp				0x10000~0x1ffff			64k
|a			cerm1				0x10400~0x113ff			4k
|b			cerm2				0x11400~0x123ff			4k
|c			cerm3				0x12400~0x133ff			4k
|d			cerm4				0x13400~0x143ff			4k
|e			username/passwd		0x14400~0x147ff			1K
|2			syslog				0x20000~0x2ffff			64k
|3			eeprom				0x30000~0x303ff			1k(reserve 1k,no use 256 bytes)
|a			proline				0x30400~0x313ff			4k
------------------------------------------------------------------------------------
*/
#define RESERVEAREA_TOTAL_SIZE RESERVEAREA_ERASE_SIZE*4
/*backupromfile*/
#define BACKUPROMFILE_RA_SIZE 0x10000
#define BACKUPROMFILE_RA_OFFSET RESERVEAREA_BLOCK_BASE
/*temp*/
#define TEMP_RA_SIZE 0x10000
#define TEMP_RA_OFFSET  (BACKUPROMFILE_RA_SIZE+BACKUPROMFILE_RA_OFFSET)
/*cerm1*/
#define CERM1_RA_SIZE 0x1000
#define CERM1_RA_OFFSET (TEMP_RA_OFFSET)
/*cerm2*/
#define CERM2_RA_SIZE 0x1000
#define CERM2_RA_OFFSET (CERM1_RA_SIZE+CERM1_RA_OFFSET)
/*cerm3*/
#define CERM3_RA_SIZE 0x1000
#define CERM3_RA_OFFSET (CERM2_RA_SIZE+CERM2_RA_OFFSET)
/*cerm4*/
#define CERM4_RA_SIZE 0x1000
#define CERM4_RA_OFFSET (CERM3_RA_SIZE+CERM3_RA_OFFSET)
#if defined(TCSUPPORT_CT_BOOTLOADER_UPGRADE)
/*username/passwd*/
#define USERNAMEPASSWD_RA_SIZE 0x400
#define USERNAMEPASSWD_RA_OFFSET (CERM4_RA_SIZE+CERM4_RA_OFFSET)
#else
#define USERNAMEPASSWD_RA_SIZE 0x0
#define USERNAMEPASSWD_RA_OFFSET (CERM4_RA_SIZE+CERM4_RA_OFFSET)
#endif
/*syslog*/
#define SYSLOG_RA_SIZE 0x10000
#define SYSLOG_RA_OFFSET (TEMP_RA_SIZE+TEMP_RA_OFFSET)
/*eeprom*/
#define EEPROM_RA_SIZE 0x400
#define EEPROM_RA_OFFSET (SYSLOG_RA_SIZE+SYSLOG_RA_OFFSET)
#ifdef TCSUPPORT_PRODUCTIONLINE
#define PROLINE_CWMPPARA_RA_SIZE 0x1000
#define PROLINE_CWMPPARA_RA_OFFSET (EEPROM_RA_SIZE+EEPROM_RA_OFFSET)
#else
#define PROLINE_CWMPPARA_RA_SIZE 0x0
#define PROLINE_CWMPPARA_RA_OFFSET (EEPROM_RA_SIZE+EEPROM_RA_OFFSET)
#endif

#ifdef TCSUPPORT_MT7570
#define BOB_RA_SIZE 0xa0
#define BOB_RA_OFFSET (PROLINE_CWMPPARA_RA_OFFSET + 0x200)
#endif

#else
/*
---------------------------------------------------------------------------------
@reserve area table@
|sector		name				cover area				note
|0			backupromfile			0~0xffff					64k
|1			defaultromfile			0x10000~0x1ffff			64k
|2			syslog				0x20000~0x2ffff			64k
|3			eeprom				0x30000~0x303ff			1k(reserve 1k,no use 256 bytes)
|4			cerm1				0x30400~0x313ff			4k
|5			cerm2				0x31400~0x323ff			4k
|6			cerm3				0x32400~0x333ff			4k
|7			cerm4				0x33400~0x343ff			4k
|8			username/passwd		0x34400~0x347ff			1K
------------------------------------------------------------------------------------
*/
#define RESERVEAREA_TOTAL_SIZE RESERVEAREA_ERASE_SIZE*4
/*backupromfile*/
#define BACKUPROMFILE_RA_SIZE 0x10000
#define BACKUPROMFILE_RA_OFFSET RESERVEAREA_BLOCK_BASE
/*defaultromfile*/
#define DEFAULTROMFILE_RA_SIZE 0x10000
#define DEFAULTROMFILE_RA_OFFSET (BACKUPROMFILE_RA_OFFSET+BACKUPROMFILE_RA_SIZE)
/*syslog*/
#define SYSLOG_RA_SIZE 0x10000
#define SYSLOG_RA_OFFSET (DEFAULTROMFILE_RA_OFFSET+DEFAULTROMFILE_RA_SIZE)
/*eeprom*/
#define EEPROM_RA_SIZE 0x400
#define EEPROM_RA_OFFSET (SYSLOG_RA_OFFSET+SYSLOG_RA_SIZE)
/*cerm1*/
#define CERM1_RA_SIZE 0x1000
#define CERM1_RA_OFFSET (EEPROM_RA_SIZE+EEPROM_RA_OFFSET)
/*cerm2*/
#define CERM2_RA_SIZE 0x1000
#define CERM2_RA_OFFSET (CERM1_RA_SIZE+CERM1_RA_OFFSET)
/*cerm3*/
#define CERM3_RA_SIZE 0x1000
#define CERM3_RA_OFFSET (CERM2_RA_SIZE+CERM2_RA_OFFSET)
/*cerm4*/
#define CERM4_RA_SIZE 0x1000
#define CERM4_RA_OFFSET (CERM3_RA_SIZE+CERM3_RA_OFFSET)
#if defined(TCSUPPORT_CT_BOOTLOADER_UPGRADE)
/*username/passwd*/
#define USERNAMEPASSWD_RA_SIZE 0x400
#define USERNAMEPASSWD_RA_OFFSET (CERM4_RA_SIZE+CERM4_RA_OFFSET)
#else
#define USERNAMEPASSWD_RA_SIZE 0x0
#define USERNAMEPASSWD_RA_OFFSET (CERM4_RA_SIZE+CERM4_RA_OFFSET)
#endif
#ifdef TCSUPPORT_PRODUCTIONLINE
#if defined(TCSUPPORT_C7)
#define PROLINE_CWMPPARA_RA_SIZE 0x1000
#else
#define PROLINE_CWMPPARA_RA_SIZE 0x180
#endif
#define PROLINE_CWMPPARA_RA_OFFSET (USERNAMEPASSWD_RA_SIZE+USERNAMEPASSWD_RA_OFFSET)
#else
#define PROLINE_CWMPPARA_RA_SIZE 0x0
#define PROLINE_CWMPPARA_RA_OFFSET (USERNAMEPASSWD_RA_SIZE+USERNAMEPASSWD_RA_OFFSET)
#endif

#ifdef TCSUPPORT_MT7570
#define BOB_RA_SIZE 0xa0
#define BOB_RA_OFFSET (PROLINE_CWMPPARA_RA_OFFSET + 0x200)
#endif

#endif
/*==========TCSUPPORT_RESERVEAREA_BLOCK==7========================*/
#elif (TCSUPPORT_RESERVEAREA_BLOCK==7)
//defined( TCSUPPORT_RESERVEAREA_BLOCK ==7)
#ifdef TCSUPPORT_NAND_FLASH
#if defined(TCSUPPORT_CT_PON)
/*
---------------------------------------------------------------------------------
@reserve area table 7 BLOCK@
|sector		name				cover area				note
|1			backupromfile			0~0x3ffff					256k
|2			defaultromfile			0x40000~0x7ffff			256k
|3			syslog				0x80000~0xBffff			256k
|4			proline      			0xc0000~0xfffff			256k
|5			temp				0x100000~0x13ffff			256k
|5.1			cerm1				0x100000~0x100fff		4k
|5.2			cerm2				0x101000~0x101fff		4k
|5.3			cerm3				0x102000~0x102fff		4k
|5.4			cerm4				0x103000~0x103fff		4k
|6			block6				0x140000~0x17ffff			256k
|6.1			eeprom				0x140000~0x1403ff		1k(reserve 1k,no use 256 bytes)
|6.2			bob.conf				0x140400~0x14049f		160bytes
|7			block7				0x180000~0x1bffff			256k
|7.1			imgbootflag			0x180000~0x18003f		64bytes
|7.2			11ac					0x180040~0x18023f		512bytes
|7.3			11ac	 reserved			0x180240~0x18043f		512bytes
------------------------------------------------------------------------------------
*/
#define RESERVEAREA_TOTAL_SIZE RESERVEAREA_ERASE_SIZE*7
/*backupromfile*/
#define BACKUPROMFILE_RA_SIZE 0x40000
#define BACKUPROMFILE_RA_OFFSET RESERVEAREA_BLOCK_BASE
/*defaultromfile*/
#define DEFAULTROMFILE_RA_SIZE 0x40000
#define DEFAULTROMFILE_RA_OFFSET (BACKUPROMFILE_RA_OFFSET+BACKUPROMFILE_RA_SIZE)
/*syslog*/
#define SYSLOG_RA_SIZE 0x40000
#define SYSLOG_RA_OFFSET (DEFAULTROMFILE_RA_OFFSET+DEFAULTROMFILE_RA_SIZE)
/*product para*/
#define PROLINE_CWMPPARA_RA_SIZE 0x40000
#define PROLINE_CWMPPARA_RA_OFFSET (SYSLOG_RA_OFFSET+SYSLOG_RA_SIZE)
/*temp data*/
#define TEMP_RA_SIZE 0x40000
#define TEMP_RA_OFFSET (PROLINE_CWMPPARA_RA_OFFSET+PROLINE_CWMPPARA_RA_SIZE)
/*cerm1*/
#define CERM1_RA_SIZE 0x1000
#define CERM1_RA_OFFSET (TEMP_RA_OFFSET)
/*cerm2*/
#define CERM2_RA_SIZE 0x1000
#define CERM2_RA_OFFSET (CERM1_RA_OFFSET+CERM1_RA_SIZE)
/*cerm3*/
#define CERM3_RA_SIZE 0x1000
#define CERM3_RA_OFFSET (CERM2_RA_OFFSET+CERM2_RA_SIZE)
/*cerm4*/
#define CERM4_RA_SIZE 0x1000
#define CERM4_RA_OFFSET (CERM3_RA_OFFSET+CERM3_RA_SIZE)
/*eeprom*/
#define EEPROM_RA_SIZE 0x400
#define EEPROM_RA_OFFSET (TEMP_RA_OFFSET+TEMP_RA_SIZE)
/*bob info*/
#define BOB_RA_SIZE 0xa0
#define BOB_RA_OFFSET  (EEPROM_RA_OFFSET+EEPROM_RA_SIZE)
/*image boot flag*/
#define IMG_BOOT_FLAG_SIZE 	1
#define IMG_BOOT_FLAG_OFFSET  	(EEPROM_RA_OFFSET + RESERVEAREA_ERASE_SIZE)
#define IMG_BOOT_FLAG_RESERVE_SIZE 		63
#define IMG_BOOT_FLAG_RESERVE_OFFSET	(IMG_BOOT_FLAG_OFFSET + IMG_BOOT_FLAG_SIZE)
#define EEPROM_RA_AC_OFFSET				(IMG_BOOT_FLAG_RESERVE_OFFSET+IMG_BOOT_FLAG_RESERVE_SIZE)
#define EEPROM_RA_AC_RESERVE_OFFSET		(EEPROM_RA_AC_OFFSET+EEPROM_RA_AC_SIZE)
#else
/*
---------------------------------------------------------------------------------
@reserve area table@
|sector		name				cover area				note
|0			backupromfile			0~0xffff					64k
|1			defaultromfile			0x10000~0x1ffff			64k
|2			syslog				0x20000~0x2ffff			64k
|3			proline      			0x30000~0x3ffff			64k
|4			temp				0x40000~0x4ffff			64k
|5			cerm1				0x40000~0x40fff			4k
|6			cerm2				0x41000~0x41fff			4k
|7			cerm3				0x42000~0x42fff			4k
|8			cerm4				0x43000~0x43fff			4k
|9			eeprom				0x50000~0x503ff			1k(reserve 1k,no use 256 bytes)
|10			imgbootflag			0x60000~0x6ffff			64k
------------------------------------------------------------------------------------
*/
#define RESERVEAREA_TOTAL_SIZE RESERVEAREA_ERASE_SIZE*7
/*backupromfile*/
#define BACKUPROMFILE_RA_SIZE 0x10000
#define BACKUPROMFILE_RA_OFFSET RESERVEAREA_BLOCK_BASE
/*defaultromfile*/
#define DEFAULTROMFILE_RA_SIZE 0x10000
#define DEFAULTROMFILE_RA_OFFSET (BACKUPROMFILE_RA_OFFSET+BACKUPROMFILE_RA_SIZE)
/*syslog*/
#define SYSLOG_RA_SIZE 0x10000
#define SYSLOG_RA_OFFSET (DEFAULTROMFILE_RA_OFFSET+DEFAULTROMFILE_RA_SIZE)
/*product para*/
#define PROLINE_CWMPPARA_RA_SIZE 0x10000
#define PROLINE_CWMPPARA_RA_OFFSET (SYSLOG_RA_OFFSET+SYSLOG_RA_SIZE)
/*temp data*/
#define TEMP_RA_SIZE 0x10000
#define TEMP_RA_OFFSET (PROLINE_CWMPPARA_RA_OFFSET+PROLINE_CWMPPARA_RA_SIZE)
/*cerm1*/
#define CERM1_RA_SIZE 0x1000
#define CERM1_RA_OFFSET (TEMP_RA_OFFSET)
/*cerm2*/
#define CERM2_RA_SIZE 0x1000
#define CERM2_RA_OFFSET (CERM1_RA_OFFSET+CERM1_RA_SIZE)
/*cerm3*/
#define CERM3_RA_SIZE 0x1000
#define CERM3_RA_OFFSET (CERM2_RA_OFFSET+CERM2_RA_SIZE)
/*cerm4*/
#define CERM4_RA_SIZE 0x1000
#define CERM4_RA_OFFSET (CERM3_RA_OFFSET+CERM3_RA_SIZE)
/*eeprom*/
#define EEPROM_RA_SIZE 0x400
#define EEPROM_RA_OFFSET (TEMP_RA_OFFSET+TEMP_RA_SIZE)
/*bob info*/
#define BOB_RA_SIZE 0xa0
#define BOB_RA_OFFSET  (EEPROM_RA_OFFSET+EEPROM_RA_SIZE)
/*image boot flag*/
#define IMG_BOOT_FLAG_SIZE 	1
#define IMG_BOOT_FLAG_OFFSET  	(EEPROM_RA_OFFSET + RESERVEAREA_ERASE_SIZE)
#endif
#else
/*
---------------------------------------------------------------------------------
@reserve area table@
|sector		name				cover area				note
|0			backupromfile			0~0xffff					64k
|1			defaultromfile			0x10000~0x1ffff			64k
|2			syslog				0x20000~0x2ffff			64k
|3			proline      			0x30000~0x3ffff			64k
|4			temp				0x40000~0x4ffff			64k
|5			cerm1				0x40000~0x40fff			4k
|6			cerm2				0x41000~0x41fff			4k
|7			cerm3				0x42000~0x42fff			4k
|8			cerm4				0x43000~0x43fff			4k
|9			eeprom				0x50000~0x503ff			1k(reserve 1k,no use 256 bytes)
|10			imgbootflag			0x60000~0x6ffff			64k
------------------------------------------------------------------------------------
*/
#define RESERVEAREA_TOTAL_SIZE RESERVEAREA_ERASE_SIZE*7
/*backupromfile*/
#define BACKUPROMFILE_RA_SIZE 0x10000
#define BACKUPROMFILE_RA_OFFSET RESERVEAREA_BLOCK_BASE
/*defaultromfile*/
#define DEFAULTROMFILE_RA_SIZE 0x10000
#define DEFAULTROMFILE_RA_OFFSET (BACKUPROMFILE_RA_OFFSET+BACKUPROMFILE_RA_SIZE)
/*syslog*/
#define SYSLOG_RA_SIZE 0x10000
#define SYSLOG_RA_OFFSET (DEFAULTROMFILE_RA_OFFSET+DEFAULTROMFILE_RA_SIZE)
/*product para*/
#define PROLINE_CWMPPARA_RA_SIZE 0x10000
#define PROLINE_CWMPPARA_RA_OFFSET (SYSLOG_RA_OFFSET+SYSLOG_RA_SIZE)
/*temp data*/
#define TEMP_RA_SIZE 0x10000
#define TEMP_RA_OFFSET (PROLINE_CWMPPARA_RA_OFFSET+PROLINE_CWMPPARA_RA_SIZE)
/*cerm1*/
#define CERM1_RA_SIZE 0x1000
#define CERM1_RA_OFFSET (TEMP_RA_OFFSET)
/*cerm2*/
#define CERM2_RA_SIZE 0x1000
#define CERM2_RA_OFFSET (CERM1_RA_OFFSET+CERM1_RA_SIZE)
/*cerm3*/
#define CERM3_RA_SIZE 0x1000
#define CERM3_RA_OFFSET (CERM2_RA_OFFSET+CERM2_RA_SIZE)
/*cerm4*/
#define CERM4_RA_SIZE 0x1000
#define CERM4_RA_OFFSET (CERM3_RA_OFFSET+CERM3_RA_SIZE)
/*eeprom*/
#define EEPROM_RA_SIZE 0x400
#define EEPROM_RA_OFFSET (TEMP_RA_OFFSET+TEMP_RA_SIZE)
/*bob info*/
#define BOB_RA_SIZE 0xa0
#define BOB_RA_OFFSET  (EEPROM_RA_OFFSET+EEPROM_RA_SIZE)
/*image boot flag*/
#define IMG_BOOT_FLAG_SIZE 	1
#define IMG_BOOT_FLAG_OFFSET  	(EEPROM_RA_OFFSET + RESERVEAREA_ERASE_SIZE)
#endif
/*==========TCSUPPORT_RESERVEAREA_BLOCK==7========================*/
#else
/*==========TCSUPPORT_RESERVEAREA_BLOCK==9========================*/
//defined( TCSUPPORT_RESERVEAREA_BLOCK ==9)
#if defined(MT7615E)|| defined(MT7615_11N) || defined(MT7613E)|| defined(MT7915D) || defined(MT7915N)|| defined(MT7915E) || defined(MT7916D) || defined(MT7916N)|| defined(MT7916E)
#ifdef TCSUPPORT_NAND_FLASH
#if defined(TCSUPPORT_CT_PON)
/*
---------------------------------------------------------------------------------
@reserve area table 9 BLOCK@
|sector		name				cover area				note
|1.1			eeprom(11n part)					0~0x0fff					4k
|1.2			TRX_SET1_SET2(11n part)			0x1000~0x9fff				36k
|1.3			eeprom(7915 2.4G) 				0xa000~0xafff			4k																	//0~2 for 11n
|1.4			Reserved 						0xb000~0x3ffff			212k																	
|2.1			eeprom(11ac part)					0x40000~0x40fff			4k
|2.2			TRX_SET1_SET2(11ac part)			0x41000~0x49fff			36k
|2.3			Reserved 						0x4a000~0x4afff		4k
//------------------7613 EEPROM+PRECAL & 7915 EEPROM+PRECAL shared 28k
|2.4			eeprom(7613)+PRE-CAL  						0x4b000~0x52fff		(28+4)k 
|2.5			eeprom(7915 5G)+PRE-CAL 				0x4c000~0x4cfff		(28+100)k  
|2.6			Reserved 						0x4d000~0x7ffff			80k
|3			backupromfile						0x80000~0xbffff			256k
|4			defaultromfile						0xc0000~0xfffff			256k
|5			syslog							0x100000~0x13ffff			256k
|6			proline      						0x140000~0x17ffff			256k
|7			temp							0x180000~0x1bffff			256k
|7.1			cerm1							0x180000~0x180fff			4k
|7.2			cerm2							0x181000~0x181fff			4k
|7.3			cerm3							0x182000~0x182fff			4k
|7.4			cerm4							0x183000~0x183fff			4k
|8			block8							0x1c0000~0x1fffff			256k
|8.1			eeprom							0x1c0000~0x1c03ff			1k(reserve 1k,no use 256 bytes)
|8.2			bob.conf							0x1c0400~0x1c049f		160bytes
|9			block9							0x200000~0x23ffff			256k
|9.1			imgbootflag						0x200000~0x20003f		64bytes
|9.2			11ac								0x200040~0x20023f		512bytes
|9.3			11ac reserved						0x200240~0x20043f		512bytes
------------------------------------------------------------------------------------
*/
#define RESERVEAREA_TOTAL_SIZE RESERVEAREA_ERASE_SIZE*9

/*11n part*/
#define EEPROM_761511N_RA_SIZE 0x1000
#define EEPROM_761511N_RA_OFFSET RESERVEAREA_BLOCK_BASE
#define TRX_SET1_SET2_11N_RA_SIZE 0x9000
#define TRX_SET1_SET2_11N_RA_OFFSET (EEPROM_761511N_RA_OFFSET+EEPROM_761511N_RA_SIZE)

/*7915N &7915E(4X4+4X4)part*/
/*NOTICE:This define is only for 7915N+7915E*/
#define EEPROM_791511N_RA_SIZE 0x40000
#define EEPROM_791511N_RA_OFFSET RESERVEAREA_BLOCK_BASE

/*7916N &7915E(4X4+4X4)part*/
/*NOTICE:This define is only for 7916N+7916E*/
#define EEPROM_791611N_RA_SIZE 0x40000
#define EEPROM_791611N_RA_OFFSET RESERVEAREA_BLOCK_BASE

/*11ac part*/
#define EEPROM_761511AC_RA_SIZE 0x1000
#define EEPROM_RA_761511AC_OFFSET (EEPROM_761511N_RA_OFFSET+RESERVEAREA_ERASE_SIZE)
#define TRX_SET1_SET2_11AC_RA_SIZE 0x9000
#define TRX_SET1_SET2_11AC_RA_OFFSET (EEPROM_RA_761511AC_OFFSET+EEPROM_761511AC_RA_SIZE)
#define RESERVED_761511AC_SIZE 0x1000

/*7613 part*/
#define EEPROM_761311AC_RA_SIZE 0x1000
#define EEPROM_RA_761311AC_OFFSET (TRX_SET1_SET2_11AC_RA_OFFSET+TRX_SET1_SET2_11AC_RA_SIZE+RESERVED_761511AC_SIZE)

/*7915 5G part*/
#define EEPROM_791511AC_RA_SIZE 0x20000    //FOR MT77915,need 0x20000 FOR EEPROM+PRECAL DATA
#define EEPROM_RA_791511AC_OFFSET (EEPROM_RA_761311AC_OFFSET+EEPROM_761311AC_RA_SIZE)

/*7916D part*/
#define EEPROM_791611AC_RA_SIZE 0x20000    //FOR MT7916,need 0x20000 FOR EEPROM+PRECAL DATA
#define EEPROM_RA_791611AC_OFFSET (EEPROM_RA_761311AC_OFFSET+EEPROM_761311AC_RA_SIZE)

/*backupromfile*/
#define BACKUPROMFILE_RA_SIZE 0x40000
#define BACKUPROMFILE_RA_OFFSET (EEPROM_RA_761511AC_OFFSET+RESERVEAREA_ERASE_SIZE)

/*defaultromfile*/
#define DEFAULTROMFILE_RA_SIZE 0x40000
#define DEFAULTROMFILE_RA_OFFSET (BACKUPROMFILE_RA_OFFSET+BACKUPROMFILE_RA_SIZE)
/*syslog*/
#define SYSLOG_RA_SIZE 0x40000
#define SYSLOG_RA_OFFSET (DEFAULTROMFILE_RA_OFFSET+DEFAULTROMFILE_RA_SIZE)
/*product para*/
#define PROLINE_CWMPPARA_RA_SIZE 0x40000
#define PROLINE_CWMPPARA_RA_OFFSET (SYSLOG_RA_OFFSET+SYSLOG_RA_SIZE)
/*temp data*/
#define TEMP_RA_SIZE 0x40000
#define TEMP_RA_OFFSET (PROLINE_CWMPPARA_RA_OFFSET+PROLINE_CWMPPARA_RA_SIZE)
/*cerm1*/
#define CERM1_RA_SIZE 0x1000
#define CERM1_RA_OFFSET (TEMP_RA_OFFSET)
/*cerm2*/
#define CERM2_RA_SIZE 0x1000
#define CERM2_RA_OFFSET (CERM1_RA_OFFSET+CERM1_RA_SIZE)
/*cerm3*/
#define CERM3_RA_SIZE 0x1000
#define CERM3_RA_OFFSET (CERM2_RA_OFFSET+CERM2_RA_SIZE)
/*cerm4*/
#define CERM4_RA_SIZE 0x1000
#define CERM4_RA_OFFSET (CERM3_RA_OFFSET+CERM3_RA_SIZE)
/*eeprom*/
#define EEPROM_RA_SIZE 0x400
#define EEPROM_RA_OFFSET (TEMP_RA_OFFSET+TEMP_RA_SIZE)
/*bob info*/
#define BOB_RA_SIZE 0xa0
#define BOB_RA_OFFSET  (EEPROM_RA_OFFSET+EEPROM_RA_SIZE)
/*image boot flag*/
#define IMG_BOOT_FLAG_SIZE 	1
#define IMG_BOOT_FLAG_OFFSET  	(EEPROM_RA_OFFSET + RESERVEAREA_ERASE_SIZE)
#define IMG_BOOT_FLAG_RESERVE_SIZE 		63
#define IMG_BOOT_FLAG_RESERVE_OFFSET	(IMG_BOOT_FLAG_OFFSET + IMG_BOOT_FLAG_SIZE)
/*eeprom ac*/
#define EEPROM_RA_AC_OFFSET				(IMG_BOOT_FLAG_RESERVE_OFFSET+IMG_BOOT_FLAG_RESERVE_SIZE)
#define EEPROM_RA_AC_RESERVE_OFFSET		(EEPROM_RA_AC_OFFSET+EEPROM_RA_AC_SIZE)
#else
/*
---------------------------------------------------------------------------------
@reserve area table 9 BLOCK@
|sector		name				cover area				note
|1.1			eeprom(11n part)					0~0x0fff					4k
|1.2			TRX_SET1_SET2(11n part)			0x1000~0x9fff				36k
|1.3			Reserved 						0xa000~0xffff			       24k																	//0~2 for 11n
|2.1			eeprom(11ac part)					0x10000~0x10fff			4k
|2.2			TRX_SET1_SET2(11ac part)			0x11000~0x19fff			36k
|2.3			Reserved 						0x1a000~0x1afff			4k
|2.4			eeprom(7613)  					0x1b000~0x1bfff			4k
|2.5			Reserved 						0x1c000~0x1ffff			16k
|3			backupromfile						0x20000~0x2ffff			64k
|4			defaultromfile						0x30000~0x3ffff			64k
|5			syslog							0x40000~0x4ffff			64k
|6			proline      						0x50000~0x5ffff			64k
|7			temp							0x60000~0x6ffff			64k
|7.1			cerm1							0x60000~0x60fff			4k
|7.2			cerm2							0x61000~0x61fff			4k
|7.3			cerm3							0x62000~0x62fff			4k
|7.4			cerm4							0x63000~0x63fff			4k
|8			block8							0x70000~0x7ffff			64k
|8.1			eeprom							0x70000~0x703ff			1k(reserve 1k,no use 256 bytes)
|8.2			bob.conf							0x70400~0x7049f		       160bytes
|9			block9							0x80000~0x8ffff			64k
|9.1			imgbootflag						0x80000~0x80001		       1bytes
------------------------------------------------------------------------------------
*/
#define RESERVEAREA_TOTAL_SIZE RESERVEAREA_ERASE_SIZE*9

/*11n part*/
#define EEPROM_761511N_RA_SIZE 0x1000
#define EEPROM_761511N_RA_OFFSET RESERVEAREA_BLOCK_BASE
#define TRX_SET1_SET2_11N_RA_SIZE 0x9000
#define TRX_SET1_SET2_11N_RA_OFFSET (EEPROM_761511N_RA_OFFSET+EEPROM_761511N_RA_SIZE)

/*7915N &7915E(4X4+4X4)part*/
/*NOTICE:This define is only for 7915N+7915E*/
#define EEPROM_791511N_RA_SIZE 0x40000
#define EEPROM_791511N_RA_OFFSET RESERVEAREA_BLOCK_BASE

/*11ac part*/
#define EEPROM_761511AC_RA_SIZE 0x1000
#define EEPROM_RA_761511AC_OFFSET (EEPROM_761511N_RA_OFFSET+RESERVEAREA_ERASE_SIZE)
#define TRX_SET1_SET2_11AC_RA_SIZE 0x9000
#define TRX_SET1_SET2_11AC_RA_OFFSET (EEPROM_RA_761511AC_OFFSET+EEPROM_761511AC_RA_SIZE)
#define RESERVED_761511AC_SIZE 0x1000

/*7613 part*/
#define EEPROM_761311AC_RA_SIZE 0x1000
#define EEPROM_RA_761311AC_OFFSET (TRX_SET1_SET2_11AC_RA_OFFSET+TRX_SET1_SET2_11AC_RA_SIZE+RESERVED_761511AC_SIZE)

/*7915 5G part*/
#define EEPROM_791511AC_RA_SIZE 0x1000
#define EEPROM_RA_791511AC_OFFSET (EEPROM_RA_761311AC_OFFSET+EEPROM_761311AC_RA_SIZE)


/*backupromfile*/
#define BACKUPROMFILE_RA_SIZE 0x10000
#define BACKUPROMFILE_RA_OFFSET (EEPROM_RA_761511AC_OFFSET+RESERVEAREA_ERASE_SIZE)

/*defaultromfile*/
#define DEFAULTROMFILE_RA_SIZE 0x10000
#define DEFAULTROMFILE_RA_OFFSET (BACKUPROMFILE_RA_OFFSET+BACKUPROMFILE_RA_SIZE)
/*syslog*/
#define SYSLOG_RA_SIZE 0x10000
#define SYSLOG_RA_OFFSET (DEFAULTROMFILE_RA_OFFSET+DEFAULTROMFILE_RA_SIZE)
/*product para*/
#define PROLINE_CWMPPARA_RA_SIZE 0x10000
#define PROLINE_CWMPPARA_RA_OFFSET (SYSLOG_RA_OFFSET+SYSLOG_RA_SIZE)
/*temp data*/
#define TEMP_RA_SIZE 0x10000
#define TEMP_RA_OFFSET (PROLINE_CWMPPARA_RA_OFFSET+PROLINE_CWMPPARA_RA_SIZE)
/*cerm1*/
#define CERM1_RA_SIZE 0x1000
#define CERM1_RA_OFFSET (TEMP_RA_OFFSET)
/*cerm2*/
#define CERM2_RA_SIZE 0x1000
#define CERM2_RA_OFFSET (CERM1_RA_OFFSET+CERM1_RA_SIZE)
/*cerm3*/
#define CERM3_RA_SIZE 0x1000
#define CERM3_RA_OFFSET (CERM2_RA_OFFSET+CERM2_RA_SIZE)
/*cerm4*/
#define CERM4_RA_SIZE 0x1000
#define CERM4_RA_OFFSET (CERM3_RA_OFFSET+CERM3_RA_SIZE)
/*eeprom*/
#define EEPROM_RA_SIZE 0x400
#define EEPROM_RA_OFFSET (TEMP_RA_OFFSET+TEMP_RA_SIZE)
/*bob info*/
#define BOB_RA_SIZE 0xa0
#define BOB_RA_OFFSET  (EEPROM_RA_OFFSET+EEPROM_RA_SIZE)
/*image boot flag*/
#define IMG_BOOT_FLAG_SIZE 	1
#define IMG_BOOT_FLAG_OFFSET  	(EEPROM_RA_OFFSET + RESERVEAREA_ERASE_SIZE)
#endif
#else
/*
---------------------------------------------------------------------------------
@reserve area table 9 BLOCK@
|sector		name				cover area				note
|1.1			eeprom(11n part)					0~0x0fff					4k
|1.2			TRX_SET1_SET2(11n part)			0x1000~0x9fff				36k
|1.3			Reserved 						0xa000~0xffff			       24k																	//0~2 for 11n
|2.1			eeprom(11ac part)					0x10000~0x10fff			4k
|2.2			TRX_SET1_SET2(11ac part)			0x11000~0x19fff			36k
|2.3			Reserved 						0x1a000~0x1afff			4k
|2.4			eeprom(7613)  					0x1b000~0x1bfff			4k
|2.5			Reserved 						0x1c000~0x1ffff			16k
|3			backupromfile						0x20000~0x2ffff			64k
|4			defaultromfile						0x30000~0x3ffff			64k
|5			syslog							0x40000~0x4ffff			64k
|6			proline      						0x50000~0x5ffff			64k
|7			temp							0x60000~0x6ffff			64k
|7.1			cerm1							0x60000~0x60fff			4k
|7.2			cerm2							0x61000~0x61fff			4k
|7.3			cerm3							0x62000~0x62fff			4k
|7.4			cerm4							0x63000~0x63fff			4k
|8			block8							0x70000~0x7ffff			64k
|8.1			eeprom							0x70000~0x703ff			1k(reserve 1k,no use 256 bytes)
|8.2			bob.conf							0x70400~0x7049f		       160bytes
|9			block9							0x80000~0x8ffff			64k
|9.1			imgbootflag						0x80000~0x80001		       1bytes
------------------------------------------------------------------------------------
*/
#define RESERVEAREA_TOTAL_SIZE RESERVEAREA_ERASE_SIZE*9

/*11n part*/
#define EEPROM_761511N_RA_SIZE 0x1000
#define EEPROM_761511N_RA_OFFSET RESERVEAREA_BLOCK_BASE
#define TRX_SET1_SET2_11N_RA_SIZE 0x9000
#define TRX_SET1_SET2_11N_RA_OFFSET (EEPROM_761511N_RA_OFFSET+EEPROM_761511N_RA_SIZE)

/*11ac part*/
#define EEPROM_761511AC_RA_SIZE 0x1000
#define EEPROM_RA_761511AC_OFFSET (EEPROM_761511N_RA_OFFSET+RESERVEAREA_ERASE_SIZE)
#define TRX_SET1_SET2_11AC_RA_SIZE 0x9000
#define TRX_SET1_SET2_11AC_RA_OFFSET (EEPROM_RA_761511AC_OFFSET+EEPROM_761511AC_RA_SIZE)
#define RESERVED_761511AC_SIZE 0x1000

/*7613 part*/
#define EEPROM_761311AC_RA_SIZE 0x1000
#define EEPROM_RA_761311AC_OFFSET (TRX_SET1_SET2_11AC_RA_OFFSET+TRX_SET1_SET2_11AC_RA_SIZE+RESERVED_761511AC_SIZE)

/*backupromfile*/
#define BACKUPROMFILE_RA_SIZE 0x10000
#define BACKUPROMFILE_RA_OFFSET (EEPROM_RA_761511AC_OFFSET+RESERVEAREA_ERASE_SIZE)

/*defaultromfile*/
#define DEFAULTROMFILE_RA_SIZE 0x10000
#define DEFAULTROMFILE_RA_OFFSET (BACKUPROMFILE_RA_OFFSET+BACKUPROMFILE_RA_SIZE)
/*syslog*/
#define SYSLOG_RA_SIZE 0x10000
#define SYSLOG_RA_OFFSET (DEFAULTROMFILE_RA_OFFSET+DEFAULTROMFILE_RA_SIZE)
/*product para*/
#define PROLINE_CWMPPARA_RA_SIZE 0x10000
#define PROLINE_CWMPPARA_RA_OFFSET (SYSLOG_RA_OFFSET+SYSLOG_RA_SIZE)
/*temp data*/
#define TEMP_RA_SIZE 0x10000
#define TEMP_RA_OFFSET (PROLINE_CWMPPARA_RA_OFFSET+PROLINE_CWMPPARA_RA_SIZE)
/*cerm1*/
#define CERM1_RA_SIZE 0x1000
#define CERM1_RA_OFFSET (TEMP_RA_OFFSET)
/*cerm2*/
#define CERM2_RA_SIZE 0x1000
#define CERM2_RA_OFFSET (CERM1_RA_OFFSET+CERM1_RA_SIZE)
/*cerm3*/
#define CERM3_RA_SIZE 0x1000
#define CERM3_RA_OFFSET (CERM2_RA_OFFSET+CERM2_RA_SIZE)
/*cerm4*/
#define CERM4_RA_SIZE 0x1000
#define CERM4_RA_OFFSET (CERM3_RA_OFFSET+CERM3_RA_SIZE)
/*eeprom*/
#define EEPROM_RA_SIZE 0x400
#define EEPROM_RA_OFFSET (TEMP_RA_OFFSET+TEMP_RA_SIZE)
/*bob info*/
#define BOB_RA_SIZE 0xa0
#define BOB_RA_OFFSET  (EEPROM_RA_OFFSET+EEPROM_RA_SIZE)
/*image boot flag*/
#define IMG_BOOT_FLAG_SIZE 	1
#define IMG_BOOT_FLAG_OFFSET  	(EEPROM_RA_OFFSET + RESERVEAREA_ERASE_SIZE)
#endif

#endif
/*==========TCSUPPORT_RESERVEAREA_BLOCK==9========================*/

#endif
#else
//********************************
//	sectors define of reserve area 	//
//********************************
#ifdef TCSUPPORT_NAND_BMT
#define RESERVEAREA_ERASE_SIZE NAND_FLASH_BLOCK_SIZE //this define should be changed baccording to  flash erase size  
#else
#define RESERVEAREA_ERASE_SIZE 0x10000 //this define should be changed baccording to  flash erase size
#endif
#define RESERVEAREA_BLOCK_BASE 0

//#if defined( TCSUPPORT_RESERVEAREA_1_BLOCK)

//#elif defined( TCSUPPORT_RESERVEAREA_2_BLOCK)

//#elif defined( TCSUPPORT_RESERVEAREA_3_BLOCK)

#if (TCSUPPORT_RESERVEAREA_BLOCK==1)
/*
---------------------------------------------------------------------------------
@reserve area table@
|sector		name				cover area				note		
|0			eeprom				0x00000~0x003ff			1k(reserve 1k,no use 256 bytes)
|1			cwmppara			0x00400~0x00600			512 Bytes
|2			not use now			0x00601~0x00fff			(4k-1k-512 Bytes)
|3			mrd					0x01000~0x01fff			4k
|4			rom-t				0x02000~0x0ffff			56k
------------------------------------------------------------------------------------
*/
#define RESERVEAREA_TOTAL_SIZE RESERVEAREA_ERASE_SIZE*1

/*eeprom*/
#define EEPROM_RA_SIZE 0x400
#define EEPROM_RA_OFFSET RESERVEAREA_BLOCK_BASE 

/*device information*/
#ifdef TCSUPPORT_PRODUCTIONLINE
#define PROLINE_CWMPPARA_RA_SIZE 0x200
#define PROLINE_CWMPPARA_RA_OFFSET (EEPROM_RA_OFFSET+EEPROM_RA_SIZE)
#else
#define PROLINE_CWMPPARA_RA_SIZE 0x0
#define PROLINE_CWMPPARA_RA_OFFSET (EEPROM_RA_OFFSET+EEPROM_RA_SIZE)
#endif

#ifdef TCSUPPORT_MT7570
#define BOB_RA_SIZE 0xa0
#define BOB_RA_OFFSET (PROLINE_CWMPPARA_RA_OFFSET + 0x200)
#endif

#define MRD_RA_SIZE 0x1000
#define MRD_RA_OFFSET (EEPROM_RA_OFFSET + 0x1000) 

#define ROM_T_RA_SIZE 0xe000
#define ROM_T_RA_OFFSET (MRD_RA_OFFSET+MRD_RA_SIZE)

#elif (TCSUPPORT_RESERVEAREA_BLOCK==2)
/*
---------------------------------------------------------------------------------
@reserve area table@
|sector		name				cover area				note
|0			backupromfile 		0~0xffff					64k
|1			syslog				0x10000~0x10000			
|2			eeprom				0x10000~0x103ff			1k(reserve 1k,no use 256 bytes)
|3			cwmppara			0x00400~0x00600			512 Bytes
|4			not use now			0x00601~0x00fff			(4k-1k-512 Bytes)
|5			mrd				0x01000~0x01fff			4k
|6			rom-t				0x02000~0x0ffff			56k
------------------------------------------------------------------------------------
*/
#define RESERVEAREA_TOTAL_SIZE RESERVEAREA_ERASE_SIZE*2
/*backupromfile*/
#define BACKUPROMFILE_RA_SIZE 0x10000
#define BACKUPROMFILE_RA_OFFSET RESERVEAREA_BLOCK_BASE
/*defaultromfile*/
//#define DEFAULTROMFILE_RA_SIZE 0x0
//#define DEFAULTROMFILE_RA_OFFSET (BACKUPROMFILE_RA_OFFSET+BACKUPROMFILE_RA_SIZE)
/*syslog*/
#define SYSLOG_RA_SIZE 0x0
//#define SYSLOG_RA_OFFSET (DEFAULTROMFILE_RA_OFFSET+DEFAULTROMFILE_RA_SIZE)
#define SYSLOG_RA_OFFSET (BACKUPROMFILE_RA_OFFSET+BACKUPROMFILE_RA_SIZE)
/*eeprom*/
#define EEPROM_RA_SIZE 0x400
#define EEPROM_RA_OFFSET (SYSLOG_RA_OFFSET+SYSLOG_RA_SIZE)

/*device information*/
#ifdef TCSUPPORT_PRODUCTIONLINE
#define PROLINE_CWMPPARA_RA_SIZE 0x200
#define PROLINE_CWMPPARA_RA_OFFSET (EEPROM_RA_OFFSET+EEPROM_RA_SIZE)
#else
#define PROLINE_CWMPPARA_RA_SIZE 0x0
#define PROLINE_CWMPPARA_RA_OFFSET (EEPROM_RA_OFFSET+EEPROM_RA_SIZE)
#endif

#ifdef TCSUPPORT_MT7570
#define BOB_RA_SIZE 0xa0
#define BOB_RA_OFFSET (PROLINE_CWMPPARA_RA_OFFSET + 0x200)
#endif

#define MRD_RA_SIZE 0x1000
#define MRD_RA_OFFSET (EEPROM_RA_OFFSET + 0x1000)

#define ROM_T_RA_SIZE 0xe000
#define ROM_T_RA_OFFSET (MRD_RA_OFFSET+MRD_RA_SIZE)

#elif (TCSUPPORT_RESERVEAREA_BLOCK==3)
/*
---------------------------------------------------------------------------------
@reserve area table@
|sector		name				cover area				note
|0			backupromfile 		0~0xffff					64k
|1			defaultromfile		0x10000~0x1ffff			64k
|2			eeprom			0x20000~0x203ff			1k(reserve 1k,no use 256 bytes)
|3			cwmppara			0x00400~0x00600			384 Bytes
|4			not use now			0x00601~0x00fff			(4k-1k-512 Bytes)
|5			mrd					0x01000~0x01fff			4k
|6			rom-t				0x02000~0x0ffff			56k
------------------------------------------------------------------------------------
*/
#define RESERVEAREA_TOTAL_SIZE RESERVEAREA_ERASE_SIZE*3
/*backupromfile*/
#define BACKUPROMFILE_RA_SIZE 0x10000
#define BACKUPROMFILE_RA_OFFSET RESERVEAREA_BLOCK_BASE
/*defaultromfile*/
#define DEFAULTROMFILE_RA_SIZE 0x10000
#define DEFAULTROMFILE_RA_OFFSET (BACKUPROMFILE_RA_OFFSET+BACKUPROMFILE_RA_SIZE)
/*eeprom*/
#define EEPROM_RA_SIZE 0x400
#define EEPROM_RA_OFFSET (DEFAULTROMFILE_RA_SIZE+DEFAULTROMFILE_RA_OFFSET)

/*device information*/
#ifdef TCSUPPORT_PRODUCTIONLINE
#define PROLINE_CWMPPARA_RA_SIZE 0x200
#define PROLINE_CWMPPARA_RA_OFFSET (EEPROM_RA_OFFSET+EEPROM_RA_SIZE)
#else
#define PROLINE_CWMPPARA_RA_SIZE 0x0
#define PROLINE_CWMPPARA_RA_OFFSET (EEPROM_RA_OFFSET+EEPROM_RA_SIZE)
#endif

#ifdef TCSUPPORT_MT7570
#define BOB_RA_SIZE 0xa0
#define BOB_RA_OFFSET (PROLINE_CWMPPARA_RA_OFFSET + 0x200)
#endif

#define MRD_RA_SIZE 0x1000
#define MRD_RA_OFFSET (EEPROM_RA_OFFSET + 0x1000)

#define ROM_T_RA_SIZE 0xe000
#define ROM_T_RA_OFFSET (MRD_RA_OFFSET+MRD_RA_SIZE)

//#else
#elif (TCSUPPORT_RESERVEAREA_BLOCK==4)
//defined( TCSUPPORT_RESERVEAREA_BLOCK == 4)
#ifdef TCSUPPORT_NAND_BMT

/*
---------------------------------------------------------------------------------
@reserve area table@
|sector		name				cover area				note
|0			backupromfile 		0~0x1ffff					128k
|1			defaultromfile			0x20000~0x3ffff			128k
|2			syslog				0x40000~0x5ffff			128k
|3			eeprom				0x60000~0x603ff			1k(reserve 1k,no use 256 bytes)
|4			cwmppara			0x60400~0x60600			512 Bytes
|5			not use now			0x60601~0x60fff			(4k-1k-512 Bytes)
|6			mrd					0x61000~0x61fff			4k
|7			rom-t				0x62000~0x6ffff			56k
|8			CA1-CA4				0x70000~0x73fff			16k
------------------------------------------------------------------------------------
*/
#ifdef TCSUPPORT_NOR_FLASH_USED
#define RESERVEAREA_ERASE_SIZE 0x10000
#endif

#define RESERVEAREA_TOTAL_SIZE RESERVEAREA_ERASE_SIZE*4
#ifdef TCSUPPORT_WLAN_AC
#define MAX_EEPROM_BIN_FILE_SIZE	512
#endif
/*backupromfile*/
#define BACKUPROMFILE_RA_SIZE RESERVEAREA_ERASE_SIZE
#define BACKUPROMFILE_RA_OFFSET RESERVEAREA_BLOCK_BASE
/*defaultromfile*/
#define DEFAULTROMFILE_RA_SIZE RESERVEAREA_ERASE_SIZE
#define DEFAULTROMFILE_RA_OFFSET (BACKUPROMFILE_RA_OFFSET+BACKUPROMFILE_RA_SIZE)
/*syslog*/
#define SYSLOG_RA_SIZE RESERVEAREA_ERASE_SIZE
#define SYSLOG_RA_OFFSET (DEFAULTROMFILE_RA_OFFSET+DEFAULTROMFILE_RA_SIZE)
/*eeprom*/
#define EEPROM_RA_SIZE 0x400
#define EEPROM_RA_OFFSET (SYSLOG_RA_OFFSET+SYSLOG_RA_SIZE)
#ifdef TCSUPPORT_WLAN_AC
#ifndef TCSUPPORT_EEPROM_ACEXT
#define EEPROM_RA_AC_OFFSET (EEPROM_RA_OFFSET+MAX_EEPROM_BIN_FILE_SIZE)
#endif
#endif

/*device information*/
#ifdef TCSUPPORT_PRODUCTIONLINE
#define PROLINE_CWMPPARA_RA_SIZE 0x200
#define PROLINE_CWMPPARA_RA_OFFSET (EEPROM_RA_OFFSET+EEPROM_RA_SIZE)
#else
#define PROLINE_CWMPPARA_RA_SIZE 0x0
#define PROLINE_CWMPPARA_RA_OFFSET (EEPROM_RA_OFFSET+EEPROM_RA_SIZE)
#endif

#ifdef TCSUPPORT_MT7570
#define BOB_RA_SIZE 0xa0
#define BOB_RA_OFFSET (PROLINE_CWMPPARA_RA_OFFSET + 0x200)
#endif

/*for MT7615 use cause the size of MT7615 EEPROM is 1k*/
#ifdef TCSUPPORT_WLAN_AC
#ifdef TCSUPPORT_EEPROM_ACEXT   
#define EEPROM_RA_AC_OFFSET (PROLINE_CWMPPARA_RA_OFFSET+PROLINE_CWMPPARA_RA_SIZE)
#endif
#endif

#define MRD_RA_SIZE 0x1000
#define MRD_RA_OFFSET (EEPROM_RA_OFFSET + 0x1000)

#define ROM_T_RA_SIZE 0xe000
#define ROM_T_RA_OFFSET (MRD_RA_OFFSET+MRD_RA_SIZE)
#ifdef TCSUPPORT_GOOGLE_FIBER
/*overwrite syslog area as google fiber  SFU does not include syslog function*/
/*cerm1*/
#define OMCI_RA_SIZE 0x1000
#define CERM1_RA_SIZE 0x1000
#define CERM1_RA_OFFSET (SYSLOG_RA_OFFSET+OMCI_RA_SIZE)
/*cerm2*/
#define CERM2_RA_SIZE 0x1000
#define CERM2_RA_OFFSET (CERM1_RA_OFFSET+CERM1_RA_SIZE)
/*cerm3*/
#define CERM3_RA_SIZE 0x1000
#define CERM3_RA_OFFSET (CERM2_RA_OFFSET+CERM2_RA_SIZE)
/*cerm4*/
#define CERM4_RA_SIZE 0x1000
#define CERM4_RA_OFFSET (CERM3_RA_OFFSET+CERM3_RA_SIZE)
#else
/*cerm1*/
#define CERM1_RA_SIZE 0x1000
#define CERM1_RA_OFFSET (ROM_T_RA_OFFSET+ROM_T_RA_SIZE)
/*cerm2*/
#define CERM2_RA_SIZE 0x1000
#define CERM2_RA_OFFSET (CERM1_RA_OFFSET+CERM1_RA_SIZE)
/*cerm3*/
#define CERM3_RA_SIZE 0x1000
#define CERM3_RA_OFFSET (CERM2_RA_OFFSET+CERM2_RA_SIZE)
/*cerm4*/
#define CERM4_RA_SIZE 0x1000
#define CERM4_RA_OFFSET (CERM3_RA_OFFSET+CERM3_RA_SIZE)
#endif /*end else of googlefiber*/

/*image boot flag*/
#define IMG_BOOT_FLAG_SIZE 	1
#ifdef TCSUPPORT_NOR_FLASH_USED
#define IMG_BOOT_FLAG_OFFSET  	SYSLOG_RA_OFFSET
#else
#define IMG_BOOT_FLAG_OFFSET  (MRD_RA_OFFSET - IMG_BOOT_FLAG_SIZE)
#endif

#else

/*
---------------------------------------------------------------------------------
@reserve area table@
|sector		name				cover area				note
|0			backupromfile 		0~0xffff					64k
|1			defaultromfile			0x10000~0x1ffff			64k
|2			syslog				0x20000~0x2ffff			64k
|3			eeprom				0x30000~0x303ff			1k(reserve 1k,no use 256 bytes)
|4			cwmppara			0x00400~0x00600			512 Bytes
|5			not use now			0x00601~0x00fff			(4k-1k-512 Bytes)
|6			mrd					0x01000~0x01fff			4k
|7			rom-t				0x02000~0x0ffff			56k
------------------------------------------------------------------------------------
*/
#define RESERVEAREA_TOTAL_SIZE RESERVEAREA_ERASE_SIZE*4
#ifdef TCSUPPORT_WLAN_AC
#define MAX_EEPROM_BIN_FILE_SIZE	512
#endif
/*backupromfile*/
#define BACKUPROMFILE_RA_SIZE 0x10000
#define BACKUPROMFILE_RA_OFFSET RESERVEAREA_BLOCK_BASE
/*defaultromfile*/
#define DEFAULTROMFILE_RA_SIZE 0x10000
#define DEFAULTROMFILE_RA_OFFSET (BACKUPROMFILE_RA_OFFSET+BACKUPROMFILE_RA_SIZE)
/*syslog*/
#define SYSLOG_RA_SIZE 0x10000
#define SYSLOG_RA_OFFSET (DEFAULTROMFILE_RA_OFFSET+DEFAULTROMFILE_RA_SIZE)
/*eeprom*/
#define EEPROM_RA_SIZE 0x400
#define EEPROM_RA_OFFSET (SYSLOG_RA_OFFSET+SYSLOG_RA_SIZE)
#ifdef TCSUPPORT_WLAN_AC
#ifndef TCSUPPORT_EEPROM_ACEXT
#define EEPROM_RA_AC_OFFSET (EEPROM_RA_OFFSET+MAX_EEPROM_BIN_FILE_SIZE)
#endif
#endif

/*device information*/
#ifdef TCSUPPORT_PRODUCTIONLINE
#define PROLINE_CWMPPARA_RA_SIZE 0x200
#define PROLINE_CWMPPARA_RA_OFFSET (EEPROM_RA_OFFSET+EEPROM_RA_SIZE)
#else
#define PROLINE_CWMPPARA_RA_SIZE 0x0
#define PROLINE_CWMPPARA_RA_OFFSET (EEPROM_RA_OFFSET+EEPROM_RA_SIZE)
#endif

#ifdef TCSUPPORT_MT7570
#define BOB_RA_SIZE 0xa0
#define BOB_RA_OFFSET (PROLINE_CWMPPARA_RA_OFFSET + 0x200)
#endif

/*for MT7615 use cause the size of MT7615 EEPROM is 1k*/
#ifdef TCSUPPORT_WLAN_AC
#ifdef TCSUPPORT_EEPROM_ACEXT   
#define EEPROM_RA_AC_OFFSET (PROLINE_CWMPPARA_RA_OFFSET+PROLINE_CWMPPARA_RA_SIZE)
#endif
#endif

#define MRD_RA_SIZE 0x1000
#define MRD_RA_OFFSET (EEPROM_RA_OFFSET + 0x1000)

#define ROM_T_RA_SIZE 0xe000
#define ROM_T_RA_OFFSET (MRD_RA_OFFSET+MRD_RA_SIZE)
#endif

#elif (TCSUPPORT_RESERVEAREA_BLOCK==6)
//defined( TCSUPPORT_RESERVEAREA_BLOCK == 6)
#if defined(MT7615E)|| defined(MT7615_11N) || defined(MT7613E) || defined(MT7915D)|| defined(MT7915N)|| defined(MT7915E) || defined(MT7916D)|| defined(MT7916N)|| defined(MT7916E)
#ifdef TCSUPPORT_NAND_BMT
/*
---------------------------------------------------------------------------------
@reserve area table@
|sector		name							cover area				note
|0			eeprom(11n part)					0~0x0fff					4k
|1			TRX_SET1_SET2(11n part)			0x1000~0x9fff			36k
|2			Reserved 						0xa000~0x1ffff			88k
																			//0~2 for 11n
|3			eeprom(11ac part)					0x20000~0x20fff			4k
|4			TRX_SET1_SET2(11ac part)			0x21000~0x29fff			36k
|5			Reserved 						0x2a000~0x2afff			4k
|5.1			eeprom(7613 part)					0x2b000~0x2bfff			4k
|5.2			Reserved 						0x2c000~0x3ffff			80k
																			//3~5 for 11ac
|6			backupromfile 					0x40000~0x5ffff			128k
|7			defaultromfile						0x60000~0x7ffff			128k
|8			syslog							0x80000~0x9ffff			128k
|9			eeprom							0xa0000~0xa03ff			1k(reserve 1k,no use 256 bytes)
|10			cwmppara						0xa0400~0xa05ff			512 Bytes
|11			not use now						0xa0600~0xa0fff			(4k-1k-512 Bytes)
|12			mrd								0xa1000~0xa1fff			4k
|13			rom-t							0xa2000~0xaffff			56k
------------------------------------------------------------------------------------
*/
#define RESERVEAREA_TOTAL_SIZE RESERVEAREA_ERASE_SIZE*6
#ifdef TCSUPPORT_WLAN_AC
#define MAX_EEPROM_BIN_FILE_SIZE	512
#endif
/*11n part*/
#define EEPROM_761511N_RA_SIZE 0x1000
#define EEPROM_761511N_RA_OFFSET RESERVEAREA_BLOCK_BASE
#define TRX_SET1_SET2_11N_RA_SIZE 0x9000
#define TRX_SET1_SET2_11N_RA_OFFSET (EEPROM_761511N_RA_OFFSET+EEPROM_761511N_RA_SIZE)

/*11ac part*/
#define EEPROM_761511AC_RA_SIZE 0x1000
#define EEPROM_RA_761511AC_OFFSET (EEPROM_761511N_RA_OFFSET+RESERVEAREA_ERASE_SIZE)
#define TRX_SET1_SET2_11AC_RA_SIZE 0x9000
#define TRX_SET1_SET2_11AC_RA_OFFSET (EEPROM_RA_761511AC_OFFSET+EEPROM_761511AC_RA_SIZE)
#define RESERVED_761511AC_SIZE 0x1000

/*7613 part*/
#define EEPROM_761311AC_RA_SIZE 0x1000
#define EEPROM_RA_761311AC_OFFSET (TRX_SET1_SET2_11AC_RA_OFFSET+TRX_SET1_SET2_11AC_RA_SIZE+RESERVED_761511AC_SIZE)

/*backupromfile*/
#define BACKUPROMFILE_RA_SIZE RESERVEAREA_ERASE_SIZE
#define BACKUPROMFILE_RA_OFFSET (EEPROM_RA_761511AC_OFFSET+RESERVEAREA_ERASE_SIZE)
/*defaultromfile*/
#define DEFAULTROMFILE_RA_SIZE RESERVEAREA_ERASE_SIZE
#define DEFAULTROMFILE_RA_OFFSET (BACKUPROMFILE_RA_OFFSET+BACKUPROMFILE_RA_SIZE)
/*syslog*/
#define SYSLOG_RA_SIZE RESERVEAREA_ERASE_SIZE
#define SYSLOG_RA_OFFSET (DEFAULTROMFILE_RA_OFFSET+DEFAULTROMFILE_RA_SIZE)
/*eeprom*/
#define EEPROM_RA_SIZE 0x400
#define EEPROM_RA_OFFSET (SYSLOG_RA_OFFSET+SYSLOG_RA_SIZE)
#ifdef TCSUPPORT_WLAN_AC
#define EEPROM_RA_AC_OFFSET (EEPROM_RA_OFFSET+MAX_EEPROM_BIN_FILE_SIZE)
#endif

/*device information*/
#ifdef TCSUPPORT_PRODUCTIONLINE
#define PROLINE_CWMPPARA_RA_SIZE 0x200
#define PROLINE_CWMPPARA_RA_OFFSET (EEPROM_RA_OFFSET+EEPROM_RA_SIZE)
#else
#define PROLINE_CWMPPARA_RA_SIZE 0x0
#define PROLINE_CWMPPARA_RA_OFFSET (EEPROM_RA_OFFSET+EEPROM_RA_SIZE)
#endif

#ifdef TCSUPPORT_MT7570
#define BOB_RA_SIZE 0xa0
#define BOB_RA_OFFSET (PROLINE_CWMPPARA_RA_OFFSET + 0x200)
#endif

#define MRD_RA_SIZE 0x1000
#define MRD_RA_OFFSET (EEPROM_RA_OFFSET + 0x1000)

#define ROM_T_RA_SIZE 0xe000
#define ROM_T_RA_OFFSET (MRD_RA_OFFSET+MRD_RA_SIZE)
#else

/*
---------------------------------------------------------------------------------
@reserve area table@
|sector		name							cover area				note
|0			eeprom(11n part)					0~0x0fff					4k
|1			TRX_SET1_SET2(11n part)			0x1000~0x9fff			36k
|2			Reserved 						0xa000~0xffff				24k
																			//0~2 for 11n
|3			eeprom(11ac part)					0x10000~0x10fff			4k
|4			TRX_SET1_SET2(11ac part)			0x11000~0x19fff			36k
|5			Reserved							0x1a000~0x1afff			4k
|5.1			eeprom(7613 part)					0x1b000~0x1bfff			4k
|5.2			Reserved 						0x1c000~0x1ffff			16k
																			//3~5 for 11ac
|6			backupromfile 					0x20000~0x2ffff			64k
|7			defaultromfile						0x30000~0x3ffff			64k
|8			syslog							0x40000~0x5ffff			64k
|9			eeprom							0x60000~0x603ff			1k(reserve 1k,no use 256 bytes)
|10			cwmppara						0x60400~0x605ff			512 Bytes
|11			not use now						0x60600~0x60fff			(4k-1k-512 Bytes)
|12			mrd								0x61000~0x61fff			4k
|13			rom-t							0x62000~0x6ffff			56k
------------------------------------------------------------------------------------
*/
#define RESERVEAREA_TOTAL_SIZE RESERVEAREA_ERASE_SIZE*6
#ifdef TCSUPPORT_WLAN_AC
#define MAX_EEPROM_BIN_FILE_SIZE	512
#endif

/*11n part*/
#define EEPROM_761511N_RA_SIZE 0x1000
#define EEPROM_761511N_RA_OFFSET RESERVEAREA_BLOCK_BASE
#define TRX_SET1_SET2_11N_RA_SIZE 0x9000
#define TRX_SET1_SET2_11N_RA_OFFSET (EEPROM_761511N_RA_OFFSET+EEPROM_761511N_RA_SIZE)
	
/*11ac part*/
#define EEPROM_761511AC_RA_SIZE 0x1000
#define EEPROM_RA_761511AC_OFFSET (EEPROM_761511N_RA_OFFSET+0x10000)
#define TRX_SET1_SET2_11AC_RA_SIZE 0x9000
#define TRX_SET1_SET2_11AC_RA_OFFSET (EEPROM_RA_761511AC_OFFSET+EEPROM_761511AC_RA_SIZE)
#define RESERVED_761511AC_SIZE 0x1000

/*7613 part*/
#define EEPROM_761311AC_RA_SIZE 0x1000
#define EEPROM_RA_761311AC_OFFSET (TRX_SET1_SET2_11AC_RA_OFFSET+TRX_SET1_SET2_11AC_RA_SIZE+RESERVED_761511AC_SIZE)

	
/*backupromfile*/
#define BACKUPROMFILE_RA_SIZE 0x10000
#define BACKUPROMFILE_RA_OFFSET (EEPROM_RA_761511AC_OFFSET+0x10000)
/*defaultromfile*/
#define DEFAULTROMFILE_RA_SIZE 0x10000
#define DEFAULTROMFILE_RA_OFFSET (BACKUPROMFILE_RA_OFFSET+BACKUPROMFILE_RA_SIZE)
/*syslog*/
#define SYSLOG_RA_SIZE 0x10000
#define SYSLOG_RA_OFFSET (DEFAULTROMFILE_RA_OFFSET+DEFAULTROMFILE_RA_SIZE)
/*eeprom*/
#define EEPROM_RA_SIZE 0x400
#define EEPROM_RA_OFFSET (SYSLOG_RA_OFFSET+SYSLOG_RA_SIZE)
#ifdef TCSUPPORT_WLAN_AC
#define EEPROM_RA_AC_OFFSET (EEPROM_RA_OFFSET+MAX_EEPROM_BIN_FILE_SIZE)
#endif
	
/*device information*/
#ifdef TCSUPPORT_PRODUCTIONLINE
#define PROLINE_CWMPPARA_RA_SIZE 0x200
#define PROLINE_CWMPPARA_RA_OFFSET (EEPROM_RA_OFFSET+EEPROM_RA_SIZE)
#else
#define PROLINE_CWMPPARA_RA_SIZE 0x0
#define PROLINE_CWMPPARA_RA_OFFSET (EEPROM_RA_OFFSET+EEPROM_RA_SIZE)
#endif

#ifdef TCSUPPORT_MT7570
#define BOB_RA_SIZE 0xa0
#define BOB_RA_OFFSET (PROLINE_CWMPPARA_RA_OFFSET + 0x200)
#endif
	
#define MRD_RA_SIZE 0x1000
#define MRD_RA_OFFSET (EEPROM_RA_OFFSET + 0x1000)
	
#define ROM_T_RA_SIZE 0xe000
#define ROM_T_RA_OFFSET (MRD_RA_OFFSET+MRD_RA_SIZE)
#endif
#endif
#elif (TCSUPPORT_RESERVEAREA_BLOCK==7)
//defined( TCSUPPORT_RESERVEAREA_BLOCK == 7)
#if defined(MT7615E)|| defined(MT7615_11N) || defined(MT7613E) || defined(MT7915D)|| defined(MT7915N)|| defined(MT7915E) || defined(MT7916D)|| defined(MT7916N)|| defined(MT7916E)
#ifdef TCSUPPORT_NAND_BMT
/*
---------------------------------------------------------------------------------
@reserve area table@
|sector		name				cover area				note
|0			eeprom(11n part)					0~0x0fff					4k
|1			TRX_SET1_SET2(11n part)			0x1000~0x9fff			36k
|2			Reserved 						0xa000~0x1ffff			88k
																			//0~2 for 11n
|3			eeprom(11ac part)					0x20000~0x20fff			4k
|4			TRX_SET1_SET2(11ac part)			0x21000~0x29fff			36k
|5			Reserved for 7615					0x2a000~0x2afff			4k
|5.1			eeprom(7613 part)					0x2b000~0x2bfff 		4k
|5.2			Reserved	or WiFi					0x2c000~0x3ffff 		80k

																			//3~5 for 11ac
|6			backupromfile						0x40000~0x7ffff			256k
|7			gpon_bob						0x80000~0x9ffff			128k
|8			syslog							0xa0000~0xbffff			128k
|9			eeprom							0xc0000~0xc03ff			1k(reserve 1k,no use 256 bytes)
|10			cerm1							0xc0400~0xc13ff			4k
|11			cerm2							0xc1400~0xc823ff			4k
|12			cerm3							0xc2400~0xc33ff			4k
|13			cerm4							0xc3400~0xc43ff			4k
|14			username/passwd					0xc4400~0xc47ff			1K
|15			mrd								0xc0000~0xc0fff			4k
|16			imgbootflag						0xc5800~0xc5801			

------------------------------------------------------------------------------------
*/

#define RESERVEAREA_TOTAL_SIZE RESERVEAREA_ERASE_SIZE*7

#ifdef TCSUPPORT_WLAN_AC
#define MAX_EEPROM_BIN_FILE_SIZE	512
#endif
/*11n part*/
#define EEPROM_761511N_RA_SIZE 0x1000
#define EEPROM_761511N_RA_OFFSET RESERVEAREA_BLOCK_BASE
#define TRX_SET1_SET2_11N_RA_SIZE 0x9000
#define TRX_SET1_SET2_11N_RA_OFFSET (EEPROM_761511N_RA_OFFSET+EEPROM_761511N_RA_SIZE)

/*11ac part*/
#define EEPROM_761511AC_RA_SIZE 0x1000
#define EEPROM_RA_761511AC_OFFSET (EEPROM_761511N_RA_OFFSET+NAND_FLASH_BLOCK_SIZE)
#define TRX_SET1_SET2_11AC_RA_SIZE 0x9000
#define TRX_SET1_SET2_11AC_RA_OFFSET (EEPROM_RA_761511AC_OFFSET+EEPROM_761511AC_RA_SIZE)
#define RESERVED_761511AC_SIZE 0x1000

/*7613 part*/
#define EEPROM_761311AC_RA_SIZE 0x1000
#define EEPROM_RA_761311AC_OFFSET (TRX_SET1_SET2_11AC_RA_OFFSET+TRX_SET1_SET2_11AC_RA_SIZE+RESERVED_761511AC_SIZE)

/*backupromfile*/
#define BACKUPROMFILE_RA_SIZE NAND_FLASH_BLOCK_SIZE*2
#define BACKUPROMFILE_RA_OFFSET (EEPROM_RA_761511AC_OFFSET+NAND_FLASH_BLOCK_SIZE)

/*defaultromfile*/
#define BOB_RA_SIZE NAND_FLASH_BLOCK_SIZE
#define BOB_RA_OFFSET (BACKUPROMFILE_RA_OFFSET+BACKUPROMFILE_RA_SIZE)
/*syslog*/
#define SYSLOG_RA_SIZE NAND_FLASH_BLOCK_SIZE
#define SYSLOG_RA_OFFSET (GPON_BOB_OFFSET+BOB_RA_SIZE)
/*eeprom*/
#define EEPROM_RA_SIZE 0x400
#define EEPROM_RA_OFFSET (SYSLOG_RA_OFFSET+SYSLOG_RA_SIZE)
/*cerm1*/
#define CERM1_RA_SIZE 0x1000
#define CERM1_RA_OFFSET (EEPROM_RA_SIZE+EEPROM_RA_OFFSET)
/*cerm2*/
#define CERM2_RA_SIZE 0x1000
#define CERM2_RA_OFFSET (CERM1_RA_SIZE+CERM1_RA_OFFSET)
/*cerm3*/
#define CERM3_RA_SIZE 0x1000
#define CERM3_RA_OFFSET (CERM2_RA_SIZE+CERM2_RA_OFFSET)
/*cerm4*/
#define CERM4_RA_SIZE 0x1000
#define CERM4_RA_OFFSET (CERM3_RA_SIZE+CERM3_RA_OFFSET)
#if defined(TCSUPPORT_CT_BOOTLOADER_UPGRADE)
/*username/passwd*/
#define USERNAMEPASSWD_RA_SIZE 0x400
#define USERNAMEPASSWD_RA_OFFSET (CERM4_RA_SIZE+CERM4_RA_OFFSET)
#else
#define USERNAMEPASSWD_RA_SIZE 0x0
#define USERNAMEPASSWD_RA_OFFSET (CERM4_RA_SIZE+CERM4_RA_OFFSET)
#endif
#ifdef TCSUPPORT_PRODUCTIONLINE
#define PROLINE_CWMPPARA_RA_SIZE 0x180
#define PROLINE_CWMPPARA_RA_OFFSET (USERNAMEPASSWD_RA_SIZE+USERNAMEPASSWD_RA_OFFSET)
#else
#define PROLINE_CWMPPARA_RA_SIZE 0x0
#define PROLINE_CWMPPARA_RA_OFFSET (USERNAMEPASSWD_RA_SIZE+USERNAMEPASSWD_RA_OFFSET)
#endif

#ifdef TCSUPPORT_MT7570
#define BOB_RA_SIZE 0xa0
#define BOB_RA_OFFSET (PROLINE_CWMPPARA_RA_OFFSET + 0x200)
#endif

#define MRD_RA_SIZE 0x1000
#define MRD_RA_OFFSET (EEPROM_RA_OFFSET + 0x1000)

/*image boot flag*/
#define IMG_BOOT_FLAG_SIZE 	1
#define IMG_BOOT_FLAG_OFFSET  	(PROLINE_CWMPPARA_RA_SIZE+PROLINE_CWMPPARA_RA_OFFSET)
#endif
#endif

#else
//defined( TCSUPPORT_RESERVEAREA_BLOCK == 5)
#ifdef TCSUPPORT_NAND_BMT
/*
---------------------------------------------------------------------------------
@reserve area table@
|sector		name				cover area				note
|0			backupromfile			0~0x3ffff					256k
|1			gpon_bob			0x40000~0x5ffff			128k
|2			syslog				0x60000~0x7ffff			128k
|3			eeprom				0x80000~0x803ff			1k(reserve 1k,no use 256 bytes)
|4			cerm1				0x80400~0x813ff			4k
|5			cerm2				0x81400~0x823ff			4k
|6			cerm3				0x82400~0x833ff			4k
|7			cerm4				0x83400~0x843ff			4k
|8			username/passwd		0x84400~0x847ff			1K
|9			mrd					0x80000~0x80fff			4k
|10			imgbootflag			0x85800~0x85801			

------------------------------------------------------------------------------------
*/
#define RESERVEAREA_TOTAL_SIZE RESERVEAREA_ERASE_SIZE*5
/*backupromfile*/
#define BACKUPROMFILE_RA_SIZE NAND_FLASH_BLOCK_SIZE*2
#define BACKUPROMFILE_RA_OFFSET RESERVEAREA_BLOCK_BASE
/*defaultromfile*/
#define BOB_RA_SIZE NAND_FLASH_BLOCK_SIZE
#define BOB_RA_OFFSET (BACKUPROMFILE_RA_OFFSET+BACKUPROMFILE_RA_SIZE)
/*syslog*/
#define SYSLOG_RA_SIZE NAND_FLASH_BLOCK_SIZE
#define SYSLOG_RA_OFFSET (GPON_BOB_OFFSET+BOB_RA_SIZE)
/*eeprom*/
#define EEPROM_RA_SIZE 0x400
#define EEPROM_RA_OFFSET (SYSLOG_RA_OFFSET+SYSLOG_RA_SIZE)
/*cerm1*/
#define CERM1_RA_SIZE 0x1000
#define CERM1_RA_OFFSET (EEPROM_RA_SIZE+EEPROM_RA_OFFSET)
/*cerm2*/
#define CERM2_RA_SIZE 0x1000
#define CERM2_RA_OFFSET (CERM1_RA_SIZE+CERM1_RA_OFFSET)
/*cerm3*/
#define CERM3_RA_SIZE 0x1000
#define CERM3_RA_OFFSET (CERM2_RA_SIZE+CERM2_RA_OFFSET)
/*cerm4*/
#define CERM4_RA_SIZE 0x1000
#define CERM4_RA_OFFSET (CERM3_RA_SIZE+CERM3_RA_OFFSET)
#if defined(TCSUPPORT_CT_BOOTLOADER_UPGRADE)
/*username/passwd*/
#define USERNAMEPASSWD_RA_SIZE 0x400
#define USERNAMEPASSWD_RA_OFFSET (CERM4_RA_SIZE+CERM4_RA_OFFSET)
#else
#define USERNAMEPASSWD_RA_SIZE 0x0
#define USERNAMEPASSWD_RA_OFFSET (CERM4_RA_SIZE+CERM4_RA_OFFSET)
#endif
#ifdef TCSUPPORT_PRODUCTIONLINE
#define PROLINE_CWMPPARA_RA_SIZE 0x180
#define PROLINE_CWMPPARA_RA_OFFSET (USERNAMEPASSWD_RA_SIZE+USERNAMEPASSWD_RA_OFFSET)
#else
#define PROLINE_CWMPPARA_RA_SIZE 0x0
#define PROLINE_CWMPPARA_RA_OFFSET (USERNAMEPASSWD_RA_SIZE+USERNAMEPASSWD_RA_OFFSET)
#endif

#ifdef TCSUPPORT_MT7570
#define BOB_RA_SIZE 0xa0
#define BOB_RA_OFFSET (PROLINE_CWMPPARA_RA_OFFSET + 0x200)
#endif

#define MRD_RA_SIZE 0x1000
#define MRD_RA_OFFSET (EEPROM_RA_OFFSET + 0x1000)

/*image boot flag*/
#define IMG_BOOT_FLAG_SIZE 	1
#define IMG_BOOT_FLAG_OFFSET  	(PROLINE_CWMPPARA_RA_SIZE+PROLINE_CWMPPARA_RA_OFFSET)

#endif



#endif

#ifdef TCSUPPORT_NAND_BADBLOCK_CHECK
/*syslog*/
#define SYSLOG_RA_SIZE 0x20000
#define SYSLOG_RA_OFFSET RESERVEAREA_BLOCK_BASE

/*backupromfile*/
#define BACKUPROMFILE_RA_SIZE 0x20000
#define BACKUPROMFILE_RA_OFFSET (SYSLOG_RA_OFFSET + SYSLOG_RA_SIZE * 11)

/*eeprom*/
#define EEPROM_RA_SIZE 0x400
#define EEPROM_RA_OFFSET (BACKUPROMFILE_RA_OFFSET + BACKUPROMFILE_RA_SIZE * 15)
#endif

#endif
#endif
#ifndef TCSUPPORT_RESERVEAREA_EXTEND
#define IMG_BOOT_FLAG_SIZE 	1
#if defined(TCSUPPORT_CT)
#define IMG_BOOT_FLAG_OFFSET  	(CERM1_RA_OFFSET - IMG_BOOT_FLAG_SIZE)
#else
#define IMG_BOOT_FLAG_OFFSET  	(MRD_RA_OFFSET - IMG_BOOT_FLAG_SIZE)
#endif
#endif

#endif
