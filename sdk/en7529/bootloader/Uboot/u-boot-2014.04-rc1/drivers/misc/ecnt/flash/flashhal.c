#include <asm/tc3162.h>
#include <asm/io.h>
#include <flashhal.h>
#include <spi/spi_nand_flash.h>
#include <malloc.h>
#include "flash_layout/tc_partition.h"

unsigned long flash_base;
extern struct mtd_info mtd;
extern struct ra_nand_chip ra;

#ifndef _SPI_NAND_BLOCK_SIZE_512KBYTE
#define _SPI_NAND_BLOCK_SIZE_512KBYTE		0x80000
#endif

unsigned char block_buf[_SPI_NAND_BLOCK_SIZE_512KBYTE];

int flash_partial_write(unsigned long to, unsigned long len, const unsigned char *buf)
{
	unsigned long erasesize = 0;
	unsigned long block_start_addr = 0;
	unsigned long block_offset_addr = 0;
	unsigned long retlen = 0;

    if (IS_NANDFLASH) 
	{
		erasesize = ( 1 << ra.flash->erase_shift );
	}
#ifdef TCSUPPORT_NEW_SPIFLASH
	else if (IS_SPIFLASH)
	{
		to &= ~(mtd.offset);
		erasesize = mtd.erasesize;
	}
#endif
	else
	{
		printf("only support spiflash/nandflash\n");
		return -1;
	}

	block_offset_addr = to % erasesize;
	block_start_addr = to - block_offset_addr;

	if (block_offset_addr + len > erasesize)
	{
		printf("over block boundary\n");
		return -1;
	}

	memset(block_buf, 0, _SPI_NAND_BLOCK_SIZE_512KBYTE);

	if (flash_read(block_start_addr, erasesize, &retlen, block_buf) != 0)
	{
		printf("read error\n");
		return -1;
	}

	if (flash_erase(block_start_addr, erasesize) != 0)
	{
		printf("erase error\n");
		return -1;
	}

	memcpy(&block_buf[block_offset_addr], buf, len);

	if (flash_write(block_start_addr, erasesize, &retlen, block_buf) != 0)
	{
		printf("write error\n");
		return -1;
	}

	return 0;
}

int flash_erase_non_block(unsigned long addr, unsigned long size)
{
	unsigned long start = 0, end = 0, add_head_size = 0, add_tail_size = 0, temp = 0;
	unsigned char *p_head_buf = NULL, *p_tail_buf = NULL;
	int result = -1;

	if (IS_NANDFLASH)
	{
		SPI_NAND_FLASH_RTN_T status;
		struct SPI_NAND_FLASH_INFO_T flash_info_t;

		SPI_NAND_Flash_Get_Flash_Info(&flash_info_t);

		temp = (addr % flash_info_t.erase_size);
		if (temp != 0)										/* addr isn't block alignment */
		{
			start = addr - temp;							/* record start_addr of first block */
			add_head_size = temp;							/* record redundant size of first block */

			p_head_buf = malloc((size_t) add_head_size);	/* alloc buf and read redundant data of first block */

			if (p_head_buf == NULL)
			{
				goto error;
			}

			if (nandflash_read(start, add_head_size, &temp, p_head_buf, &status))
			{
				goto error;
			}
		}

		temp = ((addr + size) % flash_info_t.erase_size);
		if (temp != 0)										/* end_addr isn't block alignment */
		{
			end = addr + size;								/* record end_addr */
			add_tail_size = flash_info_t.erase_size - temp;	/* record redundant size of last block */

			p_tail_buf = malloc((size_t) add_tail_size);	/* alloc buf and read redundant data of last block */

			if (p_tail_buf == NULL)
			{
				goto error;
			}

			if (nandflash_read(end, add_tail_size, &temp, p_tail_buf, &status))
			{
				goto error;
			}
		}

		if ((add_head_size == 0) && (add_tail_size == 0))
		{
			return nandflash_erase(addr, size);
		}
		else
		{
			if (nandflash_erase(start, (size + add_head_size + add_tail_size)))
			{
				goto error;
			}

			if (add_head_size != 0)
			{
				if (nandflash_write(start, add_head_size, &temp, p_head_buf))
				{
					goto error;
				}
			}

			if (add_tail_size != 0)
			{
				if (nandflash_write(end, add_tail_size, &temp, p_tail_buf))
				{
					goto error;
				}
			}
		}

	}
	else if (IS_SPIFLASH)
	{
		return -1;
	}
	else
		return -1;

	result = 0;

error:
	if (p_head_buf != NULL)
	{
		free(p_head_buf);
	}

	if (p_tail_buf != NULL)
	{
		free(p_tail_buf);
	}

	return result;
}

int flash_init(unsigned long rom_base)
{
	if (IS_NANDFLASH)
	{
		flash_base = 0x0;
		return nandflash_init(flash_base);
	}
#ifdef TCSUPPORT_NEW_SPIFLASH
	else if (IS_SPIFLASH)
	{
		VPint(0x1fb00038) &= 0xffe0e0e0;
		VPint(0x1fb00038) |= 0x80070F00;
		flash_base = 0x1c000000;

		return spiflash_init(flash_base);
	}
#endif
	else
		return -1;
}

int flash_erase(unsigned long addr, unsigned long size)
{
	if (IS_NANDFLASH)
	{
		return nandflash_erase(addr, size);
	}
#ifdef TCSUPPORT_NEW_SPIFLASH
	else if (IS_SPIFLASH)
	{
		return spiflash_erase(addr, size);
	} 
#endif
	else
		return -1;
}

int flash_read(unsigned long from,
	unsigned long len, unsigned long *retlen, unsigned char *buf)
{
	SPI_NAND_FLASH_RTN_T status;

	if (IS_NANDFLASH)
	{
		return nandflash_read(from, len, retlen, buf, &status);
	}
#ifdef TCSUPPORT_NEW_SPIFLASH
	else if (IS_SPIFLASH)
	{
		return spiflash_read(from, len, retlen, buf);
	}
#endif
	else
		return -1;
}

int flash_write(unsigned long to,
	unsigned long len, unsigned long *retlen, const unsigned char *buf)
{
	if (IS_NANDFLASH)
	{
		return nandflash_write(to, len, retlen, buf);
	}
#ifdef TCSUPPORT_NEW_SPIFLASH
	else if (IS_SPIFLASH)
	{
		return spiflash_write(to, len, retlen, buf);
	}
#endif
	else
		return -1;
}

