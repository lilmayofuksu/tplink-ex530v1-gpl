/*  Copyright(c) 2009-2015 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 * file		ClntOptMaxSolicitTimeout.cpp
 * brief	add SOL_MAX_RT option support	
 * details	
 *
 * author	Li Weijie
 * version	
 * date		25Nov15
 *
 * history 	\arg	1.0.0,Li Weijie, create file
 */
#include "DHCPConst.h"
#include "ClntOptMaxSolicitTimeout.h"
#include "OptDUID.h"
#include "ClntMsg.h"
#include "Logger.h"

using namespace std;

TClntOptMaxSolicitTimeout::TClntOptMaxSolicitTimeout(char * buf,  int n, TMsg* parent)
	:TOptInteger(OPTION_SOL_MAX_RT, OPTION_SOL_MAX_RT_LEN, buf,n, parent){

}

TClntOptMaxSolicitTimeout::TClntOptMaxSolicitTimeout( char pref, TMsg* parent)
	:TOptInteger(OPTION_SOL_MAX_RT, OPTION_SOL_MAX_RT_LEN, pref, parent) {
}

bool TClntOptMaxSolicitTimeout::doDuties()
{
    string reason = "trying to set Solicit Max Retransmission time.";

    if (!Parent) {
        Log(Error) << "Unable to set SOL_MAX_RT: option parent not set." << LogEnd;
        return false;
    }

    int ifindex = Parent->getIface();

    SPtr<TOptDUID> duid = (Ptr*)Parent->getOption(OPTION_SERVERID);

    if (!duid) {
	Log(Error) << "Unable to find proper DUID while " << reason << LogEnd;
	return false;
    }

    SPtr<TClntIfaceIface> iface = (Ptr*)ClntIfaceMgr().getIfaceByID(ifindex);
    if (!iface) {
        Log(Error) << "Unable to find interface ifindex=" << ifindex
            << reason << LogEnd;
        return false;
    }

    return iface->setMaxSolicitTimeout(duid->getDUID(), Parent->getRemoteAddr(), Value);
}
