CC=gcc
LD=ld

all:mkimage5_qtl mkboot5_qtl
	cp ./mkimage5_qtl ../



mkimage5_qtl: mkimage.c md5.c md5_interface.c
	$(CC) -m32 -g -o mkimage5_qtl -DFULL_BUILD mkimage.c md5.c md5_interface.c
	
mkboot5_qtl: mkboot.c md5.c md5_interface.c
	$(CC) -m32 -g -o mkboot5_qtl mkboot.c md5.c md5_interface.c

clean:
	-rm -f mkimage5_qtl

