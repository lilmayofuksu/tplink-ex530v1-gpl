#include <asm/io.h>
#include <common.h>
#include <image.h>
#include <libfdt.h>
#include <fdt.h>
#include <flashhal.h>
#include <spi/spi_nand_flash.h>
#include <ecnt/image/ecnt_image.h>
#include <ecnt/bootargs/ecnt_bootargs.h>
#include <trx.h>

#ifdef CONFIG_TP_IMAGE
#include <tp_bootlib.h>

static int macStrToEth(const char *pStr, unsigned char *pMac)
{
	int i = 0, c = 0;

	if ((NULL == pStr) || (NULL == pMac))
	{
		return -1;
	}

	for (i = 0; i < 6; i++)
	{
		if (*pStr == '-' || *pStr == ':')
		{
			pStr++;
		}

		if (*pStr >= '0' && *pStr <= '9')
		{
			c  = (unsigned char) (*pStr++ - '0');
		}
		else if (*pStr >= 'a' && *pStr <= 'f')
		{
			c  = (unsigned char) (*pStr++ - 'a') + 10;
		}
		else if (*pStr >= 'A' && *pStr <= 'F')
		{
			c  = (unsigned char) (*pStr++ - 'A') + 10;
		}
		else
		{
			return -1;
		}

		c <<= 4;

		if (*pStr >= '0' && *pStr <= '9')
		{
			c |= (unsigned char) (*pStr++ - '0');
		}
		else if (*pStr >= 'a' && *pStr <= 'f')
		{
			c |= (unsigned char) (*pStr++ - 'a') + 10;
		}
		else if (*pStr >= 'A' && *pStr <= 'F')
		{
			c |= (unsigned char) (*pStr++ - 'A') + 10;
		}
		else
		{
			return -1;
		}

		pMac[i] = (unsigned char) c;

	}

	return 0;
}

#endif /* CONFIG_TP_IMAGE */

#define DBG_FLASH_COMMAND 0

static int flash_command(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long			addr = 0, first_imgAddr = 0, second_imgAddr = 0;
	unsigned long			len = 0;
	unsigned long			buf = CONFIG_SYS_LOAD_ADDR;
	unsigned long			retlen = 0;
	unsigned long			img_size = 0;
	image_header_t			*img_hdr = (image_header_t *)buf;
    unsigned int            bootflag = 0;
    struct tclinux_imginfo	first_imginfo, second_imginfo;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (!strncmp(argv[1], "init", 5))
	{
		printf("init: %d\n", flash_init(0));
	}
	else if (!strncmp(argv[1], "erase", 6))
	{	
		addr = simple_strtoul(argv[2], NULL, 16);	
		len  = simple_strtoul(argv[3], NULL, 16);

		printf("erase: addr=0x%lx, len=0x%lx\n", addr, len);
		flash_erase(addr, len);
	}
	else if (!strncmp(argv[1], "read", 5))
	{
		addr = simple_strtoul(argv[2], NULL, 16);	
		len  = simple_strtoul(argv[3], NULL, 16);
		if (argc == 5)
		{
			buf = simple_strtoul(argv[4], NULL, 16);
		}

		printf("read: src=0x%lx, len=0x%lx, dst=0x%lx\n", addr, len, buf);
		flash_read(addr, len, &retlen, (unsigned char *) buf);
	}
	else if (!strncmp(argv[1], "write", 6))
	{
		addr = simple_strtoul(argv[2], NULL, 16);	
		len  = simple_strtoul(argv[3], NULL, 16);
		if (argc == 5)
		{
			buf = simple_strtoul(argv[4], NULL, 16);
		}

		printf("write: src=0x%lx, len=0x%lx, dst=0x%lx\n", buf, len, addr);
		flash_erase(addr, len);
		flash_write(addr, len, &retlen, (unsigned char *) buf);
	}
	else if (!strncmp(argv[1], "imgread", 6))
	{			
		len  = simple_strtoul(argv[2], NULL, 16);
		if (argc >= 5)
		{
			buf = simple_strtoul(argv[3], NULL, 16);
		}

#ifdef CONFIG_USE_IRQ
		/* close mac before decompress	*/

		gic_dic_clear_enable_all_intr();
		gic_dic_clear_pending_all_intr();
#endif
#ifdef CONFIG_ECNT_MULTIUPGRADE
		VPint(CR_MAC_MACCR)=0;
		resetSwMAC3262(); 
#endif

		first_imgAddr = ecnt_get_tclinux_flash_offset() + sizeof(struct trx_header);
		second_imgAddr = ecnt_get_tclinux_slave_flash_offset() + sizeof(struct trx_header);

		memset((void*) &first_imginfo, 0, sizeof(struct tclinux_imginfo));
		memset((void*) &second_imginfo, 0, sizeof(struct tclinux_imginfo));

#if DBG_FLASH_COMMAND
		printf("=== imgread first_imgAddr:0x%08x ===\n", first_imgAddr);
		printf("=== imgread first_imgLen:0x%08x ===\n", len);
#endif

#ifdef CONFIG_DUAL_IMAGE
        bootflag = readBootFlagFromFlash();
#else
		bootflag = 0;
#endif

		if(boot_verify(buf, bootflag))
		{
			printf("Verify image fail\n");
	        return -1;
		}

		if (bootflag==0) {
            printf("\nbootflag==%d --> booting from Main Image\n", bootflag);

			setenv("root", CONFIG_ENV_ROOT);
#ifdef CONFIG_DUAL_IMAGE
			/* Get second kernel & rootfs offset & size */
			if (get_tclinux_imginfo((const unsigned long)(second_imgAddr), &second_imginfo, NOT_BOOT_IMG, (unsigned char *)buf)) {
		        /* No second image */
				printf("Parse second image fail.\n");
			}
#endif
			/* Get first kernel & rootfs offset & size, and load kernel to DRAM */
			if (get_tclinux_imginfo((const unsigned long)(first_imgAddr), &first_imginfo, BOOT_IMG, (unsigned char *)buf)) {
				printf("Parse main image fail.\n");
		        return -1;
			}
        }
#ifdef CONFIG_DUAL_IMAGE
        else { /* bootflag==1 */
            printf("\nbootflag==%d --> booting from second image\n", bootflag);

			setenv("root", CONFIG_ENV_ROOT_SLAVE);

			/* Get first kernel & rootfs offset & size */
			if (get_tclinux_imginfo((const unsigned long)(first_imgAddr), &first_imginfo, NOT_BOOT_IMG, (unsigned char *)buf)) {
				printf("Parse main image fail.\n");
			}
			
			/* Get second kernel & rootfs offset & size, and load kernel to DRAM */
			if (get_tclinux_imginfo((const unsigned long)(second_imgAddr), &second_imginfo, BOOT_IMG, (unsigned char *)buf)) {
				printf("Parse second image fail.\n");
		        return -1;
			}
        }
#endif
		if(bootargs_init(&first_imginfo, &second_imginfo, bootflag)) {
			printf("bootargs init fail.\n");
			return -1;
		}
	}
#ifdef CONFIG_TP_IMAGE
	else if (!strncmp(argv[1], "tpimg", 5))
	{
		IMAGE_TAG *pTag;
		char macStr[18] = "00:0A:EB:13:09:69";
		unsigned char mac[6] = {0};

#ifdef CONFIG_USE_IRQ
		/* close mac before decompress	*/

		gic_dic_clear_enable_all_intr();
		gic_dic_clear_pending_all_intr();
#endif
#ifdef CONFIG_ECNT_MULTIUPGRADE
		VPint(CR_MAC_MACCR)=0;
		resetSwMAC3262();
#endif

		tp_boot_get_compressed_kernel(buf);

		pTag = (IMAGE_TAG *)buf;
		if (pTag->tagVersion != TP_TAG_VERSION)
		{
			printf("tp image format error.\n");
			return -1;
		}
		flash_read(flash_base + MTD_OFS_OTHER + OTHER_OFS_MAC, sizeof(mac), &retlen, mac);
		if (is_valid_ether_addr(mac))
		{
			snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
						mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		}
		setenv("ethaddr", macStr);
#ifdef INCLUDE_DUAL_IMAGE
		if (tp_boot_get_boot_index() == 1)
		{
			setenv("root", CONFIG_ENV_ROOT_SLAVE);
		}
		else
		{
			setenv("root", CONFIG_ENV_ROOT);
		}
#endif /* INCLUDE_DUAL_IMAGE */
		if(bootargs_init(NULL, NULL, 0)) {
			printf("bootargs init fail.\n");
			return -1;
		}
	}
	else if (!strncmp(argv[1], "macset", 6))
	{
		unsigned char mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

		if (macStrToEth(argv[2], mac) != 0 || !is_valid_ether_addr(mac))
		{
			printf("Invalid MAC addr\n");
			return -1;
		}
		flash_read(flash_base + MTD_OFS_OTHER, MTD_BLOCK_SIZE, &retlen, buf);
		memcpy(buf + OTHER_OFS_MAC, mac, sizeof(mac));
		flash_write(flash_base + MTD_OFS_OTHER, MTD_BLOCK_SIZE, &retlen, buf);
	}
	else if (!strncmp(argv[1], "bflag", 5))
	{
		int bflag = -1;
		bflag = simple_strtoul(argv[2], NULL, 10);
		if (bflag != 0 && bflag != 1)
		{
			printf("Invalid bflag\n");
			return -1;
		}
		tp_boot_set_boot_index(bflag);
	}
#endif /* CONFIG_TP_IMAGE */
	else if (!strncmp(argv[1], "imgwrite", 6))
	{
		addr = simple_strtoul(argv[2], NULL, 16);	
		len  = simple_strtoul(argv[3], NULL, 16);
		if (argc == 5)
		{
			buf = simple_strtoul(argv[4], NULL, 16);
		}

		if (FDT_MAGIC == (unsigned long) fdt_magic(buf))
		{
			/*
			* If img_size is large than 0, it specifies that user assigns a image size.
			* Or we read image size from image header.
			*/

			img_size = (unsigned long) fdt_totalsize(buf);
			printf("This is a FIT Image, img_size = 0x%lx\n", img_size);
		}
		else if (IH_MAGIC == uimage_to_cpu(img_hdr->ih_magic))
		{
			img_size = uimage_to_cpu(img_hdr->ih_size)+sizeof(image_header_t);
			printf("This is a uImage, img_size = 0x%lx\n", img_size);
		}

		if (img_size && (img_size != len))
		{
			printf("len(0x%lx) and img_size(0x%lx) is different\n", len, img_size);				
		}

		printf("erase: addr=0x%lx, len=0x%lx\n", addr, len);
		flash_erase(addr, len);

		printf("write: src=0x%lx, len=0x%lx, dst=0x%lx\n", buf, len, addr);
		flash_write(addr, len, &retlen, (unsigned char *) buf);
	}
	else if (!strncmp(argv[1], "upgrade", 6))
	{
		addr = simple_strtoul(argv[2], NULL, 16);
		len  = simple_strtoul(argv[3], NULL, 16);
		if (argc == 5)
		{
			buf = simple_strtoul(argv[4], NULL, 16);
		}

		flash_erase_non_block(addr, len);
		flash_write(addr, len, &retlen, (unsigned char *) buf);
	}
	else
		return CMD_RET_USAGE;
					     
	return 0;
}

U_BOOT_CMD(
		flash,   5,      0,      flash_command,
		"flash - flash command\n",
		"flash usage:\n"
		"	flash init\n"
		"	flash erase [addr] [len]\n"
		"	flash read [src] [len] *[dst]\n"
		"	flash write [dst] [len] *[src]"
);

static int imginfo_command(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct trx_header	trxHdr;
	int					ret = 0;
	unsigned long		first_imgAddr = 0, second_imgAddr = 0;

	first_imgAddr = ecnt_get_tclinux_flash_offset();
	second_imgAddr = ecnt_get_tclinux_slave_flash_offset();

	if(get_tclinux_img_version(first_imgAddr, &trxHdr) == 0) {
		printf("os1:%s",trxHdr.version);
	} else {
		ret = -1;
		printf("Read first tclinux.bin from flash 0x%x error.\n", first_imgAddr);
	}

	if(get_tclinux_img_version(second_imgAddr, &trxHdr) == 0) {
		printf("os2:%s",trxHdr.version);
	} else {
		ret = -1;
		printf("Read second tclinux.bin from flash 0x%x error.\n", second_imgAddr);
	}

	return ret;
}

U_BOOT_CMD(
		imginfo,   1,      0,      imginfo_command,
		"imginfo - Show tclinux.bin version command\n",
		"imginfo usage:\n"
		"	imginfo\n"
);

