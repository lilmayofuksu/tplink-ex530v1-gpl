#
# Makefile for USB Host Lib
#

ifeq ($(CONFIG_USB_MU3H_DRV),y)
ifneq ($(strip $(TCSUPPORT_OPENWRT)),)
    EXTRA_CFLAGS += -Idrivers/usb/host/econet/mu3h/mu3h_drv 
else
    EXTRA_CFLAGS += -Idrivers/usb/mu3h/mu3h_drv 
endif
#    EXTRA_CFLAGS += -I$(srctree)/../kernel_ext/drivers/usb/mu3h/mu3h_drv 
#    EXTRA_CFLAGS += -I$(srctree)/../kernel_ext/drivers/usb/mu3h/mu3h_drv 
endif

ifeq ($(CONFIG_USB_MU3H_TEST_DRV),y)
#    EXTRA_CFLAGS += -Idrivers/usb/mu3h/mu3h_test_drv 
    EXTRA_CFLAGS += -I$(srctree)/../kernel_ext/drivers/usb/mu3h/mu3h_test_drv 
endif

#EXTRA_CFLAGS += -Idrivers/usb/mu3_phy
#EXTRA_CFLAGS += -I$(srctree)/../kernel_ext/drivers/usb/mu3_phy
ifneq ($(strip $(TCSUPPORT_OPENWRT)),)
EXTRA_CFLAGS += -Idrivers/usb/host/econet/mu3h/mu3h_drv 
EXTRA_CFLAGS += -Idrivers/usb/host/econet/mu3_phy
EXTRA_CFLAGS += -I../../mu3_phy
else
EXTRA_CFLAGS += -I../kernel_ext/drivers/usb/mu3h/mu3h_drv 
EXTRA_CFLAGS += -I../kernel_ext/drivers/usb/mu3_phy
EXTRA_CFLAGS += -I../../mu3_phy
endif
obj-$(CONFIG_USB_MU3H)	+= xhci-mtk.o xhci-mtk-power.o xhci-mtk-scheduler.o
#obj-$(CONFIG_USB_MU3H)	+= xhci-mtk.o xhci-mtk-power.o xhci-mtk-sch.o
