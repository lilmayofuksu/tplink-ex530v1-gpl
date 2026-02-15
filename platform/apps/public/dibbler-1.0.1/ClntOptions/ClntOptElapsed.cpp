/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 *
 * released under GNU GPL v2 only licence
 *
 * $Id: ClntOptElapsed.cpp,v 1.8 2008-08-29 00:07:28 thomson Exp $
 *
 */

#include "Portable.h"
#include "DHCPConst.h"
#include "ClntOptElapsed.h"
#include "Logger.h"

TClntOptElapsed::TClntOptElapsed( char * buf,  int n, TMsg* parent)
    :TOptInteger(OPTION_ELAPSED_TIME, OPTION_ELAPSED_TIME_LEN, buf,n, parent)
{
    Timestamp = (uint32_t)time(NULL);
	gettimeofday(&uTimestamp, NULL);
}

TClntOptElapsed::TClntOptElapsed(TMsg* parent)
    :TOptInteger(OPTION_ELAPSED_TIME, OPTION_ELAPSED_TIME_LEN, 0, parent)
{
    Timestamp = (uint32_t)time(NULL);
	gettimeofday(&uTimestamp, NULL);
}

bool TClntOptElapsed::doDuties()
{
    return false;
}

char * TClntOptElapsed::storeSelf(char* buf)
{
    //Value = (unsigned int)((uint32_t)time(NULL) - Timestamp)*100;	
    struct timeval currtime, difftime;
	gettimeofday(&currtime, NULL);
	timersub(&currtime, &uTimestamp, &difftime);

	Value = (difftime.tv_sec * 100 + (difftime.tv_usec / 10000));
	
	Log(Debug) << "ElapedTime storeSelf MaxElapedTimeout :" << Value << LogEnd;
	/*modify by lwj, if Value is near MAX ElapedTime, we Set it to 0xFFFF */
	if (Value >= 65535) { /* 0xFFFF/100 ~= 655*/
		/* if ElapedTime is bigger rather than 65535, we set it to 0xffff */
		Value = 0xFFFF;	/* MAX ElapedTime */
		Log(Debug) << "Set ElapedTime Value to MAX Value 0xFFFF" << LogEnd;
	}
    return TOptInteger::storeSelf(buf);
}
