ROOTPATH=$(shell pwd)


libdir=	../lib
privatelibdir= ../../private/lib

#for mjson lib
MJSONLIB = libmjson
MJSONSRCDIR = $(ROOTPATH)/json-1.5
MJSONLIBDIR = $(ROOTPATH)/
MJSONLIBINC = $(ROOTPATH)/mjson-1.5

EXEC = exec-proc
EXECDIR = $(ROOTPATH)/exec-process

ifneq ($(TC_CFLAGS),)
CFLAGS+=$(TC_CFLAGS)
endif

ifneq ($(COMPILEOPTION_CFLAGS), )
CFLAGS+=$(COMPILEOPTION_CFLAGS)
endif

ifneq ($(COMPILEOPTION_LDFLAGS), )
		TCAPILIB+=$(COMPILEOPTION_LDFLAGS)
endif

OBJDIR = ./
SRC = $(wildcard ./src/*.c ./parameter/src/*c)
OBJ = $(SRC:.c=.o)
RM = rm -rf
MV = mv -f
CP = cp -f

all : $(MJSONLIB) 

$(MJSONLIB):
	cd $(MJSONSRCDIR) &&\
	./configure  --prefix=$(ROOTPATH) --libdir=$(MJSONLIBDIR) --includedir=$(MJSONLIBDIR) --disable-static --enable-shared \
	--host=$(HOST) CFLAGS="-g -O2 -Wall $(ARCH_CFLAGS)" \
	&& make clean && make && make install


install:
	cp $(MJSONLIBINC)/json.h $(LIB_INSTALL_DIR)
	cp $(MJSONLIBINC)/json_helper.h $(LIB_INSTALL_DIR)
	
clean:
	- cd $(MJSONSRCDIR) && make && make clean
	- $(RM) $(MJSONLIBDIR)/libmjson.*
	- $(RM) $(MJSONLIBDIR)/libmjson-1.5.*
		
.PHONY : all $(EXEC) $(MOBILELIB) $(MJSONLIB) $(EXE) $(SERVEREXE) $(CLIENTEXE) $(PLUGINEXE) clean
