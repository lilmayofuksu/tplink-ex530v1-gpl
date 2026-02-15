/*
 * Copyright (C) 2004  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


typedef struct _ROMFILE_STRUCT
{
	unsigned int signOffset;	/* flash UID signature */
	unsigned int macOffset;	/* MAC address */
	unsigned int pinOffset;	/* PIN code for wireless */
	unsigned int rfpiOffset;	/* RFPI code for dect */
	unsigned int deviceIdOffset;	/* Device ID for cloud */
}ROMFILE_STRUCT;

typedef struct _LINUX_FLASH_STRUCT
{
	unsigned int appSize;			/* kernel+rootfs size */
	unsigned int bootOffset;		/* boot loader	*/	
	unsigned int kernelOffset;		/* kernel */
	unsigned int rootfsOffset;		/* rootfs */
	unsigned int configOffset;		/* config */
	unsigned int miscOffset;		/* miscellaneous data */
}LINUX_FLASH_STRUCT;

#define MAC_MISC_OFFSET 0x2F100
#define PIN_MISC_OFFSET 0x2F200

/* 
 * brief Total image tag length	
 */
#define TAG_LEN         512

/* 
 * brief cloud ID length	
 */
#define CLOUD_ID_BYTE_LEN	16

/* 
 * brief Token length	
 */
#define TOKEN_LEN       20

/* 
 * brief magic number length	
 */
#define MAGIC_NUM_LEN	20

/* 
 * brief signature length	
 */
#define SIG_LEN		128


#define BOOTLOADER_TAG_LEN TAG_LEN

#define RESSIG_LEN (SIG_LEN - 24)

typedef uint64_t  UINT64;
typedef uint32_t  UINT32;

/* 
 * brief Image tag struct,have different position in Linux and vxWorks(see TAG_OFFSET)	
 */
typedef struct _LINUX_FILE_TAG
{	
	UINT32 tagVersion;			/* tag version number */  
	unsigned char hardwareId[CLOUD_ID_BYTE_LEN];		/* HWID for cloud */
	unsigned char firmwareId[CLOUD_ID_BYTE_LEN];		/* FWID for cloud */
	unsigned char oemId[CLOUD_ID_BYTE_LEN];			/* OEMID for cloud */
	UINT32 productId;	/* product id */  
	UINT32 productVer;	/* product version */
	UINT32 addHver;		/* Addtional hardware version */
	
	unsigned char imageValidToken[TOKEN_LEN];	/* image validation token - md5 checksum */
	unsigned char magicNum[MAGIC_NUM_LEN];	 	/* magic number */
	
	UINT64 kernelTextAddr; 	/* text section address of kernel */
	UINT64 kernelEntryPoint; /* entry point address of kernel */
	
	UINT32 totalImageLen;	/* the sum of kernelLen+rootfsLen+tagLen */
	
	UINT32 kernelAddress;	/* starting address (offset from the beginning of FILE_TAG) 
									 * of kernel image 
									 */
	UINT32 kernelLen;		/* length of kernel image */
	
	UINT32 rootfsAddress;	/* starting address (offset) of filesystem image */
	UINT32 rootfsLen;		/* length of filesystem image */

	UINT32 bootAddress;		/* starting address (offset) of bootloader (LK) image */
	UINT32 bootLen;			/* length of bootloader image */

	UINT32 swRevision;		/* software revision */
	UINT32 platformVer;		/* platform version */
	UINT32 specialVer;

	UINT32 binCrc32;			/* CRC32 for bin(kernel+rootfs) */

	UINT32 preloaderAddress;          /*starting address (offset) of preloader image */
	UINT32 preloaderLen;              /*length of preloader image*/
	UINT32 mcf1Address;        /*starting address (offset) of mcf1*/
	UINT32 mcf1Len;            /*length of mcf1*/
	UINT32 mcf2Address;      /*mcf2 address offset of image*/
	UINT32 mcf2Len;          /*mcf2 length*/
	UINT32 md1imgAddress;         /*starting address of md1img of image*/
	UINT32 md1imgLen;             /*length of md1img*/
	UINT32 md1dspAddress;       /*starting address of md1dsp image*/
	UINT32 md1dspLen;           /*length of md1dsp*/
	
	UINT32 reserved1[1];		/* reserved for future */

	unsigned char sig[SIG_LEN];		/* signature for update */
	unsigned char resSig[RESSIG_LEN];	/* reserved for signature */
	UINT32 spmfwAddress;       /*starting address of spmfw image*/
	UINT32 spmfwLen;           /*length of spmfw*/
	UINT32 pi_imgAddress;          /*starting address of pi_img image*/
	UINT32 pi_imgLen;              /*length of pi_img*/
	
	UINT32 dpmAddress;          /*starting address of dpm image*/
	UINT32 dpmLen;              /*length of dpm*/
	UINT32 medmcuAddress;          /*starting address of medmcu image*/
	UINT32 medmcuLen;              /*length of medmcu*/
	UINT32 sspmAddress;          /*starting address of sspm image*/
	UINT32 sspmLen;              /*length of sspm*/
	UINT32 mcupmAddress;          /*starting address of mcupm image*/
	UINT32 mcupmLen;              /*length of mcupm*/
	UINT32 teeAddress;          /*starting address of tee image*/
	UINT32 teeLen;              /*length of tee*/
	UINT32 rootfs_sigAddress;          /*starting address of tee image*/
	UINT32 rootfs_sigLen;              /*length of tee*/
	UINT32 loader_extAddress;          /*starting address of loader_ext image*/
	UINT32 loader_extLen;              /*length of loader_ext*/
}LINUX_FILE_TAG;


#define ROUND 1
#define MAX_FILENAME_LEN 256 
/* three files will input-bootloader, kernel, rootfs */
#define FILE_NUM 4

enum IMAGE_FILES
{
	BOOT,
	KERNEL,
	FS,
	CONFIG
};

#define TAG_VERSION		0x03000003 
#define VERSION_INFO	"ver. 3.0"

unsigned char md5Key[16] = 
{	/* linux - wr841n */
	0xDC, 0xD7, 0x3A, 0xA5, 0xC3, 0x95, 0x98, 0xFB, 
	0xDC, 0xF9, 0xE7, 0xF4, 0x0E, 0xAE, 0x47, 0x37
};

unsigned char mk5Key_bootloader[16] =
{	/* linux bootloader - u-boot/redboot */
	0x8C, 0xEF, 0x33, 0x5F, 0xD5, 0xC5, 0xCE, 0xFA,
	0xAC, 0x9C, 0x28, 0xDA, 0xB2, 0xE9, 0x0F, 0x42
};


unsigned char def_mac[6] = {0x00, 0x0a, 0xeb, 0x13, 0x09, 0x69};

unsigned char magicNum[MAGIC_NUM_LEN] = {0x55, 0xAA, 0x55, 0xAA, 0xF1, 0xE2, 0xD3, 0xC4, 0xE5, 0xA6, 0x6A, 0x5E, 0x4C, 0x3D, 0x2E, 0x1F, 0xAA, 0x55, 0xAA, 0x55};

typedef struct _USRCONF_STRUCT
{
	unsigned int	length;			/* length in byte */
	unsigned int 	signature;		/* magic number */
	unsigned int	checksum;		/* checksum */
	unsigned int	isCompressed;	/* indicate config data is compressed */
}USRCONF_STRUCT;

#define CONF_FLASH_SIGNATURE			0x98765432

