/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 *
 * released under GNU GPL v2 only licence
 *
 */

#include "SrvMsgInfRequest.h"
#include "SmartPtr.h"
#include "DHCPConst.h"
#include "SrvMsgAdvertise.h"
#include "OptDUID.h"
#include "SrvOptIA_NA.h"

TSrvMsgInfRequest::TSrvMsgInfRequest(int iface,  SPtr<TIPv6Addr> addr, char* buf, int bufSize)
    :TSrvMsg(iface, addr, buf, bufSize) {
}

void TSrvMsgInfRequest::doDuties() {
    return;
}

bool TSrvMsgInfRequest::check() {
	/* add by lwj, check Infomation request valid, ServerID && IA_NA is not allowed */
	SPtr<TOptDUID> optSrvID = (Ptr*) this->getOption(OPTION_SERVERID);
    if (optSrvID) {
        return false;
    }

	SPtr<TSrvOptIA_NA> optIA_NA = (Ptr*) this->getOption(OPTION_IA_NA);
	if (optIA_NA) {
		return false;
	}
	
    return true;
}

unsigned long TSrvMsgInfRequest::getTimeout() {
    return 0;
}

std::string TSrvMsgInfRequest::getName() const {
    return "INF-REQUEST";
}


TSrvMsgInfRequest::~TSrvMsgInfRequest(){
}
