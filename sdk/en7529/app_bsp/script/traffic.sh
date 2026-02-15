#!/bin/bash

PROFILE_CFG=/userfs/profile.cfg
if [ -f $PROFILE_CFG ] ; then
    . $PROFILE_CFG
fi

#local loopback interface
ifconfig lo 127.0.0.1
route add -net 127.0.0.0 netmask 255.255.0.0 lo

if [ "$TCSUPPORT_IS_FH_PON" != "" ] ;then
#lan port init
brctl addbr br0
brctl addif br0 eth0
brctl addif br0 eth1
brctl addif br0 eth2
if [ "$TCSUPPORT_NP" = "" ] ;then
brctl addif br0 eth3
fi

ifconfig eth 0.0.0.0
ifconfig eth up
ifconfig eth0 up
ifconfig eth1 up
ifconfig eth2 up
if [ "$TCSUPPORT_NP" = "" ] ;then
ifconfig eth3 up
fi

else
#lan port init
brctl addbr br0
brctl addif br0 eth0.1
brctl addif br0 eth0.2
brctl addif br0 eth0.3
brctl addif br0 eth0.4

ifconfig eth0 0.0.0.0
ifconfig eth0 up
ifconfig eth0.1 up
ifconfig eth0.2 up
ifconfig eth0.3 up
ifconfig eth0.4 up
echo 1 >/proc/tc3162/vport_enable
fi

ifconfig br0 192.168.1.1 broadcast 192.168.1.255 netmask 255.255.255.0 up

if [ "$TCSUPPORT_CPU_EN7580" != "" ] || [ "$TCSUPPORT_CPU_EN7527" != "" ] || [ "$TCSUPPORT_CPU_EN7528" != "" ] ;then
if [ "$TCSUPPORT_XPON_HAL_API_MCST" != "" ] ;then
echo "mtk xpon multicast BSP"
insmod /lib/modules/mtk_xpon_multicast.ko
fi
fi

if [ "$TCSUPPORT_IS_FH_PON" != "" ] ;then
insmod /lib/modules/fh_vlan.ko
fi
