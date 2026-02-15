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
	char model_name[BUFFER_LEN64];
	char build_date[BUFFER_LEN64];
	char dev_ver[BUFFER_LEN64];
	unsigned char hw_id[CLOUD_ID_BYTE_LEN];
	unsigned char fw_id[CLOUD_ID_BYTE_LEN];
	unsigned char oem_id[CLOUD_ID_BYTE_LEN];
	unsigned char is_beta;
	unsigned char is_trans;
	char build_spec[BUFFER_LEN64];
}DEV_INFO;

static LINUX_FLASH_STRUCT l_flash_struct;
static DEV_INFO l_dev_info;

static int l_boot_size = 0;
static int l_target_endian = -1; /* 0-big, 1-little */

#define STORE32_LE(X)		(l_target_endian ? (X) : bswap_32(X))



/**********************************************************************/

void usage(void) __attribute__ (( __noreturn__ ));

void replaceBlank(char *str);
int getStrAttrVal(char *buf, char *attr, char *value, int maxLen);
int idstrToByte(const char *pIdstr, unsigned char *pByte);
int getVersion(const char *reduced_xml_name);
int fill_buffer(char *buffer, char bootname[MAX_FILENAME_LEN], char *vmlinux);


void usage(void)
{
		fprintf(stderr, 
	"Usage: mkimage [OPTIONS]\n\n"
	"  -l, --bootsize=SIZE		MAX Bootloader size\n"
	"  -e, --targetEndian=endian Target CPU endian\n"
	"  -b, --boot=FILE		Boot file\n"
	"  -i, --image-path=PATH	image path\n"
	"  -p, --xmlName=FILE		reduced_data_model file\n"
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
		/* Get beta info */		
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


int fill_buffer(char *buffer, char bootname[MAX_FILENAME_LEN], char *vmlinux)
{
	FILE *in;
	size_t n;
	size_t boot_len;
	uint32_t cur_len;
	char *buf = buffer;
	unsigned int maxlen = l_boot_size;
	struct _LINUX_FILE_TAG *kernel_tag;
	unsigned int image_len;
	int vm_fd;
	Elf32_Ehdr ehdr; 
	Elf32_Shdr shdr;
	
	fprintf(stderr, "================== BUFFER STRUCTURE =================\n");
	
	cur_len = 0;
	
	if (!(in= fopen(bootname, "r"))) {
		fprintf(stderr, "can not open \"%s\" for reading\n", bootname);
		usage();
	}
	fprintf(stderr, "bootloader is 0x%x\n", cur_len);
	n = fread(buf + cur_len, 1, maxlen - cur_len, in);
	if (!feof(in)) {
		fprintf(stderr, "fread failure or file \"%s\" too large\n", bootname);
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
	cur_len = l_flash_struct.kernelOffset;
	kernel_tag = (struct _LINUX_FILE_TAG *) (buf + cur_len);
	
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

	kernel_tag->tagVersion = STORE32_LE(TAG_VERSION);
	memcpy(kernel_tag->magicNum, magicNum, MAGIC_NUM_LEN);

	kernel_tag->productId = STORE32_LE(l_dev_info.product_id);
	kernel_tag->productVer = STORE32_LE(l_dev_info.product_ver);

	kernel_tag->swRevision = STORE32_LE(l_dev_info.sw_revision);
	kernel_tag->platformVer = STORE32_LE(l_dev_info.platform_ver);
	kernel_tag->addHver = STORE32_LE(l_dev_info.add_hver);

	kernel_tag->specialVer = STORE32_LE(l_dev_info.special_ver);

	memcpy(kernel_tag->hardwareId, l_dev_info.hw_id, CLOUD_ID_BYTE_LEN);
	memcpy(kernel_tag->firmwareId, l_dev_info.fw_id, CLOUD_ID_BYTE_LEN);
	memcpy(kernel_tag->oemId, l_dev_info.oem_id, CLOUD_ID_BYTE_LEN);

	memset(kernel_tag->imageValidToken, 0, TOKEN_LEN);
	
	kernel_tag->kernelTextAddr = (shdr.sh_addr);
	kernel_tag->kernelEntryPoint = (ehdr.e_entry);

	image_len = l_flash_struct.appSize;
	kernel_tag->totalImageLen = STORE32_LE(image_len);

	kernel_tag->bootAddress = STORE32_LE(0x0);
	kernel_tag->bootLen = STORE32_LE(0x0);

	kernel_tag->kernelAddress = STORE32_LE(0x0);
	kernel_tag->kernelLen = STORE32_LE(0x0);

	kernel_tag->rootfsAddress = STORE32_LE(0x0);
	kernel_tag->rootfsLen = STORE32_LE(0x0);


	memset(kernel_tag->sig, 0, SIG_LEN);
	memset(kernel_tag->resSig, 0, SIG_LEN);
	
		
	fprintf(stderr, "=====================================================\n\n");
		
	return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
	FILE *out = stdout;
	char *buf;
	char bootname[MAX_FILENAME_LEN] = {0};
	
	char vmlinux_name[MAX_FILENAME_LEN] = {0};
	char reduced_xml_name[MAX_FILENAME_LEN] = {0};
	char image_path[MAX_FILENAME_LEN] = {0};

	int c = 0;


	char out_filename[MAX_FILENAME_LEN] = {0};
	
	fprintf(stderr, "YangXv <yangxu@tp-link.com.cn> modified from trx for make image\n");

	static const struct option arg_options[] = {
		{"bootsize",	required_argument,	0, 'l'},
		{"targetEndian",required_argument,	0, 'e'},
		{"boot", 		required_argument,	0, 'b'},
		{"output",		required_argument,	0, 'o'},
		{"xmlName",		required_argument,	0, 'p'},
		{"imagep",		required_argument,	0, 'i'},
		{"vmlinux",		required_argument,	0, 'v'},
		{"help",		no_argument,		0, 'h'},
		{0, 0, 0, 0}
	};	
	
	/* get options */
	while (1) {
		int option_index = 0;
		c = getopt_long(argc, argv, "l:e:b:o:p:i:v:h", arg_options, &option_index);
		if (c == -1) break;
		
		switch (c) {
		case 'l':
			sscanf(optarg, "%X", &l_boot_size);
			break;				
		case 'e':
			sscanf(optarg, "%d", &l_target_endian);
			break;
		case 'b':
			strncpy(bootname, optarg, 
				strlen(optarg) > MAX_FILENAME_LEN?MAX_FILENAME_LEN:strlen(optarg));
			break;
		case 'v':
			strncpy(vmlinux_name, optarg, 
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
		case 'h':
			usage();
			break;
		default:
			break;
		}
	}
	if(bootname == 0 || image_path[0] == 0  || vmlinux_name[0] == 0)
	{
		fprintf(stderr, "None file name\n");
		return EXIT_FAILURE;
	}

	if (-1 == l_target_endian)
	{
		fprintf(stderr, "Must set cpu endian\n");
		return EXIT_FAILURE;		
	}

	if (!(buf = malloc(l_boot_size + BOOTLOADER_TAG_LEN))) 
	{
		fprintf(stderr, "malloc failed\n");
		return EXIT_FAILURE;
	}
	memset(buf, 0xFF, l_boot_size + BOOTLOADER_TAG_LEN);

	if (l_boot_size == 0)
	{
		fprintf(stderr, "None MAX Bootloader size\n");
		return EXIT_FAILURE;
	}

	l_flash_struct.bootOffset = 0;
	l_flash_struct.kernelOffset = l_boot_size;
	fprintf(stderr, "==================== FLASH PARTS ====================\n");
	fprintf(stderr, "bootOffset	is %x\n", l_flash_struct.bootOffset);
	fprintf(stderr, "kernelOffset	is %x\n", l_flash_struct.kernelOffset);
	fprintf(stderr, "=====================================================\n\n");
	/* end added */

	fprintf(stderr, "================== DATAMODEL PARAM ==================\n");
	if (getVersion(reduced_xml_name) < 0)
	{
		fprintf(stderr, "get version error!\n");
		return EXIT_FAILURE;
	}

	replaceBlank(l_dev_info.model_name);

	fprintf(stderr, "ModelName   is %s\n", l_dev_info.model_name);
	fprintf(stderr, "BuildSpec    is %s\n", l_dev_info.build_spec);
	fprintf(stderr, "product_id  is 0x%08x\n", l_dev_info.product_id);
	fprintf(stderr, "product_ver is 0x%08x\n", l_dev_info.product_ver);
	fprintf(stderr, "swRevision  is 0x%08x\n", l_dev_info.sw_revision);
	fprintf(stderr, "platformVer is 0x%08x\n", l_dev_info.platform_ver);
	fprintf(stderr, "addHwVer    is 0x%08x\n", l_dev_info.add_hver);
	fprintf(stderr, "specilaVer  is 0x%08x\n", l_dev_info.special_ver);
	fprintf(stderr, "devModelVer is v%ld\n", strtoul(l_dev_info.dev_ver, 0, 0));
	fprintf(stderr, "buildDate   is %s\n", l_dev_info.build_date);
	fprintf(stderr, "isBeta      is %d\n", l_dev_info.is_beta);
	fprintf(stderr, "isTrans     is %d\n\n", l_dev_info.is_trans);
	fprintf(stderr, "=====================================================\n\n");
	
	if (fill_buffer(buf, bootname, vmlinux_name) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	sprintf(out_filename, "%s/boot.bin", image_path);
	if (!(out = fopen(out_filename, "w"))) {
		fprintf(stderr, "can not open \"%s\" for writing\n", out_filename);
		usage();
	}

	if (!fwrite(buf, l_boot_size + BOOTLOADER_TAG_LEN, 1, out) || fflush(out)) {
		fprintf(stderr, "fwrite failed\n");
		return EXIT_FAILURE;
	}

	fclose(out);

	free(buf);
	
	return EXIT_SUCCESS;
}

