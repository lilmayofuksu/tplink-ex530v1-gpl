/*  Copyright(c) 2009-2017 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 * file		tp_bootlib.h
 * brief	
 * details	
 *
 * author	wangwenhao
 * version	1.0
 * date		4May17
 *
 * warning	
 *
 * history arg	1.0, 4May17
 */
#ifndef __TP_BOOTLIB_H__
#define __TP_BOOTLIB_H__


/**************************************************************************************************/
/*                                           DEFINES                                              */
/**************************************************************************************************/
/*
 * brief	copy from oal_sys.h
 * By	wangwenhao, 22May17
 */
/************************************* partition size defines *************************************/
/* MTD_BLOCK_SIZE defined in menuconfig */
/* MTD_IMAGE_SIZE defined in menuconfig */
/* MTD_BOOT_SIZE defined in menuconfig */
/* MTD_MISC_SIZE defined in menuconfig */
/* MTD_KERNEL_SIZE defined in menuconfig */
#ifdef INCLUDE_MTD_TYPE_FS
#define MTD_BFLAG_SIZE		(MTD_MISC_SIZE)
#define MTD_APP_SIZE		(MTD_IMAGE_SIZE - MTD_BOOT_SIZE - 2 * MTD_MISC_SIZE)
#else /* INCLUDE_MTD_TYPE_FS */
#define MTD_BFLAG_SIZE		(MTD_BLOCK_SIZE)
#define MTD_APP_SIZE		(MTD_IMAGE_SIZE - MTD_BOOT_SIZE - MTD_MISC_SIZE)
#endif /* INCLUDE_MTD_TYPE_FS */
#define MTD_ROOTFS_SIZE		(MTD_APP_SIZE - MTD_KERNEL_SIZE)

/************************************* partition offset defines ***********************************/
#define MTD_OFS_BOOT		(0)

#ifdef INCLUDE_MTD_TYPE_RAW1
#define MTD_OFS_KERNEL		(MTD_BOOT_SIZE)
#define MTD_OFS_ROOTFS		(MTD_BOOT_SIZE + MTD_KERNEL_SIZE)
#define MTD_OFS_MISC		(MTD_IMAGE_SIZE - MTD_MISC_SIZE)
#endif /* INCLUDE_MTD_TYPE_RAW1 */

#ifdef INCLUDE_MTD_TYPE_RAW2
#define MTD_OFS_MISC		(MTD_BOOT_SIZE)
#define MTD_OFS_KERNEL		(MTD_BOOT_SIZE + MTD_MISC_SIZE)
#define MTD_OFS_ROOTFS		(MTD_BOOT_SIZE + MTD_MISC_SIZE + MTD_KERNEL_SIZE)
#endif /* INCLUDE_MTD_TYPE_RAW2 */

#ifdef INCLUDE_MTD_TYPE_FS
#define MTD_OFS_MISC		(MTD_BOOT_SIZE)
#define MTD_OFS_MISC_RO		(MTD_BOOT_SIZE)
#define MTD_OFS_MISC_RW		(MTD_BOOT_SIZE + MTD_MISC_SIZE)
#define MTD_OFS_KERNEL		(MTD_BOOT_SIZE + 2 * MTD_MISC_SIZE)
#define MTD_OFS_ROOTFS		(MTD_BOOT_SIZE + 2 * MTD_MISC_SIZE + MTD_KERNEL_SIZE)
#endif /* INCLUDE_MTD_TYPE_FS */

#define MTD_OFS_KERNEL2		(MTD_IMAGE_SIZE)
#define MTD_OFS_ROOTFS2		(MTD_IMAGE_SIZE + MTD_KERNEL_SIZE)

#ifdef INCLUDE_MTD_TYPE_FS
#define MTD_OFS_BFLAG		(2 * MTD_IMAGE_SIZE - 2 * MTD_MISC_SIZE)
#else /* INCLUDE_MTD_TYPE_FS */
#define MTD_OFS_BFLAG		(2 * MTD_IMAGE_SIZE - MTD_BLOCK_SIZE)
#endif  /* INCLUDE_MTD_TYPE_FS */


/************************************** block index defines ***************************************/
enum
{
	MTD_IDX_CONFIG = 0, /* Let DM config size to 2 * MTD_BLOCK_SIZE */

#ifdef INCLUDE_OPTION66
	MTD_IDX_ISP_CONFIG = 1,
#endif /* INCLUDE_OPTION66 */

	MTD_IDX_OTHER = 2, /* Let DM config size to 2 * MTD_BLOCK_SIZE */
	MTD_IDX_DATA,      /*store important data,such as cloud message*/
	MTD_IDX_WIFI,
	MTD_IDX_WIFI_5G,
#ifdef INCLUDE_WIFI_6G
	MTD_IDX_WIFI_6G,
#endif /* INCLUDE_WIFI_6G */

#ifdef INCLUDE_PON
	MTD_IDX_PON,
#endif /* INCLUDE_PON */

#ifdef INCLUDE_IMPORTANT_CONFIG
	MTD_IDX_IMCONF,
#endif /* INCLUDE_IMPORTANT_CONFIG */

#ifdef INCLUDE_DUAL_CONFIG
	MTD_IDX_CONFIG_BAK,
#endif /* INCLUDE_DUAL_CONFIG */
	
#ifdef INCLUDE_PARENTCONTROL_V2
	MTD_IDX_PC_HISTORY_0,
	#ifdef INCLUDE_PC_V2_SAVE_ALL_HISTORY
	MTD_IDX_PC_HISTORY_1, /* Placeholder, not used actually. */
	MTD_IDX_PC_HISTORY_2, /* Placeholder, not used actually. */
	#endif /* INCLUDE_PC_V2_SAVE_ALL_HISTORY */
#endif /* INCLUDE_PARENTCONTROL_V2 */
	
	/* ----------- add above this line ---------------- */
	MTD_IDX_MAX
};
/* end index in MISC partition */


/******************************** Aginet Config offset defines ************************************/
#ifdef INCLUDE_OPTION66
enum
{
	ISP_CONFIG_OFS_FLAG	= 0xF800,
#ifdef INCLUDE_OPTION66_NEED_TAG
	ISP_CONFIG_OFS_TAG 	= 0xF900,
#endif
	ISP_CONFIG_OFS_MAX
};
#endif  /* INCLUDE_OPTION66 */


/************************************** block offset defines **************************************/
#ifdef INCLUDE_MTD_TYPE_RAW1
#define MTD_OFS_CONFIG		(MTD_IMAGE_SIZE - MTD_BLOCK_SIZE - MTD_IDX_CONFIG * MTD_BLOCK_SIZE)
#define MTD_OFS_WIFI		(MTD_IMAGE_SIZE - MTD_BLOCK_SIZE - MTD_IDX_WIFI * MTD_BLOCK_SIZE)
#define MTD_OFS_OTHER		(MTD_IMAGE_SIZE - MTD_BLOCK_SIZE - MTD_IDX_OTHER * MTD_BLOCK_SIZE)
#ifdef INCLUDE_WIFI_DUALBAND
#define MTD_OFS_WIFI_5G		(MTD_IMAGE_SIZE - MTD_BLOCK_SIZE - MTD_IDX_WIFI_5G * MTD_BLOCK_SIZE)
#endif /* INCLUDE_WIFI_DUALBAND */
#ifdef INCLUDE_PON_MTK
#define MTD_OFS_PON			(MTD_IMAGE_SIZE - MTD_BLOCK_SIZE - MTD_IDX_PON * MTD_BLOCK_SIZE)
#endif /* INCLUDE_PON_MTK */
#ifdef INCLUDE_IMPORTANT_CONFIG
#define MTD_OFS_IMCONF		(MTD_IMAGE_SIZE - MTD_BLOCK_SIZE - MTD_IDX_IMCONF * MTD_BLOCK_SIZE)
#endif /* INCLUDE_IMPORTANT_CONFIG */
// TODO: MTD_OFS_ISP_CONFIG
#endif /* INCLUDE_MTD_TYPE_RAW1 */


/* used by INCLUDE_MTD_TYPE_FS for filename gen */
#if defined(INCLUDE_MTD_TYPE_RAW2) || defined(INCLUDE_MTD_TYPE_FS)
#define MTD_OFS_CONFIG		(MTD_OFS_MISC + MTD_IDX_CONFIG * MTD_BLOCK_SIZE)
#define MTD_OFS_WIFI		(MTD_OFS_MISC + MTD_IDX_WIFI * MTD_BLOCK_SIZE)
#define MTD_OFS_OTHER		(MTD_OFS_MISC + MTD_IDX_OTHER * MTD_BLOCK_SIZE)
#ifdef INCLUDE_WIFI_DUALBAND
#define MTD_OFS_WIFI_5G		(MTD_OFS_MISC + MTD_IDX_WIFI_5G * MTD_BLOCK_SIZE)
#endif /* INCLUDE_WIFI_DUALBAND */
#ifdef INCLUDE_PON_MTK
#define MTD_OFS_PON			(MTD_OFS_MISC + MTD_IDX_PON * MTD_BLOCK_SIZE)
#endif /* INCLUDE_PON_MTK */
#ifdef INCLUDE_IMPORTANT_CONFIG
#define MTD_OFS_IMCONF		(MTD_OFS_MISC + MTD_IDX_IMCONF * MTD_BLOCK_SIZE)
#endif /* INCLUDE_IMPORTANT_CONFIG */

#ifdef INCLUDE_OPTION66
#define MTD_OFS_ISP_CONFIG	(MTD_OFS_MISC + MTD_IDX_ISP_CONFIG * MTD_BLOCK_SIZE)/* Agile config */
#endif /* INCLUDE_OPTION66 */

#endif /* INCLUDE_MTD_TYPE_RAW2 || INCLUDE_MTD_TYPE_FS */


/************************************* offsets in other block *************************************/
#ifndef OTHER_OFS
#define OTHER_OFS
enum
{
	OTHER_OFS_MAC				= 0xF100,	/* LAN MAC needs 6 Bytes, uses 0xF100 - 0xF110 */
	OTHER_OFS_OEMID 			= 0xF110,	/* OEMID needs 33 + 8 = 41 Bytes, uses 0xF110 - 0xF140 */ 
	OTHER_OFS_ZONE				= 0xF140,	/* Zone needs 16 + 8 = 24 Bytes, uses 0xF140 - 0xF160 */
	OTHER_OFS_HWID				= 0xF160,	/* HWID needs 33 + 8 = 41 Bytes, uses 0xF160 - 0xF190 */
	
#if defined(INCLUDE_LABEL_MAC)
	OTHER_OFS_LABEL_MAC		 	= 0xF1E0,	/* LABEL_MAC needs 6 bytes, uses 0xF1E0 - 0xF1F0 */
	OTHER_OFS_LABEL_MAC_TYPE 	= 0xF1F0,	/* LABEL_MAC_TYPE needs 4 bytes, uses 0xF1F0 - 0xF200 */
#endif /* INCLUDE_LABEL_MAC */

	OTHER_OFS_PIN				= 0xF200,
	OTHER_OFS_DEVID				= 0xF300,
	OTHER_OFS_RFPI				= 0xF400,	/* for DECT only */
	OTHER_OFS_RXTUN         		= 0xF410,
#ifdef INCLUDE_WRITE_SN
	OTHER_OFS_SN				= 0xF500,	/* for Serial Number */
#endif

#ifdef INCLUDE_WRITE_COUNTRY
	OTHER_OFS_DOMAIN			= 0xF600,	/* for Country Code  */
#endif

#ifdef INCLUDE_WRITE_FUNCTION_CODE
	OTHER_OFS_FUNCTION_CODE		= 0xF700,	/* for Function Code  */
#endif /* INCLUDE_WRITE_FUNCTION_CODE */

#ifdef INCLUDE_WRITE_MNGT_URL
	OTHER_OFS_MNGT_URL			= 0xFA00,	/* for Managemet Server URL,URL is 256 string length */
#endif /* INCLUDE_WRITE_MNGT_URL */

#ifdef INCLUDE_FACTORY_PRE_PAIRED
	OTHER_OFS_FTCONFGROUPID 	= 0xFB00,	/* for Factory pre paired  */
	OTHER_OFS_FTCONFKEY 		= 0xFC00,	/* for Factory pre paired  */
#endif /* INCLUDE_FACTORY_PRE_PAIRED */

#ifdef INCLUDE_WRITE_WIFI_PWD
	OTHER_OFS_WIFI_PWD			= 0xFD00,	/* wifi pwd needs 64 Bytes, give buffer 64 Bytes, uses 0xFD00 - 0xFD40 */
	
	#ifdef INCLUDE_WRITE_MULTI_GUEST_PWD
	OTHER_OFS_MULTI1_PWD		= 0xFDC0,	/* wifi pwd needs 64 Bytes, give buffer 64 Bytes, uses 0xFDC0 - 0xFE00 */
	OTHER_OFS_MULTI2_PWD		= 0xFE00,	/* wifi pwd needs 64 Bytes, give buffer 64 Bytes, uses 0xFE00 - 0xFE40 */
	OTHER_OFS_GUEST_PWD 		= 0xFE40,	/* wifi pwd needs 64 Bytes, give buffer 64 Bytes, uses 0xFE40 - 0xFE80 */
	#endif /* INCLUDE_WRITE_MULTI_GUEST_PWD */
#endif /* INCLUDE_WRITE_WIFI_PWD */

#ifdef INCLUDE_WRITE_ADMIN_PWD
	OTHER_OFS_ADMIN_PWD 		= 0xFD40,	/* admin pwd needs 64 Bytes, give buffer 64 Bytes, uses 0xFD40 - 0xFD80 */
#endif

#ifdef INCLUDE_WRITE_USER_PWD
	OTHER_OFS_USER_PWD			= 0xFD80,	/* user pwd needs 64 Bytes, give buffer 64 Bytes, uses 0xFD80 - 0xFDC0 */
#endif

#ifdef INCLUDE_LTEWAN
	OTHER_OFS_GOLD		= 0xB200,
#endif /* INCLUDE_LTEWAN */

	OTHER_OFS_SIGN		= 0xE000, 	/* for devid flash only, mostly for domestic product */
	OTHER_OFS_MICFLAG	= 0xD000,
	OTHER_OFS_GPONSN	= 0xC000,
#if defined(INCLUDE_SPEC_EX220)
	/* nothing, why? */
#else
	OTHER_OFS_ADDHVER	= 0xFF00,
#endif

#ifdef INCLUDE_WRITE_CWMP_PWD
	OTHER_OFS_CWMP_PWD	= 0xB000,	/*	CWMP pwd needs 256 Bytes, give buffer 256 Bytes, uses 0xB000 - 0xB100 */
#endif

	/* ------------------- add above the line -------------------- */
	OTHER_OFS_MAX
};
#endif  /* OTHER_OFS */

/************************************** type fs mount point ***************************************/
#define FS_PATH_MAX_SIZE	64
#define FS_PATH_FORMAT		"%s/0x%08X"
#define FS_PATH_MISC_RO		"/var/run/misc/misc_ro"
#define FS_PATH_MISC_RW		"/var/run/misc/misc_rw"


/***************************************** for image tag ******************************************/
#define TP_TAG_VERSION		0x03000003 
#define TP_TAG_LEN         512
#define TP_CLOUD_ID_BYTE_LEN	16
#define TP_TOKEN_LEN	20
#define TP_MAGIC_NUM_LEN	20
#define TP_SIG_LEN		128

#ifdef INCLUDE_DUAL_IMAGE
#define TP_IMAGE_SLAVE 1
#endif

#define BOOTROM_EXT_FREE_ADDR 0x80020000

/**************************************************************************************************/
/*                                           TYPES                                                */
/**************************************************************************************************/
typedef struct
{	
	unsigned long tagVersion;			/* tag version number */   
	unsigned char hardwareId[TP_CLOUD_ID_BYTE_LEN];		/* HWID for cloud */
	unsigned char firmwareId[TP_CLOUD_ID_BYTE_LEN];		/* FWID for cloud */
	unsigned char oemId[TP_CLOUD_ID_BYTE_LEN];			/* OEMID for cloud */
	unsigned long productId;	/* product id */  
	unsigned long productVer;	/* product version */
	unsigned long addHver;		/* Addtional hardware version */
	
	unsigned char imageValidToken[TP_TOKEN_LEN];	/* image validation token - md5 checksum */
	unsigned char magicNum[TP_MAGIC_NUM_LEN];	 	/* magic number */
	
	unsigned long kernelTextAddr; 	/* text section address of kernel */
	unsigned long kernelEntryPoint; /* entry point address of kernel */
	
	unsigned long totalImageLen;	/* the sum of kernelLen+rootfsLen+tagLen */
	
	unsigned long kernelAddress;	/* starting address (offset from the beginning of FILE_TAG) 
									 * of kernel image 
									 */
	unsigned long kernelLen;		/* length of kernel image */
	
	unsigned long rootfsAddress;	/* starting address (offset) of filesystem image */
	unsigned long rootfsLen;		/* length of filesystem image */

	unsigned long bootAddress;		/* starting address (offset) of bootloader image */
	unsigned long bootLen;			/* length of bootloader image */

	unsigned long swRevision;		/* software revision */
	unsigned long platformVer;		/* platform version */
	unsigned long specialVer;

	unsigned long binCrc32;			/* CRC32 for bin(kernel+rootfs) */

	unsigned long reserved1[13];	/* reserved for future */

	unsigned char sig[TP_SIG_LEN];		/* signature for update */
	unsigned char resSig[TP_SIG_LEN];	/* reserved for signature */

	unsigned long reserved2[12];	/* reserved for future */
}IMAGE_TAG;

typedef struct
{
	struct
	{
		unsigned char is_committed;		
		unsigned char is_active;		
		unsigned char is_valid;
	}image[2];
	unsigned char active_flag;
	unsigned char csum;
}IMG_BOOT_INFO;
/*
 * brief	copy end
 * By	wangwenhao, 22May17
 */


/**************************************************************************************************/
/*                                           VARIABLES                                            */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                           FUNCTIONS                                            */
/**************************************************************************************************/
void tp_boot_set_all_led_on(void);

void tp_boot_set_all_led_off(void);

/* must set this buf before using TYPE FS function */
void tp_boot_set_block_buf(unsigned char *buf);

#ifdef INCLUDE_DUAL_IMAGE
int tp_boot_get_boot_index(void);
int tp_boot_set_boot_index(int index);
#endif /* INCLUDE_DUAL_IMAGE */

#ifdef INCLUDE_MTD_TYPE_FS
int tp_boot_get_filename(unsigned int start, unsigned int end, char *prefix, char *name);

int tp_boot_read_file(unsigned int start, unsigned int end, 
						char *prefix, unsigned char *buf, unsigned int buflen);
#endif /* INCLUDE_MTD_TYPE_FS */

void tp_boot_get_compressed_kernel(unsigned char *target);

int tp_boot_write_image(unsigned char *image, unsigned int size);

#ifdef INCLUDE_MTD_TYPE_FS
int tp_boot_write_oobimage(unsigned char *image, unsigned int size);

int tp_boot_write_oobimage_partable(unsigned char *image, unsigned int size, 
									unsigned char* partable, unsigned int partable_len);
#endif /* INCLUDE_MTD_TYPE_FS */

#endif /* __TP_BOOTLIB_H__ */

