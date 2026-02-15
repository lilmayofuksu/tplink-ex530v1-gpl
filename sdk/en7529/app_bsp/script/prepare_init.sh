#!/bin/bash

/bin/busybox mkdir /tmp/etc
/bin/busybox cp -a /usr/etc/ethertypes /tmp/etc
/bin/busybox cp -a /usr/etc/Wireless /tmp/etc

/bin/busybox mkdir -m 0777 -p /tmp/var
/bin/busybox mkdir -m 0777 -p /var/lock
/bin/busybox mkdir -m 0777 -p /var/log
/bin/busybox mkdir -m 0777 -p /var/run
/bin/busybox mkdir -m 0777 -p /var/tmp
