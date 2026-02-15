#ifndef _CLIENTPACKET_H
#define _CLIENTPACKET_H

#include "packet.h"

unsigned long random_xid_dongle(void);
int send_discover_dongle(unsigned long xid, unsigned long requested);
int send_selecting_dongle(unsigned long xid, unsigned long server, unsigned long requested);
int send_renew_dongle(unsigned long xid, unsigned long server, unsigned long ciaddr);
int send_release_dongle(unsigned long server, unsigned long ciaddr);
int get_raw_packet_dongle(struct dhcpMessage *payload, int fd);

#endif
