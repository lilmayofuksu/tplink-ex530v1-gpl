#!/bin/sh

# Change usb mode from storage mode to ethernet mode
# Option
# 	-v, --default-vendor NUM      vendor ID of original mode (mandatory)
#	-p, --default-product NUM     product ID of original mode (mandatory)
#	-M, --message-content <msg>   message to send (hex number as string)
#
# Ex.
# 	ZTE-MF832S default vendor ID is 0x19d2, default product ID is 0x0198
usb_modeswitch -v 0x19d2 -p 0x0198 \
	-M  "5553424312345679000000000000061b000000020000000000000000000000"

echo "wait for 5 seconds after usb mode switch"
sleep 5

# Pppd daemon applies point-to-point protocol to establish internet links over dial-up modems
# Command-line options are explained as follows:
# /dev/ttyUSB* -- Specifies the device you are going to use. 
#				  This will usually be /dev/ttyUSB0, /dev/ttyUSB1, /dev/ttyUSB2
# 115200 	   -- The speed the modem is to be set to. This is only the speed 
#				  between the PC and the modem and has nothing to do with the actual data throughput. 
# debug 	   -- Output debug information
# modem		   -- Use modem control lines. This is actually the default.
# noauth
# defaultroute -- Create an IP route after PPP comes alive. Henceforth, packets will go to the right place.
# noccp 
# nobsdcomp
# novj
# nodetach     -- Remain always a foreground process. This allows you to watch pppd run and stop it with ^C
# noipdefault 
# usepeerdns
# connect <script> -- Specifies the script that pppd must use to start things up. When you use a modem manually, 
#					  you need to go through the steps of initializing the modem, causing a dial, connecting, 
#					  logging in, and finally telling the remote computer that you would like to set the connection 
#					  to ``data communication'' mode. The <script> is the automation of this manual procedure.
pppd_usbDongle /dev/ttyUSB0 115200 debug modem noauth \
defaultroute noccp nobsdcomp novj noipdefault usepeerdns \
connect "/userfs/bin/chat -v -s -f /etc/ppp/zte_cmnet"

echo "wait for 120 seconds after send AT command through pppd_usbDongle daemon"
sleep 120

echo "Send AT command \"AT+CFUN=1\""
cat /dev/ttyUSB0 & echo "AT+CFUN=1" > /dev/ttyUSB0
sleep 5

echo "Send AT command \"AT+CEREG?\""
cat /dev/ttyUSB0 & echo "AT+CEREG?" > /dev/ttyUSB0
sleep 5

echo "Send AT command \"AT+ZGACT=1,1\""
cat /dev/ttyUSB0 & echo "AT+ZGACT=1,1" > /dev/ttyUSB0
sleep 5

echo "Send AT command \"AT+CEREG?\""
cat /dev/ttyUSB0 & echo "AT+CEREG?" > /dev/ttyUSB0
sleep 5


# DHCP client obtains private IP address from DHCP server on ZTE dongle
udhcpc_usbDongle -i eth1 -s /usr/script/udhcpc.sh


