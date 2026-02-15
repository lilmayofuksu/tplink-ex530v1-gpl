#include <string.h>
#include "include/ebtables_u.h"

static struct ebt_u_replace replace;
void ebt_early_init_once();

#ifdef INCLUDE_BCM_NETFILTER
extern void get_global_mutex();
extern void release_global_mutex();
#endif

int main(int argc, char *argv[])
{
#ifdef INCLUDE_BCM_NETFILTER
	get_global_mutex();
#endif

	ebt_silent = 0;
	ebt_early_init_once();
	strcpy(replace.name, "filter");
	do_command(argc, argv, EXEC_STYLE_PRG, &replace);

#ifdef INCLUDE_BCM_NETFILTER
	release_global_mutex();
#endif

	return 0;
}
