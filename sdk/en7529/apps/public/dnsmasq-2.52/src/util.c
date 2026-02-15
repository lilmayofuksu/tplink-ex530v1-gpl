/* dnsmasq is Copyright (c) 2000-2010 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991, or
   (at your option) version 3 dated 29 June, 2007.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
      
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* The SURF random number generator was taken from djbdns-1.05, by 
   Daniel J Bernstein, which is public domain. */


#include "dnsmasq.h"

#ifdef HAVE_BROKEN_RTC
#include <sys/times.h>
#endif

#ifdef LOCALEDIR
#include <idna.h>
#endif

#if defined(TCSUPPORT_CT_JOYME2)	
#include <modules/vpn_info/info_ioctl.h>

int vpn_ip_number[VPN_INSTANCE_NUM] 		= {0};
int vpn_domain_number[VPN_INSTANCE_NUM] 	= {0};
#endif

#ifdef HAVE_ARC4RANDOM
void rand_init(void)
{
  return;
}

unsigned short rand16(void)
{
   return (unsigned short) (arc4random() >> 15);
}

#else

/* SURF random number generator */

typedef unsigned int uint32;

static uint32 seed[32];
static uint32 in[12];
static uint32 out[8];

void rand_init()
{
  int fd = open(RANDFILE, O_RDONLY);
  
  if (fd == -1 ||
      !read_write(fd, (unsigned char *)&seed, sizeof(seed), 1) ||
      !read_write(fd, (unsigned char *)&in, sizeof(in), 1))
    die(_("failed to seed the random number generator: %s"), NULL, EC_MISC);
  
  close(fd);
}

#define ROTATE(x,b) (((x) << (b)) | ((x) >> (32 - (b))))
#define MUSH(i,b) x = t[i] += (((x ^ seed[i]) + sum) ^ ROTATE(x,b));

static void surf(void)
{
  uint32 t[12]; uint32 x; uint32 sum = 0;
  int r; int i; int loop;

  for (i = 0;i < 12;++i) t[i] = in[i] ^ seed[12 + i];
  for (i = 0;i < 8;++i) out[i] = seed[24 + i];
  x = t[11];
  for (loop = 0;loop < 2;++loop) {
    for (r = 0;r < 16;++r) {
      sum += 0x9e3779b9;
      MUSH(0,5) MUSH(1,7) MUSH(2,9) MUSH(3,13)
      MUSH(4,5) MUSH(5,7) MUSH(6,9) MUSH(7,13)
      MUSH(8,5) MUSH(9,7) MUSH(10,9) MUSH(11,13)
    }
    for (i = 0;i < 8;++i) out[i] ^= t[i + 4];
  }
}

unsigned short rand16(void)
{
  static int outleft = 0;

  if (!outleft) {
    if (!++in[0]) if (!++in[1]) if (!++in[2]) ++in[3];
    surf();
    outleft = 8;
  }

  return (unsigned short) out[--outleft];
}

#endif

static int check_name(char *in)
{
  /* remove trailing . 
     also fail empty string and label > 63 chars */
  size_t dotgap = 0, l = strlen(in);
  char c;
  int nowhite = 0;
  
  if (l == 0 || l > MAXDNAME) return 0;
  
  if (in[l-1] == '.')
    {
      if (l == 1) return 0;
      in[l-1] = 0;
    }
  
  for (; (c = *in); in++)
    {
      if (c == '.')
	dotgap = 0;
      else if (++dotgap > MAXLABEL)
	return 0;
      else if (isascii(c) && iscntrl(c)) 
	/* iscntrl only gives expected results for ascii */
	return 0;
#ifndef LOCALEDIR
      else if (!isascii(c))
	return 0;
#endif
      else if (c != ' ')
	nowhite = 1;
    }

  if (!nowhite)
    return 0;

  return 1;
}

/* Hostnames have a more limited valid charset than domain names
   so check for legal char a-z A-Z 0-9 - _ 
   Note that this may receive a FQDN, so only check the first label 
   for the tighter criteria. */
int legal_hostname(char *name)
{
  char c;

  if (!check_name(name))
    return 0;

  for (; (c = *name); name++)
    /* check for legal char a-z A-Z 0-9 - _ . */
    {
      if ((c >= 'A' && c <= 'Z') ||
	  (c >= 'a' && c <= 'z') ||
	  (c >= '0' && c <= '9') ||
	  c == '-' || c == '_')
	continue;
      
      /* end of hostname part */
      if (c == '.')
	return 1;
      
      return 0;
    }
  
  return 1;
}
  
char *canonicalise(char *in, int *nomem)
{
  char *ret = NULL;
#ifdef LOCALEDIR
  int rc;
#endif

  if (nomem)
    *nomem = 0;
  
  if (!check_name(in))
    return NULL;
  
#ifdef LOCALEDIR
  if ((rc = idna_to_ascii_lz(in, &ret, 0)) != IDNA_SUCCESS)
    {
      if (ret)
	free(ret);

      if (nomem && (rc == IDNA_MALLOC_ERROR || rc == IDNA_DLOPEN_ERROR))
	{
	  my_syslog(LOG_ERR, _("failed to allocate memory"));
	  *nomem = 1;
	}
    
      return NULL;
    }
#else
  if ((ret = whine_malloc(strlen(in)+1)))
    strcpy(ret, in);
  else if (nomem)
    *nomem = 1;    
#endif

  return ret;
}

unsigned char *do_rfc1035_name(unsigned char *p, char *sval)
{
  int j;
  
  while (sval && *sval)
    {
      unsigned char *cp = p++;
      for (j = 0; *sval && (*sval != '.'); sval++, j++)
	*p++ = *sval;
      *cp  = j;
      if (*sval)
	sval++;
    }
  return p;
}

/* for use during startup */
void *safe_malloc(size_t size)
{
  void *ret = malloc(size);
  
  if (!ret)
    die(_("could not get memory"), NULL, EC_NOMEM);
     
  return ret;
}    

void safe_pipe(int *fd, int read_noblock)
{
  if (pipe(fd) == -1 || 
      !fix_fd(fd[1]) ||
      (read_noblock && !fix_fd(fd[0])))
    die(_("cannot create pipe: %s"), NULL, EC_MISC);
}

void *whine_malloc(size_t size)
{
  void *ret = malloc(size);

  if (!ret)
    my_syslog(LOG_ERR, _("failed to allocate %d bytes"), (int) size);

  return ret;
}

int sockaddr_isequal(union mysockaddr *s1, union mysockaddr *s2)
{
  if (s1->sa.sa_family == s2->sa.sa_family)
    { 
      if (s1->sa.sa_family == AF_INET &&
	  s1->in.sin_port == s2->in.sin_port &&
	  s1->in.sin_addr.s_addr == s2->in.sin_addr.s_addr)
	return 1;
#ifdef HAVE_IPV6      
      if (s1->sa.sa_family == AF_INET6 &&
	  s1->in6.sin6_port == s2->in6.sin6_port &&
	  IN6_ARE_ADDR_EQUAL(&s1->in6.sin6_addr, &s2->in6.sin6_addr))
	return 1;
#endif
    }
  return 0;
}

int sa_len(union mysockaddr *addr)
{
#ifdef HAVE_SOCKADDR_SA_LEN
  return addr->sa.sa_len;
#else
#ifdef HAVE_IPV6
  if (addr->sa.sa_family == AF_INET6)
    return sizeof(addr->in6);
  else
#endif
    return sizeof(addr->in); 
#endif
}

/* don't use strcasecmp and friends here - they may be messed up by LOCALE */
int hostname_isequal(char *a, char *b)
{
  unsigned int c1, c2;
  
  do {
    c1 = (unsigned char) *a++;
    c2 = (unsigned char) *b++;
    
    if (c1 >= 'A' && c1 <= 'Z')
      c1 += 'a' - 'A';
    if (c2 >= 'A' && c2 <= 'Z')
      c2 += 'a' - 'A';
    
    if (c1 != c2)
      return 0;
  } while (c1);
  
  return 1;
}
    
time_t dnsmasq_time(void)
{
#ifdef HAVE_BROKEN_RTC
  struct tms dummy;
  static long tps = 0;

  if (tps == 0)
    tps = sysconf(_SC_CLK_TCK);

  return (time_t)(times(&dummy)/tps);
#else
  return time(NULL);
#endif
}

int is_same_net(struct in_addr a, struct in_addr b, struct in_addr mask)
{
  return (a.s_addr & mask.s_addr) == (b.s_addr & mask.s_addr);
} 

/* returns port number from address */
int prettyprint_addr(union mysockaddr *addr, char *buf)
{
  int port = 0;
  
#ifdef HAVE_IPV6
  if (addr->sa.sa_family == AF_INET)
    {
      inet_ntop(AF_INET, &addr->in.sin_addr, buf, ADDRSTRLEN);
      port = ntohs(addr->in.sin_port);
    }
  else if (addr->sa.sa_family == AF_INET6)
    {
      inet_ntop(AF_INET6, &addr->in6.sin6_addr, buf, ADDRSTRLEN);
      port = ntohs(addr->in6.sin6_port);
    }
#else
  strcpy(buf, inet_ntoa(addr->in.sin_addr));
  port = ntohs(addr->in.sin_port); 
#endif
  
  return port;
}

void prettyprint_time(char *buf, unsigned int t)
{
  if (t == 0xffffffff)
    sprintf(buf, _("infinite"));
  else
    {
      unsigned int x, p = 0;
       if ((x = t/86400))
	p += sprintf(&buf[p], "%dd", x);
       if ((x = (t/3600)%24))
	p += sprintf(&buf[p], "%dh", x);
      if ((x = (t/60)%60))
	p += sprintf(&buf[p], "%dm", x);
      if ((x = t%60))
	p += sprintf(&buf[p], "%ds", x);
    }
}


/* in may equal out, when maxlen may be -1 (No max len). */
int parse_hex(char *in, unsigned char *out, int maxlen, 
	      unsigned int *wildcard_mask, int *mac_type)
{
  int mask = 0, i = 0;
  char *r;
    
  if (mac_type)
    *mac_type = 0;
  
  while (maxlen == -1 || i < maxlen)
    {
      for (r = in; *r != 0 && *r != ':' && *r != '-'; r++);
      if (*r == 0)
	maxlen = i;
      
      if (r != in )
	{
	  if (*r == '-' && i == 0 && mac_type)
	   {
	      *r = 0;
	      *mac_type = strtol(in, NULL, 16);
	      mac_type = NULL;
	   }
	  else
	    {
	      *r = 0;
	      mask = mask << 1;
	      if (strcmp(in, "*") == 0)
		mask |= 1;
	      else
		out[i] = strtol(in, NULL, 16);
	      i++;
	    }
	}
      in = r+1;
    }
  
  if (wildcard_mask)
    *wildcard_mask = mask;

  return i;
}

/* return 0 for no match, or (no matched octets) + 1 */
int memcmp_masked(unsigned char *a, unsigned char *b, int len, unsigned int mask)
{
  int i, count;
  for (count = 1, i = len - 1; i >= 0; i--, mask = mask >> 1)
    if (!(mask & 1))
      {
	if (a[i] == b[i])
	  count++;
	else
	  return 0;
      }
  return count;
}

/* _note_ may copy buffer */
int expand_buf(struct iovec *iov, size_t size)
{
  void *new;

  if (size <= (size_t)iov->iov_len)
    return 1;

  if (!(new = whine_malloc(size)))
    {
      errno = ENOMEM;
      return 0;
    }

  if (iov->iov_base)
    {
      memcpy(new, iov->iov_base, iov->iov_len);
      free(iov->iov_base);
    }

  iov->iov_base = new;
  iov->iov_len = size;

  return 1;
}

char *print_mac(char *buff, unsigned char *mac, int len)
{
  char *p = buff;
  int i;
   
  if (len == 0)
    sprintf(p, "<null>");
  else
    for (i = 0; i < len; i++)
      p += sprintf(p, "%.2x%s", mac[i], (i == len - 1) ? "" : ":");
  
  return buff;
}

void bump_maxfd(int fd, int *max)
{
  if (fd > *max)
    *max = fd;
}

int retry_send(void)
{
   struct timespec waiter;
   if (errno == EAGAIN)
     {
       waiter.tv_sec = 0;
       waiter.tv_nsec = 10000;
       nanosleep(&waiter, NULL);
       return 1;
     }
   
   if (errno == EINTR)
     return 1;

   return 0;
}

int read_write(int fd, unsigned char *packet, int size, int rw)
{
  ssize_t n, done;
  
  for (done = 0; done < size; done += n)
    {
    retry:
      if (rw)
        n = read(fd, &packet[done], (size_t)(size - done));
      else
        n = write(fd, &packet[done], (size_t)(size - done));

      if (n == 0)
        return 0;
      else if (n == -1)
        {
          if (retry_send() || errno == ENOMEM || errno == ENOBUFS)
            goto retry;
          else
            return 0;
        }
    }
  return 1;
}

#if defined(TCSUPPORT_CT_L2TP_VPN)

/* vpn dns group header */
struct vpn_dns_group *gVPNDNSGrp = NULL;
/* delete vpn dns group temp header */
struct vpn_dns_group *gDelTEMP_VPNDNSGrp = NULL;

/*
check delete domain, return 0 if domain match
*/
int checkDelDomain(char *domain, int vpn_entry)
{
	struct vpn_dns_group *cur_vpndns_group = NULL;

	if ( !domain )
		return -1;

	for ( cur_vpndns_group = gDelTEMP_VPNDNSGrp; cur_vpndns_group;
		cur_vpndns_group = cur_vpndns_group->next )
	{
		if ( vpn_entry == cur_vpndns_group->vpn_tunnel_idx
			&& 0 == strcmp(domain, cur_vpndns_group->domain) )
			return 0;
	}

	return -2;
}
/*
free delete domain temp list
*/
int freeDelDomain()
{
	struct vpn_dns_group *start = NULL;
	struct vpn_dns_group *temp = NULL;

	if ( !gDelTEMP_VPNDNSGrp )
		return -1;

	for ( start = gDelTEMP_VPNDNSGrp; start;  )
	{
		temp = start->next;
		if ( start->domain )
			free(start->domain);
		free(start);
		start = temp;
	}

	return 0;
}

#if defined(TCSUPPORT_CT_JOYME2)
int get_vpn_dns_list(char *domain_node, int idx, struct vpn_dns_group ** new_dns_vpn_group)
{
	int domain_idx = 0;
	char ipdomain[256] = {0};
	char attrname[64] = {0};
	int dnsbufLen = 0;
	
	/* get domain list */		
	for ( domain_idx = 0; domain_idx < VPN_ATTACH_NUM; domain_idx ++)
	{
		bzero(ipdomain, sizeof(ipdomain));
		snprintf(attrname, sizeof(attrname) - 1
			, "domain%d", domain_idx);
		if ( 0 == tcapi_get(domain_node, attrname, ipdomain)
			&& 0 != ipdomain[0] )
		{
			if ( !(*new_dns_vpn_group) )
				*new_dns_vpn_group =
				safe_malloc(sizeof(struct vpn_dns_group));

			/* malloc group failed. */
			if ( !(*new_dns_vpn_group) )
				return -1;

			(*new_dns_vpn_group)->domain = NULL;
			dnsbufLen = strlen(ipdomain) + 1;
			(*new_dns_vpn_group)->domain = safe_malloc(dnsbufLen);

			/* copy domain from node. */
			bzero((*new_dns_vpn_group)->domain, dnsbufLen);
			strncpy((*new_dns_vpn_group)->domain
				, ipdomain, dnsbufLen);
			(*new_dns_vpn_group)->vpn_tunnel_idx = idx;
			(*new_dns_vpn_group)->domain_name_idx = domain_idx;

			(*new_dns_vpn_group)->next = NULL;
			new_dns_vpn_group = &((*new_dns_vpn_group)->next);
		}
		else
		{
			break;
		}
	}

	return 0;
}

int get_vpn_ip_list(char* ips_node, int idx)
{
	int ips_idx = 0;
	char ipdomain[256] = {0};
	char attrname[64] = {0};
	
	for ( ips_idx = 0; ips_idx < VPN_ATTACH_NUM; ips_idx ++)
	{
		bzero(ipdomain, sizeof(ipdomain));
		snprintf(attrname, sizeof(attrname) - 1
			, "ips%d", ips_idx);
		if ( 0 == tcapi_get(ips_node, attrname, ipdomain)
			&& 0 != ipdomain[0] )
		{
			/* check domain type, continue if IP type address */
			if ( 0 == checkDomainType(ipdomain, idx, ips_idx) )
				continue;
		}
		else
		{
			break;
		}
	}

	return 0;
}

int get_vpn_mac_list(char* mac_node, int idx)
{
	int mac_idx = 0;
	char ipdomain[256] = {0};
	char attrname[64] = {0};
	
	for (mac_idx = 0; mac_idx < VPN_ATTACH_NUM; mac_idx ++)
	{
		bzero(ipdomain, sizeof(ipdomain));
		snprintf(attrname, sizeof(attrname) - 1
			, "terminal_mac%d", mac_idx);
		if ( 0 == tcapi_get(mac_node, attrname, ipdomain)
			&& 0 != ipdomain[0] )
		{
			/*set mac ebtables rule */
			addMacHost2VPNEntry(ipdomain, idx);
		}
		else
		{
			break;
		}
	}
}

int dnsmasq_notify_info_utility(char *ipaddr, int ip_mask, int tunnel, 
									int ips_domain_type, int domain_name_id)
{
	int fd;
	info_ioctl_data info_data;
	char vpn_entry[32] = {0};
	char ip_domain_number[8] = {0};

	/*set the ip numbers and domains numbers into vpn entry*/
	snprintf(vpn_entry, sizeof(vpn_entry), "VPN_Entry%d", tunnel);
	
	if (IP_TYPE == ips_domain_type)
	{
		vpn_ip_number[tunnel]++;
		snprintf(ip_domain_number, sizeof(ip_domain_number), "%d", vpn_ip_number[tunnel]);
		tcapi_set(vpn_entry, "vpn_ip_number", ip_domain_number);	
	}
	else if (DOMAIN_TYPE == ips_domain_type)
	{
		if (vpn_domain_number[tunnel] < domain_name_id + 1)
		{
			vpn_domain_number[tunnel] = domain_name_id + 1;
		}	
		snprintf(ip_domain_number, sizeof(ip_domain_number), "%d", vpn_domain_number[tunnel]);
		tcapi_set(vpn_entry, "vpn_domain_number", ip_domain_number);
	}

	/*notify info_utility modules by ioctl*/
	fd = open("/dev/vpn_info", O_RDWR);
	if (fd < 0)
	{
		tcdbg_printf("Open /dev/vpn_info fail.\n");
		return -1;
	}

	memset(&info_data, 0, sizeof(info_ioctl_data));
	strncpy(info_data.ip, ipaddr, strlen(ipaddr));
	info_data.vpn_entry_idx = tunnel;
	info_data.domain_ip_type = ips_domain_type;
	info_data.domain_ip_idx = domain_name_id;
	info_data.ip_mask = ip_mask;
	info_data.count = 0;
	if (0 != ioctl(fd, CMD_VPN_ADD_IP, &info_data))
	{
		printf("Info ioctl fail.\n");
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}
#endif


/*
init vpn dns data.
*/
int initVPNDnsGroup()
{
	char nodename[64] = {0}, attrname[64] = {0};
	char ipdomain[256] = {0}, svripAddr[64] = {0};
	char vpnentrybuf[16] = {0};
	int idx = 0, domain_idx = 0, dnsbufLen = 0, vpn_entryidx = 0;
	char cmdbuf[256] = {0};
	struct vpn_dns_group **new_dns_vpn_group = NULL;
	struct vpn_dns_group **new_del_dns_vpn_group = NULL;
#if defined(TCSUPPORT_CT_JOYME2)
	char domain_node[64] = {0};
	char ips_node[64] = {0};
	char mac_node[64] = {0};
	char attach_mode[32] = {0};
	char vpn_port[8] = {0};
#endif

	new_dns_vpn_group = &gVPNDNSGrp;
	new_del_dns_vpn_group = &gDelTEMP_VPNDNSGrp;

	/* clear vpn chains for LAN */
	for ( idx = VPN_INSTANCE_NUM - 1; idx >=0; idx -- )
	{
		/* flush and zero it */
		bzero(cmdbuf, sizeof(cmdbuf));
		snprintf(cmdbuf, sizeof(cmdbuf) - 1
			, "ebtables -t broute -F vpn_lan_entry%d", idx);
		system(cmdbuf);

		bzero(cmdbuf, sizeof(cmdbuf));
		snprintf(cmdbuf, sizeof(cmdbuf) - 1
			, "ebtables -t broute -Z vpn_lan_entry%d", idx);
		system(cmdbuf);
	}

	/*
	  init delete vpn dns list
	*/
	bzero(nodename, sizeof(nodename));
	snprintf(nodename, sizeof(nodename) - 1
		, "%s", VPN_DELENTRY);
	for ( idx = 0; idx < VPN_ATTACH_NUM; idx ++)
	{
		snprintf(attrname, sizeof(attrname) - 1
			, "ipDomainNameAddr%d", idx);
		bzero(ipdomain, sizeof(ipdomain));
		if ( 0 != 
			tcapi_get(nodename, attrname, ipdomain)
			|| 0 == ipdomain[0] )
			continue;
		snprintf(attrname, sizeof(attrname) - 1
			, "ipDomainNameIdx%d", idx);
		bzero(vpnentrybuf, sizeof(vpnentrybuf));
		if ( 0 != 
			tcapi_get(nodename, attrname, vpnentrybuf)
			|| 0 == vpnentrybuf[0] )
			continue;
		vpn_entryidx = atoi(vpnentrybuf);
		if ( vpn_entryidx < 0 || vpn_entryidx >= VPN_INSTANCE_NUM )
			continue;

		if ( !(*new_del_dns_vpn_group) )
			*new_del_dns_vpn_group =
			safe_malloc(sizeof(struct vpn_dns_group));

		/* malloc group failed. */
		if ( !(*new_del_dns_vpn_group) )
			return -1;

		(*new_del_dns_vpn_group)->domain = NULL;
		dnsbufLen = strlen(ipdomain) + 1;
		(*new_del_dns_vpn_group)->domain = safe_malloc(dnsbufLen);

		/* copy domain from node. */
		bzero((*new_del_dns_vpn_group)->domain, dnsbufLen);
		strncpy((*new_del_dns_vpn_group)->domain
			, ipdomain, dnsbufLen);
		(*new_del_dns_vpn_group)->vpn_tunnel_idx = vpn_entryidx;

		(*new_del_dns_vpn_group)->next = NULL;
		new_del_dns_vpn_group = &((*new_del_dns_vpn_group)->next);
	}

	/*
	  init vpn dns group.
	*/
#if defined(TCSUPPORT_CT_JOYME2)
	for ( idx = 0; idx < VPN_INSTANCE_NUM; idx ++)
	{
		snprintf(nodename, sizeof(nodename) - 1
			, VPN_ENTRY"%d", idx);

		bzero(attach_mode, sizeof(attach_mode));
		if (0 != tcapi_get(nodename, "attach_mode", attach_mode))
		{
			continue;
		}

		tcapi_get(nodename, "vpn_port", vpn_port);
		if ( 0 == strncmp(vpn_port, "1701", 4))
		{
			if ((0 == strcmp(attach_mode, "0"))
				|| (0 == strcmp(attach_mode, "1")))
			{
				/* get domain list */
				snprintf(domain_node, sizeof(domain_node) - 1
				, VPNDOAMIN_ENTRY"%d", idx);
				get_vpn_dns_list(domain_node, idx, new_dns_vpn_group);

				/*get ip list*/
				snprintf(ips_node, sizeof(ips_node) - 1
				, VPNIPS_ENTRY"%d", idx);
				get_vpn_ip_list(ips_node, idx);
				
			}

			if ((0 == strcmp(attach_mode, "0"))
				|| (0 == strcmp(attach_mode, "2")))
			{
				/*get mac list*/
				snprintf(mac_node, sizeof(mac_node) - 1
				, VPNMAC_ENTRY"%d", idx);
				get_vpn_mac_list(mac_node, idx);
				
			}
		}
	}
#else
	for ( idx = 0; idx < VPN_INSTANCE_NUM; idx ++)
	{
		snprintf(nodename, sizeof(nodename) - 1
			, VPN_ENTRY"%d", idx);

		bzero(svripAddr, sizeof(svripAddr));
		if ( 0 != 
			tcapi_get(nodename, "serverIpAddr", svripAddr)
			|| 0 == svripAddr[0] )
			continue;

		/* get domain list */
		for ( domain_idx = 0; domain_idx < VPN_ATTACH_NUM; domain_idx ++)
		{
			bzero(ipdomain, sizeof(ipdomain));
			snprintf(attrname, sizeof(attrname) - 1
				, "ipDomainNameAddr%d", domain_idx);
			if ( 0 == 
				tcapi_get(nodename, attrname, ipdomain)
				&& 0 != ipdomain[0] )
			{
				/* delete domains which need to be deleted */
				if ( 0 == checkDelDomain(ipdomain, idx) )
				{
					tcapi_set(nodename, attrname, "");
					continue;
				}

				/* check domain type, continue if IP type address */
				if ( 0 == checkDomainType(ipdomain, idx, domain_idx) )
					continue;

				if ( !(*new_dns_vpn_group) )
					*new_dns_vpn_group =
					safe_malloc(sizeof(struct vpn_dns_group));

				/* malloc group failed. */
				if ( !(*new_dns_vpn_group) )
					return -1;

				(*new_dns_vpn_group)->domain = NULL;
				dnsbufLen = strlen(ipdomain) + 1;
				(*new_dns_vpn_group)->domain = safe_malloc(dnsbufLen);

				/* copy domain from node. */
				bzero((*new_dns_vpn_group)->domain, dnsbufLen);
				strncpy((*new_dns_vpn_group)->domain
					, ipdomain, dnsbufLen);
				(*new_dns_vpn_group)->vpn_tunnel_idx = idx;
				(*new_dns_vpn_group)->domain_name_idx = domain_idx;

				(*new_dns_vpn_group)->next = NULL;
				new_dns_vpn_group = &((*new_dns_vpn_group)->next);
			}
		}	
	}
#endif
	bzero(nodename, sizeof(nodename));
	snprintf(nodename, sizeof(nodename) - 1
		, "%s", VPN_DELENTRY);
	tcapi_unset(nodename);
	tcapi_save();
	freeDelDomain();

	return 0;
}

/*
init vpn dns data.
*/
int domain_reg_match(char* domain, char* namebuf)
{
	int cflags = REG_EXTENDED, status = -1;
	int chidx = 0;
	const size_t nmatch = 1;
	regex_t reg;
	regmatch_t pmatch[1];
	char pattern[MAXDNAME] = {0};

	if (domain)
	{
		bzero(pattern, sizeof(pattern));
		chidx = 0;
		/* remove regx character "\" as C not support it */
		while ( *domain )
		{
			if ( '\\' != *domain )
				pattern[chidx++] = *domain;
			domain ++;
		}

		if ( 0 == pattern[0] )
			return -1;
				
		/* 1. compile regx */
		if ( regcomp(&reg, pattern, cflags) < 0 )
		{
			tcdbg_printf("\n[%s]==>regcomp err occur\n"
				,__FUNCTION__);
			return -1;
		}
		/* 2. match it */
		status = regexec(&reg, namebuf, nmatch, pmatch, 0);
		/* 3.  free it */
		regfree(&reg);
	}

	return status;
}

int vpnDNSTunnelHandle
(char *domain, struct all_addr *addr, int addrlen, int flags)
{
	int cflags = REG_EXTENDED, status = 0, match_domain = 0;
	int vpn_entry_idx = 0, chidx = 0;
	int domain_name_id = 0;
	const size_t nmatch = 1;
	regex_t reg;
	regmatch_t pmatch[1];
	char pattern[MAXDNAME] = {0};
	struct vpn_dns_group *cur_vpndns_group = NULL;
	int ips_domain_type = 0;

	if ( !domain || !addr )
		return -1;

	if ( !(flags & F_IPV4) )
	{
		return -2;
	}

	for ( cur_vpndns_group = gVPNDNSGrp; cur_vpndns_group;
		cur_vpndns_group = cur_vpndns_group->next )
	{
		status = -1;
		if ( !(cur_vpndns_group->domain) )
			continue;

		if (0 == domain_reg_match(cur_vpndns_group->domain, domain))
		{
			match_domain = 1;
			vpn_entry_idx = cur_vpndns_group->vpn_tunnel_idx;
			domain_name_id = cur_vpndns_group->domain_name_idx;
			break;
		}
	}

	if ( match_domain )
	{
#if defined(TCSUPPORT_CT_JOYME2)
		ips_domain_type = DOMAIN_TYPE;
#endif
		if ( flags & F_IPV4 )
			return addIPv4Host2VPNEntry(addr, 32, vpn_entry_idx, ips_domain_type, domain_name_id);

		return 0;
	}

	return -20;
}

/*
add resolve IPv4 address to VPN LAN Entry.
*/
int addIPv4Host2VPNEntry
(struct all_addr *addr, int mask, int tunnel, int ips_domain_type, int domain_name_id)
{
	char ipaddr[64] = {0};
#if 0
	char cmdbuf[256] = {0};
	int tunnel_mark = 0;
	const char *ebtfrmt =
		"ebtables -t broute -%s vpn_lan_entry%d -p IPv4 "
		"--ip-dst %s/%d "
		"-j mark --mark-or 0x%x --mark-target CONTINUE";
#define VPN_PPP_FWMAKR_START 0x6e0000
#endif
	if ( !addr || tunnel < 0 || tunnel >= VPN_INSTANCE_NUM )
		return -1;
#if 0
	tunnel_mark = tunnel << 16;
#endif
	bzero(ipaddr, sizeof(ipaddr));
	inet_ntop(AF_INET, addr
			, ipaddr, sizeof(ipaddr));

	if ( 0 == ipaddr[0] )
		return -2;
#if 0
	/* delete old rule and add it */
	bzero(cmdbuf, sizeof(cmdbuf));
	snprintf(cmdbuf, sizeof(cmdbuf) - 1
		, ebtfrmt
		, "D"
		, tunnel
		, ipaddr, mask
		, VPN_PPP_FWMAKR_START + tunnel_mark);
	system(cmdbuf);

	bzero(cmdbuf, sizeof(cmdbuf));
	snprintf(cmdbuf, sizeof(cmdbuf) - 1
		, ebtfrmt
		, "A"
		, tunnel
		, ipaddr, mask
		, VPN_PPP_FWMAKR_START + tunnel_mark);
	system(cmdbuf);
#endif	
#if defined(TCSUPPORT_CT_JOYME2)	
	dnsmasq_notify_info_utility(ipaddr, mask, tunnel, ips_domain_type, domain_name_id);
#endif

	return 0;
}

#if defined(TCSUPPORT_CT_JOYME2)
void mac_format_transform(char* dest, char* src)
{
	int i = 0;
	int j = 0;
	int num  = 0;
	
	if ((NULL == dest) || (NULL == src))
	{
		return ;
	}

	while (('\0' != src[i]) && ('\n' != src[i]))
	{
		if (2 == num)
		{
			dest[j] = ':';
			j++;
			num = 0;
			continue;
		}
		dest[j] = src[i];
		num++;
		i++;
		j++;
	}
	dest[j] = '\0';

	return ;
}

/*add mac address to VPN LAN Entry.*/
int addMacHost2VPNEntry
(char *mac_addr, int tunnel)
{
	char cmdbuf[256] = {0};
	char mac[32] = {0};
	int tunnel_mark = 0;
	const char *ebtfrmt =
		"ebtables -t broute -%s vpn_lan_entry%d -s %s "
		"-j mark --mark-or 0x%x --mark-target CONTINUE";
#if 0
	const char *dns_ebtfrmt =
		"ebtables -t broute -%s vpn_lan_entry%d "
		"-p ipv4 --ip-proto 17 --ip-dport 53 -j RETURN";
#endif

#define VPN_PPP_FWMAKR_START 0x6e0000

	if ( tunnel < 0 || tunnel >= VPN_INSTANCE_NUM )
		return -1;

	tunnel_mark = tunnel << 16;

	mac_format_transform(mac, mac_addr);

	/* delete old rule and add it */
#if 0
	bzero(cmdbuf, sizeof(cmdbuf));
	snprintf(cmdbuf, sizeof(cmdbuf) - 1
		, dns_ebtfrmt
		, "D"
		, tunnel);
	system(cmdbuf);
#endif
	
	bzero(cmdbuf, sizeof(cmdbuf));
	snprintf(cmdbuf, sizeof(cmdbuf) - 1
		, ebtfrmt
		, "D"
		, tunnel
		, mac
		, VPN_PPP_FWMAKR_START + tunnel_mark);
	system(cmdbuf);

#if 0
	bzero(cmdbuf, sizeof(cmdbuf));
	snprintf(cmdbuf, sizeof(cmdbuf) - 1
		, dns_ebtfrmt
		, "A"
		, tunnel);
	system(cmdbuf);
#endif
	
	bzero(cmdbuf, sizeof(cmdbuf));
	snprintf(cmdbuf, sizeof(cmdbuf) - 1
		, ebtfrmt
		, "A"
		, tunnel
		, mac
		, VPN_PPP_FWMAKR_START + tunnel_mark);
	system(cmdbuf);
	
	return 0;
}

#endif

/*
check domain type, return 0 if domain is IP type
*/
int checkDomainType(char *domain, int tunnel, int domain_name_idx)
{
	char partA[MAXDNAME] = {0}, partB[MAXDNAME] = {0};
	int mask = 0;
	unsigned int addr_idx = 0, laddr_l = 0, laddr_r = 0;
	int ips_domain_type = 0;
	unsigned int tmp = 0;

	if ( !domain )
		return -1;

	bzero(partA, sizeof(partA));
	bzero(partB, sizeof(partB));

	
#if defined(TCSUPPORT_CT_JOYME2)	
	ips_domain_type = IP_TYPE;
#endif

	/* chek type 1: xxxx.xxxx.xxxx.xxxx */
	if ( 1 == inet_pton(AF_INET, domain, &laddr_l) )
	{
		addIPv4Host2VPNEntry(&laddr_l, 32, tunnel, ips_domain_type, domain_name_idx);
		return 0;
	}

	/* chek type 2: xxxx.xxxx.xxxx.xxxx/xx */
	sscanf(domain, "%[^/]/%d", partA, &mask);
	if ( 1 == inet_pton(AF_INET, partA, &laddr_l)
		&& mask > 1 && mask <= 32 )
	{
		addIPv4Host2VPNEntry(&laddr_l, mask, tunnel, ips_domain_type, domain_name_idx);
		return 0;
	}

	/* chek type 3: xxxx.xxxx.xxxx.xxxx-xxxx.xxxx.xxxx.xxxx */
	sscanf(domain, "%[^-]-%s", partA, partB);
	if ( 1 == inet_pton(AF_INET, partA, &laddr_l)
		&& 1 == inet_pton(AF_INET, partB, &laddr_r)
		&& (0 == memcmp(&laddr_l, &laddr_r, 3)
			|| 0 == memcmp(&laddr_l, &laddr_r, 2))		
		)
	{
		tmp = htonl(1);
		for ( addr_idx = laddr_l; addr_idx < laddr_r; addr_idx += tmp)
		{
			addIPv4Host2VPNEntry(&addr_idx, 32, tunnel, ips_domain_type, domain_name_idx);
		}
		
		if (addr_idx == laddr_r)
		{
			addIPv4Host2VPNEntry(&addr_idx, 32, tunnel, ips_domain_type, domain_name_idx);
		}

		return 0;
	}

	return -2;
}


#endif

#if defined(TCSUPPORT_CT_VLAN_BIND)
static int g_VlanBindGroup[MAX_LAN_PORT_NUM];
int initVlanBindGroup()
{
	int idx = 0;
	char wan_node[32] = {0}, vbindActive[6] = {0};

	bzero(g_VlanBindGroup, sizeof(g_VlanBindGroup));
	for ( idx = 0; idx < MAX_LAN_PORT_NUM; idx ++ )
	{
		snprintf(wan_node, sizeof(wan_node), "VlanBind_Entry%d", idx);
		if ( 0 == tcapi_get(wan_node, "Active", vbindActive)
			&& 0 == strcmp(vbindActive, "Yes") )
			g_VlanBindGroup[idx] = 1;
	}

	return 0;
}

int get_VlanBindByPort(int portid)
{
	if ( portid < 0 || portid >= MAX_LAN_PORT_NUM )
		return 0;

	return g_VlanBindGroup[portid];
}
#ifdef TCSUPPORT_CMCCV2
static struct wan_portbind_group g_PortBindGroup[MAX_SMUX_NUM];
#else
static struct wan_portbind_group g_PortBindGroup[PVC_NUM * MAX_SMUX_NUM];
#endif
int initPortBindGroup()
{
#ifdef TCSUPPORT_CMCCV2
	int pvc_num = 1;
#else
	int pvc_num = PVC_NUM;
#endif
	int pvc = 0, entry = 0, wan_index = 0, lan_idx = 0, wan_idx = 0;
	char wan_node[32] = {0}, lanport[12] = {0}, lanBind[6] = {0};
	char wan_if_val[20];

	bzero(g_PortBindGroup, sizeof(g_PortBindGroup));

	for (pvc = 0; pvc < pvc_num; pvc++) {
		for ( entry = 0; entry < MAX_SMUX_NUM; entry ++ ) {
			
			wan_idx = pvc*MAX_SMUX_NUM + entry;
			snprintf(wan_node, sizeof(wan_node), "Wan_PVC%d_Entry%d", pvc, entry);

			/* get WAN iface name. */
			if ( 0 != tcapi_get(wan_node, "IFName"
					, g_PortBindGroup[wan_idx].wan_iface) )
				continue;

			/* get portbind for LAN */
			for ( lan_idx = 1; lan_idx <= MAX_ECNT_ETHER_PORT_NUM; lan_idx ++ )
			{
				snprintf(lanport, sizeof(lanport), "LAN%d", lan_idx);
				if ( 0 == tcapi_get(wan_node, lanport, lanBind)
					&& 0 == strcmp(lanBind, "Yes") )
					g_PortBindGroup[wan_idx].portBindGroup[lan_idx - 1] = 1;
			}
			/* get portbind for SSID */
			for ( lan_idx = 1; lan_idx <= MAX_ECNT_WALN_PORT_NUM; lan_idx ++ )
			{
				snprintf(lanport, sizeof(lanport), "SSID%d", lan_idx);
				if ( 0 == tcapi_get(wan_node, lanport, lanBind)
					&& 0 == strcmp(lanBind, "Yes") )
					g_PortBindGroup[wan_idx].portBindGroup[lan_idx - 1 + MAX_ECNT_ETHER_PORT_NUM] = 1;
			}
			/* get portbind for SSIDAC 
			0-3 LAN1 - LAN4
			4-7 SSID1 - SSID4
			8 USB0
			9 WDS0
			10 SSIDAC1 - SSIDAC4
			*/
			for ( lan_idx = 1; lan_idx <= MAX_ECNT_WALNAC_PORT_NUM; lan_idx ++ )
			{
				snprintf(lanport, sizeof(lanport), "SSIDAC%d", lan_idx);
				if ( 0 == tcapi_get(wan_node, lanport, lanBind)
					&& 0 == strcmp(lanBind, "Yes") )
					g_PortBindGroup[wan_idx].portBindGroup[lan_idx - 1 + ECNT_WLANAC_PORT_OFFSET] = 1;
			}
		}
	}
	
	return 0;
}
int get_PortBindByPort(int portid, char *wanif, int wanif_len)
{
#ifdef TCSUPPORT_CMCCV2
	int pvc_num = 1;
#else
	int pvc_num = PVC_NUM;
#endif
	int pvc_idx = 0, entry_idx = 0, wan_idx = 0;

	if ( portid < 0 || portid >= MAX_LAN_PORT_NUM
		|| NULL == wanif )
		return 0;

	for (pvc_idx = 0; pvc_idx < pvc_num; pvc_idx++) {
		for ( entry_idx = 0; entry_idx < MAX_SMUX_NUM; entry_idx ++ ) {
			wan_idx = pvc_idx * MAX_SMUX_NUM + entry_idx;
			if ( 1 == g_PortBindGroup[wan_idx].portBindGroup[portid] )
			{
				snprintf(wanif, wanif_len, "%s", g_PortBindGroup[wan_idx].wan_iface);
				return 1;
			}
		}
	}
	return 0;
}

#endif

int dnsmasq_trace_sw = 0;
/*
* check switch on / off.
*/
int dnsmasq_trace_switch_check()
{
	FILE *fp = NULL;

	fp = fopen("/tmp/dnsmasq_trace", "r");
	if ( fp )
	{
		dnsmasq_trace_sw = 1;
		fclose(fp);
	}
	else
		dnsmasq_trace_sw = 0;

	return 0;
}

int dnsmasq_trace(const char *format, ...)
{
	FILE *fp = NULL;
	va_list args;

	if ( 0 == dnsmasq_trace_sw )
		return -1;

	fp = fopen("/proc/tc3162/dbg_msg", "w");
	if ( fp )
	{
		va_start(args, format);
		vfprintf(fp, format, args);
		va_end(args);
		fclose(fp);
	}

	return 0;
}


