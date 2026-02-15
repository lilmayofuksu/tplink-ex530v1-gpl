CC=gcc
LD=ld

all:mkimage5 #mkboot5
	cp mkimage5_rtl ../

mkimage5: 
	$(CC) -g -o mkimage5_rtl -DFULL_BUILD mkimage.c md5.c md5_interface.c
	
mkboot5:
	$(CC) -m32 -g -o mkboot5 mkboot.c md5.c md5_interface.c

clean:
	-rm -f mkimage5_rtl

