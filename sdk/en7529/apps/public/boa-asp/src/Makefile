#THIS MAKEFILE BUILD WEB SERVER TARGET WITH ASP 
# Generated automatically from Makefile.in by configure.
# $Id: //BBN_Linux/Branch/Branch_for_Rel_JOYME3_Turnkey_20180818/tclinux_phoenix/apps/public/boa-asp/src/Makefile#2 $

.SUFFIXES:
.SUFFIXES: .o .c
.PHONY: clean mrclean distclean depend all dist

GCC_FLAGS = -Wstrict-prototypes -Wpointer-arith -Wcast-align -Wcast-qual\
  -Wtraditional\
  -Wshadow\
  -Wconversion\
  -Waggregate-return\
  -Wmissing-prototypes\
  -Wnested-externs\
  -Wall \
  -Wundef -Wwrite-strings -Wredundant-decls -Winline


srcdir = .
VPATH = .:./../extras
LDFLAGS =  -lcrypt
LIBS = $(LIBINSTALL_LDFLAGS) -lcrypt -lmxml $(BLLIB_TRAFFIC)
CFLAGS = -pipe -Wall -I. -I./asp 
ifneq ($(TCSUPPORT_CT_JOYME4),)
LIBS += $(ECNT_EVENT_LIB)
endif

#CFLAGS+= -mips1 -G0 -msoft-float -O2 -fshort-wchar 
CFLAGS+= -g0 -fshort-wchar $(KERNELHEAD_CFLAGS) $(LIBINSTALL_LDFLAGS)
CFLAGS+= -DTRENDCHIP

ifeq ($(PURE_BRIDGE),1)
CFLAGS += -DPURE_BRIDGE
endif
ifeq ($(WITHVOIP),1)
CFLAGS += -DWITHVOIP
endif

ifneq ($(strip $(TCSUPPORT_CODE_SHRINK)),)
CFLAGS += -Os
else
CFLAGS += -O2
endif

ifneq ($(TC_CFLAGS), )
        CFLAGS+=$(TC_CFLAGS)
endif

CFLAGS+=$(LIBINSTALL_CFLAGS)

ifneq ($(TCSUPPORT_IPV6), )
	CFLAGS += -DINET6
endif

ifneq ($(TCSUPPORT_CHARSET_CHANGE),)
LIBS += $(ECNT_UTILITY_LIB)
endif

ifneq ($(TCSUPPORT_WEBSERVER_SSL), )
LIBS +=  -lmatrixssl
CFLAGS += -I../../matrixssl-1-8
endif
ifneq ($(TCSUPPORT_ECNT_MAP),)
LIBS += $(LIBINSTALL_LDFLAGS) -lmapd_interface_client
CFLAGS += $(LIBINSTALL_CFLAGS)
endif

#CROSS=mips-linux-gnu-
#CFLAGS+= -DBOA_DEBUG
#CFLAGS += -DFASCIST_LOGGING
#CC = $(CROSS)gcc -mips32r2 -msoft-float -muclibc
#CC = $(CROSS)gcc
CPP = $(CC) -E

# Change these if necessary

#if use tcapi_lib
#exec_prefix     =       /usr/local
#libdir  =       $(exec_prefix)/lib/
#TCAPILIB= -lm -L $(libdir) -ltcapi
#TCAPILIB= -lm -L ../../lib -ltcapi
TCAPILIB= $(TCAPI_LIB)

YACC = bison -y 
LEX = flex 

SOURCES = alias.c boa.c buffer.c cgi.c cgi_header.c config.c escape.c \
	get.c hash.c ip.c log.c mmap_cache.c pipe.c queue.c read.c hnapRpc.c \
	request.c response.c select.c signals.c util.c sublog.c hnapSoap.c \

ifneq ($(TCSUPPORT_CGNX), )
SOURCES += libcaptcha.c
endif
ifneq ($(TCSUPPORT_WEBSERVER_SSL), )
SOURCES += boa_ssl.c
endif

ifneq ($(TCSUPPORT_CT_JOYME2),)
SOURCES += md5.c ctc_auth_api.c ecnt_json_api.c
endif
ifneq ($(TCSUPPORT_CMCCV2)$(TCSUPPORT_ANDLINK),)
ifneq ($(TCSUPPORT_ANDLINK),)
SOURCES += cmcc_np_api.c
else
SOURCES += cmcc_itms_api.c
endif
endif
ifneq ($(TCSUPPORT_ECNT_MAP),)
SOURCES += mesh_getjson_api.c
endif

OBJS = y.tab.o lex.yy.o $(SOURCES:.c=.o) timestamp.o 
	
ASPOBJS= \
	asp/asp_handler.o\
	asp/asp.o\
	asp/gb-lex.o\
	asp/http-get-utils.o\
	asp/grammar.o\
	asp/parsers.o\
		
all: asp_file boa boa_indexer

mxml_lib:
	$(MAKE) -C $(APP_MXML_DIR) clean
	$(MAKE) -C $(APP_MXML_DIR)
	$(MAKE) -C $(APP_MXML_DIR) install

tcapi_lib:
	$(MAKE) -C $(APP_TCAPILIB_DIR) clean
	$(MAKE) -C $(APP_TCAPILIB_DIR)
	$(MAKE) -C $(APP_TCAPILIB_DIR) install

asp_file:
	-(cd asp && $(MAKE) $(MFLAGS))
	
boa:	$(OBJS) $(ASPOBJS)
ifeq ($(strip $(TCSUPPORT_GCC_VER)),493)
	$(CC)  -o $@ $^ $(LDFLAGS) $(LIBS) $(TCAPILIB)
else
	$(CC) -Wl -o $@ $^ $(LDFLAGS) $(LIBS) $(TCAPILIB)
endif
	$(STRIP) boa
	
boa_indexer:	index_dir.o escape.o   
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS) $(TCAPILIB)

clean:
	-(cd asp && $(MAKE) $(MFLAGS) clean)
	rm -f $(OBJS) boa core *~ boa_indexer index_dir.o  *.gdb 
# 	rm -f $(OBJS) boa core lex.yy.c y.tab.c y.tab.h *~ boa_indexer index_dir.o  *.gdb 
       
distclean:	mrclean

mrclean:	clean
	rm -f config.status config.cache config.h Makefile config.log

# parser dependencies

#y.tab.c y.tab.h:	boa_grammar.y
#	$(YACC) -d $^

#lex.yy.c:	boa_lexer.l
#	$(LEX) $^

# timestamp

timestamp.o:	$(SOURCES) boa_grammar.y boa_lexer.l

# depend stuff
.depend:
	$(CPP) -MM $(SOURCES) > .depend
        
depend:
	-rm -f .depend
	$(MAKE) .depend
        
include .depend

# tags
tags:	$(SOURCES)
	ctags -o tags $^ *.h

# dist
dist:
	$(MAKE) clean
	./makedist.sh
        
# object dump
boa.objdump:    boa
	objdump --disassemble-all --source boa > $@

