#!/bin/sh

if [ $# != 1 ] ; then
	echo "usage: $0 [PVCn]"
	exit 0
fi

i=$1
CONFFILE=/etc/isp$i.conf
#echo $CONFFILE

iptables -D FORWARD -o nas$i -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu
iptables -D FORWARD -o ppp$i -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu

iptables -t nat -D POSTROUTING -o nas$i -j MASQUERADE
iptables -t nat -D POSTROUTING -o ppp$i -j MASQUERADE
#iptables -t nat -D POSTROUTING -j ADDRMAP_POS$i
iptables -t nat -D PREROUTING -j VS_PRE$i
iptables -t nat -D PREROUTING -j DMZ_PRE$i
#iptables -t nat -D PREROUTING -j ADDRMAP_PRE$i
# flush nat rule
iptables -t nat -F VS_PRE$i
iptables -t nat -F DMZ_PRE$i
#iptables -t nat -F ADDRMAP_PRE$i
#iptables -t nat -F ADDRMAP_POS$i
# flush ipfilter rule
iptables -F PKT_FLT

if [ -f /var/run/ppp$i.pid ] ; then
        kill -15 `cat /var/run/ppp$i.pid`
        sleep 1
	if [ -f /var/run/ppp$i.pid ] ; then
        	kill -9 `cat /var/run/ppp$i.pid`
	fi
fi
if [ -f /var/run/udhcpc-nas$i.pid ] ; then
	#shnwind modify ->use SIGTERM to send release
	kill -SIGTERM `cat /var/run/udhcpc-nas$i.pid`
fi
#/sbin/ifconfig nas$i down
#brctl delif br0 nas$i
if [ -f /var/run/nas$i.pid ] ; then
	kill -9 `cat /var/run/nas$i.pid`
	rm /var/run/nas$i.pid
fi

MAC_CONFFILE=/etc/mac.conf
if [ -f $MAC_CONFFILE ]; then
	chmod 777 $MAC_CONFFILE
	. $MAC_CONFFILE
fi
		
if [ "$LAN_MAC" != "" ]; then
	/sbin/ifconfig br0 hw ether $LAN_MAC
fi

if [ -f /var/run/pppoe-relay_nas$i.pid ] ; then
	kill -9 `cat /var/run/pppoe-relay_nas$i.pid`
	rm /var/run/pppoe-relay_nas$i.pid
fi
#FW_CONF=/etc/firewall.conf
#if [ -f $FW_CONF ]; then
#	chmod +x $FW_CONF
#	. $FW_CONF
#	if [ "$spi_status" = "1" ]; then
#		/usr/bin/iptables -D SPI_FW -i nas$i -m state --state NEW,INVALID -j DROP
#		/usr/bin/iptables -D SPI_FW -i ppp$i -m state --state NEW,INVALID -j DROP
#		/usr/bin/iptables -D SPI_FW -i nas$i -p icmp --icmp-type echo-request -j DROP
#		/usr/bin/iptables -D SPI_FW -i ppp$i -p icmp --icmp-type echo-request -j DROP
#	fi
#fi									

#/usr/script/ipfilter_stop.sh #wtw add
