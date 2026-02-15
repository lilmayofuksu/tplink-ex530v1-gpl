/*  Copyright(c) 2009-2015 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 * file		ClntOptMaxSolicitTimeout.h
 * brief 	add SOL_MAX_RT option to dibbler client 	
 * details	
 *
 * author	Li Weijie
 * version	
 * date		25Nov15
 *
 * history 	\arg	1.0.0 create file 	
 */

#ifndef CLNTOPTMAXSOLICITTIMEOUT_H
#define CLNTOPTMAXSOLICITTIMEOUT_H

#include "DHCPConst.h"
#include "OptInteger.h"

class TClntOptMaxSolicitTimeout : public TOptInteger
{
 public:
    TClntOptMaxSolicitTimeout(char * buf,  int n, TMsg* parent);
    
    TClntOptMaxSolicitTimeout(char pref, TMsg* parent);
    bool doDuties();
};

#endif
