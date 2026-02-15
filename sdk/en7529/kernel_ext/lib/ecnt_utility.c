/***************************************************************************************
 *      Copyright(c) 2014 ECONET Incorporation All rights reserved.
 *
 *      This is unpublished proprietary source code of ECONET Networks Incorporation
 *
 *      The copyright notice above does not evidence any actual or intended
 *      publication of such source code.
 ***************************************************************************************
 */

/*
 *  kernel_ext/lib/ecnt_utility.c
 */

#include <linux/types.h>
#include <linux/module.h>

extern void *memcpy(void *dest, const void *src, size_t count);

/**
 * memcpy4 - Copy one area of memory to another
 * @dest: Where to copy to
 * @src: Where to copy from
 * @count: The size of the area.
 *
 * You should not use this function to access IO space, use memcpy_toio()
 * or memcpy_fromio() instead.
 */
void *memcpy4(void *dest, const void *src, size_t count)
{
	unsigned int *d4 = (unsigned int *)dest;
	unsigned int *s4 = (unsigned int *)src;

	if((((unsigned int)s4 & 0x3) != 0) ||
	   (((unsigned int)d4 & 0x3) != 0)) {
		return memcpy(dest, src, count);
	}

	while(count > 0) {
		if(count >> 2) {
			*d4++ = *s4++;
			count -= 4;
		} else {
			memcpy(d4, s4, count);
			count = 0;
		}
	}

	return dest;
}
EXPORT_SYMBOL(memcpy4);

/* End of [ecnt_utility.c] */
