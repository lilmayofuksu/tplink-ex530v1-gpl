WLAN_MODULE_RT5392=/lib/modules/rt5392ap.ko
WLAN_MODULE_RT5390=/lib/modules/rt5390ap.ko
WLAN_MODULE_RT3390=/lib/modules/rt3390ap.ko
WLAN_MODULE_RT3090=/lib/modules/rt3090ap.ko
WLAN_MODULE_RT2561=/lib/modules/rt61ap.ko

/sbin/ifconfig ra0 down
/sbin/ifconfig ra1 down
/sbin/ifconfig ra2 down
/sbin/ifconfig ra3 down
if [ -f $WLAN_MODULE_RT5392 ] ; then
        /sbin/rmmod rt5392ap
fi

if [ -f $WLAN_MODULE_RT5390 ] ; then
	/sbin/rmmod rt5390ap
fi

if [ -f $WLAN_MODULE_RT3390 ] ; then
	/sbin/rmmod rt3390ap
fi

if [ -f $WLAN_MODULE_RT3090 ] ; then
	/sbin/rmmod rt3090ap
fi

if [ -f $WLAN_MODULE_RT2561 ] ; then
	/sbin/rmmod rt61ap
fi


/usr/bin/killall syslogd
/usr/bin/killall klogd
/usr/bin/killall dnsmasq
/usr/bin/killall bftpd
/usr/bin/killall utelnetd 
/usr/bin/killall udhcpd
/usr/bin/killall radvd
/usr/bin/killall arp
/usr/bin/killall wscd
/usr/bin/killall inetd
/usr/bin/killall -9 tftpd
/usr/bin/killall -9 wscd

echo "3" > /proc/sys/vm/drop_caches
