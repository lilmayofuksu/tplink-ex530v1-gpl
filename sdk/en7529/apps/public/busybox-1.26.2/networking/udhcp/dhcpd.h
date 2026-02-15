/* vi: set sw=4 ts=4: */
/*
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */
#ifndef UDHCP_DHCPD_H
#define UDHCP_DHCPD_H 1

PUSH_AND_SET_FUNCTION_VISIBILITY_TO_HIDDEN

#if defined(CWMP) && defined(TR111)
#include <unistd.h>
#endif

/* Defaults you may want to tweak */
/* Default max_lease_sec */
#define DEFAULT_LEASE_TIME      (60*60*24 * 10)
#define LEASES_FILE             CONFIG_DHCPD_LEASES_FILE
#define LEASE_PATH 				"/etc/udhcp_lease"
/* Where to find the DHCP server configuration file */
#define DHCPD_CONF_FILE         "/etc/udhcpd.conf"


struct static_lease {
	struct static_lease *next;
	uint32_t nip;
	uint8_t mac[6];
};

struct server_config_t {
	char *interface;                /* interface to use */
//TODO: ifindex, server_nip, server_mac
// are obtained from interface name.
// Instead of querying them *once*, create update_server_network_data_cache()
// and call it before any usage of these fields.
// update_server_network_data_cache() must re-query data
// if more than N seconds have passed after last use.
	int ifindex;
	uint32_t server_nip;
#if ENABLE_FEATURE_UDHCP_PORT
	uint16_t port;
#endif
	uint8_t server_mac[6];          /* our MAC address (used only for ARP probing) */
	struct option_set *options;     /* list of DHCP options loaded from the config file */
	/* start,end are in host order: we need to compare start <= ip <= end */
	uint32_t start_ip;              /* start address of leases, in host order */
	uint32_t end_ip;                /* end of leases, in host order */
	uint32_t max_lease_sec;         /* maximum lease time (host order) */
	uint32_t min_lease_sec;         /* minimum lease time a client can request */
	uint32_t max_leases;            /* maximum number of leases (including reserved addresses) */
	uint32_t auto_time;             /* how long should udhcpd wait before writing a config file.
	                                 * if this is zero, it will only write one on SIGUSR1 */
#if defined(DHCP_PROFILE)||(defined(CWMP) && defined(TR111))
	char remaining;	
#endif
	uint32_t decline_time;          /* how long an address is reserved if a client returns a
	                                 * decline message */
	uint32_t conflict_time;         /* how long an arp conflict offender is leased for */
	uint32_t offer_time;            /* how long an offered address is reserved */
	uint32_t siaddr_nip;            /* "next server" bootp option */
	char *lease_file;
	char *pidfile;
	char *notify_file;              /* what to run whenever leases are written */
	char *sname;                    /* bootp server name */
	char *boot_file;                /* bootp boot file option */
	struct static_lease *static_leases; /* List of ip/mac pairs to assign static leases */
} FIX_ALIASING;

#define server_config (*(struct server_config_t*)bb_common_bufsiz1)
/* client_config sits in 2nd half of bb_common_bufsiz1 */

#if defined(DHCP_PROFILE)||(defined(CWMP) && defined(TR111))
int add_option_string(uint8_t *optionptr, uint8_t *string);
#endif

#if ENABLE_FEATURE_UDHCP_PORT
#define SERVER_PORT  (server_config.port)
#define SERVER_PORT6 (server_config.port)
#else
#define SERVER_PORT  67
#define SERVER_PORT6 547
#endif


typedef uint32_t leasetime_t;
typedef int32_t signed_leasetime_t;

struct dyn_lease {
	/* Unix time when lease expires. Kept in memory in host order.
	 * When written to file, converted to network order
	 * and adjusted (current time subtracted) */
	leasetime_t expires;
	/* "nip": IP in network order */
	uint32_t lease_nip;
	/* We use lease_mac[6], since e.g. ARP probing uses
	 * only 6 first bytes anyway. We check received dhcp packets
	 * that their hlen == 6 and thus chaddr has only 6 significant bytes
	 * (dhcp packet has chaddr[16], not [6])
	 */
	uint8_t lease_mac[6];
	char hostname[20];
	uint8_t pad[2];
	/* total size is a multiply of 4 */
} PACKED;

#if defined(CWMP) && defined(TR111)
#define DEVICE_INFO_FILE         "/etc/devices.conf"
#define DEVICE_HOSTINDEX_INFO_FILE         "/etc/device_hostindex.conf"
#define ATTRIBUTE_ACTIVE 0x20
#define OPT_VENDOR_INFOR	0x7d 

#define DSL_FORUM_IANA_NUMBER	3561
#define DSL_FORUM_IANA_NUMBER_LENGTH	4
#ifdef TR111_TEST
#define DSL_FORUM_IANA_NUMBER_TEST	59661
#define DSL_FORUM_IANA_NUMBER_LENGTH_TEST	2
#endif

#define CLINETID_LENGTH 7
#define HOST_NAME_LEN 32

typedef struct ManageableDev_S
{
	char oui[7];
	char sn[65];
	char pclass[65];
	uint8_t clientID[CLINETID_LENGTH];
	uint32_t hostnum; 
	uint32_t hostindex[8];
	struct ManageableDev_S* next;	
}mDev_t;

typedef struct Tr111_Param_s
{
	uint32_t 	iTotalNumber; /* Number of Entries */
	uint32_t	iNotifyTimer; /* Notification timer */
	uint32_t  iNotifyFlag;
	mDev_t* VIHeader;	
	mDev_t* VITail;
} Tr111_Param_t;

int check_vi(struct dhcp_packet *packet);
void parseDev(struct dhcp_packet *packet, mDev_t *dev);
void insertDevice(struct dhcp_packet *packet);
int findDevice(struct dhcp_packet *packet,mDev_t* pdev);
void deleteDevice(struct dhcp_packet *packet);
void deleteDeviceByMac(uint8_t *macAddr);
unsigned long getDeviceNum(void);
void writeDevicesFile(void);
void writeHostIndexFile(void);
void cwmpShowmDev(void);
void dhcpRefresh(void);
void addVIOption(uint8_t *optionptr);
int lease_expired(struct dyn_lease *lease);

#endif

#ifdef DHCP_PROFILE

#define DHCPD_OPTION_CONF_FILE         "/etc/udhcpd_option.conf"
#define VENDOR_CLASS_ID_LEN_MAX	64
#define READ_CONFIG_BUF_SIZE 80

typedef struct Option60_Param_s{
	uint8_t enable;
	char *dhcp_opt60_vid;

} Option60_Param_s;

typedef struct Option_Param_t{
	uint8_t opt60_enable;
	char *opt60_vid;
	uint8_t opt240_enable;
	char *opt240_value;
} Option_Param_t;

void addOPT240Option(uint8_t *optionptr);

#endif

#if defined(CWMP)
void sendCwmpMsg(int msgType);
#endif

POP_SAVED_FUNCTION_VISIBILITY

#endif
