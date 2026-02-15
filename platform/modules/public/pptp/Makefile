ifeq ($(INCLUDE_TTNET), y)
	CFLAGS_pptp.o += -DINCLUDE_TTNET
endif

obj-m += pptp.o
default:
	make -C $(KERNELPATH) LANG=C M=`pwd` modules

clean:
	-rm -f *.o *.ko .*.cmd .*.flags *.mod.c
