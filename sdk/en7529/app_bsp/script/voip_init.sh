#!/bin/bash

echo "Init VoIP"

PROFILE_CFG=/userfs/profile.cfg
if [ -f $PROFILE_CFG ] ; then
    . $PROFILE_CFG
fi

if [ "$TCSUPPORT_VOIP" != "" ] ;then
echo "MTK DSP support"
if [ -f /userfs/bin/voip_loader ] ;then
/userfs/bin/voip_loader
VOIP_LOADER_SUCCESS=yes
fi

if [ "$VOIP_LOADER_SUCCESS" != "yes" ] ;then
insmod /lib/modules/sys_mod.ko
insmod /lib/modules/DSPCore.ko
insmod /lib/modules/pcm1.ko
insmod /lib/modules/lec.ko
insmod /lib/modules/spi.ko
insmod /lib/modules/fxs3.ko  
insmod /lib/modules/slic3.ko type=ZSI slic="le9642" 
insmod /lib/modules/ksocket.ko
insmod /lib/modules/ortp.ko
insmod /lib/modules/acodec_x.ko
insmod /lib/modules/foip.ko
insmod /lib/modules/ovdsp.ko
fi #end of VOIP_LOADER_SUCCESS

. /usr/script/lib_voip
taskset -p 0x8 `pidof vtspr`

sleep 1
taskset -p 0x8 `pidof ORTP_TASK`
taskset -p 0x8 `pidof fxs_task`
taskset -p 0x8 `pidof slicint_task`
taskset -p 0x8 `pidof DSPProc`
taskset -p 0x8 `pidof DspDlTask`
taskset -p 0x8 `pidof DspUlTask`
taskset -p 0x8 `pidof cid_task`
taskset -p 0x8 `pidof pcmreinit_task`

SLICTYPE = `cat proc/fxs/slicType`
if ["$SLICTYPE" != "2"]; then
echo "1" >/proc/DSPCore/pcmreset
fi

fi
