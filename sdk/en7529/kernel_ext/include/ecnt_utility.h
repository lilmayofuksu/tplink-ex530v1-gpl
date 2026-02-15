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
 *  global_inc/lib/ecnt_utility.h
 */


#ifndef _ECNT_UTILITY_H_
#define _ECNT_UTILITY_H_

/**
 * memcpy4 - Copy one area of memory to another
 * @dest: Where to copy to
 * @src: Where to copy from
 * @count: The size of the area.
 *
 * You should not use this function to access IO space, use memcpy_toio()
 * or memcpy_fromio() instead.
 */
extern void *memcpy4(void *dest, const void *src, size_t count);

#endif // #ifndef _ECNT_UTILITY_H_

