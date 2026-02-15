#
#Configured directories
#
#prefix	=	/usr/local
#exec_prefix	=	/usr/local
#libdir	=	$(exec_prefix)/lib
libdir=	../lib
privatelibdir= ../../private/lib
#
#Install commands
#
INSTALL=	/usr/bin/install -c
INSTALL_DIR	=	$(INSTALL) -d
INSTALL_LIB	=	$(LIBTOOL)	$(INSTALL) -m 755

#CROSS=mips-linux-gnu-

#CC=$(CROSS)gcc -mips32r2 -msoft-float -muclibc
AR=$(CROSS)ar
LD=$(CROSS)ld 
STRIP=$(CROSS)strip
RANLIB=$(CROSS)ranlib
SIZE=$(CROSS)size
LIBDIR=$(CROSS_PATH)/lib

DSOFLAGS=	-Wl,-soname,libecb3.so.1,-rpath,$(libdir) -shared $(OPTIM)
LN      =	/bin/ln -s
RM      =	/bin/rm -rf
OPTIM	=	-fPIC -Os -g



#CFLAGS= -mips1 -msoft-float 
CFLAGS +=-fPIC -Os -g  -Wall

ifneq ($(TC_CFLAGS), )
        CFLAGS+=$(TC_CFLAGS)
endif

LIBRARIES +=$(LIBINSTALL_LDFLAGS) $(TCAPI_LIB)

.SUFFIXES:  .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<

LIBOBJS =       md5.o ecb3_enc.o
LIBECB3=       libecb3.so.1.4
TARGETS =       $(LIBECB3)
SOFTLINK=		libecb3.so libecb3.so.1

all:  $(TARGETS)

$(LIBOBJS): ecb3_enc.h
ecb3_enc.o:ecb3_enc.h

libecb3.so.1.4: $(LIBOBJS)
	$(CC) $(CFLAGS) $(DSOFLAGS) -o libecb3.so.1.4 $(LIBOBJS) $(LIBRARIES)
	$(STRIP) --strip-unneeded $(LIBECB3)
	$(RM) libecb3.so libecb3.so.1
	$(LN) libecb3.so.1.4 libecb3.so
	$(LN) libecb3.so.1.4 libecb3.so.1

install:
	$(INSTALL_DIR)  $(LIB_INSTALL_DIR)
	$(RM) $(LIB_INSTALL_DIR)/libecb3.so
	cp -rf libecb3.so*  $(LIB_INSTALL_DIR)
	cp ecb3_enc.h  $(LIB_INSTALL_DIR)
		
clean:
	$(RM) $(OBJS) $(TARGETS) $(LIBOBJS) $(SOFTLINK)
	$(RM) $(LIB_INSTALL_DIR)/ecb3_enc.h 
	$(RM) $(LIB_INSTALL_DIR)/libecb3.so*
