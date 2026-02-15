# $Id$

#CROSS=mips-linux-
#CC=$(CROSS)gcc
AR=$(CROSS)ar
LD=$(CROSS)ld
STRIP=$(CROSS)strip
RANLIB=$(CROSS)ranlib
SIZE=$(CROSS)size
CFLAGS= $(ARCH_CFLAGS) -msoft-float -DTRENDCHIP
ifneq ($(strip $(TCSUPPORT_CODE_SHRINK)),)
CFLAGS += -Os
endif
#LIBTCAPI= -lm -L /usr/local/lib -ltcapi
#LIBTCAPI= -lm -L ../lib -ltcapi
LIBTCAPI=$(LIBINSTALL_LDFLAGS) $(TCAPI_LIB) $(BLLIB_SYSTEM)
INCLUDE = $(KERNELHEAD_CFLAGS) $(LIBINSTALL_CFLAGS)
ifneq ($(TC_CFLAGS), )
CFLAGS += $(TC_CFLAGS)
endif
CFLAGS+=$(LIBINSTALL_CFLAGS)
ifeq ($(WITHVOIP),1)
CFLAGS += -DWITHVOIP
endif
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_C1_NEW_GUI)
ifeq ($(TCSUPPORT_C1_NEW_GUI),1)
CFLAGS += -DTCSUPPORT_C1_NEW_GUI
endif
#endif/*TCSUPPORT_COMPILE*/




all: tftpd

tcapi_lib:
	$(MAKE) -C $(APP_TCAPILIB_DIR) clean
	$(MAKE) -C $(APP_TCAPILIB_DIR)
	$(MAKE) -C $(APP_TCAPILIB_DIR) install

%.o: %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(INCLUDE) -c -o $@ $^

tftpd: tftpd.o
	$(CC) $(LIBTCAPI) -o $@ $^
	$(STRIP) $@

clean:
	rm -f *.o tftpd
