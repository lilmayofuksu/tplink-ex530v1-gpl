#
# Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

FLASH_SRCS	:=	$(addprefix plat/ecnt/common/drivers/flash/,	\
				flashhal.c										\
				spi_nor_flash.c									\
				bmt.c											)

FLASH_SRCS	+=	$(KERNEL_EXT_SPI_NAND_DIR)/spi_nand_flash.c			\
				$(KERNEL_EXT_SPI_NAND_DIR)/spi_nand_flash_table.c	\
				$(KERNEL_EXT_SPI_NAND_DIR)/spi_controller.c			\
				$(KERNEL_EXT_SPI_NAND_DIR)/spi_ecc.c				\
				$(KERNEL_EXT_SPI_NAND_DIR)/spi_nfi.c

FLASH_REBUILD_SRCS += $(KERNEL_EXT_SPI_NAND_DIR)/spi_nand_flash_table.c
