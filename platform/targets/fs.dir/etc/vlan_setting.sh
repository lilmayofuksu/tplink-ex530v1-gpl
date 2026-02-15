#!/bin/sh

ssdk_sh vlan entry flush

ssdk_sh portVlan ingress set 0 check
ssdk_sh portVlan ingress set 1 check
ssdk_sh portVlan ingress set 2 check
ssdk_sh portVlan ingress set 3 check
ssdk_sh portVlan ingress set 4 check
ssdk_sh portVlan ingress set 5 check
ssdk_sh portVlan ingress set 6 check

##############################################	
ssdk_sh portVlan defaultCVid set 5 10
ssdk_sh portVlan defaultCVid set 0 10
 
ssdk_sh portVlan defaultSVid set 5 10
ssdk_sh portVlan defaultSVid set 0 10

ssdk_sh vlan entry append 10 10 0,5 null 0,5 null no no

ssdk_sh portVlan egress set 0 untagged
ssdk_sh portVlan egress set 5 untagged

#############################################

ssdk_sh vlan entry append 2 0xffff 1,6 6 1 null no no
ssdk_sh vlan entry append 3 0xffff 2,6 6 2 null no no
ssdk_sh vlan entry append 4 0xffff 3,6 6 3 null no no
ssdk_sh vlan entry append 5 0xffff 4,6 6 4 null no no

ssdk_sh vlan entry append 0x802 0xffff 1,2,3,4,6 6 1,2,3,4 null no no
ssdk_sh vlan entry append 0x803 0xffff 1,2,3,4,6 6 1,2,3,4 null no no
ssdk_sh vlan entry append 0x804 0xffff 1,2,3,4,6 6 1,2,3,4 null no no
ssdk_sh vlan entry append 0x805 0xffff 1,2,3,4,6 6 1,2,3,4 null no no

####################################################################

ssdk_sh portVlan defaultCVid set 1 0x802
ssdk_sh portVlan defaultCVid set 2 0x803
ssdk_sh portVlan defaultCVid set 3 0x804
ssdk_sh portVlan defaultCVid set 4 0x805

ssdk_sh portVlan defaultSVid set 1 0x802
ssdk_sh portVlan defaultSVid set 2 0x803
ssdk_sh portVlan defaultSVid set 3 0x804
ssdk_sh portVlan defaultSVid set 4 0x805

####################################################################

ssdk_sh portVlan defaultCVid set 6 0
ssdk_sh portVlan defaultSVid set 6 0

ssdk_sh portVlan egress set 6 tagged
ssdk_sh portVlan egress set 1 untagged
ssdk_sh portVlan egress set 2 untagged
ssdk_sh portVlan egress set 3 untagged
ssdk_sh portVlan egress set 4 untagged

ssdk_sh fdb entry flush 0