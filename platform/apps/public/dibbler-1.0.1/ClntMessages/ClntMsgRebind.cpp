/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 * changes: Michal Kowalczuk <michal@kowalczuk.eu>
 *
 * released under GNU GPL v2 only licence
 *
 */

#include <cmath>
#include <iostream>
#include <sstream>
#include "SmartPtr.h"
#include "ClntMsg.h"
#include "ClntMsgRebind.h"
#include "ClntOptIA_NA.h"
#include "ClntOptIA_PD.h"
#include "OptDUID.h"
#include "OptAddr.h"
#include "AddrIA.h"
#include "Logger.h"
#include "ClntOptOptionRequest.h"
#include "ClntOptStatusCode.h"
#include "ClntMsgRequest.h"
#include "ClntTransMgr.h"

#ifdef CMM_MSG	/* Add by ZDB: TP-Link COS support, 13Nov15 */
extern "C" {
#include <os_msg.h>
}

extern CMSG_FD dhcp6cMsgFd;
extern CMSG_BUFF dhcp6cMsg;
extern DHCP6C_INFO_MSG_BODY dhcp6cInfoMsgBody;
#endif /* CMM_MSG */


using namespace std;

TClntMsgRebind::TClntMsgRebind(TOptList ptrOpts, int iface)
  :TClntMsg(iface, SPtr<TIPv6Addr>(), REBIND_MSG)
{
    Options=ptrOpts;
    IRT=REB_TIMEOUT;
    MRT=REB_MAX_RT;
    MRC=0;
    RT=0;

    // there are options copied from RENEW. Get rid of some of them
    delOption(OPTION_ELAPSED_TIME);
    delOption(OPTION_SERVERID);
    SPtr<TOpt> opt;
    
    // calculate timeout (how long should be the REBIND message transmitted)
    unsigned long maxMRD=0;    
    firstOption();
    while(opt=getOption())
    {
      switch (opt->getOptType()) {
        case OPTION_IA_NA:
          {
            SPtr<TClntOptIA_NA> ptrIA=(Ptr*) opt;
            SPtr<TAddrIA> ptrAddrIA= ClntAddrMgr().getIA(ptrIA->getIAID());
            if (ptrAddrIA && maxMRD<ptrAddrIA->getMaxValidTimeout())
              maxMRD=ptrAddrIA->getMaxValidTimeout();
            break;
          }
        case OPTION_IA_PD:
          {
            SPtr<TClntOptIA_PD> pd = (Ptr*) opt;
            SPtr<TAddrIA> addrPd = ClntAddrMgr().getPD(pd->getIAID());
            if (addrPd && maxMRD<addrPd->getMaxValidTimeout())
              maxMRD=addrPd->getMaxValidTimeout();
            break;
          }
      }
    }
    MRD= maxMRD;

    appendElapsedOption();
    appendAuthenticationOption();

    IsDone = false;    
    send();
}

void TClntMsgRebind::answer(SPtr<TClntMsg> Reply)
{
	/* add by lwj, For Rebind Message receives Nobinding status, RFC 3315 18.1.8*/
	SPtr<TOpt> opt;
    unsigned int iaCnt = 0; 
    
    // get DUID
    SPtr<TOptDUID> srvDUID;
    srvDUID = (Ptr*) Reply->getOption(OPTION_SERVERID);
    SPtr<TDUID> duid = srvDUID->getDUID();
    
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

    while ( opt = Reply->getOption() ) {
        switch (opt->getOptType()) {
            case OPTION_IA_NA: {
                iaCnt++;
                SPtr<TClntOptIA_NA> ptrOptIA = (Ptr*)opt;
                if (ptrOptIA->getStatusCode()!=STATUSCODE_SUCCESS) {
                    if(ptrOptIA->getStatusCode() == STATUSCODE_NOBINDING){
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
                            Log(Debug) << "Recives IA_NA with NoBinding status, Send Request [Rebind]!" << LogEnd;
                            ClntTransMgr().sendRequest(requestIALst, duid, ifaceID);
                        } 
                        //ClntTransMgr().sendRequest(Options, Iface);
                        IsDone = true;
                        return;
                    }
                    else{
                    SPtr<TClntOptStatusCode> status = (Ptr*) ptrOptIA->getOption(OPTION_STATUS_CODE);
                        Log(Warning) << "Received IA (iaid=" << ptrOptIA->getIAID() << ") with status code " << 
                        StatusCodeToString(status->getCode()) << ": " 
                             << status->getText() << LogEnd;
                        break;
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
                    }
                    else{
                        SPtr<TClntOptStatusCode> status = (Ptr*) pd->getOption(OPTION_STATUS_CODE);
                        Log(Warning) << "Received PD (iaid=" << pd->getIAID() << ") with status code " << 
                        StatusCodeToString(status->getCode()) << ": " 
                             << status->getText() << LogEnd;
                        break;
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
                iaCnt++;
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

	/* remove by lwj, 19Jan16, For CE-Router test Script 20160107, No need Send a New Rebind */
	/* if reply message without any IA_PD, Send a new rebind message */
	/* if (this->getOption(OPTION_IA_PD) && (!Reply->getOption(OPTION_IA_PD))){
		Log(Warning) << "Rebind Message received Reply Message without IA_PD, send new one rebind" << LogEnd;
		ClntTransMgr().sendRebind(Options,getIface());
		IsDone = true;
		return ;
	}*/

	if (0 == iaCnt){
		Log(Info) << "Rebind Message recived Reply Message without Any IA, ignore it" << LogEnd;
		return ;
    }

#ifdef CMM_MSG
    if (dhcp6cInfoMsgBody.status != 0) {
            dhcp6cMsg.type = CMSG_IPV6_DHCP6C_STATUS;
            memcpy(dhcp6cMsg.content, &dhcp6cInfoMsgBody, sizeof(DHCP6C_INFO_MSG_BODY));
            Log(Debug) << "revive rebind answer dibbler client send dhcp6c message to cos!" << LogEnd;
            msg_connCliAndSend(CMSG_ID_COS, &dhcp6cMsgFd, &dhcp6cMsg);
    }
#endif

	IsDone = true;
    //TClntMsg::answer(Reply);
    return;

#if 0
  /// @todo: Fix REPLY support for REBIND
  
SPtr<TOpt> opt;
    
    // get DUID
    SPtr<TClntOptServerIdentifier> ptrDUID;
    ptrDUID = (Ptr*) Reply->getOption(OPTION_SERVERID);
    
    SPtr<TClntOptOptionRequest> ptrOptionReqOpt=(Ptr*)getOption(OPTION_ORO);

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
                    ClntTransMgr().sendRequest(Options,Iface);
                    IsDone = true;
                    return;
                }else{
		    SPtr<TClntOptStatusCode> status = (Ptr*) ptrOptIA->getOption(OPTION_STATUS_CODE);
                    Log(Warning) << "Received IA (iaid=" << ptrOptIA->getIAID() << ") with status code " <<
                        StatusCodeToString(status->getCode()) << ": "
                                 << status->getText() << LogEnd;
                    break;
                }
            }
            ptrOptIA->setContext(ClntIfaceMgr, ClntTransMgr, ClntCfgMgr, ClntAddrMgr,
                                 ptrDUID->getDUID(), SPtr<TIPv6Addr>() /*NULL*/, Reply->getIface());

            ptrOptIA->doDuties();
            break;
        }
        case OPTION_IA_PD: {
            iaCnt++;
            SPtr<TClntOptIA_PD> pd = (Ptr*) opt;
            if (pd->getStatusCode() != STATUSCODE_SUCCESS) {
                if(pd->getStatusCode() == STATUSCODE_NOBINDING){
                    ClntTransMgr->sendRequest(Options,Iface);
                    IsDone = true;
                    return;
                }
                else{
                    SPtr<TClntOptStatusCode> status = (Ptr*) pd->getOption(OPTION_STATUS_CODE);
                    Log(Warning) << "Received PD (iaid=" << pd->getIAID() << ") with status code " <<
                        StatusCodeToString(status->getCode()) << ": "
                                 << status->getText() << LogEnd;
                    break;
                }
            }
            pd->setContext(ClntIfaceMgr, ClntTransMgr, ClntCfgMgr, ClntAddrMgr, ptrDUID->getDUID(), 0, (TMsg*)this);
            pd->doDuties();
            break;
        }
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
    //Here we received answer from our server, which updated the "whole information"
    //There is no use to send Rebind even if server realesed some addresses/IAs
    //in such a case new Solicit message should be sent
 
   if(iaCnt) IsDone = true;
#endif
}

void TClntMsgRebind::updateIA(SPtr <TClntOptIA_NA> ptrOptIA,
			      SPtr<TOptDUID> optSrvDUID, 
			      SPtr<TOptAddr> optUnicast) {

    SPtr< TAddrIA> ptrAddrIA;
    bool found = false;
    
    // ..find IA in addrMgr...
    ClntAddrMgr().firstIA();
    while  (ptrAddrIA = ClntAddrMgr().getIA() ) {
	if (ptrOptIA->getIAID() == ptrAddrIA->getIAID()) {
	    found = true;
	    break;
	}
    }

    if (found) {

	// set new server's DUID to handle this IA
	ptrAddrIA->setDUID(optSrvDUID->getDUID());

	//
	if (optUnicast) {
	    ptrAddrIA->setUnicast(optUnicast->getAddr());
	} else {
	    ptrAddrIA->setMulticast();
	}

	// IAID found, set up new received options.
	SPtr<TAddrAddr> ptrAddrAddr;
	SPtr<TClntOptIAAddress> ptrOptAddr;
	
	// are all addrs configured?
	if (ptrOptIA->countAddr() != ptrAddrIA->countAddr() ) {
	    this->releaseIA(ptrOptIA->getIAID());
	}
	
	// for each address in IA option...
	ptrOptIA->firstAddr();
	while (ptrOptAddr = ptrOptIA->getAddr() ) {
	    ptrAddrAddr = ptrAddrIA->getAddr( ptrOptAddr->getAddr() );
	    if (!ptrAddrAddr) {
		// there is no such addr in db
		/// @todo: what to do with new addrs?
		// (Thomson:I think we should RELEASE all addrs, and issue new REQUEST for this IA)
	    } else {
		if ( (ptrOptAddr->getPref() == 0) || (ptrOptAddr->getValid() == 0) ) {
		    releaseIA( ptrOptIA->getIAID() );
		    break; // analyze next option OPTION_IA_NA
		}
		// set up new options
		if (ptrOptAddr->getPref() != ptrAddrAddr->getPref() )
		{
		    // received diffrent prefered-lifetime
		    /// @todo:
		}
		if (ptrOptAddr->getValid() != ptrAddrAddr->getValid() )
		{
		    // received diffrent prefered-lifetime
		    /// @todo:
		}
		ptrAddrAddr->setTimestamp();
	    }
	}
	ptrAddrIA->setT1( ptrOptIA->getT1() );
	ptrAddrIA->setT2( ptrOptIA->getT2() );
	ptrAddrIA->setTimestamp();
	ptrAddrIA->setState(STATE_CONFIGURED);
    } else {
	// unknown IAID, ignore it
	Log(Warning) << "Received message contains unknown IA (IAID="
	     << ptrOptIA->getIAID() << "). Ignoring it." << LogEnd;
    }
}

void TClntMsgRebind::doDuties()
{
    SPtr<TIfaceIface> iface = ClntIfaceMgr().getIfaceByID(this->Iface);

    if (!MRD)
    {
        ostringstream iaLst;
        ostringstream pdLst;
        SPtr<TOpt> ptrOpt;

        memset(&dhcp6cMsgFd, 0 , sizeof(CMSG_FD));
        memset(&dhcp6cMsg, 0 , sizeof(CMSG_BUFF));
        memset(&dhcp6cInfoMsgBody, 0, sizeof(DHCP6C_INFO_MSG_BODY));

        firstOption();
        while(ptrOpt=getOption())
        {
	        switch( ptrOpt->getOptType()) {
            case OPTION_IA_NA:
              {
                SPtr<TClntOptIA_NA> ptrIA=(Ptr*)ptrOpt;
                iaLst << ptrIA->getIAID() << " ";
                releaseIA(ptrIA->getIAID());
                break;
              }
            case OPTION_IA_PD:
              {
                SPtr<TClntOptIA_PD> ptrPD = (Ptr*)ptrOpt;
                ptrPD->setContext(SPtr<TDUID>(), SPtr<TIPv6Addr>(), this);
                ptrPD->delPrefixes();
                break;
              }
            };
        }

        Log(Debug) << "IA is expired notify cos to delete it" << LogEnd;
        dhcp6cMsg.type = CMSG_IPV6_DHCP6C_STATUS;
        strncpy(dhcp6cInfoMsgBody.intfName, iface->getName(), sizeof(dhcp6cInfoMsgBody.intfName));
        memcpy(dhcp6cMsg.content, &dhcp6cInfoMsgBody, sizeof(DHCP6C_INFO_MSG_BODY));
        msg_connCliAndSend(CMSG_ID_COS, &dhcp6cMsgFd, &dhcp6cMsg);

        Log(Warning) << "REBIND for the IA(s):" << iaLst.str()
                     << ", PD(s):" << pdLst.str()
                     << " failed on the " << iface->getFullName() 
                     << " interface." << LogEnd;
        Log(Warning) << "Restarting server discovery process." << LogEnd;
        
        IsDone=true;
    }
    else
        this->send();
}

void TClntMsgRebind::releaseIA(int IAID)
{
    SPtr<TAddrIA> ptrAddrIA=ClntAddrMgr().getIA(IAID);
    if (!ptrAddrIA)
    {
        Log(Error) << "IA has not been found in Address Manager."<< LogEnd;
        return;
    }

    SPtr<TAddrAddr> ptrAddr;
    ptrAddrIA->firstAddr();
    while(ptrAddr=ptrAddrIA->getAddr())
    {
        //remove outdated address from interface
        SPtr<TIfaceIface> ptrIface = ClntIfaceMgr().getIfaceByID(ptrAddrIA->getIfindex());
        if (!ptrIface) {
            Log(Error) << "Unable to find interface with ifindex " << ptrAddrIA->getIfindex() << LogEnd;
            continue;
        }

        memcpy(&dhcp6cInfoMsgBody.ip.addr, ptrAddr->get()->getAddr(), sizeof(struct in6_addr));
        dhcp6cInfoMsgBody.ip.plen = ptrIface->getPrefixLength();
        dhcp6cInfoMsgBody.ip.mode = IPV6_INFO_DEL;
        dhcp6cInfoMsgBody.status |= DHCP6C_ASSIGNED_ADDR;

        ptrIface->delAddr(ptrAddr->get(), ptrIface->getPrefixLength());
        //and from db
        ptrAddrIA->delAddr(ptrAddr->get());
    }
    ptrAddrIA->setState(STATE_NOTCONFIGURED);

    SPtr<TClntCfgIA> cfgIA = ClntCfgMgr().getIA(IAID);
    if (!cfgIA)
	return;
    cfgIA->setState(STATE_NOTCONFIGURED);
}

bool TClntMsgRebind::check() {
	return 0;
}

std::string TClntMsgRebind::getName() const {
    return "REBIND";
}

TClntMsgRebind::~TClntMsgRebind() {
}
