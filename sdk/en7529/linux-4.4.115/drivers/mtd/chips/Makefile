#
# linux/drivers/chips/Makefile
#

obj-$(CONFIG_MTD)		+= chipreg.o
obj-$(CONFIG_MTD_CFI)		+= cfi_probe.o
obj-$(CONFIG_MTD_CFI_UTIL)	+= cfi_util.o
obj-$(CONFIG_MTD_CFI_STAA)	+= cfi_cmdset_0020.o
obj-$(CONFIG_MTD_CFI_AMDSTD)	+= cfi_cmdset_0002.o
obj-$(CONFIG_MTD_CFI_INTELEXT)	+= cfi_cmdset_0001.o
obj-$(CONFIG_MTD_GEN_PROBE)	+= gen_probe.o
obj-$(CONFIG_MTD_JEDECPROBE)	+= jedec_probe.o
ifneq ($(TCSUPPORT_NEW_SPIFLASH),)
obj-$(CONFIG_MTD_SPIFLASH)	+= newspiflash.o
endif
ifneq ($(strip $(TCSUPPORT_CPU_EN7512) $(TCSUPPORT_CPU_EN7521)),)
ifneq ($(strip $(TCSUPPORT_SPI_CONTROLLER_ECC)),)
obj-y += spi_nand_flash_table.o
obj-y += spi_nand_flash.o
obj-y += spi_controller.o
obj-y += spi_ecc.o
obj-y += spi_nfi.o
else
obj-y += spi_nand_flash.o
obj-y += spi_controller.o
obj-y += spi_ecc.o
obj-y += spi_nfi.o
endif
endif
ifneq ($(strip $(TCSUPPORT_PARALLEL_NAND)),)
obj-y += parallel_nand_flash_table.o
obj-y += parallel_nand_flash.o
endif
ifneq ($(strip $(TCSUPPORT_CPU_EN7523)),)
ifneq ($(strip $(TCSUPPORT_AUTOBENCH)),)
obj-y += flash_test.o
endif
endif
obj-$(CONFIG_MTD_RAM)		+= map_ram.o
obj-$(CONFIG_MTD_ROM)		+= map_rom.o
obj-$(CONFIG_MTD_ABSENT)	+= map_absent.o
