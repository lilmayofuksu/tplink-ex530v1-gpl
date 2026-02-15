#include <common.h>
#include <environment.h>
#include <malloc.h>
#include <search.h>
#include <errno.h>
#include <flashhal.h>

DECLARE_GLOBAL_DATA_PTR;

char *env_name_spec = "ECNT Flash";

int saveenv(void)
{
	env_t			env_new;
	ssize_t			len = 0;
	char			*res = NULL;
	unsigned long	retlen = 0;

	res = (char *) &env_new.data;
	len = hexport_r(&env_htab, '\0', 0, &res, ENV_SIZE, 0, NULL);
	if (len < 0)
	{
		error("Cannot export environment: errno = %d\n", errno);
		return 1;
	}
	env_new.crc = crc32(0, env_new.data, ENV_SIZE);


	if (flash_erase_non_block(CONFIG_ENV_OFFSET, CONFIG_ENV_SIZE))
	{
		return 1;
	}

	if (flash_write(CONFIG_ENV_OFFSET, CONFIG_ENV_SIZE, &retlen, (unsigned char *) &env_new))
	{
		return 1;
	}

	return 0;
}

void env_relocate_spec(void)
{
	int				ret = 0;
	char			*buf = NULL;
	unsigned long	retlen;

	buf = (char *) malloc(CONFIG_ENV_SIZE);

	ret = flash_read(CONFIG_ENV_OFFSET,
				CONFIG_ENV_SIZE, &retlen, (unsigned char *) buf);
	if (ret) {
		set_default_env("!flash_read() failed");
		goto out;
	}

	ret = env_import(buf, 1);
	if (ret)
		gd->env_valid = 1;
out:
	if (buf)
		free(buf);
}

int env_init(void)
{
	/* SPI flash isn't usable before relocation */
	gd->env_addr = (ulong)&default_environment[0];
	gd->env_valid = 1;

	return 0;
}

