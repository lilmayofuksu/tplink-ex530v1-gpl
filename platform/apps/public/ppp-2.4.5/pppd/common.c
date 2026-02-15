/***********************************************************************
*
* common.c
*
* Implementation of user-space PPPoE redirector for Linux.
*
* Common functions used by PPPoE client and server
*
* Copyright (C) 2000 by Roaring Penguin Software Inc.
*
* This program may be distributed according to the terms of the GNU
* General Public License, version 2 or (at your option) any later version.
*
***********************************************************************/

static char const RCSID[] =
"$Id: common.c,v 1.3 2008/06/09 08:34:23 paulus Exp $";

#define _GNU_SOURCE 1
#include "pppoe.h"
#include "pppd.h"

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <syslog.h>	/* for LOG_DEBUG */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif


#ifdef INCLUDE_PADT_PRECEDE
unsigned int oldSession;
unsigned char serverMAC[6];
/* Every time start the pppd process, try to send a PAdT package to terminate the last session. Added by Wang Jianfeng 2014-05-05*/
unsigned char enablePadtPrecede;
#endif
unsigned int discoveryTimeout;

/**********************************************************************
*%FUNCTION: parsePacket
*%ARGUMENTS:
* packet -- the PPPoE discovery packet to parse
* func -- function called for each tag in the packet
* extra -- an opaque data pointer supplied to parsing function
*%RETURNS:
* 0 if everything went well; -1 if there was an error
*%DESCRIPTION:
* Parses a PPPoE discovery packet, calling "func" for each tag in the packet.
* "func" is passed the additional argument "extra".
***********************************************************************/
int
parsePacket(PPPoEPacket *packet, ParseFunc *func, void *extra)
{
    UINT16_t len = ntohs(packet->length);
    unsigned char *curTag;
    UINT16_t tagType, tagLen;

    if (PPPOE_VER(packet->vertype) != 1) {
	error("Invalid PPPoE version (%d)", PPPOE_VER(packet->vertype));
	return -1;
    }
    if (PPPOE_TYPE(packet->vertype) != 1) {
	error("Invalid PPPoE type (%d)", PPPOE_TYPE(packet->vertype));
	return -1;
    }

    /* Do some sanity checks on packet */
    if (len > ETH_DATA_LEN - 6) { /* 6-byte overhead for PPPoE header */
	error("Invalid PPPoE packet length (%u)", len);
	return -1;
    }

    /* Step through the tags */
    curTag = packet->payload;
    while(curTag - packet->payload < len) {
	/* Alignment is not guaranteed, so do this by hand... */
	tagType = (curTag[0] << 8) + curTag[1];
	tagLen = (curTag[2] << 8) + curTag[3];
	if (tagType == TAG_END_OF_LIST) {
	    return 0;
	}
	if ((curTag - packet->payload) + tagLen + TAG_HDR_SIZE > len) {
	    error("Invalid PPPoE tag length (%u)", tagLen);
	    return -1;
	}
	func(tagType, tagLen, curTag+TAG_HDR_SIZE, extra);
	curTag = curTag + TAG_HDR_SIZE + tagLen;
    }
    return 0;
}

#ifdef INCLUDE_PADT_PRECEDE
/***********************************************************************
*%FUNCTION: sendTermination
*%ARGUMENTS:
* conn -- PPPoEConnection structure
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Sends a PADT packet after reboot
***********************************************************************/
void
sendTermination(PPPoEConnection *conn, unsigned int session_id)
{
    PPPoEPacket packet;
    unsigned char *cursor = packet.payload;
    char mac[6];
    UINT16_t plen = 0;

    if (session_id == 0) return;
    /* Do nothing if no session established yet */
    //if (!conn->session) return;

    /* Do nothing if no discovery socket */
    if (conn->discoverySocket < 0) return;
    memcpy(packet.ethHdr.h_dest, serverMAC, ETH_ALEN);
    //memcpy(packet.ethHdr.h_dest, conn->peerEth, ETH_ALEN);
    memcpy(packet.ethHdr.h_source, conn->myEth, ETH_ALEN);

    packet.ethHdr.h_proto = htons(Eth_PPPOE_Session);
    packet.vertype = 0x11;
    packet.code = CODE_SESS;
    packet.session = htons(session_id);//conn->session;

    /* if last session doesn't terminate ,terminate it now! */
    cursor[0] = 0xc0;
    cursor[1] = 0x21;
    cursor[2] = 0x05;
    cursor[3] = 0x02;
    cursor[4] = 0x00;
    cursor[5] = 0x10;
    cursor[6] = 0x55;
    cursor[7] = 0x73;
    cursor[8] = 0x65;
    cursor[9] = 0x72;
    cursor[10] = 0x20;
    cursor[11] = 0x72;
    cursor[12] = 0x65;
    cursor[13] = 0x71;
    cursor[14] = 0x75;
    cursor[15] = 0x65;
    cursor[16] = 0x73;
    cursor[17] = 0x74;

    plen = 18;
    packet.length = htons(plen);
    sendPacket(conn, conn->discoverySocket, &packet, (int) (plen + HDR_SIZE));
//info("Sent TERMINATION");
}
#endif
/***********************************************************************
*%FUNCTION: sendPADT
*%ARGUMENTS:
* conn -- PPPoE connection
* msg -- if non-NULL, extra error message to include in PADT packet.
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Sends a PADT packet
***********************************************************************/
void
sendPADT(PPPoEConnection *conn, char const *msg)
{
    PPPoEPacket packet;
    unsigned char *cursor = packet.payload;

    UINT16_t plen = 0;

    /* Do nothing if no session established yet */
#ifdef INCLUDE_PADT_PRECEDE
    if ((!conn->session) && (oldSession == 0xffff )) return;
#else
    if (!conn->session) return;
#endif

    /* Do nothing if no discovery socket */
    if (conn->discoverySocket < 0) return;

#ifdef INCLUDE_PADT_PRECEDE
	/* 每次启动pppd进程，都尝试发PADT包终结上次的session. Modified by Wang Jianfeng 2014-05-05*/
	if (!conn->session)
	{
		memcpy(packet.ethHdr.h_dest, serverMAC, ETH_ALEN);
	}
	else
	{
		memcpy(packet.ethHdr.h_dest, conn->peerEth, ETH_ALEN);
	}
#else
    memcpy(packet.ethHdr.h_dest, conn->peerEth, ETH_ALEN);
#endif

    memcpy(packet.ethHdr.h_source, conn->myEth, ETH_ALEN);

    packet.ethHdr.h_proto = htons(Eth_PPPOE_Discovery);
    packet.vertype = PPPOE_VER_TYPE(1, 1);
    packet.code = CODE_PADT;

#ifdef INCLUDE_PADT_PRECEDE
	/* 每次启动pppd进程，都尝试发PADT包终结上次的session. Modified by Wang Jianfeng 2014-05-05*/
	if (!conn->session)
	{
		packet.session = htons((UINT16_t)oldSession);
	}
	else
	{
		packet.session = conn->session;
	}
#else
    packet.session = conn->session;
#endif

    /* Reset Session to zero so there is no possibility of
       recursive calls to this function by any signal handler */
    conn->session = 0;

    /* If we're using Host-Uniq, copy it over */
#ifdef INCLUDE_PADT_PRECEDE
    if (conn->useHostUniq && (oldSession == 0xffff )) {
#else
    if (conn->useHostUniq) {
#endif
	PPPoETag hostUniq;
	pid_t pid = getpid();
	hostUniq.type = htons(TAG_HOST_UNIQ);
	hostUniq.length = htons(sizeof(pid));
	memcpy(hostUniq.payload, &pid, sizeof(pid));
	memcpy(cursor, &hostUniq, sizeof(pid) + TAG_HDR_SIZE);
	cursor += sizeof(pid) + TAG_HDR_SIZE;
	plen += sizeof(pid) + TAG_HDR_SIZE;
    }

    /* Copy error message */
    if (msg) {
	PPPoETag err;
	size_t elen = strlen(msg);
	err.type = htons(TAG_GENERIC_ERROR);
	err.length = htons(elen);
	strcpy(err.payload, msg);
	memcpy(cursor, &err, elen + TAG_HDR_SIZE);
	cursor += elen + TAG_HDR_SIZE;
	plen += elen + TAG_HDR_SIZE;
    }

    /* Copy cookie and relay-ID if needed */
    if (conn->cookie.type) {
	CHECK_ROOM(cursor, packet.payload,
		   ntohs(conn->cookie.length) + TAG_HDR_SIZE);
	memcpy(cursor, &conn->cookie, ntohs(conn->cookie.length) + TAG_HDR_SIZE);
	cursor += ntohs(conn->cookie.length) + TAG_HDR_SIZE;
	plen += ntohs(conn->cookie.length) + TAG_HDR_SIZE;
    }

    if (conn->relayId.type) {
	CHECK_ROOM(cursor, packet.payload,
		   ntohs(conn->relayId.length) + TAG_HDR_SIZE);
	memcpy(cursor, &conn->relayId, ntohs(conn->relayId.length) + TAG_HDR_SIZE);
	cursor += ntohs(conn->relayId.length) + TAG_HDR_SIZE;
	plen += ntohs(conn->relayId.length) + TAG_HDR_SIZE;
    }

    packet.length = htons(plen);
    sendPacket(conn, conn->discoverySocket, &packet, (int) (plen + HDR_SIZE));
    info("Sent PADT");
}

#define EH(x)	(x)[0], (x)[1], (x)[2], (x)[3], (x)[4], (x)[5]

/* Print out a PPPOE packet for debugging */
void pppoe_printpkt(PPPoEPacket *packet,
		    void (*printer)(void *, char *, ...), void *arg)
{
    int len = ntohs(packet->length);
    int i, tag, tlen, text;

    switch (ntohs(packet->ethHdr.h_proto)) {
    case ETH_PPPOE_DISCOVERY:
	printer(arg, "PPPOE Discovery V%dT%d ", PPPOE_VER(packet->vertype),
		PPPOE_TYPE(packet->vertype));
	switch (packet->code) {
	case CODE_PADI:
	    printer(arg, "PADI");
	    break;
	case CODE_PADO:
	    printer(arg, "PADO");
	    break;
	case CODE_PADR:
	    printer(arg, "PADR");
	    break;
	case CODE_PADS:
	    printer(arg, "PADS");
	    break;
	case CODE_PADT:
	    printer(arg, "PADT");
	    break;
	default:
	    printer(arg, "unknown code %x", packet->code);
	}
	printer(arg, " session 0x%x length %d\n", ntohs(packet->session), len);
	break;
    case ETH_PPPOE_SESSION:
	printer(arg, "PPPOE Session V%dT%d", PPPOE_VER(packet->vertype),
		PPPOE_TYPE(packet->vertype));
	printer(arg, " code 0x%x session 0x%x length %d\n", packet->code,
		ntohs(packet->session), len);
	break;
    default:
	printer(arg, "Unknown ethernet frame with proto = 0x%x\n",
		ntohs(packet->ethHdr.h_proto));
    }

    printer(arg, " dst %x:%x:%x:%x:%x:%x ", EH(packet->ethHdr.h_dest));
    printer(arg, " src %x:%x:%x:%x:%x:%x\n", EH(packet->ethHdr.h_source));
    if (ntohs(packet->ethHdr.h_proto) != ETH_PPPOE_DISCOVERY)
	return;

    for (i = 0; i + TAG_HDR_SIZE <= len; i += tlen) {
	tag = (packet->payload[i] << 8) + packet->payload[i+1];
	tlen = (packet->payload[i+2] << 8) + packet->payload[i+3];
	if (i + tlen + TAG_HDR_SIZE > len)
	    break;
	text = 0;
	i += TAG_HDR_SIZE;
	printer(arg, " [");
	switch (tag) {
	case TAG_END_OF_LIST:
	    printer(arg, "end-of-list");
	    break;
	case TAG_SERVICE_NAME:
	    printer(arg, "service-name");
	    text = 1;
	    break;
	case TAG_AC_NAME:
	    printer(arg, "AC-name");
	    text = 1;
	    break;
	case TAG_HOST_UNIQ:
	    printer(arg, "host-uniq");
	    break;
	case TAG_AC_COOKIE:
	    printer(arg, "AC-cookie");
	    break;
	case TAG_VENDOR_SPECIFIC:
	    printer(arg, "vendor-specific");
	    break;
	case TAG_RELAY_SESSION_ID:
	    printer(arg, "relay-session-id");
	    break;
	case TAG_SERVICE_NAME_ERROR:
	    printer(arg, "service-name-error");
	    text = 1;
	    break;
	case TAG_AC_SYSTEM_ERROR:
	    printer(arg, "AC-system-error");
	    text = 1;
	    break;
	case TAG_GENERIC_ERROR:
	    printer(arg, "generic-error");
	    text = 1;
	    break;
	default:
	    printer(arg, "unknown tag 0x%x", tag);
	}
	if (tlen) {
	    if (text)
		printer(arg, " %.*v", tlen, &packet->payload[i]);
	    else if (tlen <= 32)
		printer(arg, " %.*B", tlen, &packet->payload[i]);
	    else
		printer(arg, " %.32B... (length %d)",
			&packet->payload[i], tlen);
	}
	printer(arg, "]");
    }
    printer(arg, "\n");
}

void pppoe_log_packet(const char *prefix, PPPoEPacket *packet)
{
    init_pr_log(prefix, LOG_DEBUG);
    pppoe_printpkt(packet, pr_log, NULL);
    end_pr_log();
}
