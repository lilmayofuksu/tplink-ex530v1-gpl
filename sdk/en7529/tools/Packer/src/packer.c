
#include "packer.h"


int get_fileType(t_elf * elf,int isLittleEndian)
{
	int file_type = -1;
	
	if(isLittleEndian)
	{
		file_type = elf->elf_header->e_type;
	}
	else
	{
		file_type = ntohs(elf->elf_header->e_type);
	}

	return file_type;
}

void release_memory(t_elf * elf,int sh_num)
{
	for (uint16_t id = 0; id < sh_num; id += 1) 
	{
    	free(elf->section_data[id]);
  	}
  	free(elf->elf_header);
  	free(elf->prog_header);
  	free(elf->section_data);
  	free(elf->section_header);
  	free(elf);
}

void get_section_info(t_elf * elf,int isLittleEndian,int * sh_num,int *sh_off)
{
	if(isLittleEndian)
	  {
		*sh_num = elf->elf_header->e_shnum;
		*sh_off = elf->elf_header->e_shoff;
	  }
	  else
	  {
		*sh_num = ntohs(elf->elf_header->e_shnum);
		*sh_off = ntohl(elf->elf_header->e_shoff);
	  }

   printf("sh_num is %d,sh_off is %d\n",*sh_num,*sh_off);

}

int		main(int argc, char **argv)
{
  
  void		*data = NULL;
  t_elf		*elf = NULL;
  size_t	size;
  int isLittleEndian = 1;
  int need_pack = 1;
  int file_type = -1;
  int sh_num = 0;
  int sh_off = 0;
  
  if (argc < 2) {
    printf("usage: %s FILE\n", argv[0]);
    return (0);
  }


  if (!(data = map_file(argv[1], &size))) {
  	printf("map_file error\n");
    return (0);
  }

  

  printf("map_file executed\n");

  if (!(elf = map_elf(data, size,&isLittleEndian,&need_pack,&file_type))) {
  	munmap(data, size);
    return (0);
  }
   
  printf("map_elf executed\n");

  

  if(!need_pack)
  	return 0;

  if(file_type == 1)
    add_pack_magic(elf);


  
  get_section_info(elf,isLittleEndian,&sh_num,&sh_off);
  

  if(file_type <=0)
  {
  	printf("Wrong file type\n");
	release_memory(elf,sh_num);
	return -1;
  }
  
  
   
  if(file_type == 2 || file_type == 3) 
  {
  	if(file_type == 2)
		printf("Process executable file\n");
	else
		printf("Process so file\n");
  	destory_section_info(elf,isLittleEndian);
	printf("destory_section_info executed\n");
  }
  else
  {
  	printf("Process ko file\n");
  	modify_section_header_offset(elf,isLittleEndian);
	printf("modify_section_header_offset executed\n");
  }
  	
  
  write_file(argv[1],elf,sh_num,sh_off,isLittleEndian);

  
  release_memory(elf,sh_num);

  printf("Pack finished\n");
  return (0);
}
