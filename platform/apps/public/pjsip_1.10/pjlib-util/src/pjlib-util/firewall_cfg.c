/*  Copyright(c) 2009-2011 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 * file		firewall_cfg.c
 * brief		Provide a API for Developer to configurate Firewall filter rule.
 * details	
 *
 * author	Yu Chuwei
 * version	1.0.0
 * date		30Nov11
 *
 * warning	
 *
 * history \arg	
 */

#include <pjlib-util/firewall_cfg.h>
#include <pjlib-util/errno.h>
#include <pj/assert.h>
#include <pj/errno.h>
#include <pj/string.h>
#include <pj/compat/socket.h>
#include <pjsua-lib/pjsua.h>
#include <pjsua-lib/pjsua_internal.h>

/**************************************************************************************************/
/*                                           DEFINES                                              */
/**************************************************************************************************/

/* 
 * brief	Macro of a digit constant--256
 */
#define BUF_SIZE_256 256

/* 
 * brief	Macro of a digit constant--128
 */
#define BUF_SIZE_128 128

/* 
 * brief	Macro of a digit constant--16
 */
#define BUF_SIZE_16 16

/**************************************************************************************************/
/*                                           TYPES                                                */
/**************************************************************************************************/
/* 
 * brief	Bind protocol type to name(string).
 */
typedef struct _PROTO_NAME_MAP
{
	PJ_TRANSPORT_PROTO type; /*protocol name*/
	pj_str_t name; 			 /*string name*/
}PROTO_NAME_MAP;

/**************************************************************************************************/
/*                                           EXTERN_PROTOTYPES                                    */
/**************************************************************************************************/
extern struct pjsua_data pjsua_var;

/**************************************************************************************************/
/*                                           LOCAL_PROTOTYPES                                     */
/**************************************************************************************************/
/* 
 * fn		char* lookupProtoName(PJ_TRANSPORT_PROTO proto)
 * brief	Lookup protocol's string name in Array "l_protoName", according to the protocol type.
 * details	
 *
 * param[in]	proto:protocol type for which we lookup name.
 * param[out]	
 *
 * return	protocol name if successful.
 * retval	NULL if protocol type is valid.
 *
 * note		
 */
pj_int32_t lookupProtoName(PJ_TRANSPORT_PROTO proto);
/**************************************************************************************************/
/*                                           VARIABLES                                            */
/**************************************************************************************************/
static int tp_system(char *cmd)
{
	int status = 0;
	int pid;

	if((pid = vfork()) < 0)
	{
		status = -1;
	}
	else if(pid == 0)
	{
		const char *new_argv[4];

		new_argv[0] = "sh";
		new_argv[1] = "-c";
		new_argv[2] = cmd;
		new_argv[3] = NULL;

		if(execve("/bin/sh", (char *const *)new_argv, NULL) < 0)
		{
			printf("fail to execve %s\n",cmd);
			exit(1);
		}

		else
		{
			exit(0);
		}
	}
	else
	{
		waitpid(pid, &status, 0);
	}

	return status;
}
static PROTO_NAME_MAP l_protoName[] = 
{
	{	PJ_TRANSPORT_UDP, 
		{"udp", 3}
	},
	
	{	PJ_TRANSPORT_TCP, 
		{"tcp", 3}
	},
	
	{	PJ_TRANSPORT_ALL, 
		{"udp or tcp", 10}
	}
};
/**************************************************************************************************/
/*                                           LOCAL_FUNCTIONS                                      */
/**************************************************************************************************/
/* 
 * fn		char* lookupProtoName(PJ_TRANSPORT_PROTO proto)
 * brief	Lookup protocol's string name in Array "protoName", according to the protocol type.
 * details	
 *
 * param[in]	proto:protocol type for which we lookup name.
 * param[out]	
 *
 * return	index of array l_protoName if successful.
 * retval	-1 if protocol type is valid.
 *
 * note		
 */
pj_int32_t lookupProtoName(PJ_TRANSPORT_PROTO proto)
{
	pj_uint32_t index;
	pj_uint32_t size = PJ_ARRAY_SIZE(l_protoName);

	PJ_ASSERT_RETURN(proto >= PJ_TRANSPORT_UDP && proto <= PJ_TRANSPORT_ALL, PJLIB_UTIL_EFIREWALLINPROTO);

	for (index = 0; index < size; ++index)
	{
		if (l_protoName[index].type == proto)
		{
			return index;
		}
	}

	return -1;	
}
/**************************************************************************************************/
/*                                           PUBLIC_FUNCTIONS                                     */
/**************************************************************************************************/
/* 
 * fn		pj_status_t pj_firewall_set_rule_accept(PJ_FIREWALLCFG_ACTION action, PJ_TRANSPORT_PROTO proto,
 *																char* dst, char* dstNetmask, pj_uint16_t dport,
 *																char* src, pj_uint16_t sport)
 * brief	Add a rule to netfilter and the target is "ACCEPT". Table is "filter", chains are "INPUT" and 
 *			"FORWARD".
 * details	
 *
 * param[in]	action:	to add or delete this rule. 
 *					proto:	network protocol of this rule."udp" or "tcp" or "udp or tcp"
 *					dst:		destination IP of this rule. now is ipv4. must not be NULL or "0.0.0.0".
 *					dstNetmask: netmask of the destination network, may be NULL or "0.0.0.0". If 
 *								set no netmask, you must set it to NULL, not "0.0.0.0", or the rule will be
 *								wrong.
 *					dport:	port of the destination host.
 *					src:		source IP of this rule. now is ipv4. must not be NULL or "0.0.0.0".
 *					srcNetmask: netmask of the source network, may be NULL or "0.0.0.0". If 
 *								set no netmask, you must set it to NULL, not "0.0.0.0", or the rule will be
 *								wrong.
 *					sport:	port of the source host.
 * param[out]	
 *
 * return	PJ_SUCCESS if set successfully.
 *				PJ_EINVAL if arguments are invalid.
 *				PJ_
 * retval	
 *
 * note		
 */
pj_status_t pj_firewall_set_rule_accept(PJ_FIREWALLCFG_ACTION action, PJ_TRANSPORT_PROTO proto,
												const pj_str_t* dst, const pj_str_t* dstNetmask, pj_int32_t dport,
												const pj_str_t* src, const pj_str_t* srcNetmask, pj_int32_t sport,
												PJ_FIREWALLCFG_CTRL ctrl
#ifdef INCLUDE_VOIP_SUPPORT_IPV6
												, int family
#endif /* INCLUDE_VOIP_SUPPORT_IPV6 */
												, pj_uint32_t tcMark)
{
/* cxadd for set mark for bandwidth control */
#define INNER_DEBUG		printf
#define SET_MARK		(1)
	
	char act;
	pj_str_t* protoName = NULL;
	char destination[BUF_SIZE_128] = {0};
	char source[BUF_SIZE_128] = {0};
	char dportStr[BUF_SIZE_16] = {0};
	char sportStr[BUF_SIZE_16] = {0};
#if SET_MARK
	char mark_destination[BUF_SIZE_128] = {0};
	char mark_source[BUF_SIZE_128] = {0};
	char mark_dportStr[BUF_SIZE_16] = {0};
	char mark_sportStr[BUF_SIZE_16] = {0};
#endif
	int index;
	char cmd[BUF_SIZE_256];
	int ret;
	const char *cmd_iptables = "iptables";
	
	PJ_ASSERT_RETURN(PJ_FIREWALLCFG_ADD == action 
							|| PJ_FIREWALLCFG_DEL == action, PJLIB_UTIL_EFIREWALLINACTION);
	PJ_ASSERT_RETURN(proto >= PJ_TRANSPORT_UDP 
							&& proto <= PJ_TRANSPORT_ALL, PJLIB_UTIL_EFIREWALLINPROTO);

	if (PJ_FIREWALLCFG_IN_NONE == ctrl)
	{
		return PJ_SUCCESS;
	}

	switch (action)
	{
	case PJ_FIREWALLCFG_ADD:
		act = 'A';
	break;

	case PJ_FIREWALLCFG_DEL:
		act = 'D';
	break;

	default:
		return PJLIB_UTIL_EFIREWALLINACTION;
	}


	index = lookupProtoName(proto);
	if (index < 0)
	{
		return PJLIB_UTIL_EFIREWALLINPROTO;
	}
	protoName = &l_protoName[index].name;


	if (NULL == dst || 0 == dst->slen)
	{
		destination[0] = 0;
#if SET_MARK
		mark_destination[0] = 0;
#endif
	}
	else
	{
		if (dstNetmask && dstNetmask->slen > 0)
		{
			sprintf(destination, "-d %.*s/%.*s", (int)dst->slen, dst->ptr, (int)dstNetmask->slen, dstNetmask->ptr);
#if SET_MARK
			sprintf(mark_destination, "-s %.*s/%.*s", (int)dst->slen, dst->ptr, (int)dstNetmask->slen, dstNetmask->ptr);
#endif
		}
		else
		{
			sprintf(destination, "-d %.*s", (int)dst->slen, dst->ptr);
#if SET_MARK
			sprintf(mark_destination, "-s %.*s", (int)dst->slen, dst->ptr);
#endif
		}
	}

		
	if (NULL == src || 0 == src->slen)
	{
		source[0] = 0;
#if SET_MARK
		mark_source[0] = 0;
#endif
	}
	else
	{
		if (srcNetmask && srcNetmask->slen > 0)
		{
			sprintf(source, "-s %.*s/%.*s", (int)src->slen, src->ptr, (int)srcNetmask->slen, srcNetmask->ptr);
#if SET_MARK
			sprintf(mark_source, "-d %.*s/%.*s", (int)src->slen, src->ptr, (int)srcNetmask->slen, srcNetmask->ptr);
#endif
		}
		else
		{
			sprintf(source, "-s %.*s", (int)src->slen, src->ptr);
#if SET_MARK
			sprintf(mark_source, "-d %.*s", (int)src->slen, src->ptr);
#endif
		}
	}

	if (dport >= 0)
	{
		sprintf(dportStr, "--dport %d", dport);
#if SET_MARK
		sprintf(mark_dportStr, "--sport %d", dport);
#endif
	}

	if (sport >= 0)
	{
		sprintf(sportStr, "--sport %d", sport);
#if SET_MARK
		sprintf(mark_sportStr, "--dport %d", sport);
#endif
	}

#ifdef INCLUDE_VOIP_SUPPORT_IPV6
	if (AF_INET6 == family)
	{
		cmd_iptables = "ip6tables";
	}
#endif /* INCLUDE_VOIP_SUPPORT_IPV6 */

	ret = sprintf(cmd, "%s -%c INPUT -p %.*s %s %s %s %s -j ACCEPT",
						cmd_iptables, act, (int)protoName->slen, protoName->ptr, destination, dportStr, source, sportStr);
	if (ret >= PJ_ARRAY_SIZE(cmd))
	{
		return PJLIB_UTIL_EFIREWALLINBUFSMALL;
	}
	INNER_DEBUG("\033[0;40;32m""%s""\033[0m\n", cmd);
	
	if (tp_system(cmd) < 0)
	{
		return PJLIB_UTIL_EFIREWALLINIPTABLES;
	}

#if SET_MARK
	/* mark defined from oal_tc.h and rsl_tc.c 
	 * 0x8000 defined for all wan incoming traffic	(MASK_FOR_WAN2LAN in oal_tc.h)
	 * 0x61 defined for VOIP MARK.	(in rsl_tc.c)
	 * should we defined these macro together?
	 * CM 2014-10-15
	 */
	ret = snprintf(cmd, PJ_ARRAY_SIZE(cmd), "%s -t mangle -%c  OUTPUT -p %.*s %s %s %s %s -j MARK --set-mark 0x%x",
						cmd_iptables,act, (int)protoName->slen, protoName->ptr, mark_destination, mark_dportStr, 
						mark_source, mark_sportStr, tcMark);
						
	if (ret >= PJ_ARRAY_SIZE(cmd))
	{
		return PJLIB_UTIL_EFIREWALLINBUFSMALL;
	}
	INNER_DEBUG("\033[0;40;32m""%s""\033[0m\n", cmd);
	
	if (system(cmd) < 0)
	{
		return PJLIB_UTIL_EFIREWALLINIPTABLES;
	}

#if 0	/* 匹配后return，这是为了防止再被其他规则重新打MARK，但是这就导致不能再其后加上其他目的的target */
	ret = snprintf(cmd, PJ_ARRAY_SIZE(cmd), "%s -%c OUTPUT -p %.*s %s %s %s %s -j RETURN",
						cmd_iptables, act, (int)protoName->slen, protoName->ptr, mark_destination, mark_dportStr, mark_source, mark_sportStr);
	if (ret >= PJ_ARRAY_SIZE(cmd))
	{
		return PJLIB_UTIL_EFIREWALLINBUFSMALL;
	}
	INNER_DEBUG("\033[0;40;32m""%s""\033[0m\n", cmd);
	

	if (system(cmd) < 0)
	{
		return PJLIB_UTIL_EFIREWALLINIPTABLES;
	}

#endif
#endif

	if (PJ_FIREWALLCFG_IN_NAT_NETFILTER == ctrl)
	{
		ret = sprintf(cmd, "%s -t nat -%c PREROUTING_VOIP -p %.*s %s %s %s %s -j ACCEPT",
							cmd_iptables, act, (int)protoName->slen, protoName->ptr, destination, dportStr, source, sportStr);
		if (ret >= PJ_ARRAY_SIZE(cmd))
		{
			return PJLIB_UTIL_EFIREWALLINBUFSMALL;
		}
		
		if (system(cmd) < 0)
		{
			return PJLIB_UTIL_EFIREWALLINIPTABLES;
		}
	}
	
	return PJ_SUCCESS;
}

pj_uint8_t pj_firewall_get_fwtype(int family)
{
	if (AF_INET == family)
	{
		return pjsua_var.fwType;
	}
#ifdef INCLUDE_VOIP_SUPPORT_IPV6
	else
	{
		return pjsua_var.fwType6;
	}
#endif /* INCLUDE_VOIP_SUPPORT_IPV6 */
	return PJSUA_INVALID_ID;
}
/**************************************************************************************************/
/*                                           GLOBAL_FUNCTIONS                                     */
/**************************************************************************************************/

