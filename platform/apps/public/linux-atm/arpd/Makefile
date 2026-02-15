# brcm
#LIBS=-latmd
#LIBDEPS=../lib/libatmd.a
#LIBS=-lfl # lex may want  -ll  here
INCLUDES += ../lib
#include ../Rules.make
MIPS=true
DEBUG=false
UCLIB=true

# dbs modified for qsdk toolchain -static option cannot work 2014-10-30
ifeq ($(strip $(HOSTCC)), arm-openwrt-linux-uclibcgnueabi-gcc)
LDFLAGS= -s 
else ifeq ($(strip $(HOSTCC)),mips-openwrt-linux-gcc)
LDFLAGS= -s
else
LDFLAGS= -s --static
endif	
# end 2014-10-30

KERNEL_VERSION := $(KERNELVERSION) # From Makefile.sdk

define min_version
MIN_KER_VER=$(shell expr $1 \* 65536 + $2 \* 256 + $3)
endef

MIN_KER_VER_CHECK=$(shell echo test | awk '{if($(KERNEL_VERSION)>$(call min_version,$1,$2,$3)) print 1; else print 0}')

ifeq ($(call MIN_KER_VER_CHECK,3,8,0),1)
CFLAGS= -I$(INCLUDES) -I./include -fomit-frame-pointer -Wall -Werror=implicit-function-declaration
else
CFLAGS= -I$(INCLUDES) -I$(KERNELPATH)/usr/include -fomit-frame-pointer -Wall -Werror=implicit-function-declaration
endif
LIBS=-lc
UCLIB_ARM_WRAPPER_DIR=../../libs/uClibc/extra/gcc-uClibc

CROSS=$(TOOLPREFIX)
CC=$(CROSS)gcc
AR=$(CROSS)ar
RANLIB=$(CROSS)ranlib
LD=$(CROSS)ld
LIBDIR=/usr/lib
#LIBS=-lelf -lc //7300 have

ifeq ($(strip $(UCLIB)), true)
	# Using uClibc
	ifeq ($(strip $(ARM)),true)
		CROSS=arm-linux-
		CC=$(UCLIB_ARM_WRAPPER_DIR)/arm-uclibc-gcc
		LD=$(UCLIB_ARM_WRAPPER_DIR)/arm-uclibc-ld
		LIBDIR=/skiff/local/arm-linux/lib
	endif
	###################5100##############################
	ifeq ($(strip $(MIPS)),true)
		CROSS_PATH=/usr/local/mips-uclinux
		CROSS=$(CROSS_PATH)/bin/mips-uclibc-
		CC=$(CROSS)gcc
		AR=$(CROSS)ar
		LD=$(CROSS)ld
		LIBDIR=$(CROSS_PATH)/lib
		RANLIB=$(CROSS)ranlib
        ###################################################
	else
		CC=$(UCLIB_ARM_WRAPPER_DIR)/i386-uclibc-gcc
		LD=$(UCLIB_ARM_WRAPPER_DIR)/i386-uclibc-ld
		LIBDIR=/usr/lib
	endif
else
	ifeq ($(strip $(ARM)),true)
		CROSS=arm-linux-
		LIBDIR=/skiff/local/arm-linux/lib
	else
		CROSS=
		LIBDIR=/usr/lib
	endif
endif
ifeq ($(strip $(DEBUG)),true)
	CFLAGS=-g -Wall 
endif

#CROSS=mips-linux-
CROSS=$(TOOLPREFIX)
CC=$(CROSS)gcc
AR=$(CROSS)ar
LD=$(CROSS)ld
STRIP=$(CROSS)strip
RANLIB=$(CROSS)ranlib
SIZE=$(CROSS)size
LIBDIR=$(CROSS_PATH)/lib

EXEC = atmarp #rt2684d 
EXECD = atmarpd
EXECS = $(EXEC) $(EXECD)
#ifeq ($(strip $(BUILD_RT2684D)),static)
#CFLAGS += -DBUILD_STATIC
#endif
LDFLAGS += -Wl,-allow-shlib-undefined
#OBJS=rt2684d.o arp.o io.o itf.o table.o
OBJ=atmarp.o 
OBJD=atmarpd.o io.o itf.o table.o arp.o 
#BOOTPGMS=atmarp atmarpd
MAN8=atmarp.8 atmarpd.8
SYSHDR=atmarp.h atmarpd.h


all: $(EXECS) 


dynamic: lib-atm-atmd $(EXECS) install

static: lib-atm-atmd rt2684d.a

lib-atm-atmd:
	$(MAKE) -C ../lib $(MFLAGS) all

install:
	install -m 755 rt2684d $(INSTALL_DIR)/bin
	$(STRIP) $(INSTALL_DIR)/bin/rt2684d

rt2684d.a: $(OBJS)
	$(AR) rcs rt2684d.a $(OBJS)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) -L../lib -latmd -latm

$(EXECD): $(OBJD)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) -L../lib -latmd -latm


#.c.o:
#	$(CC) -c $(CFLAGS) $<

clean:
	$(MAKE) -C ../lib clean
	rm -f *.o *.a $(EXECS)
