#!/bin/bash

PROFILE_CFG=/userfs/profile.cfg
if [ -f $PROFILE_CFG ] ; then
    . $PROFILE_CFG
fi


if [ "$TCSUPPORT_IS_FH_PON" != "" ] ;then
echo Enable Yes > /proc/tc3162/eth1_stats

ifconfig wan0 up

fi

