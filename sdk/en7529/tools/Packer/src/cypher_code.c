
#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "packer.h"



void add_pack_magic(t_elf * elf)
{
	char magic1 = 'E';
	char magic2 = 'C';
	char magic3 = 'N';
	char magic4 = 'T';

	
	elf->elf_header->e_ident[9] = magic1;
	elf->elf_header->e_ident[10] = magic2;
	elf->elf_header->e_ident[11] = magic3;
	elf->elf_header->e_ident[12] = magic4;

	printf("Need to pack,magic number added to ident.\n");
}

int32_t destory_section_info(t_elf *elf,int isLittleEndian)
{
    if(isLittleEndian)
    {
        elf->elf_header->e_shnum = 0;
        elf->elf_header->e_shoff = 0;
        elf->elf_header->e_shstrndx = 0;
        elf->elf_header->e_shentsize = 0;
    }
    else
    {
        elf->elf_header->e_shnum = htons(0);
        elf->elf_header->e_shoff = htonl(0);
        elf->elf_header->e_shstrndx = htons(0);
        elf->elf_header->e_shentsize = htons(0);
    }
    
    elf->elf_header->e_ident[9] = 0;
    elf->elf_header->e_ident[10] = 0;
	elf->elf_header->e_ident[11] = 0;
	elf->elf_header->e_ident[12] = 0;
    return (0);
}

int32_t modify_section_header_offset(t_elf *elf,int isLittleEndian)
{
	


    int rand_num = 0;
    int fd = open("/dev/urandom",O_RDONLY);
    if(fd == -1)
    {
        printf("Open /dev/urandom error\n");
        return -1;
    }
    if(read(fd,(char *)&rand_num,sizeof(int)) != sizeof(int))
    {
        printf("error\n");
        close(fd);
        return -1;
    }
    close(fd);
    
    
    printf("rand_num is %d\n",rand_num);
    int rand_offset = abs(rand_num %100) + 1;
    printf("rand_offset is %d\n",rand_offset);
	if(isLittleEndian)
  	{
    	
    	elf->elf_header->e_shoff += rand_offset;
		elf->elf_header->e_shnum += rand_offset;
		elf->elf_header->e_shstrndx += rand_offset;
		
    	
  }
  else
  {
    
    int orig_shoff = ntohl(elf->elf_header->e_shoff);
	int orig_shnum = ntohs(elf->elf_header->e_shnum);
	int orig_shstrndx = ntohs(elf->elf_header->e_shstrndx);
	
	printf("orig_shoff is %d,orig_shnum is %d,orig_shstrndx is %d",orig_shoff,orig_shnum,orig_shstrndx);
	
	elf->elf_header->e_shoff = htonl(orig_shoff + rand_offset);
	elf->elf_header->e_shnum = htons(orig_shnum + rand_offset);
	elf->elf_header->e_shstrndx = htons(orig_shstrndx + rand_offset);
    
  }

  elf->elf_header->e_ident[15] = rand_offset;
  return (0);
}