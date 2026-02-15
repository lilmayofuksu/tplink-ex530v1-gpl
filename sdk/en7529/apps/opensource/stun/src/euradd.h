
#include "stun.h"

/*API to monitor and maintain the binding*/
void stun_client_maintainbind(StunAddress4 sAddr,StunAddress4& stunServerAddr, int minKeepAlive, int maxKeepAlive, bool verbose, Socket myFd1,Socket myFd2);

