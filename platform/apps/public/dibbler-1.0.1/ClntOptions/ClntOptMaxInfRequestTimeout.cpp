/*  Copyright(c) 2009-2015 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 * file		ClntOptMaxInfRequestTimeout.cpp
 * brief	add INF_MAX_RT option support for dibbler-client	
 * details	
 *
 * author	Li Weijie
 * version	
 * date		25Nov15
 *
 * history 	\arg	1.0.0,Li Weijie, create file
 */
#include "DHCPConst.h"
#include "ClntOptMaxInfRequestTimeout.h"
#include "OptDUID.h"
#include "ClntMsg.h"
#include "Logger.h"

using namespace std;

TClntOptMaxInfRequestTimeout::TClntOptMaxInfRequestTimeout(char * buf,  int n, TMsg* parent)
	:TOptInteger(OPTION_INF_MAX_RT, OPTION_INF_MAX_RT_LEN, buf,n, parent){

}

TClntOptMaxInfRequestTimeout::TClntOptMaxInfRequestTimeout( char pref, TMsg* parent)
	:TOptInteger(OPTION_INF_MAX_RT, OPTION_INF_MAX_RT_LEN, pref, parent) {
}

bool TClntOptMaxInfRequestTimeout::doDuties()
{
    string reason = "trying to set Infomation Request Max Retransmission Time.";

    if (!Parent) {
        Log(Error) << "Unable to set INFO_MAX_RT: option parent not set." << LogEnd;
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

    return iface->setMaxInfRequestTimeout(duid->getDUID(), Parent->getRemoteAddr(), Value);
}
