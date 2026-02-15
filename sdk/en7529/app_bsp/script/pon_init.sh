#!/bin/bash

PROFILE_CFG=/userfs/profile.cfg
if [ -f $PROFILE_CFG ] ; then
    . $PROFILE_CFG
fi

#for 7570 bob for epon/gpon
if [ -f "/userfs/7570_bob.conf" ] ;then
	cp -rf /userfs/7570_bob.conf /tmp/7570_bob.conf 
else
	/userfs/bin/mtd readflash /tmp/7570_bob.conf 160 1536 reservearea
fi

### for MT7520 modules install
if [ "$TCSUPPORT_CPU_MT7520" != "" ] || [ "$TCSUPPORT_CPU_EN7521" != "" ]  || [ "$TCSUPPORT_CPU_EN7523" != "" ] || [ "$TCSUPPORT_CPU_EN7580" != "" ] || [ "$TCSUPPORT_CPU_EN7527" != "" ] || [ "$TCSUPPORT_CPU_EN7528" != "" ];then
if [ "$TCSUPPORT_CPU_EN7580" != "" ] ;then
insmod /lib/modules/phy_10g.ko
else
insmod /lib/modules/phy.ko
fi
if [ "$TCSUPPORT_CPU_EN7580" != "" ] ;then
XPONMODE=`cat /proc/tc3162/sys_xpon_mode`
echo "$XPONMODE"
if [ "$XPONMODE" = "1" ] ;then
	insmod /lib/modules/xpon.ko
else
	insmod /lib/modules/xpon_10g.ko
if [ "TCSUPPORT_XPON_HAL_API_EXT" != "" ] ;then
	insmod /lib/modules/xpon_custom_fh.ko
fi
fi
        insmod /lib/modules/xpon_int.ko
else
insmod /lib/modules/xpon.ko
if [ "$TCSUPPORT_CPU_EN7523" != "" ] ;then
	insmod /lib/modules/xpon_int.ko
fi
fi

if [ "$TCSUPPORT_IS_FH_PON" != "" ] ;then
ifconfig pon0 up
else
ifconfig pon up
fi

if [ "$TCSUPPORT_WAN_GPON" != "" ] ;then
ifconfig omci up
fi
if [ "$TCSUPPORT_WAN_EPON" != "" ] ;then
ifconfig oam up
fi
if [ "$TCSUPPORT_GPON_MAPPING" != "" ] || [ "$TCSUPPORT_EPON_MAPPING" != "" ] ;then
insmod /lib/modules/xponmap.ko
fi
fi

insmod /lib/modules/gpon_flow.ko
if [ "$TCSUPPORT_IS_FH_PON" = "" ] ;then
#pon vlan
if [ "$TCSUPPORT_PON_VLAN" != "" ] ;then
insmod /lib/modules/ponvlan.ko
fi

#pon igmp snooping
if [ "$TCSUPPORT_XPON_IGMP" != "" ] ;then
insmod /lib/modules/xpon_igmp.ko
fi

#pon mac filter
if [ "$TCSUPPORT_PON_MAC_FILTER" != "" ] ;then
insmod /lib/modules/ponmacfilter.ko
fi
fi

if [ "$TCSUPPORT_CPU_EN7580" != "" ] ;then
echo "for en7580 delay 10 seconds !"
sleep 10
fi

#start gponmgr before omci startup, using for store omci capability!
if [ "$TCSUPPORT_WAN_GPON" != "" ] ;then
/userfs/bin/gponmgr idle &
fi

if [ "$TCSUPPORT_WAN_EPON" != "" ] ;then
/userfs/bin/eponmgr idle &
fi

