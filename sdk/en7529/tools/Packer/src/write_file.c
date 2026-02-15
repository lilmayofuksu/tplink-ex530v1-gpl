
#include "packer.h"

#define FILENAME	("libtest.so")

static uint32_t	off = 0;

static inline void	write_to_file(int fd, void *data, uint32_t size)
{
  if (write(fd, data, size) != (ssize_t)size) {
    perror("write");
    exit(-1);
  }
  off += size;
}

static void		pad_zero(int fd, uint32_t end)
{
  static const char	c = 0;

  while (off < end) {
    write_to_file(fd, (void *)&c, 1);
  }
}

void write_file(char* fileName,t_elf *elf,int sh_num,int sh_off,int isLittleEndian)
{
  int			fd;

  if ((fd = open(fileName, O_WRONLY, 0744)) < 0) {
    perror("open");
    return ;
  }

  write_to_file(fd, elf->elf_header, sizeof(Elf32_Ehdr));

  printf("ELF Header write done\n");

/*
  int ph_off = 0;
  int ph_num = 0;

  if(isLittleEndian)
  {
      ph_off = elf->elf_header->e_phoff;
      ph_num = elf->elf_header->e_phnum;
  }
  else
  {
    ph_off = ntohl(elf->elf_header->e_phoff);
    ph_num = ntohs(elf->elf_header->e_phnum);
  }
  
  printf("Program Header offset is %d\n",ph_off);
  printf("Program Header number is %d\n",ph_num);

  pad_zero(fd, ph_off);
  write_to_file(fd, elf->prog_header, sizeof(Elf32_Phdr) * ph_num);

  printf("Program Header write done\n");

  printf("Section Header number is %d\n",sh_num);

  for (uint16_t id = 0; id < sh_num; id += 1) {

    int sh_type = 0;
    if(isLittleEndian)
      sh_type = elf->section_header[id].sh_type;
    else
    {
      sh_type = ntohl(elf->section_header[id].sh_type);
    }
    
    if (sh_type != SHT_NOBITS) {


      int sh_offset = 0;
      int sh_size = 0;

      if(isLittleEndian)
  {
      sh_offset = elf->section_header[id].sh_offset;
      sh_size = elf->section_header[id].sh_size;
  }
  else
  {
    sh_offset = ntohl(elf->section_header[id].sh_offset);
    sh_size = ntohl(elf->section_header[id].sh_size);
  }

      pad_zero(fd, sh_offset);
      write_to_file(fd, elf->section_data[id], sh_size);
    }
  }
  pad_zero(fd, sh_off);

  for (uint16_t id = 0; id < sh_num; id += 1) {
    write_to_file(fd, &elf->section_header[id], sizeof(Elf32_Shdr));
  }

  */

  close(fd);

  printf("file created: '%s'\n", fileName);
}
