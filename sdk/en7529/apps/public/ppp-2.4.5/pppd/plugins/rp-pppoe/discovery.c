/***********************************************************************
*
* discovery.c
*
* Perform PPPoE discovery
*
* Copyright (C) 1999 by Roaring Penguin Software Inc.
*
***********************************************************************/

static char const RCSID[] =
"$Id$";

#define _GNU_SOURCE 1
#include "pppoe.h"
#include "pppd/pppd.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef USE_LINUX_PACKET
#include <sys/ioctl.h>
#include <fcntl.h>
#endif

#include <signal.h>
#if defined(TCSUPPORT_STARTUP_OPTIMIZATION)
extern unsigned char shortoldmac[ETH_ALEN];
extern int shortoldsid;
extern int sendPADTflag;
extern int shortflag;
#endif
#if defined(TCSUPPORT_CT_PPPOE_EMULATOR)
extern int isPPPEmulator();
#endif
int retries = MAX_PADI_ATTEMPTS;

#if defined(TCSUPPORT_CT_JOYME) || defined(TCSUPPORT_CT_JOYME2)
int retransmits = 0;
#endif

#if defined(TCSUPPORT_CT_JOYME) || defined(TCSUPPORT_CT_JOYME2)
int discover_timeout_yh[] = {10, 20, 40, 90 };
int max_coun_yh = sizeof(discover_timeout_yh)/sizeof(int), wait_position_yh = 0; 
#endif

/**********************************************************************
*%FUNCTION: parseForHostUniq
*%ARGUMENTS:
* type -- tag type
* len -- tag length
* data -- tag data.
* extra -- user-supplied pointer.  This is assumed to be a pointer to int.
*%RETURNS:
* Nothing
*%DESCRIPTION:
* If a HostUnique tag is found which matches our PID, sets *extra to 1.
***********************************************************************/
static void
parseForHostUniq(UINT16_t type, UINT16_t len, unsigned char *data,
		 void *extra)
{
    int *val = (int *) extra;
    if (type == TAG_HOST_UNIQ && len == sizeof(pid_t)) {
	pid_t tmp;
	memcpy(&tmp, data, len);
	if (tmp == getpid()) {
	    *val = 1;
	}
    }
}

/**********************************************************************
*%FUNCTION: packetIsForMe
*%ARGUMENTS:
* conn -- PPPoE connection info
* packet -- a received PPPoE packet
*%RETURNS:
* 1 if packet is for this PPPoE daemon; 0 otherwise.
*%DESCRIPTION:
* If we are using the Host-Unique tag, verifies that packet contains
* our unique identifier.
***********************************************************************/
static int
packetIsForMe(PPPoEConnection *conn, PPPoEPacket *packet)
{
    int forMe = 0;

    /* If packet is not directed to our MAC address, forget it */
    if (memcmp(packet->ethHdr.h_dest, conn->myEth, ETH_ALEN)) return 0;

    /* If we're not using the Host-Unique tag, then accept the packet */
    if (!conn->useHostUniq) return 1;

    parsePacket(packet, parseForHostUniq, &forMe);
    return forMe;
}

/**********************************************************************
*%FUNCTION: parsePADOTags
*%ARGUMENTS:
* type -- tag type
* len -- tag length
* data -- tag data
* extra -- extra user data.  Should point to a PacketCriteria structure
*          which gets filled in according to selected AC name and service
*          name.
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Picks interesting tags out of a PADO packet
***********************************************************************/
static void
parsePADOTags(UINT16_t type, UINT16_t len, unsigned char *data,
	      void *extra)
{
    struct PacketCriteria *pc = (struct PacketCriteria *) extra;
    PPPoEConnection *conn = pc->conn;
    int i;

    switch(type) {
    case TAG_AC_NAME:
	pc->seenACName = 1;
	if (conn->printACNames) {
	    info("Access-Concentrator: %.*s", (int) len, data);
	}
	if (conn->acName && len == strlen(conn->acName) &&
	    !strncmp((char *) data, conn->acName, len)) {
	    pc->acNameOK = 1;
	}
	break;
    case TAG_SERVICE_NAME:
	pc->seenServiceName = 1;
	if (conn->serviceName && len == strlen(conn->serviceName) &&
	    !strncmp((char *) data, conn->serviceName, len)) {
	    pc->serviceNameOK = 1;
	}
	break;
    case TAG_AC_COOKIE:
	conn->cookie.type = htons(type);
	conn->cookie.length = htons(len);
	memcpy(conn->cookie.payload, data, len);
	break;
    case TAG_RELAY_SESSION_ID:
	conn->relayId.type = htons(type);
	conn->relayId.length = htons(len);
	memcpy(conn->relayId.payload, data, len);
	break;
    case TAG_SERVICE_NAME_ERROR:
	error("PADO: Service-Name-Error: %.*s", (int) len, data);
	conn->error = 1;
	break;
    case TAG_AC_SYSTEM_ERROR:
	error("PADO: System-Error: %.*s", (int) len, data);
	conn->error = 1;
	break;
    case TAG_GENERIC_ERROR:
	error("PADO: Generic-Error: %.*s", (int) len, data);
	conn->error = 1;
	break;
    }
}

/**********************************************************************
*%FUNCTION: parsePADSTags
*%ARGUMENTS:
* type -- tag type
* len -- tag length
* data -- tag data
* extra -- extra user data (pointer to PPPoEConnection structure)
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Picks interesting tags out of a PADS packet
***********************************************************************/
static void
parsePADSTags(UINT16_t type, UINT16_t len, unsigned char *data,
	      void *extra)
{
    PPPoEConnection *conn = (PPPoEConnection *) extra;
    switch(type) {
    case TAG_SERVICE_NAME:
	dbglog("PADS: Service-Name: '%.*s'", (int) len, data);
	break;
    case TAG_SERVICE_NAME_ERROR:
	error("PADS: Service-Name-Error: %.*s", (int) len, data);
	conn->error = 1;
	break;
    case TAG_AC_SYSTEM_ERROR:
	error("PADS: System-Error: %.*s", (int) len, data);
	conn->error = 1;
	break;
    case TAG_GENERIC_ERROR:
	error("PADS: Generic-Error: %.*s", (int) len, data);
	conn->error = 1;
	break;
    case TAG_RELAY_SESSION_ID:
	conn->relayId.type = htons(type);
	conn->relayId.length = htons(len);
	memcpy(conn->relayId.payload, data, len);
	break;
    }
}

/***********************************************************************
*%FUNCTION: sendPADI
*%ARGUMENTS:
* conn -- PPPoEConnection structure
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Sends a PADI packet
***********************************************************************/
static void
sendPADI(PPPoEConnection *conn)
{
    PPPoEPacket packet;
    unsigned char *cursor = packet.payload;
    PPPoETag *svc = (PPPoETag *) (&packet.payload);
    UINT16_t namelen = 0;
    UINT16_t plen;
    int omit_service_name = 0;

    if (conn->serviceName) {
	namelen = (UINT16_t) strlen(conn->serviceName);
	if (!strcmp(conn->serviceName, "NO-SERVICE-NAME-NON-RFC-COMPLIANT")) {
	    omit_service_name = 1;
	}
    }

    /* Set destination to Ethernet broadcast address */
    memset(packet.ethHdr.h_dest, 0xFF, ETH_ALEN);
    memcpy(packet.ethHdr.h_source, conn->myEth, ETH_ALEN);

    packet.ethHdr.h_proto = htons(Eth_PPPOE_Discovery);
    packet.vertype = PPPOE_VER_TYPE(1, 1);
    packet.code = CODE_PADI;
    packet.session = 0;

    if (!omit_service_name) {
	plen = TAG_HDR_SIZE + namelen;
	CHECK_ROOM(cursor, packet.payload, plen);

	svc->type = TAG_SERVICE_NAME;
	svc->length = htons(namelen);

	if (conn->serviceName) {
	    memcpy(svc->payload, conn->serviceName, strlen(conn->serviceName));
	}
	cursor += namelen + TAG_HDR_SIZE;
    } else {
	plen = 0;
    }

    /* If we're using Host-Uniq, copy it over */
    if (conn->useHostUniq) {
	PPPoETag hostUniq;
	pid_t pid = getpid();
	hostUniq.type = htons(TAG_HOST_UNIQ);
	hostUniq.length = htons(sizeof(pid));
	memcpy(hostUniq.payload, &pid, sizeof(pid));
	CHECK_ROOM(cursor, packet.payload, sizeof(pid) + TAG_HDR_SIZE);
	memcpy(cursor, &hostUniq, sizeof(pid) + TAG_HDR_SIZE);
	cursor += sizeof(pid) + TAG_HDR_SIZE;
	plen += sizeof(pid) + TAG_HDR_SIZE;
    }

    packet.length = htons(plen);

    sendPacket(conn, conn->discoverySocket, &packet, (int) (plen + HDR_SIZE));
}

/**********************************************************************
*%FUNCTION: waitForPADO
*%ARGUMENTS:
* conn -- PPPoEConnection structure
* timeout -- how long to wait (in seconds)
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Waits for a PADO packet and copies useful information
***********************************************************************/
void
waitForPADO(PPPoEConnection *conn, int timeout)
{
    fd_set readable;
    int r;
    struct timeval tv;
    PPPoEPacket packet;
    int len;

    struct PacketCriteria pc;
    pc.conn          = conn;
    pc.acNameOK      = (conn->acName)      ? 0 : 1;
    pc.serviceNameOK = (conn->serviceName) ? 0 : 1;
    pc.seenACName    = 0;
    pc.seenServiceName = 0;
    conn->error = 0;

    do {
	if (BPF_BUFFER_IS_EMPTY) {
	    tv.tv_sec = timeout;
	    tv.tv_usec = 0;

	    FD_ZERO(&readable);
	    FD_SET(conn->discoverySocket, &readable);

	    while(1) {
	        r = select(conn->discoverySocket+1, &readable, NULL, NULL, &tv);
	        if (r >= 0 || errno != EINTR)
	            break;
	        if (r < 0) {
	         	error("select (waitForPADO): %m");
	            return;
	        }
	    }
        if (r == 0) return;        /* Timed out */
	}

	/* Get the packet */
	receivePacket(conn->discoverySocket, &packet, &len);

	/* Check length */
	if (ntohs(packet.length) + HDR_SIZE > len) {
	    error("Bogus PPPoE length field (%u)",
		   (unsigned int) ntohs(packet.length));
	    continue;
	}

#ifdef USE_BPF
	/* If it's not a Discovery packet, loop again */
	if (etherType(&packet) != Eth_PPPOE_Discovery) continue;
#endif

	/* If it's not for us, loop again */
	if (!packetIsForMe(conn, &packet)) continue;

	if (packet.code == CODE_PADO) {
	    if (NOT_UNICAST(packet.ethHdr.h_source)) {
		error("Ignoring PADO packet from non-unicast MAC address");
		continue;
	    }
	    if (conn->req_peer
		&& memcmp(packet.ethHdr.h_source, conn->req_peer_mac, ETH_ALEN) != 0) {
		warn("Ignoring PADO packet from wrong MAC address");
		continue;
	    }
	    if (parsePacket(&packet, parsePADOTags, &pc) < 0)
		return;
	    if (conn->error)
		return;
	    if (!pc.seenACName) {
		error("Ignoring PADO packet with no AC-Name tag");
		continue;
	    }
	    if (!pc.seenServiceName) {
		error("Ignoring PADO packet with no Service-Name tag");
		continue;
	    }
	    conn->numPADOs++;
	    if (pc.acNameOK && pc.serviceNameOK) {
		memcpy(conn->peerEth, packet.ethHdr.h_source, ETH_ALEN);
		conn->discoveryState = STATE_RECEIVED_PADO;
		break;
	    }
	}
    } while (conn->discoveryState != STATE_RECEIVED_PADO);
}

/***********************************************************************
*%FUNCTION: sendPADR
*%ARGUMENTS:
* conn -- PPPoE connection structur
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Sends a PADR packet
***********************************************************************/
static void
sendPADR(PPPoEConnection *conn)
{
    PPPoEPacket packet;
    PPPoETag *svc = (PPPoETag *) packet.payload;
    unsigned char *cursor = packet.payload;

    UINT16_t namelen = 0;
    UINT16_t plen;

    if (conn->serviceName) {
	namelen = (UINT16_t) strlen(conn->serviceName);
    }
    plen = TAG_HDR_SIZE + namelen;
    CHECK_ROOM(cursor, packet.payload, plen);

    memcpy(packet.ethHdr.h_dest, conn->peerEth, ETH_ALEN);
    memcpy(packet.ethHdr.h_source, conn->myEth, ETH_ALEN);

    packet.ethHdr.h_proto = htons(Eth_PPPOE_Discovery);
    packet.vertype = PPPOE_VER_TYPE(1, 1);
    packet.code = CODE_PADR;
    packet.session = 0;

    svc->type = TAG_SERVICE_NAME;
    svc->length = htons(namelen);
    if (conn->serviceName) {
	memcpy(svc->payload, conn->serviceName, namelen);
    }
    cursor += namelen + TAG_HDR_SIZE;

    /* If we're using Host-Uniq, copy it over */
    if (conn->useHostUniq) {
	PPPoETag hostUniq;
	pid_t pid = getpid();
	hostUniq.type = htons(TAG_HOST_UNIQ);
	hostUniq.length = htons(sizeof(pid));
	memcpy(hostUniq.payload, &pid, sizeof(pid));
	CHECK_ROOM(cursor, packet.payload, sizeof(pid)+TAG_HDR_SIZE);
	memcpy(cursor, &hostUniq, sizeof(pid) + TAG_HDR_SIZE);
	cursor += sizeof(pid) + TAG_HDR_SIZE;
	plen += sizeof(pid) + TAG_HDR_SIZE;
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
}

/**********************************************************************
*%FUNCTION: waitForPADS
*%ARGUMENTS:
* conn -- PPPoE connection info
* timeout -- how long to wait (in seconds)
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Waits for a PADS packet and copies useful information
***********************************************************************/
static void
waitForPADS(PPPoEConnection *conn, int timeout)
{
    fd_set readable;
    int r;
    struct timeval tv;
    PPPoEPacket packet;
    int len;
#if defined(TCSUPPORT_CT_PPPOE_EMULATOR)
	char emulator_tmp[8] = {0};
#endif

    conn->error = 0;
    do {
	if (BPF_BUFFER_IS_EMPTY) {
	    tv.tv_sec = timeout;
	    tv.tv_usec = 0;

	    FD_ZERO(&readable);
	    FD_SET(conn->discoverySocket, &readable);

	    while(1) {
			r = select(conn->discoverySocket+1, &readable, NULL, NULL, &tv);
			if (r >= 0 || errno != EINTR)
				break;
			if (r < 0) {
				error("select (waitForPADS): %m");
				return;
			}
	    }
		if (r == 0) return;
	}

	/* Get the packet */
	receivePacket(conn->discoverySocket, &packet, &len);

	/* Check length */
	if (ntohs(packet.length) + HDR_SIZE > len) {
	    error("Bogus PPPoE length field (%u)",
		   (unsigned int) ntohs(packet.length));
	    continue;
	}

#ifdef USE_BPF
	/* If it's not a Discovery packet, loop again */
	if (etherType(&packet) != Eth_PPPOE_Discovery) continue;
#endif

	/* If it's not from the AC, it's not for me */
	if (memcmp(packet.ethHdr.h_source, conn->peerEth, ETH_ALEN)) continue;

	/* If it's not for us, loop again */
	if (!packetIsForMe(conn, &packet)) continue;

	/* Is it PADS?  */
	if (packet.code == CODE_PADS) {
	    /* Parse for goodies */
	    if (parsePacket(&packet, parsePADSTags, conn) < 0)
		return;
	    if (conn->error)
		return;
	    conn->discoveryState = STATE_SESSION;
	    break;
	}
    } while (conn->discoveryState != STATE_SESSION);

    /* Don't bother with ntohs; we'll just end up converting it back... */
    conn->session = packet.session;

    info("PPP session is %d", (int) ntohs(conn->session));
#if defined(TCSUPPORT_PPPOE_SIMULATE)
		char sim_tmp[8] = {0};
		
		tcapi_get("PPPoESimulate_Entry","PPPUnit", sim_tmp);
		if(req_unit == atoi(sim_tmp)){
			memset(sim_tmp, 0, sizeof(sim_tmp));
			sprintf(sim_tmp, "%d", ntohs(conn->session));
			tcapi_set("PPPoESimulate_Entry","PPPSessionID", sim_tmp);
		}
#endif 
#if defined(TCSUPPORT_CT_PPPOE_EMULATOR)
		tcapi_get("PppoeEmulator_Entry","PPPUnit", emulator_tmp);
		if(req_unit == atoi(emulator_tmp)){
			memset(emulator_tmp, 0, sizeof(emulator_tmp));
			sprintf(emulator_tmp, "%d", (int) ntohs(conn->session));
			tcapi_set("PppoeEmulator_Entry","PPPSessionID", emulator_tmp);
		}	
#endif

    /* RFC 2516 says session id MUST NOT be zero or 0xFFFF */
    if (ntohs(conn->session) == 0 || ntohs(conn->session) == 0xFFFF) {
	error("Access concentrator used a session value of %x -- the AC is violating RFC 2516", (unsigned int) ntohs(conn->session));
    }
}
#ifdef TCSUPPORT_PADT_BEFORE_PADI	
void
fileRead(char *path, char *buf, int size){
	int  fd=0;

	memset(buf,0, size);
	fd = open(path,O_RDONLY);
	if(fd == -1){
		return;
	}
	read(fd, buf, size);
	close(fd);
}/*end fileRead*/
#endif
/**********************************************************************
*%FUNCTION: discovery
*%ARGUMENTS:
* conn -- PPPoE connection info structure
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Performs the PPPoE discovery phase
***********************************************************************/
void
discovery(PPPoEConnection *conn)
{
    int padiAttempts = 0;
    int padrAttempts = 0;
    int timeout = conn->discoveryTimeout;
#ifdef TCSUPPORT_PADT_BEFORE_PADI
	unsigned char oldmac[ETH_ALEN];
	int oldsid;
	char tmpPath[32] = {0};
	char tmp[256] = {0};
#endif


    conn->discoverySocket =
	openInterface(conn->ifName, Eth_PPPOE_Discovery, conn->myEth);
#ifdef TCSUPPORT_PADT_BEFORE_PADI
	memset(tmpPath, 0, sizeof(tmpPath));
	memset(tmp, 0, sizeof(tmp));
	sprintf(tmpPath, "/tmp/pppsid-%s", conn->ifName);
	fileRead(tmpPath, tmp, sizeof(tmp));
	if(strlen(tmp) > 0){
		sscanf(tmp, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%d"
			,&oldmac[0]
			,&oldmac[1]
			,&oldmac[2]
			,&oldmac[3]
			,&oldmac[4]
			,&oldmac[5]
			,&oldsid);

		memcpy(conn->peerEth, oldmac, ETH_ALEN);
		conn->session = oldsid;
		sendPADT(conn, NULL);
	}
									
	remove(tmpPath);
#endif

#if defined(TCSUPPORT_CT_PON) && !defined(TCSUPPORT_CT_PON_SC)
	  if (maxfail < 10)
	  {
#if defined(TCSUPPORT_CT_PON_CZ_GD)
		retries = 10;
#else
		retries = 3;
#endif
#if defined(TCSUPPORT_CT_JOYME2)
		retries = -1; 
#endif
#if defined(TCSUPPORT_CT_PPPOE_EMULATOR)
		if ( isPPPEmulator() )
			retries = 1;
#endif
	  }
#endif
#if defined(TCSUPPORT_CT_JOYME)
		  retries = -1; // never end
#endif

    do {
	padiAttempts++;
#if !defined(TCSUPPORT_CZ_OTE) && !defined(TCSUPPORT_CZ_H168N)
#if defined(TCSUPPORT_STARTUP_OPTIMIZATION)
	if(shortflag == 0){
#endif
	if (padiAttempts > retries && retries != -1 ) {
	    warn("Timeout waiting for PADO packets");
	    close(conn->discoverySocket);
	    conn->discoverySocket = -1;
	    return;
	}

#if defined(TCSUPPORT_STARTUP_OPTIMIZATION)
	}
#endif
#endif
#if defined(TCSUPPORT_STARTUP_OPTIMIZATION)
	if(shortflag && sendPADTflag == 0){
		memcpy(conn->peerEth, shortoldmac, ETH_ALEN);
		conn->session = shortoldsid;
		sendPADT(conn, NULL);
	}
#endif

	sendPADI(conn);
	conn->discoveryState = STATE_SENT_PADI;
	waitForPADO(conn, timeout);

#if !defined(TCSUPPORT_CZ_GENERAL) 
#if defined(TCSUPPORT_STARTUP_OPTIMIZATION)
	if(shortflag == 0){
#endif
	timeout *= 2;
#if defined(TCSUPPORT_STARTUP_OPTIMIZATION)
	}
#endif
#endif
#if defined(TCSUPPORT_CT_JOYME) || defined(TCSUPPORT_CT_JOYME2)
			 ++retransmits;
			 if(wait_position_yh < max_coun_yh) {
				timeout = discover_timeout_yh[wait_position_yh];
				wait_position_yh++;
			 }
			 else {
				timeout = discover_timeout_yh[max_coun_yh-1];
			 }
#endif
    } while (conn->discoveryState == STATE_SENT_PADI);

    timeout = conn->discoveryTimeout;
    do {
	padrAttempts++;
#if !defined(TCSUPPORT_CZ_OTE) && !defined(TCSUPPORT_CZ_H168N)	
#if defined(TCSUPPORT_STARTUP_OPTIMIZATION)
	if(shortflag == 0){
#endif	
	if (padrAttempts > MAX_PADI_ATTEMPTS) {
	    warn("Timeout waiting for PADS packets");
	    close(conn->discoverySocket);
	    conn->discoverySocket = -1;
	    return;
	}
#if defined(TCSUPPORT_STARTUP_OPTIMIZATION)
	}
#endif
#endif
	
	sendPADR(conn);
	conn->discoveryState = STATE_SENT_PADR;
	waitForPADS(conn, timeout);
#if !defined(TCSUPPORT_CZ_OTE) 
#if defined(TCSUPPORT_STARTUP_OPTIMIZATION)
	if(shortflag == 0){
#endif
	timeout *= 2;
#if defined(TCSUPPORT_STARTUP_OPTIMIZATION)
	}
#endif
#endif
    } while (conn->discoveryState == STATE_SENT_PADR);

    /* We're done. */
    conn->discoveryState = STATE_SESSION;
    return;
}
