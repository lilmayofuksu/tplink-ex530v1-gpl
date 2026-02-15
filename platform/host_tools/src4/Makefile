CC=gcc
LD=ld

all:mkimage4

mkimage4: 
	$(CC) -Wall -m32 -g -o mkimage4 mkimage.c md5.c md5_interface.c

clean:
	-rm -f mkimage4

