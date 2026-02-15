#CROSS_COMPILE=mips-linux-
#CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
LD=$(CROSS_COMPILE)ld
STRIP=$(CROSS_COMPILE)strip
RANLIB=$(CROSS_COMPILE)ranlib
SIZE=$(CROSS_COMPILE)size



#CFLAGS = -mips1 -msoft-float -O2 -fomit-frame-pointer
CFLAGS = -fomit-frame-pointer

ifneq ($(strip $(TCSUPPORT_CODE_SHRINK)),)
CFLAGS += -Os
else
CFLAGS += -O2
endif

EXEC = iperf3
OBJS = cjson.o iperf_api.o iperf_client_api.o iperf_error.o iperf_locale.o iperf_sctp.o iperf_server_api.o iperf_tcp.o iperf_udp.o  iperf_util.o main.o net.o tcp_info.o tcp_window_size.o timer.o units.o 

all: $(EXEC)

.c.o:
	$(CC) -c $(CFLAGS) $<

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -lm -o $@
	$(STRIP) -s $@ 

clean:
	-rm -f $(EXEC) *.elf *.o

