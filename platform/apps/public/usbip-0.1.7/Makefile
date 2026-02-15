APP_PATH	= src

usbip_server: usbipd

usbipd:
	( export CFLAGS+="-Wall -Werror=implicit-function-declaration"; \
	  cd $(APP_PATH) && \
	  ./configure ac_config_files=lib/sysfs/Makefile --host=$(HOST) --enable-shared=no --enable-static=yes --without-PACKAGE && \
	  make && cp cmd/usbipd .. && cd .. \
	)

clean:
	-rm -f usbipd
	if test -e $(APP_PATH)/Makefile; then make -C $(APP_PATH) clean; fi
