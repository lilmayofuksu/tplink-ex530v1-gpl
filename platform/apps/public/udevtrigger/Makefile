OBJS= udevtrigger.o
EXEC= udevtrigger

CFLAGS = -g -O2

all: $(EXEC)
	$(STRIP) $(EXEC)

.c.o:
	$(CC) -c $(CFLAGS) $<

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@
	$(STRIP) --strip-unneeded $@

clean:
	rm -f *.o *.a $(EXEC)