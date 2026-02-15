#!/bin/bash

PROFILE_CFG=/userfs/profile.cfg
if [ -f $PROFILE_CFG ] ; then
    . $PROFILE_CFG
fi


if [ "$TCSUPPORT_VOIP" = "" ];then
if [ -f "/proc/tc3162/sys_is_1004k_support" ] ;then
/userfs/bin/irqCpuBind set qdma_lan0 3
else
echo 8 >/proc/irq/22/smp_affinity
fi
fi

if [ "$TCSUPPORT_DUAL_WLAN_MT7615E" != "" ]|| [ "$TCSUPPORT_WLAN_MT7615D" != "" ];then
if [ -f "/proc/tc3162/sys_is_1004k_support" ] ;then
/userfs/bin/irqCpuBind set pcie0 0
/userfs/bin/irqCpuBind set pcie1 1
else
echo 3 > /proc/irq/24/smp_affinity
echo 3 > /proc/irq/25/smp_affinity
fi
fi

if [ "$TCSUPPORT_DUAL_WLAN_RT5592_RT3593" != "" ] ;then
/userfs/bin/iwpriv rai0 set WirelessMode=10
/userfs/bin/iwpriv rai0 set Channel=157
/userfs/bin/iwpriv rai0 set SSID="Aband_AP"
if [ -f "/proc/tc3162/sys_is_1004k_support" ] ;then
/userfs/bin/irqCpuBind set pcie0 3
else
echo 8 >/proc/irq/24/smp_affinity
fi
if [ "$TCSUPPORT_BONDING" != "" ] ;then
echo 8 >/proc/irq/25/smp_affinity
else
if [ -f "/proc/tc3162/sys_is_1004k_support" ] ;then
/userfs/bin/irqCpuBind set pcie1 2
else
echo 4 >/proc/irq/25/smp_affinity
fi
fi
fi	

echo 1 > /proc/irq/24/smp_affinity
echo 2 > /proc/irq/40/smp_affinity
echo 4 > /proc/irq/41/smp_affinity
echo 8 > /proc/irq/42/smp_affinity

echo 1 > /proc/irq/25/smp_affinity
echo 2 > /proc/irq/43/smp_affinity
echo 4 > /proc/irq/44/smp_affinity
echo 8 > /proc/irq/45/smp_affinity

/userfs/bin/irqCpuBind set qdma_lan0 0
/userfs/bin/irqCpuBind set pcie0 0
/userfs/bin/irqCpuBind set pcie1 1
/userfs/bin/irqCpuBind set qdma_lan2 2
/userfs/bin/irqCpuBind set qdma_lan3 3

if [ "$TCSUPPORT_CPU_EN7516" != "" ] || [ "$TCSUPPORT_CPU_EN7527" != "" ] || [ "$TCSUPPORT_CPU_EN7528" != "" ] || [ "$TCSUPPORT_CPU_EN7580" != "" ] ;then
echo "TCSUPPORT_WLAN_SW_RPS"
insmod /lib/modules/sw_rps_for_wifi.ko
if [ "$TCSUPPORT_DUAL_WLAN_MT7612E" != "" ] ;then
echo "7612 sw rps"
echo 0 0 1 1 > /proc/tc3162/sw_rps
else
if [ "$TCSUPPORT_DUAL_WLAN_MT7613E" != "" ] ;then
/userfs/bin/irqCpuBind set qdma_lan0 1
/userfs/bin/irqCpuBind set pcie0 2
/userfs/bin/irqCpuBind set pcie1 2
/userfs/bin/irqCpuBind set qdma_lan2 0
/userfs/bin/irqCpuBind set qdma_lan3 3
echo "7613 sw rps"
echo 0 0 0 0 > /proc/tc3162/sw_rps
echo 1 0 1 1 > /proc/tc3162/sw_rps_2g
else
echo 0 0 0 0 > /proc/tc3162/sw_rps
echo 0 0 0 0 > /proc/tc3162/sw_rps_2g
fi
fi
fi