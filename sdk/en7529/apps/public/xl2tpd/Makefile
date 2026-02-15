ROOTPATH=$(shell pwd)

NEW_VERSION=YES

ANTLRDIR = $(ROOTPATH)/antlr

LIBPCAP=libpcap
LIBPCAPDIR=$(ROOTPATH)/libpcap-1.7.4
LIBPCAPPREFIX=$(ROOTPATH)/libpcap_install

XL2TPD=xl2tpd
XL2TPDDIR=$(ROOTPATH)/xl2tpd-1.3.6
XL2TPDPREFIX=$(ROOTPATH)/xl2tpd_install

MTD-UTILS = mtd-utils
MTD-UTILSDIR = $(ROOTPATH)/mtd-utils-1.4.5
MTD-UTILSREFIX = $(ROOTPATH)/mtd-utils_install

INSTALLDIR =_install/usr/local

CTGWLIBROOT = $(ROOTPATH)/../../private

RM = rm -rf
MV = mv -f
CP = cp -fpR

all : $(XL2TPD) install

$(LIBPCAP):
	#cd $(LIBPCAPDIR) && ./configure --help
	if [ -f $(LIBPCAPDIR)/_is_configure ] ; then \
	echo "already configred..... "; \
	else \
	cd $(LIBPCAPDIR) && ./configure --host=mips-linux --prefix=$(LIBPCAPPREFIX) --enable-shared --enable-static --disable-yydebug \
	 --with-pcap=linux --without-septel --without-dag --without-libnl --without-snf --disable-can	--disable-canusb --disable-dbus --disable-bluetooth --enable-ipv6 && make && make install; \
	touch $(LIBPCAPDIR)/_is_configure; \
	fi

$(XL2TPD): $(LIBPCAP)
	cd $(XL2TPDDIR) && make && make PREFIX=$(XL2TPDPREFIX) install; \

install:
	# xl2tpd
	cp $(XL2TPDPREFIX)/sbin/xl2tpd $(FILESYSTEM_DIR)/userfs/bin/
	cp $(XL2TPDPREFIX)/sbin/xl2tpd-control $(FILESYSTEM_DIR)/userfs/bin/

clean : $(XL2TPD)-clean

$(XL2TPD)-clean: $(LIBPCAP)-clean
	- make -C $(XL2TPDDIR) clean
	- $(RM) $(XL2TPDDIR)/_is_configure

$(LIBPCAP)-clean:
	- make -C $(LIBPCAPDIR) clean distclean
	- $(RM) $(LIBPCAPDIR)/_is_configure

.PHONY : all $(CLASSPATH) $(JAMVM) clean $(JAMVM)-clean $(CLASSPATH)-clean
