ROOTPATH=$(shell pwd)
include $(BSP_ROOTDIR)/ecnt-function.mk

#CROSS=mips-linux-gnu-
#CROSS=mips-linux-
#CC=$(CROSS)gcc
AR=$(CROSS)ar
LD=$(CROSS)ld
STRIP=$(CROSS)strip
RANLIB=$(CROSS)ranlib
SIZE=$(CROSS)size

CFLAGS= -DTRENDCHIP
ifneq ($(strip $(TCSUPPORT_CODE_SHRINK)),)
CFLAGS += -Os
endif
#CFLAGS= -mips32r2 -msoft-float -DTRENDCHIP -muclibc
ifneq ($(BSP_CFLAGS), )
        CFLAGS+=$(BSP_CFLAGS)
endif

INCLUDE = -I$(TOOLS_TRX_DIR) -I$(GLOBAL_FLASH_LAYOUT_INC_DIR)

all: mtd install

%.o: %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(INCLUDE) -c -o $@ $^

mtd: mtd.o
ifneq ($(strip $(TCSUPPORT_OPENWRT)),)
	$(CC) -o $@ $^
	$(STRIP) $@
else
	$(CC) -o $@ $^
	$(CC) -static -o mtd-static $^
	$(STRIP) $@
	$(STRIP) mtd-static
endif

install:
	$(call BspExclUserFsBinInstall, $(ROOTPATH), mtd)

clean:
	rm -f *.o mtd
	$(call BspExclUserFsBinClean, mtd)
