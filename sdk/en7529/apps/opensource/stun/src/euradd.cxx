/**********************************************************************************

	STUN in TR069 For Easycwmp
	Author: EmbedUR

	
*************************************************************************************/
#include <cassert>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <sys/select.h>

#include "udp.h"
#include "euradd.h"

#define EUR_DEFAULT_KEEPALIVE_INTERVAL 30
using namespace std;

int RAflag =0;
extern int BCFlag;
/*To send binding req and receive resp periodically*/
static void periodicBindReqResp(Socket Fd, StunAddress4& stunServerAddr,
              const StunAtrString& username, const StunAtrString& password, bool verbose){
	
	stunSendTest( Fd, stunServerAddr, username, password, 1, verbose );
}
static void periodicListen(Socket Fd, StunAddress4& stunServerAddr,
              const StunAtrString& username, const StunAtrString& password, bool verbose, int interval)
{
   	struct timeval tv;
   	fd_set fdSet;
	FD_ZERO(&fdSet);
   	int fdSetSize=0,err=0,e=0;
	tv.tv_sec=interval;
        tv.tv_usec=0;
        char msg[EUR_UDP_STUN_MAX_MESSAGE_SIZE];
        int msgLen = EUR_UDP_STUN_MAX_MESSAGE_SIZE;
	while(1){
		FD_ZERO(&fdSet); fdSetSize=0;
		FD_SET(Fd,&fdSet); fdSetSize = (Fd+1>fdSetSize) ? Fd+1 : fdSetSize;
		if (tv.tv_sec == 0){
			clog << "tv sec has become 0 resetting" << endl;
			tv.tv_sec=interval;
			tv.tv_usec=0;
		}
		err = select(fdSetSize, &fdSet, NULL, NULL, &tv);
		e = getErrno();
		if ( err == SOCKET_ERROR )
		{
        		// error occured
        		cerr << "Error " << e << " " << strerror(e) << " in select" << endl;
		}
		else if ( err == 0 )
		{
        		if (verbose) clog << "Select fell through, going for periodic call periodic call" << endl;
        		periodicBindReqResp( Fd, stunServerAddr, username, password, verbose );
			if (BCFlag) BCFlag=0;
		}else{
			clog << "Time for resp" << tv.tv_sec << endl;
        		StunAddress4 from;
			bool ok;

			ok = getMessage( Fd,msg,&msgLen,&from.addr,&from.port,verbose );
    			if ( ok ) {
            			StunMessage resp;
            			memset(&resp, 0, sizeof(StunMessage));

            			if (verbose){ 
					clog << "Got a response" << endl;
            				clog << "Message before parse" << msg << "Message Len" << msgLen << endl;
				}

            			stunParseMessage( msg,msgLen, resp,verbose );

            			if ( verbose )
            			{
                    			clog << "\t ok=" << ok << endl;
                    			clog << "\t id=" << resp.msgHdr.id << endl;
                    			clog << "\t mappedAddr=" << resp.mappedAddress.ipv4 << endl;
                    			clog << "\t changedAddr=" << resp.changedAddress.ipv4 << endl;
                    			clog << endl;
            			}
    			}
			memset(msg,'0',EUR_UDP_STUN_MAX_MESSAGE_SIZE);
			msgLen = EUR_UDP_STUN_MAX_MESSAGE_SIZE;
		}
  	}      
}

/*To maintain the binding we shoul send binding req/resp based on the configured min and max keep alives*/
void stun_client_maintainbind(StunAddress4 sAddr,StunAddress4& stunServerAddr, int minKeepAlive, int maxKeepAlive,bool verbose, Socket myFd1, Socket myFd2){

   UInt32 interfaceIp = sAddr.addr;
   int port = sAddr.port;

/*Username and password is set to pass to the API*/
   StunAtrString username;
   StunAtrString password;
   username.sizeValue = 0;
   password.sizeValue = 0;
  
   clog << "saddr is" << sAddr.port << sAddr.addr << endl << "serveraddr is" << stunServerAddr.addr << stunServerAddr.port << endl;
   if ( ( myFd1 == INVALID_SOCKET) || ( myFd2 == INVALID_SOCKET) )
   {
       cerr << "Some problem opening port/interface to send on" << endl;
   }

   assert( myFd1 != INVALID_SOCKET );
   assert( myFd2 != INVALID_SOCKET );
   /*Checking for invalid values configured for keepalive and if values less than 30 is configured, value will be set to 30 to reduce CPU usage*/
   if ( ((minKeepAlive <= 0 ) && (maxKeepAlive <= 0)) || (minKeepAlive > maxKeepAlive) || (maxKeepAlive <=30) || ((minKeepAlive == maxKeepAlive) && (maxKeepAlive <=30))  )
   {
	periodicListen(myFd1, stunServerAddr, username, password, verbose, EUR_DEFAULT_KEEPALIVE_INTERVAL);
   
   }else if (minKeepAlive == maxKeepAlive){
	periodicListen(myFd1, stunServerAddr, username, password, verbose, maxKeepAlive);
   }
   else{
   /*If both min and max are different discover binding has timed out before the max, through binary search
	and if binding has not changed set max as the periodic interval to send binding resp/req periodically*/
                int i =0;
                bool sec_res =false;
		int periodic_interval = 0;
		char msg[EUR_UDP_STUN_MAX_MESSAGE_SIZE];
                int msgLen = EUR_UDP_STUN_MAX_MESSAGE_SIZE;

                for(i=minKeepAlive;i<=maxKeepAlive;i++){
                        RAflag = 1; //sending from sec port set response address flag to 1
                        stunSendTest( myFd2, stunServerAddr, username, password, 1, verbose );


                        StunAddress4 from;
                        sec_res=getMessage( myFd1,
                                msg,
                                &msgLen,
                                &from.addr,
                                &from.port,verbose );
                        if (!sec_res){
                                clog << "Error received msg in Response Address" << endl;
                                periodic_interval = i;
                                RAflag = 0;
				BCFlag = 1;
                                break;
                        }
                        StunMessage resp;
                        memset(&resp, 0, sizeof(StunMessage));

                        if ( verbose ) clog << "Got a response" << endl;
 			bool ok = stunParseMessage( msg,msgLen, resp,verbose );
			memset(msg,'0',EUR_UDP_STUN_MAX_MESSAGE_SIZE);
                        msgLen = EUR_UDP_STUN_MAX_MESSAGE_SIZE;
                        if ( verbose )
                        {
                                clog << "\t ok=" << ok << endl;
                                clog << "\t id=" << resp.msgHdr.id << endl;
                                clog << "\t mappedAddr=" << resp.mappedAddress.ipv4 << endl;
                                clog << "\t changedAddr=" << resp.changedAddress.ipv4 << endl;
                                clog << endl;
                        }

                }
	        if (RAflag == 1){
			 clog << "Binding has not changed till max keep alive time , so setting interval as maxKeepAlive" << endl;
			 periodic_interval = maxKeepAlive;
                         RAflag = 0;
		}
		clog << "Periodic sent started" << endl ;
	        periodicListen( myFd1, stunServerAddr, username, password, verbose ,periodic_interval);
   }
   closesocket(myFd1);
   closesocket(myFd2);
}
/* ====================================================================
 * The Vovida Software License, Version 1.0
 *
 * Copyright (c) 2000 Vovida Networks, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The names "VOCAL", "Vovida Open Communication Application Library",
 *    and "Vovida Open Communication Application Library (VOCAL)" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact vocal@vovida.org.
 *
 * 4. Products derived from this software may not be called "VOCAL", nor
 *    may "VOCAL" appear in their name, without prior written
 *    permission of Vovida Networks, Inc.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL VOVIDA
 * NETWORKS, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT DAMAGES
 * IN EXCESS OF $1,000, NOR FOR ANY INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * ====================================================================
 *
 * This software consists of voluntary contributions made by Vovida
 * Networks, Inc. and many individuals on behalf of Vovida Networks,
 * Inc.  For more information on Vovida Networks, Inc., please see
 * <http://www.vovida.org/>.
 *
 */

// Local Variables:
// mode:c++
// c-file-style:"ellemtel"
// c-file-offsets:((case-label . +))
// indent-tabs-mode:nil
// End:

