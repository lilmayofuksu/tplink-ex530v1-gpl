#ifndef DROPBEAR_OPTIONS_H
#ifndef PSW_PRIV_FILENAME
#define PSW_PRIV_FILENAME "/var/tmp/dropbear/dropbearpwd"
#endif
#define DROPBEAR_OPTIONS_H
#define DROPBEAR_PWD 1

/* 
            > > > Don't edit this file any more! < < <
            
Local compile-time configuration should be defined in localoptions.h
in the build directory.
See default_options.h.in for a description of the available options.
*/

/* Some configuration options or checks depend on system config */
#include "config.h"

#ifdef LOCALOPTIONS_H_EXISTS
#include "localoptions.h"
#endif

#ifdef INCLUDE_CLS_L1_IMDA_TPAPP
 #define DROPBEAR_FAIL_COUNT "/tmp/dropbear/fail_count"
 #define DROPBEAR_FILE "/tmp/dropbear"
 #define DROPBEAR_ARP_TABLE  "/proc/net/arp"
#endif /* INCLUDE_CLS_L1_IMDA_TPAPP */

/* default_options.h is processed to add #ifndef guards */
#include "default_options_guard.h"

#define USER_ACCOUNT "dropbear"
/* Some other defines that mostly should be left alone are defined
 * in sysoptions.h */
#include "sysoptions.h"

#endif /* DROPBEAR_OPTIONS_H */
