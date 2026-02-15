/*
 * Dibbler - a portable DHCPv6
 *
 * authors: zengdongbiao
 *
 * $Id: ClntOptMapt.c
 *
 * Revision 1.0  2015/11/17 
 *
 *
 */

#ifdef INCLUDE_IPV6_MAP 
 
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include "Portable.h"
#include "OptAddr.h"
#include "Logger.h"
#include "OptRtPrefix.h"
#include "OptGeneric.h"

#include "ClntOptMapt.h"

#ifdef CMM_MSG	/* Add by ZDB: TP-Link COS support, 13Nov15 */
extern "C" {
#include <os_msg.h>
}

#define PSID_MAX_LEN (16)

extern DHCP6C_MAP_DOMAIN mapMsgBody;
#endif /* CMM_MSG */

TClntOptMapt::TClntOptMapt(int type, const char * buf, unsigned short bufSize, TMsg* parent)
    :TOpt(type, parent) {

	Valid = parseOptions(SubOptions, buf, bufSize, parent);
}

#if 0
TClntOptMapt::TClntOptMapt(int type, SPtr<TIPv6Addr> addr, TMsg* parent)
    :TOpt(type, parent) {
    this->Addr = addr;
}
#endif

size_t TClntOptMapt::getSize() {
    int mySize = 4;
	
    return mySize + getSubOptSize();
}

char * TClntOptMapt::storeSelf(char* buf) {
    // store generic header
    buf = writeUint16( buf, OptType );
    buf = writeUint16( buf, getSize() - 4 );

    // store sub-options (if three are any)
    return storeSubOpt(buf);
}

#if 0
bool TClntOptMapt::isFMR() {
	if (Flags_ | 0x01) {
		return true;
	}
	else {
		return false;
	}
}

uint8_t TClntOptMapt::getEaLen() {
    return EaLen_;
}

uint8_t TClntOptMapt::getPrefix4Len() {
    return Ipv4PrefixLen_;
}

uint8_t TClntOptMapt::getIpv4Prefix() {
    return Ipv4Prefix_;
}

uint8_t TClntOptMapt::getPrefix6Len() {
    return Ipv6PrefixLen_;
}

SPtr<TIPv6Addr> TClntOptMapt::getIpv6Prefix() {
    return Ipv6Prefix;
}

std::string TClntOptMapt::getPlain() {
    return Ipv6Prefix->getPlain();
}
#endif

bool TClntOptMapt::doDuties() { 
	int pos=0;
	int optLen = 0;
	int ipv6PrefixBufInUse = 0;
	char *buf;
	unsigned int ruleCnt = 0;
	
	firstOption();
    SPtr<TOptGeneric> opt;
    while(opt = (Ptr*)getOption()) {
		pos = 0;
		optLen = opt->getDataLen();
		buf = opt->getData();

		Log(Debug) << "subOptType: " << opt->getOptType() << ", subOptLen: " << optLen << LogEnd;

		if (!buf) {
			continue;
		}
		
		switch (opt->getOptType())
		{
        case OPTION_S46_RULE:
			Flags_ = readUint8(buf + pos);
			pos += sizeof(uint8_t);
			optLen -= sizeof(uint8_t);

			EaLen_ = readUint8(buf + pos);
			pos += sizeof(uint8_t);
			optLen -= sizeof(uint8_t);
		
			Ipv4PrefixLen_ = readUint8(buf + pos);
			pos += sizeof(uint8_t);
			optLen -= sizeof(uint8_t);

			memcpy(&Ipv4Prefix, buf + pos, sizeof(struct in_addr));
			pos += 4;
			optLen -= 4;
		
			RuleIpv6PrefixLen_ = readUint8(buf + pos);
			pos += sizeof(uint8_t);
			optLen -= sizeof(uint8_t);

			/*  ipv6-prefix (variable length) */
			if ((RuleIpv6PrefixLen_ % 8) == 0) {
				ipv6PrefixBufInUse = RuleIpv6PrefixLen_ >> 3;
			}
			else {
				ipv6PrefixBufInUse = (RuleIpv6PrefixLen_ >> 3) + 1;
			}
			memset(&RuleIpv6Prefix, 0, sizeof(RuleIpv6Prefix));
			memcpy(&RuleIpv6Prefix, buf + pos, ipv6PrefixBufInUse);
		    pos += ipv6PrefixBufInUse;
			optLen -= ipv6PrefixBufInUse;
			
			#ifdef CMM_MSG	
			if (ruleCnt < RULE_NUMS_MAX) {
				mapMsgBody.rule[ruleCnt].isFMR = (Flags_ & 0x1);
				mapMsgBody.rule[ruleCnt].EALen = EaLen_;
				memcpy(&mapMsgBody.rule[ruleCnt].ipv4Prefix, &Ipv4Prefix, sizeof(mapMsgBody.rule[ruleCnt].ipv4Prefix));
				mapMsgBody.rule[ruleCnt].ipv4PrefixLen = Ipv4PrefixLen_;
				memcpy(&mapMsgBody.rule[ruleCnt].ipv6Prefix, &RuleIpv6Prefix, sizeof(mapMsgBody.rule[ruleCnt].ipv6Prefix));
				mapMsgBody.rule[ruleCnt].ipv6PrefixLen = RuleIpv6PrefixLen_;
		
				Log(Debug) << "Flags: " << (int)(mapMsgBody.rule[ruleCnt].isFMR) << LogEnd;
				Log(Debug) << "EaLen: " << (int)(mapMsgBody.rule[ruleCnt].EALen) << LogEnd;
		
				ruleCnt++;
				mapMsgBody.rulesCnt = ruleCnt;
				Log(Debug) << "ruleCnt: " << (int)(mapMsgBody.rulesCnt) << LogEnd;
			}
			#endif /* CMM_MSG */
			
		    while(optLen > 0)
		    {
		        int subCode = readUint16(buf + pos);
				pos += sizeof(uint16_t);
		        int subOptLen = readUint16(buf + pos);
				pos += sizeof(uint16_t);

				optLen -= (8 + subOptLen);
		       
		        if (subCode == OPTION_S46_PORTPARAMS) {
		            ContainPortParameter = true;
					
					Offset_= readUint8(buf + pos);
					pos += sizeof(uint8_t);
					subOptLen -= sizeof(uint8_t);
					Log(Debug) << "Offset: " << (int)Offset_<< LogEnd;

					PSIDLen_= readUint8(buf + pos);
					pos += sizeof(uint8_t);
					subOptLen -= sizeof(uint8_t);
					Log(Debug) << "PSIDLen: " << (int)PSIDLen_<< LogEnd;

					PSID_ = readUint16(buf + pos);
					PSID_ = PSID_ >> (PSID_MAX_LEN - PSIDLen_);
					pos += sizeof(uint16_t);
					subOptLen -= sizeof(uint16_t);
					Log(Debug) << "PSID: " << (int)PSID_<< LogEnd;
					
					#ifdef CMM_MSG	
					mapMsgBody.PSIDLen = PSIDLen_;
					mapMsgBody.PSIDOffset = Offset_;
					mapMsgBody.PSID = (unsigned int)PSID_;
					#endif /* CMM_MSG */

					if (subOptLen != 0) {
						Log(Error) << "error OPTION_S46_PORTPARAMS " << LogEnd;
						return false;
					}
					
		        }
				else {
		            Log(Error) <<"Option opttype = " << subCode<< "n ot supported"<<LogEnd;
		            return false;
		        }
		    }
			
			break;

		case OPTION_S46_DMR:
			DMRIpv6PrefixLen_ = readUint8(buf + pos);
			pos += sizeof(uint8_t);
			optLen -= sizeof(uint8_t);

			/*  ipv6-prefix (variable length) */
			if ((DMRIpv6PrefixLen_ % 8) == 0) {
				ipv6PrefixBufInUse = DMRIpv6PrefixLen_ >> 3;
			}
			else {
				ipv6PrefixBufInUse = (DMRIpv6PrefixLen_>> 3) + 1;
			}
			memset(&DMRIpv6Prefix, 0, sizeof(DMRIpv6Prefix));
			memcpy(&DMRIpv6Prefix, buf + pos, ipv6PrefixBufInUse);
			pos += ipv6PrefixBufInUse;
			
			#ifdef CMM_MSG	
			memcpy(&mapMsgBody.dmrPrefix, &DMRIpv6Prefix, sizeof(struct in6_addr));
			mapMsgBody.dmrPrefixLen = DMRIpv6PrefixLen_;
			#endif /* CMM_MSG */
			
		#ifdef INCLUDE_IPV6_MAP_MAPE
		case OPTION_S46_BR:
			memset(&DMRIpv6Address, 0, sizeof(DMRIpv6Address));
			memcpy(&DMRIpv6Address, buf + pos, 16);
			pos += 16;
			
			#ifdef CMM_MSG	
			memcpy(&mapMsgBody.dmrAddress, &DMRIpv6Address, sizeof(struct in6_addr));
			#endif /* CMM_MSG */
			
			break;
		#endif /*INCLUDE_IPV6_MAP_MAPE*/
		
		default:
			Log(Error) <<"Option opttype=" << opt->getOptType() << "not supported"<<LogEnd;
		    break;
		}
    }	

	#ifdef CMM_MSG	/* Add by ZDB: sent address to COS, 14Nov15.*/
	mapMsgBody.status |= DHCP6C_ASSIGNED_MAPT;
	#endif /* CMM_MSG */
  
    return true;
}

#endif /* INCLUDE_IPV6_MAP  */