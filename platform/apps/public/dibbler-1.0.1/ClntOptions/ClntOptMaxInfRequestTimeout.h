/*  Copyright(c) 2009-2015 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 * file		ClntOptMaxInfRequestTimeout.h
 * brief	add INF_MAX_RT option supported, RF7083	
 * details	
 *
 * author	Li Weijie
 * version	
 * date		25Nov15
 *
 * history 	\arg	1.0.0, Li Weijie, create file
 */

#ifndef CLNTOPTMAXINFREQUESTTIMEOUT_H
#define CLNTOPTMAXINFREQUESTTIMEOUT_H

#include "DHCPConst.h"
#include "OptInteger.h"

class TClntOptMaxInfRequestTimeout : public TOptInteger
{
 public:
    TClntOptMaxInfRequestTimeout(char * buf,  int n, TMsg* parent);
    
    TClntOptMaxInfRequestTimeout(char pref, TMsg* parent);
    bool doDuties();
};

#endif
