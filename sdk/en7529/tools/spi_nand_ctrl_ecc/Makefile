INCLUDEDIR = .
CFLAGS =

CFLAGS := -I$(INCLUDEDIR)  -O2
RM = rm -f
CC = gcc

OBJS = main.o bch_bma_hw.o bch_encoder_hw.o

ifneq ($(strip $(TARGETS)),)
	TARGETS	= spi_nand_ctrl_ecc
endif

ifeq ($(strip $(TCSUPPORT_SECURE_BOOT)),1)
	CFLAGS += -DTCSUPPORT_SECURE_BOOT
	CFLAGS += -I$(INSTALL_GLOBALINC)/uapi/boot
endif

ifeq ($(strip $(TCSUPPORT_LITTLE_ENDIAN)),1)
	CFLAGS	+= -DTCSUPPORT_LITTLE_ENDIAN
endif

ifeq ($(strip $(TCSUPPORT_CPU_EN7522)),1)
	CFLAGS	+= -DTCSUPPORT_CPU_EN7522
endif

ifeq ($(TCSUPPORT_SECURE_BOOT), 1)
CFLAGS += -DTCSUPPORT_SECURE_BOOT
CFLAGS += -I$(INSTALL_GLOBALINC)
endif

ifeq ($(TCSUPPORT_SECURE_BOOT_V1), 1)
CFLAGS += -DTCSUPPORT_SECURE_BOOT_V1
endif

ifeq ($(TCSUPPORT_SECURE_BOOT_V2), 1)
CFLAGS += -DTCSUPPORT_SECURE_BOOT_V2
endif

all: $(TARGETS)

.c.o:
	$(CC) -c $(CFLAGS) $<

$(TARGETS): clean $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGETS) $(OBJS) $(LIBDIR)

clean:
	$(RM) $(OBJS) $(TARGETS)