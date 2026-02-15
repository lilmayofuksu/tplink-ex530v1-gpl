#ifndef __LINUX_BRIDGE_EBT_IEEE1905_H 
#define __LINUX_BRIDGE_EBT_IEEE1905_H 
#define PAYLOAD_MAX_LEN 16
struct ebt_ieee1905_info {
        unsigned char payload[PAYLOAD_MAX_LEN];
        /* EBT_ACCEPT, EBT_DROP, EBT_CONTINUE or EBT_RETURN */
        int len;
        int target;
};
#define EBT_1905VNDR_TARGET "ieee1905"

#endif

