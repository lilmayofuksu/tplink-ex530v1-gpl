#CROSS_COMPILE=mips-linux-
#CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
LD=$(CROSS_COMPILE)ld
STRIP=$(CROSS_COMPILE)strip
RANLIB=$(CROSS_COMPILE)ranlib
SIZE=$(CROSS_COMPILE)size

LIBS= -lecnt_env
LDFLAGS += $(BSPAPP_LDFLAG1)

#CFLAGS = -mips1 -msoft-float -O2 -fomit-frame-pointer
CFLAGS = -fomit-frame-pointer 
CFLAGS += -I$(shell pwd)/../../../app_bsp/private/API/ecnt_lib_env \
	-I$(shell pwd)/../../../app_bsp/private/API/ecnt_lib_env/include \
	-I$(shell pwd)/../../../app_bsp/private/API/ecnt_lib_env/arch/arm/include \
	-DUSE_HOSTCC

ifneq ($(strip $(TCSUPPORT_CODE_SHRINK)),)
CFLAGS += -Os
else
CFLAGS += -O2
endif

EXEC = showenv setenv
OBJS = fw_env_main.o

all: $(EXEC)

.c.o:
	$(CC) -c $(CFLAGS) $<

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) $(LIBS)
	$(STRIP) -s $@ 

clean:
	-rm -f $(EXEC) *.elf *.o

