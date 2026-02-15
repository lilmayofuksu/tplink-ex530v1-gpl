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
#include <ctype.h>
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


#include "mkimage.h"
#include "md5_interface.h"


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
	char dev_ver[BUFFER_LEN64];
	unsigned char hw_id[CLOUD_ID_BYTE_LEN];
	unsigned char fw_id[CLOUD_ID_BYTE_LEN];
	unsigned char oem_id[CLOUD_ID_BYTE_LEN];
}DEV_INFO;

static DEV_INFO l_dev_info;

static int l_image_size = 0;
static int l_mtd_type = -1;
static int l_boot_size = 0;
static int l_kernel_size = 0;
static int l_rootfs_size = 0; /* calc */
static int l_misc_size = 0;
static int l_target_endian = -1; /* 0-big, 1-little */

#define STORE32_LE(X)		(l_target_endian ? (X) : bswap_32(X))



/**********************************************************************/

void usage(void) __attribute__ (( __noreturn__ ));

void replaceBlank(char *str);
int getStrAttrVal(char *buf, char *attr, char *value, int maxLen);
int idstrToByte(const char *pIdstr, unsigned char *pByte);
int getVersion(const char *reduced_xml_name);
int fill_buffer(char *buffer, unsigned int imglen, char filename[][MAX_FILENAME_LEN], char *vmlinux);


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
	"  -c, --config=FILE		Manufacture config file\n"
	"  -a, --otherarg=FILE		Other config file that may include hwver, default MAC/PIN\n"
	"  -o, --output=FILE		Output Filename prefix\n"
	"  -i, --image-path=PATH	image path\n"
	"  -p, --xmlName=FILE		reduced_data_model file\n"
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
		
		str++;
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
	if (q == NULL)
	{
		fprintf(stderr, "%s(): \"%s\" cannot find \"d=\"\n", __FUNCTION__, attr);
		return -1;
	}
	p = strchr(q, '>');
	if (p == NULL)
	{
		fprintf(stderr, "%s(): \"%s\" cannot find \">\"\n", __FUNCTION__, attr);
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

	#define MAX_LEN	(80 * 1024)
	char buf[MAX_LEN] = {0};

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
		q = strstr(p, "d=");

		if (q != 0)
		{
			q += 2;
			strncpy(tmp, q, 10);			
			l_dev_info.special_ver = strtoul(tmp, NULL, 16);  
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

		printf("hwid is %s\n", tmpId);

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

		break;
	}

	close(fd);

	return 0;
}

#define FLASH_WRITE_SIZE (64 * 1024)	
int doFileCopy(FILE *in, FILE *out)
{
	unsigned char *buf = NULL;
	size_t write_size = 0;
	int total_size = 0;
	
	if (!(buf = malloc(FLASH_WRITE_SIZE))) 
	{
		fprintf(stderr, "Malloc failed\n");
		return -1;
	}
	
	while(!feof(in))
	{
		write_size = fread(buf, 1, FLASH_WRITE_SIZE, in);
		
		if (fwrite(buf, 1, write_size, out) != write_size)
		{
			fprintf(stderr, "do file copy failed\n");
			free(buf);
			return -1;
		}
		
		total_size += write_size;
	}
	
	free(buf);
	return total_size;
}

#define PAD_SIZE (64 * 1024)	
int doPadLen(FILE* fd, unsigned int len)
{
	unsigned char buf[PAD_SIZE];
	
	memset(buf, 0xff, PAD_SIZE);
	
	while(len > PAD_SIZE)
	{
		if (fwrite(buf, 1, PAD_SIZE, fd) != PAD_SIZE)
		{
			fprintf(stderr, "pad write error\n");
			return -1;
		}
		
		len -= PAD_SIZE;
	}
	
	if (len)
	{
		if (fwrite(buf, 1, len, fd) != len)
		{
			fprintf(stderr, "pad write error\n");
			return -1;
		}
	}
	
	return 0;
}

int fill_buffer(char *buf, unsigned int imglen, char filename[][MAX_FILENAME_LEN], char *vmlinux)
{
	FILE *in;
	size_t n;
	uint32_t cur_len;
	struct _LINUX_FILE_TAG *file_tag;
	unsigned int image_len;
	int vm_fd;
	Elf32_Ehdr ehdr; 
	Elf32_Shdr shdr;
	
	printf("================== BUFFER STRUCTURE =================\n");
	file_tag = (struct _LINUX_FILE_TAG *)buf;
	
	memset(file_tag, 0, BOOTLOADER_TAG_LEN);
	cur_len = BOOTLOADER_TAG_LEN;
	
	/* 1) bootloader */
	if (filename[BOOT][0] != 0)
	{
		file_tag->bootAddress = STORE32_LE(0x0);
		if (!(in = fopen(filename[BOOT], "r"))) {
			fprintf(stderr, "can not open \"%s\" for reading\n", filename[BOOT]);
			usage();
		}
		
		printf("bootloader is 0x%x\n", cur_len);
		n = 0;
		while(!feof(in))
		{
			n += fread(buf + cur_len + n, 1, imglen, in);
		}
		fclose(in);
		
		file_tag->bootLen = STORE32_LE(n);
		cur_len += n;
	}
	/* end read bootloader */

	/* 2) linux kernel */
	if (filename[KERNEL][0] != 0)
	{
		file_tag->kernelAddress = STORE32_LE(cur_len - BOOTLOADER_TAG_LEN);
		
		if (!(in = fopen(filename[KERNEL], "r"))) {
			fprintf(stderr, "can not open \"%s\" for reading\n", filename[KERNEL]);
			usage();
		}
		
		printf("kernel is 0x%x\n", cur_len);
		n = 0;
		while(!feof(in))
		{
			n += fread(buf + cur_len + n, 1, imglen, in);
		}
		fclose(in);
		
		file_tag->kernelLen = STORE32_LE(n);
		cur_len += n;
	}
	/* end read linux kernel */

	/* 3) rootfs */
	if (filename[FS][0] != 0)
	{
		file_tag->rootfsAddress = STORE32_LE(cur_len - BOOTLOADER_TAG_LEN);
		
		if (!(in = fopen(filename[FS], "r"))) {
			fprintf(stderr, "can not open \"%s\" for reading\n", filename[FS]);
			usage();
		}

		printf("rootfs is 0x%x\n", cur_len);
		n = 0;
		while(!feof(in))
		{
			n += fread(buf + cur_len + n, 1, imglen, in);
		}
		fclose(in);
		
		file_tag->rootfsLen = STORE32_LE(n);
		cur_len += n;
	}
	/* end read rootfs */

	/* 3) prehook file */
	if (filename[PREHOOK][0] != 0)
	{
		file_tag->prehookFsAddress = STORE32_LE(cur_len - BOOTLOADER_TAG_LEN);
		
		if (!(in = fopen(filename[PREHOOK], "r"))) {
			fprintf(stderr, "can not open \"%s\" for reading\n", filename[PREHOOK]);
			usage();
		}

		printf("prehook is 0x%x\n", cur_len);
		n = 0;
		while(!feof(in))
		{
			n += fread(buf + cur_len + n, 1, imglen, in);
		}
		fclose(in);
		
		file_tag->prehookFsLen = STORE32_LE(n);
		cur_len += n;
	}
	/* end read rootfs */

	if (cur_len != imglen)
	{
		fprintf(stderr, "Image size error readsize %d, statsize %d\n", cur_len, imglen);
		return EXIT_FAILURE;
	}

	/* 4) kernel tag */
	if ((vm_fd = open(vmlinux, O_RDONLY)) < 0) {
		fprintf(stderr, "can not open \"%s\" for reading\n", vmlinux);
		usage();
	}
	
	read(vm_fd, &ehdr, sizeof(ehdr)); 
	read(vm_fd, &shdr, sizeof(shdr));
	
	close(vm_fd);				
	 
	printf("Entry Point: %#X\n", STORE32_LE(ehdr.e_entry));
	printf("Text Addr: %#X\n", STORE32_LE(shdr.sh_addr));

	file_tag->tagVersion = STORE32_LE(TAG_VERSION);
	memcpy(file_tag->magicNum, magicNum, MAGIC_NUM_LEN);

	file_tag->productId = STORE32_LE(l_dev_info.product_id);
	file_tag->productVer = STORE32_LE(l_dev_info.product_ver);

	file_tag->swRevision = STORE32_LE(l_dev_info.sw_revision);
	file_tag->platformVer = STORE32_LE(l_dev_info.platform_ver);
	file_tag->addHver = STORE32_LE(l_dev_info.add_hver);
	file_tag->dmVersion = STORE32_LE(l_dev_info.dm_ver);

	file_tag->specialVer = STORE32_LE(l_dev_info.special_ver);

	memcpy(file_tag->hardwareId, l_dev_info.hw_id, CLOUD_ID_BYTE_LEN);
	memcpy(file_tag->firmwareId, l_dev_info.fw_id, CLOUD_ID_BYTE_LEN);
	memcpy(file_tag->oemId, l_dev_info.oem_id, CLOUD_ID_BYTE_LEN);

	memset(file_tag->imageValidToken, 0, TOKEN_LEN);
	
	file_tag->kernelTextAddr = (shdr.sh_addr);
	file_tag->kernelEntryPoint = (ehdr.e_entry);

	image_len = cur_len - BOOTLOADER_TAG_LEN;
	file_tag->totalImageLen = STORE32_LE(image_len);

	file_tag->binCrc32 = STORE32_LE(calc_crc32((const char *)buf + BOOTLOADER_TAG_LEN, image_len));

	#if 0
	memcpy(boot_tag->imageValidToken, mk5Key_bootloader, 16);
	md5_make_digest(md5, (unsigned char*)boot_tag, up_boot_image_len);
	memcpy(boot_tag->imageValidToken, md5, MD5_HASH_SIZE);
	#endif
	/* end fill boot tag */

	printf("=====================================================\n\n");

	return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
	FILE *in;
	FILE *out = stdout;
	char *buf = NULL;
	char filename[FILE_NUM][MAX_FILENAME_LEN] = {{0}, {0}, {0}};
	
	char vmlinux_name[MAX_FILENAME_LEN] = {0};
	char reduced_xml_name[MAX_FILENAME_LEN] = {0};
	char image_path[MAX_FILENAME_LEN] = {0};

	int c = 0;

	char prefix[MAX_FILENAME_LEN] = {0};
	char *suffix = NULL;
	char out_filename[MAX_FILENAME_LEN] = {0};
	
	int imglen = 0;
	struct stat file_stat;

	fprintf(stderr, "YangXv <yangxu@tp-link.com.cn> modified from trx for make image\n");

	static const struct option arg_options[] = {
		{"imagesize", required_argument,	0, 's'},
		{"mtdtype", required_argument, 0, 't'},
		{"bootsize", required_argument,	0, 'l'},
		{"maxkernelsize",	required_argument,	0, 'm'},
		{"miscsize",	required_argument,	0, 'n'},
		{"targetEndian", required_argument,	0, 'e'},
		{"boot", required_argument,	0, 'b'},
		{"kernel", required_argument,	0, 'k'},
		{"fs", required_argument,	0, 'f'},
		{"config", required_argument, 0, 'c'},
		{"otherarg", required_argument, 0, 'a'},
		{"output", required_argument,	0, 'o'},
		{"xmlName", required_argument,	0, 'p'},
		{"imagep", required_argument,	0, 'i'},
		{"vmlinux", required_argument,	0, 'v'},
		{"help", no_argument,		0, 'h'},
		{0, 0, 0, 0}
	};	
	
	/* get options */
	while (1) {
		int option_index = 0;
		c = getopt_long(argc, argv, "s:t:l:m:n:e:b:k:f:c:a:q:o:p:i:v:h", arg_options, &option_index);
		if (c == -1) break;
		
		switch (c) {
		case 's':
			sscanf(optarg, "%X", &l_image_size);
			break;
		case 't':
			sscanf(optarg, "%X", &l_mtd_type);
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
		case 'a':
			strncpy(filename[OTHER], optarg, 
				strlen(optarg) > MAX_FILENAME_LEN?MAX_FILENAME_LEN:strlen(optarg));
			break;
		case 'q':
			strncpy(filename[PREHOOK], optarg, 
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
		case 'v':
			strncpy(vmlinux_name, optarg, 
				strlen(optarg) > MAX_FILENAME_LEN?MAX_FILENAME_LEN:strlen(optarg));
			break;
		case 'h':
			usage();
			break;
		default:
			break;
		}
	}
	
	if (filename[BOOT][0] == 0 && filename[KERNEL][0] != 0 && filename[FS][0] == 0)
	{
		suffix = "tag_kernel";
	}
	else if (filename[BOOT][0] == 0 && filename[KERNEL][0] != 0 && filename[FS][0] != 0)
	{
		suffix = "UP";
	}
	else if (filename[BOOT][0] != 0 && filename[KERNEL][0] != 0 && filename[FS][0] != 0)
	{
		suffix = "UP_BOOT";
	}
	else
	{
		fprintf(stderr, "Filename group not supported. Current available:\n  kernel\n  kernel+rootfs\n  boot+kernel+rootfs\n\n");
		return EXIT_FAILURE;
	}
	
	if (image_path[0] == 0  || vmlinux_name[0] == 0)
	{
		fprintf(stderr, "None file name\n");
		return EXIT_FAILURE;
	}

	if (-1 == l_target_endian)
	{
		fprintf(stderr, "Must set cpu endian\n");
		return EXIT_FAILURE;
	}
	
	imglen = sizeof(LINUX_FILE_TAG);
	
	if (filename[BOOT][0] != 0)
	{
		if (stat(filename[BOOT], &file_stat) != 0 || !S_ISREG(file_stat.st_mode))
		{
			fprintf(stderr, "Boot file \"%s\" not found or not correct\n", filename[BOOT]);
			usage();
		}
		if (l_boot_size != 0)
		{
			if (file_stat.st_size > l_boot_size)
			{
				fprintf(stderr, "boot size is %lx, bigger than max %x\n", file_stat.st_size, l_boot_size);
				return EXIT_FAILURE;
			}
		}
		imglen += file_stat.st_size;
	}
	
	if (filename[KERNEL][0] != 0)
	{
		if (stat(filename[KERNEL], &file_stat) != 0 || !S_ISREG(file_stat.st_mode))
		{
			fprintf(stderr, "Kernel file \"%s\" not found or not correct\n", filename[KERNEL]);
			usage();
		}
		if (l_kernel_size != 0)
		{
			if (file_stat.st_size > l_kernel_size)
			{
				fprintf(stderr, "kernel size is %lx, bigger than max %x\n", file_stat.st_size, l_kernel_size);
				return EXIT_FAILURE;
			}
		}
		imglen += file_stat.st_size;
	}
	
	if (filename[FS][0] != 0)
	{
		if (stat(filename[FS], &file_stat) != 0 || !S_ISREG(file_stat.st_mode))
		{
			fprintf(stderr, "Rootfs file \"%s\" not found or not correct\n", filename[FS]);
			usage();
		}
		if (l_image_size != 0 && l_boot_size != 0 && l_kernel_size != 0 && l_misc_size != 0 && l_mtd_type != -1)
		{
			if (l_mtd_type == 2) /* MTD_TYPE_FS */
			{
				l_rootfs_size = l_image_size - l_boot_size - l_kernel_size - 2 * l_misc_size;
			}
			else
			{
				l_rootfs_size = l_image_size - l_boot_size - l_kernel_size - l_misc_size;
			}
			if (file_stat.st_size > l_rootfs_size)
			{
				fprintf(stderr, "rootfs size is %lx, bigger than max %x\n", file_stat.st_size, l_rootfs_size);
				return EXIT_FAILURE;
			}
		}
		imglen += file_stat.st_size;
	}
	
	if (filename[CONFIG][0] != 0)
	{
		if (stat(filename[CONFIG], &file_stat) != 0 || !S_ISREG(file_stat.st_mode))
		{
			fprintf(stderr, "Config file \"%s\" not found or not correct\n", filename[CONFIG]);
			usage();
		}
		
		/* only correct for MTD_TYPE_FS. but RAW mf_config copy from MTD device, not generated, no need to check */
		if (l_misc_size != 0)
		{
			if (file_stat.st_size > l_misc_size)
			{
				fprintf(stderr, "config size is %lx, bigger than max %x\n", file_stat.st_size, l_misc_size);
				return EXIT_FAILURE;
			}
		}
	}
	
	if (filename[OTHER][0] != 0)
	{
		if (stat(filename[OTHER], &file_stat) != 0 || !S_ISREG(file_stat.st_mode))
		{
			fprintf(stderr, "Other file \"%s\" not found or not correct\n", filename[OTHER]);
			usage();
		}
		
		/* only correct for MTD_TYPE_FS. but RAW mf_config copy from MTD device, not generated, no need to check */
		if (l_misc_size != 0)
		{
			if (file_stat.st_size > l_misc_size)
			{
				fprintf(stderr, "other size is %lx, bigger than max %x\n", file_stat.st_size, l_misc_size);
				return EXIT_FAILURE;
			}
		}
	}

	if (filename[PREHOOK][0] != 0)
	{
		if (stat(filename[PREHOOK], &file_stat) != 0 || !S_ISREG(file_stat.st_mode))
		{
			fprintf(stderr, "Rootfs file \"%s\" not found or not correct\n", filename[PREHOOK]);
			usage();
		}
		imglen += file_stat.st_size;
	}
	
	if (!(buf = malloc(2 * imglen))) 
	{
		fprintf(stderr, "Malloc failed\n");
		return EXIT_FAILURE;
	}
	memset(buf, 0xFF, imglen);

	printf("================== DATAMODEL PARAM ==================\n");
	if (getVersion(reduced_xml_name) < 0)
	{
		fprintf(stderr, "get version error!\n");
		return EXIT_FAILURE;
	}

	replaceBlank(l_dev_info.model_name);
	printf("IMAGE_TAG size %x\n", sizeof(LINUX_FILE_TAG));
	printf("ModelName   is %s\n", l_dev_info.model_name);
	printf("product_id  is 0x%08x\n", l_dev_info.product_id);
	printf("product_ver is 0x%08x\n", l_dev_info.product_ver);
	printf("swRevision  is 0x%08x\n", l_dev_info.sw_revision);
	printf("platformVer is 0x%08x\n", l_dev_info.platform_ver);
	printf("addHwVer    is 0x%08x\n", l_dev_info.add_hver);
	printf("DMVer       is 0x%08x\n", l_dev_info.dm_ver);
	printf("specilaVer  is 0x%08x\n\n", l_dev_info.special_ver);
	printf("devModelVer is v%d\n", (unsigned int)strtoul(l_dev_info.dev_ver, 0, 0));
	printf("buildDate   is %s\n", l_dev_info.build_date);
	printf("=====================================================\n\n");
	
	if (fill_buffer(buf, imglen, filename, vmlinux_name) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	
	sprintf(out_filename, "%s/%s_%s.bin",
					image_path, (prefix[0] == 0) ? l_dev_info.model_name : prefix, suffix);

	/* create image */
	if (!(out = fopen(out_filename, "w"))) {
		fprintf(stderr, "can not open \"%s\" for writing\n", out_filename);
		usage();
	}

	if (!fwrite(buf, imglen, 1, out) || fflush(out)) {
		fprintf(stderr, "fwrite failed\n");
		return EXIT_FAILURE;
	}
	fclose(out);
					
	if (prefix[0] == 0)
	{
		sprintf(out_filename, "%s/%sv%d_%d.%d.0_%d.%d.%d_%s(%s).bin",
						image_path, l_dev_info.model_name,
						(unsigned int)strtoul(l_dev_info.dev_ver, 0, 0),
						(l_dev_info.sw_revision >> 8) & 0xff, 
						l_dev_info.sw_revision & 0xff,
						(l_dev_info.platform_ver >> 16) & 0xff,
						(l_dev_info.platform_ver >> 8) & 0xff,
						l_dev_info.platform_ver & 0xff,
						suffix, l_dev_info.build_date);

		if (!(out = fopen(out_filename, "w"))) {
			fprintf(stderr, "can not open \"%s\" for writing\n", out_filename);
			usage();
		}

		if (!fwrite(buf, imglen, 1, out) || fflush(out)) {
			fprintf(stderr, "fwrite failed\n");
			return EXIT_FAILURE;
		}
		fclose(out);
	}
	/* end create */
 
	free(buf);
	
	/* generate single image flash file */
	if (filename[BOOT][0] != 0 && filename[KERNEL][0] != 0 && filename[FS][0] != 0 
			&& l_image_size != 0 && l_boot_size != 0 && l_kernel_size != 0 && l_misc_size != 0
			&& l_mtd_type != -1)
	{
		int write_size;
		int remain_size;
		int total_size;
		
		if (!(buf = malloc(FLASH_WRITE_SIZE))) 
		{
			fprintf(stderr, "Malloc failed\n");
			return EXIT_FAILURE;
		}
	
		sprintf(out_filename, "%s/%s_FLASH.bin",
						image_path, (prefix[0] == 0) ? l_dev_info.model_name : prefix);
	
		if (!(out = fopen(out_filename, "w"))) {
			fprintf(stderr, "can not open \"%s\" for writing\n", out_filename);
			usage();
		}
		
		/* 1) boot for flash */
		if (!(in = fopen(filename[BOOT], "r"))) {
			fprintf(stderr, "can not open \"%s\" for reading\n", filename[BOOT]);
			usage();
		}
		
		if ((write_size = doFileCopy(in, out)) < 0)
		{
			fprintf(stderr, "write \"%s\" for flash image failed\n", filename[BOOT]);
			return EXIT_FAILURE;
		}
		
		if (doPadLen(out, l_boot_size - write_size) < 0)
		{
			fprintf(stderr, "pad \"%s\" for flash image failed\n", filename[BOOT]);
			return EXIT_FAILURE;
		}
		fclose(in);
		
		/* 2) write hwver, then pad misc for flash when MTD_TYPE_FS */
		if (l_mtd_type == 2)
		{
			write_size = 0;
			if (filename[OTHER][0] != 0)
			{
				if (!(in = fopen(filename[OTHER], "r"))) {
					fprintf(stderr, "can not open \"%s\" for reading\n", filename[OTHER]);
					usage();
				}
				
				if ((write_size = doFileCopy(in, out)) < 0)
				{
					fprintf(stderr, "write \"%s\" for flash image failed\n", filename[OTHER]);
					return EXIT_FAILURE;
				}
			}
			else
			{
				fprintf(stderr, "\e[0;31mFlash not include additional HW-ver!\e[0m\n");
			}
			
			if (doPadLen(out, l_misc_size - write_size) < 0)
			{
				fprintf(stderr, "pad misc for flash image failed\n");
				return EXIT_FAILURE;
			}
		}
		
		/* 3) write mf_config, then pad misc for flash when MTD_TYPE_RAW2/MTD_TYPE_FS */
		if (l_mtd_type == 1 || l_mtd_type == 2)
		{
			write_size = 0;
			if (filename[CONFIG][0] != 0)
			{
				if (!(in = fopen(filename[CONFIG], "r"))) {
					fprintf(stderr, "can not open \"%s\" for reading\n", filename[CONFIG]);
					usage();
				}
				
				if ((write_size = doFileCopy(in, out)) < 0)
				{
					fprintf(stderr, "write \"%s\" for flash image failed\n", filename[CONFIG]);
					return EXIT_FAILURE;
				}
			}
			else
			{
				fprintf(stderr, "\e[0;31mFlash not include MFG config!\e[0m\n");
			}
			
			if (doPadLen(out, l_misc_size - write_size) < 0)
			{
				fprintf(stderr, "pad misc for flash image failed\n");
				return EXIT_FAILURE;
			}
		}
	
		/* 3) kernel for flash */
		if (!(in = fopen(filename[KERNEL], "r"))) {
			fprintf(stderr, "can not open \"%s\" for reading\n", filename[KERNEL]);
			usage();
		}
		
		if ((write_size = doFileCopy(in, out)) < 0)
		{
			fprintf(stderr, "write \"%s\" for flash image failed\n", filename[KERNEL]);
			return EXIT_FAILURE;
		}
		
		if (l_mtd_type == 2) /* MTD_TYPE_FS */
		{
			total_size = l_boot_size + 2 * l_misc_size + l_kernel_size;
			remain_size = l_kernel_size - write_size;
			printf("================== BADBLOCK ENDURANCE ===============\n");
			printf("  kernel: 0x%08X/0x%08X (%05.2f%%)\n", remain_size, total_size, (float)remain_size * 100 / total_size);
		}
		
		if (doPadLen(out, l_kernel_size - write_size) < 0)
		{
			fprintf(stderr, "pad \"%s\" for flash image failed\n", filename[KERNEL]);
			return EXIT_FAILURE;
		}
		fclose(in);

		/* 4) rootfs for flash */
		if (!(in = fopen(filename[FS], "r"))) {
			fprintf(stderr, "can not open \"%s\" for reading\n", filename[FS]);
			usage();
		}
		
		if ((write_size = doFileCopy(in, out)) < 0)
		{
			fprintf(stderr, "write \"%s\" for flash image failed\n", filename[FS]);
			return EXIT_FAILURE;
		}
		
		if (l_mtd_type == 2) /* MTD_TYPE_FS */
		{
			remain_size = l_rootfs_size - write_size;
			printf("  rootfs: 0x%08X/0x%08X (%05.2f%%)\n", l_rootfs_size - write_size, l_image_size, (float)remain_size * 100 / l_image_size);
			printf("=====================================================\n\n");
		}
		
		if (doPadLen(out, l_rootfs_size - write_size) < 0)
		{
			fprintf(stderr, "pad \"%s\" for flash image failed\n", filename[KERNEL]);
			return EXIT_FAILURE;
		}
		fclose(in);
		
		/* 5) write mf_config, then pad misc for flash when MTD_TYPE_RAW1 */
		if (l_mtd_type == 0) /* MTD_TYPE_RAW1 */
		{
			write_size = 0;
			if (filename[CONFIG][0] != 0)
			{
				if (!(in = fopen(filename[CONFIG], "r"))) {
					fprintf(stderr, "can not open \"%s\" for reading\n", filename[CONFIG]);
					usage();
				}
				
				if ((write_size = doFileCopy(in, out)) < 0)
				{
					fprintf(stderr, "write \"%s\" for flash image failed\n", filename[CONFIG]);
					return EXIT_FAILURE;
				}
			}
			else
			{
				fprintf(stderr, "\e[0;31mFlash not include MFG config!\e[0m\n");
			}
			
			if (doPadLen(out, l_misc_size - write_size) < 0)
			{
				fprintf(stderr, "pad misc for flash image failed\n");
				return EXIT_FAILURE;
			}
		}
	}
	
	return EXIT_SUCCESS;
}
