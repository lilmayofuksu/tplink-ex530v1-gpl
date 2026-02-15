/*
 * (C) Copyright 2000-2009
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <configs/en7523_evb.h>
#include <ecnt/image/ecnt_image.h>
#include <flashhal.h>
#include <spi/spi_nand_flash.h>
#include <bmt.h>
#include "flash_layout/tc_partition.h"

#define DBG_MI_CONF 0
#define DBG_BOOTARGS 0
extern unsigned int get_fip_offset(void);

extern struct mtd_info mtd;

char *mi_conf[] = {"sdram_conf",
					"vendor_name",
					"product_name",
					"ethaddr",
					"snmp_sysobjid",
					"country_code",
					"ether_gpio",
					"power_gpio",
					"username",
					"password",
					"dsl_gpio",
					"internet_gpio",
					"multi_upgrade_gpio",
					"onu_type",
					"qdma_init",
					"root",
					"console",
					"bootflag",
					"serdes_sel",
					TCLINUX_IMG_INFO_STR};

static int parse_env_config(char *var)
{
	int i;
	int n_mi_conf;
	char *val;
	unsigned int totalLen = 0;

	n_mi_conf = (sizeof(mi_conf) / sizeof(const char *));
	totalLen = strlen(var);

	for(i = 0; i < n_mi_conf; i++) {
		val = getenv(mi_conf[i]);
#if DBG_MI_CONF
		printf("=== totalLen:%d ===\n", totalLen);
		printf("parse %s=%s\n", mi_conf[i], val);
#endif
		if(val) {
			totalLen += (strlen(mi_conf[i]) + strlen("=")
						+ strlen(val) + strlen(" "));
			if(totalLen >= (BOOTARGS_STR_MAX_LEN - 1)) {
				printf("bootargs len:%d more than %d ===\n", totalLen, BOOTARGS_STR_MAX_LEN);
				return -1;
			}
			
			strncat(var, mi_conf[i], BOOTARGS_STR_MAX_LEN - 1);
			strncat(var, "=", BOOTARGS_STR_MAX_LEN - 1);
			strncat(var, val, BOOTARGS_STR_MAX_LEN - 1);
			strncat(var, " ", BOOTARGS_STR_MAX_LEN - 1);
		}
	}
	
	return 0;
}

static int set_tclinux_img_env(struct tclinux_imginfo *firstInfo, struct tclinux_imginfo *secondInfo)
{
	char info_str[86 * 2] = ""; /* 86 = 16 * 5 + 5 char ("," * 4 + " ") */
	char tclinux_size_str[16] = {0};
	char kernel_off_str[16] = {0};
	char kernel_size_str[16] = {0};
	char rootfs_off_str[16] = {0};
	char rootfs_size_str[16] = {0};

	if(firstInfo) {
		sprintf(tclinux_size_str, "0x%x", firstInfo->tclinux_size);
		sprintf(kernel_off_str, "0x%x", (firstInfo->kernel_off + get_fip_offset()));
		sprintf(kernel_size_str, "0x%x", firstInfo->kernel_size);
		sprintf(rootfs_off_str, "0x%x", (firstInfo->rootfs_off + get_fip_offset()));
		sprintf(rootfs_size_str, "0x%x", firstInfo->rootfs_size);
		strncat(info_str, tclinux_size_str, sizeof(info_str) - 1);
		strncat(info_str, ",", sizeof(info_str) - 1);
		strncat(info_str, kernel_off_str, sizeof(info_str) - 1);
		strncat(info_str, ",", sizeof(info_str) - 1);
		strncat(info_str, kernel_size_str, sizeof(info_str) - 1);
		strncat(info_str, ",", sizeof(info_str) - 1);
		strncat(info_str, rootfs_off_str, sizeof(info_str) - 1);
		strncat(info_str, ",", sizeof(info_str) - 1);
		strncat(info_str, rootfs_size_str, sizeof(info_str) - 1);
	}

	if(secondInfo) {
		strncat(info_str, ",", sizeof(info_str) - 1);
		sprintf(tclinux_size_str, "0x%x", secondInfo->tclinux_size);
		sprintf(kernel_off_str, "0x%x", (secondInfo->kernel_off + get_fip_offset()));
		sprintf(kernel_size_str, "0x%x", secondInfo->kernel_size);
		sprintf(rootfs_off_str, "0x%x", (secondInfo->rootfs_off + get_fip_offset()));
		sprintf(rootfs_size_str, "0x%x", secondInfo->rootfs_size);
		strncat(info_str, tclinux_size_str, sizeof(info_str) - 1);
		strncat(info_str, ",", sizeof(info_str) - 1);
		strncat(info_str, kernel_off_str, sizeof(info_str) - 1);
		strncat(info_str, ",", sizeof(info_str) - 1);
		strncat(info_str, kernel_size_str, sizeof(info_str) - 1);
		strncat(info_str, ",", sizeof(info_str) - 1);
		strncat(info_str, rootfs_off_str, sizeof(info_str) - 1);
		strncat(info_str, ",", sizeof(info_str) - 1);
		strncat(info_str, rootfs_size_str, sizeof(info_str) - 1);
	}

	setenv(TCLINUX_IMG_INFO_STR, info_str);

	return 0;
}

static void set_bootflag_env(unsigned int bootflag)
{
	if(bootflag == 0) {
		setenv("bootflag", "0");
	} else {
		setenv("bootflag", "1");
	}
}

int bootargs_init(struct tclinux_imginfo *first_imginfo, struct tclinux_imginfo *second_imginfo, unsigned int bootflag)
{
	char var[BOOTARGS_STR_MAX_LEN] = {0};
	
	if(set_tclinux_img_env(first_imginfo, second_imginfo)) {
		return -1;
	}

	set_bootflag_env(bootflag);

	/* set MAC address to kernel */
	if(parse_env_config(var)) {
		return -1;
	}

#if DBG_BOOTARGS
	printf("%s, var:%s\n", __func__, var);
#endif

	setenv("bootargs", var);
}
unsigned int get_boot_flag_addr()
{
	unsigned int boot_flag_addr = 0;
		if (IS_NANDFLASH) {
        #if 0
			printf("\nflash_base:0x%x; avalable_size:0x%x, reservearea_size:0x%x, block size:%d, IMG_BOOT_FLAG_OFFSET:0x%x\n",
				flash_base, nand_flash_avalable_size, reservearea_size, TCSUPPORT_RESERVEAREA_BLOCK, IMG_BOOT_FLAG_OFFSET);
        #endif
#if defined(TCSUPPORT_OPENWRT)
			boot_flag_addr =  flash_base + ecnt_get_reservearea_flash_offset() +  reservearea_size*(TCSUPPORT_RESERVEAREA_BLOCK-1); 	
#else
			boot_flag_addr =  flash_base + nand_flash_avalable_size - (reservearea_size * TCSUPPORT_RESERVEAREA_BLOCK) + IMG_BOOT_FLAG_OFFSET;
#endif
		}
#ifdef TCSUPPORT_NEW_SPIFLASH	
		else {
        #if 0
			printf("\nflash_base: %x; mtd.size: %x, erasesize: %x, block size: %d, IMG_BOOT_FLAG_OFFSET: %x\n",
				flash_base, mtd.size, mtd.erasesize, TCSUPPORT_RESERVEAREA_BLOCK, IMG_BOOT_FLAG_OFFSET);
        #endif
			boot_flag_addr =  flash_base + mtd.size - mtd.erasesize * TCSUPPORT_RESERVEAREA_BLOCK + IMG_BOOT_FLAG_OFFSET;
		}
#endif

	return boot_flag_addr;
}

char readBootFlagFromFlash(void)
{
	unsigned long retlen = 0;
	unsigned int boot_flag_addr = 0;
	char flag = 0;

	boot_flag_addr=get_boot_flag_addr();

	if (IS_NANDFLASH) {
        flash_read(boot_flag_addr, 1, &retlen, &flag);
    }
#ifdef TCSUPPORT_NEW_SPIFLASH	
    else {
       memcpy(&flag, boot_flag_addr, 1);
    }
#endif

	flag = flag - '0';
	//printf("boot_flag_addr:0x%x; flag:0x%x\n", boot_flag_addr, flag);
	if (flag != 0 && flag != 1){
		flag = 0;
        printf("(flag != 0 && flag != 1) --> boot from Master\n");
	}

	return flag;
}

int writeBootFlagtoFlash(char flag)
{
	unsigned int boot_flag_addr = 0;
    char old_flag;
	int retVal;

	if (flag != 0 && flag != 1){
		printf("\nError: flag:%d !=0 && !=1\n", flag);
		return -1;
	}

    old_flag = readBootFlagFromFlash();
    if (old_flag==flag) {
        printf("\nboot_flag is %d already, so just exit\n", flag);
        return -1;
    }
    
	flag = flag + '0';
	boot_flag_addr=get_boot_flag_addr();

    printf("write %c to boot_flag_addr:0x%x\n", flag, boot_flag_addr);
    
	retVal = flash_partial_write(boot_flag_addr, 1, &flag);
    
    printf("\nnew bootflag==%d\n", readBootFlagFromFlash());

	return retVal;
}

int swap_bootflag(void)
{
    char bootflag = readBootFlagFromFlash();

    if (bootflag==0)
        bootflag=1;
    else
        bootflag=0;

    return writeBootFlagtoFlash(bootflag);
}

