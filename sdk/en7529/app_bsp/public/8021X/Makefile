ROOTPATH=$(shell pwd)
include $(BSP_ROOTDIR)/ecnt-function.mk

ifeq ($(CONFIG_SUPPORT_OPENWRT), y)
EXE:=8021xd

EXTRA_CFLAGS += -O2 -Wall -g $(CFLAGS)

# If you want to support multiple radius server for multiple bssid, add following line
EXTRA_CFLAGS +=  -DMULTIPLE_RADIUS=1

# If you want to debug daemon, add following line
EXTRA_CFLAGS +=  -DDBG=1

# If you want to support passpoint release 2 feature
EXTRA_CFLAGS +=  -DHOTSPOT_R2=1

OBJS =	rtdot1x.o eloop.o eapol_sm.o radius.o md5.o  \
	config.o ieee802_1x.o  \
	sta_info.o   radius_client.o

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(EXTRA_CFLAGS) -o $@ $(OBJS)

clean:
	-@rm -f *~ *.o $(EXE) *.d

$(OBJS): %.o : %.c
	$(CC) $(EXTRA_CFLAGS) -c $< -o $@

else

#PLATFORM = 5VT
#PLATFORM = PC
#PLATFORM = STAR
#PLATFORM = RT2880

ifeq ($(PLATFORM),RT2880)
CROSS_COMPILE = mipsel-linux-
endif

ifeq ($(PLATFORM),5VT)
CROSS_COMPILE = /opt/crosstool/uClibc_v5te_le_gcc_4_1_1/bin/arm-linux-
endif

ifeq ($(PLATFORM),STAR)
CROSS_COMPILE = /opt/star/tools/arm-linux/bin/arm-linux-
endif

ifeq ($(PLATFORM),PC)
CROSS_COMPILE =
endif

#CC = $(CROSS_COMPILE)gcc
#CC = $(CROSS)gcc

ifndef CFLAGS
CFLAGS = -O2 -Wall -g $(BSP_CFLAGS)
endif

# If you want to support multiple radius server for multiple bssid, add following line
CFLAGS +=  -DMULTIPLE_RADIUS

# If you want to debug daemon, add following line
CFLAGS +=  -DDBG 

# If you want to support passpoint release 2 feature
CFLAGS +=  -DHOTSPOT_R2

OBJS =	rtdot1x.o eloop.o eapol_sm.o radius.o md5.o  \
	config.o ieee802_1x.o  \
	sta_info.o   radius_client.o

all: rtdot1xd install

rtdot1xd: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

install:
	$(call BspAppUserFsBinInstall, $(ROOTPATH), rtdot1xd)		

clean:
	rm -f core *~ *.o $@ *.d
	$(call BspAppUserFsBinClean, rtdot1xd)	

#
# add by YYHuang@Ralink
romfs:
	$(ROMFSINST) 8021xd /bin/8021xd
	ln -sf 8021xd $(ROMFSDIR)/bin/rtinicapd
	ln -sf 8021xd $(ROMFSDIR)/bin/rtwifi3apd	
	ln -sf 8021xd $(ROMFSDIR)/bin/8021xd_x

-include $(OBJS:%.o=%.d)

endif
