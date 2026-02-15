#ifndef PACKER_H_
# define PACKER_H_

# include <stdio.h>
# include <elf.h>
# include <string.h>
# include <stdlib.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <sys/mman.h>
# include <sys/types.h>
# include <unistd.h>
# include <arpa/inet.h>

typedef struct	s_elf
{
  Elf32_Ehdr	*elf_header;
  Elf32_Phdr	*prog_header;
  Elf32_Shdr	*section_header;
  uint8_t	**section_data;
}		t_elf;

void		*map_file(char *filename, size_t *size_ptr);

void		*map_elf(void *, size_t,int *,int *,int *);

void		write_file(char *,t_elf *,int ,int,int);

void add_pack_magic(t_elf *);
int32_t  destory_section_info(t_elf *,int);

int32_t modify_section_header_offset(t_elf *,int);

#endif /* !PACKER_H_ */
