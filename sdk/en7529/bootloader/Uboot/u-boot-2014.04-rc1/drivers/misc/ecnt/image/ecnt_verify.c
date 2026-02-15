
#include <common.h>
#include <command.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/stddef.h>
#include <linux/compat.h>
#include <hash.h>
#include <sha256.h>
#include <sha512.h>

#define MAX_STRING	30
#define MAX_NUMBER	5
#define HASH_MODE_MASK		(0x1)

#define FIP_MAX_HEADER_SIZE	(0x2000)

extern unsigned int get_fip_offset(void);

static struct hash_algo sha_algo[2] = {
	{
		"sha256",
		SHA256_SUM_LEN,
		sha256_csum_wd,
		CHUNKSZ_SHA256,
	},
	{
		"sha512",
		SHA512_SUM_LEN,
		sha512_csum_wd,
		CHUNKSZ_SHA512,
	},
};

unsigned char output[HASH_MAX_DIGEST_SIZE]  __aligned(PAGE_SIZE);

char filename[MAX_NUMBER][MAX_STRING] =
{
	"uboot_filename",
	"kernel_filename",
	"\0"
};

extern unsigned int parse_fip(char **buf, unsigned int *buff_size, uint32_t *offset, uint32_t *size);

static int get_sha_alg(void)
{
	unsigned int r0 = 0, r1 = 0, r2 = 0, r3 = 0;

	r0 = 0x82000003;
	r0 = do_smc(r0, r1, r2, r3);

	return r0;
}

static int check_image_name(viod)
{
	char *name = NULL, *valid_name = NULL;
	int verify = 0, i = 0;

	name = getenv("filename");

	if (name == NULL)
		return 0;
	while ((i < MAX_NUMBER) && (verify == 0))
	{
		valid_name = getenv(filename[i]);
		if (valid_name != NULL)
		{
			if (!strcmp(name, valid_name))
				return 1;
		}
		i++;
	}

	return 0;
}

static int verify_img(char *buf, unsigned int buf_size, int sha_alg, unsigned int offset, unsigned int size, unsigned int smc_id)
{
	unsigned int ret = 0, r1 = 0, r2 = 0;

	unsigned char *addr = NULL;

	addr = (unsigned char *) ((uint32_t) buf + offset);
	sha_algo[sha_alg].hash_func_ws(addr, size, output, sha_algo[sha_alg].chunk_size);

	r1 = (unsigned int) output;
	r2 = (unsigned int) buf;
	if (buf_size == FIP_MAX_HEADER_SIZE)
		buf_size = offset;

	flush_cache(r2, buf_size);
	flush_cache(r1, 4096);

	ret = do_smc(smc_id, r1, r2, buf_size);

	return ret;
}


int upgrade_verify(char *buf)
{
#if defined(TCSUPPORT_ARM_SECURE_BOOT)
	int verify = 0;
	int sha_alg = 0;
	unsigned int offset = 0, size = 0;
	unsigned int smc_id = 0;
	unsigned int buf_size = FIP_MAX_HEADER_SIZE;

	verify = check_image_name();
	if (verify != 0)
	{
		smc_id = parse_fip(&buf, &buf_size, &offset, &size);
		if (smc_id != 0)
		{
			sha_alg = get_sha_alg();
			sha_alg = (sha_alg & HASH_MODE_MASK);
			return verify_img(buf, buf_size, sha_alg, offset, size, smc_id);
		}
	}

	return verify;
#else
	return 0;
#endif
}

int read_flash_and_verify(char *buf, unsigned long addr)
{
#if defined(TCSUPPORT_ARM_SECURE_BOOT)
	int sha_alg = 0;
	unsigned int offset = 0, size = 0;
	unsigned int smc_id = 0;
	unsigned int buf_size = FIP_MAX_HEADER_SIZE;
	unsigned long retlen = 0;

	flash_read(addr, buf_size, &retlen, buf);

	smc_id = parse_fip(&buf, &buf_size, &offset, &size);
	if (smc_id != 0)
	{
		flash_read(addr, (offset + size), &retlen, buf);
		sha_alg = get_sha_alg();
		sha_alg = (sha_alg & HASH_MODE_MASK);
		return verify_img(buf, buf_size, sha_alg, offset, size, smc_id);
	}

	return -1;
#else
	return 0;
#endif
}


int boot_verify(char *buf, unsigned int bootflag)
{
	unsigned long kernel_addr = 0;

	if (bootflag == 0)
	{
		kernel_addr = ecnt_get_tclinux_flash_offset();
	}
	else
	{
		kernel_addr = ecnt_get_tclinux_slave_flash_offset();
	}

	if (read_flash_and_verify(buf, kernel_addr))
	{
		printf("verify kernel:0x%x error\n", kernel_addr);
		return -1;
	}

	return 0;
}

static int fip_test_command(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#if defined(TCSUPPORT_ARM_SECURE_BOOT)
	int sha_alg = 0;
	unsigned int offset = 0, size = 0;
	unsigned int smc_id = 0;
	char *buf = (char *) CONFIG_SYS_LOAD_ADDR;
	unsigned int buf_size =  0;

	if (argc >= 2)
		buf = (char *) simple_strtoul(argv[1], NULL, 16);

	smc_id = parse_fip(&buf, &buf_size, &offset, &size);
	if (smc_id != 0)
	{
		sha_alg = get_sha_alg();
		sha_alg = (sha_alg & HASH_MODE_MASK);
		return verify_img(buf, buf_size, sha_alg, offset, size, smc_id);
	}
#endif
	return 0;
}

U_BOOT_CMD(
		fip_test,   4,      0,      fip_test_command,
		"fip_test - test command\n",
		"fip_test usage:\n"
		"	fip_test CMD ARG1 ARG2\n"
);
