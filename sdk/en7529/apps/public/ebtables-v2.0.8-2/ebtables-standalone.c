#include <string.h>
#include "include/ebtables_u.h"
#include <errno.h>

static struct ebt_u_replace replace;
void ebt_early_init_once();

int main(int argc, char *argv[])
{
	int again_flag = 0;
	struct ebt_u_replace *handle = NULL;
	
	ebt_silent = 0;
	ebt_early_init_once();
	strcpy(replace.name, "filter");

	do_command(argc, argv, EXEC_STYLE_PRG, &replace);

retry:	
	if (errno == EAGAIN)
	{
		if(again_flag < 5)
		{
			usleep(50*1000);
			handle = NULL;
			do_command(argc, argv, EXEC_STYLE_PRG, handle);
			again_flag++;
			goto retry;
		}
	}
		
	return 0;
}
