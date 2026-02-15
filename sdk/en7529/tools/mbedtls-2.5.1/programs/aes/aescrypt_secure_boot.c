/*
 *  AES-256 file encryption program
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

enum {
	MODE_ENCRYPT_RSA_48BITS = 0,
	MODE_ENCRYPT_IMGKEY_RSA,
	MODE_ENCRYPT_NORMAL,
	MODE_ENCRYPT_RSA,
	MODE_DECRYPT_RSA_48BITS,
	MODE_DECRYPT_IMGKEY_RSA,
	MODE_DECRYPT_NORMAL,
	MODE_DECRYPT_RSA,
	MODE_PAD_RSA_PUBLIC_KEY,
	MODE_SHA256_RSA_PUBLIC_KEY,
	MODE_CRYPTO_MAX
};

#define IV_HASH_TIMES (32)

#define USAGE   \
    "\n  aescrypt2 <mode> <input filename> <output filename> <keyfile>\n" \
    "\n   <mode>: 0 = encrypt RSA public key with 48th to last bits with RSA public key first 48 bits\n" \
    "\n   <mode>: 1 = encrypt image AES key with RSA public key\n" \
    "\n   <mode>: 2 = encrypt\n" \
    "\n   <mode>: 3 = encrypt RSA public key with keyfile\n" \
    "\n   <mode>: 4 = decrypt RSA public key with 48th to last bits with RSA public key first 48 bits\n" \
    "\n   <mode>: 5 = decrypt image AES key with RSA public key\n" \
    "\n   <mode>: 6 = decrypt\n" \
    "\n   <mode>: 7 = decrypt RSA public key with keyfile\n" \
    "\n   <mode>: 8 = to pad RSA public key to input file\n" \
    "\n   <mode>: 9 = to HASH(SHA256) RSA public key to input file\n" \
    "\n  example: aescrypt2 0 rsa_pub.txt rsa_pub.txt.cihper rsa_pub.txt\n" \
    "\n  example: aescrypt2 1 aes.key aes.key.cipher rsa_pub.txt\n" \
    "\n  example: aescrypt2 2 file file.cipher aes.key\n" \
    "\n  example: aescrypt2 3 file.aes file rsa_pub.txt\n" \
    "\n  example: aescrypt2 5 file.aes file keyfile\n" \
    "\n  example: aescrypt2 8 file.in file.out rsa_pub.txt\n" \
    "\n  example: aescrypt2 9 rsa_pub.txt file.out\n" \
    "\n           need file efuse_data created.\n" \
    "\n"

#if !defined(MBEDTLS_AES_C) || !defined(MBEDTLS_SHA256_C) || \
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

    unsigned int i, n;
    int mode, lastn;
    size_t keylen = 512;
    FILE *fefuse = NULL, *fin = NULL, *fout = NULL, *fkey = NULL;
	mbedtls_rsa_context rsa, rsaKey;

    unsigned char IV[16];
	unsigned char *key = NULL;
	unsigned char rsa_pub_data[512];
    unsigned char digest[32];
    unsigned char buffer[1024];
    unsigned char diff;
	const char magic[] = "ECNT";
	const char efuse_file[] = "efuse_data.bin";
	const unsigned int efuse_len = 6; //bytes
	unsigned char tmp[16];

    mbedtls_aes_context aes_ctx;
    mbedtls_md_context_t sha_ctx;
	
#if	DBG_PRINT
	unsigned int dbg_p_idx;
#endif

#if defined(_WIN32_WCE)
    long filesize, offset;
#elif defined(_WIN32)
       LARGE_INTEGER li_size;
    __int64 filesize, offset;
#else
      off_t filesize, offset;
#endif

    mbedtls_aes_init( &aes_ctx );
    mbedtls_md_init( &sha_ctx );
	mbedtls_rsa_init( &rsa, MBEDTLS_RSA_PKCS_V15, 0 );
	mbedtls_rsa_init( &rsaKey, MBEDTLS_RSA_PKCS_V15, 0 );

    ret = mbedtls_md_setup( &sha_ctx, mbedtls_md_info_from_type( MBEDTLS_MD_SHA256 ), 1 );
    if( ret != 0 )
    {
        mbedtls_printf( "  ! mbedtls_md_setup() returned -0x%04x\n", -ret );
        goto exit;
    }

    /*
     * Parse the command-line arguments.
     */
    if( argc != 5 && argc != 4)
    {
        mbedtls_printf( USAGE );

#if defined(_WIN32)
        mbedtls_printf( "\n  Press Enter to exit this program.\n" );
        fflush( stdout ); getchar();
#endif
        goto err_exit;
    }

    mode = atoi( argv[1] );
    memset(IV, 0, sizeof(IV));
	memset(rsa_pub_data, 0, sizeof(rsa_pub_data));
    memset(digest, 0, sizeof(digest));
    memset(buffer, 0, sizeof(buffer));

    if( mode > MODE_CRYPTO_MAX || mode < 0 )
    {
        mbedtls_fprintf( stderr, "invalide operation mode\n" );
        goto err_exit;
    }

    if( strcmp( argv[2], argv[3] ) == 0 )
    {
        mbedtls_fprintf( stderr, "input and output filenames must differ\n" );
        goto err_exit;
    }

    if( ( fin = fopen( argv[2], "rb" ) ) == NULL )
    {
        mbedtls_fprintf( stderr, "fopen(%s,rb) failed\n", argv[2] );
        goto err_exit;
    }

    if( ( fout = fopen( argv[3], "wb+" ) ) == NULL )
    {
        mbedtls_fprintf( stderr, "fopen(%s,wb+) failed\n", argv[3] );
        goto err_exit;
    }

	if(mode != MODE_SHA256_RSA_PUBLIC_KEY) {
		if( ( fkey = fopen( argv[4], "rb" ) ) == NULL )
	    {
	        mbedtls_fprintf( stderr, "fopen(%s,rb) failed\n", argv[4] );
	        goto err_exit;
	    }
	}

	if(mode == MODE_PAD_RSA_PUBLIC_KEY) {
		if( ( filesize = lseek( fileno( fin ), 0, SEEK_END ) ) < 0 )
	    {
	        perror( "lseek" );
	        goto err_exit;
	    }

		/* SHA256 hash value must be 32 bytes */
		if(filesize != 32) {
			mbedtls_fprintf( stderr, "SHA256 hash value size:%ld is not 32 bytes\n", filesize);
	        goto err_exit;
		}

		if( fseek( fin, 0, SEEK_SET ) < 0 )
	    {
	        mbedtls_fprintf( stderr, "fseek(0,SEEK_SET) fkey failed\n" );
	        goto err_exit;
	    }

		if( fread( buffer, 1, filesize, fin ) != filesize )
        {
            mbedtls_fprintf( stderr, "fread(%ld bytes) failed\n", filesize );
            goto err_exit;
        }

		if( fwrite( buffer, 1, filesize, fout ) != filesize )
        {
            mbedtls_fprintf( stderr, "fwrite(%ld bytes) failed\n", filesize );
            goto err_exit;
        }
		
		if( ( ret = mbedtls_mpi_read_file( &rsaKey.N, 16, fkey ) ) != 0 ||
			( ret = mbedtls_mpi_read_file( &rsaKey.E, 16, fkey ) ) != 0 )
		{
			mbedtls_fprintf( stderr, "mbedtls_mpi_read_file(%s,rb) failed\n", argv[4] );
			goto exit;
		}

		for(i = 0; i < (64 - (filesize / 4)); i++) {
			if( fwrite( &(rsaKey.N.p[i]), 1, 4, fout ) != 4 )
	        {
	            mbedtls_fprintf( stderr, "fwrite(%d bytes) failed\n", 4 );
	            goto err_exit;
	        }
	    }

		ret = 0;
		goto exit;
	}

	if(mode == MODE_SHA256_RSA_PUBLIC_KEY) {
		if( fseek( fin, 0, SEEK_SET ) < 0 )
	    {
	        mbedtls_fprintf( stderr, "fseek(0,SEEK_SET) fkey failed\n" );
	        goto err_exit;
	    }

		if( ( ret = mbedtls_mpi_read_file( &rsaKey.N, 16, fin ) ) != 0 ||
			( ret = mbedtls_mpi_read_file( &rsaKey.E, 16, fin ) ) != 0 )
		{
			mbedtls_fprintf( stderr, "mbedtls_mpi_read_file(%s,rb) failed\n", argv[2] );
			goto exit;
		}

		rsaKey.len = ( mbedtls_mpi_bitlen( &rsaKey.N ) + 7 ) >> 3;

		mbedtls_md_starts( &sha_ctx );
        mbedtls_md_update( &sha_ctx, rsaKey.N.p, rsaKey.len );
        mbedtls_md_finish( &sha_ctx, digest );

		if( fwrite( digest, 1, 32, fout ) != 32 )
        {
            mbedtls_fprintf( stderr, "fwrite(%u bytes) failed\n", 32 );
            goto err_exit;
        }

		ret = 0;
		goto exit;
	}
	
	/* read rsa public key file as AES key */
	if((mode == MODE_ENCRYPT_RSA_48BITS) || (mode == MODE_ENCRYPT_IMGKEY_RSA) ||
	   (mode == MODE_DECRYPT_RSA_48BITS) || (mode == MODE_DECRYPT_IMGKEY_RSA)) {
		if( ( ret = mbedtls_mpi_read_file( &rsaKey.N, 16, fkey ) ) != 0 ||
			( ret = mbedtls_mpi_read_file( &rsaKey.E, 16, fkey ) ) != 0 )
		{
			mbedtls_fprintf( stderr, "mbedtls_mpi_read_file(%s,rb) failed\n", argv[4] );
			goto exit;
		}

		rsaKey.len = ( mbedtls_mpi_bitlen( &rsaKey.N ) + 7 ) >> 3;

		if((mode == MODE_ENCRYPT_RSA_48BITS) || (mode == MODE_DECRYPT_RSA_48BITS)) {
			keylen = efuse_len;
		} else {
			keylen = 512;
		}

		key = (unsigned char *)mbedtls_calloc(keylen, sizeof(unsigned char));
		if(key == NULL) {
			mbedtls_fprintf( stderr, "mbedtls_calloc key failed\n");
			goto err_exit;
		}

		if((mode == MODE_ENCRYPT_RSA_48BITS) || (mode == MODE_DECRYPT_RSA_48BITS)) {
			memcpy(key, rsaKey.N.p, keylen);
		} else {
			if(rsaKey.len > keylen) {
				mbedtls_fprintf( stderr, "RSA public key size:%lu more than %lu.\n", rsaKey.len, keylen);
				goto err_exit;
			}
			memcpy(key, rsaKey.N.p, rsaKey.len);
		}
		
#if DBG_PRINT
		mbedtls_printf("rsaKey.len:%d\n", rsaKey.len);
		mbedtls_printf("rsaKey.N.s:%d\n", rsaKey.N.s);
		mbedtls_printf("rsaKey.N.n:%d\n", rsaKey.N.n);
		mbedtls_printf("rsaKey public key:\n");
		for(dbg_p_idx = 0; dbg_p_idx < keylen; dbg_p_idx++) {
			mbedtls_printf("0x%x ", key[dbg_p_idx]);
			if(((dbg_p_idx + 1) % 8) == 0) {
				mbedtls_printf("\n");
			}
		}
#endif
	} else {
		if( ( filesize = lseek( fileno( fkey ), 0, SEEK_END ) ) < 0 )
	    {
	        perror( "lseek" );
	        goto err_exit;
	    }

		if( fseek( fkey, 0, SEEK_SET ) < 0 )
	    {
	        mbedtls_fprintf( stderr, "fseek(0,SEEK_SET) fkey failed\n" );
	        goto err_exit;
	    }

		key = (unsigned char *)mbedtls_calloc(filesize, sizeof(unsigned char));
		if(key == NULL) {
			mbedtls_fprintf( stderr, "mbedtls_calloc key failed\n");
			goto err_exit;
		}
		
		keylen = fread( key, 1, filesize, fkey );
		if( keylen != filesize) {
			perror( "fread key error.\n" );
	        goto err_exit;
		}
#if DBG_PRINT
		mbedtls_printf("key:\n");
		for(dbg_p_idx = 0; dbg_p_idx < keylen; dbg_p_idx++) {
			mbedtls_printf("0x%x ", key[dbg_p_idx]);
			if(((dbg_p_idx + 1) % 8) == 0) {
				mbedtls_printf("\n");
			}
		}
		for(dbg_p_idx = 0; dbg_p_idx < (keylen / 4); dbg_p_idx++) {
			mbedtls_printf("0x%x ", *(((unsigned int *)key) + dbg_p_idx));
			if(((dbg_p_idx + 1) % 4) == 0) {
				mbedtls_printf("\n");
			}
		}
#endif
	}

    if( (mode == MODE_ENCRYPT_RSA_48BITS) || (mode == MODE_ENCRYPT_IMGKEY_RSA) || 
		(mode == MODE_ENCRYPT_NORMAL) || (mode == MODE_ENCRYPT_RSA))
    {
    	if((mode == MODE_ENCRYPT_RSA_48BITS) || (mode == MODE_ENCRYPT_RSA)) {
			if( ( ret = mbedtls_mpi_read_file( &rsa.N, 16, fin ) ) != 0 ||
				( ret = mbedtls_mpi_read_file( &rsa.E, 16, fin ) ) != 0 )
			{
				mbedtls_fprintf( stderr, "mbedtls_mpi_read_file(%s,rb) failed\n", argv[2] );
				goto exit;
			}

			rsa.len = ( mbedtls_mpi_bitlen( &rsa.N ) + 7 ) >> 3;
			filesize = sizeof(rsa_pub_data);
			if(rsa.len > filesize) {
				mbedtls_fprintf( stderr, "RSA public key size:%lu more than %ld.\n", rsa.len, filesize);
				goto err_exit;
			}
			memcpy(rsa_pub_data, rsa.N.p, rsa.len);

			/* clear 48bits */
			if(mode == MODE_ENCRYPT_RSA_48BITS) {
				memset(rsa_pub_data, 0, efuse_len);

				/*
			     * write efuse_data.txt from RSA public key 48bits.
			     */
			    if( ( fefuse = fopen( efuse_file, "wb+" ) ) == NULL ) {
					mbedtls_fprintf( stderr, "fopen(%s,rb) failed\n", efuse_file );
					goto err_exit;
			    }

				if( fwrite( key, 1, keylen, fefuse ) != keylen )
		        {
		            mbedtls_fprintf( stderr, "fwrite(%lu bytes) to %s failed\n", keylen, efuse_file );
		            goto err_exit;
		        }
			}
    	} else {
			if( ( filesize = lseek( fileno( fin ), 0, SEEK_END ) ) < 0 )
		    {
		        perror( "lseek" );
		        goto err_exit;
		    }

			if( fseek( fin, 0, SEEK_SET ) < 0 )
		    {
		        mbedtls_fprintf( stderr, "fseek(0,SEEK_SET) failed\n" );
		        goto err_exit;
		    }
    	}

        /*
         * Generate the initialization vector as:
         * IV = SHA-256( filesize || filename )[0..15]
         */
        for( i = 0; i < 8; i++ )
            buffer[i] = (unsigned char)( filesize >> ( i << 3 ) );

        mbedtls_md_starts( &sha_ctx );
        mbedtls_md_update( &sha_ctx, buffer, 8 );
        mbedtls_md_update( &sha_ctx, magic, strlen( magic ) );
        mbedtls_md_finish( &sha_ctx, digest );

        memcpy( IV, digest, 16 );

        /*
         * The last four bits in the IV are actually used
         * to store the file size modulo the AES block size.
         */
        lastn = (int)( filesize & 0x0F );

        IV[15] = (unsigned char)
            ( ( IV[15] & 0xF0 ) | lastn );

        /*
         * Append the IV at the beginning of the output.
         */
        if( fwrite( IV, 1, 16, fout ) != 16 )
        {
            mbedtls_fprintf( stderr, "fwrite(%d bytes) failed\n", 16 );
            goto err_exit;
        }

        /*
         * Hash the IV and the secret key together IV_HASH_TIMES times
         * using the result to setup the AES context and HMAC.
         */
        memset( digest, 0,  32 );
        memcpy( digest, IV, 16 );

        for( i = 0; i < IV_HASH_TIMES; i++ )
        {
            mbedtls_md_starts( &sha_ctx );
            mbedtls_md_update( &sha_ctx, digest, 32 );
            mbedtls_md_update( &sha_ctx, key, keylen );
            mbedtls_md_finish( &sha_ctx, digest );
        }

        mbedtls_aes_setkey_enc( &aes_ctx, digest, 256 );
        mbedtls_md_hmac_starts( &sha_ctx, digest, 32 );

        /*
         * Encrypt and write the ciphertext.
         */
        for( offset = 0; offset < filesize; offset += 16 )
        {
            n = ( filesize - offset > 16 ) ? 16 : (int)
                ( filesize - offset );

			if((mode == MODE_ENCRYPT_RSA_48BITS) || (mode == MODE_ENCRYPT_RSA)) {
				if( (n % 4) != 0 )
	            {
	                mbedtls_fprintf( stderr, "File size not a multiple of 4\n");
	                goto err_exit;
	            }

				memcpy(buffer, (rsa_pub_data + offset), n);
			} else {
				if( fread( buffer, 1, n, fin ) != (size_t) n )
	            {
	                mbedtls_fprintf( stderr, "fread(%d bytes) failed\n", n );
	                goto err_exit;
	            }
			}

#if DBG_PRINT
			mbedtls_printf("input data:\n");
			for(dbg_p_idx = 0; dbg_p_idx < n; dbg_p_idx++) {
				mbedtls_printf("0x%x ", buffer[dbg_p_idx]);
			}
			mbedtls_printf("\n");
#endif
            for( i = 0; i < 16; i++ )
                buffer[i] = (unsigned char)( buffer[i] ^ IV[i] );

            mbedtls_aes_crypt_ecb( &aes_ctx, MBEDTLS_AES_ENCRYPT, buffer, buffer );
            mbedtls_md_hmac_update( &sha_ctx, buffer, 16 );

            if( fwrite( buffer, 1, 16, fout ) != 16 )
            {
                mbedtls_fprintf( stderr, "fwrite(%d bytes) failed\n", 16 );
                goto err_exit;
            }

            memcpy( IV, buffer, 16 );
        }

        /*
         * Finally write the HMAC.
         */
        mbedtls_md_hmac_finish( &sha_ctx, digest );

        if( fwrite( digest, 1, 32, fout ) != 32 )
        {
            mbedtls_fprintf( stderr, "fwrite(%d bytes) failed\n", 16 );
            goto err_exit;
        }
    }

    if( (mode == MODE_DECRYPT_RSA_48BITS) || (mode == MODE_DECRYPT_IMGKEY_RSA) || 
		(mode == MODE_DECRYPT_NORMAL) || (mode == MODE_DECRYPT_RSA))
    {
		
#if defined(_WIN32_WCE)
		filesize = fseek( fin, 0L, SEEK_END );
#else
#if defined(_WIN32)
		/*
		 * Support large files (> 2Gb) on Win32
		 */
		li_size.QuadPart = 0;
		li_size.LowPart  =
			SetFilePointer( (HANDLE) _get_osfhandle( _fileno( fin ) ),
							li_size.LowPart, &li_size.HighPart, FILE_END );
	
		if( li_size.LowPart == 0xFFFFFFFF && GetLastError() != NO_ERROR )
		{
			mbedtls_fprintf( stderr, "SetFilePointer(0,FILE_END) failed\n" );
			goto err_exit;
		}
	
		filesize = li_size.QuadPart;
#else
		if( ( filesize = lseek( fileno( fin ), 0, SEEK_END ) ) < 0 )
		{
			perror( "lseek" );
			goto err_exit;
		}
#endif
#endif

		if( fseek( fin, 0, SEEK_SET ) < 0 )
		{
			mbedtls_fprintf( stderr, "fseek(0,SEEK_SET) failed\n" );
			goto err_exit;
		}

        /*
         *  The encrypted file must be structured as follows:
         *
         *        00 .. 15              Initialization Vector
         *        16 .. 31              AES Encrypted Block #1
         *           ..
         *      N*16 .. (N+1)*16 - 1    AES Encrypted Block #N
         *  (N+1)*16 .. (N+1)*16 + 32   HMAC-SHA-256(ciphertext)
         */
        if( filesize < 48 )
        {
            mbedtls_fprintf( stderr, "File too short to be encrypted.\n" );
            goto err_exit;
        }

        if( ( filesize & 0x0F ) != 0 )
        {
            mbedtls_fprintf( stderr, "File size not a multiple of 16.\n" );
            goto err_exit;
        }

        /*
         * Subtract the IV + HMAC length.
         */
        filesize -= ( 16 + 32 );

        /*
         * Read the IV and original filesize modulo 16.
         */
        if( fread( buffer, 1, 16, fin ) != 16 )
        {
            mbedtls_fprintf( stderr, "fread(%d bytes) failed\n", 16 );
            goto err_exit;
        }

        memcpy( IV, buffer, 16 );
        lastn = IV[15] & 0x0F;

#if DBG_PRINT
		mbedtls_printf("IV:\n");
		for(dbg_p_idx = 0; dbg_p_idx < 16; dbg_p_idx++) {
			mbedtls_printf("0x%x ", IV[dbg_p_idx]);
		}
		mbedtls_printf("\n");
#endif

        /*
         * Hash the IV and the secret key together IV_HASH_TIMES times
         * using the result to setup the AES context and HMAC.
         */
        memset( digest, 0,  32 );
        memcpy( digest, IV, 16 );

        for( i = 0; i < IV_HASH_TIMES; i++ )
        {
            mbedtls_md_starts( &sha_ctx );
            mbedtls_md_update( &sha_ctx, digest, 32 );
            mbedtls_md_update( &sha_ctx, key, keylen );
            mbedtls_md_finish( &sha_ctx, digest );
        }

#if DBG_PRINT
		mbedtls_printf("digest:\n");
		for(dbg_p_idx = 0; dbg_p_idx < 32; dbg_p_idx++) {
			mbedtls_printf("0x%x ", digest[dbg_p_idx]);
		}
		mbedtls_printf("\n");
#endif

        mbedtls_aes_setkey_dec( &aes_ctx, digest, 256 );
        mbedtls_md_hmac_starts( &sha_ctx, digest, 32 );

        /*
         * Decrypt and write the plaintext.
         */
        for( offset = 0; offset < filesize; offset += 16 )
        {
            if( fread( buffer, 1, 16, fin ) != 16 )
            {
                mbedtls_fprintf( stderr, "fread(%d bytes) failed\n", 16 );
                goto err_exit;
            }

#if DBG_PRINT
			mbedtls_printf("rsa chipher:\n");
			for(dbg_p_idx = 0; dbg_p_idx < 16; dbg_p_idx++) {
				mbedtls_printf("0x%x ", buffer[dbg_p_idx]);
			}
			mbedtls_printf("\n");
#endif

            memcpy( tmp, buffer, 16 );

            mbedtls_md_hmac_update( &sha_ctx, buffer, 16 );
            mbedtls_aes_crypt_ecb( &aes_ctx, MBEDTLS_AES_DECRYPT, buffer, buffer );

#if DBG_PRINT
			mbedtls_printf("before XOR IV:\n");
			for(dbg_p_idx = 0; dbg_p_idx < 16; dbg_p_idx++) {
				mbedtls_printf("0x%x ", buffer[dbg_p_idx]);
			}
			mbedtls_printf("\n");

			mbedtls_printf("IV:\n");
			for(dbg_p_idx = 0; dbg_p_idx < 16; dbg_p_idx++) {
				mbedtls_printf("0x%x ", IV[dbg_p_idx]);
			}
			mbedtls_printf("\n");
#endif

            for( i = 0; i < 16; i++ )
                buffer[i] = (unsigned char)( buffer[i] ^ IV[i] );

            memcpy( IV, tmp, 16 );

            n = ( lastn > 0 && offset == filesize - 16 )
                ? lastn : 16;

#if DBG_PRINT
			mbedtls_printf("plaintext:\n");
			for(dbg_p_idx = 0; dbg_p_idx < n; dbg_p_idx++) {
				mbedtls_printf("0x%x ", buffer[dbg_p_idx]);
			}
			mbedtls_printf("\n");
#endif
			
            if( fwrite( buffer, 1, n, fout ) != (size_t) n )
            {
                mbedtls_fprintf( stderr, "fwrite(%d bytes) failed\n", n );
                goto err_exit;
            }
        }

        /*
         * Verify the message authentication code.
         */
        mbedtls_md_hmac_finish( &sha_ctx, digest );

        if( fread( buffer, 1, 32, fin ) != 32 )
        {
            mbedtls_fprintf( stderr, "fread(%d bytes) failed\n", 32 );
            goto err_exit;
        }

#if DBG_PRINT
		mbedtls_printf("HMAC digest:\n");
		for(dbg_p_idx = 0; dbg_p_idx < 32; dbg_p_idx++) {
			mbedtls_printf("0x%x ", digest[dbg_p_idx]);
		}
		mbedtls_printf("\n");
		
		mbedtls_printf("HMAC:\n");
		for(dbg_p_idx = 0; dbg_p_idx < 32; dbg_p_idx++) {
			mbedtls_printf("0x%x ", buffer[dbg_p_idx]);
		}
		mbedtls_printf("\n");
#endif

        /* Use constant-time buffer comparison */
        diff = 0;
        for( i = 0; i < 32; i++ ) {
            diff |= digest[i] ^ buffer[i];
#if DBG_PRINT
			mbedtls_printf("diff:0x%x\n", diff);
#endif

        }

        if( diff != 0 )
        {
            mbedtls_fprintf( stderr, "HMAC check failed: wrong key, "
                             "or file corrupted.\n" );
            goto err_exit;
        }
    }

    ret = 0;
	goto exit;

err_exit:
	ret = -1;
exit:
    if( fin )
        fclose( fin );
    if( fout )
        fclose( fout );
	if( fefuse )
		fclose( fefuse );
	if( fkey )
		fclose( fkey );
	//if( key )
	//	free( key );

    memset( buffer, 0, sizeof( buffer ) );
    memset( digest, 0, sizeof( digest ) );

    mbedtls_aes_free( &aes_ctx );
    mbedtls_md_free( &sha_ctx );

    return( ret );
}
#endif /* MBEDTLS_AES_C && MBEDTLS_SHA256_C && MBEDTLS_FS_IO */
