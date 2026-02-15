ROOTPATH=$(shell pwd)

DEBUG_LOCAL:=
#DEBUG_LOCAL:=y

LXC=lxc
LXCDIR=$(ROOTPATH)/lxc-1.0.8
LXCPREFIX=$(ROOTPATH)/install_lxclib
LXCBIN=$(ROOTPATH)/install_lxcsbin

LIBCAP=libpcap
LIBCAPDIR=$(ROOTPATH)/libcap-2.17
LIBCAPPREFIX=$(ROOTPATH)/install_libcap

LIBPCRE=libpcre
LIBPCREDIR=$(ROOTPATH)/pcre-8.43
#LIBPCREPREFIX=$(ROOTPATH)/install_PCRE
LIBPCREPREFIX=$(GLIB_PRLIB)


INSTALL_ROOT=$(ROOTPATH)/install-all
INSTALL_COMMON=$(INSTALL_ROOT)/install-common

ifneq ($(strip $(TCSUPPORT_GLIBC_VER)),)
ifneq ($(strip $(TCSUPPORT_LITTLE_ENDIAN)),)
INSTALL_COMMON_TOOLCHAIN=$(INSTALL_ROOT)/install-common-glibc-el
else
INSTALL_COMMON_TOOLCHAIN=$(INSTALL_ROOT)/install-common-glibc-eb
endif
else
ifneq ($(strip $(TCSUPPORT_LITTLE_ENDIAN)),)
INSTALL_COMMON_TOOLCHAIN=$(INSTALL_ROOT)/install-common-uclibc-el
else
INSTALL_COMMON_TOOLCHAIN=$(INSTALL_ROOT)/install-common-uclibc-eb
endif
endif

DBUS=dbus
DBUSDIR=$(ROOTPATH)/dbus-1.8.8
DBUSPREFIX=$(ROOTPATH)/install_dbuslib
DBUSBIN=$(ROOTPATH)/install_dbusbin

DBUSDIR2=$(ROOTPATH)/dbus-1.10.24

ifneq ($(strip $(TCSUPPORT_CUC)),)
ifneq ($(strip $(TCSUPPORT_GLIBC_VER)),)
DBUSBIN_DIR=$(INSTALL_COMMON_TOOLCHAIN)/cuc/install_dbusbin
LXCBIN_INSTALL=$(INSTALL_COMMON_TOOLCHAIN)/cuc/install_lxcbin
GLIBBIN_INSTALL=$(INSTALL_COMMON_TOOLCHAIN)/cuc/install_glibbin
GLIB_INSTALL=$(INSTALL_COMMON)/cuc/install_glib
else
DBUSBIN_DIR=$(INSTALL_COMMON_TOOLCHAIN)/install_dbusbin
GLIBBIN_INSTALL=$(INSTALL_COMMON_TOOLCHAIN)/install_glibbin
LXCBIN_INSTALL=$(INSTALL_COMMON_TOOLCHAIN)/install_cuc_lxcbin
DBUSBIN_CT=$(INSTALL_COMMON_TOOLCHAIN)/install_cuc_dbus
endif
SAFBIN_DIR=$(INSTALL_COMMON_TOOLCHAIN)/install_ufwmg
else
ifneq ($(strip $(TCSUPPORT_GLIBC_VER)),)
DBUSBIN_DIR=$(INSTALL_COMMON_TOOLCHAIN)/ct/install_dbusbin
LXCBIN_INSTALL=$(INSTALL_COMMON_TOOLCHAIN)/ct/install_lxcbin
GLIBBIN_INSTALL=$(INSTALL_COMMON_TOOLCHAIN)/ct/install_glibbin
GLIB_INSTALL=$(INSTALL_COMMON)/ct/install_glib
else
DBUSBIN_DIR=$(INSTALL_COMMON_TOOLCHAIN)/install_dbusbin
GLIBBIN_INSTALL=$(INSTALL_COMMON_TOOLCHAIN)/install_glibbin
LXCBIN_INSTALL=$(INSTALL_COMMON_TOOLCHAIN)/install_lxcbin
DBUSBIN_CT=$(INSTALL_COMMON_TOOLCHAIN)/install_ctc_dbus
endif
SAFBIN_DIR=$(INSTALL_COMMON_TOOLCHAIN)/install_saf
endif

DBUS_CONF=$(ROOTPATH)/ecnt_dbus_config

GLIB2=glib2
GLIBDIR2=$(ROOTPATH)/glib-2.56.4
GLIB2CACHEFILE=$(GLIBDIR2)/mips.cache

GLIB=glib
#GLIBDIR=$(ROOTPATH)/glib-2.46.2
GLIBDIR=$(ROOTPATH)/glib-2.41.1
GLIB_PRLIB=$(ROOTPATH)/install_glib_prelib
GLIBPREFIX=$(ROOTPATH)/install_glib
GLIBBIN=$(ROOTPATH)/install_glibbin

GLIBCACHEFILE=$(GLIBDIR)/mips.cache

ZLIB =zlib
ZLIBDIR = $(ROOTPATH)/zlib-1.2.8
#ZLIBPREFIX = $(ROOTPATH)/zlib_install
ZLIBPREFIX = $(GLIB_PRLIB)

EXPAT = expat
EXPATDIR = $(ROOTPATH)/expat-2.1.0
EXPATPREFIX = $(ROOTPATH)/install_expat

ICONV = libiconv
ICONVDIR = $(ROOTPATH)/libiconv-1.16
#ICONVPREFIX = $(ROOTPATH)/libiconv_install
ICONVPREFIX = $(GLIB_PRLIB)

FFI = libffi
FFIDIR = $(ROOTPATH)/libffi-3.2.1
#FFIPREFIX = $(ROOTPATH)/libffi_install
FFIPREFIX = $(GLIB_PRLIB)

GETTEXT = gettext
GETTEXTDIR = $(ROOTPATH)/gettext-0.19.7
GETTEXTDIR = $(ROOTPATH)/gettext-0.20.1
#GETTEXTPREFIX = $(ROOTPATH)/gettext_install
GETTEXTPREFIX = $(GLIB_PRLIB)

GDBUS=gdbus-test

_TARGET_LEVEL := 0
TARGET_ALL= $(DBUS) $(GLIB) install
TARGET_CLEAN = $(GLIB)-clean $(DBUS)-clean $(LXC)-clean

ifeq ($(strip $(_TARGET_LEVEL)), 1)
TARGET_ALL=$(LXC) install
TARGET_CLEAN = $(LXC)-clean
endif

ifneq ($(strip $(SUBTARGET)),)
TARGETS:=$(subst /, , $(SUBTARGET))
else
TARGETS:=install
endif

#all : $(DBUS) $(GLIB) install  $(LXC)
all :  $(TARGETS)
#http://download.chinaunix.net/download.php?id=28466&ResourceID=5757
#notices!!!!!! makefile.rules change gcc to mips-gcc
$(LIBCAP):
	cd $(LIBCAPDIR) &&  make clean all && make prefix=$(LIBCAPPREFIX) lib=lib install

#cd $(LXCDIR) && ./configure --host=$(HOST) --target=$(COMPILE_TOOLCHAIN) --prefix=$(LXCPREFIX) --exec-prefix=/usr \

$(LXC):
	cd $(LXCDIR) && ./configure --help
	#for lxc clean error
	- cp -rf lxc-Makefile.backup $(LXCDIR)/config/etc/Makefile.in
	cd $(LXCDIR) && ./autogen.sh && ./configure --help
	if [ -f $(LXCDIR)/_is_configure ] ; then \
	echo "already configred..... "; \
	else \
	cd $(LXCDIR) && ./configure --host=$(HOST) --target=$(COMPILE_TOOLCHAIN) --prefix=$(LXCPREFIX) \
	--disable-apparmor \
	--disable-doc \
	--disable-api-docs \
	--disable-selinux \
	--disable-cgmanager \
	--disable-examples \
	--disable-python \
	--disable-bash \
	--disable-lua \
	--enable-capabilities=yes \
	--disable-seccomp \
	CFLAGS="-I$(LIBCAPPREFIX)/include/sys" \
	LDFLAGS="-L$(LIBCAPPREFIX)/lib" \
	LIBS="-lcap" \
	CC="$(CC)  -D__NR_setns=4344" ;\
	touch $(LXCDIR)/_is_configure; \
	fi
	
	cd $(LXCDIR) && make
#&& make install
	/bin/mkdir -p $(LXCBIN)/usr/bin
	/bin/mkdir -p $(LXCBIN)/usr/lib
	/bin/mkdir -p $(LXCBIN)/usr/libexec/lxc
	/bin/mkdir -p $(LXCBIN)/usr/sbin
	/bin/mkdir -p $(LXCBIN)/usr/include/lxc
	/bin/mkdir -p $(LXCBIN)/usr/share/lxc/templates
	/bin/mkdir -p $(LXCBIN)/usr/lib/lxc/rootfs
	/bin/mkdir -p $(LXCBIN)/usr/share/lxc/hooks
	/bin/mkdir -p $(LXCBIN)/usr/var/lib/lxc
	/bin/mkdir -p $(LXCBIN)/usr/var/cache/lxc
	/bin/mkdir -p $(LXCBIN)/usr/lib/pkgconfig
	/bin/mkdir -p $(LXCBIN)/usr/etc/lxc
	/bin/mkdir -p $(LXCBIN)/usr/etc/rc.d/init.d
	/bin/mkdir -p $(LXCBIN)/usr/share/lxc/config
	/bin/mkdir -p $(LXCBIN)/usr/share/lxc
	/bin/mkdir -p $(LXCBIN)/usr/share/lxc/selinux
	
	cd $(LXCDIR)/src/lxc && /usr/bin/install  -c lxc-attach lxc-autostart lxc-cgroup lxc-clone lxc-config lxc-console lxc-create lxc-destroy lxc-execute lxc-freeze lxc-info lxc-monitor lxc-snapshot lxc-start lxc-stop lxc-unfreeze lxc-unshare lxc-usernsexec lxc-wait  $(LXCBIN)/usr/bin
	cd $(LXCDIR)/src/lxc &&	install -c -m 644 lxc.functions $(LXCBIN)/usr/share/lxc	
	cd $(LXCDIR)/src/lxc &&	/usr/bin/install -c lxc-checkconfig legacy/lxc-ls $(LXCBIN)/usr/bin	
	cd $(LXCDIR)/src/lxc && mv liblxc.so liblxc.so.1.0.8 && ln -sf liblxc.so.1.0.8 liblxc.so.1 && ln -sf liblxc.so.1 liblxc.so && cp -rf liblxc.so liblxc.so.1.0.8 liblxc.so.1 $(LXCBIN)/usr/lib -d	
	cd $(LXCDIR)/src/lxc &&	/usr/bin/install -c lxc-monitord lxc-user-nic $(LXCBIN)/usr/libexec/lxc	
	cd $(LXCDIR)/src/lxc &&	/usr/bin/install -c init.lxc $(LXCBIN)/usr/sbin
	cd $(LXCDIR)/src/lxc &&	/usr/bin/install -c -m 644 attach_options.h lxccontainer.h version.h $(LXCBIN)/usr/include/lxc	
	cd $(LXCDIR)/templates &&	/usr/bin/install -c lxc-alpine lxc-altlinux lxc-archlinux lxc-busybox lxc-centos lxc-cirros lxc-debian lxc-download lxc-fedora lxc-gentoo lxc-openmandriva lxc-opensuse lxc-oracle lxc-plamo lxc-sshd lxc-ubuntu lxc-ubuntu-cloud $(LXCBIN)/usr/share/lxc/templates	
	cd $(LXCDIR)/doc/rootfs && /usr/bin/install -c -m 644 README $(LXCBIN)/usr/lib/lxc/rootfs
	cd $(LXCDIR)/hooks/ && /usr/bin/install -c clonehostname mountecryptfsroot ubuntu-cloud-prep squid-deb-proxy-client $(LXCBIN)/usr/share/lxc/hooks
	cd $(LXCDIR) && /usr/bin/install -c -m 644 lxc.pc $(LXCBIN)/usr/lib/pkgconfig	
	cd $(LXCDIR)/config/etc && /usr/bin/install -c -m 644 default.conf $(LXCBIN)/usr/etc/lxc
	cd $(LXCDIR)/config/init/sysvinit/ && /usr/bin/install -c lxc $(LXCBIN)/usr/etc/rc.d/init.d/lxc	
	cd $(LXCDIR)/config/templates && /usr/bin/install -c -m 644 centos.common.conf centos.userns.conf common.seccomp debian.common.conf debian.userns.conf fedora.common.conf fedora.userns.conf gentoo.common.conf gentoo.moresecure.conf gentoo.userns.conf oracle.common.conf oracle.userns.conf plamo.common.conf plamo.userns.conf ubuntu-cloud.common.conf ubuntu-cloud.lucid.conf ubuntu-cloud.userns.conf ubuntu.common.conf ubuntu.lucid.conf ubuntu.userns.conf $(LXCBIN)/usr/share/lxc/config	
	cd $(LXCDIR)/config/yum/ && /usr/bin/install -c -m 644 lxc-patch.py $(LXCBIN)/usr/share/lxc	
	cd $(LXCDIR)/config/selinux && /usr/bin/install -c -m 644 lxc.if lxc.te $(LXCBIN)/usr/share/lxc/selinux	
	
	cp $(LIBCAPPREFIX)/lib/libcap.so $(LXCBIN)/usr/lib -d
	cp $(LIBCAPPREFIX)/lib/libcap.so.2 $(LXCBIN)/usr/lib -d
	cp $(LIBCAPPREFIX)/lib/libcap.so.2.17 $(LXCBIN)/usr/lib -d

$(EXPAT):
	echo "test"
	cd $(EXPATDIR) && ./configure --help
	if [ -f $(EXPATDIR)/_is_configure ] ; then \
	echo "already configred..... "; \
	else \
	cd $(EXPATDIR) && ./configure --host=$(HOST) --target=$(COMPILE_TOOLCHAIN) --prefix=$(EXPATPREFIX) && make && make install; \
	touch $(EXPATDIR)/_is_configure; \
	fi

$(ICONV):
	cd $(ICONVDIR) && ./configure --help
	if [ -f $(ICONVDIR)/_is_configure ] ; then \
	echo "already configred..... "; \
	else \
	cd $(ICONVDIR) && ./configure --host=$(HOST) --target=$(COMPILE_TOOLCHAIN) --prefix=$(ICONVPREFIX) --disable-tests && make && make install; \
	touch $(ICONVDIR)/_is_configure; \
	fi
	cd $(ICONVDIR) && make install
	
$(LIBPCRE):
	cd $(LIBPCREDIR) && ./configure --help
	if [ -f $(LIBPCREDIR)/_is_configure ] ; then \
	echo "already configred..... "; \
	else \
	cd $(LIBPCREDIR) && ./configure --host=$(HOST) --target=$(COMPILE_TOOLCHAIN) --prefix=$(ICONVPREFIX) && make && make install; \
	touch $(LIBPCREDIR)/_is_configure; \
	fi	
	cd $(LIBPCREDIR) && make install

$(ZLIB):
	#cd $(ZLIBDIR) && ./configure --help
	if [ -f $(ZLIBDIR)/_is_configure ] ; then \
	echo "already configred..... "; \
	else \
	cd $(ZLIBDIR) && ./configure --prefix=$(ZLIBPREFIX) && make && make install; \
	touch $(ZLIBDIR)/_is_configure; \
	fi
	cd $(ZLIBDIR) && make install
#notice add COMPILE_TOOLCHAIN0 for glib toolchain
$(FFI):
	cd $(FFIDIR) && ./configure --help
	if [ -f $(FFIDIR)/_is_configure ] ; then \
	echo "already configred..... "; \
	else \
	cd $(FFIDIR) && ./configure --host=$(HOST) --target=$(COMPILE_TOOLCHAIN0) --prefix=$(FFIPREFIX) --includedir=$(FFIPREFIX)/include && make && make install; \
	touch $(FFIDIR)/_is_configure; \
	fi
	cd $(FFIDIR) && make install

$(GETTEXT):
	#cd $(GETTEXTDIR) && libtoolize --automake --copy --debug --force && aclocal && autoconf && ./configure --help
	#notice!!!!!!:  need disable doc in gettext-tools/makefile.in 
	cd $(GETTEXTDIR) && ./configure --help
	if [ -f $(GETTEXTDIR)/_is_configure ] ; then \
	echo "already configred..... "; \
	else \
	echo test~~~~~~~~~~~~~~~~~~~~~~~~~~~~; \
	cd $(GETTEXTDIR) && ./configure --host=$(HOST) --target=$(COMPILE_TOOLCHAIN) --prefix=$(GETTEXTPREFIX) \
	CFLAGS="-I$(ICONVPREFIX)/include " \
	LDFLAGS="-L$(ICONVPREFIX)/lib " ;\
	touch $(GETTEXTDIR)/_is_configure; \
	cd $(GETTEXTDIR)  && make && make install; \
	fi
	cd $(GETTEXTDIR) && make install

#--with-dbus-user=root \
	#-disable-tests \
	#--prefix=/usr \
	#--bindir=/usr/bin \
	#--sbindir=/usr/sbin \
	#--libdir=/usr/lib \
	#install
	
dbus2:
	cd $(DBUSDIR2) && ./configure --help
	if [ -f $(DBUSDIR2)/_is_configure ] ; then \
	echo "already configred..... "; \
	else \
	cd $(DBUSDIR2) && ./configure --host=$(HOST) --target=$(COMPILE_TOOLCHAIN) \
	--prefix=$(DBUSPREFIX) \
	--build=x86_64-linux-gnu --program-prefix= --program-suffix=  --sysconfdir=/etc \
	--localstatedir=/var --mandir=/usr/man --infodir=/usr/info --disable-nls --enable-shared --enable-static \
	--disable-abstract-sockets --disable-ansi --disable-asserts --disable-console-owner-file --disable-doxygen-docs --disable-compiler_coverage \
	--disable-selinux --disable-tests --enable-verbose-mode --disable-xml-docs --with-xml="expat" --with-dbus-user=CUAdmin \
	--with-dbus-daemondir=/usr/sbin --with-system-socket=/var/run/dbus/system_bus_socket --with-system-pid-file=/var/run/dbus.pid \
	--without-x --datadir=/etc --libexecdir=/usr/lib/dbus-1 \
	CFLAGS="-I$(EXPATPREFIX)/include" \
	LDFLAGS="-L$(EXPATPREFIX)/lib"; \
	touch $(DBUSDIR2)/_is_configure; \
	fi
	#--bindir=/usr/bin 
		cd $(DBUSDIR2) && make && ! make install
		
#expat-clean/expat/dbus2-clean/dbus2/dbus-install		
dbus-install:
	- rm -rf $(DBUSBIN)/
	- mkdir -p $(DBUSBIN)/usr/sbin/ 
	- mkdir -p $(DBUSBIN)/usr/bin/ 
	- mkdir -p $(DBUSBIN)/lib/
	- mkdir -p $(DBUSBIN)/usr/lib/dbus-1/
	- mkdir -p $(DBUSBIN)/usr/etc/dbus-1/system.d/
	- mkdir -p $(DBUSBIN)/usr/etc/dbus-1/session.d/
	
	cp  $(DBUSDIR2)/bus/dbus-daemon  $(DBUSBIN)/usr/sbin/ 
	cp  $(DBUSDIR2)/bus/dbus-daemon-launch-helper  $(DBUSBIN)/usr/lib/dbus-1 
	cp  $(DBUSDIR2)/tools/.libs/dbus-uuidgen  $(DBUSBIN)/usr/sbin/  
	cp  $(DBUSDIR2)/tools/dbus-launch  $(DBUSBIN)/usr/sbin/ 
	cp  $(DBUSDIR2)/tools/.libs/dbus-run-session  $(DBUSBIN)/usr/sbin/
	
	cp  $(DBUSDIR2)/tools/dbus-cleanup-sockets  $(DBUSBIN)/usr/bin/ 
	cp  $(DBUSDIR2)/tools/.libs/dbus-send  $(DBUSBIN)/usr/bin/
	cp  $(DBUSDIR2)/tools/.libs/dbus-monitor  $(DBUSBIN)/usr/bin/
	
	cp  $(DBUSDIR2)/dbus/.libs/libdbus-1.so  $(DBUSBIN)/lib/ 
	cp  $(DBUSDIR2)/dbus/.libs/libdbus-1.so.*  $(DBUSBIN)/lib/
	cp  $(DBUSDIR2)/bus/*.conf  $(DBUSBIN)/usr/etc/dbus-1/
	cp -rf $(DBUS_CONF)/* $(DBUSBIN)/usr/etc/
	#expat lib
	cp $(EXPATPREFIX)/lib/*.so  $(DBUSBIN)/lib/
	cp $(EXPATPREFIX)/lib/*.so.*  $(DBUSBIN)/lib/	
	
dbus2-clean:
	- rm -rf $(DBUSBIN)/
	- make -C $(DBUSDIR2) clean 
	- make -C $(DBUSDIR2) distclean
	- $(RM) $(DBUSDIR2)/_is_configure	
	
$(DBUS): $(EXPAT)
	cd $(DBUSDIR2) && ./configure --help
	if [ -f $(DBUSDIR)/_is_configurex ] ; then \
	echo "already configred..... "; \
	else \
	cd $(DBUSDIR2) && ./configure --host=$(HOST) --target=$(COMPILE_TOOLCHAIN) \
	--prefix=$(DBUSPREFIX) \
	--enable-shared --enable-static \
	--disable-abstract-sockets \
	--disable-ansi \
	--disable-asserts \
	--disable-console-owner-file \
	--disable-doxygen-docs \
	--disable-compiler_coverage \
	--disable-selinux \
	--disable-tests \
	--enable-verbose-mode \
	--disable-xml-docs \
	--with-xml="expat" \
	--with-dbus-user=root \
	--with-dbus-daemondir="/usr/sbin" \
	--with-system-socket="/var/run/dbus/system_bus_socket" \
	--with-system-pid-file="/var/run/dbus.pid" \
	--without-x \
	--libexecdir=/usr/lib/dbus-1 \
	--localstatedir=/var \
	--sysconfdir=/var/run/dbus \
	CFLAGS="-I$(EXPATPREFIX)/include" \
	LDFLAGS="-L$(EXPATPREFIX)/lib"; \
	touch $(DBUSDIR2)/_is_configure; \
	fi
	cd $(DBUSDIR2) && make && ! make install
	- rm -rf $(DBUSBIN)/
	- mkdir -p $(DBUSBIN)/usr/sbin/ 
	- mkdir -p $(DBUSBIN)/usr/bin/ 
	- mkdir -p $(DBUSBIN)/lib/
	- mkdir -p $(DBUSBIN)/usr/lib/dbus-1/
	- mkdir -p $(DBUSBIN)/usr/etc/dbus-1/system.d/
	- mkdir -p $(DBUSBIN)/usr/etc/dbus-1/session.d/
	
	cp  $(DBUSDIR)/bus/dbus-daemon  $(DBUSBIN)/usr/sbin/ 
	cp  $(DBUSDIR)/bus/dbus-daemon-launch-helper  $(DBUSBIN)/usr/lib/dbus-1 
	cp  $(DBUSDIR)/tools/.libs/dbus-uuidgen  $(DBUSBIN)/usr/sbin/  
	cp  $(DBUSDIR)/tools/dbus-launch  $(DBUSBIN)/usr/sbin/ 
	cp  $(DBUSDIR)/tools/.libs/dbus-run-session  $(DBUSBIN)/usr/sbin/
	
	cp  $(DBUSDIR)/tools/dbus-cleanup-sockets  $(DBUSBIN)/usr/bin/ 
	cp  $(DBUSDIR)/tools/.libs/dbus-send  $(DBUSBIN)/usr/bin/
	cp  $(DBUSDIR)/tools/.libs/dbus-monitor  $(DBUSBIN)/usr/bin/
	
	cp  $(DBUSDIR)/dbus/.libs/libdbus-1.so  $(DBUSBIN)/lib/ 
	cp  $(DBUSDIR)/dbus/.libs/libdbus-1.so.*  $(DBUSBIN)/lib/
	cp  $(DBUSDIR)/bus/*.conf  $(DBUSBIN)/usr/etc/dbus-1/
	cp -rf $(DBUS_CONF)/* $(DBUSBIN)/usr/etc/
	#expat lib
	cp $(EXPATPREFIX)/lib/*.so  $(DBUSBIN)/lib/
	cp $(EXPATPREFIX)/lib/*.so.*  $(DBUSBIN)/lib/
	
#aptitude install libglib2.0-dev
#copy libconfig.h from /lib/glib-2.0/include ###############
#$(GLIB): $(ICONV) $(ZLIB) $(FFI) $(GETTEXT) 
$(GLIB):
	cd $(GLIBDIR) && ./configure --help=recursive 
	if [ -f $(GLIBDIR)/_is_configure ] ; then \
	echo "already configred..... "; \
	else \
	echo ac_cv_type_long_long=ll>$(GLIBCACHEFILE); \
	echo glib_cv_stack_grows=no>>$(GLIBCACHEFILE); \
	echo glib_cv_uscore=no>>$(GLIBCACHEFILE); \
	echo ac_cv_func_posix_getpwuid_r=yes>>$(GLIBCACHEFILE); \
	echo ac_cv_func_posix_getgrgid_r=yes>>$(GLIBCACHEFILE); \
	echo ac_cv_path_GLIB_COMPILE_SCHEMAS=yes>>$(GLIBCACHEFILE); \
	echo ac_cv_path_GLIB_COMPILE_RESOURCES=yes>>$(GLIBCACHEFILE); \
	cd $(GLIBDIR) && CFLAGS="-I$(ICONVPREFIX)/include" \
	LDFLAGS="-L$(ICONVPREFIX)/lib -lffi -liconv -lintl" \
	ZLIB_CFLAGS="-I$(ZLIBPREFIX)/include" \
	ZLIB_LIBS="-L$(ZLIBPREFIX)/lib -lz" \
	LIBFFI_CFLAGS="-I$(FFIPREFIX)/lib/libffi-3.2.1/include" \
	LIBFFI_LIBS="-L$(FFIPREFIX)/lib -lffi" \
	./configure --host=$(HOST) --target=$(COMPILE_TOOLCHAIN) \
	--prefix=$(GLIBPREFIX) \
	--cache-file=$(GLIBCACHEFILE) \
	--enable-shared \
	--enable-static \
	--enable-debug=yes \
	--disable-selinux \
	--enable-iconv=no --with-libiconv=gnu \
	--disable-fam ; \
	touch $(GLIBDIR)/_is_configure; \
	fi
	cd $(GLIBDIR) && make LDFLAGS="-L$(ICONVPREFIX)/lib -lffi -lintl -liconv" lIBS="-lffi -lintl -liconv" && make install
	#cd $(GLIBDIR) && make LDFLAGS="--rpath=$(ICONVPREFIX)/lib -L$(ICONVPREFIX)/lib -lffi -lintl -liconv" lIBS="-lffi -lintl -liconv" && make install
	#glib
	- mkdir -p $(GLIBBIN)/usr/bin
	- mkdir -p $(GLIBBIN)/usr/lib/glib-2.0
	- mkdir -p $(GLIBBIN)/lib/
	cp  -rf $(GLIBPREFIX)/bin/*  $(GLIBBIN)/usr/bin/ 
	cp  -rf $(GLIBPREFIX)/lib/*.so  $(GLIBBIN)/usr/lib/glib-2.0/
	cp  -rf $(GLIBPREFIX)/lib/*.so.*  $(GLIBBIN)/usr/lib/glib-2.0/
	#libs
	cp  -rf $(GLIB_PRLIB)/lib/*.so  $(GLIBBIN)/lib/
	cp  -rf $(GLIB_PRLIB)/lib/*.so.*  $(GLIBBIN)/lib/
#config from CUC, used now
glib3:
	cd $(GLIBDIR2) && ./configure --help=recursive 
	if [ -f $(GLIBDIR2)/_is_configure ] ; then \
	echo "already configred..... "; \
	else \
	echo ac_cv_type_long_long=ll>$(GLIB2CACHEFILE); \
	echo glib_cv_stack_grows=no>>$(GLIB2CACHEFILE); \
	echo glib_cv_uscore=no>>$(GLIB2CACHEFILE); \
	echo ac_cv_func_posix_getpwuid_r=yes>>$(GLIB2CACHEFILE); \
	echo ac_cv_func_posix_getgrgid_r=yes>>$(GLIB2CACHEFILE); \
	echo ac_cv_path_GLIB_COMPILE_SCHEMAS=yes>>$(GLIB2CACHEFILE); \
	echo ac_cv_path_GLIB_COMPILE_RESOURCES=yes>>$(GLIB2CACHEFILE); \
	cd $(GLIBDIR2) && CFLAGS="-I$(ICONVPREFIX)/include" \
	LDFLAGS="-L$(ICONVPREFIX)/lib -lffi -liconv -lintl" \
	ZLIB_CFLAGS="-I$(ZLIBPREFIX)/include" \
	ZLIB_LIBS="-L$(ZLIBPREFIX)/lib -lz" \
	LIBFFI_CFLAGS="-I$(FFIPREFIX)/lib/libffi-3.2.1/include" \
	LIBFFI_LIBS="-L$(FFIPREFIX)/lib -lffi" \
	./configure --host=$(HOST) --target=$(COMPILE_TOOLCHAIN) --build=x86_64-linux-gnu --program-prefix= --program-suffix= --prefix=$(GLIBPREFIX) \
	--cache-file=$(GLIB2CACHEFILE) \
	--disable-nls --enable-shared --enable-static --enable-debug=no --disable-selinux --enable-libmount=no --disable-fam --with-libiconv=gnu --with-pcre=internal; \
	touch $(GLIBDIR2)/_is_configure; \
	fi
	cd $(GLIBDIR2) && make LDFLAGS="-L$(ICONVPREFIX)/lib -lffi -lintl -liconv" lIBS="-lffi -lintl -liconv" && make install

#notice: need compile libpcre first	
glib2:
	cd $(GLIBDIR2) && ./configure --help=recursive 
	if [ -f $(GLIBDIR2)/_is_configure ] ; then \
	echo "already configred..... "; \
	else \
	echo ac_cv_type_long_long=ll>$(GLIBCACHEFILE); \
	echo glib_cv_stack_grows=no>>$(GLIBCACHEFILE); \
	echo glib_cv_uscore=no>>$(GLIBCACHEFILE); \
	echo ac_cv_func_posix_getpwuid_r=yes>>$(GLIBCACHEFILE); \
	echo ac_cv_func_posix_getgrgid_r=yes>>$(GLIBCACHEFILE); \
	echo ac_cv_path_GLIB_COMPILE_SCHEMAS=yes>>$(GLIBCACHEFILE); \
	echo ac_cv_path_GLIB_COMPILE_RESOURCES=yes>>$(GLIBCACHEFILE); \
	cd $(GLIBDIR2) && CFLAGS="-I$(ICONVPREFIX)/include" \
	LDFLAGS="-L$(ICONVPREFIX)/lib -lffi -liconv -lintl -lpcre" \
	ZLIB_CFLAGS="-I$(ZLIBPREFIX)/include" \
	ZLIB_LIBS="-L$(ZLIBPREFIX)/lib -lz" \
	LIBFFI_CFLAGS="-I$(FFIPREFIX)/lib/libffi-3.2.1/include" \
	LIBFFI_LIBS="-L$(FFIPREFIX)/lib -lffi" \
	PCRE_CFLAGS="-I$(GLIB_PRLIB)/include" \
	PCRE_LIBS="-L$(GLIB_PRLIB)/lib -lpcre" \
	./configure --host=$(HOST) --target=$(COMPILE_TOOLCHAIN0) \
	--prefix=$(GLIBPREFIX) \
	--cache-file=$(GLIBCACHEFILE) \
	--enable-shared \
	--enable-static \
	--enable-debug=no \
	--disable-gtk-doc \
	--disable-man \
	--disable-selinux \
	--enable-libmount=no \
	--enable-iconv=no --with-libiconv=gnu \
	--disable-fam ; \
	touch $(GLIBDIR2)/_is_configure; \
	fi
	cd $(GLIBDIR2) && make LDFLAGS="-L$(ICONVPREFIX)/lib -lffi -lintl -liconv" lIBS="-lffi -lintl -liconv" && make install
	
#glib-prelib-clean/glib-prelib/libpcre-clean/libpcre/glib2-clean/glib3		
glib-install:
	- rm -rf $(GLIBBIN)/*
	- mkdir -p $(GLIBBIN)/usr/bin
	- mkdir -p $(GLIBBIN)/usr/lib/glib-2.0
	- mkdir -p $(GLIBBIN)/lib/
	cd $(GLIBDIR2) && make install
	cp  -rf $(GLIBPREFIX)/bin/*  $(GLIBBIN)/usr/bin/ 
	cp  -rf $(GLIBPREFIX)/lib/*.so*  $(GLIBBIN)/usr/lib/glib-2.0/
	#cp  -rf $(GLIBPREFIX)/lib/*.so.*  $(GLIBBIN)/usr/lib/glib-2.0/
	#libs
	cp  -rf $(GLIB_PRLIB)/lib/*.so*  $(GLIBBIN)/lib/
	#cp  -rf $(GLIB_PRLIB)/lib/*.so.*  $(GLIBBIN)/lib/
	$(STRIP) $(GLIBBIN)/lib/*
	$(STRIP) $(GLIBBIN)/usr/lib/glib-2.0/*
#	$(STRIP) $(GLIBBIN)/usr/bin/*
	
$(GLIB2)-clean:
	- rm -rf $(GLIBBIN)/
	- rm -rf $(GLIBPREFIX)/
	- make -C $(GLIBDIR2) clean 
	- make -C $(GLIBDIR2) distclean
	- $(RM) $(GLIBDIR2)/_is_configure

glib-prelib: $(ICONV) $(ZLIB) $(FFI) $(GETTEXT)
	echo test!!!!!!!!!!!
	
glib-prelib-clean: $(ICONV)-clean $(ZLIB)-clean $(FFI)-clean $(GETTEXT)-clean $(FFI)-clean $(LIBPCRE)-clean
	
install:
ifneq ($(strip $(DEBUG_LOCAL)),)
	cd $(GLIBBIN)/usr/lib && tar cvf  ~/workdir/tftpdir/glib.tar ./glib-2.0
else
	rm -rf $(DBUSBIN)/*
	cp -rf $(DBUSBIN_DIR)/* $(DBUSBIN)/
	#for dbus igd apps lib use
ifneq ($(strip $(TCSUPPORT_GLIBC_VER)),)
	rm -rf $(GLIBPREFIX)
	cp -rf $(GLIB_INSTALL) $(ROOTPATH)/
endif
	rm -rf $(GLIBBIN)
	cp -rf $(GLIBBIN_INSTALL) $(ROOTPATH)/
	cp -rf $(GLIBBIN)/* $(FILESYSTEM_DIR)/
	-mkdir -p $(DBUSBIN)/usr/etc/dbus-1/system.d/
	-mkdir -p $(DBUSBIN)/usr/etc/dbus-1/session.d/
ifeq ($(strip $(TCSUPPORT_GLIBC_VER)),)
	cp -rf $(DBUS_CONF)/* $(DBUSBIN)/usr/etc/
endif
	cp -rf $(DBUSBIN)/usr/etc/dbus-1 $(DBUSBIN)/usr/etc/dbus
	cp -rf $(DBUSBIN)/* $(FILESYSTEM_DIR)/
	rm -rf $(LXCBIN_INSTALL)/usr/include
	cp -rf $(LXCBIN_INSTALL)/* $(FILESYSTEM_DIR)/
ifeq ($(strip $(TCSUPPORT_GLIBC_VER)),)
	cp -rf $(SAFBIN_DIR)/* $(FILESYSTEM_DIR)/
endif
	###copy dbus from ct 
ifeq ($(strip $(DBUS_MODE)),)
ifeq ($(strip $(TCSUPPORT_GLIBC_VER)),)
	cp -rf $(DBUSBIN_CT)/* $(FILESYSTEM_DIR)/
endif
endif
	#cp -rf $(LXCBIN)/*  $(FILESYSTEM_DIR)/
endif
	
$(GDBUS):
	make -C $(ROOTPATH)/gdbus-daemon-1.0 clean all
	make -C $(ROOTPATH)/eventfd_test clean all
	make -C $(ROOTPATH)/gdbus-server-test clean all
	#make -C $(ROOTPATH)/gdbus-client-test clean all

#clean : $(GLIB)-clean $(DBUS)-clean

clean : $(TARGET_CLEAN)

$(LXC)-clean:
	- rm -rf $(LXCBIN)/
	- make -C $(LXCDIR) distclean
	- $(RM) $(LXCDIR)/_is_configure

#$(GLIB)-clean: $(FFI)-clean $(ICONV)-clean $(GETTEXT)-clean $(ZLIB)-clean
$(GLIB)-clean:
	- rm -rf $(GLIBBIN)/
	- make -C $(GLIBDIR) clean 
	- make -C $(GLIBDIR) distclean
	- $(RM) $(GLIBDIR)/_is_configure

$(ZLIB)-clean:
	- make -C $(ZLIBDIR) clean distclean
	- $(RM) $(ZLIBDIR)/_is_configure

$(GETTEXT)-clean:
	- make -C $(GETTEXTDIR) clean
	- make -C $(GETTEXTDIR) distclean
	- $(RM) $(GETTEXTDIR)/_is_configure

$(FFI)-clean:
	- make -C $(FFIDIR) clean distclean
	- $(RM) $(FFIDIR)/_is_configure

$(LIBPCRE)-clean :
	- make -C $(LIBPCREDIR) clean
	- make -C $(LIBPCREDIR) distclean
	- $(RM) $(LIBPCREDIR)/_is_configure

$(ICONV)-clean:
	- make -C $(ICONVDIR) clean distclean
	- $(RM) $(ICONVDIR)/_is_configure

#$(DBUS)-clean: $(EXPAT)-clean
$(DBUS)-clean:
	- rm -rf $(DBUSBIN)/
	- make -C $(DBUSDIR) clean 
	- make -C $(DBUSDIR) distclean
	- $(RM) $(DBUSDIR)/_is_configure

$(LIBCAP)-clean:
	- make -C $(LIBCAPDIR) clean 
	- make -C $(LIBCAPDIR) distclean
	- rm -rf $(LIBCAPPREFIX)/*
	- $(RM) $(LIBCAPDIR)/_is_configure

$(EXPAT)-clean:
	- make -C $(EXPATDIR) clean distclean
	- $(RM) $(EXPATDIR)/_is_configure

		
.PHONY : all $(DBUS) clean $(EXPAT)-clean

