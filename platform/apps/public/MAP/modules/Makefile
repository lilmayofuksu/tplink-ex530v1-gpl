obj-m		+=	ivi.o
ivi-objs	:=	ivi_rule.o ivi_rule6.o ivi_map.o ivi_map_tcp.o ivi_xmit.o ivi_nf.o ivi_ioctl.o ivi_module.o
PWD		:=	$(shell pwd)
ifneq ($(strip $(INCLUDE_BROADCOM)),)
EXTRA_CFLAGS += -I$(KERNELPATH)/../bcmkernel/include 
endif
ifeq ($(strip $(SUPPLIER)),qca-12.1)
EXTRA_CFLAGS += -Wno-error=unused-variable
endif
all:
	$(MAKE) -C $(KERNELPATH) M=$(PWD) modules

clean:
	rm -rf *.ko *.o *.mod.c core Module.symvers Module.markers modules.order .*.cmd .tmp_versions
