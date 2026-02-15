#!/bin/sh
echo "mknod ..."

mknod	dev/console	c	4	64
mknod	dev/flash0	c	200	0
mknod	dev/adsl0   c	100	0
mknod	dev/gpio	c	10	123
mknod	dev/led		c	10	151

mknod	dev/ppp		c	108	0
mknod	dev/ptmx	c	5	2

mknod	dev/ptyp0	c	2	0
mknod	dev/ptyp1	c	2	1
mknod	dev/ptyp2	c	2	2

mknod	dev/tty0	c	4	0
mknod	dev/watchdog	c	10	130

mknod	dev/dk0		c	63	0
mknod	dev/caldata	b	31	5

mknod	dev/net/tun	c	10	200

mknod 	dev/sda 	b 	8 	0
mknod 	dev/sda1 	b 	8 	1
mknod 	dev/sda2 	b 	8 	2
mknod 	dev/sdb 	b 	8 	16
mknod 	dev/sdb1 	b 	8 	17
mknod 	dev/sdb2 	b 	8 	18

mknod 	dev/pmap 	c 	200	0
mknod 	dev/qostype 	c 	111	2

#mknod 	dev/fuse	c 	10 	229	
#mknod 	dev/misc/fuse	c 	10 	229	

mknod 	dev/ttyUSB0	c 	188	0
mknod 	dev/ttyUSB1 	c 	188	1
mknod 	dev/ttyUSB2 	c 	188	2
mknod 	dev/ttyUSB3 	c 	188	3
mknod 	dev/ttyUSB4 	c 	188 4
mknod 	dev/ttyUSB5 	c 	188 5	
mknod 	dev/ttyUSB6 	c 	188 6
mknod 	dev/ttyUSB7 	c 	188 7
mknod 	dev/ttyUSB8 	c 	188 8
mknod 	dev/ttyUSB9 	c 	188 9
mknod 	dev/ttyUSB10 	c 	188 10
mknod 	dev/ttyUSB11 	c 	188 11
mknod 	dev/ttyUSB12 	c 	188 12
mknod 	dev/ttyUSB13 	c 	188 13
mknod 	dev/ttyUSB14 	c 	188 14
mknod 	dev/ttyUSB15 	c 	188 15

mknod 	dev/ttyACM0 	c 	166 0
mknod 	dev/ttyACM1 	c 	166 1
mknod 	dev/ttyACM2 	c 	166 2
mknod 	dev/ttyACM3 	c 	166 3
mknod 	dev/ttyACM4 	c 	166 4
mknod 	dev/ttyACM5 	c 	166 5
mknod 	dev/ttyACM6 	c 	166 6
mknod 	dev/ttyACM7 	c 	166 7
mknod 	dev/ttyACM8 	c 	166 8
mknod 	dev/ttyACM9 	c 	166 9
mknod 	dev/ttyACM10 	c 	166 10
mknod 	dev/ttyACM11 	c 	166 11
mknod 	dev/ttyACM12 	c 	166 12
mknod 	dev/ttyACM13 	c 	166 13
mknod 	dev/ttyACM14 	c 	166 14
mknod 	dev/ttyACM15 	c 	166 15

mknod dev/pppox_iptables c 220 0
mknod dev/bcmadsl0 c 208 0
mknod dev/bdmf_shell c 215 0
mknod dev/bcmendpoint0 c 209 0
mknod dev/voip c 214 0
mknod dev/pcmshim0 c 217 0
mknod dev/ac97 c 222 0
mknod dev/slac c 223 0
mknod dev/bcmprof c 224 0
mknod dev/si3215 c 225 0
mknod dev/pts c 228 0
mknod dev/bcmxtmcfg0 c 228 0
mknod dev/bcmvlan c 238 0
mknod dev/pwrmngt c 240 0
mknod dev/bcmfap c 241 0
mknod dev/fcache c 242 0
mknod dev/ingqos c 243 0
mknod dev/bpm c 244 0
mknod dev/bcmarl c 245 0
mknod dev/gmac c 249 0
mknod dev/tms c 250 0
mknod dev/pktrunner c 252 0