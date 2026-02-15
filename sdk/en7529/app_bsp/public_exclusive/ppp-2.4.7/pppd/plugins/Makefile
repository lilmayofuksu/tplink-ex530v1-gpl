#CC	= gcc
COPTS	= -O2 -g
CFLAGS	= $(COPTS) -I.. -I../../include -fPIC
LDFLAGS	= -shared
INSTALL	= install

DESTDIR = $(INSTROOT)/usr/local
BINDIR = $(DESTDIR)/sbin
MANDIR = $(DESTDIR)/share/man/man8
LIBDIR = $(DESTDIR)/lib/pppd/$(VERSION)

#becaus lack of -I $(KERNEL_HEADER) for some profile
#SUBDIRS := rp-pppoe pppoatm pppol2tp
SUBDIRS := rp-pppoe pppoatm
# Uncomment the next line to include the radius authentication plugin
SUBDIRS += radius
PLUGINS := minconn.so passprompt.so passwordfd.so winbind.so

# This setting should match the one in ../Makefile.linux
MPPE=y

ifdef MPPE
CFLAGS   += -DMPPE=1
endif

# include dependencies if present
ifeq (.depend,$(wildcard .depend))
include .depend
endif

all:	$(PLUGINS)
	for d in $(SUBDIRS); do $(MAKE) $(MFLAGS) -C $$d all; done

%.so: %.c
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $^

VERSION = $(shell awk -F '"' '/VERSION/ { print $$2; }' ../patchlevel.h)

install: $(PLUGINS)
	$(INSTALL) -d $(LIBDIR)
	$(INSTALL) $? $(LIBDIR)
	for d in $(SUBDIRS); do $(MAKE) $(MFLAGS) -C $$d install; done

clean:
	rm -f *.o *.so *.a
	for d in $(SUBDIRS); do $(MAKE) $(MFLAGS) -C $$d clean; done

depend:
	$(CPP) -M $(CFLAGS) *.c >.depend
	for d in $(SUBDIRS); do $(MAKE) $(MFLAGS) -C $$d depend; done
