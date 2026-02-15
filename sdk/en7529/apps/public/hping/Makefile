ROOTPATH=$(shell pwd)

HPING3PATH = $(ROOTPATH)/hping3
TCLPATH = $(ROOTPATH)/tcl8.4.20/unix

RM = rm -rf
MV = mv -f
CP = cp -fpR

all : hping3

tcl:
	cd $(TCLPATH) && ./configure --host=$(COMPILE_TOOLCHAIN) --target=$(COMPILE_TOOLCHAIN) --prefix=$(TCLPATH)/tcl_install && make CC=$(CROSS)gcc AR=$(CROSS)ar RANLIB=$(CROSS)ranlib LD=$(CROSS)ld&& make install
	#cd $(TCLPATH) && make && make install
	$(CP) $(TCLPATH)/../generic/tcl*.h $(LIB_DIR)
	$(CP) $(TCLPATH)/tcl_install/lib/*.so $(LIB_DIR)
	$(CP) $(TCLPATH)/tcl_install/lib/*.so $(FILESYSTEM_DIR)/lib/
	
hping3: tcl
	#cd $(HPING3PATH) && ./configure --libpcap-dir=$(LIB_DIR) 
	cd $(HPING3PATH) && make

clean : tcl_clean hping3_clean

tcl_clean:
	- cd $(TCLPATH) && make clean
	- $(RM) $(FILESYSTEM_DIR)/lib/libtcl*.so

hping3_clean:
	- cd $(HPING3PATH) && make clean
	
.PHONY : all clean hping3 tcl hping3_clean tcl_clean