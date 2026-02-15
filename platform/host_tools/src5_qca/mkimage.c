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

/* July 29, 2004
 *
 * This is a hacked replacement for the 'trx' utility used to create
 * wrt54g .trx firmware files.  It isn't pretty, but it does the job
 * for me.
 *
 * As an extension, you can specify a larger maximum length for the
 * .trx file using '-m'.  It will be rounded up to be a multiple of 4K.
 * NOTE: This space will be malloc()'d.
 *
 * August 16, 2004
 *
 * Sigh... Make it endian-neutral.
 *
 * TODO: Support '-b' option to specify offsets for each file.
 *
 * February 19, 2005 - mbm
 *
 * Add -a (align offset) and -b (absolute offset)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <byteswap.h>
#include <getopt.h>
#include <fcntl.h>
#include <time.h>
#include <elf.h>
#include <sys/stat.h>
#include <ctype.h>

#include "mkimage.h"
#include "md5_interface.h"
#include <unistd.h>



#if 0
#define __BYTE_ORDER __LITTLE_ENDIAN

#if __BYTE_ORDER == __BIG_ENDIAN
#define STORE32_LE(X)		bswap_32(X)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define STORE32_LE(X)		(X)
#endif


#define STORE32_LE(X)		bswap_32(X)
#endif


#define CLOUD_ID_STR_LEN    33
#define BUFFER_LEN64	64


typedef struct _DEV_INFO
{
	unsigned int sw_revision;
	unsigned int platform_ver;
	unsigned int special_ver;
	unsigned int product_id;
	unsigned int product_ver;
	unsigned int add_hver;
	unsigned int dm_ver;
	char model_name[BUFFER_LEN64];
	char build_date[BUFFER_LEN64];
	unsigned int build_time;
	char dev_ver[BUFFER_LEN64];
	unsigned char hw_id[CLOUD_ID_BYTE_LEN];
	unsigned char fw_id[CLOUD_ID_BYTE_LEN];
	unsigned char oem_id[CLOUD_ID_BYTE_LEN];
	unsigned char is_beta;
	unsigned char is_trans;
	unsigned char is_datecode;
	char build_spec[BUFFER_LEN64];
}DEV_INFO;

typedef struct _qca_partion_file
{
	char partion_name[64];
	char file_name[512];
	unsigned int base_addr;
	unsigned int size;
	unsigned int file_size;
}qca_partion_file;

typedef struct _qca_partion_table
{
	unsigned int base_block;
	unsigned int end_block;
	unsigned int actual_blocks;
	unsigned int reserved;
} qca_partion_table;

#define  IMAGE_BUF_LEN (70 * 1024 * 1024)
static LINUX_FLASH_STRUCT l_flash_struct;
static DEV_INFO l_dev_info;

static int l_flash_size = 0;
static int l_boot_size = 0;
static int l_kernel_size = 0;
static int l_misc_size = 0;
static int l_target_endian = -1; /* 0-big, 1-little */

#define BLOCK_SIZE 0x20000

#define STORE32_LE(X)		(l_target_endian ? (X) : bswap_32(X))



/**********************************************************************/

void usage(void) __attribute__ (( __noreturn__ ));

void replaceBlank(char *str);
int getStrAttrVal(char *buf, char *attr, char *value, int maxLen);
int idstrToByte(const char *pIdstr, unsigned char *pByte);
int getVersion(const char *reduced_xml_name);
int fill_buffer(char *buffer, char filename[][MAX_FILENAME_LEN], char *vmlinux);


void usage(void)
{
		fprintf(stderr, 
	"Usage: mkimage [OPTIONS]\n\n"
	"  -s, --flashsize=SIZE		Flash size\n"
	"  -l, --bootsize=SIZE		MAX Bootloader size\n"
	"  -m, --maxkernelsize=SIZE	MAX Kernel size\n"
	"  -n, --miscsize=SIZE	Reserved for Misc size\n"
	"  -e, --targetEndian=endian Target CPU endian\n"
	"  -b, --boot=FILE		Boot file\n"
	"  -k, --kernel=FILE		Kernel file\n"
	"  -f, --fs=FILE			Filesystem file\n"
	"  -c, --config=FILE		Flash Config file\n"
	"  -o, --output=FILE		Output Filename prefix\n"
	"  -i, --image-path=PATH	image path\n"
	"  -p, --xmlName=FILE		reduced_data_model file\n"
	"  -d, --dualImage		    Have Dual Image\n"
	"  -t, --mtdType=type		MTD part type\n"
	"  -h, --help			    Display this message\n");

	exit(EXIT_FAILURE);
}

void replaceBlank(char *str)
{
	if (NULL == str)
	{
		fprintf(stderr, "Get NULL replace blank string\n");
		return;
	}

	while(*str != '\0')
	{
		if (' ' == *str)
		{
			*str = '_';
		}
		
		*str++;
	}

	return;
}

int getStrAttrVal(char *buf, char *attr, char *value, int maxLen)
{
	char *p = NULL; 
	char *q = NULL; 
	char *valStart = NULL; 
	char *valEnd = NULL;

	if ((buf == NULL) || (attr == NULL) || (value == NULL))
	{
		fprintf(stderr, "%s(): get attr \"%s\" parameter error\n", __FUNCTION__, attr);
		return -1;
	}
	
	p = strstr(buf, attr);		
	if (p == NULL)
	{
		fprintf(stderr, "%s(): cannot find attr %s\n", __FUNCTION__, attr);
		return -1;
	}

	q = strstr(p , "d=");
	p = strchr(q, '>');
	if ((q == NULL) || (p == NULL))
	{
		fprintf(stderr, "%s(): \"%s\" cannot find \"d=\" Or \">\"\n", __FUNCTION__, attr);
		return -1;
	}

	q += 2;
	
	while (isspace(*q))
	{
		q++;
	}

	/*string value with quote */
	if ((*q == '\"') || (*q == '\''))
	{
		valStart = q + 1;
		valEnd = strchr(valStart, *q);

		if ((valEnd > p) || (valEnd - valStart > maxLen))
		{
			fprintf(stderr, "%s(): \"%s\" can't find right quote Or exceed max len\n", __FUNCTION__, attr);
			return -1;
		}
	}
	else
	{
		valStart = q;

		while(!isspace(*q) && (*q != '>'))
		{
			if ((*q) == '/' && (*(q + 1) == '>'))
			{
				break;
			}
			q++;
		}
		valEnd = q;

		if (valEnd - valStart > maxLen)
		{
			fprintf(stderr, "%s(): value of attr \"%s\" exceed max length\n", __FUNCTION__, attr);
			return -1;
		}
	}
	
	memcpy(value, valStart, valEnd - valStart);

	return 0;
}

int idstrToByte(const char *pIdstr, unsigned char *pByte)
{
	int i = 0, c = 0;
	
	for (i = 0; i < 16; i++)
	{	
		if (*pIdstr >= '0' && *pIdstr <= '9')
		{
			c  = (unsigned char) (*pIdstr++ - '0'); 
		}
		else if (*pIdstr >= 'a' && *pIdstr <= 'f')
		{
			c  = (unsigned char) (*pIdstr++ - 'a') + 10;
		}
		else if (*pIdstr >= 'A' && *pIdstr <= 'F')
		{
			c  = (unsigned char) (*pIdstr++ - 'A') + 10; 
		}
		else
		{
			return -1;
		}

		c <<= 4;

		if (*pIdstr >= '0' && *pIdstr <= '9')
		{
			c |= (unsigned char) (*pIdstr++ - '0');
		}
		else if (*pIdstr >= 'a' && *pIdstr <= 'f')
		{
			c |= (unsigned char) (*pIdstr++ - 'a') + 10;
		}
		else if (*pIdstr >= 'A' && *pIdstr <= 'F')
		{
			c |= (unsigned char) (*pIdstr++ - 'A') + 10;
		}
		else
		{
			return -1;
		}

		pByte[i] = (unsigned char) c;

	}

	return 0;
}


int getVersion(const char *reduced_xml_name)
{
	int fd;
	char tmp[12] = {0};
	char tmpId[33] = {0};

	#define MAX_LEN	(10 * 1024)
	char buf[MAX_LEN] = {0};
	//printf("reduced_xml_name is :%s\n",reduced_xml_name);
	fd = open(reduced_xml_name, O_RDONLY);

	if (fd < 0)
	{
		perror("open profile error\n");
		return -1;
	}
	while (read(fd, buf, MAX_LEN - 1) > 0)
	{
		buf[MAX_LEN - 1] = 0;
		/* Get software revision */
		char *q = strstr(buf, "X_TP_SoftwareRevision");
		char *p = strstr(q, "d=");
		char *pEnd = NULL;
		int strLen = 0;
		
		if (p != 0)
		{
			p += 2;
			strncpy(tmp, p, 10);			
			l_dev_info.sw_revision = strtoul(tmp, NULL, 16);
		}
		/* Get platform version */
		p = strstr(buf, "X_TP_PlatformVersion");
		q = strstr(p, "d=");
		if (q != 0)
		{
			q += 2;
			strncpy(tmp, q, 10);			
			l_dev_info.platform_ver = strtoul(tmp, NULL, 16);  
		}
		/* Get product Id */		
		p = strstr(buf, "X_TP_ProductID");
		q = strstr(p, "d=");
		if (q != 0)
		{
			q += 2;
			strncpy(tmp, q, 10);			
			l_dev_info.product_id = strtoul(tmp, NULL, 16);  
		}
		/* Get product version */		
		p = strstr(buf, "X_TP_ProductVersion");
		q = strstr(p, "d=");
		if (q != 0)
		{
			q += 2;
			strncpy(tmp, q, 10);			
			l_dev_info.product_ver = strtoul(tmp, NULL, 16);  
		}
		/* Get special version */		
		p = strstr(buf, "X_TP_SpecialVersion");
		if(p != NULL)
		{
			q = strstr(p, "d=");
			if (q != 0)
			{
				q += 2;
				strncpy(tmp, q, 10);			
				l_dev_info.special_ver = strtoul(tmp, NULL, 16);  
			}
		}
		/* Get Additional HardwareVersion */		
		memset(tmp, 0, 12);	
		if (getStrAttrVal(buf, "AdditionalHardwareVersion", tmp, 10) != 0)
		{
			close(fd);
			return -1;
		}
		l_dev_info.add_hver = strtoul(tmp, NULL, 16);

		/* Get DataModel Version */
		memset(tmp, 0, 12);
		if (getStrAttrVal(buf, "X_TP_DMVersion", tmp, 10) != 0)
		{
			l_dev_info.dm_ver = 0;
		}
		else
		{
			l_dev_info.dm_ver = strtoul(tmp, NULL, 16);
		}

		/* Get build date */		
		if (getStrAttrVal(buf, "X_TP_BuildDate", l_dev_info.build_date, BUFFER_LEN64) != 0)
		{
			close(fd);
			return -1;
		}		
		/* Get build time */
		p = strstr(buf, "X_TP_BuildTime");
		q = strstr(p, "d=");

		memset(tmp, 0, 12);
		if (q != 0)
		{
			q += 2;
			strncpy(tmp, q, 5);
			l_dev_info.build_time = strtoul(tmp, NULL, 10);  
		}

		/* Get device model version */		
		if (getStrAttrVal(buf, "X_TP_DevModelVersion", l_dev_info.dev_ver, BUFFER_LEN64) != 0)
		{
			close(fd);
			return -1;
		}

		/* Get model name */		
		if (getStrAttrVal(buf, "ModelName", l_dev_info.model_name, BUFFER_LEN64) != 0)
		{
			close(fd);
			return -1;
		}
		/* Get HardwareID */		
		if (getStrAttrVal(buf, "X_TP_HardwareID", tmpId, CLOUD_ID_STR_LEN) != 0)
		{
			close(fd);
			return -1;
		}

		if (idstrToByte(tmpId, l_dev_info.hw_id) != 0)
		{
			close(fd);
			return -1;
		}
		/* Get FirmwareID */	
		if (getStrAttrVal(buf, "X_TP_FirmwareID", tmpId, CLOUD_ID_STR_LEN) != 0)
		{
			close(fd);
			return -1;
		}

		printf("fwid is %s\n", tmpId);

		if (idstrToByte(tmpId, l_dev_info.fw_id) != 0)
		{
			close(fd);
			return -1;
		}		

		/* Get OemID */	
		if (getStrAttrVal(buf, "X_TP_OemID", tmpId, CLOUD_ID_STR_LEN) != 0)
		{
			close(fd);
			return -1;
		}
		printf("oemid is %s\n", tmpId);
		if (idstrToByte(tmpId, l_dev_info.oem_id) != 0)
		{
			close(fd);
			return -1;
		}

		/* Get beta info */		
		p = strstr(buf, "X_TP_IsBeta");
		
		if	(p)
		{
			q = strstr(p, "d=");
		
			if (q != 0)
			{
				q += 2;
				strncpy(tmp, q, 10);			
				l_dev_info.is_beta = strtoul(tmp, NULL, 16);  
			}
		}
		/* Get trans info */		
		p = strstr(buf, "X_TP_IsTrans");
		
		if	(p)
		{
			q = strstr(p, "d=");
		
			if (q != 0)
			{
				q += 2;
				strncpy(tmp, q, 10);			
				l_dev_info.is_trans = strtoul(tmp, NULL, 16);  
			}
		}

		/* Get date code info */
		p = strstr(buf, "X_TP_IsDateCode");
		
		if	(p)
		{
			q = strstr(p, "d=");

			if (q != 0)
			{
				q += 2;
				strncpy(tmp, q, 10);
				l_dev_info.is_datecode = strtoul(tmp, NULL, 16);
			}
		}
		/* Get build date */		
		if (getStrAttrVal(buf, "X_TP_BuildSpec", l_dev_info.build_spec, BUFFER_LEN64) != 0)
		{
			close(fd);
			return -1;
		}

		break;
	}

	close(fd);

	return 0;
}

int fill_tag_buffer(LINUX_FILE_TAG *tag,int image_size)
{
	tag->tagVersion = STORE32_LE(TAG_VERSION);
	memcpy(tag->magicNum, magicNum, MAGIC_NUM_LEN);

	tag->productId = STORE32_LE(l_dev_info.product_id);
	tag->productVer = STORE32_LE(l_dev_info.product_ver);

	tag->swRevision = STORE32_LE(l_dev_info.sw_revision);
	tag->platformVer = STORE32_LE(l_dev_info.platform_ver);
	tag->addHver = STORE32_LE(l_dev_info.add_hver);
	tag->dmVersion = STORE32_LE(l_dev_info.dm_ver);

	tag->specialVer = STORE32_LE(l_dev_info.special_ver);

	memcpy(tag->hardwareId, l_dev_info.hw_id, CLOUD_ID_BYTE_LEN);
	memcpy(tag->firmwareId, l_dev_info.fw_id, CLOUD_ID_BYTE_LEN);
	memcpy(tag->oemId, l_dev_info.oem_id, CLOUD_ID_BYTE_LEN);

	memset(tag->imageValidToken, 0, TOKEN_LEN);

	tag->totalImageLen = STORE32_LE(image_size + TAG_LEN);


	memset(tag->sig, 0, SIG_LEN);
	memset(tag->resSig, 0, SIG_LEN);
	return EXIT_SUCCESS;
}
int fill_buffer(char *buffer, char filename[][MAX_FILENAME_LEN], char *vmlinux)
{
	FILE *in;
	size_t n;
	size_t boot_len, kernel_len, fs_len, config_len;
	uint32_t cur_len;
	char *buf = buffer;
	unsigned int maxlen = l_flash_size;
	struct _LINUX_FILE_TAG *kernel_tag, *boot_tag;
	unsigned int image_len;
	int vm_fd;
	Elf32_Ehdr ehdr; 
	Elf32_Shdr shdr;
	
	fprintf(stderr, "================== BUFFER STRUCTURE =================\n");
	/* 1) bootloader */
	boot_tag = (struct _LINUX_FILE_TAG *)buf;
	
	cur_len = BOOTLOADER_TAG_LEN;
	
	if (!(in= fopen(filename[BOOT], "r"))) {
		fprintf(stderr, "can not open \"%s\" for reading\n", filename[BOOT]);
		usage();
	}
	fprintf(stderr, "bootloader is 0x%x\n", cur_len);
	n = fread(buf + cur_len, 1, maxlen - cur_len, in);
	if (!feof(in)) {
		fprintf(stderr, "fread failure or file \"%s\" too large\n", filename[BOOT]);
		fclose(in);
		return EXIT_FAILURE;
	}	
	fclose(in);

	/* 对齐操作，比如之前n为816645，操作之后n变为816648 */
	if (n & (ROUND-1)) {
		memset(buf + cur_len + n, 0, ROUND - (n & (ROUND-1)));
		n += ROUND - (n & (ROUND-1));
	}

	boot_len = n;
	
	if (boot_len > l_boot_size)
	{
		fprintf(stderr, "boot size is %x, too large %x\n", boot_len, (l_flash_struct.kernelOffset - l_flash_struct.bootOffset));
		return EXIT_FAILURE;
	}
	/* end read bootloader */

	/* 2) linux kernel */
	cur_len = l_flash_struct.kernelOffset + BOOTLOADER_TAG_LEN;
	kernel_tag = (struct _LINUX_FILE_TAG *) (buf + cur_len);

	cur_len += TAG_LEN;

	if (!(in= fopen(filename[KERNEL], "r"))) {
		fprintf(stderr, "can not open \"%s\" for reading\n", filename[KERNEL]);
		usage();
	}
	fprintf(stderr, "kernel is 0x%x\n", cur_len);
	n = fread(buf + cur_len, 1, maxlen - cur_len, in);
	if (!feof(in)) {
		fprintf(stderr, "fread failure or file \"%s\" too large\n", filename[KERNEL]);
		fclose(in);
		return EXIT_FAILURE;
	}	
	fclose(in);

	if (n & (ROUND-1)) {
		memset(buf + cur_len + n, 0, ROUND - (n & (ROUND-1)));
		n += ROUND - (n & (ROUND-1));
	}

	kernel_len = n;

	/* use dynamic value by yangxv, 2011.11.20 */
	/* if (kernel_len >= MAX_KERNEL_SIZE) */
	if (kernel_len > l_kernel_size-TAG_LEN)
	{
		fprintf(stderr, "kernel size is %x, too large\n", kernel_len);
		return EXIT_FAILURE;
	}
	/* end read linux kernel */

	/* 3) rootfs */
	cur_len = l_flash_struct.rootfsOffset + BOOTLOADER_TAG_LEN;
	
	if (!(in= fopen(filename[FS], "r"))) {
		fprintf(stderr, "can not open \"%s\" for reading\n", filename[FS]);
		usage();
	}

	fprintf(stderr, "rootfs is 0x%x\n", cur_len);
	n = fread(buf + cur_len, 1, maxlen - cur_len, in);
	if (!feof(in)) {
		fprintf(stderr, "fread failure or file \"%s\" too large\n", filename[FS]);
		fclose(in);
		return EXIT_FAILURE;
	}	
	fclose(in);

	if (n & (ROUND-1)) {
		memset(buf + cur_len + n, 0, ROUND - (n & (ROUND-1)));
		n += ROUND - (n & (ROUND-1));
	}

	fs_len = n;

	if (fs_len > l_flash_struct.appSize - l_kernel_size)
	{
		fprintf(stderr, "rootfs size is %x, too large\n", fs_len);
		return EXIT_FAILURE;
	}
	/* end read rootfs */

	/* 4) config */
	if(strlen(filename[CONFIG]))
	{
		USRCONF_STRUCT *pUsrconf = NULL;
		unsigned long *pUintAddr = NULL;
		unsigned long index = 0;
		unsigned long checksum = 0;

		cur_len = l_flash_struct.configOffset + sizeof(USRCONF_STRUCT) + BOOTLOADER_TAG_LEN;

		if (!(in= fopen(filename[CONFIG], "r"))) {
			fprintf(stderr, "can not open \"%s\" for reading\n", filename[CONFIG]);
			usage();
		}

		fprintf(stderr, "config is 0x%x\n", cur_len);
		n = fread(buf + cur_len, 1, maxlen - cur_len, in);
		if (!feof(in)) {
			fprintf(stderr, "fread failure or file \"%s\" too large\n", filename[CONFIG]);
			fclose(in);
			return EXIT_FAILURE;
		}	
		fclose(in);

		if (n & (ROUND-1)) {
			memset(buf + cur_len + n, 0, ROUND - (n & (ROUND-1)));
			n += ROUND - (n & (ROUND-1));
		}

		config_len = n;

		if (config_len > 0x10000) /* will it bigger?*/
		{
			fprintf(stderr, "config size is %x, too large\n", config_len);
			return EXIT_FAILURE;
		}
		
		pUsrconf = (USRCONF_STRUCT *)(buf + cur_len - sizeof(USRCONF_STRUCT));
		pUsrconf->length = STORE32_LE(config_len);
		pUsrconf->signature = STORE32_LE(CONF_FLASH_SIGNATURE);
		pUsrconf->isCompressed = STORE32_LE(0x0);
		pUsrconf->checksum = STORE32_LE(0x0);
		buf[cur_len + config_len] = '\0';
		fprintf(stderr, "config size is %d\n", config_len);
		/* Calculate checksum. */
		pUintAddr = (unsigned long *)pUsrconf;
		for (index = 0; index < (config_len + sizeof(USRCONF_STRUCT)) / sizeof(unsigned long); index++)
		{
			checksum += STORE32_LE(pUintAddr[index]);
		}
		
		pUsrconf->checksum = STORE32_LE(0 - checksum);

	}
	/* 5) kernel tag */
	if ((vm_fd = open(vmlinux, O_RDONLY)) < 0) {
		fprintf(stderr, "can not open \"%s\" for reading\n", vmlinux);
		usage();
	}
	
	read(vm_fd, &ehdr, sizeof(ehdr)); 
	read(vm_fd, &shdr, sizeof(shdr));
	
	close(vm_fd);				
	 
	printf("Entry Point: %#X\n", STORE32_LE(ehdr.e_entry));
	printf("Text Addr: %#X\n", STORE32_LE(shdr.sh_addr));

	boot_tag->tagVersion = STORE32_LE(TAG_VERSION);
	memcpy(boot_tag->magicNum, magicNum, MAGIC_NUM_LEN);

	boot_tag->productId = STORE32_LE(l_dev_info.product_id);
	boot_tag->productVer = STORE32_LE(l_dev_info.product_ver);

	boot_tag->swRevision = STORE32_LE(l_dev_info.sw_revision);
	boot_tag->platformVer = STORE32_LE(l_dev_info.platform_ver);
	boot_tag->addHver = STORE32_LE(l_dev_info.add_hver);

	boot_tag->specialVer = STORE32_LE(l_dev_info.special_ver);

	memcpy(boot_tag->hardwareId, l_dev_info.hw_id, CLOUD_ID_BYTE_LEN);
	memcpy(boot_tag->firmwareId, l_dev_info.fw_id, CLOUD_ID_BYTE_LEN);
	memcpy(boot_tag->oemId, l_dev_info.oem_id, CLOUD_ID_BYTE_LEN);

	memset(boot_tag->imageValidToken, 0, TOKEN_LEN);
	
	boot_tag->kernelTextAddr = (shdr.sh_addr);
	boot_tag->kernelEntryPoint = (ehdr.e_entry);

	image_len = l_flash_struct.appSize;
	boot_tag->totalImageLen = STORE32_LE(image_len);

	boot_tag->bootAddress = STORE32_LE(0x0);
	boot_tag->bootLen = STORE32_LE(0x0);

	boot_tag->kernelAddress = STORE32_LE(TAG_LEN);
	boot_tag->kernelLen = STORE32_LE(kernel_len);

	boot_tag->rootfsAddress = STORE32_LE(l_flash_struct.rootfsOffset - l_flash_struct.kernelOffset);
	boot_tag->rootfsLen = STORE32_LE(fs_len);


	memset(boot_tag->sig, 0, SIG_LEN);
	memset(boot_tag->resSig, 0, SIG_LEN);
	
	/* deleted by yangxv,
	 * do md5 checksum caculate when signature firmware
	 */
#if 0
	unsigned char md5[MD5_HASH_SIZE];

	memcpy(kernel_tag->imageValidToken, md5Key, 16);
	md5_make_digest(md5, (unsigned char*)kernel_tag, image_len);
	memcpy(kernel_tag->imageValidToken, md5, MD5_HASH_SIZE);
	/* end fill kernel tag */
#endif

#if 0
	/* 5) bootloader tag */
	int up_boot_image_len = l_flash_struct.configOffset + BOOTLOADER_TAG_LEN;

	/* some value use are same as kernel tag */
	memcpy(boot_tag, kernel_tag, TAG_LEN);
	
	boot_tag->totalImageLen = STORE32_LE(up_boot_image_len);
	boot_tag->bootloaderAddress	= STORE32_LE(0x0);
	boot_tag->bootloaderLen = STORE32_LE(boot_len);

	memcpy(boot_tag->imageValidationToken, mk5Key_bootloader, 16);
	md5_make_digest(md5, (unsigned char*)boot_tag, up_boot_image_len);
	memcpy(boot_tag->imageValidationToken, md5, MD5_HASH_SIZE);
	/* end fill boot tag */
#endif
	/* 5) fill default mac and pin */
	memcpy(buf + BOOTLOADER_TAG_LEN + l_flash_struct.miscOffset + MAC_MISC_OFFSET, def_mac, 6);
	memset(buf + BOOTLOADER_TAG_LEN + l_flash_struct.miscOffset + PIN_MISC_OFFSET, 0, 8);
	/* end fill mac and pin */

    /* 6) bootloader tag */
    int up_boot_image_len 
    	= l_flash_struct.appSize + (l_flash_struct.kernelOffset - l_flash_struct.bootOffset) + BOOTLOADER_TAG_LEN;

    /* some value use are same as kernel tag */
	if (l_kernel_size > 0)
    	memcpy(kernel_tag, boot_tag, TAG_LEN);

    boot_tag->totalImageLen = STORE32_LE(up_boot_image_len);
    boot_tag->bootAddress     = STORE32_LE(0x0);
    boot_tag->bootLen = STORE32_LE(boot_len);

#if 0
    memcpy(boot_tag->imageValidToken, mk5Key_bootloader, 16);
    md5_make_digest(md5, (unsigned char*)boot_tag, up_boot_image_len);
    memcpy(boot_tag->imageValidToken, md5, MD5_HASH_SIZE);
#endif
    /* end fill boot tag */
		
		fprintf(stderr, "=====================================================\n\n");
		
		return EXIT_SUCCESS;
}
int paser_partion_file(char *parttion_file_name,qca_partion_file image_file[],char *file_path, int x64_flag)
{
	FILE *fp = NULL;
	char line[512] = {0};
	char *pLine = NULL;
	char *pFit = NULL;
	char *pChar = NULL;
	int parttion_check = 0;
	int image_num = 0;
	char tmp_file_name[64] = {0};

	if(parttion_file_name == NULL || parttion_file_name[0] == 0)
	{
		printf("parttion file is null");
		return 0;
	}

	fp = fopen(parttion_file_name,"r");
	if(fp == NULL)
	{
		printf("open parttion %s fail\n",parttion_file_name);
		return 0;
	}
	while((pLine = fgets(line,sizeof(line),fp)) != NULL)
	{
		pFit= strstr(pLine,"<partition>");
		if((pFit == NULL) && (parttion_check == 0))
		{
			continue;
		}
		else if(pFit != NULL)
		{
			if(parttion_check == 1)
			{
				printf("%s,%d,ERROR parttion_file_name format is ERROR\n",__FUNCTION__,__LINE__);
				fclose(fp);
				return 0;
			}
			parttion_check = 1;
			continue;
		}
		pFit = strstr(pLine,"</partition>");
		if(pFit != NULL)
		{
			if(parttion_check == 0)
			{
				printf("%s,%d,ERROR parttion_file_name format is ERROR\n",__FUNCTION__,__LINE__);
				fclose(fp);
				return 0;
			}
			parttion_check = 0;
			image_num++;
			continue;
		}
		pFit = strstr(pLine,"<name ");
		if(pFit != NULL)
		{
			pChar = strchr(pLine,'>');
			if(pChar != NULL)
			{
				pFit = strstr(pChar,"</name>");
				if(pFit == NULL)
				{
					printf("%s,%d,ERROR parttion_file_name format is ERROR\n",__FUNCTION__,__LINE__);
					printf("error line :%s\n",line);
					fclose(fp);
					return 0;
				}
				strncpy(image_file[image_num].partion_name,pChar +1,(pFit - pChar -1));
			}
			else
			{
				printf("%s,%d,ERROR parttion_file_name format is ERROR\n",__FUNCTION__,__LINE__);
				printf("error line :%s\n",line);
				fclose(fp);
				return 0;
			}
			continue;
		}
		pFit = strstr(pLine,"<size_kb");
		if(pFit != NULL)
		{
			pChar = strchr(pLine,'>');
			if(pChar != NULL)
			{
				pFit = strstr(pChar,"</size_kb>");
				if(pFit == NULL)
				{
					printf("%s,%d,ERROR parttion_file_name format is ERROR\n",__FUNCTION__,__LINE__);
					printf("error line :%s\n",line);
					fclose(fp);
					return 0;
				}

				*pFit = 0;
				image_file[image_num].size = atoi(pChar + 1) * 1024;
			}
			else
			{
				printf("%s,%d,ERROR parttion_file_name format is ERROR\n",__FUNCTION__,__LINE__);
				printf("error line :%s\n",line);
				fclose(fp);
				return 0;
			}
			continue;
		}

		pFit = strstr(pLine,"<pad_kb");
		if(pFit != NULL)
		{
			pChar = strchr(pLine,'>');
			if(pChar != NULL)
			{
				pFit = strstr(pChar,"</pad_kb>");
				if(pFit == NULL)
				{
					printf("%s,%d,ERROR parttion_file_name format is ERROR\n",__FUNCTION__,__LINE__);
					printf("error line :%s\n",line);
					fclose(fp);
					return 0;
				}

				*pFit = 0;
				image_file[image_num].size += atoi(pChar + 1) * 1024;
			}
			else
			{
				printf("%s,%d,ERROR parttion_file_name format is ERROR\n",__FUNCTION__,__LINE__);
				printf("error line :%s\n",line);
				fclose(fp);
				return 0;
			}
			continue;
		}
		pFit = strstr(pLine,"<img_name");
		if(pFit != NULL)
		{
			pChar = strchr(pLine,'>');
			if(pChar != NULL)
			{
				pFit = strstr(pChar,"</img_name>");
				if(pFit == NULL)
				{
					printf("%s,%d,ERROR parttion_file_name format is ERROR\n",__FUNCTION__,__LINE__);
					printf("error line :%s\n",line);
					fclose(fp);
					return 0;
				}
				/*partition file may configure 32 bit and 64 bit rootfs file */
				if( (!x64_flag && strstr(pLine,"mode=\"64\"") == NULL) ||  /* filter for 32 bit image */
					(x64_flag && strstr(pLine,"mode=\"32\"") == NULL) )	/* filter for 64 bit image */
				{
					//strncpy(image_file[image_num].file_name,pChar +1,(pFit - pChar -1));
					memset(tmp_file_name,0,sizeof(tmp_file_name));
					strncpy(tmp_file_name,pChar +1,(pFit - pChar -1));
					snprintf(image_file[image_num].file_name,sizeof(image_file[image_num].file_name),\
						"%s%s",file_path,tmp_file_name);
				}
			}
			else
			{
				printf("%s,%d,ERROR parttion_file_name format is ERROR\n",__FUNCTION__,__LINE__);
				printf("error line :%s\n",line);
				fclose(fp);
				return 0;
			}
			continue;
		}
	}

	fclose(fp);
	if(parttion_check == 1)
	{
		printf("%s,%d,ERROR parttion_file_name format is ERROR\n",__FUNCTION__,__LINE__);
		return 0;
	}
	return image_num;
}

int make_partition_table_file(int file_num, qca_partion_file *image_file, char *out_filename)
{
	int i = 0, partitionNum = 0;
	unsigned int baseBlock, endBlock, actualBlocks;
	qca_partion_file *pPartition = NULL;
	qca_partion_table *pPartitionTbl = NULL, *pTblTmp = NULL;
	FILE *out = NULL;

	pPartitionTbl = (qca_partion_table *)malloc(file_num * sizeof(qca_partion_table));

	if (!pPartitionTbl)
	{
		printf("malloc qca_partion_table failed!\n");
		return -1;
	}

	memset(pPartitionTbl, 0xFF, file_num * sizeof(qca_partion_table));

	for (i = 0; i < file_num; i++)
	{
		pPartition = &image_file[i];
		if (pPartition->file_name[0] == 0)
		{
			continue;
		}

		pTblTmp = pPartitionTbl + partitionNum;
		pTblTmp->base_block = STORE32_LE(pPartition->base_addr / BLOCK_SIZE);
		pTblTmp->end_block = STORE32_LE((pPartition->base_addr + pPartition->size) / BLOCK_SIZE);

		actualBlocks = pPartition->file_size / BLOCK_SIZE;
		if (pPartition->file_size % BLOCK_SIZE)
			actualBlocks++;
		pTblTmp->actual_blocks = STORE32_LE(actualBlocks);
#if 0
		fprintf(stderr, "base_block = %3d, end_block = %3d, actual_blocks = %3d\n", 
			pTblTmp->base_block, pTblTmp->end_block, pTblTmp->actual_blocks);
#endif
		partitionNum++;
	}

	// create partition table file
	out = fopen(out_filename, "w");
	if (!out)
	{
		fprintf(stderr, "can not open partition table file \"%s\" for writing\n", out_filename);
		return -1;
	}

	partitionNum++;
	if (!fwrite(pPartitionTbl, partitionNum * sizeof(qca_partion_table), 1, out) || fflush(out))
	{
		fclose(out);
		free(pPartitionTbl);
		fprintf(stderr, "partition table file, fwrite failed\n");
		return -1;
	}

	fclose(out);
	free(pPartitionTbl);

	return 0;
}

int make_qca_flash_image(int file_num,qca_partion_file image_file[],char *image_buf,int buf_len,LINUX_FILE_TAG* file_tag)
{
	int index = 0;
	int data_len = 0;
	int count = 0;
	int fd = 0;
	int file_len = 0;
	int ret = 0;
	struct stat fileStat;
	qca_partion_file *pPartition = NULL;

	if((image_buf == NULL) || (file_tag == NULL))
	{
		return -1;
	}

	for (index = 0; index < file_num; index++)
	{
		pPartition = &image_file[index];
		//printf("partion_name = %s, file_name = %s\n", pPartition->partion_name, pPartition->file_name);

		if (data_len + pPartition->size > buf_len)
		{
			printf("ERROR image lager than %d\n",buf_len);
			return -1;
		}

		if (pPartition->file_name[0] == 0)
		{
			data_len = data_len + pPartition->size;
			continue;
		}

		ret = stat(pPartition->file_name,&fileStat);
		if(ret < 0)
		{
			printf("ERROR get %s stat info fail\n", pPartition->file_name);
			return -1;
		}

		pPartition->base_addr = data_len;
		pPartition->file_size = (unsigned int)fileStat.st_size;
		if (pPartition->file_size > pPartition->size)
		{
			printf("%s size: %d is larger the the partition size %d,\n",
				pPartition->file_name, pPartition->file_size, pPartition->size);
			return -1;
		}

		if(strcmp(pPartition->partion_name,"0:SBL1") == 0)
		{
			file_tag->sblAddress = data_len;
			file_tag->sblLen = pPartition->size;
		}
		else if(strcmp(pPartition->partion_name,"0:MIBIB") == 0)
		{
			file_tag->mibibAddress= data_len;
			file_tag->mibibLen= pPartition->size;
		}
		else if(strcmp(pPartition->partion_name,"0:BOOTCONFIG") == 0)
		{
			file_tag->bootCfgAddress= data_len;
			file_tag->bootCfgLen= pPartition->size;
		}
		else if(strcmp(pPartition->partion_name,"0:QSEE") == 0)
		{
			file_tag->qseeAddress= data_len;
			file_tag->qseeLen= pPartition->size;
		}
		else if(strcmp(pPartition->partion_name,"0:DEVCFG") == 0)
		{
			file_tag->devCfgAddress= data_len;
			file_tag->devCfgLen= pPartition->size;
		}
		else if(strcmp(pPartition->partion_name,"0:CDT") == 0)
		{
			file_tag->cdtAddress= data_len;
			file_tag->cdtLen= pPartition->size;
		}
		else if(strcmp(pPartition->partion_name,"0:APPSBL") == 0)
		{
			file_tag->bootAddress= data_len;
			file_tag->bootLen= pPartition->size;
		}
		else if(strcmp(pPartition->partion_name,"rootfs") == 0)
		{
			file_tag->rootfsAddress= data_len;
			file_tag->rootfsLen= pPartition->file_size;
		}

		printf("parttion = %15s, base = 0x%08x, size = 0x%08x, file = %s\n", 
			pPartition->partion_name, data_len, 
			pPartition->size, pPartition->file_name);

		fd = open(pPartition->file_name, O_RDONLY);
		if (fd < 0)
		{
			printf("ERROR open %s fail\n", pPartition->file_name);
			return -1;
		}

		count = read(fd, image_buf + data_len, pPartition->file_size);
		close(fd);
		if(count != pPartition->file_size)
		{
			printf("ERROR read file %s fail\n", pPartition->file_name);
			return -1;
		}

		if(strcmp(pPartition->partion_name, "rootfs") == 0)
		{
			// use file size instead of partition size, which makes the flash image smaller
			data_len = data_len + count;
			printf("reach the last partition %s\n", pPartition->partion_name);
			break;
		}

		data_len = data_len + pPartition->size;
	}

	return data_len;
}

static int insert_ecc_to_image(char *in_file_name, char *out_file_name)
{
    char cmd[512] = {0};
    snprintf(cmd, 512, "./img_gen '%s' -h -4 -O 64 -o '%s' ", in_file_name, out_file_name);
    //snprintf(cmd, 512, "./img_gen %s -h -4 -o %s", in_file_name, out_file_name);
    system(cmd);
    return 0;
}

int main(int argc, char **argv)
{
	FILE *out = stdout;
	char *ofn = NULL;
	char *buf;
	char filename[FILE_NUM][MAX_FILENAME_LEN] = {0, 0, 0, 0};
	
	char vmlinux_name[MAX_FILENAME_LEN] = {0};
	char reduced_xml_name[MAX_FILENAME_LEN] = {0};
	char image_path[MAX_FILENAME_LEN] = {0};

	int c = 0;

	char prefix[MAX_FILENAME_LEN] = {0};
	char suffix[MAX_FILENAME_LEN] = {0};
	char out_filename[MAX_FILENAME_LEN] = {0};
	char out_filename2[MAX_FILENAME_LEN] = {0};
    char customized_name[MAX_FILENAME_LEN] = {0};

	char image_name_prefix[MAX_FILENAME_LEN] = {0};
	char image_name_prefix_full[MAX_FILENAME_LEN] = {0};
	char flash_image_name_suffix[MAX_FILENAME_LEN] = {0};
	char update_image_name_suffix[MAX_FILENAME_LEN] = {0};

	int dual_image = 0;
	int mtd_type = 0;
	char parttion_file_name[MAX_FILENAME_LEN] = {0};
	int qca_flag = 0;
	int file_num = 0;
	qca_partion_file image_file[32] = {0};
	char *image_buf = NULL;
	int index = 0;
	int image_buf_size = 0;
	int real_image_size = 0;
	char image_directory[64] = {0};
	struct _LINUX_FILE_TAG *file_tag;
	int x64_flag = 0;
	int oob_flag = 0;
	int uboot_flag = 0;
	
	fprintf(stderr, "YangXv <yangxu@tp-link.com.cn> modified from trx for make image\n");

	static const struct option arg_options[] = {
		{"flashsize",	required_argument,	0, 's'},
		{"bootsize",	required_argument,	0, 'l'},
		{"maxkernelsize",	required_argument,	0, 'm'},
		{"miscsize",	required_argument,	0, 'n'},
		{"targetEndian",required_argument,	0, 'e'},
		{"boot", 		required_argument,	0, 'b'},
		{"kernel",		required_argument,	0, 'k'},
		{"fs",			required_argument,	0, 'f'},
		{"config",		required_argument,	0, 'c'},
		{"output",		required_argument,	0, 'o'},
		{"xmlName",		required_argument,	0, 'p'},
		{"imagep",		required_argument,	0, 'i'},
		{"vmlinux",		required_argument,	0, 'v'},
		{"dualImage",	required_argument,	0, 'd'},
		{"mtdType",		required_argument,	0, 't'},
		{"customized sp name", required_argument, 0, 'x'},
		{"partition_file",     required_argument,0,'P' },
		{"directory",    required_argument, 0, 'D'},
		{"X64",			required_argument,	0,	'X'},
		{"flashWithOOB",    required_argument, 0, 'O'},
		{"help",		no_argument,		0, 'h'},
		{0, 0, 0, 0}
	};	
	
	/* get options */
	while (1) {
		int option_index = 0;
		c = getopt_long(argc, argv, "s:l:m:n:e:b:k:f:c:r:a:o:p:q:i:v:d:t:x:P:D:X:O:u:h", arg_options, &option_index);
		if (c == -1) break;
		
		switch (c) {
		case 's':
			sscanf(optarg, "%X", &l_flash_size);
			break;
		case 'l':
			sscanf(optarg, "%X", &l_boot_size);
			break;
		case 'm':
			sscanf(optarg, "%X", &l_kernel_size);
			break;			
		case 'n':
			sscanf(optarg, "%X", &l_misc_size);
			break;		
		case 'e':
			sscanf(optarg, "%d", &l_target_endian);
			break;
		case 'b':
			strncpy(filename[BOOT], optarg, 
				strlen(optarg) > MAX_FILENAME_LEN?MAX_FILENAME_LEN:strlen(optarg));
			break;
		case 'k':
			strncpy(filename[KERNEL], optarg, 
				strlen(optarg) > MAX_FILENAME_LEN?MAX_FILENAME_LEN:strlen(optarg));
			break;
		case 'f':
			strncpy(filename[FS], optarg, 
				strlen(optarg) > MAX_FILENAME_LEN?MAX_FILENAME_LEN:strlen(optarg));
			break;
		case 'c':
			strncpy(filename[CONFIG], optarg, 
				strlen(optarg) > MAX_FILENAME_LEN?MAX_FILENAME_LEN:strlen(optarg));
			break;
		case 'r':
			strncpy(filename[ONLYREAD], optarg, 
				strlen(optarg) > MAX_FILENAME_LEN?MAX_FILENAME_LEN:strlen(optarg));
			break;
		case 'q':
			strncpy(filename[PREHOOK], optarg,
			strlen(optarg) > MAX_FILENAME_LEN?MAX_FILENAME_LEN:strlen(optarg));
			break;
		case 'v':
			strncpy(vmlinux_name, optarg, 
				strlen(optarg) > MAX_FILENAME_LEN?MAX_FILENAME_LEN:strlen(optarg));
			break;
		case 'a':
			strncpy(suffix, optarg, 
				strlen(optarg) > MAX_FILENAME_LEN?MAX_FILENAME_LEN:strlen(optarg));
			break;
		case 'o':
			strncpy(prefix, optarg, 
				strlen(optarg) > MAX_FILENAME_LEN?MAX_FILENAME_LEN:strlen(optarg));
			break;
		case 'p':
			strncpy(reduced_xml_name, optarg,
				strlen(optarg) > MAX_FILENAME_LEN?MAX_FILENAME_LEN:strlen(optarg));
			break;
		case 'i':
			strncpy(image_path, optarg,
				strlen(optarg) > MAX_FILENAME_LEN?MAX_FILENAME_LEN:strlen(optarg));
			break;
		case 'd':
			dual_image = 1;
			break;
		case 't':
			sscanf(optarg, "%d", &mtd_type);
			break;
		case 'x':
			sscanf(optarg, "%s", customized_name);
 			break;
		case 'P':
			strncpy(parttion_file_name, optarg,
				strlen(optarg) > MAX_FILENAME_LEN?MAX_FILENAME_LEN:strlen(optarg));
			qca_flag = 1;
			break;
		case 'D':
			strncpy(image_directory, optarg,
				strlen(optarg) > MAX_FILENAME_LEN?MAX_FILENAME_LEN:strlen(optarg));
			qca_flag = 1;
			break;
		case 'X':
			x64_flag = 1;
			break;
		case 'O':
			oob_flag = 1;
			break;
		case 'u':
			uboot_flag = 1;
			break;
		case 'h':
			usage();
			break;
		default:
			break;
		}
	}

	if (parttion_file_name[0] == 0)
	{
		printf("ERROR please input partition file use -P optino\n");
		return -1;
	}

	memset(image_file,0,sizeof(image_file));
	file_num = paser_partion_file(parttion_file_name,image_file,image_directory, x64_flag);

	if (file_num == 0)
	{
		printf("parttion file error\n");
		return -1;
	}

	for (index = 0; index < file_num; index++)
	{
		//printf("index=%d, partion_name=%s, file_name=%s\n", 
		//	index, image_file[index].partion_name, image_file[index].file_name);

		if (0 == strcmp(image_file[index].partion_name, "misc_data_rw"))
		{
			snprintf(image_file[index].file_name,sizeof(image_file[index].file_name),
				"%s%s", image_directory, filename[CONFIG]);
		}
		else if (0 == strcmp(image_file[index].partion_name, "misc_data_ro"))
		{
			snprintf(image_file[index].file_name,sizeof(image_file[index].file_name),
				"%s%s", image_directory, filename[ONLYREAD]);
		}

		image_buf_size = image_buf_size + image_file[index].size;
	}

	if (image_buf_size > l_flash_size)
	{
		printf("image_buf_size = 0x%x, which exceeds flash size 0x%x!\n", image_buf_size, l_flash_size);
		return -1;
	}

	image_buf = (char*)malloc(image_buf_size + TAG_LEN);
	if (image_buf == NULL)
	{
		printf("ERROR malloc image buf fail\n");
		return -1;
	}

	memset(image_buf, 0xff, image_buf_size + TAG_LEN);
	file_tag = (LINUX_FILE_TAG *)image_buf;
	real_image_size = make_qca_flash_image(file_num, image_file, image_buf + TAG_LEN, image_buf_size, file_tag);


	fprintf(stderr, "\n================== DATAMODEL PARAM ==================\n");
	if (getVersion(reduced_xml_name) < 0)
	{
		free(image_buf);
		fprintf(stderr, "get version error: %s!\n", reduced_xml_name);
		return EXIT_FAILURE;
	}

	replaceBlank(l_dev_info.model_name);

	fprintf(stderr, "ModelName   is %s\n", l_dev_info.model_name);
	fprintf(stderr, "BuildSpec   is %s\n", l_dev_info.build_spec);
	fprintf(stderr, "product_id  is 0x%08x\n", l_dev_info.product_id);
	fprintf(stderr, "product_ver is 0x%08x\n", l_dev_info.product_ver);
	fprintf(stderr, "swRevision  is 0x%08x\n", l_dev_info.sw_revision);
	fprintf(stderr, "platformVer is 0x%08x\n", l_dev_info.platform_ver);
	fprintf(stderr, "addHwVer    is 0x%08x\n", l_dev_info.add_hver);
	fprintf(stderr, "specilaVer  is 0x%08x\n", l_dev_info.special_ver);
	fprintf(stderr, "devModelVer is v%ld\n", strtoul(l_dev_info.dev_ver, 0, 0));
	fprintf(stderr, "buildDate   is %s\n", l_dev_info.build_date);
	fprintf(stderr, "buildTime   is %d\n", l_dev_info.build_time);
	fprintf(stderr, "sp name     is %s\n", customized_name);
	fprintf(stderr, "isBeta      is %d\n", l_dev_info.is_beta);
	fprintf(stderr, "isTrans     is %d\n", l_dev_info.is_trans);
	fprintf(stderr, "isDateCode  is %d\n", l_dev_info.is_datecode);
	fprintf(stderr, "=====================================================\n\n");

	if (fill_tag_buffer(file_tag,real_image_size) != EXIT_SUCCESS)
	{
		free(image_buf);
		return EXIT_FAILURE;
	}

	if(customized_name[0] != '\0')
	{
		sprintf(image_name_prefix_full, "%s/%sv%ld_%d.%d.0_%d.%d.%d",
					image_path, 
                    (prefix[0] == 0) ? l_dev_info.model_name : prefix, 
					//l_dev_info.build_spec,
					//customized_name,
					strtoul(l_dev_info.dev_ver, 0, 0),
					(l_dev_info.sw_revision >> 8) & 0xff, 
					l_dev_info.sw_revision & 0xff,
					(l_dev_info.platform_ver >> 16) & 0xff,
					(l_dev_info.platform_ver >> 8) & 0xff,
					l_dev_info.platform_ver & 0xff);
	}
	else 
	{
		sprintf(image_name_prefix_full, "%s/%sv%ld_%d.%d.0_%d.%d.%d",
					image_path, 
                    (prefix[0] == 0) ? l_dev_info.model_name : prefix, 
					//l_dev_info.build_spec,
					strtoul(l_dev_info.dev_ver, 0, 0),
					(l_dev_info.sw_revision >> 8) & 0xff, 
					l_dev_info.sw_revision & 0xff,
					(l_dev_info.platform_ver >> 16) & 0xff,
					(l_dev_info.platform_ver >> 8) & 0xff,
					l_dev_info.platform_ver & 0xff);
	}
	
	sprintf(image_name_prefix, "%s/%s",
					image_path, (prefix[0] == 0) ? l_dev_info.model_name : prefix);
	strcpy(image_name_prefix,image_name_prefix_full);
	if (l_dev_info.is_datecode==1)
	{
		sprintf(flash_image_name_suffix, "Datecode%s%02u%02u", l_dev_info.build_date, l_dev_info.build_time/3600, (l_dev_info.build_time%3600)/60);
	}
	else
	{
		sprintf(flash_image_name_suffix, "%s", l_dev_info.is_trans? "_trans": (l_dev_info.is_beta? "_beta":""));
	}

	strcpy(update_image_name_suffix, flash_image_name_suffix);
	
	if (0 != suffix[0])
	{
		sprintf(flash_image_name_suffix, "%s_%s", flash_image_name_suffix, suffix);
	}

	if(oob_flag)
	{
		fprintf(stderr, "generate partition table file...\n");
		sprintf(out_filename, "%s_FLASH%s.bin.partable", image_name_prefix, flash_image_name_suffix);
		make_partition_table_file(file_num, image_file, out_filename);
	}

	sprintf(out_filename, "%s_FLASH%s.bin", image_name_prefix, flash_image_name_suffix);

	/* create image */
	if (!(out = fopen(out_filename, "w"))) {
		fprintf(stderr, "can not open \"%s\" for writing\n", out_filename);
		usage();
	}

	if (!fwrite(image_buf + BOOTLOADER_TAG_LEN, real_image_size, 1, out) || fflush(out)) {
		fclose(out);
		free(image_buf);
		fprintf(stderr, "fwrite failed\n");
		return EXIT_FAILURE;
	}
	fclose(out);
	/* end create */
	
	if(oob_flag)
	{
		fprintf(stderr, "generate flash file with oob, please wait...\n");
		sprintf(out_filename2, "%s_FLASH%s.bin.oob", image_name_prefix, flash_image_name_suffix);
		fprintf(stderr, "%s out_filename2 ...\n", out_filename2);
		insert_ecc_to_image(out_filename, out_filename2);
	}
	/* only provide update firmware with bootloader 
	 * yangxv, 2013.05.07
	 */
	 
	/* create update firmware with bootloader */
	sprintf(out_filename, "%s_UP_BOOT%s.bin", image_name_prefix, update_image_name_suffix);
	if (!(out = fopen(out_filename, "w"))) {
		fprintf(stderr, "can not open \"%s\" for writing\n", out_filename);
		usage();
	}
#if 0
	if (!fwrite(image_buf, real_image_size+ BOOTLOADER_TAG_LEN, 1, out) || fflush(out)) {
			fprintf(stderr, "fwrite 1111\n");
			free(image_buf);
			fprintf(stderr, "fwrite failed\n");
			return EXIT_FAILURE;
	}
#endif
	/* create update firmware with uboot + rootfs */
	int ubootAddress = file_tag->bootAddress;
	int rootfsAddress = file_tag->rootfsAddress;
	if(1 == uboot_flag)
	{
		file_tag->bootAddress = 0;
		file_tag->rootfsAddress = file_tag->bootLen;
	}
	else
	{
		file_tag->bootLen = 0;
		file_tag->rootfsAddress = 0;
	}
	
	char *prehook_buf = NULL;

	if (filename[PREHOOK][0] != 0)
	{
		struct stat file_stat;
		FILE *in = NULL;
		unsigned int file_len = 0;
		size_t n = 0;

		if (stat(filename[PREHOOK], &file_stat) != 0 || !S_ISREG(file_stat.st_mode))
		{
			fprintf(stderr, "Prehook file \"%s\" not found or not correct\n", filename[PREHOOK]);
			usage();
		}
		file_len = file_stat.st_size;

		if (!(in = fopen(filename[PREHOOK], "r"))) {
			fprintf(stderr, "can not open \"%s\" for reading\n", filename[PREHOOK]);
			usage();
		}

		prehook_buf = malloc(file_len);
		if (!prehook_buf)
		{
			fprintf(stderr, "malloc for \"%s\" failed\n", filename[PREHOOK]);
		}
		printf("prehook is 0x%x\n", file_tag->rootfsLen);
		file_tag->prehookFsAddress = file_tag->rootfsLen;
		file_tag->prehookFsLen = STORE32_LE(file_len);

		n = 0;
		while(!feof(in) && file_len != n)
		{
			n += fread(prehook_buf + n, 1, file_len - n, in);
		}
		fclose(in);
	}

	if (!fwrite(image_buf, BOOTLOADER_TAG_LEN, 1, out) || fflush(out)) {
		fclose(out);
		free(image_buf);
		free(prehook_buf);
		fprintf(stderr, "fwrite failed\n");
		return EXIT_FAILURE;
	}
	if ((file_tag->bootLen != 0 && !fwrite(image_buf + BOOTLOADER_TAG_LEN + ubootAddress, file_tag->bootLen, 1, out)) || fflush(out)) {
		fclose(out);
		free(image_buf);
		free(prehook_buf);
		fprintf(stderr, "fwrite uboot failed\n");
		return EXIT_FAILURE;
	}
	if (!fwrite(image_buf + BOOTLOADER_TAG_LEN + rootfsAddress, file_tag->rootfsLen, 1, out) || fflush(out)) {
		fclose(out);
		free(image_buf);
		free(prehook_buf);
		fprintf(stderr, "fwrite rootfs failed\n");
		return EXIT_FAILURE;
	}
	if (prehook_buf)
	{
		if (!fwrite(prehook_buf, file_tag->prehookFsLen, 1, out) || fflush(out)) {
			fclose(out);
			free(prehook_buf);
			free(image_buf);
			fprintf(stderr, "fwrite failed\n");
			return EXIT_FAILURE;
		}
		free(prehook_buf);
	}

	fclose(out);
	free(image_buf);
	return EXIT_SUCCESS;
}
