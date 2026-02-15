#
# Makefile for the kernel USB device drivers.
#

# Object files in subdirectories

obj-$(CONFIG_USB)		+= core/
obj-$(CONFIG_USB_SUPPORT)	+= phy/

obj-$(CONFIG_USB_DWC3)		+= dwc3/
obj-$(CONFIG_USB_DWC2)		+= dwc2/
obj-$(CONFIG_USB_ISP1760)	+= isp1760/

obj-$(CONFIG_USB_MON)		+= mon/

ifneq ($(strip $(TCSUPPORT_MUH_TEST)),)
obj-$(CONFIG_USB_XHCI_HCD)  += host/mtk_test/
else
ifneq ($(strip $(TCSUPPORT_CPU_EN7580) $(TCSUPPORT_CPU_EN7528) $(TCSUPPORT_CPU_EN7523)),)
ifneq ($(strip $(TCSUPPORT_OPENWRT)),)
MTK_DR_M3U_HCD = ../usb/host/econet
else
MTK_DR_M3U_HCD = ../../../kernel_ext/drivers/usb
endif
obj-$(CONFIG_PCI)       += $(MTK_DR_M3U_HCD)/mu3h/
obj-$(CONFIG_USB_EHCI_HCD)  += $(MTK_DR_M3U_HCD)/mu3h/
obj-$(CONFIG_USB_ISP116X_HCD)   += $(MTK_DR_M3U_HCD)/mu3h/
obj-$(CONFIG_USB_OHCI_HCD)  += $(MTK_DR_M3U_HCD)/mu3h/
obj-$(CONFIG_USB_UHCI_HCD)  += $(MTK_DR_M3U_HCD)/mu3h/
obj-$(CONFIG_USB_FHCI_HCD)  += $(MTK_DR_M3U_HCD)/mu3h/
obj-$(CONFIG_USB_XHCI_HCD)  += $(MTK_DR_M3U_HCD)/mu3h/
obj-$(CONFIG_USB_SL811_HCD) += $(MTK_DR_M3U_HCD)/mu3h/
obj-$(CONFIG_USB_ISP1362_HCD)   += $(MTK_DR_M3U_HCD)/mu3h/
obj-$(CONFIG_USB_U132_HCD)  += $(MTK_DR_M3U_HCD)/mu3h/
obj-$(CONFIG_USB_R8A66597_HCD)  += $(MTK_DR_M3U_HCD)/mu3h/
obj-$(CONFIG_USB_HWA_HCD)   += $(MTK_DR_M3U_HCD)/mu3h/
obj-$(CONFIG_USB_ISP1760_HCD)   += $(MTK_DR_M3U_HCD)/mu3h/
obj-$(CONFIG_USB_IMX21_HCD) += $(MTK_DR_M3U_HCD)/mu3h/
obj-$(CONFIG_USB_FSL_MPH_DR_OF) += $(MTK_DR_M3U_HCD)/mu3h/
obj-$(CONFIG_USB_FUSBH200_HCD)  += $(MTK_DR_M3U_HCD)/mu3h/
obj-$(CONFIG_USB_FOTG210_HCD)   += $(MTK_DR_M3U_HCD)/mu3h/
obj-$(CONFIG_USB_MAX3421_HCD)   += $(MTK_DR_M3U_HCD)/mu3h/
obj-$(CONFIG_USB)       += $(MTK_DR_M3U_HCD)/mu3_phy/
else
obj-$(CONFIG_PCI)		+= host/
obj-$(CONFIG_USB_EHCI_HCD)	+= host/
obj-$(CONFIG_USB_ISP116X_HCD)	+= host/
obj-$(CONFIG_USB_OHCI_HCD)	+= host/
obj-$(CONFIG_USB_UHCI_HCD)	+= host/
obj-$(CONFIG_USB_FHCI_HCD)	+= host/
obj-$(CONFIG_USB_XHCI_HCD)	+= host/
obj-$(CONFIG_USB_SL811_HCD)	+= host/
obj-$(CONFIG_USB_ISP1362_HCD)	+= host/
obj-$(CONFIG_USB_U132_HCD)	+= host/
obj-$(CONFIG_USB_R8A66597_HCD)	+= host/
obj-$(CONFIG_USB_HWA_HCD)	+= host/
obj-$(CONFIG_USB_IMX21_HCD)	+= host/
obj-$(CONFIG_USB_FSL_MPH_DR_OF)	+= host/
obj-$(CONFIG_USB_FOTG210_HCD)	+= host/
obj-$(CONFIG_USB_MAX3421_HCD)	+= host/
endif
endif

obj-$(CONFIG_USB_C67X00_HCD)	+= c67x00/

obj-$(CONFIG_USB_WUSB)		+= wusbcore/

obj-$(CONFIG_USB_ACM)		+= class/
obj-$(CONFIG_USB_PRINTER)	+= class/
obj-$(CONFIG_USB_WDM)		+= class/
obj-$(CONFIG_USB_TMC)		+= class/

obj-$(CONFIG_USB_STORAGE)	+= storage/
obj-$(CONFIG_USB)		+= storage/

obj-$(CONFIG_USB_MDC800)	+= image/
obj-$(CONFIG_USB_MICROTEK)	+= image/

obj-$(CONFIG_USB_SERIAL)	+= serial/

obj-$(CONFIG_USB)		+= misc/
obj-$(CONFIG_EARLY_PRINTK_DBGP)	+= early/

obj-$(CONFIG_USB_ATM)		+= atm/
obj-$(CONFIG_USB_SPEEDTOUCH)	+= atm/

obj-$(CONFIG_USB_MUSB_HDRC)	+= musb/
obj-$(CONFIG_USB_CHIPIDEA)	+= chipidea/
obj-$(CONFIG_USB_RENESAS_USBHS)	+= renesas_usbhs/
obj-$(CONFIG_USB_GADGET)	+= gadget/

obj-$(CONFIG_USB_COMMON)	+= common/

obj-$(CONFIG_USBIP_CORE)	+= usbip/
