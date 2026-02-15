#!/bin/bash

PROFILE_CFG=/userfs/profile.cfg
if [ -f $PROFILE_CFG ] ; then
    . $PROFILE_CFG
fi

if [ "$TCSUPPORT_IS_FH_PON" != "" ] ;then
if [ "$TCSUPPORT_CT_VLAN_TAG" != "" ] ;then
insmod /lib/modules/vlantag_ct.ko
fi

if [ "$TCSUPPORT_SMUX" != "" ] ;then
insmod /lib/modules/multiwan.ko
fi
fi

if [ "$TCSUPPORT_QDMA_BUFMGR" != "" ] ;then
if [ "$TCSUPPORT_CPU_EN7512" != "" ] || [ "$TCSUPPORT_CPU_EN7521" != "" ] || [ "$TCSUPPORT_CPU_EN7523" != "" ] || [ "$TCSUPPORT_CPU_EN7580" != "" ] || [ "$TCSUPPORT_CPU_EN7516" != "" ] || [ "$TCSUPPORT_CPU_EN7527" != "" ] || [ "$TCSUPPORT_CPU_EN7528" != "" ];then
if [ "$TCSUPPORT_NP" != "" ] ;then
echo "=======no need insmod qdma_wan.ko========"
else
insmod /lib/modules/qdma_wan.ko
###########pending discussion start#####################
if [ "$TCSUPPORT_CPU_EN7580" != "" ] ;then
/userfs/bin/qdmamgr_wan set general_rx_ratelimit config 0 enable packet slow
/userfs/bin/qdmamgr_wan set general_rx_ratelimit value 0 4000
/userfs/bin/qdmamgr_wan set general_rx_ratelimit config 1 enable packet fast
/userfs/bin/qdmamgr_wan set general_rx_ratelimit value 1 1000000
else
/userfs/bin/qdmamgr_wan set rxratelimit config Enable packet
/userfs/bin/qdmamgr_wan set rxratelimit value 0 6000
/userfs/bin/qdmamgr_wan set rxratelimit value 1 1000000
fi
fi
###########pending discussion end#######################
else
insmod /lib/modules/qdma.ko
fi
fi
if [ "$TCSUPPORT_WAN_PTM" != "" ] || [ "$TCSUPPORT_WAN_ATM" != "" ] ;then
insmod /lib/modules/tc3162_dmt.ko
fi

if [ "$TCSUPPORT_NP" != "" ] ;then
sh /userfs/script/ethwan_init.sh
else
#load xpon reference
if [ "$TCSUPPORT_WAN_GPON" != "" ] || [ "$TCSUPPORT_WAN_EPON" != "" ] ;then
sh /userfs/script/pon_init.sh
fi
fi
