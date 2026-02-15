#Cross Compile Setup
nullstring :=
space :=$(nullstring) #end of line


ifeq ($(strip $(TCSUPPORT_TOOLCHAIN_VER)),493)
#export PATH :=/proj/mtk69527/econet-toolchain/buildroot-2015.08.1/output/host/usr/bin:$(PATH)
export PATH :=/opt/trendchip/mips-linux-uclibc-4.9.3/usr/bin:$(PATH)
export HOST=mips-linux
export CROSS=mips-buildroot-linux-uclibc-
export CROSS_COMPILE=$(CROSS)
export LD=$(CROSS)ld
export AR=$(CROSS)ar
export STRIP=$(CROSS)strip
export RANLIB = $(CROSS)ranlib
CFG_CFLAGS += -mips32r2 -muclibc
#export SYSROOT=/proj/mtk69527/econet-toolchain/buildroot-2015.08.1/output/host/usr/mips-buildroot-linux-uclibc/sysroot
export SYSROOT=/opt/trendchip/mips-linux-uclibc-4.9.3/usr/mips-buildroot-linux-uclibc/sysroot
export CC=$(CROSS)gcc -mips32r2 -msoft-float
export CCC=$(CROSS)g++ -mips32r2 -msoft-float
export ARCH_CFLAGS = -mips32r2 -uclibc
endif

ifeq ($(strip $(TCSUPPORT_TOOLCHAIN_VER)),436)
#ifeq ($(strip $(COMPILE_TOOLCHAIN)),mips-unknown-linux-uclibc)
export PATH := /opt/trendchip/mips-linux-uclibc-4.3.6-v2/usr/bin:$(PATH)
export HOST=mips-linux
export CROSS=mips-unknown-linux-uclibc-
export CROSS_COMPILE=$(CROSS)
ifneq ($(strip $(TCSUPPORT_MEMORY_SHRINK)),)
export CC=$(CROSS)gcc -mips32r2 -msoft-float -Os
else
export CC=$(CROSS)gcc -mips32r2 -msoft-float
endif
export CCC=$(CROSS)g++ -mips32r2 -msoft-float
export LD=$(CROSS)ld
export AR=$(CROSS)ar
export STRIP=$(CROSS)strip
export RANLIB = $(CROSS)ranlib
export SYSROOT=/opt/trendchip/mips-linux-uclibc-4.3.6-v2/usr/mips-unknown-linux-uclibc/sysroot
export LD_LIBRARY_PATH:=/opt/trendchip/mips-linux-uclibc-4.3.6-v2/usr/lib:$(LD_LIBRARY_PATH)
CFG_CFLAGS += -mips32r2 -muclibc
export ARCH_CFLAGS = -mips32r2 -uclibc
endif

ifeq ($(strip $(COMPILE_TOOLCHAIN)),mips-linux)
export PATH :=/opt/trendchip/mips-linux-3.4.6/bin:$(PATH)
export HOST=mips-linux
#export CROSS=/opt/trendchip/mips-linux-3.4.6/bin/mips-linux-
export CROSS=mips-linux-
export CROSS_COMPILE=$(CROSS)
export CC=$(CROSS)gcc
export LD=$(CROSS)ld
export AR=$(CROSS)ar
export STRIP=$(CROSS)strip
export RANLIB = $(CROSS)ranlib
TC3162_CFLAG=$(space)-mips1 -msoft-float
CFG_CFLAGS += -mips1
export ARCH_CFLAGS = -mips1
endif

ifneq ($(strip $(TCSUPPORT_GCC4_6_GLIBC2_20)),)
ifneq ($(strip $(TCSUPPORT_LITTLE_ENDIAN)),)
export PATH:=/opt/trendchip/mipsel-linux-glibc-4.6.3-kernel3.18/usr/bin:$(PATH)
export HOST=mipsel-linux
export CROSS=mipsel-buildroot-linux-gnu-
export CROSS_COMPILE=$(CROSS)
export CC=$(CROSS)gcc -mips32r2 -msoft-float
export CCC=$(CROSS)g++ -mips32r2 -msoft-float
export LD=$(CROSS)ld
export AR=$(CROSS)ar
export STRIP=$(CROSS)strip
export RANLIB = $(CROSS)ranlib
export SYSROOT=/opt/trendchip/mipsel-linux-glibc-4.6.3-kernel3.18/usr/mipsel-buildroot-linux-gnu/sysroot
else
export PATH:=/opt/trendchip/mips-linux-glibc-4.6.3/usr/bin:$(PATH)
export HOST=mips-linux
export CROSS=mips-buildroot-linux-gnu-
export CROSS_COMPILE=$(CROSS)
export CC=$(CROSS)gcc -mips32r2 -msoft-float
export CCC=$(CROSS)g++ -mips32r2 -msoft-float
export LD=$(CROSS)ld
export AR=$(CROSS)ar
export STRIP=$(CROSS)strip
export RANLIB = $(CROSS)ranlib
export SYSROOT=/opt/trendchip/mips-linux-glibc-4.6.3/usr/mips-buildroot-linux-gnu/sysroot
endif
export ARCH_CFLAGS = -mips32r2 -uclibc
endif

ifneq ($(strip $(TCSUPPORT_GCC4_9_GLIBC2_20)),)
export PATH:=/opt/trendchip/mips-linux-glibc-4.9.3/usr/bin:$(PATH)
export HOST=mips-linux
export CROSS=mips-buildroot-linux-gnu-
export CROSS_COMPILE=$(CROSS)
export CC=$(CROSS)gcc -mips32r2 -msoft-float
export CCC=$(CROSS)g++ -mips32r2 -msoft-float
export LD=$(CROSS)ld
export AR=$(CROSS)ar
export STRIP=$(CROSS)strip
export RANLIB = $(CROSS)ranlib
export SYSROOT=/opt/trendchip/mips-linux-glibc-4.9.3/usr/mips-buildroot-linux-gnu/sysroot
export ARCH_CFLAGS = -mips32r2 -uclibc
endif

ifeq ($(strip $(COMPILE_TOOLCHAIN)),mips-linux-uclibc)
export PATH :=/opt/trendchip/mips-linux-uclibc/usr/bin:$(PATH)
export HOST=mips-linux
export CROSS=mips-linux-uclibc-
export CROSS_COMPILE=$(CROSS)
export CC=mips-linux-uclibc-gcc -mips32r2 -msoft-float
export STRIP=mips-linux-uclibc-strip
export SYSROOT=/opt/trendchip/mips-linux-uclibc
export LD=$(CROSS)ld
export AR=$(CROSS)ar
CFG_CFLAGS += -mips32r2 -muclibc
export RANLIB = $(CROSS)ranlib
export ARCH_CFLAGS = -mips32r2 -uclibc
endif

ifneq ($(strip $(TCSUPPORT_GCC4_6_GLIBC2_22)),)
ifneq ($(strip $(TCSUPPORT_LITTLE_ENDIAN)),)
export PATH:=/opt/trendchip/mipsel-linux-glibc2.22-4.6.3/usr/bin:$(PATH)
export HOST=mipsel-linux
export CROSS=mipsel-buildroot-linux-gnu-
export CROSS_COMPILE=$(CROSS)
export CROSS_TARGET=mipsel-buildroot-linux-gnu
export CC=$(CROSS)gcc -mips32r2 -msoft-float
export CCC=$(CROSS)g++ -mips32r2 -msoft-float
export CXX=$(CROSS)g++ -mips32r2 -msoft-float
export LD=$(CROSS)ld
export AR=$(CROSS)ar
export STRIP=$(CROSS)strip
export RANLIB = $(CROSS)ranlib
export SYSROOT=/opt/trendchip/mipsel-linux-glibc2.22-4.6.3/usr/mipsel-buildroot-linux-gnu/sysroot
export TOOLCHAIN_LIB=/opt/trendchip/mipsel-linux-glibc2.22-4.6.3/usr/mipsel-buildroot-linux-gnu/sysroot/lib
else
export PATH:=/opt/trendchip/mips-linux-glibc2.22-4.6.3/usr/bin:$(PATH)
export HOST=mips-linux
export CROSS=mips-buildroot-linux-gnu-
export CROSS_COMPILE=$(CROSS)
export CROSS_TARGET=mips-buildroot-linux-gnu
export CC=$(CROSS)gcc -mips32r2 -msoft-float
export CCC=$(CROSS)g++ -mips32r2 -msoft-float
export CXX=$(CROSS)g++ -mips32r2 -msoft-float
export LD=$(CROSS)ld
export AR=$(CROSS)ar
export STRIP=$(CROSS)strip
export RANLIB = $(CROSS)ranlib
export SYSROOT=/opt/trendchip/mips-linux-glibc2.22-4.6.3/usr/mips-buildroot-linux-gnu/sysroot
export TOOLCHAIN_LIB=/opt/trendchip/mips-linux-glibc2.22-4.6.3/usr/mips-buildroot-linux-gnu/sysroot/lib
endif
export ARCH_CFLAGS = -mips32r2 -uclibc
endif

ifneq ($(strip $(TCSUPPORT_GCC4_6_UCLIBC)),)
ifneq ($(strip $(TCSUPPORT_LITTLE_ENDIAN)),)
export PATH :=/opt/trendchip/mipsel-linux-uclibc-4.6.3-kernel3.18/usr/bin/:$(PATH)
export HOST=mipsel-linux
export CROSS=mipsel-buildroot-linux-uclibc-
export CROSS_COMPILE=$(CROSS)
export CROSS_TARGET=mipsel-buildroot-linux-uclibc
export CC=$(CROSS)gcc -mips32r2 -msoft-float
export CCC=$(CROSS)g++ -mips32r2 -msoft-float
export LD=$(CROSS)ld
export AR=$(CROSS)ar
export STRIP=$(CROSS)strip
export RANLIB = $(CROSS)ranlib
export SYSROOT=/opt/trendchip/mipsel-linux-uclibc-4.6.3-kernel3.18/usr/mipsel-buildroot-linux-uclibc/sysroot/
else
export PATH :=/opt/trendchip/mips-linux-uclibc-4.6.3-kernel3.18/usr/bin:$(PATH)
export HOST=mips-linux
export CROSS=mips-buildroot-linux-uclibc-
export CROSS_COMPILE=$(CROSS)
export CROSS_TARGET=mips-buildroot-linux-uclibc
export CC=$(CROSS)gcc -mips32r2 -msoft-float
export CCC=$(CROSS)g++ -mips32r2 -msoft-float
export LD=$(CROSS)ld
export AR=$(CROSS)ar
export STRIP=$(CROSS)strip
export RANLIB = $(CROSS)ranlib
export SYSROOT=/opt/trendchip/mips-linux-uclibc-4.6.3-kernel3.18/usr/mips-buildroot-linux-uclibc/sysroot
endif
export ARCH_CFLAGS = -mips32r2 -uclibc
endif

ifeq ($(strip $(COMPILE_TOOLCHAIN)),arm32-gcc493-uclibc09332)
ifeq ($(strip $(TCSUPPORT_CPU_ARMV8_64)),)
export TOOLCHAIN_BASE=/opt/trendchip/buildroot-gcc493_uclibc09332_arm32/usr
export HOST=arm-linux-gnueabi
export CROSS=arm-buildroot-linux-uclibcgnueabi-
else
export TOOLCHAIN_BASE=/opt/trendchip/buildroot-gcc910_uclibc-ng1031_arm64
export HOST=aarch64-linux-uclibc
export CROSS=aarch64-buildroot-linux-uclibc-
endif
export PATH :=$(TOOLCHAIN_BASE)/bin/:$(PATH)
export CROSS_COMPILE=$(CROSS)
ifeq ($(strip $(TCSUPPORT_CPU_ARMV8_64)),)
export CC=$(CROSS)gcc -mfloat-abi=soft
export CCC=$(CROSS)g++ -mfloat-abi=soft
else
export CC=$(CROSS)gcc
export CCC=$(CROSS)g++
endif
export LD=$(CROSS)ld
# Always use GNU ld
ifneq ($(shell $(CROSS_COMPILE)ld.bfd -v 2> /dev/null),)
export LD = $(CROSS)ld.bfd
else
export LD = $(CROSS)ld
endif
export AR=$(CROSS)ar
export STRIP=$(CROSS)strip
export RANLIB = $(CROSS)ranlib
export SYSROOT=$(TOOLCHAIN_BASE)/arm-buildroot-linux-uclibcgnueabi/sysroot
export LD_LIBRARY_PATH:=$(TOOLCHAIN_BASE)/lib:$(LD_LIBRARY_PATH)
export ARCH_CFLAGS =
endif

ifneq ($(strip $(TCSUPPORT_GCC4_9_3_GLIBC2_22_ARM)),)
TC_CFLAGS += -fsigned-char
BSP_CFLAGS += -fsigned-char
#TOOLCHAIN_BASE=/opt/trendchip/buildroot-gcc493_glibc222_arm32/usr
export TOOLCHAIN_BASE=/opt/trendchip/buildroot-gcc493_glibc222_arm32_32bServer/usr
export HOST=arm-linux
export CROSS=arm-buildroot-linux-gnueabi-
export PATH :=$(TOOLCHAIN_BASE)/bin/:$(PATH)
export CROSS_COMPILE=$(CROSS)
export CC=$(CROSS)gcc -mfloat-abi=soft
export CCC=$(CROSS)g++ -mfloat-abi=soft
export LD=$(CROSS)ld
# Always use GNU ld
ifneq ($(shell $(CROSS_COMPILE)ld.bfd -v 2> /dev/null),)
export LD = $(CROSS)ld.bfd
else
export LD = $(CROSS)ld
endif
export AR=$(CROSS)ar
export STRIP=$(CROSS)strip
export RANLIB = $(CROSS)ranlib
export SYSROOT=$(TOOLCHAIN_BASE)/arm-buildroot-linux-gnueabi/sysroot
export LD_LIBRARY_PATH:=$(TOOLCHAIN_BASE)/lib:$(LD_LIBRARY_PATH)
export ARCH_CFLAGS =
endif

ifeq ($(strip $(COMPILE_TOOLCHAIN)),arm32-gcc910-glibc229)
ifeq ($(strip $(TCSUPPORT_CPU_ARMV8_64)),)
export TOOLCHAIN_BASE=/opt/trendchip/buildroot-gcc910_glibc229_arm32
export HOST=arm-linux-gnueabi
export CROSS=arm-buildroot-linux-gnueabi-
else
export TOOLCHAIN_BASE=/opt/trendchip/buildroot-gcc910_glibc229_arm64
export HOST=aarch64-linux-gnu
export CROSS=aarch64-buildroot-linux-gnu-
endif
export PATH :=$(TOOLCHAIN_BASE)/bin/:$(PATH)
export CROSS_COMPILE=$(CROSS)
ifeq ($(strip $(TCSUPPORT_CPU_ARMV8_64)),)
export CC=$(CROSS)gcc -mfloat-abi=soft
export CCC=$(CROSS)g++ -mfloat-abi=soft
else
export CC=$(CROSS)gcc
export CCC=$(CROSS)g++
endif
export LD=$(CROSS)ld
# Always use GNU ld
ifneq ($(shell $(CROSS_COMPILE)ld.bfd -v 2> /dev/null),)
export LD = $(CROSS)ld.bfd
else
export LD = $(CROSS)ld
endif
export AR=$(CROSS)ar
export STRIP=$(CROSS)strip
export RANLIB = $(CROSS)ranlib
export SYSROOT=$(TOOLCHAIN_BASE)/arm-buildroot-linux-gnueabi/sysroot
export LD_LIBRARY_PATH:=$(TOOLCHAIN_BASE)/lib:$(LD_LIBRARY_PATH)
export ARCH_CFLAGS =
endif

ifneq ($(strip $(TCSUPPORT_LITTLE_ENDIAN)),)
export ENDIANNESS=little
else
export ENDIANNESS=big
endif