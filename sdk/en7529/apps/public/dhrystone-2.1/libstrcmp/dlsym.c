#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

typedef int(*STRCMP)(const char*, const char*);

int strcmp(const char*, const char*)
{
	static void *handle = NULL;
	static STRCMP old_strcmp = NULL;

	if(!handle)
	{
		handle = dlopen("libc.so.0", RTLD_LAZY)
		old_strcmp = (STRCMP)dlsym(handle, "strcmp");
	}
	printf("oops!!! back function invoked. s1=<%s> s2=<%s>\n", s1, s2);
	return old_strcmp(s1, s2);
}