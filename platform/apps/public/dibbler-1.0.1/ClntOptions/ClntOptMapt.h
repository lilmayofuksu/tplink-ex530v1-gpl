/*
 * Dibbler - a portable DHCPv6
 *
 * authors: zengdongbiao
 *
 * $Id: ClntOptMapt.h
 *
 * Revision 1.0  2015/11/17 
 *
 *
 */

#ifndef CLNTOPTMAPT_H
#define CLNTOPTMAPT_H

#ifdef INCLUDE_IPV6_MAP 

#include "Opt.h"
#include "SmartPtr.h"

class TClntOptMapt : public TOpt
{
 public:
    TClntOptMapt(int type, const char * buf, unsigned short len, TMsg* parent);
    //TOptAddr(int type, SPtr<TIPv6Addr> addr, TMsg * parent);
    size_t getSize();
    char * storeSelf( char* buf);

//	bool isFMR();

//	uint8_t getEaLen();
//	uint8_t getPrefix4Len();
//	uint8_t getIpv4Prefix();
//	uint8_t getPrefix6Len();
//	SPtr<TIPv6Addr> getIpv6Prefix();
//	std::string getPlain();
	bool doDuties();
	
 protected:		
 	/* Suboption S46_RULE (89) parameters.  */
 	uint8_t Flags_; 
 	uint8_t EaLen_;
	uint8_t Ipv4PrefixLen_;
	struct in_addr Ipv4Prefix;
	uint8_t RuleIpv6PrefixLen_;
	struct in6_addr RuleIpv6Prefix;

	/* Subsuboption S46_PORTPARAMS(93) parameters. */
	bool ContainPortParameter;
	uint8_t Offset_;
	uint8_t PSIDLen_;
	uint16_t PSID_;

	/* Suboption S46_DMR (91) parameters.  */
	uint8_t DMRIpv6PrefixLen_;
	
	#ifdef INCLUDE_IPV6_MAP_MAPE
	struct in6_addr DMRIpv6Address;
	#endif /*INCLUDE_IPV6_MAP_MAPE*/
	
	struct in6_addr DMRIpv6Prefix;
};

#endif /* INCLUDE_IPV6_MAP  */

#endif /* MAPT  */