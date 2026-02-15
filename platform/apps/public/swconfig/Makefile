ifndef CFLAGS
CFLAGS = -O2 -g -I ../libnl-tiny/include -D_GNU_SOURCE 
endif
LIBS=-L../libnl-tiny -lnl -lnl-genl

all: libsw.a

%.o: %.c
	$(CC) $(CFLAGS) -fPIC -c -o $@ $^

libsw.a: swlib.o
	$(AR) rcu $@ swlib.o
	$(RANLIB) $@

clean:
	rm -f swlib.o libsw.a