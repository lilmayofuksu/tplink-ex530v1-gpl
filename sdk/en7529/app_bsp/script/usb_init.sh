#!/bin/bash

PROFILE_CFG=/userfs/profile.cfg
if [ -f $PROFILE_CFG ] ; then
    . $PROFILE_CFG
fi


#load driver
if [ "$TCSUPPORT_USBHOST" != "" ]; then
echo "insmod USB Storage driver"
insmod /lib/modules/usbhost/scsi_mod.ko
insmod /lib/modules/usbhost/sd_mod.ko
insmod /lib/modules/usbhost/nls_utf8.ko
insmod /lib/modules/usbhost/nls_cp936.ko
insmod /lib/modules/usbhost/usb-storage.ko
fi

