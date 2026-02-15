#KERNELDIR := ../../../linux
ifeq ($(KERNELPATH), )
	KERNELPATH := ../../../linux-2.6.x
else

endif

obj-m += xt_pctl.o

all:
	$(MAKE) -C $(KERNELPATH) M=`pwd` modules
ifneq ($(STRIP_KERNEL),)
	$(STRIP_KERNEL) --strip-unneeded xt_pctl.ko
else
	$(STRIP) --strip-unneeded xt_pctl.ko
endif

clean:
	$(MAKE) -C $(KERNELPATH) M=`pwd` clean

