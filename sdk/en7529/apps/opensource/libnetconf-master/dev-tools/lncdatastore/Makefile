
IDGIT = "built from git $(shell git show --pretty=oneline | head -1 | cut -c -20)"

CC = mips-buildroot-linux-uclibc-gcc -mips32r2 -msoft-float
CFLAGS = -Wall -Wextra -I../../src/ -I/proj/mtk69551/tony/Git/netconf/tclinux_phoenix/apps/opensource/NETCONF_LIB/include -I/proj/mtk69551/tony/Git/netconf/tclinux_phoenix/apps/opensource/NETCONF_LIB/include/libxml2 -I/proj/mtk69551/tony/Git/netconf/tclinux_phoenix/apps/opensource/NETCONF_LIB/include/libxml2  -O3 -DNDEBUG -fvisibility=hidden -pthread
CPPFLAGS = -DRCSID=\"$(IDGIT)\" -DNC_WORKINGDIR_PATH=\"\" -DDISABLE_VALIDATION -DDISABLE_URL -DDISABLE_LIBSSH
LIBS = -L/proj/mtk69551/tony/Git/netconf/tclinux_phoenix/apps/opensource/NETCONF_LIB/lib -lxml2 -lm -ldl -pthread -lreadline -lxslt -ldl -lrt

OBJDIR = .obj

LNC_PREFIX = ../../src/
LNC_SRCS = session.c \
	internal.c \
	with_defaults.c \
	nacm.c \
	messages.c \
	notifications.c \
	callbacks.c \
	error.c \
	transapi/transapi.c \
	transapi/xmldiff.c \
	datastore/edit_config.c \
	transapi/yinparser.c \
	datastore/custom/datastore_custom.c \
	datastore/file/datastore_file.c \
	datastore/empty/datastore_empty.c

SRCS = main.c \
	commands.c \
	mreadline.c

OBJS = $(SRCS:%.c=$(OBJDIR)/%.o)
LNC_OBJS = $(LNC_SRCS:%.c=$(OBJDIR)/lnc/%.o)

lncdatastore: $(OBJS) $(LNC_OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(INCLUDE) $^ $(LIBS) -o $@

$(OBJDIR)/lnc/%.o: $(LNC_PREFIX)/%.c
	@[ -d $$(dirname $@) ] || \
		(mkdir -p $$(dirname $@))
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.c
	@[ -d $$(dirname $@) ] || \
		(mkdir -p $$(dirname $@))
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	@-rm -rf $(OBJDIR) lncdatastore
