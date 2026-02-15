# SPDX-License-Identifier: GPL-2.0
# Copyright(c) 1999 - 2019 Intel Corporation.

###########################################################################
# Driver files
FAMILYC = 82571.c ich8lan.c 80003es2lan.c
FAMILYH = 82571.h ich8lan.h 80003es2lan.h

# core driver files
CFILES = netdev.c ethtool.c param.c $(FAMILYC) \
         mac.c nvm.c phy.c manage.c kcompat.c
HFILES = e1000.h hw.h regs.h defines.h \
         mac.h nvm.h phy.h manage.h $(FAMILYH) kcompat.h

DRIVER_NAME = e1000e
obj-m += $(DRIVER_NAME).o
$(DRIVER_NAME)-objs := $(CFILES:.c=.o)
