CC=gcc
LD=ld

all:mkimage5 mkboot5


mkimage5: 
	$(CC) -m32 -g -o mkimage5 -DFULL_BUILD mkimage.c md5.c md5_interface.c
	
mkboot5:
	$(CC) -m32 -g -o mkboot5 mkboot.c md5.c md5_interface.c

clean:
	-rm -f mkimage5 

