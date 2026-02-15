# $Id$

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
ifneq ($(TC_CFLAGS), )
        CFLAGS+=$(TC_CFLAGS)
endif

INCLUDE = $(KERNELHEAD_CFLAGS)

all: mtd

%.o: %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(INCLUDE) -c -o $@ $^

mtd: mtd.o
	$(CC) -o $@ $^
	$(CC) -static -o mtd-static $^
	$(STRIP) $@
	$(STRIP) mtd-static

clean:
	rm -f *.o mtd
