#!/bin/bash

PROFILE_CFG=/userfs/profile.cfg
if [ -f $PROFILE_CFG ] ; then
    . $PROFILE_CFG
fi

if [ "$TCSUPPORT_CPU_EN7512" != "" ] || [ "$TCSUPPORT_CPU_EN7521" != "" ] ;then
echo 8192 > /proc/net/skbmgr_driver_max_skb
echo 6000 > /proc/net/skbmgr_limit
echo 3072 > /proc/net/skbmgr_4k_limit
else
echo 81920 > /proc/net/skbmgr_driver_max_skb
echo 60000 > /proc/net/skbmgr_limit
echo 30720 > /proc/net/skbmgr_4k_limit
fi
