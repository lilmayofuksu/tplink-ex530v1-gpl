#
#Configured directories
#
#prefix	=	/usr/local
#exec_prefix	=	/usr/local
#libdir	=	$(exec_prefix)/lib
libdir=	../lib
#publiclibdir= ../../public/lib
#
#Install commands
#
#INSTALL=	/usr/bin/install -c
#INSTALL_DIR	=	$(INSTALL) -d
#INSTALL_LIB	=	$(LIBTOOL)	$(INSTALL) -m 755

#CROSS=mips-linux-gnu-

#CC=$(CROSS)gcc -mips32r2 -msoft-float -muclibc
#AR=$(CROSS)ar
#LD=$(CROSS)ld 
STRIP=$(CROSS)strip
#RANLIB=$(CROSS)ranlib
#SIZE=$(CROSS)size
#LIBDIR=$(CROSS_PATH)/lib

DSOFLAGS=	-Wl,-soname,$(LIBNAME).so.1,-rpath,$(libdir) -shared $(OPTIM)
LN      =	/bin/ln -s
RM      =	/bin/rm -f
#OPTIM	=	-fPIC -Os -g

ENDIAN_FLAG=-EB
ENDIAN_CPPFLAG=-DEB
LIBNAME=libstrcmp
#INCLUDE = -I$(KERNEL_DIR)/include 

#CFLAGS= -mips1 -msoft-float 
CFLAGS = -mips32r2 -msoft-float -fPIC -O4 -g  -Wall# -flto
#LDFLAGS=-lpthread

.SUFFIXES:  .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<
	
LIBOBJS =       strcmp.o
LIBSTRCMP2=      $(LIBNAME).so.1.4
TARGETS =       $(LIBSTRCMP2)
SOFTLINK=		$(LIBNAME).so $(LIBNAME).so.1

all:  $(TARGETS)

$(LIBOBJS): %.o: %.S
	$(CC) -c $(CFLAGS) $<

$(LIBNAME).so.1.4: $(LIBOBJS)
	$(CC) $(LIBOBJS) $(CFLAGS) $(DSOFLAGS) $(ENDIAN_FLAG) $(ENDIAN_CPPFLAG) $(AS_TUNE_FLAGS) -D_ASSEMBLER_ -DMIPS32R2 -o $(LIBNAME).so.1.4 $(LDFLAGS)
	$(STRIP) --strip-unneeded $(LIBSTRCMP2)
	$(RM) $(LIBNAME).so $(LIBNAME).so.1
	$(LN) $(LIBNAME).so.1.4 $(LIBNAME).so
	$(LN) $(LIBNAME).so.1.4 $(LIBNAME).so.1

#install:
#	$(INSTALL_DIR) $(BUILDROOT)$(libdir)
#	$(INSTALL_LIB) $(LIBNAME).so.1.4 $(BUILDROOT)$(libdir)
#	$(RM) $(BUILDROOT)$(libdir)/$(LIBNAME).so
#	$(LN) $(LIBNAME).so.1.4 $(BUILDROOT)$(libdir)/$(LIBNAME).so
#	$(RM) $(BUILDROOT)$(libdir)/$(LIBNAME).so.1
#	$(LN) $(LIBNAME).so.1.4 $(BUILDROOT)$(libdir)/$(LIBNAME).so.1
#	cp $(LIBNAME).h $(BUILDROOT)$(libdir)
#	cp $(LIBNAME).h $(BUILDROOT)$(publiclibdir)
		
clean:
	$(RM) $(OBJS) $(TARGETS) $(SOFTLINK) $(LIBOBJS)
