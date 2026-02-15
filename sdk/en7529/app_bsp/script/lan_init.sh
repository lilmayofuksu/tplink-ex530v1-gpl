#!/bin/bash

PROFILE_CFG=/userfs/profile.cfg
if [ -f $PROFILE_CFG ] ; then
    . $PROFILE_CFG
fi

#load driver
echo "insmod ETH_LAN driver"
insmod /lib/modules/fe_core.ko
if [ "$TCSUPPORT_IFC_EN" != "" ] ;then
insmod /lib/modules/ifc.ko
fi
insmod /lib/modules/qdma_lan.ko

if [ "$TCSUPPORT_IS_FH_PON" != "" ] ;then
if [ "$TCSUPPORT_NP" != "" ] ;then
insmod /lib/modules/eth.ko lan_itf=eth wan_itf=wan0 sep_itf=eth itf_start_idx=0 itf_num=3
else
insmod /lib/modules/eth.ko lan_itf=eth sep_itf=eth itf_start_idx=0 itf_num=4
fi
else
insmod /lib/modules/eth.ko
fi

insmod /lib/modules/eth_ephy.ko

if [ "$TCSUPPORT_CT_LOOPDETECT" != "" ] ;then
insmod /lib/modules/loopdetect.ko
fi

###########pending discussion start#####################
if [ "$TCSUPPORT_CPU_EN7580" != "" ] ;then
/userfs/bin/qdmamgr_lan set general_rx_ratelimit config 0 enable packet slow
/userfs/bin/qdmamgr_lan set general_rx_ratelimit value 0 4000
/userfs/bin/qdmamgr_lan set general_rx_ratelimit config 1 enable packet fast
/userfs/bin/qdmamgr_lan set general_rx_ratelimit value 1 1000000
else
/userfs/bin/qdmamgr_lan set rxratelimit config Enable packet
/userfs/bin/qdmamgr_lan set rxratelimit value 0 6000
/userfs/bin/qdmamgr_lan set rxratelimit value 1 1000000
if [ "$TCSUPPORT_CPU_EN7516" != "" ] || [ "$TCSUPPORT_CPU_EN7527" != "" ] || [ "$TCSUPPORT_CPU_EN7528" != "" ] ;then
/userfs/bin/qdmamgr_lan set rxratelimit value 2 1000000
/userfs/bin/qdmamgr_lan set rxratelimit value 3 1000000
/userfs/bin/qdmamgr_lan set rxratelimit value 4 1000000
/userfs/bin/qdmamgr_lan set rxratelimit value 5 1000000
/userfs/bin/qdmamgr_lan set rxratelimit value 6 1000000
/userfs/bin/qdmamgr_lan set rxratelimit value 7 1000000
/userfs/bin/qdmamgr_lan set rxratelimit value 8 1000000
/userfs/bin/qdmamgr_lan set rxratelimit value 9 1000000
/userfs/bin/qdmamgr_lan set rxratelimit value 10 1000000
/userfs/bin/qdmamgr_lan set rxratelimit value 11 1000000
/userfs/bin/qdmamgr_lan set rxratelimit value 12 1000000
/userfs/bin/qdmamgr_lan set rxratelimit value 13 1000000
/userfs/bin/qdmamgr_lan set rxratelimit value 14 1000000
/userfs/bin/qdmamgr_lan set rxratelimit value 15 1000000
fi
fi
###########pending discussion end#####################

