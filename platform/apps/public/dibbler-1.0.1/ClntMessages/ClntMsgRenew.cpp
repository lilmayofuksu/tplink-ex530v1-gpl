/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 * changes: Krzysztof Wnuk <keczi@poczta.onet.pl>
 *          Michal Kowalczuk <michal@kowalczuk.eu>
 *
 * released under GNU GPL v2 only licence
 *
 * $Id: ClntMsgRenew.cpp,v 1.23 2009-03-24 23:17:17 thomson Exp $
 *
 */

#include "ClntMsgRenew.h"
#include "DHCPConst.h"
#include "ClntOptIA_NA.h"
#include "ClntOptIA_PD.h"
#include "OptDUID.h"
#include "ClntOptOptionRequest.h"
#include "ClntOptStatusCode.h"
#include "Logger.h"
#include "ClntTransMgr.h"
#include "ClntIfaceMgr.h"
#include <cmath>
#include "OptDomainLst.h"

#ifdef CMM_MSG	
extern "C" {
#include <os_msg.h>
}
extern CMSG_FD dhcp6cMsgFd;
extern CMSG_BUFF dhcp6cMsg;
extern DHCP6C_INFO_MSG_BODY dhcp6cInfoMsgBody;
#endif /* CMM_MSG */

TClntMsgRenew::TClntMsgRenew(List(TAddrIA) IALst,
                             List(TAddrIA) PDLst)
    :TClntMsg(0, SPtr<TIPv6Addr>(), RENEW_MSG)
{
   // set transmission parameters
    IRT=REN_TIMEOUT;
    MRT=REN_MAX_RT;
    MRC=0;
    RT=0;

    if (IALst.count() + PDLst.count() == 0) {
        Log(Error) << "Unable to send RENEW. No IAs and no PDs defined." << LogEnd;
        IsDone = true;
        return;
    }

    // retransmit until T2 has been reached or any address has expired
    //it should be the same for all IAs
    unsigned int timeout = DHCPV6_INFINITY;

    SPtr<TAddrIA> ia;
    if (IALst.count()) {
        IALst.first();
	ia = IALst.get();
    } else {
        PDLst.first();
	ia = PDLst.get();
    }

    if (!ia) {
	Log(Error) << "No IA to renew. Something is wrong." << LogEnd;
	IsDone = true;
	return;
    }

    MRD      = ia->getT2Timeout();  
    Iface    = ia->getIfindex();
    PeerAddr_ = ia->getSrvAddr();

    if (RT>MRD) 
        RT=MRD;

    // store our DUID
    Options.push_back(new TOptDUID(OPTION_CLIENTID, ClntCfgMgr().getDUID(), this));

    // and say who's this message is for
    if (IALst.count())
      	Options.push_back( new TOptDUID(OPTION_SERVERID,IALst.getFirst()->getDUID(),this));
    else
	Options.push_back( new TOptDUID(OPTION_SERVERID,PDLst.getFirst()->getDUID(),this));
    
    //Store all IAs to renew
    IALst.first();
    while(ia=IALst.get()) {
	      if (timeout > ia->getT2Timeout())
	          timeout = ia->getT2Timeout();
	      Options.push_back(new TClntOptIA_NA(ia,this));
    }

    PDLst.first();
    while (ia=PDLst.get()) {
	      if (timeout > ia->getT2Timeout())
	          timeout = ia->getT2Timeout();
	      Options.push_back(new TClntOptIA_PD(ia, this));
    }

    appendRequestedOptions();
    appendAuthenticationOption();

    IsDone = false;
    send();
}


void TClntMsgRenew::answer(SPtr<TClntMsg> Reply)
{
    SPtr<TOptDUID> ptrDUID;
    ptrDUID = (Ptr*) Reply->getOption(OPTION_SERVERID);
    if (!ptrDUID) {
        Log(Warning) << "Received REPLY message without SERVER ID option. Message ignored." << LogEnd;
        return;
    }
    SPtr<TDUID> duid = ptrDUID->getDUID();

    SPtr<TOpt> opt;
    unsigned int iaCnt = 0;
#ifdef CMM_MSG	
    unsigned int nobinding = 0;
#endif
    // get DUID
    SPtr<TOptDUID> srvDUID;
    srvDUID = (Ptr*) this->getOption(OPTION_SERVERID);
    
    SPtr<TClntOptOptionRequest> ptrOptionReqOpt=(Ptr*)getOption(OPTION_ORO);
    SPtr<TClntIfaceIface> iface = (Ptr*)ClntIfaceMgr().getIfaceByID(getIface());
    if (!iface) {
        Log(Error) << "Unable to find physical interface with ifindex=" << getIface() << LogEnd;
        return;
    }
    SPtr<TClntCfgIface> cfgIface = ClntCfgMgr().getIface( getIface());
    
#ifdef CMM_MSG	
    Log(Debug) << "interface name: " << iface->getFullName() << LogEnd;
    memset(&dhcp6cMsgFd, 0 , sizeof(CMSG_FD));
    memset(&dhcp6cMsg, 0 , sizeof(CMSG_BUFF));
    memset(&dhcp6cInfoMsgBody, 0, sizeof(DHCP6C_INFO_MSG_BODY));	
    strncpy(dhcp6cInfoMsgBody.intfName, iface->getName(), sizeof(dhcp6cInfoMsgBody.intfName));
#endif /* CMM_MSG */

    Reply->firstOption();
    // for each option in message... (there should be only one IA option, as we send 
    // separate RENEW for each IA, but we check all options anyway)
    while ( opt = Reply->getOption() ) {
        switch (opt->getOptType()) {
            case OPTION_IA_NA: {
                iaCnt++;
                SPtr<TClntOptIA_NA> ptrOptIA = (Ptr*)opt;
                if (ptrOptIA->getStatusCode()!=STATUSCODE_SUCCESS) {
                    if(ptrOptIA->getStatusCode() == STATUSCODE_NOBINDING){
#ifdef CMM_MSG	
                        dhcp6cInfoMsgBody.replyStaCode = STATUSCODE_NOBINDING;
                        dhcp6cInfoMsgBody.status |= DHCP6C_ASSIGNED_ADDR;
                        nobinding = 1;
                        Log(Debug) << "Recives IA_PD with NoBinding status, Send NoBinding to Cos!" << LogEnd;
                        break;
#else	
                        /* Modify by lwj, when the client get status code NoBinding, 
                         * the client should send Request Message 
                         */
                        SPtr<TAddrIA> ia;
                        SPtr<TDUID> duid;
                        List(TAddrIA) requestIALst;
                        int ifaceID = 0;

                        requestIALst.clear();
                        ClntAddrMgr().firstIA();
                        while( ia=ClntAddrMgr().getIA() ) {
                            if (ia->getIAID() == ptrOptIA->getIAID()){
                                requestIALst.append(ia);
                                ia->setState(STATE_INPROCESS);
                                duid = ia->getDUID();
                                ifaceID = ia->getIfindex();
                            }
                        }

                        if (requestIALst.count()) {
                            // create REQUEST message
                        	Log(Debug) << "Recives IA_NA with NoBinding status, Send Request [Renew]!" << LogEnd;
                        	ClntTransMgr().sendRequest(requestIALst, duid, ifaceID);
                        } 
                        //ClntTransMgr().sendRequest(Options, Iface);
                        IsDone = true;
                        return;
#endif
                    }
                    else {
                        SPtr<TClntOptStatusCode> status = (Ptr*) ptrOptIA->getOption(OPTION_STATUS_CODE);
                        Log(Warning) << "Received IA (iaid=" << ptrOptIA->getIAID() << ") with status code " << 
                        StatusCodeToString(status->getCode()) << ": " 
                             << status->getText() << LogEnd;
                        break;
                   }
                }
                else{ 
                    ptrOptIA->firstAddr();
                    SPtr<TClntOptIAAddress> iaAddr;
                    while (iaAddr = ptrOptIA->getAddr()) {
                        if (iaAddr->getValid() < iaAddr->getPref()) {
                            Log(Warning) << "IA Address  preferred lifetime > vaild lifetime send renew" << LogEnd;
                            ClntTransMgr().sendRenew();
                            IsDone = true;
                            return;
                        }
                    }
                }
                ptrOptIA->setContext(srvDUID->getDUID(), SPtr<TIPv6Addr>(), Reply->getIface());
                ptrOptIA->doDuties();
                
                break;
            }
            case OPTION_IA_PD: {
                iaCnt++;
                SPtr<TClntOptIA_PD> pd = (Ptr*) opt;
                if (pd->getStatusCode() != STATUSCODE_SUCCESS) {
                    if(pd->getStatusCode() == STATUSCODE_NOBINDING){
#ifdef CMM_MSG
                        dhcp6cInfoMsgBody.replyStaCode = STATUSCODE_NOBINDING;
                        dhcp6cInfoMsgBody.status |= DHCP6C_ASSIGNED_PREFIX;
                        nobinding = 1;
                        Log(Debug) << "Recives IA_PD with NoBinding status, Send NoBinding to Cos!" << LogEnd;
                        break;
#else
                        /* Modify by lwj, when the client get status code NoBinding, 
                         * the client should send Request Message 
                         */
                        SPtr<TAddrIA> ia;
                        SPtr<TDUID> duid;
                        List(TAddrIA) requestIALst;
                        int ifaceID = 0;

                        requestIALst.clear();
                        ClntAddrMgr().firstIA();
                        while( ia=ClntAddrMgr().getIA() ) {
                            if (ia->getIAID() == pd->getIAID()){
                                requestIALst.append(ia);
                                ia->setState(STATE_INPROCESS);
                                duid = ia->getDUID();
                                ifaceID = ia->getIfindex();
                            }
                        }

                        if (requestIALst.count()) {
                            // create REQUEST message
                        	Log(Debug) << "Recives IA_PD with NoBinding status, Send Request [Renew]!" << LogEnd;
                        	ClntTransMgr().sendRequest(requestIALst, duid, ifaceID);
                        } 
                        //ClntTransMgr().sendRequest(Options,Iface);
                        IsDone = true;
                        return;
#endif
                    }
                    else{
                            SPtr<TClntOptStatusCode> status = (Ptr*) pd->getOption(OPTION_STATUS_CODE);
                        Log(Warning) << "Received PD (iaid=" << pd->getIAID() << ") with status code " << 
                            StatusCodeToString(status->getCode()) << ": " 
                    	         << status->getText() << LogEnd;
                        break;
                   }
               }
               else {
                    pd->firstPrefix();
                    SPtr<TClntOptIAPrefix> ppref;
                    while (ppref = pd->getPrefix()) {
                        if (ppref->getValid() < ppref->getPref()){
                            Log(Warning) << "prefrred lifetime > vaild lifetime send renew" << LogEnd;
                            ClntTransMgr().sendRenew();
                            IsDone = true;
                            return;
                        }
                    } 
               }

                /* add by lwj, For Reply Message with status code success, but don't contained any IA_PD addresss, must return */
                if (!pd->countPrefix()){
                    Log(Warning) << "Recevied PD (iaid=" << pd->getIAID() << ") without Prefix addresss" << LogEnd;
                    return ;
                }
                pd->setContext(srvDUID->getDUID(), SPtr<TIPv6Addr>(), (TMsg*)this);
                pd->doDuties();
                break;
            }
            case OPTION_DNS_SERVERS: {
                SPtr<TOptAddrLst> dnsservers = (Ptr*) opt;
                cfgIface->setDNSServerState(STATE_CONFIGURED);
                iface->setDNSServerLst(duid, Reply->getRemoteAddr(), dnsservers->getAddrLst());
#ifdef CMM_MSG	
                List(TIPv6Addr) dnsSrvsAddrs = dnsservers->getAddrLst();
                SPtr<TIPv6Addr> dnsSrvAddr;
                int dnsCnt = 0;

                dnsSrvsAddrs.first();
                while (dnsSrvAddr = dnsSrvsAddrs.get()) 
                {
                    if (dnsCnt >= 2)
                    {
                        Log(Debug) << "Too many dnsserver address, Only send two of them to cos." << LogEnd;
                        break;
                    }
                    memcpy(&dhcp6cInfoMsgBody.dns[dnsCnt++], dnsSrvAddr->getAddr(), sizeof(struct in6_addr));
                }

                if (dnsCnt > 0) 
                {
                    dhcp6cInfoMsgBody.status |= DHCP6C_ASSIGNED_DNS;
                }
#endif /* CMM_MSG */
                break;
            }
#ifdef CMM_MSG	
			case OPTION_AFTR_NAME:
				{
					SPtr<TOptDomainLst> dsliteOpt = (Ptr*)opt;
					strncpy(dhcp6cInfoMsgBody.dsliteName, dsliteOpt->getPlain().c_str(), sizeof(dhcp6cInfoMsgBody.dsliteName));
					dhcp6cInfoMsgBody.dsliteName[sizeof(dhcp6cInfoMsgBody.dsliteName) - 1] = '\0';
					dhcp6cInfoMsgBody.status |= DHCP6C_ASSIGNED_DSLITE_NAME;
					Log(Debug) << "Received OPTION_AFTR_NAME: "<< dhcp6cInfoMsgBody.dsliteName << LogEnd;				
				}
#endif /* CMM_MSG */
            case OPTION_ORO:
            case OPTION_RELAY_MSG:
            case OPTION_INTERFACE_ID:
            case OPTION_IAADDR:
            case OPTION_RECONF_MSG:
                Log(Warning) << "Illegal option (" << opt->getOptType() 
                    << ") in received REPLY message." << LogEnd;
                break;
            default:
                // what to do with unknown/other options? execute them
                opt->setParent(this);
                opt->doDuties();
	    }
    }

	/* if reply have any IA Message, ignore it,add by lwj*/
	if (0 == iaCnt){
		return ;
	}

#ifdef CMM_MSG
    if (dhcp6cInfoMsgBody.status != 0) {
        dhcp6cMsg.type = CMSG_IPV6_DHCP6C_STATUS;
        memcpy(dhcp6cMsg.content, &dhcp6cInfoMsgBody, sizeof(DHCP6C_INFO_MSG_BODY));
        Log(Debug) << "revive renew answer dibbler client send dhcp6c message to cos!" << LogEnd;
        msg_connCliAndSend(CMSG_ID_COS, &dhcp6cMsgFd, &dhcp6cMsg);
    }
#endif

    //Here we received answer from our server, which updated the "whole information"
    //There is no use to send Rebind even if server realesed some addresses/IAs
    //in such a case new Solicit message should be sent
    IsDone = true;
}

/** 
 * @brief changes IA state to not cofigured.
 * 
 * @param iaid
 */
void TClntMsgRenew::releaseIA(long iaid)
{
    SPtr<TAddrIA> ia = ClntAddrMgr().getIA(iaid);
    if(ia){
        ia->setState(STATE_NOTCONFIGURED);
    }  
}

void TClntMsgRenew::doDuties()
{
    /// @todo: increase RT from REN_TIMEOUT to REN_MAX_RT

    // should we send RENEW once more or start sending REBIND
    if (!MRD) 
    {
	Log(Notice) << "RENEW remains unanswered and timeout T2 reached, so REBIND will be sent." << LogEnd;
        ClntTransMgr().sendRebind(Options,getIface());
        IsDone = true;
        return;
    }
    send();
}


bool TClntMsgRenew::check()
{
    // this should never happen
    return false;
}

std::string TClntMsgRenew::getName() const {
    return "RENEW";
}

TClntMsgRenew::~TClntMsgRenew() {
}
