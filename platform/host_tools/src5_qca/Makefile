CC=gcc
LD=ld

all:mkimage5_qca #mkboot5
	cp ./mkimage5_qca ../



mkimage5_qca: mkimage.c md5.c md5_interface.c
	$(CC) -m32 -g -o $@ -DFULL_BUILD mkimage.c md5.c md5_interface.c
	
mkboot5: mkboot.c md5.c md5_interface.c
	$(CC) -m32 -g -o mkboot5 mkboot.c md5.c md5_interface.c

clean:
	-rm -f mkimage5_qca

