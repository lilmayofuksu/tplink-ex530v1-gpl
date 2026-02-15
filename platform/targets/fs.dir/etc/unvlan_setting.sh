#!/bin/sh

ssdk_sh vlan entry flush



ssdk_sh portVlan ingress set 0 disable
ssdk_sh portVlan ingress set 1 disable
ssdk_sh portVlan ingress set 2 disable
ssdk_sh portVlan ingress set 3 disable
ssdk_sh portVlan ingress set 4 disable
ssdk_sh portVlan ingress set 5 disable
ssdk_sh portVlan ingress set 6 disable

##############################################

ssdk_sh portVlan defaultCVid set 0 0
ssdk_sh portVlan defaultCVid set 1 0
ssdk_sh portVlan defaultCVid set 2 0
ssdk_sh portVlan defaultCVid set 3 0
ssdk_sh portVlan defaultCVid set 4 0
ssdk_sh portVlan defaultCVid set 5 0
ssdk_sh portVlan defaultCVid set 6 0

ssdk_sh portVlan defaultSVid set 0 0
ssdk_sh portVlan defaultSVid set 1 0
ssdk_sh portVlan defaultSVid set 2 0
ssdk_sh portVlan defaultSVid set 3 0
ssdk_sh portVlan defaultSVid set 4 0
ssdk_sh portVlan defaultSVid set 5 0
ssdk_sh portVlan defaultSVid set 6 0

ssdk_sh vlan entry append 1 1 6,1,2,3,4 null 6,1,2,3,4 default default default
ssdk_sh vlan entry append 2 2 0,5  null 0,5 default default default
ssdk_sh portVlan egress set 0 untagged
ssdk_sh portVlan egress set 1 untagged
ssdk_sh portVlan egress set 2 untagged
ssdk_sh portVlan egress set 3 untagged
ssdk_sh portVlan egress set 4 untagged
ssdk_sh portVlan egress set 5 untagged
ssdk_sh portVlan egress set 6 untagged

ssdk_sh fdb entry flush 0