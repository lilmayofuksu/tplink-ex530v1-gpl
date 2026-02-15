#ifndef	__UAPI_INFO_IOCTL_H__
#define	__UAPI_INFO_IOCTL_H__

#define CMD_VPN_ADD_IP			(0)
#define CMD_VPN_GET_COUNT		(1)
#define CMD_VPN_CLEAR_IPS		(2)


#define Info_DEVNAME          	"vpn_info"
#define Info_MAJOR             	(249)
#define IP_TYPE					(1)
#define DOMAIN_TYPE 			(2)
#define MAX_IP_ADDR_SIZE		(32)


typedef struct _info_ioctl_data_s
{
	char ip[32];
	int vpn_entry_idx;
	/*domain_ip_type=1 represents ips_entry, domain_ip_type=2 represents domain_entry*/
	int domain_ip_type;
	int domain_ip_idx;
	int ip_mask;
	unsigned long long count;
}info_ioctl_data;

typedef struct _info_ioctl_return_data_s
{
	unsigned int vpn_entry_idx;
	unsigned long long *payload;
}info_ioctl_return_data;


#endif
