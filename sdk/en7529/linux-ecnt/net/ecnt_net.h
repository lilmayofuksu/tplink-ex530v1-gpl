#ifndef _LINUX_ECNT_NET_H
#define _LINUX_ECNT_NET_H
#include <uapi/linux/in.h>
#include <uapi/linux/ecnt_in.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <net/sock.h>
#include <net/inet_sock.h>
#include <ecnt_hook/ecnt_hook.h>


/*for alg switch*/
extern int nf_conntrack_ftp_enable;
extern int nf_conntrack_sip_enable;
extern int nf_conntrack_h323_enable;
extern int nf_conntrack_rtsp_enable;
extern int nf_conntrack_l2tp_enable;
extern int nf_conntrack_ipsec_enable;
extern int nf_conntrack_pptp_enable;
extern int nf_conntrack_portscan_enable;
extern int nf_conntrack_ftp_port;
extern int nf_conntrack_esp_timeout;
extern int nf_conntrack_rtcp_enable;
extern unsigned int nf_conntrack_rtsp_src_ip4_mask;
extern int ecnt_shortcut_app_list_check(char * name);

static inline int ecnt_skbmark_to_sockmark_copy
(struct sock *sk, struct sk_buff *skb)
{
	if ( !sk || !skb )
		return ECNT_CONTINUE;

	sk->sk_mark &= 0x0ff0ffff;
	/* not change QoS info stored into sk->sk_mark */
	sk->sk_mark |= (skb->mark & (~QOS_FILTER_MARK));

	return ECNT_CONTINUE;
}

#if defined(TCSUPPORT_CT_VLAN_BIND)
static inline int ecnt_dns_vlanid_store
(struct inet_sock *inet, struct sock *sk, struct sk_buff *skb)
{
	if ( !inet || !sk || !skb )
		return ECNT_CONTINUE;


	if( ( VBIND_DNSPORT == inet->inet_sport)
		&& (sk->lVlanId < VBIND_INVALID_VLANID) )
	{
		skb->vlan_tags[0] = sk->lVlanId;
		skb->vlan_tag_flag |= VLAN_TAG_FOR_DNS;
	}

	return ECNT_CONTINUE;
}
#endif

#if defined(TCSUPPORT_CT_QOS)
static inline int ecnt_set_qoshigh_hook
(struct sk_buff *skb)
{
	/* set arp packet to first queue */
	skb->mark &= (~QOS_FILTER_MARK);
	skb->mark |= QOS_HH_PRIORITY;
	/* printk("v4 arp packet to first queue.skb->mark is 0x%x\n", skb->mark); */

	return ECNT_CONTINUE;
}
#endif

static inline int ecnt_sock_create_inline_hook(struct socket *sock)
{
#if defined(TCSUPPORT_TEST_SAMBA_SHORTCUT)
	if(strcmp(current->comm, "smbd") == 0){
		sock->sk->sk_smbd_info.smbd_sk = 1;
	}else{
		sock->sk->sk_smbd_info.smbd_sk = 0;
	}
#endif

	return ECNT_CONTINUE;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,115)
static inline int ecnt_sock_accept4_inline_hook(struct socket *sock)
{
    if (ecnt_shortcut_app_list_check(current->comm))
    {
        sock->sk->sk_shortcut_info.shortcut_sk = 1;
        sock->sk->sk_shortcut_info.shortcut_speed = 0;
        sock->sk->sk_shortcut_info.out_dev = NULL;
    }
    else
    {
        sock->sk->sk_shortcut_info.shortcut_sk = 0;
        sock->sk->sk_shortcut_info.shortcut_speed = 0;
        sock->sk->sk_shortcut_info.out_dev = NULL;	
    }

	return ECNT_CONTINUE;
}
#endif
#endif

