
CFLAGS += -Wall -Werror=implicit-function-declaration

all:	ivictl.c ../modules/ivi_ioctl.h
	$(CC) $(CFLAGS)  -o ivictl ivictl.c

clean:
	rm -rf ivictl
