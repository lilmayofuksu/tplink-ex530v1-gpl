/*
 *  SHA256 file hash program
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

#define DBG_PRINT 0

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_fprintf    fprintf
#define mbedtls_printf     printf
#endif

#include "mbedtls/aes.h"
#include "mbedtls/md.h"
#include "mbedtls/rsa.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#if !defined(_WIN32_WCE)
#include <io.h>
#endif
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#define IV_HASH_TIMES (32)

#define USAGE   \
    "\n  sha256 <input filename> <output filename>\n" \
    "\n"

#if !defined(MBEDTLS_SHA256_C) || \
    !defined(MBEDTLS_FS_IO) || !defined(MBEDTLS_MD_C)
int main( void )
{
    mbedtls_printf("MBEDTLS_AES_C and/or MBEDTLS_SHA256_C "
                    "and/or MBEDTLS_FS_IO and/or MBEDTLS_MD_C "
                    "not defined.\n");
    return( 0 );
}
#else
int main( int argc, char *argv[] )
{
    int ret = 1;

    unsigned int n;
    FILE *fin = NULL, *fout = NULL;

    mbedtls_md_context_t sha_ctx;
	unsigned char digest[32];
	unsigned char buffer[1024];

#if defined(_WIN32_WCE)
	long filesize, offset;
#elif defined(_WIN32)
	LARGE_INTEGER li_size;
	__int64 filesize, offset;
#else
	off_t filesize, offset;
#endif

#if	DBG_PRINT
	unsigned int dbg_p_idx;
#endif

    mbedtls_md_init( &sha_ctx );

    ret = mbedtls_md_setup( &sha_ctx, mbedtls_md_info_from_type( MBEDTLS_MD_SHA256 ), 1 );
    if( ret != 0 )
    {
        mbedtls_printf( "  ! mbedtls_md_setup() returned -0x%04x\n", -ret );
        goto exit;
    }

    /*
     * Parse the command-line arguments.
     */
    if( argc != 3)
    {
        mbedtls_printf( USAGE );

#if defined(_WIN32)
        mbedtls_printf( "\n  Press Enter to exit this program.\n" );
        fflush( stdout ); getchar();
#endif

        goto exit;
    }

    if( strcmp( argv[1], argv[2] ) == 0 )
    {
        mbedtls_fprintf( stderr, "input and output filenames must differ\n" );
        goto exit;
    }

    if( ( fin = fopen( argv[1], "rb" ) ) == NULL )
    {
        mbedtls_fprintf( stderr, "fopen(%s,rb) failed\n", argv[2] );
        goto exit;
    }

    if( ( fout = fopen( argv[2], "wb+" ) ) == NULL )
    {
        mbedtls_fprintf( stderr, "fopen(%s,wb+) failed\n", argv[3] );
        goto exit;
    }

	if( ( filesize = lseek( fileno( fin ), 0, SEEK_END ) ) < 0 )
    {
        perror( "lseek" );
        goto exit;
    }

	if( fseek( fin, 0, SEEK_SET ) < 0 )
    {
        mbedtls_fprintf( stderr, "fseek(0,SEEK_SET) failed\n" );
        goto exit;
    }

    mbedtls_md_starts( &sha_ctx );
	for( offset = 0; offset < filesize; offset += sizeof(buffer) )
    {
        n = ( filesize - offset > sizeof(buffer) ) ? sizeof(buffer) : ( filesize - offset );

		if( fread( buffer, 1, n, fin ) != (size_t) n )
        {
            mbedtls_fprintf( stderr, "fread(%d bytes) failed\n", n );
            goto exit;
        }
		mbedtls_md_update( &sha_ctx, buffer, n );
	}
    mbedtls_md_finish( &sha_ctx, digest );

	if( fwrite( digest, 1, 32, fout ) != 32 )
    {
        mbedtls_fprintf( stderr, "fwrite(%d bytes) failed\n", 16 );
        goto exit;
    }

    ret = 0;

exit:
    if( fin )
        fclose( fin );
    if( fout )
        fclose( fout );

    memset( buffer, 0, sizeof( buffer ) );
    memset( digest, 0, sizeof( digest ) );

    mbedtls_md_free( &sha_ctx );

    return( ret );
}
#endif /* MBEDTLS_AES_C && MBEDTLS_SHA256_C && MBEDTLS_FS_IO */
