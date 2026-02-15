#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <errno.h>
#include <limits.h>

#include "ECCGenerator.h"
#include <byteswap.h>
#if defined(TCSUPPORT_SECURE_BOOT)
#include <sHeader.h>
#endif

#define SECTOR_SIZE 512
#define FDM_SIZE 8
#define NFI_SPARE_AREA_MAX_SIZE 28

#define SPI_NAND_INFO 0x28

#define LIVE_PRINT_COUNT 128

//#define ECC_FILEMODE (S_IRUSR | S_IWUSR)
#define ECC_FILEMODE (O_CREAT | O_RDWR)

#ifdef __LITTLE_ENDIAN	//Compile PC define
#if defined(TCSUPPORT_LITTLE_ENDIAN)	//SoC define
#define STORE32_LE(X)	(X)
#else
#define STORE32_LE(X)	bswap_32(X)
#endif
#else
#if defined(TCSUPPORT_LITTLE_ENDIAN)	//SoC define
#define STORE32_LE(X)	bswap_32(X)
#else
#define STORE32_LE(X)	(X)
#endif
#endif

int parseNumber(const char *cp, unsigned int base, unsigned long *parseAddr)
{
	int retVal = 0;
	char *strtolEnd = NULL;

	errno = 0;
	*parseAddr = strtoul(cp, &strtolEnd, base);

	if (strtolEnd == cp) {
		fprintf(stderr, "%s: not a heximal number\n", cp);
		retVal = -1;
	} else if ('\0' != *strtolEnd) {
		fprintf(stderr, "%s: extra characters at end of input: %s\n", cp, strtolEnd);
		retVal = -1;
	} else if (ULONG_MAX == *parseAddr && ERANGE == errno) {
		fprintf(stderr, "%s out of range of type unsigned long\n", cp);
		retVal = -1;
	} else if (0 == *parseAddr && EINVAL == errno) {
		fprintf(stderr, "%s is invalid of type unsigned long\n", cp);
		retVal = -1;
	}

	printf("parseAddr:0x%lu\n", *parseAddr);

	return retVal;
}

int main(int argc, char *argv[])
{
  unsigned char buf[SECTOR_SIZE + NFI_SPARE_AREA_MAX_SIZE] = {0};
  FILE *ifd = NULL;
  FILE *ofd = NULL;
  ssize_t nread = 0;
  int i = 0;
  int count = 0;
  unsigned char name_buf[40] = {0};
  int sector_idx = 0;
  int padding = 0;
  unsigned char *padding_buf = NULL;
  unsigned long page_size = 0;
  unsigned long flash_spare_area_size = 0;
  unsigned long ecc_ability = 0;
  int isGetSpiNandInfo = 0;
  unsigned long nfi_spare_area_size = 0;
  unsigned long spi_nand_info = 0;
  int sector_num_per_page = 0;
  int live_print = 0;
  int sHeaderLen = 0;

#if defined(TCSUPPORT_SECURE_BOOT_V2)
	SECURE_HEADER_V2 sHeader;
	sHeaderLen = sizeof(SECURE_HEADER_V2);
#elif defined(TCSUPPORT_SECURE_BOOT_V1) || defined(TCSUPPORT_SECURE_BOOT_FLASH_OTP)
	SECURE_HEADER_V1 sHeader;
	sHeaderLen = sizeof(SECURE_HEADER_V1);
#endif

#ifdef TCSUPPORT_CPU_EN7522
  if(argc < 4) {
	printf("usage: %s <src image> <dst path> <spare area size>\n", argv[0]);
	exit(0);        
  }
#else
  if(argc < 5) {
	printf("usage: %s <src image> <dst path> <spare area size> <page size>\n", argv[0]);
	exit(0);		
  }
#endif

  
  
  ifd = fopen(argv[1], "rb");
  if(!ifd) {
      perror("open error.");        
      goto exit;
  }
  
  ofd = fopen(argv[2], "wb");
  if(!ofd) {
      perror("create error.");        
      goto exit;
  }

  if(parseNumber(argv[3], 10, &flash_spare_area_size) == -1) {
  	printf("parseNumber argv[3]:%s error\n", argv[3]);
	goto exit;
  }

  if(flash_spare_area_size > 512) {
  	printf("flash_spare_area_size:0x%02lx more than 512.\n", flash_spare_area_size);
	goto exit;
  }

#ifdef TCSUPPORT_CPU_EN7522
  page_size = 2048;
#else
  page_size = atoi(argv[4]);
#endif

  sector_num_per_page = page_size / SECTOR_SIZE;
  
  printf("\ninput:%s\n", argv[1]);  
  /* for coverity(CID:270136), avoid printf_arg_mismatch*/
  printf("  page size:0x%04lx\n", page_size); 
  /* for coverity(CID:270135), avoid printf_arg_mismatch*/
  printf("  flash spare area size:0x%02lx\n", flash_spare_area_size); 
  printf("\nGenerating tclinux_allinone_CtrlECC_NAND..., Please wait a moment.\n");
  
  memset(buf, 0xFF, sizeof(buf));
    
  /* read from image file, SECTOR_SIZE bytes at one time */
  nread = fread(buf, sizeof(unsigned char), SECTOR_SIZE, ifd);
  
  do {
      /*  */
      if(nread < SECTOR_SIZE) {
          /* if more than one time, this program will generate incorrect result.
           * we have not yet processed this special case.
           */
           if(++count > 1) {
                /* for coverity(CID:270139), avoid printf_arg_mismatch*/
                printf("error: read size:%zu < %d more than one time, please execute again!\n", nread, SECTOR_SIZE);
                goto exit;
           }
           
           for(i = nread; i < sizeof(buf); i++) {
                 buf[i] = 0x0;      
           }
      }
      
      if(isGetSpiNandInfo == 0) {
#ifdef TCSUPPORT_CPU_EN7522
           nfi_spare_area_size = 16;
           ecc_ability = 4;
#else
           /* get NFI spare area size from 0x28 */
           spi_nand_info = *(unsigned long *)(buf + SPI_NAND_INFO + sHeaderLen);
           spi_nand_info = STORE32_LE(spi_nand_info);
       
           nfi_spare_area_size = (spi_nand_info & 0xFFFF);
           ecc_ability = ((spi_nand_info >> 16) & 0xFFFF);
#endif

           printf("parse from %s:\n", argv[1]); 
           /* for coverity(CID:270138), avoid printf_arg_mismatch*/
           printf("  nfi spare area size:%lu\n", nfi_spare_area_size);
           /* for coverity(CID:270137), avoid printf_arg_mismatch*/ 
           printf("  ECC ability:%lu\n", ecc_ability);
           
           if(flash_spare_area_size < (nfi_spare_area_size * sector_num_per_page)) {
                /* for coverity(CID:270140, 270141), avoid printf_arg_mismatch*/
                printf("error: spare area size:%lu < %lu nfi_spare_area_size * 4\n", flash_spare_area_size, nfi_spare_area_size * 4);
                goto exit;
           }
           
/* for coverity(CID:270132), avoid dead_error_condition*/
#ifndef TCSUPPORT_CPU_EN7522
           if ( (ecc_ability != 4) && (ecc_ability != 8) && (ecc_ability != 12) )
           {
                /* for coverity(CID:270142), avoid printf_arg_mismatch*/
                printf("ecc_ability[%lu] must be 4, 8 or 12, HALT!\r\n", ecc_ability);
                goto exit;
           }
#endif
			padding = (flash_spare_area_size - (nfi_spare_area_size * sector_num_per_page));
			/*for coverity(CID:207143), avoid passing tainted variable "padding" to a tainted sink*/
			if(padding > 0)
			{
				padding_buf = (unsigned char *)malloc(padding);
				if(NULL == padding_buf)
				{
					printf("malloc fail!\r\n");
					goto exit;
				}
				memset(padding_buf, 0xFF, padding);
			}

			isGetSpiNandInfo = 1;
		}
      
      /* spare area should be all 0xFF */
      for(i = SECTOR_SIZE; i < SECTOR_SIZE + NFI_SPARE_AREA_MAX_SIZE; i++) {
             buf[i] = 0xFF;      
      }
      
      /* invert data */
#ifndef TCSUPPORT_CPU_EN7522
      for(i = 0; i < SECTOR_SIZE + NFI_SPARE_AREA_MAX_SIZE; i++) {
             buf[i] = buf[i] ^ 0xFF;      
      }
#endif

      if(NULL == ECC_Generator_Main(buf, buf + SECTOR_SIZE + FDM_SIZE, SECTOR_SIZE + FDM_SIZE, ecc_ability))
      {
            printf("%s:%d error:ECC_Generator_Main output is null!\r\n", __FUNCTION__, __LINE__);
            goto exit;
      }
	  
      /* invert data */
#ifndef TCSUPPORT_CPU_EN7522
      for(i = 0; i < SECTOR_SIZE + NFI_SPARE_AREA_MAX_SIZE; i++) {
             buf[i] = buf[i] ^ 0xFF;      
      }
#endif
      
      /*
      t = 4,  outBuf_size_in_byte_org = 6.5 => 7
      t = 8,  outBuf_size_in_byte_org = 13
      t = 12, outBuf_size_in_byte_org = 19.5 => 20 
      */
      /* last byte of OOB should be 0xFF */
      switch(ecc_ability)
      {
          case 4:
               buf[SECTOR_SIZE + FDM_SIZE + 7 - 1] |= 0xF0;
               break;
/* for coverity(CID:270132), avoid dead_error_condition*/
#ifndef TCSUPPORT_CPU_EN7522
          case 12:
               buf[SECTOR_SIZE + FDM_SIZE + 20 - 1] |= 0xF0;
               break;
#endif
      }

      if(fwrite(buf, sizeof(unsigned char), SECTOR_SIZE + nfi_spare_area_size, ofd) != SECTOR_SIZE + nfi_spare_area_size) {
          perror("write error.");        
          goto exit;
      }
      
      sector_idx++;
      
      if(padding > 0) {
          if((sector_idx % sector_num_per_page) == 0) {
              if(fwrite(padding_buf, sizeof(unsigned char), padding, ofd) != padding) {
                   perror("write error 1.");        
                   goto exit;
              }
          }
      }

	  live_print++;
	  if(live_print > LIVE_PRINT_COUNT) {
	  	live_print = 0;
        printf(".");
		fflush(stdout);
	  }
	  
      nread = fread(buf, sizeof(unsigned char), SECTOR_SIZE, ifd);
  } while(nread > 0);

  printf("\n");
  
  if(nread < 0) {
        printf("read error\n");
        goto exit;
   }

exit:
  if(NULL != padding_buf) {
  	free(padding_buf);
  }
  if(NULL != ifd) {
      fclose(ifd);
  }
  if(NULL != ofd) {
      fclose(ofd);
  }
  return 0;
}
