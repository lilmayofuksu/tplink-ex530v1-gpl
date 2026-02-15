#!/bin/sh

PPP_CONFFILE=/var/run/pppoe_sim.conf
if [ -f $PPP_CONFFILE ] ; then
	. $PPP_CONFFILE
else
	echo "not found pppoe_sim" > /var/tmp/pppoe_sim_err.conf
	exit 0
fi

if [ "$VLANMode" = "tag" ] || [ "$VLANMode" = "TAG" ] ; then
			VTAG_MODE=3
#			VTAG_VID = $VLANID
			/usr/bin/smuxctl add pppoe $NAS_IF $NASName 3 $VLANID 0 0
		elif [ "$VLANMode" = "untag" ] || [ "$VLANMode" = "UNTAG" ]; then
			VTAG_MODE=1
#			VTAG_VID = 0
			/usr/bin/smuxctl add pppoe $NAS_IF $NASName 1 0 0 0
		else #transparent
			VTAG_MODE=2
			/usr/bin/smuxctl add pppoe $NAS_IF $NASName 2 0 0 0
#			VTAG_VID = 0
fi

# /usr/bin/smuxctl add pppoe $NAS_IF $NASName 3 $VLANID 0 0
# /usr/bin/smuxctl add pppoe $NAS_IF $NASName $VTAG_MODE $VTAG_VID 0 0
/sbin/ifconfig $NASName up

if [ "$Username" = "" ]; then
		if [ "$Password" = "" ]; then
		PPP_PARAM="unit $PPPUnit user \"\" password \"\" nodetach holdoff 4 maxfail $RetryTimes usepeerdns lcp-echo-interval 1 lcp-echo-failure 60"
		else
		PPP_PARAM="unit $PPPUnit user \"\" password $Password nodetach holdoff 4 maxfail $RetryTimes usepeerdns lcp-echo-interval 1 lcp-echo-failure 60"
		fi
else
		if [ "$Password" = "" ]; then
		PPP_PARAM="unit $PPPUnit user $Username password \"\" nodetach holdoff 4 maxfail $RetryTimes usepeerdns lcp-echo-interval 1 lcp-echo-failure 60"
		else
PPP_PARAM="unit $PPPUnit user $Username password $Password nodetach holdoff 4 maxfail $RetryTimes usepeerdns lcp-echo-interval 1 lcp-echo-failure 60"
		fi
fi

# PPP_PARAM="unit $PPPUnit user $Username password $Password nodetach holdoff 4 maxfail $RetryTimes usepeerdns lcp-echo-interval 1 lcp-echo-failure 60"

PPP_PARAM="$PPP_PARAM plugin pppoe $NASName"
		
if [ "$PPPAuthenticationProtocol" = "CHAP" ] ; then
	 PPP_PARAM="$PPP_PARAM -pap"
elif [ "$PPPAuthenticationProtocol" = "PAP" ] ; then
	 PPP_PARAM="$PPP_PARAM -chap"
fi

PPP_PARAM="$PPP_PARAM noipdefault"
	
PPP_PARAM="$PPP_PARAM persist"

PPP_PARAM="$PPP_PARAM mtu 1492 mru 1492"

pppd $PPP_PARAM &
echo $! > /var/run/pppoe_sim.pid
echo "pppd $PPP_PARAM &" > /var/tmp/pppoe_sim_pppd.conf
