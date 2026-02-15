#ifndef _LINUX_ECNT_KENREL_H
#define _LINUX_ECNT_KENREL_H
#include <ecnt_hook/ecnt_hook.h>

#if defined(TCSUPPORT_CT_JOYME2)
extern int panic_write2(const char* buf);
#endif

static inline void ecnt_reset_kthread_cpus_allowed(struct task_struct *task)
{
	/* Binding kernel thread to CPU0 if support VOIP */
	#ifdef TCSUPPORT_VOIP
	cpumask_var_t new_mask;
	*(unsigned long *)new_mask = 0x1;
	set_cpus_allowed_ptr(task, new_mask);
	#endif
}

static inline int ecnt_panic_write2_hook(const char *buf)
{
	int ret = -1;
#if defined(TCSUPPORT_CT_JOYME2)
	ret = panic_write2(buf);
#endif
	return ret;
}

extern int g_capable_user_root_switch;
static inline int ecnt_ns_capable_inline_hook(struct user_namespace *ns, int cap, bool *ret_code)
{
#if defined(TCSUPPORT_CT_JOYME4)
	struct task_struct *cur_task = NULL;
	int max_level = 4; /* max level: tr69 -> sh -> reboot -> sh -> xxx  */

	if( g_capable_user_root_switch )
	{	
	cur_task = current;
	/* tr69 or  boa or reboot*/
	while ( cur_task->parent && max_level >= 0 )
	{
			if ( strcmp(cur_task->comm, "tr69" )
				|| strcmp(cur_task->comm, "boa" )
				|| strcmp(cur_task->comm, "reboot") )
		{
			*ret_code = true;
			return ECNT_RETURN;
		}
		cur_task = cur_task->parent;
		max_level --;
	}
	}
#endif

	return ECNT_CONTINUE;
}

#define ECNT_BIN_NET_NETFILTER_TABLE_HOOK \
{ CTL_INT,	NET_NF_CONNTRACK_FTP_ENABLE,		"nf_conntrack_ftp_enable" },\
{ CTL_INT,	NET_NF_CONNTRACK_SIP_ENABLE,		"nf_conntrack_sip_enable" },\
{ CTL_INT,	NET_NF_CONNTRACK_H323_ENABLE,		"nf_conntrack_h323_enable" },\
{ CTL_INT,	NET_NF_CONNTRACK_RTSP_ENABLE,		"nf_conntrack_rtsp_enable" },\
{ CTL_INT,	NET_NF_CONNTRACK_L2TP_ENABLE,		"nf_conntrack_l2tp_enable" },\
{ CTL_INT,	NET_NF_CONNTRACK_IPSEC_ENABLE,		"nf_conntrack_ipsec_enable" },\
{ CTL_INT,	NET_NF_CONNTRACK_PPTP_ENABLE,		"nf_conntrack_pptp_enable" },\
{ CTL_INT,	NET_NF_CONNTRACK_PORTSCAN_ENABLE,	"nf_conntrack_portscan_enable" },\
{ CTL_INT,	NET_NF_CONNTRACK_FTP_PORT,			"nf_conntrack_ftp_port" },\
{ CTL_INT,	NET_NF_CONNTRACK_ESP_TIMEOUT,		"nf_conntrack_esp_timeout" },\
{ CTL_INT,	NET_NF_CONNTRACK_RTCP_ENABLE,		"nf_conntrack_rtcp_enable" },\
{ CTL_INT,	NET_NF_CONNTRACK_RTSP_SRC_IP4_MASK,		"nf_conntrack_rtsp_src_ip4_mask" },\

#endif