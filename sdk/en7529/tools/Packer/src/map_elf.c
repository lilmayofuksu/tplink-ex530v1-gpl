
#include "packer.h"

void		*map_file(char *filename, size_t *size_ptr)
{
  void		*data;
  off_t		size;
  int32_t	fd;

  if ((fd = open(filename, O_RDONLY)) < 0) {
      perror("open");
      return ((void *)0);
  }

  if ((size = lseek(fd, 0, SEEK_END)) < 0) {
    perror("lseek");
    return ((void *)0);
  }

  printf("In map_file,file size is:%d\n",(int)size);

  if ((data = mmap((void *)0, (size_t)size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
    perror("mmap");
    return ((void *)0);
  }

  close(fd);
  *size_ptr = (size_t)size;

  return (data);
}


static int32_t	map_elf_header(t_elf *elf, void *data, size_t size,int *little_endian,int *need_pack,int *file_type)
{
  if (size < sizeof(Elf32_Ehdr)) {
    printf("Elf32_Ehdr size error,wrong file\n");
    return (-1);
  }

  if (!(elf->elf_header = malloc(sizeof(Elf32_Ehdr)))) {
  	perror("malloc");
    return (-1);
  }

  memcpy(elf->elf_header, data, sizeof(Elf32_Ehdr));

  if (strncmp((char *)elf->elf_header->e_ident, ELFMAG, SELFMAG)) {
    printf("ELF magic error,wrong file\n");
	free(elf->elf_header);
    return (-1);
  }

  if (elf->elf_header->e_ident[EI_DATA] == 2) {
    *little_endian = 0;
  }
  else
    *little_endian = 1;

  if(*little_endian)
    printf("Little endian\n");
  else
  {
    printf("Big endian\n");
  }

  if(*little_endian)
	{
		*file_type = elf->elf_header->e_type;
	}
	else
	{
		*file_type = ntohs(elf->elf_header->e_type);
	}

    printf("file type is %d\n",*file_type);
  
  	char magic1 = 'E';
	char magic2 = 'C';
	char magic3 = 'N';
	char magic4 = 'T';

	if(*file_type == 1 && elf->elf_header->e_ident[9] == magic1 && elf->elf_header->e_ident[10] == magic2 && elf->elf_header->e_ident[11] == magic3 && elf->elf_header->e_ident[12] == magic4)
	{
		printf("ko already packed,will exit!\n");
		*need_pack = 0;
	}
    else if(*file_type == 2 || *file_type == 3)
    {
        int shoff = -1;
        int shnum = -1;
        int shstrndx = -1;
        if(*little_endian)
        {
            shoff = elf->elf_header->e_shoff;
            shnum = elf->elf_header->e_shnum;
            shstrndx = elf->elf_header->e_shstrndx;
        }
        else
        {
            shoff = ntohl(elf->elf_header->e_shoff);
	        shnum = ntohs(elf->elf_header->e_shnum);
	        shstrndx = ntohs(elf->elf_header->e_shstrndx);
        }

        if(shoff == 0 && shnum ==0 && shstrndx == 0)
        {
            printf("executable/so already packed,will exit!\n");
		    *need_pack = 0;
        }
    }
    else
    {
        printf("Need to pack\n");
        *need_pack = 1;
    }
	
  return (0);
}

static int32_t	map_prog_header(t_elf *elf, void *data, size_t size,int little_endian)
{
  size_t	size_headers;

  int ph_num = 0;

  if(little_endian)
  {
      printf("Little endian\n");
      ph_num = elf->elf_header->e_phnum;
  }
  else
  {
    printf("Big endian\n");
    ph_num = ntohs(elf->elf_header->e_phnum);
  }
  size_headers = sizeof(Elf32_Phdr) * ph_num; 

  printf("In map_prog_header File size is %d\n",size);
  printf("In map_prog_header ph_num is %d\n",ph_num);
  printf("In map_prog_header size_headers is %d\n",size_headers);
  

  if (size < sizeof(Elf32_Ehdr) + size_headers) {
    printf("File size is smaller than the sum of ehdr and phdrs,wrong file\n");
    return (-1);
  }

  if (!(elf->prog_header = malloc(size_headers))) {
    perror("malloc");
    return (-1);
  }

  uint8_t * prog_header_start = 0;
  if(little_endian)
    prog_header_start = (uint8_t *)data + elf->elf_header->e_phoff;
  else
    prog_header_start = (uint8_t *)data + ntohl(elf->elf_header->e_phoff);
  
  memcpy(elf->prog_header, prog_header_start, size_headers);

  return (0);
}

static int32_t	map_sections_header(t_elf *elf, void *data, size_t size,int isLittleEndian)
{
  uint16_t	id;

  int sh_num = 0;

  if(isLittleEndian)
  {
      printf("Little endian\n");
      sh_num = elf->elf_header->e_shnum;
  }
  else
  {
    printf("Big endian\n");
    sh_num = ntohs(elf->elf_header->e_shnum);
  }

  if (!(elf->section_header = malloc(sizeof(Elf32_Shdr) * sh_num))) {
    perror("malloc");
    return (-1);
  }

  memset(elf->section_header, 0, sizeof(Elf32_Shdr) * sh_num);

  for (id = 0; id < sh_num; id += 1) {

    int sh_off = 0;

    if(isLittleEndian)
      sh_off = elf->elf_header->e_shoff;
    else
    {
      sh_off =ntohl(elf->elf_header->e_shoff);
    }
    

    if (size < sh_off + id * sizeof(Elf32_Shdr)) {
      printf("Wrong file\n");
	  free(elf->section_header);
      return (-1);
    }

    memcpy(&(elf->section_header[id]), (uint8_t *)data + sh_off + id * sizeof(Elf32_Shdr), sizeof(Elf32_Shdr));

  }

  return (0);
}

static int32_t	map_sections_data(t_elf *elf, void *data, size_t size,int little_endian)
{
  uint16_t	id;
  uint32_t	size_section;

  int sh_num = 0;

  if(little_endian)
  {
      printf("map_sections_data Little endian\n");
      sh_num = elf->elf_header->e_shnum;
  }
  else
  {
    printf("map_sections_data Big endian\n");
    sh_num = ntohs(elf->elf_header->e_shnum);
  }

  if (!(elf->section_data = malloc(sizeof(uint8_t *) * sh_num))) {
    perror("malloc");
    return (-1);
  }

  memset(elf->section_data, 0, sizeof(uint8_t *) * sh_num);

  for (id = 0; id < sh_num; id += 1) {
    
    int sh_type = 0;

    if(little_endian)
      sh_type = elf->section_header[id].sh_type;
    else
    {
      sh_type =ntohl(elf->section_header[id].sh_type);
    }

    if (sh_type == SHT_NOBITS) {
      elf->section_data[id] = (uint8_t *)0;
    }
    else {

      int sh_offset = 0;

    if(little_endian)
      sh_offset = elf->section_header[id].sh_offset;
    else
    {
      sh_offset =ntohl(elf->section_header[id].sh_offset);
    }

     if (size < (size_t)sh_offset) {
	printf("Wrong file\n");
	int i = 0;
	for(i = 0;i<id;++i)
	{
		free(elf->section_data[i]);
	}

	free(elf->section_data);
	return (-1);
      }
      if(little_endian)
        size_section = elf->section_header[id].sh_size;
      else
      {
        size_section =ntohl(elf->section_header[id].sh_size);
      }
      
      if (!(elf->section_data[id] = malloc(size_section))) {
	perror("malloc");
	return (-1);
      }
      memset(elf->section_data[id], 0, size_section);
      memcpy(elf->section_data[id], (uint8_t *)data + sh_offset, size_section);
    }

  }

  return (0);
}

void		*map_elf(void *data, size_t size,int* little_endian,int *need_pack,int *file_type)
{
  t_elf		*elf;
  

  printf("In map_elf,file size is:%d\n",size);

  if (!(elf = malloc(sizeof(t_elf)))) {
    perror("malloc");
    return ((t_elf *)0);
  }
  memset(elf, 0, sizeof(t_elf));
  

  
  if (map_elf_header(elf, data, size,little_endian,need_pack,file_type)) {
    return ((t_elf *)0);
  }

  
  printf("map_elf_header executed,little_endian %d,need_pack %d,file_type %d\n",*little_endian,*need_pack,*file_type);


  if(!(*need_pack))
  {
  	printf("Already Packed,nothing to be done\n");
  	free(elf->elf_header);
	free(elf);
	return ((t_elf *)0);
  }

  if (map_prog_header(elf, data, size,*little_endian)) {
    return ((t_elf *)0);
  }

  printf("map_prog_header executed\n");


  if (map_sections_header(elf, data, size,*little_endian)) {
    return ((t_elf *)0);
  }

  printf("map_sections_header executed\n");


  if (map_sections_data(elf, data, size,*little_endian)) {
    return ((t_elf *)0);
  }

  printf("map_sections_data executed\n");


  return (elf);
}
