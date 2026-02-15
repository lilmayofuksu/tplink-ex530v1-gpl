# /**************************************
#  ecmh - Easy Cast du Multi Hub
#  by Jeroen Massar <jeroen@unfix.org>
# ***************************************
# $Author: vend_sr00002 $
# $Id: //BBN_Linux/DEV/main/tclinux_phoenix/apps/public/ecmh-2005.02.09/src/Makefile#4 $
# $Date: 2018/01/30 $
# **************************************/
#
# Source Makefile for ecmh - Jeroen Massar <jeroen@unfix.org>
#
# ECMH_VERSION and ECMH_OPTIONS need to be defined, gets done by toplevel Makefile
#
# One should make this using the main Makefile (thus one dir up)
#
# FreeBSD people should uncomment these if they don't have GNU make and
# then run 'make' from this directory
# On FreeBSD4 install the "libgnugetopt" port to get getopt working
#ECMH_OPTIONS=-DECMH_BPF -DECMH_SUPPORT_MLD2 -DECMH_GETIFADDR -I/usr/local/include/
#ECMH_VERSION=2004.10.09
#ECMH_LDFREEBSD=-L/usr/local/lib/ -lgnugetopt

# Below here nothing should have to be changed
BINS	= ecmh
SRCS	= ecmh.c linklist.c common.c interfaces.c quick_leave.c groups.c grpint.c subscr.c
INCS	= ecmh.h linklist.h common.h interfaces.h groups.h quick_leave.h grpint.h subscr.h mld.h
DEPS	= ../Makefile Makefile
OBJS	= ecmh.o linklist.o common.o interfaces.o quick_leave.o groups.o grpint.o subscr.o 
WARNS	= -W -Wall -pedantic -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Waggregate-return -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs -Winline -Wbad-function-cast -fshort-enums -fstrict-aliasing -fno-common -Wpacked -Wpadded
EXTRA   = -g
#EXTRA	= -O3

CFLAGS	= $(WARNS) $(EXTRA) -D_GNU_SOURCE -D'ECMH_VERSION="$(ECMH_VERSION)"' $(ECMH_OPTIONS) -I$(KERNEL_HEADER)
ifneq ($(BSP_CFLAGS), )
	CFLAGS += $(BSP_CFLAGS)
endif
LDFLAGS	= $(ECMH_LDFREEBSD)
CC	:= @echo "[Compiling] $$@"; $(CC)

all:	$(BINS)

ecmh:	$(OBJS) ${INCS} ${DEPS}
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)
#	@mips-linux-strip $@

clean:
	$(RM) -f $(OBJS) $(BINS)

# Mark targets as phony
.PHONY : all clean

