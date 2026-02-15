/*!Copyright(c) 2013-2014 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 *\file     xt_pctl.c
 *\brief    kernel/netfilter part for parental control. 
 *
 *\author   Miao Wen
 *\version  1.0.0
 *\date     10Mar17
 *
 *\history  \arg 1.0.0, creat this based on "multiurl" mod from soho.  
 *          
 *          \arg 1.1.0, 10Dec21, Liu Yuxuan
 */

/***************************************************************************/
/*                      CONFIGURATIONS                   */
/***************************************************************************/


/***************************************************************************/
/*                      INCLUDE_FILES                    */
/***************************************************************************/
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <linux/netfilter/x_tables.h>
#include <linux/version.h>
#include <linux/time.h>
#include <linux/rtc.h>
#include <linux/sort.h>
#include <linux/proc_fs.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h>

#include <net/tcp.h>
#include <net/netfilter/nf_conntrack.h>

#include "xt_pctl.h"
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,34))
#include "compat_xtnu.h"
#endif

/***************************************************************************/
/*                      DEFINES                      */
/***************************************************************************/

#define PCTL_HTTP_REFERER  0
#define PCTL_REDIRECT  0
#define PCTL_DNS_REDIRECT  1
#define PCTL_DEVICE_INFO   0
#define GET_BR_LAN_IP     1

#define HOST_STR     "\r\nHost: "
#define HOST_END_STR      "\r\n"

#if PCTL_HTTP_REFERER
#define REFERER_STR  "\r\nReferer: "
#define REFERER_END_STR  "\r\n"
#define REFERER_END_STR2  "/"
#endif

#if PCTL_DEVICE_INFO
#define USER_AGENT_STR     "\r\nUser-Agent: "
#define USER_AGENT_END_STR      "\r\n"
#endif

#define HTTP_STR  "http://"
#define HTTPS_STR  "https://"

#define GET_STR   "GET "
#define FILTER_HOST_IP    0x0a000002    /* 10.0.0.2 */
#define IS_TCP_FLAG_SYN(tcpFlag)    ((tcpFlag) == 0x0002)
#define GET_TCP_FLAG(pTcpHdr)       (((ntohl(tcp_flag_word((pTcpHdr))) & 0x0fff0000)<<4)>>20)

/*#define DEBUG   1*/

#ifdef DEBUG

#define PCTL_DEBUG(fmt, args...)   \
		printk("\033[0;40;35m[DEBUG]pctl %s %d: "fmt"\033[0m\n", __FUNCTION__, __LINE__, ##args)

#define PCTL_ERROR(fmt, args...)   \
		printk("\033[0;40;35m[ERROR]pctl %s %d: "fmt"\033[0m\n", __FUNCTION__, __LINE__, ##args) 

#else /* DEBUG */

#define PCTL_DEBUG(fmt, args...)   

#define PCTL_ERROR(fmt, args...)   \
		printk("\033[0;40;35m[ERROR]pctl %d: "fmt"\033[0m\n", __LINE__, ##args) 

#endif /* DEBUG */

#define DNS_PORT 53
#define HTTP_PORT 80
#define HTTPS_PORT 443

#define HANDSHAKE 22 /*ssl: content type.SSL_TYPE*/
#define CLIENT_HELLO 1 /*handshake: content type.HANDSHAKE_TYPE*/
#define SERVER_NAME 0 /*extension type in client hello(can only appear once in client hello).EXTENSION_TYPE*/
#define HOST_NAME 0 /*content type in SNI(in server_name extension).SERVER_NAME_TYPE*/

typedef struct _dns_header {
    unsigned short  transID;     /* packet ID */
    unsigned short  flags;       /* flag */
    unsigned short  nQDCount;    /* question section */
    unsigned short  nANCount;    /* answer section */
    unsigned short  nNSCount;    /* authority records section */
    unsigned short  nARCount;    /* additional records section */
}dns_header;

typedef struct __packed _dns_ans{
	unsigned short name;
	unsigned short _type;
	unsigned short _class;
	unsigned int ttl;
	unsigned short len;
	unsigned int ipaddr;
}dns_ans;

#define ETH_HTTP_LEN 14

#if PCTL_REDIRECT
#define RES_HTML_MAX_LEN 1024
#define RES_CONTENT_MAX_LEN ((RES_HTML_MAX_LEN) * 2)
#endif

#if GET_BR_LAN_IP
#define LAN_IP_PROC_FILENAME   "br_lan_ip"
#define LAN_IP_MAX_LEN   15
char br_lan_ip[LAN_IP_MAX_LEN+1]="192.168.0.1";
#endif

#if PCTL_DEVICE_INFO
#define DEVICE_INFO_NUM  256
#define DEVICE_INFO_USER_AGENT_LAN  255
#define DEVICE_INFO_PROC_FILENAME   "devices"
#define DEVICE_WRITE_BUF_LEN		(2)

typedef enum _DEVICE_TYPE{
    DEVICE_TYPE_NONE = -1,
    DEVICE_TYPE_OTHER = 0,
    DEVICE_TYPE_PC,
    DEVICE_TYPE_PHONE,
    DEVICE_TYPE_LABTOP,
    DEVICE_TYPE_TABLET,
    DEVICE_TYPE_ENTERTAINMENT,
    DEVICE_TYPE_PRINTER6,
    DEVICE_TYPE_IOT,
}DEVICE_TYPE;

typedef struct _device_info{
    unsigned char  mac[ETH_ALEN];
    DEVICE_TYPE    type;
}device_info;
#endif

#define PCTL_PROC_DIR           "pctl"
#define PCTL_PROC_NAME_LEN      (8)
#define PCTL_URL_HASH_SIZE      (64)
#define PCTL_LOG_NUM            (256)

#define PCTL_HISTORY_DAY_NUM    (6)

#define PCTL_HISTORY_LOG_NUM    (100)
#define PCTL_HISTORY_ACCESS_NUM (10)
#define PCTL_HISTORY_FORBID_NUM (5)

/* Adjust for local timezone */ 
extern struct timezone sys_tz; 
#define GET_TIMESTAMP()  ( get_seconds() - 60 * sys_tz.tz_minuteswest + 60 * daylight_offset) 

#define URL_LIB_PROC_FILENAME "url_lib"

/* liuyuxuan add begin */
#define ALL_LOG_PROC_FILENAME "all_log"

#define URL_CATEGORY_NUM        (7)
#define URL_CUSTOM              (0)
#define URL_LIB_BUF_LEN         (512)
#define VISIT_RANK_ENTRY_NUM    (5)
#define BLOCK_LOG_ENTRY_NUM     (32)
#define VISIT_LOG_ENTRY_NUM     (32)
#define FLAGS_BUF_LEN           (16)
#define DAYLIGHT_WRITE_BUF_LEN 	(32)

#define PCTL_SET_URL_CTL        (310)

#define CLEAR_HISTORY           (-1)

/* liuyuxuan add end */

#define DAYLIGHT_PROC_FILENAME "daylight_offset"

/***************************************************************************/
/*                      TYPES                            */
/***************************************************************************/
typedef enum _PCTL_STATUS{
    PCTL_STATUS_OK = 0,
    PCTL_STATUS_BLOCKED,        /* internet paused */
    PCTL_STATUS_TIME_LIMIT,     /* time limit */
    PCTL_STATUS_BEDTIME,        /* bedtime */
    PCTL_STATUS_FILTER,         /* url filter */
}PCTL_STATUS;

typedef struct _PROTOCOL_VERSION
{
	uint8_t majorVersion;
	uint8_t minorVersion;
}PROTOCOL_VERSION;

typedef struct _SSL_MSG{
	uint8_t type; /*len:1 byte*/
	PROTOCOL_VERSION version; /*len:2 bytes*/
	uint16_t length; /* The length (in bytes) of the following TLSPlaintext.fragment.*/
	uint8_t *pContent; /*  The application data,type is specified by the type field.*/
}SSL_MSG;

typedef uint32_t uint24_t;

typedef struct{
	uint16_t length;
	uint8_t *pData;
}CIPHER_SUITE,CH_EXTENSIONS;

typedef struct{
	uint8_t length;
	uint8_t *pData;
}SESSION_ID,COMPRESSION_METHOD;

typedef struct _TLS_EXTENSION{
	uint16_t type;
	uint16_t length;
	uint8_t *pData;
}TLS_EXTENSION;/*TLS(client hello) extension*/

typedef struct _HANDSHAKE_CLIENT_HELLO{
	uint8_t type; /*len:1 byte*/
	uint24_t length;
	PROTOCOL_VERSION clientVersion;
    uint8_t *random;/*the length is 32,but we don't need this field.So only give pointer to start position*/
    SESSION_ID sessionID;
    CIPHER_SUITE cipherSuites;
    COMPRESSION_METHOD compression_methods;
    uint8_t *pExtensions /*pointer to extensions length field*/;
}HANDSHAKE_CLIENT_HELLO;

typedef struct _pctl_stats{
    unsigned int   timestamp;    /* last visited */
    unsigned int  total;        /* minutes */
    unsigned int  forbid;        /* forbid */
}pctl_stats;

typedef struct _pctl_log_entry{
    unsigned    host_len;           /* host len */
    char        host[PCTL_URL_LEN + 1];     /* host */
    pctl_stats  stats;
}pctl_log_entry;

typedef struct _pctl_log{
    struct list_head log_node;
    struct hlist_node hash_node;
    pctl_log_entry entry;
}pctl_log;

/* liuyuxuan modify begin */
typedef struct _PCTL_BLOCK_LOG_ENTRY{
    unsigned int    host_len;
    char            host[PCTL_URL_LEN + 1];
    unsigned int    timestamp;
    unsigned int    category;
}PCTL_BLOCK_LOG_ENTRY;

typedef struct _PCTL_BLOCK_LOG{
    struct list_head        log_node;
    PCTL_BLOCK_LOG_ENTRY    entry;
}PCTL_BLOCK_LOG;

typedef struct _pctl_history{
    pctl_stats              day_stats;
    unsigned int            num; /* entry num */
    pctl_log_entry          log_entry[VISIT_RANK_ENTRY_NUM];

    unsigned int            block_log_entry_num; 
    PCTL_BLOCK_LOG_ENTRY    block_log_entry[BLOCK_LOG_ENTRY_NUM];
}pctl_history;


typedef struct _pctl_owner{
    unsigned int        id;

    /* today */
    pctl_stats          today_stats;
    unsigned int        log_len;
    struct list_head    log_list; /* point to pctl_log */
    struct hlist_head   hash_list[PCTL_URL_HASH_SIZE];
    unsigned int        block_log_len;
    struct list_head    block_log_list;
    int                 is_block;

    struct proc_dir_entry* proc_file;
    rwlock_t lock;
}pctl_owner;

/* liuyuxuan modify end */

struct xtm {
	u_int8_t month;    /* (1-12) */
	u_int8_t monthday; /* (1-31) */
	u_int8_t weekday;  /* (1-7) */
	u_int8_t hour;     /* (0-23) */
	u_int8_t minute;   /* (0-59) */
	u_int8_t second;   /* (0-59) */
    unsigned int seconds_day;
    unsigned int minutes_day;
	unsigned int dse;
};

typedef struct _url_lib{
    int num; 
    char key[PCTL_URL_LIB_SIZE][PCTL_URL_LEN + 1]; /* name */
    char host[PCTL_URL_LIB_SIZE][PCTL_URL_LEN + 1]; /* host */
}url_lib;

/* liuyuxuan add begin */

/* The same with URL_TYPE in oal_parentcontrol.h. */
typedef enum _URL_TYPE
{
    URL_BLOCKED     = 0, 
    URL_UNBLOCKED   = 1, 
} URL_TYPE;

/* The same with URL_INFO in oal_parentcontrol.h. */
typedef struct _URL_INFO
{
    URL_TYPE        type;
    int             id;     /* Profile id. */
    unsigned short  index;
    char            url[PCTL_URL_LEN + 1];
} URL_INFO;

/* The same with HISTORY_TYPE in oal_parentcontrol.h. */
typedef enum _HISTORY_TYPE
{
    TIME_USED           = 0, 
    VISIT_RANK          = 1, 
    BLOCK_HISTORY       = 2, 
    VISIT_HISTORY       = 3, 
    HISTORY_TYPE_NUM    = 4, 
} HISTORY_TYPE;

typedef struct _READ_FLAGS
{
    HISTORY_TYPE    history_type;
    unsigned int    day;    /* 0: today, 1: yesterday, and so on. */
} READ_FLAGS;

/* liuyuxuan add end */

/***************************************************************************/
/*                      EXTERN_PROTOTYPES                    */
/***************************************************************************/


/***************************************************************************/
/*                      LOCAL_PROTOTYPES                     */
/***************************************************************************/

/*!
 *\fn           unsigned char *_url_strstr(const unsigned char* start, const unsigned char* end, 
                                        const unsigned char* strCharSet)
 *\brief        find the url in str zone
 *\param[in]    start           start ptr of str zone.
 *\param[in]    end             end ptr of str zone.
 *\param[in]    strCharSet      the url you want to find
 *\return       url postion
 */
static unsigned char *_url_strstr(const unsigned char* start, const unsigned char* end, const unsigned char* strCharSet);

/*!
 *\fn           static bool match(const struct sk_buff *skb, struct xt_action_param *param)
 *\brief        find the url in skb (host in http or querys in dns)
 *\return       found or not
 */
static bool match(const struct sk_buff *skb, struct xt_action_param *par);

/*
 * fn           static bool match_ipv6(const struct sk_buff *skb, struct xt_action_param *par)
 * brief        Find the url in skb (host in http or querys in dns or servername in https(Clienthello) ).
 *
 * param[in]    skb
 * param[in]    par
 *
 * return       bool
 */
static bool match_ipv6(const struct sk_buff *skb, struct xt_action_param *par);

/*!
 *\fn           static int __init pctl_init(void)
 *\brief        mod init
 *\return       SUCCESS or not
 */
static int __init pctl_init(void);

/*!
 *\fn           static void __exit pctl_exit(void)
 *\brief        mod exit
 *\return       none
 */

static void __exit pctl_exit(void);
/*
 * fn		static bool extractHandshakeFromSSL(const uint8_t *pSSLBuff, uint8_t **ppHandshake)
 * brief	extract the handshake From SSL packet.
 * param[in]	pSSL - pointer to the start of SSL packet in skb_buff.
 * param[out]	ppHandshake - address of pointer to the start of handshake message wrapped with SSLv3/TLS.
 * return	BOOL
 * retval	true  succeed to extract handshake
 *		false fail to extract handshake
 */
static bool extractHandshakeFromSSL(const uint8_t *pSSL, uint8_t **ppHandshake);

/* 
 * fn		static bool extractSNIFromExtensions(const uint8_t *pExtensions, uint8_t *ppSNIExt) 
 * brief	extract SNI extension form extensions.
 * param[in]	pExtensions - pointer to start of extensionList.
 * param[out]	ppSNIExt      - address of pointer to SNI extension.
 * return	bool
 * retval	true - extract SNI extension successfully.
 *       	false - extract SNI extension unsuccessfully.	
 */
static bool extractSNIFromExtensions(const uint8_t *pExtensions,uint8_t **ppSNIExt);

/* 
 * fn		static  bool extractSNIFromClientHello(const uint8_t *pClientHello, uint8_t **ppSNIExt) 
 * brief	extract SNI extension(Server_name)represents host_name from client_hello.
 * param[in]	pClientHello - pointer to start position of client_hello message.
 * param[out]	ppSNIExt - address of pointer to the start position of SNI extension in client_hello.
 * return	bool
 * retval	true -get the SNI represents host_name.
 *		false - doesn't get the right SNI.
 */
static bool extractSNIFromClientHello(const uint8_t *pClientHello, uint8_t **ppSNIExt);

static int log_proc_show(struct seq_file *s, void *unused);
static ssize_t log_proc_write(struct file *file, const char __user * buf, size_t count,  loff_t *data);
static int log_proc_open(struct inode *inode, struct file *file);
//static int log_update_history(int id, unsigned int now);
static int log_clear(int id);

static int url_lib_proc_read(struct seq_file *s, void *unused);
static int url_lib_proc_open(struct inode *inode, struct file *file);
static ssize_t url_lib_proc_write(struct file *file, const char __user * buf, size_t count,  loff_t *data);

#if GET_BR_LAN_IP
static int lan_ip_proc_read(struct seq_file *s, void *unused);
static int lan_ip_proc_open(struct inode *inode, struct file *file);
static ssize_t lan_ip_proc_write(struct file *file, const char __user * buf, size_t count,  loff_t *data);
#endif
#if PCTL_DEVICE_INFO
static int device_proc_read(struct seq_file *s, void *unused);
static int device_proc_open(struct inode *inode, struct file *file);
static ssize_t device_proc_write(struct file *file, const char __user * buf, size_t count,  loff_t *data);
#endif
static int daylight_proc_open(struct inode *inode, struct file *file);
static ssize_t daylight_proc_write(struct file *file, const char __user * buf, size_t count, loff_t *data);
static int daylight_proc_read(struct seq_file *s, void *unused);

/* liuyuxuan add begin */
/*
 * fn           static int all_log_proc_show(struct seq_file *s, void *unused)
 * brief        Function to show all log. 
 *
 * param[in]    s
 * param[in]    unused
 *
 * return       
 */
static int all_log_proc_show(struct seq_file *s, void *unused);

/*
 * fn           static int all_log_proc_open(struct inode *inode, struct file *file)  
 * brief        Function to open all_log proc. 
 *
 * param[in]    inode
 * param[in]    file
 *
 * return       
 */
static int all_log_proc_open(struct inode *inode, struct file *file);


/*
 * fn           static int do_url_set_ctl(struct sock *sk, int cmd, 
 *                                        void __user *user, unsigned int len)
 * brief        Ctl function of url_sockopts. 
 *
 * param[in]    sk      Socket. 
 * param[in]    cmd     Command code. 
 * param[in]    user    Data in user space. 
 * param[in]    len     Length of data. 
 *
 * return       If successful, return 0. Otherwise, return error code. 
 */
static int do_url_set_ctl(struct sock *sk, int cmd, void __user *user, unsigned int len);

/*
 * fn           static int getChrIndex(const char *pStr, const int len, const char ch)
 * brief        Get index of character in the given string. 
 *
 * param[in]    pStr
 * param[in]    len
 * param[in]    ch
 *
 * return       If the character can be found in string, return index of it. Otherwise, return -1. 
 */
static int getChrIndex(const char *pStr, const int len, const char ch);


/* liuyuxuan add end */

static inline void localtime_1(struct xtm *r, unsigned int time);
static inline void localtime_2(struct xtm *r, unsigned int time);

/***************************************************************************/
/*                      VARIABLES                        */
/***************************************************************************/
static struct xt_match pctl_match[] = {
    { 
        .name           = "pctl",
        .family         = NFPROTO_IPV4,
        .match          = match,
        .matchsize      = XT_ALIGN(sizeof(struct _xt_pctl_info)),
        .me             = THIS_MODULE,
    },
#ifdef INCLUDE_IPV6
    { 
        .name           = "pctl",
        .family         = NFPROTO_IPV6,
        .match          = match_ipv6,
        .matchsize      = XT_ALIGN(sizeof(struct _xt_pctl_info)),
        .me             = THIS_MODULE,
    },
#endif /* INCLUDE_IPV6 */
};

#if PCTL_REDIRECT
#define REDIRECT_DOMAIN "http://%s:30000/pctl_blocking.html?"
static char *redirect_url[] = {
    REDIRECT_DOMAIN"pid=1&domain=?",
    REDIRECT_DOMAIN"pid=2&domain=?",
    REDIRECT_DOMAIN"pid=3&domain=?",
    REDIRECT_DOMAIN"pid=4&domain=%s",
};

const char http_redirection_format[] = {
	"HTTP/1.1 200 OK\r\n"
	"Connection: close\r\n"
	"Content-type: text/html\r\n"
	"Content-length: %d\r\n"
	"\r\n"
	"%s"
};
const char http_redirection_html[] = {
	"<html><head></head><body>"
	"<script type=\"text/javascript\" language=\"javascript\">location.href=\"%s\";</script>"
	"</body></html>"
};

#define ACKTIMESTAMPKIND 8
#define ACKNOPKIND 1
#endif
     
static const struct file_operations log_proc_fops = { 
    .llseek     = seq_lseek,   
    .read       = seq_read,
    .write      = log_proc_write,
    .open       = log_proc_open,
    .release    = single_release,
};

/* liuyuxuan modify begin */
static READ_FLAGS read_flags = {0};

static const struct file_operations all_log_proc_fops = {  
    .open       = all_log_proc_open,  
    .read       = seq_read,
    /*.write      = all_log_proc_write,*/
    .llseek     = seq_lseek,  
    .release    = single_release,
};

/* liuyuxuan modify end */

static pctl_owner owners[PCTL_OWNER_NUM];
static struct proc_dir_entry *proc_dir;

#if GET_BR_LAN_IP
/* add proc file to get br-lan ip */
static const struct file_operations lan_ip_proc_fops = {  
    .open       = lan_ip_proc_open,  
    .read       = seq_read,
    .write      = lan_ip_proc_write,
    .llseek     = seq_lseek,  
    .release    = single_release,
};
static struct proc_dir_entry* lan_ip_proc_file;
rwlock_t lan_ip_lock;
#endif

#if PCTL_DEVICE_INFO
static const struct file_operations device_proc_fops = {  
    .open       = device_proc_open,  
    .read       = seq_read,
    .write      = device_proc_write,
    .llseek     = seq_lseek,  
    .release    = single_release,
};
struct proc_dir_entry* device_proc_file;
device_info devices[DEVICE_INFO_NUM];
rwlock_t device_info_lock;
#endif

/* liuyuxuan modify begin */
struct proc_dir_entry* all_log_proc_file;

static url_lib *l_url_lib = NULL;
/* Every element means the last index of urls belonging to corresponding category in url lib. */
static int l_url_category_indexes[URL_CATEGORY_NUM + 1] = {0}; 
static const struct file_operations url_lib_proc_fops = {  
    .open       = url_lib_proc_open,  
    .read       = seq_read,
    .write      = url_lib_proc_write,
    .llseek     = seq_lseek,  
    .release    = single_release,
};
static struct proc_dir_entry* url_lib_proc_file;
static rwlock_t url_lib_lock;

static char blocked_url[PCTL_TOTAL_BLOCKED_NUM][PCTL_URL_LEN + 1];
static rwlock_t blocked_url_lock;

static char unblocked_url[PCTL_TOTAL_UNBLOCKED_NUM][PCTL_URL_LEN + 1];
static rwlock_t unblocked_url_lock;

typedef int (*IPT_HOOK_PTR)(void __user *user, unsigned int len);
static IPT_HOOK_PTR ipt_ctl_hook_url = NULL;
static struct nf_sockopt_ops url_sockopts = {
    .pf         = PF_INET,
    .set_optmin = PCTL_SET_URL_CTL,
    .set_optmax = PCTL_SET_URL_CTL + 1,
    .set        = do_url_set_ctl,
    .owner      = THIS_MODULE,
};

/* liuyuxuan modify end */

static struct proc_dir_entry* daylight_file;
static int daylight_offset = 0;
static const struct file_operations daylight_proc_fops = {  
    .open       = daylight_proc_open,  
    .read       = seq_read,
    .write      = daylight_proc_write,
    .llseek     = seq_lseek,  
    .release    = single_release,
};

/***************************************************************************/
/*                      LOCAL_FUNCTIONS                  */
/***************************************************************************/
#ifdef CONFIG_ATHRS17_HNAT_WIFI
extern int (*pctl_drop_packets)(struct sk_buff *skb);
typedef struct _client_info{
    unsigned char mac[PCTL_CLIENT_NUM][ETH_ALEN];
    unsigned int block[PCTL_CLIENT_NUM];
}client_info;
client_info l_client_info;
static const unsigned char l_mac_null[ETH_ALEN]={0};

static int find_client_mac(const unsigned char* mac)
{
    int hash = (mac[0] + mac[1] + mac[2] + mac[3] + mac[4] + mac[5]) % PCTL_CLIENT_NUM;
    int index = 0, step = 0;

    for(step=0; step<PCTL_CLIENT_NUM; step++) 
    {

		index=(hash + step) % PCTL_CLIENT_NUM;
		if(	!memcmp(l_client_info.mac[index], l_mac_null, ETH_ALEN) )
		{
			return -1;
		}
		else if( !memcmp(mac, l_client_info.mac[index], ETH_ALEN) ) 
        {
            return index;
        }
    }
    return -1;
}

static int record_client_mac(const struct sk_buff *skb, unsigned int block)
{
    int hash = 0; 
    int index = 0, step = 0;
    unsigned char *mac = NULL;
    const struct ethhdr *p_ethHdr = (struct ethhdr *)skb_mac_header(skb);
    
    if(!p_ethHdr) 
    {
        return FALSE;
    }
    mac = p_ethHdr->h_source;
    index = find_client_mac(mac);
    if(index >= 0)
    {   
        l_client_info.block[index] = block;
        return TRUE;
    }
    hash = (mac[0] + mac[1] + mac[2] + mac[3] + mac[4] + mac[5]) % PCTL_CLIENT_NUM;
    for(step=0; step<PCTL_CLIENT_NUM; step++) 
    {
        index=(hash + step) % PCTL_CLIENT_NUM;
        if(!memcmp(l_mac_null, l_client_info.mac[index], ETH_ALEN)) 
        {
            memcpy(l_client_info.mac[index], mac, ETH_ALEN);
            l_client_info.block[index] = block;
            return TRUE;
        }
    }
    return FALSE;
}

static int pctl_drop_packets_by_mac(const struct sk_buff *skb)
{
    const struct ethhdr *p_ethHdr = (struct ethhdr *)skb_mac_header(skb);
    const struct iphdr *iph = ip_hdr(skb); /* ipv4 only */
    int res = -1;
    if(!p_ethHdr || !iph) 
    {
        return FALSE;
    }

    res = find_client_mac(p_ethHdr->h_source);
    if (res < 0)
    {
        res = find_client_mac(p_ethHdr->h_dest);
        if (res < 0)
        {
            return FALSE;
        }
    }
    else if(TRUE == l_client_info.block[res])
    {
        return TRUE;
    }

    return FALSE;
}

#endif

#if PCTL_REDIRECT

static unsigned char js_buf[RES_CONTENT_MAX_LEN + 1];
static unsigned char js_str[RES_HTML_MAX_LEN];

static void
handle_time_stamps(unsigned char *start,int len)
{
	int TLVp = 0;

	int i = 0;
	
	while(TLVp<len)
	{
		if(start[TLVp] == ACKNOPKIND)
		{
			TLVp++;
			continue;
		}
		if(start[TLVp] == ACKTIMESTAMPKIND)
		{
			for(i=0;i<10;i++)
			{
				start[TLVp+i]=ACKNOPKIND;
			}
			return;
		}
		TLVp+=start[TLVp+1];
		if(start[TLVp] == 0)  /*reach the EOL(End of Option List)*/
		{
			return ;  
		}

		if(start[TLVp+1])
		{
			TLVp+=start[TLVp+1];
		}  
		else /*abnormal option length,just return,to avoid dead loop! */
		{
		  	return; 
		}
	}
}

static int http_ack(struct sk_buff *skb, struct net_device *in)
{
    struct sk_buff *pNewSkb = NULL;
    struct sk_buff * pSkb = skb;

    unsigned short eth_len = 0;
    unsigned short ip_len = 0;
    unsigned short ip_payload_len = 0;
    unsigned short tcp_len = 0;
    unsigned short tcp_flag = 0;

    unsigned char tmp_mac[ETH_ALEN] = {0};
    unsigned int tmp_ip = 0;
    unsigned short tmp_port;

    struct ethhdr *pEthHdr;
    struct iphdr *pIpHdr;
    struct tcphdr *pTcpHdr;

    unsigned char *pTcpPayload;
    unsigned char *pOption = NULL;
    unsigned int  option_len = 0;

    eth_len = ETH_HTTP_LEN;
    PCTL_DEBUG("http_ack");
    pEthHdr = (struct ethhdr *)skb_mac_header(pSkb);
    if (NULL == pEthHdr)
    {
        PCTL_ERROR("---->>> Get ethhdr error!");
        return -1;
    }

    pIpHdr = (struct iphdr *)((unsigned char *)pEthHdr + eth_len);
    if (NULL == pIpHdr)
    {
        PCTL_ERROR("--->>> Get iphdr error!");
        return -1;
    }
    ip_len = (pIpHdr->ihl) << 2;
    ip_payload_len = ntohs(pIpHdr->tot_len);

    pTcpHdr = (struct tcphdr *)((unsigned char *)pIpHdr + ip_len);
    if (NULL == pTcpHdr)
    {
        PCTL_ERROR("--->>> Get tcphdr error!");
        return -1;
    }
    tcp_len = (ntohl(tcp_flag_word(pTcpHdr)) & 0xf0000000) >> 26;
    tcp_flag = GET_TCP_FLAG(pTcpHdr);
    pTcpPayload = (unsigned char *)((unsigned char *)pTcpHdr + tcp_len);

    skb_push(pSkb, eth_len);
    if (skb_cloned(pSkb)){
        pNewSkb = skb_copy(pSkb, GFP_ATOMIC);
        if (NULL == pNewSkb)
        {
            PCTL_DEBUG("alloc new skb fail!");
            return -1;
        }
        pEthHdr = (struct ethhdr *)skb_mac_header(pNewSkb);
        pIpHdr = (struct iphdr *)((unsigned char *)pEthHdr + eth_len);
        pTcpHdr = (struct tcphdr *)((unsigned char *)pIpHdr + ip_len);
        skb_pull(pSkb, eth_len);
    }else
    {
        if (NULL == (pNewSkb = skb_clone(pSkb, GFP_ATOMIC)))
        {
            PCTL_ERROR("clone(simple copy) fail");
            return -1;
        }
    }

    if ((ip_payload_len == (ip_len + tcp_len) && IS_TCP_FLAG_SYN(tcp_flag)))
    {
        memcpy(tmp_mac, pEthHdr->h_dest, ETH_ALEN);
        memcpy(pEthHdr->h_dest, pEthHdr->h_source, ETH_ALEN);
        memcpy(pEthHdr->h_source, tmp_mac, ETH_ALEN);

        /* ip header */
        tmp_ip        = pIpHdr->saddr;
        pIpHdr->saddr = pIpHdr->daddr;
        pIpHdr->daddr = tmp_ip;
        pIpHdr->check = 0;
        pIpHdr->check = ip_fast_csum(pIpHdr, pIpHdr->ihl);
        
        /* tcp header */
        tmp_port           = pTcpHdr->source;
        pTcpHdr->source   = pTcpHdr->dest;
        pTcpHdr->dest     = tmp_port;
        pTcpHdr->ack      = 1;
        pTcpHdr->ack_seq  = htonl(ntohl(pTcpHdr->seq) + 1);
        pTcpHdr->seq      = htonl(0x32bfa0f1); /* hard code the server seq num */
        pTcpHdr->check    = 0;
		
		pOption      = ((unsigned char*)pTcpHdr)+sizeof(struct tcphdr);
		option_len    = tcp_len - sizeof(struct tcphdr);
        handle_time_stamps(pOption,option_len);
		
        pTcpHdr->check    = tcp_v4_check(tcp_len, pIpHdr->saddr, pIpHdr->daddr, 
                                         csum_partial(pTcpHdr, tcp_len, 0));
		
        /* send the modified pkt */
        pNewSkb->dev = in;
        if (0 > dev_queue_xmit(pNewSkb))
        {
            PCTL_ERROR("send http ack pkt fail.");
            return -1;
        }
    }

    return 0;
}

static int http_response(struct sk_buff *skb, struct net_device *in, int reason, const unsigned char* block_url)
{
	struct sk_buff *pNewSkb = NULL;
	struct sk_buff * pSkb = skb;
	
	unsigned short eth_len;
	unsigned short ip_len;
	unsigned short ip_payload_len;
	unsigned short tcp_len;
	unsigned short tcp_payload_len;
	unsigned short add_payload_len;

    unsigned char tmp_mac[ETH_ALEN] = {0};
    unsigned char tmp_redirect_url[RES_HTML_MAX_LEN]={0};

	unsigned int tmp_ip = 0;
	unsigned short tmp_port;
	unsigned int tmp_seq;
	unsigned int tmp_tcp_payload_len;

	int js_real_len;

	struct ethhdr * pEthHdr;
	struct iphdr * pIpHdr;
	struct tcphdr * pTcpHdr;

	unsigned char * pTcpPayload;

	eth_len = ETH_HTTP_LEN;
	PCTL_DEBUG("http_response");
	pEthHdr = (struct ethhdr *)skb_mac_header(pSkb);
	if (NULL == pEthHdr)
	{
		PCTL_ERROR("---->>> Get ethhdr error!");
		return -1;
	}
	
	pIpHdr = (struct iphdr *)((unsigned char *)pEthHdr + eth_len);
	if (NULL == pIpHdr)
	{
		PCTL_ERROR("--->>> Get iphdr error!");
		return -1;
	}
	ip_len = (pIpHdr->ihl) << 2;
	ip_payload_len = ntohs(pIpHdr->tot_len);

	pTcpHdr = (struct tcphdr *)((unsigned char *)pIpHdr + ip_len);
	if (NULL == pTcpHdr)
	{
		PCTL_ERROR("--->>> Get tcphdr error!");
		return -1;
	}
	tcp_len = (ntohl(tcp_flag_word(pTcpHdr)) & 0xf0000000) >> 26;
	pTcpPayload = (unsigned char *)((unsigned char *)pTcpHdr + tcp_len);

    skb_push(pSkb, eth_len);
    if (skb_cloned(pSkb)){
        pNewSkb = skb_copy(pSkb, GFP_ATOMIC);
        if (NULL == pNewSkb)
        {
            PCTL_DEBUG("alloc new skb fail!");
            return -1;
        }
        pEthHdr = (struct ethhdr *)skb_mac_header(pNewSkb);
        pIpHdr = (struct iphdr *)((unsigned char *)pEthHdr + eth_len);
        pTcpHdr = (struct tcphdr *)((unsigned char *)pIpHdr + ip_len);
        pTcpPayload = (unsigned char *)((unsigned char *)pTcpHdr + tcp_len);
        skb_pull(pSkb, eth_len);
    }else
    {
        if (NULL == (pNewSkb = skb_clone(pSkb, GFP_ATOMIC)))
		{
			PCTL_ERROR("clone(simple copy) fail");
            return -1;
		}
    }

	/* tcp payload */
	tmp_tcp_payload_len = ip_payload_len - ip_len - tcp_len;
	tcp_payload_len = ip_payload_len - ip_len - tcp_len + RES_HTML_MAX_LEN;
    memset(js_buf, 0, RES_CONTENT_MAX_LEN + 1);
    if ( reason == 4 )
    {
        sprintf(tmp_redirect_url, redirect_url[reason-1], br_lan_ip, block_url);
    }
    else
    {
        sprintf(tmp_redirect_url, redirect_url[reason-1], br_lan_ip);
    }
	snprintf(js_str, RES_HTML_MAX_LEN - 1, http_redirection_html, tmp_redirect_url);
	js_real_len = strlen(js_str);
	snprintf(js_buf, tcp_payload_len - 1, http_redirection_format, js_real_len, 
			js_str);
	add_payload_len = strlen(js_buf);
	if ((pNewSkb->tail + ip_len + tcp_len + add_payload_len - ip_payload_len) > pNewSkb->end)
	{
		PCTL_ERROR("--->>> Construct tcp payload error!");
		dev_kfree_skb(pNewSkb);
		return -1;
	}
	pNewSkb->tail = pNewSkb->tail + ip_len + tcp_len + add_payload_len - ip_payload_len;
	pNewSkb->len = pNewSkb->len + ip_len + tcp_len + add_payload_len - ip_payload_len;
	strncpy(pTcpPayload, js_buf, tcp_payload_len - 1);
	
	/* eth header */
	memcpy(tmp_mac, pEthHdr->h_dest, ETH_ALEN);
	memcpy(pEthHdr->h_dest, pEthHdr->h_source, ETH_ALEN);
	memcpy(pEthHdr->h_source, tmp_mac, ETH_ALEN);

	/* ip header */
	tmp_ip = pIpHdr->saddr;
	pIpHdr->saddr = pIpHdr->daddr;
	pIpHdr->daddr = tmp_ip;
	pIpHdr->tot_len = htons(ip_len + tcp_len + strlen(pTcpPayload));
	pIpHdr->check = 0;
	pIpHdr->check = ip_fast_csum(pIpHdr, pIpHdr->ihl);

	/* tcp header */
	tmp_port = pTcpHdr->source;
	tmp_seq = pTcpHdr->seq;
	pTcpHdr->source = pTcpHdr->dest;
	pTcpHdr->dest = tmp_port;
	pTcpHdr->urg = 0;
	pTcpHdr->ack = 1;
	pTcpHdr->psh = 0;
	pTcpHdr->rst = 0;
	pTcpHdr->syn = 0;
	pTcpHdr->fin = 1;
	pTcpHdr->seq = pTcpHdr->ack_seq;
	pTcpHdr->ack_seq = htonl(ntohl(tmp_seq) + tmp_tcp_payload_len);
	pTcpHdr->check = 0;
	pTcpHdr->check = tcp_v4_check(tcp_len + strlen(pTcpPayload), pIpHdr->saddr, 
			pIpHdr->daddr, 
			csum_partial(pTcpHdr, tcp_len + strlen(pTcpPayload), 0));

	pNewSkb->dev = in;

	if (dev_queue_xmit(pNewSkb) < 0)
	{
		PCTL_ERROR("--->>> Send Http Response error!");
		return -1;
	}

	return 0;
}
#endif

#if PCTL_DNS_REDIRECT
#if 0
static bool
acceptDNSReq(dns_header* dnshdr)
{
	if((0 != dnshdr->nARCount) || (0 != dnshdr->nNSCount)
		|| (0 != dnshdr->nANCount) || (1 != ntohs(dnshdr->nQDCount))
		|| (0 != (ntohs(dnshdr->flags)&0xfcff))) //0 0000 0 x x 0 000 0000
	{
		return false;
	}
	return true;
}

static int getDNSReqLen(char* DNSReqData)
{
	int len;
	len = 0;
	while(*DNSReqData)
	{
		len += (unsigned char)(*DNSReqData) + 1;
		DNSReqData += (unsigned char)(*DNSReqData) + 1;
		if(len>512)
		{
			return -1;
		}
	}
	len ++;    //last 0x00
	len += 4;  //req type && req class;
	return len;
}

static int dns_response(struct sk_buff *skb, struct net_device *in, int reason)
{
    struct sk_buff *pNewSkb = NULL;
	struct sk_buff * pSkb = skb;

	unsigned short eth_len;
	unsigned short ip_len;
	unsigned short ip_payload_len;

    struct ethhdr * pEthHdr;
	struct iphdr * pIpHdr;
	struct udphdr *pUdpHdr  = NULL;
	dns_header *pDNSHdr  = NULL;
	dns_ans *pDNSAns  = NULL;

	char             *pDNSReqData     = NULL;
	int              DNSReqLen        = 0;

    u8    tmpMac[ETH_ALEN] = {0};
    u32   tmpIp            = 0;
    u16   tmpPort          = 0;

    enum ip_conntrack_info ctinfo;
	const struct nf_conn *ct;

    eth_len = ETH_HTTP_LEN;
	PCTL_DEBUG("tp_dns_response");

    pEthHdr = (struct ethhdr *)skb_mac_header(pSkb);
    if (NULL == pEthHdr)
    {
        PCTL_ERROR("---->>> Get ethhdr error!");
        return -1;
    }

    pIpHdr = (struct iphdr *)((unsigned char *)pEthHdr + eth_len);
    if (NULL == pIpHdr)
    {
        PCTL_ERROR("--->>> Get iphdr error!");
        return -1;
    }
    ip_len = (pIpHdr->ihl) << 2;
    ip_payload_len = ntohs(pIpHdr->tot_len);

	/*handle DNS here*/
    pUdpHdr = (struct udphdr*)((s8*)pIpHdr + ip_len);
    if(NULL == pUdpHdr) 
    {
        PCTL_ERROR("--->>> Get udhdr error!");
        return -1;
    }

	pDNSHdr = (dns_header*)((s8*)pUdpHdr + sizeof(struct udphdr));
	if(!acceptDNSReq(pDNSHdr))
	{
        PCTL_ERROR("not acceptDNSReq.");
        return -1;
	}

	pDNSReqData = (char*)((s8*)pDNSHdr + sizeof(dns_header));
	DNSReqLen = getDNSReqLen(pDNSReqData);
	if(DNSReqLen < 0)
	{
		PCTL_ERROR("analyse pkt error.");
        return -1;
	}

    skb_push(pSkb, eth_len);
    if (skb_cloned(pSkb))
    {
        pNewSkb = skb_copy(pSkb, GFP_ATOMIC);
        if (NULL == pNewSkb)
        {
            PCTL_DEBUG("alloc new skb fail!");
            return -1;
        }
        pEthHdr = (struct ethhdr*)skb_mac_header(pNewSkb);
        pIpHdr  = (struct iphdr*)((s8*)pEthHdr + eth_len);
        pUdpHdr = (struct udphdr*)((s8*)pIpHdr + ip_len);
        pDNSHdr = (dns_header*)((s8*)pUdpHdr + sizeof(struct udphdr));
        pDNSReqData = (char*)((s8*)pDNSHdr + sizeof(dns_header));
        skb_pull(pSkb,eth_len);
    }else
    {
        if (NULL == (pNewSkb = skb_clone(pSkb, GFP_ATOMIC)))
		{
			PCTL_ERROR("clone(simple copy) fail");
            return -1;
		}
    }

    memcpy(tmpMac, pEthHdr->h_dest, ETH_ALEN);
    memcpy(pEthHdr->h_dest, pEthHdr->h_source, ETH_ALEN);
    memcpy(pEthHdr->h_source, tmpMac, ETH_ALEN);

    /* ip header */
    tmpIp         = pIpHdr->saddr;
    ct = nf_ct_get(skb, &ctinfo);
	if (ct) {
        pIpHdr->saddr = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip;
    }else
    {
        pIpHdr->saddr = pIpHdr->daddr;
    }
    pIpHdr->daddr = tmpIp;
    pIpHdr->check = 0;
    pIpHdr->tot_len = htons(ntohs(pIpHdr->tot_len)+sizeof(dns_ans));
    pIpHdr->check = ip_fast_csum(pIpHdr, pIpHdr->ihl);
	
    /* udp header */
    tmpPort           = pUdpHdr->source;
    pUdpHdr->source   = pUdpHdr->dest;
    pUdpHdr->dest     = tmpPort;
    pUdpHdr->check    = 0;

	pDNSHdr->nANCount = htons(0x0001);
	pDNSHdr->flags    = htons(0x8580);
	if(pNewSkb->end - pNewSkb->tail < sizeof(dns_ans))
	{
        PCTL_ERROR("skb is not big enough.");
		dev_kfree_skb(pNewSkb);
        return -1;
	}
	skb_put(pNewSkb, sizeof(dns_ans));

	pDNSAns           = (dns_ans*)((s8*)pDNSReqData + DNSReqLen);
	pDNSAns->name     = htons(0xc000 + sizeof(dns_header));
	pDNSAns->_type    = htons(0x0001);
	pDNSAns->_class   = htons(0x0001);
	pDNSAns->ttl      = 0;
	pDNSAns->len      = htons(0x0004);
	pDNSAns->ipaddr   = htonl(FILTER_HOST_IP);
	
    pUdpHdr->len      = htons(ntohs(pUdpHdr->len)+sizeof(dns_ans));

	/* send the modified pkt */
    pNewSkb->dev = in;
    if (dev_queue_xmit(pNewSkb) < 0)
    {
        PCTL_ERROR("send dns response pkt fail.");
        return -1;
    }

    return 0;
}
#endif //if 0
#endif	/* PCTL_DNS_REDIRECT */

static inline void init_stats(pctl_stats* pStats)
{
    pStats->total = 0;
    pStats->forbid = 0;
    pStats->timestamp = 0;
}

static inline 
int sort_by_total(const void *entry1, const void *entry2)
{
    return (*(const pctl_log_entry **)entry2)->stats.total - 
            (*(const pctl_log_entry **)entry1)->stats.total;
    
}

static inline 
int sort_by_timestamp(const void *entry1, const void *entry2)
{
    return (*(const pctl_log_entry **)entry2)->stats.timestamp - 
            (*(const pctl_log_entry **)entry2)->stats.timestamp;
}

static inline void update_stats(pctl_stats* pStats, unsigned int stamp, unsigned int forbid)
{
    int min_after = stamp / 60 - pStats->timestamp / 60;
    if(min_after > 0) 
    {
        pStats->timestamp = stamp;
        pStats->total++;
    }else if(min_after < 0) 
    {
        PCTL_DEBUG("stamp < pStats->timestamp, reset timestamp.");
        pStats->timestamp = stamp;
    }
    pStats->forbid = forbid;
}

#if 0
static int log_proc_read(struct seq_file *s, void *unused)
{
    pctl_owner *pOwner = NULL;
    pctl_log *pLog = NULL;
    pctl_history *pDay = NULL;
    pctl_log_entry *entry[PCTL_LOG_NUM] = {0};
    pctl_log_entry *entry_ptr = NULL;
    char* filename = s->private;
    int id = 0;
    int i = 0, j = 0;
    unsigned int num = 0;
    unsigned int gmt_timestamp = 0;
    unsigned int access_num = 0;
    unsigned int forbid_num = 0;

    if(!filename) 
    {
        PCTL_ERROR("filename is NULL.");
        return -1;
    }

    id = simple_strtol(filename, NULL, 10);
    if(id < 0 || id >= PCTL_OWNER_NUM) 
    {
        PCTL_ERROR("id is out of range. id = %d.", id);
        return -1;
    }

    pOwner = owners + id;
    
    write_lock_bh(&pOwner->lock);
    log_update_history(id, GET_TIMESTAMP());
    
    /* print today */
    gmt_timestamp = pOwner->today_stats.timestamp ? pOwner->today_stats.timestamp + 60 * sys_tz.tz_minuteswest : 0;

    seq_printf(s, "%u %u %u %d\n",pOwner->today_stats.total, gmt_timestamp, pOwner->log_len, pOwner->is_block); 
    j=0;
    list_for_each_entry(pLog, &pOwner->log_list, log_node)
    {
        seq_printf(s, "%d,%s,%u,%u,%u\n",
                   j,
                   pLog->entry.host,
                   pLog->entry.stats.total,
                   pLog->entry.stats.timestamp + 60 * sys_tz.tz_minuteswest,
                   pLog->entry.stats.forbid
                    );
        entry[j] = &pLog->entry;
        j++;
    }

    /* print forbid entrys and access entrys */
    num = j;
    if(num == pOwner->log_len) 
    {
        sort(entry, num, sizeof(void*), sort_by_total, NULL);
        for (i = 0; i < num; i++)
        {
            entry_ptr = entry[i];
            if ((forbid_num < PCTL_HISTORY_FORBID_NUM) && ( entry_ptr->stats.forbid == 1))
            {
                seq_printf(s, "forbid_entry,%s,%u\n",
                        entry_ptr->host,
                        entry_ptr->stats.total
                            );
                        forbid_num++;
            }
        
            if ((access_num < PCTL_HISTORY_ACCESS_NUM))
            {
                seq_printf(s, "access_entry,%s,%u\n",
                        entry_ptr->host,
                        entry_ptr->stats.total
                            );
                        access_num++;
            }
        }
     
    }


    /* print history */
    for (i=1; i<= PCTL_HISTORY_DAY_NUM; i++ )
    {
        int idx = ((pOwner->day_idx + PCTL_HISTORY_DAY_NUM - i) % PCTL_HISTORY_DAY_NUM);
        pDay = pOwner->day[idx];

        if(pDay) 
        {

            gmt_timestamp =pDay->day_stats.timestamp ? pDay->day_stats.timestamp + 60 * sys_tz.tz_minuteswest : 0;

            seq_printf(s, "%u %u %u\n",
                       pDay->day_stats.total, gmt_timestamp, pDay->num);

            for(j=0; j<pDay->num; j++) 
            {
                seq_printf(s, "%d,%s,%u,%u\n",
                           j,
                           pDay->log_entry[j].host,
                           pDay->log_entry[j].stats.total,
                           pDay->log_entry[j].stats.timestamp + 60 * sys_tz.tz_minuteswest);
            }
        }else
        {
            seq_printf(s, "%u %u %u\n",0, 0, 0);
        }
    }
    write_unlock_bh(&pOwner->lock);

    return 0;
}

static int log_proc_write(struct file *file, const char* buf, size_t count,  loff_t *data)
{
    int id  = 0;
    char cmd = 0;
    char* filename = file->f_path.dentry->d_iname;
    pctl_owner *pOwner = NULL;

    id = simple_strtol(filename, NULL, 10);
    if(id < 0 || id >= PCTL_OWNER_NUM) 
    {
        PCTL_ERROR("id is out of range. id = %d.", id);
        return -1;
    }

    if(count != 2) 
    {
         PCTL_ERROR("count = %d.", count);
         return -1;
    }
    cmd = buf[0];

    pOwner = owners + id;
    
    write_lock_bh(&pOwner->lock);

    if('f' == cmd) 
    {
        /* clear log */
        log_clear(id);

    }
    else
    {
        PCTL_ERROR("bad cmd %c.", cmd);
    }
    write_unlock_bh(&pOwner->lock);

    return count;
}
#endif /* 0 */

/* liuyuxuan modify begin */

/*
 * fn           static int getChrIndex(const char *pStr, const int len, const char ch)
 * brief        Get index of character in the given string. 
 *
 * param[in]    pStr
 * param[in]    len
 * param[in]    ch
 *
 * return       If the character can be found in string, return index of it. Otherwise, return -1. 
 */
static int getChrIndex(const char *pStr, const int len, const char ch)
{
    int i = 0;

    if (NULL == pStr)
    {
        PCTL_ERROR("pStr is NULL.");
        return -1;
    }

    for (i = 0; i < len; ++i)
    {
        if (pStr[i] == ch)
        {
            return i;
        }
    }

    return -1;
}

/*
 * fn           static int get_id(const char *p_filename)
 * brief        Get id according to the given file. 
 *
 * param[in]    p_filename
 *
 * return       
 */
static int get_id(const char *p_filename)
{
    int id = 0;

    if(!p_filename) 
    {
        PCTL_ERROR("Filename is NULL.");
        return -1;
    }

    id = simple_strtol(p_filename, NULL, 10);
    if (id < 0 || id >= PCTL_OWNER_NUM) 
    {
        PCTL_ERROR("Id is out of range. id = %d.", id);
        return -1;
    }

    return id;
}

/*
 * fn           static bool check_read_flags(void)
 * brief        Check read flags. 
 */
static bool check_read_flags(void)
{
    if (read_flags.history_type < 0 || read_flags.history_type >= HISTORY_TYPE_NUM)
    {
        PCTL_ERROR("Unknown history type. history_type: %d.", read_flags.history_type);
        return false;
    }
    if (read_flags.day > PCTL_HISTORY_DAY_NUM)
    {
        PCTL_ERROR("Day is out of range. day: %d.", read_flags.day);
        return false;
    }

    return true;
}

/*
 * fn           static void parse_read_flags(const char *p_flags_buf, const char *p_filename)
 * brief        Parse read flags. 
 *
 * param[in]    p_flags_buf
 * param[in]    p_filename
 */
static void parse_read_flags(const char *p_flags_buf, const char *p_filename)
{
    int id = 0;
    HISTORY_TYPE history_type = TIME_USED;
    unsigned int day = 0;
    int clear = 0;
    pctl_owner *p_owner = NULL;
    
    id = get_id(p_filename);
    if (id < 0) 
    {
        return;
    }

    sscanf(p_flags_buf, "%d", &clear);
    PCTL_DEBUG("clear: %d.", clear);
    if (CLEAR_HISTORY == clear)
    {
        PCTL_DEBUG("Clear history. ");
        p_owner = owners + id;
        write_lock_bh(&p_owner->lock);
        log_clear(id);
        write_unlock_bh(&p_owner->lock);
        return;
    }

    sscanf(p_flags_buf, "%u %u", &history_type, &day);

    if (history_type < 0 || history_type >= HISTORY_TYPE_NUM)
    {
        PCTL_ERROR("Unknown history type. history_type: %d.", history_type);
        return;
    }
    if (day > PCTL_HISTORY_DAY_NUM)
    {
        PCTL_ERROR("Day is out of range. id: %d.", day);
        return;
    }

    read_flags.history_type = history_type;
    read_flags.day = day;
}

/*
 * fn           static void show_time_used(struct seq_file *s, int id)
 * brief        Show time used of given profile. 
 *
 * param[in]    s
 * param[in]    id
 */
static void show_time_used(struct seq_file *s, int id)
{
    pctl_owner *p_owner = owners + id;

    PCTL_DEBUG("Get time used of day %d. ", read_flags.day);
    if (0 == read_flags.day)
    {
        seq_printf(s, "%u\n", p_owner->today_stats.total);
        PCTL_DEBUG("time used: %d. ", p_owner->today_stats.total);
    }
    else
    {
        seq_printf(s, "%u\n", 0);
        PCTL_ERROR("pctl: No time used of day %d. ", read_flags.day);
    }
}

/*
 * fn           static void show_visit_rank(struct seq_file *s, int id)
 * brief        Show visit rank of given profile.
 *
 * param[in]    s
 * param[in]    id
 */
static void show_visit_rank(struct seq_file *s, int id)
{
    pctl_log *p_log = NULL;
    pctl_log_entry *entry[PCTL_LOG_NUM] = {0};
    int entry_i = 0;
    unsigned int entry_num = 0;
    pctl_owner *p_owner = owners + id;

    PCTL_DEBUG("Get visit rank of day %d. ", read_flags.day);
    if (0 == read_flags.day)
    {
        list_for_each_entry(p_log, &p_owner->log_list, log_node)
        {
            entry[entry_i] = &p_log->entry;
            ++entry_i;
        }
        entry_num = entry_i;
        sort(entry, entry_num, sizeof(void*), sort_by_total, NULL);
        if (entry_num > VISIT_RANK_ENTRY_NUM)
        {
            entry_num = VISIT_RANK_ENTRY_NUM;
        }
        seq_printf(s, "%u\n", entry_num);
        PCTL_DEBUG("visit rank entry num: %d. ", entry_num);
        for (entry_i = 0; entry_i < entry_num; ++entry_i)
        {
            seq_printf(s, "%s %u\n", entry[entry_i]->host, entry[entry_i]->stats.total);
        }
    }
    else
    {
        seq_printf(s, "%u\n", 0);
        PCTL_ERROR("pctl: No visit rank of day %d. ", read_flags.day);
    }
}

/*
 * fn           static void show_block_history(struct seq_file *s, int id)
 * brief        Show blocked history of given profile.
 *
 * param[in]    s
 * param[in]    id
 */
static void show_block_history(struct seq_file *s, int id)
{
    PCTL_BLOCK_LOG *p_block_log = NULL;
    pctl_owner *p_owner = owners + id;

    PCTL_DEBUG("Get blocked history of day %d. ", read_flags.day);
    if (0 == read_flags.day)
    {
        seq_printf(s, "%u\n", p_owner->block_log_len);
        PCTL_DEBUG("block history entry num: %d. ", p_owner->block_log_len);
        list_for_each_entry(p_block_log, &p_owner->block_log_list, log_node)
        {
            seq_printf(s, "%s %u %u\n", 
                       p_block_log->entry.host, 
                       p_block_log->entry.timestamp, 
                       p_block_log->entry.category);
        }
    }
    else
    {
        seq_printf(s, "%u\n", 0);
        PCTL_ERROR("pctl: No block history of day %d. ", read_flags.day);
    }
}

/*
 * fn           static void show_visit_history(struct seq_file *s, int id)
 * brief        Show visit history of given profile.
 *
 * param[in]    s
 * param[in]    id
 */
static void show_visit_history(struct seq_file *s, int id)
{
    pctl_log *p_log = NULL;
    int entry_i = 0;
    unsigned int entry_num = 0;
    pctl_owner *p_owner = owners + id;

    PCTL_DEBUG("Get visit history of day %d. ", read_flags.day);
    if (0 == read_flags.day)
    {
        entry_num = p_owner->log_len < VISIT_LOG_ENTRY_NUM ? p_owner->log_len : VISIT_LOG_ENTRY_NUM;
        seq_printf(s, "%u\n", entry_num);
        list_for_each_entry(p_log, &p_owner->log_list, log_node)
        {
            seq_printf(s, "%s %u\n", p_log->entry.host, 
                p_log->entry.stats.timestamp + 60 * sys_tz.tz_minuteswest); /* App will convert time zone. (bug 624475) */
            ++entry_i;

            if (entry_i >= entry_num)
            {
                break;
            }
        }
    }
    else
    {
        seq_printf(s, "%u\n", 0);
        PCTL_ERROR("pctl: No visit rank of day %d. ", read_flags.day);
    }
}


/*
 * fn           static void show_history(struct seq_file *s)
 * brief        Function to show histroy. 
 *
 * param[in]    s
 */
static void show_history(struct seq_file *s)
{
    int id = 0;
    pctl_owner *p_owner = NULL;

    id = get_id(s->private);
    if (id < 0) 
    {
        return;
    }

    check_read_flags();

    p_owner = owners + id;
    read_lock_bh(&p_owner->lock);

    switch (read_flags.history_type)
    {
    case TIME_USED:
        show_time_used(s, id);
        break;
        
    case VISIT_RANK:
        show_visit_rank(s, id);
        break;
        
    case BLOCK_HISTORY:
        show_block_history(s, id);
        break;

    case VISIT_HISTORY:
        show_visit_history(s, id);
        break;
        
    default:
        ;
    }

    read_unlock_bh(&p_owner->lock);
}

/*
 * fn           static int log_proc_show(struct seq_file *s, void *unused)
 * brief        Function to show log. 
 *
 * param[in]    s
 * param[in]    unused
 *
 * return       
 */
static int log_proc_show(struct seq_file *s, void *unused)
{
    show_history(s);

    return 0;
}

/*
 * fn           static ssize_t log_proc_write(struct file *file, const char* __user buf, 
 *                                        size_t count, loff_t *data)
 * brief        Write function of log proc. 
 *
 * param[in]    file        
 * param[in]    buf         
 * param[in]    count       
 * param[in]    data        
 *
 * return       
 */
static ssize_t log_proc_write(struct file *file, const char __user * buf, size_t count, loff_t *data)
{
    char flags_buf[FLAGS_BUF_LEN + 1] = {0};

    if (copy_from_user(flags_buf, buf, count))
    {
        PCTL_ERROR("copy from user error\n");
        return -1;
    }
    
    parse_read_flags(flags_buf, file->f_path.dentry->d_iname);

    return count;
}

/*
 * fn           static int log_proc_open(struct inode *inode, struct file *file)  
 * brief        Function to open log proc. 
 *
 * param[in]    inode
 * param[in]    file
 *
 * return       
 */
static int log_proc_open(struct inode *inode, struct file *file)  
{  
    return single_open(file, log_proc_show, file->f_path.dentry->d_iname);
}

/*
 * fn           static void show_and_clear_all_history(struct seq_file *s)
 * brief        Function to show all histroy and then clear them. 
 *
 * param[in]    s
 */
static void show_and_clear_all_history(struct seq_file *s)
{
    int id = 0;
    pctl_owner *p_owner = NULL;

    for (id = 0; id < PCTL_OWNER_NUM; ++id)
    {
        p_owner = owners + id;
        write_lock_bh(&p_owner->lock);

        show_time_used(s, id);
        show_visit_rank(s, id);
        show_block_history(s, id);
        show_visit_history(s, id);

        log_clear(id);

        write_unlock_bh(&p_owner->lock);
    }
}


/*
 * fn           static int all_log_proc_show(struct seq_file *s, void *unused)
 * brief        Function to show all log. 
 *
 * param[in]    s
 * param[in]    unused
 *
 * return       
 */
static int all_log_proc_show(struct seq_file *s, void *unused)
{
    show_and_clear_all_history(s);

    return 0;
}

/*
 * fn           static int all_log_proc_open(struct inode *inode, struct file *file)  
 * brief        Function to open all_log proc. 
 *
 * param[in]    inode
 * param[in]    file
 *
 * return       
 */
static int all_log_proc_open(struct inode *inode, struct file *file)
{  
    return single_open(file, all_log_proc_show, file->f_path.dentry->d_iname);
}


/*
 * fn			static bool checkAndResetAddr(const struct sk_buff *pSkb, const unsigned char **pStart, 
 * 											const unsigned char **pEnd)
 * brief		If pStart and pEnd are invalid addr, set them to valid ones.
 *
 * param[in]		pSkb
 * param[in/out]	pStart
 * param[in/out]	pEnd
 *
 * return		void
 */
static void checkAndResetAddr(const struct sk_buff *pSkb, const unsigned char **pStart, 
							const unsigned char **pEnd)
{
	if (NULL == pStart || NULL == pEnd || NULL == pSkb)
	{
		PCTL_ERROR("NULL! start(0x%p) end(0x%p) skb(0x%p). ", pStart, pEnd, pSkb);
		return;
	}

	if (!(*pStart >= pSkb->head && *pStart <= pSkb->end))
	{
		*pStart = pSkb->head;
	}

	if (!(*pEnd >= pSkb->head && *pEnd <= pSkb->end))
	{
		*pEnd = pSkb->end;
	}
}

/* liuyuxuan modify end */


// AP Hash Function
static unsigned int log_hash(const char *str, const int len)
{
    unsigned int hash = 0;
    int i = 0;
 
    for (i=0; i<len; i++)
    {
        if ((i & 1) == 0)
        {
            hash ^= ((hash << 7) ^ (*str++) ^ (hash >> 3));
        }
        else
        {
            hash ^= (~((hash << 11) ^ (*str++) ^ (hash >> 5)));
        }
    }
 
    return (hash & 0x7FFFFFFF) % PCTL_URL_HASH_SIZE;
}

static int log_add(int id, const char* host, int host_len, unsigned int stamp, 
                   bool forbid, unsigned int category)
{
    pctl_owner *pOwner = NULL;
    pctl_log *pLog = NULL;

    struct hlist_head* pHash_list = NULL;
    //struct hlist_node* pHash_node = NULL;

    int ret = 0;
    unsigned int hash = 0;
    
    PCTL_BLOCK_LOG *p_block_log = NULL;
    struct xtm current_time;
    unsigned int current_timestamp = 0;
    /*int day_after = 0;*/
    int chIndex = -1;

    memset(&current_time, 0, sizeof(struct xtm));

    if (!host)
    {
        PCTL_ERROR("host is NULL.");
        return -1;
    }
    if ('\0' == host[0])
    {
        PCTL_ERROR("host is empty.");
        return -1;
    }

    /* We don't record entries like "[FF02::C]:1900". (bug 612594) */
    if ( (chIndex = getChrIndex(host, host_len, '[')) >= 0)
    {
        PCTL_ERROR("Invalid character '[' in host. ");
        return -1;
    }
    /* Remove port(e.g. "192.168.0.1:1900" -> "192.168.0.1"). (bug 612594) */
    if ( (chIndex = getChrIndex(host, host_len, ':')) >= 0)
    {
        host_len = chIndex;
    }

    if (host_len <= 0 || host_len > PCTL_URL_LEN)
    {
        PCTL_ERROR("host_len is out of range, host_len=%d.",host_len);
        return -1;
    }

    pOwner = owners + id;
    /*write_lock_bh(&pOwner->lock);*/

    if (false == forbid)
    {
#if (LINUX_VERSION_CODE == KERNEL_VERSION(2,6,36))
        struct hlist_node *tmp;
#endif /* KERNEL_VERSION(2,6,36) */

        /* 1. find host in hash. if entry exists, update it. */
        hash = log_hash(host, host_len);
        pHash_list = pOwner->hash_list + hash;
#if (LINUX_VERSION_CODE == KERNEL_VERSION(2,6,36))
        hlist_for_each_entry(pLog, tmp, pHash_list, hash_node)
#else /* !KERNEL_VERSION(2,6,36) */
        hlist_for_each_entry(pLog, pHash_list, hash_node)
#endif /* KERNEL_VERSION(2,6,36) */
        {
            if(pLog->entry.host_len == host_len && !memcmp(pLog->entry.host, host, host_len)) 
            {
                PCTL_DEBUG("host is found in hash, update. hash=%d.",hash);

                /* update info */
                update_stats(&pLog->entry.stats, stamp, forbid);

                /* move it to list head */
                list_move(&pLog->log_node, &pOwner->log_list);
                goto out;
            }
        }

        /* 2. It is a new entry. */
        if(pOwner->log_len < PCTL_LOG_NUM) 
        {
            PCTL_DEBUG("alloc new entry. hash=%d.",hash);
         
            /* alloc new entry */
            pLog = kmalloc(sizeof(pctl_log), GFP_ATOMIC);
            if(!pLog) {
                PCTL_ERROR("kmalloc failed.");
                ret = -1;
                goto out;
            }

            memset(pLog, 0, sizeof(pctl_log));
            INIT_LIST_HEAD(&pLog->log_node);
            INIT_HLIST_NODE(&pLog->hash_node);

            memcpy(pLog->entry.host, host, host_len);
            pLog->entry.host[host_len] = '\0';
            pLog->entry.host_len = host_len;
            init_stats(&pLog->entry.stats);
            update_stats(&pLog->entry.stats, stamp, forbid);

            /* add to log list */
            list_add(&pLog->log_node, &pOwner->log_list);
            pOwner->log_len++;

            /* add to log hash */
            hlist_add_head(&pLog->hash_node, &pOwner->hash_list[hash]);

        }
        else /* log list is full */
        {
            PCTL_DEBUG("replace tail node. hash=%d.",hash);

            /* replace tail */
            pLog = list_entry(pOwner->log_list.prev, pctl_log, log_node);
            if(!pLog) 
            {
                PCTL_ERROR("SHOULD NOT happen! log_len=%d.",pOwner->log_len);
                ret = -1;
                goto out;
            }

            memcpy(pLog->entry.host, host, host_len);
            pLog->entry.host[host_len] = '\0';
            pLog->entry.host_len = host_len;
            init_stats(&pLog->entry.stats);
            update_stats(&pLog->entry.stats, stamp, forbid);

            /* update hash */
            hlist_del(&pLog->hash_node);
            hlist_add_head(&pLog->hash_node, &pOwner->hash_list[hash]);

            /* move it to list head */
            list_move(&pLog->log_node, &pOwner->log_list);
        }
    }
    else
    {
        /* liuyuxuan modify begin */
        
        /*
         * brief    Update block history of today. 
         */

        /* Calculate time stamp. */
        /*day_after = stamp / 86400 - pOwner->today_stats.timestamp / 86400;*/
        
        localtime_1(&current_time, stamp);
        current_timestamp = current_time.minutes_day;

        /* Get the latest log entry. */
#if (LINUX_VERSION_CODE == KERNEL_VERSION(2,6,36))
        if (list_empty(&pOwner->block_log_list))
        {
            p_block_log = NULL;
        }
        else
        {
            p_block_log = list_first_entry(&pOwner->block_log_list, PCTL_BLOCK_LOG, log_node);
        }
#else /* !KERNEL_VERSION(2,6,36) */
        p_block_log = list_first_entry_or_null(&pOwner->block_log_list, PCTL_BLOCK_LOG, log_node);
#endif /* KERNEL_VERSION(2,6,36) */
        /* If this host had just been visited, only update the time stamp. */
        if (NULL != p_block_log && host_len == p_block_log->entry.host_len 
            && 0 == memcmp(host, p_block_log->entry.host, host_len))
        {
            p_block_log->entry.timestamp = current_timestamp;
            PCTL_DEBUG("Host %s had just been visited. ", host);
        }
        /* New log entry. */
        else
        {
            /* If log is full, delete the oldest entry. */
            if (pOwner->block_log_len >= BLOCK_LOG_ENTRY_NUM)
            {
                /* Replace tail. */
                p_block_log = list_entry(pOwner->block_log_list.prev, PCTL_BLOCK_LOG, log_node);
                if (!p_block_log) 
                {
                    PCTL_ERROR("Fail to get the oldest block log entry! block_log_len=%d.", 
                               pOwner->block_log_len);
                    ret = -1;
                    goto out;
                }
                
                memcpy(p_block_log->entry.host, host, host_len);
                p_block_log->entry.host[host_len] = '\0';
                p_block_log->entry.host_len = host_len;
                p_block_log->entry.timestamp = current_timestamp;
                p_block_log->entry.category = category;

                /* Move it to head. */
                list_move(&p_block_log->log_node, &pOwner->block_log_list);
            }
            /* Create a new entry. */
            else
            {
                p_block_log = kmalloc(sizeof(PCTL_BLOCK_LOG), GFP_ATOMIC);
                if (!p_block_log) 
                {
                    PCTL_ERROR("kmalloc failed.");
                    ret = -1;
                    goto out;
                }
                
                memset(p_block_log, 0, sizeof(PCTL_BLOCK_LOG));
                INIT_LIST_HEAD(&p_block_log->log_node);
                
                memcpy(p_block_log->entry.host, host, host_len);
                p_block_log->entry.host[host_len] = '\0';
                p_block_log->entry.host_len = host_len;
                p_block_log->entry.timestamp = current_timestamp;
                p_block_log->entry.category = category;

                /* Add to log list. */
                list_add(&p_block_log->log_node, &pOwner->block_log_list);
                ++pOwner->block_log_len;
            }
        }
        /* liuyuxuan modify end */
    }

out:
    /*write_unlock_bh(&pOwner->lock);*/
    return ret;
}

static int log_clear(int id)
{
    pctl_owner *pOwner = owners + id;
    pctl_log* pLog = NULL;
    pctl_log* pTmp = NULL;

    PCTL_BLOCK_LOG *p_block_log;
    PCTL_BLOCK_LOG *p_tmp_block_log;

    /*write_lock_bh(&pOwner->lock);*/

    init_stats(&pOwner->today_stats);
    /* free log entry */
    list_for_each_entry_safe(pLog, pTmp, &pOwner->log_list, log_node)
    {
        hlist_del(&pLog->hash_node);
        list_del(&pLog->log_node);
        if(pLog)
        {
            kfree(pLog);
            pLog = NULL;
        }
        pOwner->log_len--;
    }

    if(pOwner->log_len != 0)
    {
        PCTL_ERROR("pOwner->log_len != 0!");
    }

    /* liuyuxuan add begin */
    list_for_each_entry_safe(p_block_log, p_tmp_block_log, &pOwner->block_log_list, log_node)
    {
        if (NULL == p_block_log)
        {
            PCTL_ERROR("Null pointer. ");
            break;
        }
        list_del(&p_block_log->log_node);
        if (p_block_log)
        {
            kfree(p_block_log);
            p_block_log = NULL;
        }
        --pOwner->block_log_len;
    }

    if(pOwner->block_log_len != 0)
    {
        PCTL_ERROR("Length of block log is not 0 after clearing. pOwner->block_log_len: %d. ", 
                   pOwner->block_log_len);
    }
    /* liuyuxuan add end */

    /* free history */
    /*for(i=0; i<PCTL_HISTORY_DAY_NUM ;i++) 
    {
        if(pOwner->day[i]) 
        {
            kfree(pOwner->day[i]);
            pOwner->day[i] = NULL;
        }
    }
    pOwner->day_idx = 0;*/

    /*write_unlock_bh(&pOwner->lock);*/

    return 0;
}


#if 0
static int log_update_history(int id, unsigned int now)
{
    pctl_log *pLog = NULL;
    pctl_log *pTmp = NULL;
    pctl_history* pDay = NULL;
    pctl_log_entry *entry[PCTL_LOG_NUM] = {0};
    pctl_owner *pOwner = owners + id;
    int day_after = 0;
    int i = 0, num = 0;

    PCTL_BLOCK_LOG *p_block_log = NULL;
    PCTL_BLOCK_LOG *p_tmp_block_log = NULL;

    day_after = now / 86400 - pOwner->today_stats.timestamp / 86400;
    if (0 == day_after || 0 == pOwner->today_stats.timestamp)
    {
        /* do nothing */
        return 0;
    }else if( day_after < 0 || day_after >= PCTL_HISTORY_DAY_NUM )
    {
        /* clear log */
        PCTL_ERROR("clear history. owner_id = %d.",id);
        log_clear(id);
        return 0;
    }

    PCTL_DEBUG("new day event. day_after=%d.",day_after);

    /* save log to history array */
    pDay = pOwner->day[pOwner->day_idx];
    if(NULL == pDay) 
    {
        PCTL_DEBUG("kmalloc id=%d, day_idx=%d.",id, pOwner->day_idx);
        pDay = kmalloc(sizeof(pctl_history), GFP_ATOMIC);
        if(!pDay) 
        {
            PCTL_ERROR("kmalloc failed.");
            return -1;
        }
        pOwner->day[pOwner->day_idx] = pDay;
        memset(pDay, 0, sizeof(pctl_history));
    }else
    {
        PCTL_DEBUG("override id=%d, day_idx=%d.",id, pOwner->day_idx);
        /* override */
        memset(pDay, 0, sizeof(pctl_history));
    }

    list_for_each_entry(pLog, &pOwner->log_list, log_node)
    {
        entry[i] = &pLog->entry;
        i++;
    }
    num = i;
    if(num != pOwner->log_len) 
    {
        PCTL_ERROR("num != pOwner->log_len, %d %d.",num, pOwner->log_len);
        return -1;
    }
    
    /* liuyuxuan modify begin */
    /* 1. sort by count */
    sort(entry, num, sizeof(void*), sort_by_total, NULL);
    if (num > VISIT_RANK_ENTRY_NUM)
    {
        num = VISIT_RANK_ENTRY_NUM;
    }
    /* 2. sort by timestamp */
    /*sort(entry, num, sizeof(void*), sort_by_timestamp, NULL);*/

    /* 3. save history */
    /* Save visit rank entries. */
    pDay->day_stats = pOwner->today_stats;
    pDay->num = num;
    for(i=0; i<num; i++) 
    {
        pDay->log_entry[i] = *entry[i];
    }

    /* Save block history entries. */
    i = 0;
    list_for_each_entry(p_block_log, &pOwner->block_log_list, log_node)
    {
        if (i >= BLOCK_LOG_ENTRY_NUM)
        {
            PCTL_ERROR("Block log of today has exceeded the maximum size. ");
            break;
        }
        memcpy(&pDay->block_log_entry[i], &p_block_log->entry, sizeof(PCTL_BLOCK_LOG_ENTRY));
        ++i;
    }
    if (i != pOwner->block_log_len)
    {
        PCTL_ERROR("Length of block log error. ");
    }
    pDay->block_log_entry_num = i;

    /* 4. clear history not in PCTL_HISTORY_DAY_NUM days. */
    for(i=1; i<day_after; i++) 
    {
        int clear_day_idx = ((pOwner->day_idx + i) % PCTL_HISTORY_DAY_NUM);
        if(pOwner->day[clear_day_idx]) 
        {
            kfree(pOwner->day[clear_day_idx]);
            pOwner->day[clear_day_idx] = NULL;
        }
    }
    pOwner->day_idx = ((pOwner->day_idx + day_after) % PCTL_HISTORY_DAY_NUM);

    /* 5. clear today's log */
    init_stats(&pOwner->today_stats);
    
    /* free log entry */
    list_for_each_entry_safe(pLog, pTmp, &pOwner->log_list, log_node)
    {
        hlist_del(&pLog->hash_node);
        list_del(&pLog->log_node);
        if(pLog)
        {
            kfree(pLog);
            pLog = NULL;
        }
        pOwner->log_len--;
    }
    if(pOwner->log_len != 0)
    {
        PCTL_ERROR("pOwner->log_len != 0!");
    }

    /* Free block log entries. */
    list_for_each_entry_safe(p_block_log, p_tmp_block_log, &pOwner->block_log_list, log_node)
    {
        list_del(&p_block_log->log_node);
        if (p_block_log)
        {
            kfree(p_block_log);
            p_block_log = NULL;
        }
        --pOwner->block_log_len;
    }
    if (pOwner->block_log_len != 0)
    {
        PCTL_ERROR("Length of block log error. block_log_len after clearing: %d. ", 
                   pOwner->block_log_len);
        pOwner->block_log_len = 0;
    }
    
    /* liuyuxuan modify end */

    return 0;
}
#endif /* 0 */

static int url_lib_proc_open(struct inode *inode, struct file *file)  
{  
    return single_open(file, url_lib_proc_read, NULL);
}

static ssize_t url_lib_proc_write(struct file *file, const char __user * buf, size_t count, loff_t *data)
{
    char *ptr_index1 = NULL;
    char *ptr_index2 = NULL;
    char *arg = (char*)buf;
    unsigned int str_len = 0;
    if(count == 0) 
    {
        PCTL_ERROR("count = %d.", (int)count);
        return -1;
    }

    write_lock_bh(&url_lib_lock);
    memset(l_url_lib, 0, sizeof(struct _url_lib));

	while ((ptr_index1 = strchr(arg, ',')) != NULL && (ptr_index2 = strchr(arg, ';')) != NULL)
	{
		if (arg >= ptr_index1 || (ptr_index1 + 1) >= ptr_index2)
		{
			PCTL_ERROR("url lib format error");
			PCTL_ERROR("exp: key1,url1;key2,url2;......keyn,urln;");
			count = -1;
			goto exit;
		}

		str_len = (ptr_index1 - arg) >= PCTL_URL_LEN + 1 ? PCTL_URL_LEN : ptr_index1 - arg;
		if (copy_from_user(l_url_lib->key[l_url_lib->num], arg, str_len))
		{
			PCTL_ERROR("copy from user error");
			count = -1;
			goto exit;
		}
		arg = ptr_index1 + 1;

		str_len = (ptr_index2 - arg) >= PCTL_URL_LEN + 1 ? PCTL_URL_LEN : ptr_index2 - arg;
		if (copy_from_user(l_url_lib->host[l_url_lib->num], arg, str_len))
		{
			PCTL_ERROR("copy from user error");
			count = -1;
			goto exit;
		}
		arg = ptr_index2 + 1;

		l_url_lib->num ++;
	}

exit:
	write_unlock_bh(&url_lib_lock);
	if (count < 0)
	{
		memset(l_url_lib, 0, sizeof(struct _url_lib));
	}
	return count;
}

static int url_lib_proc_read(struct seq_file *s, void *unused)
{
    unsigned int index = 0;
    read_lock_bh(&url_lib_lock);
    for(index = 0; index < l_url_lib->num; index++)
    {
        seq_printf(s, "%s:%s\n", l_url_lib->key[index], l_url_lib->host[index]);
    }
    read_unlock_bh(&url_lib_lock);
    return 0;
}


/*
 * fn			static int daylight_proc_open(struct inode *inode, struct file *file)
 * brief		
 *
 * param[in]	inode
 * param[in]	file
 *
 * return		int
 */
static int daylight_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, daylight_proc_read, NULL);
}

/*
 * fn			static ssize_t daylight_proc_write(struct file *file, const char __user *buf, size_t count, loff_t *data)
 * brief	
 *
 * param[in]	file
 * param[in]	buf
 * param[in]	count
 * param[in]	data
 *
 * return		ssize_t
 */
static ssize_t daylight_proc_write(struct file *file, const char __user *buf, size_t count, loff_t *data)
{

	char buf_tmp[DAYLIGHT_WRITE_BUF_LEN] = {0};

	if (copy_from_user(buf_tmp, buf, count))
	{
		PCTL_ERROR("copy from user error\n");
		return -1;
	}

	sscanf(buf_tmp, "%d", &daylight_offset);
	
	return count;
}

/*
 * fn			static int daylight_proc_read(struct seq_file *s, void *unused)
 * brief		
 *
 * param[in]	s
 * param[in]	unused
 *
 * return		int
 */
static int daylight_proc_read(struct seq_file *s, void *unused)
{
	unsigned int stamp = 0;
	struct rtc_time tm;

	stamp = GET_TIMESTAMP();
	seq_printf(s, "%d\n", daylight_offset);

	rtc_time_to_tm(stamp, &tm);
	PCTL_DEBUG("time :%d-%d-%d %d:%d:%d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	return 0;
}

/* 
 * fn		static bool extractHandshakeFromSSL(const uint8_t *pSSLBuff, uint8_t **ppHandshake) 
 * brief	extract the handshake From SSL packet.
 * details	only get address of the pointer to handshake.
 *
 * param[in]	pSSL - pointer to the start of SSL packet in skb_buff.
 * param[out]	ppHandshake - address of pointer to the start of handshake message wrapped with SSLv3/TLS.
 *
 * return	BOOL
 * retval	true  succeed to extract handshake 
 *		false fail to extract handshake  
 * note		
 */
static bool extractHandshakeFromSSL(const uint8_t *pSSL, uint8_t **ppHandshake)
{
	SSL_MSG ssl;
	
	if ((ssl.type = *pSSL++) != HANDSHAKE)
	{
		return false;
	}
	/*
	ssl.version.majorVersion = *pSSL++;
	ssl.version.minorVersion = *pSSL++;
	*/
	pSSL += 2;
	
	ssl.length = ntohs(*((uint16_t *)pSSL));
	pSSL += 2;
	
	if(0 == ssl.length)
	{
		return false;
	}
	/*ssl.pContent = pSSL;*/
	*ppHandshake = (uint8_t *)pSSL;

	
	return true;
}
/* 
 * fn		static bool extractSNIFromExtensions(const uint8_t *pExtensions, uint8_t *ppSNIExt) 
 * brief	extract SNI extension form extensions.
 * details	get pointer to start position of SNI extension that exists in server name extension.
 *
 * param[in]	pExtensions - pointer to start of extensionList.
 * param[out]	ppSNIExt      - address of pointer to SNI extension.
 *
 * return	bool
 * retval	true - extract SNI extension successfully.
 *          false - extract SNI extension unsuccessfully.
 * note		
 */
static bool extractSNIFromExtensions(const uint8_t *pExtensions, uint8_t **ppSNIExt)
{
	int extensionsLen; /*length of all extensions.*/
	int handledExtLen;/*length of handled extensions.*/
	TLS_EXTENSION ext;

	extensionsLen = ntohs(*((uint16_t *)pExtensions));
	pExtensions += 2;
	
	for (handledExtLen = 0; handledExtLen < extensionsLen; )
	{
		ext.type = ntohs(*((uint16_t *)pExtensions));
		pExtensions += 2;
		ext.length = ntohs(*((uint16_t *)pExtensions));
		pExtensions += 2;
		ext.pData = (ext.length ? (uint8_t *)pExtensions : NULL);
		if (SERVER_NAME == ext.type)
		{
			*ppSNIExt = ext.pData;
			if (ext.length)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		pExtensions += ext.length;
		handledExtLen += (2 + 2 + ext.length);
	}

	return false;
}
/* 
 * fn		static  bool extractSNIFromClientHello(const uint8_t *pClientHello, uint8_t **ppSNIExt) 
 * brief	extract SNI extension(Server_name)represents host_name from client_hello.
 * details	get pointer to start position of SNI extension from client_hello message.
 *
 * param[in]	pClientHello - pointer to start position of client_hello message.
 * param[out]	ppSNIExt - address of pointer to the start position of SNI extension in client_hello.
 *
 * return	bool
 * retval	true -get the SNI represents host_name.
 *			false - doesn't get the right SNI.
 * note		
 */
static bool extractSNIFromClientHello(const uint8_t *pClientHello, uint8_t **ppSNIExt)
{
	HANDSHAKE_CLIENT_HELLO clientHello;
	/*
	clientHello.type = *pClientHello++;
	clientHello.length = NET_3BYTES_TO_HOST_UINT32(pClientHello);
	pClientHello += 3;
	Ignore type and length of client_hello.
	*/
	pClientHello += 4;
	
	clientHello.clientVersion.majorVersion = *pClientHello++;
	clientHello.clientVersion.minorVersion = *pClientHello++;
	/*SNI extension is not supported until TLS 1.0(version 0x0301)*/
	if (clientHello.clientVersion.majorVersion < 3
	 || (3 == clientHello.clientVersion.majorVersion && 0 == clientHello.clientVersion.minorVersion))
	{
		return false;
	}
	/*clientHello.random = pClientHello;*/
	pClientHello += 32;/*length of random is fixed.*/
	clientHello.sessionID.length = *pClientHello++;
	/*clientHello.sessionID.pData = pClientHello;*/
	pClientHello += clientHello.sessionID.length;
	clientHello.cipherSuites.length = ntohs(*((uint16_t *)pClientHello));
	pClientHello += 2;
	/*clientHello.cipherSuites.pData = pClientHello;*/
	pClientHello += clientHello.cipherSuites.length;
	clientHello.compression_methods.length = *pClientHello++;
	/*clientHello.compression_methods.pData = pClientHello;*/
	
	pClientHello += clientHello.compression_methods.length;
	clientHello.pExtensions = (uint8_t *)pClientHello;

	return extractSNIFromExtensions(clientHello.pExtensions, ppSNIExt);
}

/*!
 *\fn           unsigned char *_url_strstr(const unsigned char* start, const unsigned char* end, 
                                        const unsigned char* strCharSet)
 *\brief        find the url in str zone
 *\param[in]    start           start ptr of str zone.
 *\param[in]    end             end ptr of str zone.
 *\param[in]    strCharSet      the url you want to find
 *\return       url postion
 */
static unsigned char *_url_strstr(const unsigned char* start, 
                                  const unsigned char* end, const unsigned char* strCharSet)
{
    const unsigned char *s_temp = start;        /*the s_temp point to the s*/

    int l1, l2;

    l2 = strlen(strCharSet);
    
    if (!l2)
    {
        /*return (unsigned char *)start;*/
        return NULL;
    }

    l1 = end - s_temp + 1;

    while (l1 >= l2)
    {
        l1--;

        if (!memcmp(s_temp, strCharSet, l2))
        {
            return (unsigned char *)s_temp;
        }

        s_temp++;
    }

    return NULL;
}

static inline void localtime_1(struct xtm *r, unsigned int time)
{
	/* Each day has 86400s, so finding the hour/minute is actually easy. */
	r->seconds_day = time % 86400;
	r->second = r->seconds_day % 60;
	r->minutes_day = r->seconds_day / 60;
	r->minute = r->minutes_day % 60;
	r->hour   = r->minutes_day / 60;
}

static inline void localtime_2(struct xtm *r, unsigned int time)
{
	/*
	 * Here comes the rest (weekday, monthday). First, divide the SSTE
	 * by seconds-per-day to get the number of _days_ since the epoch.
	 */
	r->dse = time / 86400;

	/*
	 * 1970-01-01 (w=0) was a Thursday (4).
	 * -1 and +1 map Sunday properly onto 7.
	 */
	r->weekday = (4 + r->dse - 1) % 7 + 1;
}

static int check_bedtime(struct xtm current_time, struct _xt_pctl_info info, u_int8_t offset)
{
    int ret = PCTL_STATUS_OK;
    unsigned int minutes_day_min = 0;
    u_int8_t offset_next = offset;
    struct _xt_pctl_info today_info = info;

    /* For bedtime, there are 2 cases: 
     * 
     * 1. The beginning and end of bedtime is across the day(e.g. 22:00 - 08:00). 
     *    In this case, morning and night should be judged separately: 
     *    24:00 - begin time : judge by bedtime of today
     *    end time - 24:00 : judge by bedtime of tomorrow
     * 
     * 2. The beginning and end of bedtime is in one day(e.g. 01:00 - 08:00). 
     *    In this case, we only need to judge bedtime of today. 
     *
     * Notice: If begin time is the same as end time, block 24h. (bug 683965)
     */
    
    /* today */
    if(0 != (today_info.workday & (1<<offset)))
    {
        if(today_info.workday_begin >= today_info.workday_end)
        {
            today_info.workday_begin = minutes_day_min;
        }
        if(today_info.workday_bedtime) 
        {
            if(current_time.minutes_day >= today_info.workday_begin && 
                current_time.minutes_day <= today_info.workday_end) 
            {
                PCTL_DEBUG("ret = %d.",ret);
                ret = PCTL_STATUS_BEDTIME;
                goto out;
            }
        }
        
    }
    else if( 0 != (today_info.weekend & (1<<offset)))
    {
        if(today_info.weekend_begin >= today_info.weekend_end)
        {
            today_info.weekend_begin = minutes_day_min;
        }
        if(today_info.weekend_bedtime) 
        {
            if( current_time.minutes_day >= today_info.weekend_begin && 
                current_time.minutes_day <= today_info.weekend_end) 
            {
                PCTL_DEBUG("ret = %d.",ret);
                ret = PCTL_STATUS_BEDTIME;
                goto out;
            }
        }
    }
    /* next day */
    offset_next = (6 == offset) ? 0 : offset + 1;
    if( 0 != (info.workday & (1<<offset_next)))
    {
        if(info.workday_bedtime && 
            info.workday_begin >= info.workday_end) 
        {
            if(current_time.minutes_day >= info.workday_begin) 
            {
                PCTL_DEBUG("ret = %d.",ret);
                ret = PCTL_STATUS_BEDTIME;
                goto out;
            }
        }
    }

    else if( 0 != (info.weekend & (1<<offset_next)))
    {
        if(info.weekend_bedtime &&
            info.weekend_begin >= info.weekend_end) 
        {
            if(current_time.minutes_day >= info.weekend_begin) 
            {
                PCTL_DEBUG("ret = %d.",ret);
                ret = PCTL_STATUS_BEDTIME;
                goto out;
            }
        }
    }

out:
    return ret;
}

static int match_time(const struct sk_buff *skb, const struct _xt_pctl_info *info, unsigned int stamp)
{
    int ret = PCTL_STATUS_OK;
    u_int8_t offset = 0;
    pctl_owner *pOwner = owners + info->id;

	struct xtm current_time;

    /* check internet pause */
    if(info->blocked) 
    {
        PCTL_DEBUG("ret = %d.",ret);
        ret = PCTL_STATUS_BLOCKED;
        goto out;
    }

    localtime_1(&current_time, stamp);
    localtime_2(&current_time, stamp);

    /*PCTL_DEBUG("%d %d %d %d %d %d",current_time.month, current_time.monthday, current_time.weekday,
                                   current_time.hour, current_time.minute, current_time.second);*/

    
    if(current_time.weekday >=1 && current_time.weekday <=7)
    {
        offset = current_time.weekday - 1;
    /* not workday and weekend, just return ok */
        if(0 == (info->workday & (1<<offset)) &&
            0 == (info->weekend & (1<<offset)))
        {
            goto out;
        }
    /* workday limit*/
        if(0 != (info->workday & (1<<offset)))
        {
            if(info->workday_limit)
            {
                if(pOwner->today_stats.total >= info->workday_time) 
                {
                    PCTL_DEBUG("ret = %d.",ret);
                    ret = PCTL_STATUS_TIME_LIMIT;
                    goto out;
                }
            }

        }
        
    /* weekend limit */
        if(0 != (info->weekend & (1<<offset)))
        {
            if(info->weekend_limit)
            {
                if(pOwner->today_stats.total >= info->weekend_time) 
                {
                    PCTL_DEBUG("ret = %d.",ret);
                    ret = PCTL_STATUS_TIME_LIMIT;
                    goto out;
                }
            }
        }
    /* bedtime */
        if(info->workday_bedtime || info->weekend_bedtime) 
        {
            ret = check_bedtime(current_time, *info, current_time.weekday - 1);
            if (PCTL_STATUS_BEDTIME == ret)
            {
                PCTL_DEBUG("ret = %d.",ret);
                goto out;
            }
        }

    }
out:
    return ret;
}

/* liuyuxuan add begin */
/*
 * fn           static bool url_is_forbidden(const unsigned char *p_start, 
 *                                           const unsigned char *p_end, 
 *                                           const struct _xt_pctl_info *info, 
 *                                           unsigned int *p_category)
 *                                           char *p_filter_content)
 * brief        Judge whether the url should be filtered. 
 *
 * param[in]    start               The beginning of url. 
 * param[in]    p_end               The end of url. 
 * param[in]    p_info              Info of iptables parameters. 
 * param[out]   p_category          Category which the url belongs to. 
 * param[out]   p_filter_content    Filter content which the url matches. 
 *
 * return       If the url should be filtered, return true. Otherwise, return false. 
 */
static bool url_is_forbidden(const unsigned char *p_start, const unsigned char *p_end, 
                             const struct _xt_pctl_info *info, unsigned int *p_category, 
                             char *p_filter_content)
{
    unsigned int category_i = 0;
    unsigned int first_url_i = 0;
    unsigned int last_url_i = 0;
    unsigned int url_i = 0;
    bool forbidden = false;

    /*unsigned char url[PCTL_MAX_DNS_SIZE + 1] = {0};
    unsigned int url_len = 0;

    memset(url, 0, sizeof(url));
    url_len = end - start + 1;
    if (url_len > PCTL_MAX_DNS_SIZE)
    {
        url_len = PCTL_MAX_DNS_SIZE;
    }
    memcpy(url, start, url_len);
    PCTL_DEBUG("url: %s", url);*/
    
    /* Search url in unblocked list. If found, return true. */
    forbidden = true;
    first_url_i = info->id * PCTL_OWNER_UNBLOCKED_NUM;
    last_url_i = first_url_i + PCTL_OWNER_UNBLOCKED_NUM - 1;
    read_lock_bh(&unblocked_url_lock);
    for (url_i = first_url_i; url_i <= last_url_i; ++url_i)
    {
        if (FALSE == info->unblocked_bitmap[url_i - first_url_i])
        {
            continue;
        }
        if (_url_strstr(p_start, p_end, unblocked_url[url_i]))
        {
            PCTL_DEBUG("==== matched %s in unblocked list ====", unblocked_url[url_i]);
            forbidden = false;
            break;
        }
    }
    read_unlock_bh(&unblocked_url_lock);
    if (!forbidden)
    {
        return forbidden;
    }

    /* Search url in black list. If found, return false. */
    forbidden = false;
    first_url_i = info->id * PCTL_OWNER_BLOCKED_NUM;
    last_url_i = first_url_i + PCTL_OWNER_BLOCKED_NUM - 1;
    read_lock_bh(&blocked_url_lock);
    for (url_i = first_url_i; url_i <= last_url_i; ++url_i)
    {
        if (FALSE == info->blocked_bitmap[url_i - first_url_i])
        {
            continue;
        }
        if (_url_strstr(p_start, p_end, blocked_url[url_i]))
        {
            PCTL_DEBUG("==== matched %s in blocked list ====", blocked_url[url_i]);
            *p_category = URL_CUSTOM;
            memcpy(p_filter_content, blocked_url[url_i], PCTL_URL_LEN);
            forbidden = true;
            break;
        }
    }
    read_unlock_bh(&blocked_url_lock);
    if (forbidden)
    {
        return forbidden;
    }

    /* Search url in url lib. If found, return false. */
    forbidden = false;
    read_lock_bh(&url_lib_lock);
    for (category_i = 1; category_i <= URL_CATEGORY_NUM; ++category_i)
    {
        if (0 == (info->category & (0x1 << category_i)))
        {
            continue;
        }
        first_url_i = l_url_category_indexes[category_i - 1] + 1;
        last_url_i = l_url_category_indexes[category_i];
        if (first_url_i < 0 || last_url_i >= l_url_lib->num)
        {
            PCTL_ERROR("pctl: Index of url category is out of bound. First: %d. Last: %d. ", 
                       first_url_i, last_url_i);
            continue;
        }
        for (url_i = first_url_i; url_i <= last_url_i; ++url_i)
        {
            if ( _url_strstr(p_start, p_end, l_url_lib->host[url_i]))
            {
                PCTL_DEBUG("==== matched %s in url lib ====", l_url_lib->host[url_i]);
                *p_category = category_i;
                memcpy(p_filter_content, l_url_lib->host[url_i], PCTL_URL_LEN);
                forbidden = true;
                break;
            }
        }
    }
    read_unlock_bh(&url_lib_lock);

    return forbidden;
}
/* liuyuxuan add end */

static int match_http(const struct sk_buff *skb, const struct _xt_pctl_info *info, 
                      int status, unsigned int stamp)
{
    int ret = PCTL_STATUS_OK;

    const struct iphdr *iph = NULL;
    struct tcphdr *tcph = NULL;
    unsigned int forbid = 0;

    unsigned char *http_payload_start = NULL;
    unsigned char *http_payload_end = NULL;
    unsigned char *host_start = NULL;
    unsigned char *host_end = NULL;
    /*unsigned char* resp_doamin = NULL;*/
    unsigned char resp_domain[PCTL_URL_LEN + 1] = {0};
#if PCTL_HTTP_REFERER
    unsigned char *referer_start = NULL;
    unsigned char *referer_end = NULL;
#endif

    /* liuyuxuan add begin */
    unsigned int category = 0;
    char filter_content[PCTL_URL_LEN + 1];
#ifdef INCLUDE_IPV6
    const struct ipv6hdr *iph6 = NULL;
    unsigned int offset_tcp = 0;
    struct tcphdr tcph_buf;
#endif /* INCLUDE_IPV6 */

    memset(resp_domain, 0, sizeof(resp_domain));
    memset(filter_content, 0, sizeof(filter_content));

#ifdef INCLUDE_IPV6
    if (4 == ip_hdr(skb)->version)
    {
        iph = ip_hdr(skb);
        tcph = (void *)iph + iph->ihl * 4;
        http_payload_start = (unsigned char *)tcph + tcph->doff * 4;
        http_payload_end = http_payload_start + (ntohs(iph->tot_len) - iph->ihl * 4 - tcph->doff * 4) - 1;
    }
    else if (6 == ipv6_hdr(skb)->version)
    {
        iph6 = ipv6_hdr(skb);
        if (ipv6_find_hdr(skb, &offset_tcp, IPPROTO_TCP, NULL, NULL) < 0)
        {
            PCTL_ERROR("Fail to find ipv6 header. ");
            goto exit;
        }
        tcph = skb_header_pointer(skb, offset_tcp, sizeof(tcph_buf), &tcph_buf);
        http_payload_start = (unsigned char *)tcph + tcph->doff * 4;
        http_payload_end = (unsigned char *)iph6 + ntohs(iph6->payload_len) - 1;
    }
    else
    {
        PCTL_ERROR("Unknown ip protocol version. ");
        goto exit;
    }
#else /* INCLUDE_IPV6 */
    iph = ip_hdr(skb);
    tcph = (void *)iph + iph->ihl * 4;
    http_payload_start = (unsigned char *)tcph + tcph->doff * 4;
    http_payload_end = http_payload_start + (ntohs(iph->tot_len) - iph->ihl*4 - tcph->doff*4) - 1;
#endif /* INCLUDE_IPV6 */

    /* liuyuxuan add end */

	checkAndResetAddr(skb, &http_payload_start, &http_payload_end);

    memset(resp_domain, 0, sizeof(resp_domain));

    if (http_payload_start < http_payload_end)
    {
        host_start = _url_strstr(http_payload_start, http_payload_end, HOST_STR);
        if (host_start)
        {
            host_start += 8;
            host_end = _url_strstr(host_start, http_payload_end, HOST_END_STR);
            if(!host_end) 
            {
                host_start = NULL;
            }
        }

#if PCTL_HTTP_REFERER
        referer_start = _url_strstr(http_payload_start, http_payload_end, REFERER_STR);
        if (referer_start)
        {
            referer_start += 11;

            referer_start = _url_strstr(http_payload_start, http_payload_end, HTTP_STR);
            if(referer_start) 
            {
                referer_start += 7;
            }else
            {
                referer_start = _url_strstr(http_payload_start, http_payload_end, HTTPS_STR);
                if(referer_start)
                {
                    referer_start += 8;
                }
            }

            referer_end = _url_strstr(referer_start, http_payload_end, REFERER_END_STR);
            if(referer_end)
            {
                pTmp = _url_strstr(referer_start, referer_end, REFERER_END_STR2);
                if(pTmp) 
                {
                    referer_end = pTmp;
                }
            }else
            {
                 referer_start = NULL;
            }
        }
#endif
    }

#ifdef DEBUG
    {
        unsigned char* pStr;
        if(host_start) 
        {
            printk("HTTP HOST: ");
            for (pStr = host_start; pStr != host_end; ++pStr)
            {
                printk("%c", *pStr);
            }
        }
#if PCTL_HTTP_REFERER        
        if(referer_start) 
        {
            printk(" REFERER: ");
            for (pStr = referer_start; pStr != referer_end; ++pStr)
            {
                printk("%c", *pStr);
            }     
        }
#endif
        if(host_start)
        {
            printk("\n");
        }
    }
#endif

    if (host_start)
    {
        /* blocked by time check, no need to check */
        if(PCTL_STATUS_OK != status) 
        {
            ret = status;
            goto exit;
        }

        /* liuyuxuan modify begin */
        forbid = url_is_forbidden(host_start, host_end, info, &category, filter_content);
        log_add(info->id, host_start, host_end - host_start, stamp, forbid, category);
        
#if PCTL_HTTP_REFERER
        if (!forbid && referer_start)
        {
            forbid |= url_is_forbidden(referer_start, referer_end, info, &category, filter_content);
            log_add(info->id, referer_start, referer_end - referer_start, stamp, forbid, category);
        }
#endif /* PCTL_HTTP_REFERER */
        ret = forbid ? PCTL_STATUS_FILTER : PCTL_STATUS_OK;
        /* liuyuxuan modify end */
    }

#if PCTL_REDIRECT
    if (host_start &&  PCTL_STATUS_OK != ret && _url_strstr(http_payload_start, http_payload_end, GET_STR))
    {
        http_response(skb, skb->dev, ret, resp_domain);
    }
    else if( (iph->daddr == FILTER_HOST_IP) && IS_TCP_FLAG_SYN(GET_TCP_FLAG(tcph)))
    {
        http_ack(skb, skb->dev);
    }
#endif /* PCTL_REDIRECT */

exit:
    return ret;
}

static int match_https(const struct sk_buff *skb, const struct _xt_pctl_info *info, 
                       int status, unsigned int stamp)
{
    int ret = PCTL_STATUS_OK;

    const struct iphdr *iph = NULL;
    struct tcphdr *tcph = NULL;
    unsigned int forbid = 0;

    unsigned char *sslStart = NULL;
    unsigned char *sslEnd = NULL;
    uint8_t *pHandshake = NULL;
    uint8_t * pSNIExt = NULL;

    TLS_EXTENSION SNIExt;/*format is similar with server name extension*/
    int SNIListLen = 0;
    int handledSNILen = 0; 

#ifdef DEBUG
    int i = 0;
#endif /* DEBUG */

    /* liuyuxuan add begin */
    char filter_content[PCTL_URL_LEN + 1];
    unsigned int category = 0;
#ifdef INCLUDE_IPV6
    const struct ipv6hdr *iph6 = NULL;
    unsigned int offset_tcp = 0;
    struct tcphdr tcph_buf;
#endif /* INCLUDE_IPV6 */

    memset(filter_content, 0, sizeof(filter_content));

#ifdef INCLUDE_IPV6
    if (4 == ip_hdr(skb)->version)
    {
        iph = ip_hdr(skb);
        tcph = (void *)iph + iph->ihl * 4;
        sslStart = (unsigned char *)tcph + tcph->doff * 4;
        sslEnd = sslStart + (ntohs(iph->tot_len) - iph->ihl * 4 - tcph->doff * 4);
    }
    else if (6 == ipv6_hdr(skb)->version)
    {
        iph6 = ipv6_hdr(skb);
        if (ipv6_find_hdr(skb, &offset_tcp, IPPROTO_TCP, NULL, NULL) < 0)
        {
            PCTL_ERROR("Fail to find ipv6 header. ");
            goto exit;
        }
        tcph = skb_header_pointer(skb, offset_tcp, sizeof(tcph_buf), &tcph_buf);
        sslStart = (unsigned char *)tcph + tcph->doff * 4;
        sslEnd = sslStart + (ntohs(iph6->payload_len) + sizeof(struct ipv6hdr) - offset_tcp - tcph->doff * 4);
    }
    else
    {
        PCTL_ERROR("Unknown ip protocol version. ");
        goto exit;
    }
#else /* INCLUDE_IPV6 */
    iph = ip_hdr(skb);
    tcph = (void *)iph + iph->ihl * 4;
    sslStart = (unsigned char *)tcph + tcph->doff * 4;
    sslEnd = sslStart + (ntohs(iph->tot_len) - iph->ihl * 4 - tcph->doff * 4);
#endif /* INCLUDE_IPV6 */
    /* liuyuxuan add end */

	checkAndResetAddr(skb, &sslStart, &sslEnd);

    if(PCTL_STATUS_OK != status) 
    {
        ret = status;
        goto exit;
    }

    if (sslStart >= sslEnd)
    {
        /*UNIDENTIFY*/
        goto exit;
    }
    if ((!extractHandshakeFromSSL(sslStart, &pHandshake))
        || (*pHandshake != CLIENT_HELLO)
        || (!extractSNIFromClientHello(pHandshake, &pSNIExt)))
    {
        /*UNIDENTIFY*/
        goto exit;
    }

    SNIListLen = ntohs(*((uint16_t *)pSNIExt));
    pSNIExt += 2;

    for (handledSNILen = 0; handledSNILen < SNIListLen; )
    {
        SNIExt.type = *pSNIExt++;
        SNIExt.length = ntohs(*((uint16_t *)pSNIExt));
        pSNIExt += 2;
        SNIExt.pData = (uint8_t *)pSNIExt;
        pSNIExt += SNIExt.length;
        /*Does CLENT HELLO  fragment have impact on SNI?*/
        if (pSNIExt > sslEnd)
        {
            /*UNIDENTIFY*/
            break;
        }
        handledSNILen += (1 + 2 + SNIExt.length);

#ifdef DEBUG
        {
            printk("HTTPS HOST: ");
            for (i=0;i<SNIExt.length;i++)
                printk("%c",*(SNIExt.pData+i));
            printk("\n");
        }
#endif

        /* liuyuxuan modify begin */
        if (HOST_NAME == SNIExt.type)
        {
            forbid = url_is_forbidden(SNIExt.pData, pSNIExt, info, &category, filter_content);
            log_add(info->id, SNIExt.pData, pSNIExt - SNIExt.pData, stamp, forbid, category);
            ret = forbid ? PCTL_STATUS_FILTER : PCTL_STATUS_OK;
            break;
        }
        /* liuyuxuan modify end */
    }

exit:
    return ret;
}

#if PCTL_DNS_REDIRECT
static unsigned int _transDomain2Buf(unsigned char *dns, 
                                     unsigned char *buf, signed int bufLen)
{
    signed int index;
    signed int orig_bufLen = bufLen;
    while(('\0' != *dns) && (bufLen > 0))
    {
        for (index = *dns; (index > 0) && (bufLen > 0); index --, bufLen --)
        {
            *(buf++) = *(++dns);
        }
        *(buf ++) = '.';
        dns ++;
        bufLen --;
    }

    if (bufLen < orig_bufLen)
    {
        bufLen ++;
        buf --;
    }
    
    *buf = '\0';
    return (orig_bufLen - bufLen);
}

static int match_dns(const struct sk_buff *skb, const struct _xt_pctl_info *info, 
                       int status, unsigned int stamp)
{
    int ret = PCTL_STATUS_OK;

    const struct iphdr *iph = NULL;
    struct udphdr *udph = NULL;
    int    dns_len = 0;

    int i_count = 0;
    dns_header *pDnsHdr = NULL;
    unsigned char *pTmp = NULL;
    unsigned char domain[PCTL_MAX_DNS_SIZE];
    unsigned int pkt_len = 0;
    unsigned int domain_len = 0;

    /* liuyuxuan add begin */
    unsigned int forbid = 0;
    char filter_content[PCTL_URL_LEN + 1];
    unsigned int category = 0;
#ifdef INCLUDE_IPV6
    const struct ipv6hdr *iph6;
    unsigned int offset_udp = 0;
    struct udphdr udph_buf;
#endif /* INCLUDE_IPV6 */

    memset(filter_content, 0, sizeof(filter_content));
    /* liuyuxuan add end */
    
    if (PCTL_STATUS_OK != status) 
    {
        ret = status;
        return ret;
    }

    /* liuyuxuan add begin */
#ifdef INCLUDE_IPV6
    if (4 == ip_hdr(skb)->version)
    {
        iph = ip_hdr(skb);
        udph = (void *)iph + iph->ihl * 4;
        dns_len = (unsigned int)ntohs(udph->len) - sizeof(struct udphdr) - sizeof(dns_header);
    }
    else if (6 == ipv6_hdr(skb)->version)
    {
        iph6 = ipv6_hdr(skb);
        if (ipv6_find_hdr(skb, &offset_udp, IPPROTO_UDP, NULL, NULL) < 0)
        {
            PCTL_ERROR("Fail to find ipv6 header. ");
            goto exit;
        }
        udph = skb_header_pointer(skb, offset_udp, sizeof(udph_buf), &udph_buf);
        dns_len = (unsigned int)ntohs(udph->len) - sizeof(struct udphdr) - sizeof(dns_header);
    }
    else
    {
        PCTL_ERROR("Unknown ip protocol version. ");
        goto exit;
    }
#else /* INCLUDE_IPV6 */
    iph = ip_hdr(skb);
    udph = (void *)iph + iph->ihl * 4;
    dns_len = (unsigned int)ntohs(udph->len) - sizeof(struct udphdr) - sizeof(dns_header);
#endif /* INCLUDE_IPV6 */
    /* liuyuxuan add end */

    if (dns_len < 0)
    {
        PCTL_DEBUG("dns_len = %d. (<0)", dns_len);
        ret = PCTL_STATUS_OK;
        goto exit;
    }

    if (dns_len >= PCTL_MAX_DNS_SIZE)
    {
        PCTL_DEBUG("dns_len = %d > %d",dns_len, PCTL_MAX_DNS_SIZE);
        ret = PCTL_STATUS_OK;
        goto exit;
    }

    pDnsHdr = (void *) udph + sizeof(struct udphdr);
    if (0 != (ntohs(pDnsHdr->flags) & 0x8000)) /* If not request */
    {
        ret = PCTL_STATUS_OK;
        goto exit;
    }

    pTmp = (unsigned char *)pDnsHdr + sizeof(dns_header);
    for (i_count = 0; i_count < ntohs(pDnsHdr->nQDCount) && pkt_len < dns_len; i_count ++)
    {
        domain_len = _transDomain2Buf(pTmp, domain, PCTL_MAX_DNS_SIZE - 1);

        /* liuyuxuan modify begin */
        forbid = url_is_forbidden(domain, domain + domain_len, info, &category, filter_content);
        if (forbid)
        {
            log_add(info->id, domain, domain_len, stamp, forbid, category);
            ret = PCTL_STATUS_FILTER;
            break;
        }
        /* liuyuxuan modify end */

        pkt_len += domain_len + 4 + 1;
        pTmp    += domain_len + 4 + 1;
    }

#if 0
    if(PCTL_STATUS_OK != ret) 
    {
        if(dns_response(skb, skb->dev, ret) < 0 && (PCTL_STATUS_FILTER == ret))
        {
            ret = PCTL_STATUS_OK;
        }
    }
#endif /* 0 */

exit:
    return ret;
}
#endif

#if GET_BR_LAN_IP
static int lan_ip_proc_open(struct inode *inode, struct file *file)  
{  
    return single_open(file, lan_ip_proc_read, NULL);
}

static ssize_t lan_ip_proc_write(struct file *file, const char __user * buf, size_t count,  loff_t *data)
{
  
    if(count == 0 || count > LAN_IP_MAX_LEN) 
    {
        PCTL_ERROR("count = %d.", (int)count);
        return -1;
    }

    write_lock_bh(&lan_ip_lock);
   
    memset(br_lan_ip, 0, sizeof(br_lan_ip));
    if(copy_from_user(br_lan_ip, buf, count))
    {
        PCTL_ERROR("copy from user error\n");
    }
    write_unlock_bh(&lan_ip_lock);

    return count;
}
static int lan_ip_proc_read(struct seq_file *s, void *unused)
{

    read_lock_bh(&lan_ip_lock);

    seq_printf(s, "%s\n",br_lan_ip);
    read_unlock_bh(&lan_ip_lock);
    return 0;
}
#endif


#if PCTL_DEVICE_INFO

static int device_proc_read(struct seq_file *s, void *unused)
{
    int index = 0;
    unsigned char mac_null[ETH_ALEN] = {0};

    read_lock_bh(&device_info_lock);

    for(index=0; index<DEVICE_INFO_NUM; index++) 
    {
        if(0 != memcmp(devices[index].mac, mac_null, ETH_ALEN)) 
        {
            seq_printf(s, "%02X-%02X-%02X-%02X-%02X-%02X %d\n",
                       devices[index].mac[0],
                       devices[index].mac[1],
                       devices[index].mac[2],
                       devices[index].mac[3],
                       devices[index].mac[4],
                       devices[index].mac[5],
                       devices[index].type);
        }
    }
    read_unlock_bh(&device_info_lock);
    return 0;
}

static int device_proc_open(struct inode *inode, struct file *file)  
{  
    return single_open(file, device_proc_read, NULL);
}

static ssize_t device_proc_write(struct file *file, const char __user * buf, size_t count,  loff_t *data)
{
	char cmd = 0;
	char kbuf[DEVICE_WRITE_BUF_LEN] = {0};
	if (DEVICE_WRITE_BUF_LEN != count)
	{
		PCTL_ERROR("count = %d.", count);
		return -1;
	}

	if (copy_from_user(kbuf, buf, DEVICE_WRITE_BUF_LEN))
	{
		PCTL_ERROR("Fail to copy from user. ");
		return -EFAULT;
	}
	
	cmd = kbuf[0];

    write_lock_bh(&device_info_lock);
    if('f' == cmd) 
    {
        memset(devices, 0, sizeof(devices));
    }
    else
    {
        PCTL_ERROR("bad cmd %c.", cmd);
    }
    write_unlock_bh(&device_info_lock);

    return count;
}

static int mac_hash(const unsigned char* mac)
{
    unsigned int sum = mac[0] + mac[1] + mac[2] + mac[3] + mac[4] + mac[5];
    return (sum % DEVICE_INFO_NUM);
}

static int device_mac_find_slob(const unsigned char* mac)
{
    int hash = mac_hash(mac);
    int index = 0, step = 0;

    for(step=0; step<3; step++) 
    {
        index=(hash + step) % DEVICE_INFO_NUM;
        if(!memcmp(mac, devices[index].mac, ETH_ALEN)) 
        {
            return index;
        }
    }
    return -1;
}

static int device_mac_find_empty_slob(const unsigned char* mac)
{
    int hash = mac_hash(mac);
    int index = 0, step = 0;
    unsigned char mac_null[ETH_ALEN] = {0};

    for(step=0; step<3; step++) 
    {
        index=(hash + step) % DEVICE_INFO_NUM;
        if(!memcmp(mac_null, devices[index].mac, ETH_ALEN)) 
        {
            return index;
        }
    }
    return -1;
}

static DEVICE_TYPE check_user_agent(const unsigned char* start, const unsigned char* end)
{
    DEVICE_TYPE type = DEVICE_TYPE_OTHER;
    char buf[DEVICE_INFO_USER_AGENT_LAN + 1];
    int len = end - start;
    int i = 0;

    if(len >= DEVICE_INFO_USER_AGENT_LAN) 
    {
        PCTL_ERROR("user agent too long.");
        return type;
    }

    memset(buf, 0, DEVICE_INFO_USER_AGENT_LAN + 1);
    memcpy(buf, start, len);
    for(i=0; i<len; i++)
    {
        if(buf[i] >= 'A' && buf[i] <= 'Z') 
        {
            buf[i] = 'a' + (buf[i] - 'A');
        }
    }

    if(strstr(buf, "pad") )
    {
        type = DEVICE_TYPE_TABLET;
    }else if(strstr(buf, "android") ||
             strstr(buf, "phone") || 
             strstr(buf, "mobile") )
    {
        type = DEVICE_TYPE_PHONE;
    }else if(strstr(buf, "windows") ||
             strstr(buf, "mac") )
    {
        type = DEVICE_TYPE_PC;
    }else
    {
        type = DEVICE_TYPE_OTHER;
    }

    return type;
}

static int check_device_info(const struct sk_buff *skb)
{
    const struct ethhdr *pEthHdr = (struct ethhdr *)skb_mac_header(skb);
    const struct iphdr *iph = ip_hdr(skb);
    struct tcphdr *tcph = (void *)iph + iph->ihl*4;
    int index = -1;
    DEVICE_TYPE type = DEVICE_TYPE_OTHER;

    unsigned char* http_payload_start = (unsigned char *)tcph + tcph->doff*4;
    unsigned char* http_payload_end = http_payload_start + ((ntohs(iph->tot_len) - iph->ihl*4 - tcph->doff*4) - 1;

    unsigned char* user_agent_start = NULL;
    unsigned char* user_agent_end = NULL;

	checkAndResetAddr(skb, &http_payload_start, &http_payload_end);

    if(!pEthHdr) 
    {
        PCTL_ERROR("pEthHdr is NULL");
        goto out;
    }

    index = device_mac_find_slob(pEthHdr->h_source);
    if(index != -1)
    {
        /* already get device type ,just return */
        goto out;
    }

    if (http_payload_start < http_payload_end)
    {
        user_agent_start = _url_strstr(http_payload_start, http_payload_end, USER_AGENT_STR);
        if (user_agent_start)
        {
            user_agent_start += 14;
            user_agent_end = _url_strstr(user_agent_start, http_payload_end, USER_AGENT_END_STR);
            if(!user_agent_end) 
            {
                user_agent_start = NULL;
            }
        }
#ifdef DEBUG
        if(user_agent_start) 
        {
            unsigned char* pStr;
            printk("USER_AGENT: ");
            for (pStr = user_agent_start; pStr != user_agent_end; ++pStr)
            {
                printk("%c", *pStr);
            }
            printk("\n");
        }
#endif
        if(user_agent_start) 
        {
            type = check_user_agent(user_agent_start, user_agent_end);

            if(DEVICE_TYPE_OTHER != type) 
            {
                index = device_mac_find_empty_slob(pEthHdr->h_source);
                if(index != -1) 
                {
                    PCTL_DEBUG("device_type = %d, index=%d.",type, index);
                    memcpy(devices[index].mac, pEthHdr->h_source, ETH_ALEN);
                    devices[index].type = type;
                }else
                {
                    PCTL_ERROR("find empty slob failed.");
                    goto out;
                }
            }
        }
    }

out:
    return 0;
}
#endif

/*!
 *\fn           static bool match(const struct sk_buff *skb, struct xt_action_param *param)
 *\brief        find the url in skb (host in http or querys in dns or servername in https(Clienthello) )
 *\return       found or not
 */
static bool match(const struct sk_buff *skb, struct xt_action_param *par)
{   
    int status = PCTL_STATUS_OK;
    unsigned int stamp = GET_TIMESTAMP();

    const struct _xt_pctl_info *info = par->matchinfo;
    const struct iphdr *iph = ip_hdr(skb); /* ipv4 only */
    pctl_owner *pOwner = NULL;
    int id = info->id;

#if PCTL_DEVICE_INFO
    if(id == PCTL_OWNER_ID_ALL)
    {
        /* check type for all device */
        check_device_info(skb);
        return false;
    }
#endif

    if(id < 0 || id >= PCTL_OWNER_NUM) 
    {
        PCTL_ERROR("id is out of range. id = %d.", id);
        return false;
    }

    pOwner = owners + id;
    write_lock_bh(&pOwner->lock);

    status = match_time(skb, info, stamp);

    if(PCTL_STATUS_OK == status) 
    {
        /*log_update_history(id, stamp);*/
        update_stats(&pOwner->today_stats, stamp, 0);
        pOwner->is_block = FALSE;
    }
    else if(pOwner->is_block == FALSE)
    {
        pOwner->is_block = TRUE;
        /* liuyuxuan add begin */
        write_unlock_bh(&pOwner->lock);
        return true;
        /* liuyuxuan add end */
    }
#ifdef CONFIG_ATHRS17_HNAT_WIFI
    record_client_mac(skb, pOwner->is_block);
#endif    
    if (IPPROTO_TCP == iph->protocol)
    {
        struct tcphdr *tcph = (void *)iph + iph->ihl*4;
        
        if(HTTP_PORT == ntohs(tcph->dest))
        {
            status = match_http(skb, info, status, stamp);
        }
        else if (HTTPS_PORT == ntohs(tcph->dest))
        {
            status = match_https(skb, info, status, stamp);
        }
        else if(DNS_PORT == ntohs(tcph->dest))
        {
            status = PCTL_STATUS_OK;
        } 
    }
    else if (IPPROTO_UDP == iph->protocol)
    {
        struct udphdr *udph = (void *)iph + iph->ihl*4;
        if(DNS_PORT == ntohs(udph->dest)) 
        {
#if PCTL_DNS_REDIRECT
            status = match_dns(skb, info, status, stamp);
#else
            status = PCTL_STATUS_OK;
#endif
        }
    }

    write_unlock_bh(&pOwner->lock);

    // PCTL_ERROR("pctl_log=%d pctl_history=%d pctl_owner=%d",
    //            sizeof(pctl_log), sizeof(pctl_history), sizeof(pctl_owner));

    if (PCTL_STATUS_OK != status)
    {
        PCTL_DEBUG("%s", "PCTL_STATUS_OTHERS");
    }

    return (PCTL_STATUS_OK == status)? false : true;
}

/* liuyuxuan add begin */

#ifdef INCLUDE_IPV6

/*
 * fn           static bool match_ipv6(const struct sk_buff *skb, struct xt_action_param *par)
 * brief        Find the url in skb (host in http or querys in dns or servername in https(Clienthello) ).
 *
 * param[in]    skb
 * param[in]    par
 *
 * return       bool
 */
static bool match_ipv6(const struct sk_buff *skb, struct xt_action_param *par)
{
	int status = PCTL_STATUS_OK;
	unsigned int stamp = GET_TIMESTAMP();

	const struct _xt_pctl_info *info = par->matchinfo;
	pctl_owner *pOwner = NULL;
	int id = info->id;

	struct tcphdr tcph_buf;
	struct tcphdr *tcph = NULL;
	struct udphdr udph_buf;
	struct udphdr *udph = NULL;
	unsigned int offset_tcp = 0;
	unsigned int offset_udp = 0;

#if PCTL_DEVICE_INFO
	if (id == PCTL_OWNER_ID_ALL)
	{
		/* check type for all device */
		check_device_info(skb);
		return false;
	}
#endif

	if (id < 0 || id >= PCTL_OWNER_NUM) 
	{
		PCTL_ERROR("id is out of range. id = %d.", id);
		return false;
	}

	pOwner = owners + id;
	write_lock_bh(&pOwner->lock);

	status = match_time(skb, info, stamp);

	if (PCTL_STATUS_OK == status) 
	{
		/*log_update_history(id, stamp);*/
		update_stats(&pOwner->today_stats, stamp, 0);
		pOwner->is_block = FALSE;
	}
	else if (pOwner->is_block == FALSE)
	{
		pOwner->is_block = TRUE;
		
		write_unlock_bh(&pOwner->lock);
		return true;
	}
#ifdef CONFIG_ATHRS17_HNAT_WIFI
	record_client_mac(skb, pOwner->is_block);
#endif

	if (ipv6_find_hdr(skb, &offset_tcp, IPPROTO_TCP, NULL, NULL) >= 0)
	{
		tcph = skb_header_pointer(skb, offset_tcp, sizeof(tcph_buf), &tcph_buf);
		if (tcph)
		{
			PCTL_DEBUG("xt_pctl: recv ipv6 tcp skb");
			if (HTTP_PORT == ntohs(tcph->dest))
			{
				status = match_http(skb, info, status, stamp);
			}
			else if (HTTPS_PORT == ntohs(tcph->dest))
			{
				status = match_https(skb, info, status, stamp);
			}
			else if(DNS_PORT == ntohs(tcph->dest))
			{
				status = PCTL_STATUS_OK;
			}
		}
		
	}
	else if (ipv6_find_hdr(skb, &offset_udp, IPPROTO_UDP, NULL, NULL) >= 0)
	{
		udph = skb_header_pointer(skb, offset_udp, sizeof(udph_buf), &udph_buf);
		if (udph)
		{
			PCTL_DEBUG("xt_pctl: recv ipv6 udp skb");
			if (DNS_PORT == ntohs(udph->dest)) 
			{
#if PCTL_DNS_REDIRECT
				status = match_dns(skb, info, status, stamp);
#else
				status = PCTL_STATUS_OK;
#endif
			}
		}
	}

	write_unlock_bh(&pOwner->lock);

	/* PCTL_DEBUG("pctl_log=%d pctl_history=%d pctl_owner=%d",
				sizeof(pctl_log), sizeof(pctl_history), sizeof(pctl_owner));*/

	if (PCTL_STATUS_OK != status)
	{
		PCTL_DEBUG("%s", "PCTL_STATUS_OTHERS");
	}

	return (PCTL_STATUS_OK == status)? false : true;
}
#endif /* INCLUDE_IPV6 */

/*
 * fn           static int set_url(void __user *user, unsigned int len)
 * brief        Set url to blocked or unblocked list. 
 *
 * param[in]    user    Data in user space. 
 * param[out]   len     Length of data. 
 *
 * return       If successful, return 0. Otherwise, return error code. 
 */
static int set_url(void __user *user, unsigned int len)
{
    URL_INFO url_info;
    unsigned int url_index = 0;
    int ret = 0;

    PCTL_DEBUG("pctl: set_url begin. ");
    if (0 != copy_from_user(&url_info, user, sizeof(URL_INFO)))
    {
        PCTL_DEBUG("pctl: Fail to copy data from user space. ");
        ret = -EFAULT;
        goto exit;
    }

    PCTL_DEBUG("pctl: url type: %d. ", url_info.type);

    if (url_info.id < 0 || url_info.id >= PCTL_OWNER_NUM)
    {
        PCTL_ERROR("pctl: Id(%d) is invalid. ", url_info.id);
        goto exit;
    }

    if (URL_BLOCKED == url_info.type)
    {
        if (url_info.index < 0 || url_info.index >= PCTL_OWNER_BLOCKED_NUM)
        {
            PCTL_ERROR("pctl: Black url index(%d) is invalid. ", url_info.index);
            goto exit;
        }
        
        /* Set blocked url. */
        url_index = url_info.id * PCTL_OWNER_BLOCKED_NUM + url_info.index;
        write_lock_bh(&blocked_url_lock);
        memcpy(blocked_url[url_index], url_info.url, PCTL_URL_LEN);
        PCTL_DEBUG("Set blocked url: index %u, url %s. ", url_index, blocked_url[url_index]);
        write_unlock_bh(&blocked_url_lock);
    }
    else if (URL_UNBLOCKED == url_info.type)
    {
        if (url_info.index < 0 || url_info.index >= PCTL_OWNER_UNBLOCKED_NUM)
        {
            PCTL_ERROR("pctl: Unblocked url index(%d) is invalid. ", url_info.index);
            goto exit;
        }
        
        /* Set unblocked url. */
        url_index = url_info.id * PCTL_OWNER_UNBLOCKED_NUM + url_info.index;
        write_lock_bh(&unblocked_url_lock);
        memcpy(unblocked_url[url_index], url_info.url, PCTL_URL_LEN);
        PCTL_DEBUG("Set unblocked url: index %u, url %s. ", url_index, unblocked_url[url_index]);
        write_unlock_bh(&unblocked_url_lock);
    }
    else
    {
        PCTL_DEBUG("pctl: Unknown url type. ");
    }
    
    PCTL_DEBUG("pctl: set_url end. ");

exit:
    return 0;
}


/*
 * fn           static int do_url_set_ctl(struct sock *sk, int cmd, 
 *                                        void __user *user, unsigned int len)
 * brief        Ctl function of url_sockopts. 
 *
 * param[in]    sk      Socket. 
 * param[in]    cmd     Command code. 
 * param[in]    user    Data in user space. 
 * param[in]    len     Length of data. 
 *
 * return       If successful, return 0. Otherwise, return error code. 
 */
static int do_url_set_ctl(struct sock *sk, int cmd, void __user *user, unsigned int len)
{
    int ret;

    switch (cmd) 
    {
    case PCTL_SET_URL_CTL:
        if (ipt_ctl_hook_url != NULL)
            ret = ipt_ctl_hook_url(user, len);
        else
            ret = -EFAULT;
        break;
        
    default:
            ret = -EINVAL;
    }

    return ret;
}


/* 
 * fn           static void url_lib_init(void)
 * brief        Initialize url library according to file "url_lib". 
 */
static void url_lib_init(void)
{
    mm_segment_t fs;
    struct file *fp = NULL;
    loff_t pos = 0;
    char buf[URL_LIB_BUF_LEN + 1] = {0};
    int buf_offset = 0;
    char *p_url_entry_begin = NULL;
    char *p_url_entry_end = NULL;
    int seq_num = 0;
    int category = 0;
    int last_category = 0;
    int url_i = 0;

    fp = filp_open("/lib/modules/url_lib", O_RDONLY, 0);
    if (!IS_ERR_OR_NULL(fp))
    {
        fs = get_fs();
#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 1, 0)
        set_fs(KERNEL_DS);
#else
        set_fs(get_ds());
#endif
        memset(buf, 0, URL_LIB_BUF_LEN);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
        while (0 < kernel_read(fp, buf + buf_offset, URL_LIB_BUF_LEN - buf_offset, &pos))
#else
        while (0 < vfs_read(fp, buf + buf_offset, URL_LIB_BUF_LEN - buf_offset, &pos))
#endif
        {
            p_url_entry_begin = buf;
            while (NULL != (p_url_entry_end = strnchr(p_url_entry_begin, 
                                                      URL_LIB_BUF_LEN - buf_offset, '\n')))
            {
                if (url_i >= PCTL_URL_LIB_SIZE)
                {
                    PCTL_ERROR("pctl: Url lib is too large. ");
                    return;
                }
                *p_url_entry_end = '\0';
                sscanf(p_url_entry_begin, "%d %s %s %d", &seq_num, l_url_lib->key[url_i], 
                       l_url_lib->host[url_i], &category);
                if (category != last_category)
                {
                    l_url_category_indexes[category - 1] = url_i - 1;
                    last_category = category;
                }
                ++url_i;
                
                p_url_entry_begin = p_url_entry_end + 1;
            }
            buf_offset = URL_LIB_BUF_LEN - (p_url_entry_begin - buf);
            memcpy(buf, p_url_entry_begin, buf_offset);
        }
        l_url_category_indexes[category] = url_i - 1;
        l_url_lib->num = url_i;
        
        set_fs(fs);
        filp_close(fp, NULL);

        PCTL_DEBUG("url num: %d", l_url_lib->num);
        for (category = 0; category <= URL_CATEGORY_NUM; ++category)
        {
            PCTL_DEBUG("category index: %d", l_url_category_indexes[category]);
        }
    }
    else
    {
        PCTL_ERROR("pctl: Fail to open url_lib.");
    }
}
/* liuyuxuan add end */

/*!
 *\fn           static int __init pctl_init(void)
 *\brief        mod init
 *\return       SUCCESS or not
 */
static int __init pctl_init(void)
{
    int i = 0, j = 0;
    char filename[PCTL_PROC_NAME_LEN + 1] = {0};
    pctl_owner *pOwner = NULL;

    /* create proc dir */
    proc_dir = proc_mkdir(PCTL_PROC_DIR, NULL);
    if(!proc_dir) 
    {
        PCTL_ERROR("proc_mkdir failed.");
        return -1;
    }

    for(i = 0; i < PCTL_OWNER_NUM; i++) 
    {
        pOwner = owners + i;

        memset(pOwner, 0, sizeof(pctl_owner));
        pOwner->id = i;

        /* init list */
        INIT_LIST_HEAD(&pOwner->log_list);
        INIT_LIST_HEAD(&pOwner->block_log_list);

        /* init url hash */
        for(j = 0; j < PCTL_URL_HASH_SIZE; j++) 
        {
            INIT_HLIST_HEAD(&pOwner->hash_list[j]);
        }

        /* init rwlock */
        rwlock_init(&pOwner->lock);

        /* create proc */
        sprintf(filename, "%d", pOwner->id);

        pOwner->proc_file = proc_create(filename, S_IRUGO, proc_dir, &log_proc_fops);
        if(!pOwner->proc_file) 
        {
            PCTL_ERROR("create_proc_entry failed.");
            return -1;
        }
    }

    url_lib_proc_file = proc_create(URL_LIB_PROC_FILENAME, 0666, proc_dir, &url_lib_proc_fops);
    if(!url_lib_proc_file) 
    {
        PCTL_ERROR("create_proc_entry failed.");
        return -1;
    }

    l_url_lib = (struct _url_lib*)vmalloc(sizeof(struct _url_lib));
    if(NULL == l_url_lib)
    {
        PCTL_ERROR("vmalloc failed");
        return -1;
    }
    memset(l_url_lib, 0, sizeof(struct _url_lib));

    /* liuyuxuan add begin */
    all_log_proc_file = proc_create(ALL_LOG_PROC_FILENAME, 0666, proc_dir, &all_log_proc_fops);
    if(!all_log_proc_file) 
    {
        PCTL_ERROR("create_proc_entry failed.");
        return -1;
    }
    
    rwlock_init(&url_lib_lock);
    url_lib_init();

    rwlock_init(&blocked_url_lock);
    rwlock_init(&unblocked_url_lock);
    ipt_ctl_hook_url = set_url;
    if (0 < nf_register_sockopt(&url_sockopts))
    {
        PCTL_ERROR("Register url sockopt failed");
        return -1;
    }

    memset(&read_flags, 0, sizeof(READ_FLAGS));
    /* liuyuxuan add end */
    
#if GET_BR_LAN_IP

    lan_ip_proc_file = proc_create(LAN_IP_PROC_FILENAME, 0666, proc_dir, &lan_ip_proc_fops);
    if(!lan_ip_proc_file) 
    {
        PCTL_ERROR("create_proc_entry failed.");
        return -1;
    }

#endif /* GET_BR_LAN_IP */

#if PCTL_DEVICE_INFO

    rwlock_init(&device_info_lock);

    device_proc_file = proc_create(URL_LIB_PROC_FILENAME, 0666, proc_dir, &url_lib_proc_fops);
    if(!device_proc_file) 
    {
        PCTL_ERROR("create_proc_entry failed.");
        return -1;
    }
    
#endif /* PCTL_DEVICE_INFO */

	daylight_file = proc_create(DAYLIGHT_PROC_FILENAME, 0666, proc_dir, &daylight_proc_fops);
	if(!daylight_file) 
    {
        PCTL_ERROR("create_proc_entry failed.");
        return -1;
    }
	daylight_offset = 0;

#ifdef CONFIG_ATHRS17_HNAT_WIFI
    memset(&l_client_info, 0 , sizeof(l_client_info));
    RCU_INIT_POINTER(pctl_drop_packets, pctl_drop_packets_by_mac);
#endif

    return xt_register_matches(pctl_match, ARRAY_SIZE(pctl_match));
}

/*!
 *\fn           static void __exit pctl_exit(void)
 *\brief        mod exit
 *\return       none
 */
static void __exit pctl_exit(void)
{
    int i = 0;

    pctl_owner *pOwner = NULL;
    char filename[PCTL_PROC_NAME_LEN + 1] = {0};

    for(i = 0; i < PCTL_OWNER_NUM; i++) 
    {
        pOwner = owners + i;
        write_lock_bh(&pOwner->lock);
        log_clear(i);
        write_unlock_bh(&pOwner->lock);

       /* remove proc */
        sprintf(filename, "%d", pOwner->id);
        if(pOwner->proc_file) 
        {
            remove_proc_entry(filename, proc_dir);
        }
    }

    remove_proc_entry(URL_LIB_PROC_FILENAME, proc_dir);
    vfree(l_url_lib);

    /* liuyuxuan add begin */
    remove_proc_entry(ALL_LOG_PROC_FILENAME, proc_dir);
    remove_proc_entry(DAYLIGHT_PROC_FILENAME, proc_dir);

    nf_unregister_sockopt(&url_sockopts);
    /* liuyuxuan add end */
    
#if GET_BR_LAN_IP
    remove_proc_entry(LAN_IP_PROC_FILENAME, proc_dir);
#endif

#if PCTL_DEVICE_INFO
    remove_proc_entry(DEVICE_INFO_PROC_FILENAME, proc_dir);
#endif
    remove_proc_entry(PCTL_PROC_DIR, NULL);
#ifdef CONFIG_ATHRS17_HNAT_WIFI
    RCU_INIT_POINTER(pctl_drop_packets, NULL);    
#endif
    xt_unregister_matches(pctl_match, ARRAY_SIZE(pctl_match));
}

/***************************************************************************/
/*                      PUBLIC_FUNCTIONS                     */
/***************************************************************************/

/***************************************************************************/
/*                      GLOBAL_FUNCTIONS                     */
/***************************************************************************/
module_init(pctl_init);
module_exit(pctl_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Miao Wen <miaowen@tp-link.com.cn>");
MODULE_DESCRIPTION("Xtables: parental control");
MODULE_ALIAS("ipt_pctl");
MODULE_ALIAS("ip6t_pctl");
