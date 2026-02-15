include apps/private/apps_rule.mak
include apps/private/apps_dir.mak

release_makefile_app:
#copy makefiles
	cp $(APP_TCAPI_CWMP_LIB_DIR)/Makefile.release $(APP_TCAPI_CWMP_LIB_DIR)/Makefile

	cp $(APP_CFG_NG_DIR)/service/cfg/Makefile.release $(APP_CFG_NG_DIR)/service/cfg/Makefile

	-cp -rf $(APP_DMS_DIR)/src/dms/avt/Makefile.am.release $(APP_DMS_DIR)/src/dms/avt/Makefile.am
	-cp -rf $(APP_DMS_DIR)/src/dms/cds/cdsMgr/Makefile.am.release $(APP_DMS_DIR)/src/dms/cds/cdsMgr/Makefile.am
	-cp -rf $(APP_DMS_DIR)/src/dms/cds/clm/Makefile.am.release $(APP_DMS_DIR)/src/dms/cds/clm/Makefile.am
	-cp -rf $(APP_DMS_DIR)/src/dms/cds/fsm/Makefile.am.release $(APP_DMS_DIR)/src/dms/cds/fsm/Makefile.am
	-cp -rf $(APP_DMS_DIR)/src/dms/cms/Makefile.am.release $(APP_DMS_DIR)/src/dms/cms/Makefile.am
	-cp -rf $(APP_DMS_DIR)/src/dms/eventMgr/Makefile.am.release $(APP_DMS_DIR)/src/dms/eventMgr/Makefile.am
	-cp -rf $(APP_DMS_DIR)/src/dms/ms_mrs/Makefile.am.release $(APP_DMS_DIR)/src/dms/ms_mrs/Makefile.am
	-cp -rf $(APP_DMS_DIR)/src/dms/UnitTest/Makefile.am.release $(APP_DMS_DIR)/src/dms/UnitTest/Makefile.am
####################################################################################
backup_before_make:
ifneq ($(strip $(TCSUPPORT_CT_CUSTOMCODE)),)
	# cfg_manager
	mkdir -p $(OPEN_CODE_TMP)/$(APP_CFG_MANAGER_DIR)
	cp $(APP_CFG_MANAGER_DIR)/* $(OPEN_CODE_TMP)/$(APP_CFG_MANAGER_DIR)
	# cwmp source
	mkdir -p $(OPEN_CODE_TMP)/$(APP_CWMP_DIR)/cwmp/Sources
	cp $(APP_CWMP_DIR)/cwmp/Sources/* $(OPEN_CODE_TMP)/$(APP_CWMP_DIR)/cwmp/Sources	
	mkdir -p $(OPEN_CODE_TMP)/$(APP_RA_HWNAT_7510_DIR)
	cp $(APP_RA_HWNAT_7510_DIR)/* $(OPEN_CODE_TMP)/$(APP_RA_HWNAT_7510_DIR)

	cp -a $(OPEN_CODE_TMP)/$(TRUNK_DIR)/* $(OPEN_CODE)/
	tar -czvf ../sdk_opencode_`date +%Y%m%d`.tgz $(OPEN_CODE)/*
	rm -rf $(OPEN_CODE_TMP) $(OPEN_CODE)
endif
ifneq ($(strip $(JOYME_MULTI_SDK)),)
	mkdir -p $(JOYME_ISNTALL_CODE) && mkdir -p $(JOYME_ISNTALL_CODE_TMP)
	
	# cfg_manager
	mkdir -p $(JOYME_ISNTALL_CODE_TMP)/$(APP_CFG_MANAGER_DIR)
	cp $(APP_CFG_MANAGER_DIR)/cmccv2.c $(JOYME_ISNTALL_CODE_TMP)/$(APP_CFG_MANAGER_DIR)
	cp $(APP_CFG_MANAGER_DIR)/cmccv2.h $(JOYME_ISNTALL_CODE_TMP)/$(APP_CFG_MANAGER_DIR)
	
	# webpage
	mkdir -p $(JOYME_ISNTALL_CODE_TMP)/$(APP_WEBPAGE_DIR)/Router/cmcc
	cp -a $(APP_WEBPAGE_DIR)/Router/cmcc/boaroot $(JOYME_ISNTALL_CODE_TMP)/$(APP_WEBPAGE_DIR)/Router/cmcc/
	mkdir -p $(JOYME_ISNTALL_CODE_TMP)/$(APP_WEBPAGE_DIR)/Router/cmccv2
	cp -a $(APP_WEBPAGE_DIR)/Router/cmccv2/boaroot $(JOYME_ISNTALL_CODE_TMP)/$(APP_WEBPAGE_DIR)/Router/cmccv2/
endif


joyme_change:
ifneq ($(strip $(SDK)),)
	echo "TC_BUILD_RELEASECODE=y" > install_bsp/Project/release.chk
	echo "export TC_BUILD_RELEASECODE" >> install_bsp/Project/release.chk
	
	-mkdir -p $(APP_WEBPAGE_DIR)/Router/cmccv2
	-cp -rf $(JOYME_ISNTALL_CODE)/$(APP_INSTALL_WEBPAGE_DIR)/Router/cmccv2/* $(APP_WEBPAGE_DIR)/Router/cmccv2
	-cp -f $(JOYME_ISNTALL_CODE)/$(APP_INSTALL_BINARY_DIR)/* $(BIN_BAK_PATH)/apps/binary/$(TCPLATFORM)

ifeq ($(strip $(SDK)),V1)
	echo "TC_BUILD_JOYME_V1=y" >> install_bsp/Project/release.chk
	echo "export TC_BUILD_JOYME_V1" >> install_bsp/install_bsp/Project/release.chk

	-rm -f $(MODULES_BACKUP)/$(TCPLATFORM)/wlanshare.ko

	-rm -rf $(APP_CMCC2_DIR)/joyme_lib
	-rm -f $(LIB_DIR)/joyme.h $(LIB_DIR)/lanservices.h $(LIB_DIR)/commservices.h \
				$(LIB_DIR)/accessservices.h $(LIB_DIR)/addressservices.h $(LIB_DIR)/transferservices.h $(LIB_DIR)/voipservices.h \
				$(LIB_DIR)/libjoyme.so* 
	-rm -f $(LIB_DIR)/joyme.h $(PUBLIC_LIB_DIR)/lanservices.h $(PUBLIC_LIB_DIR)/commservices.h \
				$(PUBLIC_LIB_DIR)/accessservices.h $(PUBLIC_LIB_DIR)/addressservices.h $(PUBLIC_LIB_DIR)/transferservices.h $(PUBLIC_LIB_DIR)/voipservices.h \
				$(PUBLIC_LIB_DIR)/libjoyme.so* 
	
	-rm -rf $(APP_CMCC2_DIR)/bundle_lib
	-rm -f $(LIB_DIR)/bundle.h $(LIB_DIR)/lanservicesjni.h $(LIB_DIR)/commservicesjni.h \
				$(LIB_DIR)/accessservicesjni.h $(LIB_DIR)/addressservicesjni.h $(LIB_DIR)/transferservicesjni.h $(LIB_DIR)/voipservicesjni.h \
				$(LIB_DIR)/libbundle.so* 
	-rm -f $(PUBLIC_LIB_DIR)/bundle.h $(PUBLIC_LIB_DIR)/lanservicesjni.h $(PUBLIC_LIB_DIR)/commservicesjni.h \
				$(PUBLIC_LIB_DIR)/accessservicesjni.h $(PUBLIC_LIB_DIR)/addressservicesjni.h $(PUBLIC_LIB_DIR)/transferservicesjni.h $(PUBLIC_LIB_DIR)/voipservicesjni.h \
				$(PUBLIC_LIB_DIR)/libbundle.so* 

	-rm -rf $(APP_CMCC2_DIR)/bundle_state

	-rm -f $(APP_BINARY_DIR)/$(TCPLATFORM)/trafficmonitor $(APP_BINARY_DIR)/$(TCPLATFORM)/trafficmirror \
				 $(APP_BINARY_DIR)/$(TCPLATFORM)/trafficdetail $(APP_BINARY_DIR)/$(TCPLATFORM)/jvm_monitor \
				 $(APP_BINARY_DIR)/$(TCPLATFORM)/bandwidth

	-rm -rf $(APP_WEBPAGE_DIR)/Router/cmccv2
	
	echo -e "#ifndef _CMCCV2_H\n#define _CMCCV2_H\n#include \"cfg_manager.h\"\n" > $(APP_CFG_MANAGER_DIR)/cmccv2.h
	echo -e "int wlanshare_init(void);\n" >> $(APP_CFG_MANAGER_DIR)/cmccv2.h
	echo -e "#endif\n" >> $(APP_CFG_MANAGER_DIR)/cmccv2.h
	
	echo -e "#include \"cmccv2.h\"" > $(APP_CFG_MANAGER_DIR)/cmccv2.c
	echo -e "int wlanshare_init(void) {return 0;}\n" >> $(APP_CFG_MANAGER_DIR)/cmccv2.c
	echo -e "int checkconflictssid(mxml_node_t *top, int type, int ssid) {return SUCCESS;}\n" >> $(APP_CFG_MANAGER_DIR)/cmccv2.c
endif
ifeq ($(strip $(SDK)),V2)
	echo "TC_BUILD_JOYME_V2=y" >> install_bsp/Project/release.chk
	echo "export TC_BUILD_JOYME_V2" >> install_bsp/Project/release.chk

	-rm -f $(MODULES_BACKUP)/$(TCPLATFORM)/wlanshare.ko
	-mkdir -p $(APP_CMCC2_DIR)/joyme_lib
	-cp -rf $(JOYME_ISNTALL_CODE)/$(APP_INSTALL_CMCC2_DIR)/joyme_lib/* $(APP_CMCC2_DIR)/joyme_lib
	-rm -f $(APP_CMCC2_DIR)/joyme_lib/*_v1.c
	-mkdir -p $(APP_CMCC2_DIR)/bundle_lib
	-cp -rf $(JOYME_ISNTALL_CODE)/$(APP_INSTALL_CMCC2_DIR)/bundle_lib/* $(APP_CMCC2_DIR)/bundle_lib
	-mkdir -p $(APP_CMCC2_DIR)/bundle_state
	-cp -rf $(JOYME_ISNTALL_CODE)/$(APP_INSTALL_CMCC2_DIR)/bundle_state/* $(APP_CMCC2_DIR)/bundle_state

	-rm -f $(APP_WEBPAGE_DIR)/Router/cmccv2/boaroot/cgi-bin/app-*.asp
	-rm -f $(APP_WEBPAGE_DIR)/Router/cmccv2/boaroot/cgi-bin/app_*.cgi
	-rm -f $(APP_WEBPAGE_DIR)/Router/cmccv2/boaroot/cgi-bin/sec-*.asp
	-rm -f $(APP_WEBPAGE_DIR)/Router/cmccv2/boaroot/cgi-bin/sec_*.cgi
	-rm -f $(APP_WEBPAGE_DIR)/Router/cmccv2/boaroot/cgi-bin/mag-*.asp
	-rm -f $(APP_WEBPAGE_DIR)/Router/cmccv2/boaroot/cgi-bin/diag-*.asp

	echo -e "#ifndef _CMCCV2_H\n#define _CMCCV2_H\n#include \"cfg_manager.h\"\n" > $(APP_CFG_MANAGER_DIR)/cmccv2.h
	echo -e "int wlanshare_init(void);\n" >> $(APP_CFG_MANAGER_DIR)/cmccv2.h
	echo -e "#endif\n" >> $(APP_CFG_MANAGER_DIR)/cmccv2.h
	
	echo -e "#include \"cmccv2.h\"" > $(APP_CFG_MANAGER_DIR)/cmccv2.c
	echo -e "int wlanshare_init(void) {return 0;}\n" >> $(APP_CFG_MANAGER_DIR)/cmccv2.c
	echo -e "int checkconflictssid(mxml_node_t *top, int type, int ssid) {return SUCCESS;}\n" >> $(APP_CFG_MANAGER_DIR)/cmccv2.c
endif
ifeq ($(strip $(SDK)),V3)
	echo "TC_BUILD_JOYME_V3=y" >> install_bsp/Project/release.chk
	echo "export TC_BUILD_JOYME_V3" >> install_bsp/Project/release.chk

	-cp -f $(JOYME_ISNTALL_CODE)/$(MODULES_INSTALL_PRIV_DIR)/wlanshare.ko $(MODULES_BACKUP)/$(TCPLATFORM)

	-mkdir -p $(APP_CMCC2_DIR)/joyme_lib
	-cp -rf $(JOYME_ISNTALL_CODE)/$(APP_INSTALL_CMCC2_DIR)/joyme_lib/* $(APP_CMCC2_DIR)/joyme_lib
	-mkdir -p $(APP_CMCC2_DIR)/bundle_lib
	-cp -rf $(JOYME_ISNTALL_CODE)/$(APP_INSTALL_CMCC2_DIR)/bundle_lib/* $(APP_CMCC2_DIR)/bundle_lib
	-mkdir -p $(APP_CMCC2_DIR)/bundle_state
	-cp -rf $(JOYME_ISNTALL_CODE)/$(APP_INSTALL_CMCC2_DIR)/bundle_state/* $(APP_CMCC2_DIR)/bundle_state

	-cp -f $(JOYME_ISNTALL_CODE)/$(APP_INSTALL_CFG_MANAGER_DIR)/cmccv2.* $(APP_CFG_MANAGER_DIR)
endif
ifeq ($(strip $(SDK)),V4)
	echo "TC_BUILD_JOYME_V4=y" >> install_bsp/Project/release.chk
	echo "export TC_BUILD_JOYME_V4" >> install_bsp/Project/release.chk

	-cp -f $(JOYME_ISNTALL_CODE)/$(MODULES_INSTALL_PRIV_DIR)/wlanshare.ko $(MODULES_BACKUP)/$(TCPLATFORM)

	-mkdir -p $(APP_CMCC2_DIR)/joyme_lib
	-cp -rf $(JOYME_ISNTALL_CODE)/$(APP_INSTALL_CMCC2_DIR)/joyme_lib/* $(APP_CMCC2_DIR)/joyme_lib
	-rm -f $(APP_CMCC2_DIR)/joyme_lib/*.c
	-mkdir -p $(APP_CMCC2_DIR)/bundle_lib
	-cp -rf $(JOYME_ISNTALL_CODE)/$(APP_INSTALL_CMCC2_DIR)/bundle_lib/* $(APP_CMCC2_DIR)/bundle_lib
	-mkdir -p $(APP_CMCC2_DIR)/bundle_state
	-cp -rf $(JOYME_ISNTALL_CODE)/$(APP_INSTALL_CMCC2_DIR)/bundle_state/* $(APP_CMCC2_DIR)/bundle_state

	-cp -f $(JOYME_ISNTALL_CODE)/$(APP_INSTALL_CFG_MANAGER_DIR)/cmccv2.* $(APP_CFG_MANAGER_DIR)
endif

endif

####################################################################################
release_app: release_apps release_webpage release_led_conf
		
release_apps:
	echo "release apps..."
	mkdir -p $(BIN_BAK_PATH)/apps/binary/$(TCPLATFORM)

#release restore_linos_info
ifneq ($(strip $(TCSUPPORT_RESTORE_LINOS_INFO)),)
	cp -rf $(APP_RESTORE_LINOS_INFO_DIR)/restore_linos_info  $(BIN_BAK_PATH)/apps/binary/$(TCPLATFORM)
endif

#release skbmgr
ifeq ($(strip $(TCSUPPORT_CT)),)
	cp -rf $(APP_SKB_MANAGER_DIR)/skbmgr  $(BIN_BAK_PATH)/apps/binary/$(TCPLATFORM)	
endif
	

#Release simcard-app
ifneq ($(strip $(TCSUPPORT_CT_SIMCARD_SEPARATION)),)
	cp -rf $(APP_SIM_CARD_DIR)/simCard  $(BIN_BAK_PATH)/apps/binary/$(TCPLATFORM)
	cp -rf $(APP_SIM_CARD_DIR)/simcardapp/simtest  $(BIN_BAK_PATH)/apps/binary/$(TCPLATFORM)
endif

#Release block-process
ifneq ($(strip $(TCSUPPORT_CT_BLOCK_PROCESS)),)
	cp -rf $(APP_BLOCK_PROCESS_DIR)/blockProcess  $(BIN_BAK_PATH)/apps/binary/$(TCPLATFORM)
endif

#Release zigbee
ifneq ($(strip $(TCSUPPORT_ZIGBEE)),)
	cp -f $(APP_ZIGBEE_DIR)/build/exe/econ_Z3GatewayHost  $(BIN_BAK_PATH)/apps/binary/$(TCPLATFORM)/
endif

#Release ctc igd1 dbus
ifneq ($(strip $(TCSUPPORT_CT_DBUS)),)
	mkdir -p $(BIN_BAK_PATH)/apps/private/ctc-gdbus/ctc_igd1_dbus/$(TCPLATFORM)/src/notify
	cp -f $(APP_CTC_GDBUS_DIR)/ctc_igd1_dbus/src/notify/ctc_dbus_value_change.o $(BIN_BAK_PATH)/apps/private/ctc-gdbus/ctc_igd1_dbus/$(TCPLATFORM)/src/notify
endif

#Release stb_test
ifneq ($(strip $(TCSUPPORT_CT_STB_TEST)),)
	cp -rf $(APP_STB_TEST_DIR)/stb_test  $(BIN_BAK_PATH)/apps/binary/$(TCPLATFORM)
endif

#Release igmpproxy
ifneq ($(strip $(TCSUPPORT_IGMP_PROXY_V3)),)
	cp -rf $(APP_IGMPPROXY_DIR)/igmpproxy  $(BIN_BAK_PATH)/apps/binary/$(TCPLATFORM)
endif

#Release dms
ifneq ($(strip $(TCSUPPORT_DMS)),)
	mkdir -p  $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/avt/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/avt/*.o  $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/avt/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/avt/*.lo $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/avt/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/avt/*.la $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/avt/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/avt/.libs $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/avt/$(TCPLATFORM)	
	
	mkdir -p   $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/cds/cdsMgr/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/cds/cdsMgr/*.o  $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/cds/cdsMgr/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/cds/cdsMgr/*.lo  $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/cds/cdsMgr/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/cds/cdsMgr/*.la  $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/cds/cdsMgr/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/cds/cdsMgr/.libs  $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/cds/cdsMgr/$(TCPLATFORM)

	mkdir -p   $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/cds/clm/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/cds/clm/*.o   $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/cds/clm/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/cds/clm/*.lo  $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/cds/clm/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/cds/clm/*.la  $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/cds/clm/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/cds/clm/.libs  $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/cds/clm/$(TCPLATFORM)

	mkdir -p   $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/cds/fsm/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/cds/fsm/*.o $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/cds/fsm/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/cds/fsm/*.lo $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/cds/fsm/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/cds/fsm/*.la $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/cds/fsm/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/cds/fsm/.libs $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/cds/fsm/$(TCPLATFORM)

	mkdir -p  $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/cds/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/cds/.libs $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/cds/$(TCPLATFORM)

	mkdir -p $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/cms/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/cms/*.o  $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/cms/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/cms/*.lo $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/cms/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/cms/*.la $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/cms/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/cms/.libs $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/cms/$(TCPLATFORM)

	mkdir -p $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/eventMgr/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/eventMgr/*.o  $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/eventMgr/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/eventMgr/*.lo $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/eventMgr/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/eventMgr/*.la $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/eventMgr/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/eventMgr/.libs $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/eventMgr/$(TCPLATFORM)

	mkdir -p $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/ms_mrs/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/ms_mrs/*.o  $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/ms_mrs/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/ms_mrs/*.lo $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/ms_mrs/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/ms_mrs/*.la $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/ms_mrs/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/ms_mrs/.libs $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/ms_mrs/$(TCPLATFORM)

	mkdir -p  $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/UnitTest/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/UnitTest/*.o  $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/UnitTest/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/UnitTest/*.lo $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/UnitTest/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/UnitTest/*.la $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/UnitTest/$(TCPLATFORM)
	-cp -rf $(APP_DMS_DIR)/src/dms/UnitTest/.libs $(BIN_BAK_PATH)/apps/private/dlna_src/src/dms/UnitTest/$(TCPLATFORM)

endif

#Release ra_menu
ifneq ($(strip $(TCSUPPORT_RA_MENU)),)
	cp -rf $(APP_RA_MENU_DIR)/ra_menu $(BIN_BAK_PATH)/apps/binary/$(TCPLATFORM)/ra_menu
endif

#release cwmp
ifneq ($(strip $(TCSUPPORT_CWMP)),)
	mkdir -p $(BIN_BAK_PATH)/apps/private/TR69_64/$(TCPLATFORM)
	cp -f $(APP_CWMP_DIR)/cwmpXmlparser.o $(BIN_BAK_PATH)/apps/private/TR69_64/$(TCPLATFORM)
endif

#tcapi cwmp library
ifneq ($(strip $(TCSUPPORT_CWMP_FAST_GET)),)
	mkdir -p $(BIN_BAK_PATH)/apps/private/tcapi_lib_cwmp/$(TCPLATFORM)
	cp -rf $(APP_TCAPI_CWMP_LIB_DIR)/*.o  $(BIN_BAK_PATH)/apps/private/tcapi_lib_cwmp/$(TCPLATFORM)
	cp -rf $(APP_TCAPI_CWMP_LIB_DIR)/libtcapi_cwmp.so  $(BIN_BAK_PATH)/apps/private/tcapi_lib_cwmp/$(TCPLATFORM)
	cp -rf $(APP_TCAPI_CWMP_LIB_DIR)/libtcapi_cwmp.so.*  $(BIN_BAK_PATH)/apps/private/tcapi_lib_cwmp/$(TCPLATFORM)
endif

ifneq ($(strip $(JOYME_MULTI_SDK)),)
	# joyme_lib
	-rm -rf $(APP_JOYME_DIR)/$(TCPLATFORM)
	mkdir -p $(JOYME_ISNTALL_CODE_TMP)/$(APP_JOYME_DIR)
	cp $(APP_JOYME_DIR)/* $(JOYME_ISNTALL_CODE_TMP)/$(APP_JOYME_DIR)

	-rm -f $(APP_JOYME_DIR)/*.c

	mkdir -p $(JOYME_ISNTALL_CODE_TMP)/$(APP_BUNDLELIB_DIR)
	cp -rf $(APP_BUNDLELIB_DIR)/* $(JOYME_ISNTALL_CODE_TMP)/$(APP_BUNDLELIB_DIR)

	rm -f $(JOYME_ISNTALL_CODE_TMP)/$(APP_BUNDLELIB_DIR)/*.c 

	cp -f $(JOYME_ISNTALL_CODE_TMP)/$(APP_BUNDLELIB_DIR)/Makefile.release $(JOYME_ISNTALL_CODE_TMP)/$(APP_BUNDLELIB_DIR)/Makefile

	-rm -f $(LIB_DIR)/bundle.h $(LIB_DIR)/lanservicesjni.h $(LIB_DIR)/commservicesjni.h \
				$(LIB_DIR)/accessservicesjni.h $(LIB_DIR)/addressservicesjni.h $(LIB_DIR)/transferservicesjni.h $(LIB_DIR)/voipservicesjni.h 
	-rm -f $(PUBLIC_LIB_DIR)/bundle.h $(PUBLIC_LIB_DIR)/lanservicesjni.h $(PUBLIC_LIB_DIR)/commservicesjni.h \
				$(PUBLIC_LIB_DIR)/accessservicesjni.h $(PUBLIC_LIB_DIR)/addressservicesjni.h $(PUBLIC_LIB_DIR)/transferservicesjni.h $(PUBLIC_LIB_DIR)/voipservicesjni.h 

	mkdir -p $(JOYME_ISNTALL_CODE_TMP)/$(BUNDLE_STATE_DIR)
	cp -rf $(BUNDLE_STATE_DIR)/* $(JOYME_ISNTALL_CODE_TMP)/$(BUNDLE_STATE_DIR)
	rm -f $(JOYME_ISNTALL_CODE_TMP)/$(BUNDLE_STATE_DIR)/*.c 
endif

#release cfg_manager.o
#ifeq ($(strip $(TCSUPPORT_CT)),)
ifeq ($(strip $(TCSUPPORT_CFG_NG)),)
	mkdir -p $(BIN_BAK_PATH)/apps/private/cfg_manager/$(TCPLATFORM)
	cp -rf $(APP_CFG_MANAGER_DIR)/cfg_manager.o $(BIN_BAK_PATH)/apps/private/cfg_manager/$(TCPLATFORM)
else
	mkdir -p $(BIN_BAK_PATH)/apps/private/cfg_ng/service/cfg/$(TCPLATFORM)
	cp -rf $(APP_CFG_NG_DIR)/service/cfg/cfg_shm.o $(BIN_BAK_PATH)/apps/private/cfg_ng/service/cfg/$(TCPLATFORM)
	cp -rf $(APP_CFG_NG_DIR)/service/cfg/cfg_xml.o $(BIN_BAK_PATH)/apps/private/cfg_ng/service/cfg/$(TCPLATFORM)
endif	
	
#else
#	cp $(APP_PRIVATE_DIR)/cfg_manager_ct/cfg_manager  $(BIN_BAK_PATH)/apps/binary/$(TCPLATFORM)
#endif

release_webpage:
	echo "release webpage...."
#copy tc webpage by default
	mkdir -p $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/tc/
	cp -rf  $(APP_ROUTE_WEBPAGE_DIR)  $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/tc/

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_TTNET))
ifneq ($(strip $(TCSUPPORT_TTNET)),)
	mkdir -p $(APP_WEBPAGE_DIR).reserved/Router/ttnet/
	cp -rf  $(APP_WEBPAGE_DIR)/Router/ttnet/boaroot $(APP_WEBPAGE_DIR).reserved/Router/ttnet/
endif
#endif/*TCSUPPORT_COMPILE*/

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_TURKISH))
ifneq ($(strip $(TCSUPPORT_TURKISH)),)
	cp -rf  $(APP_WEBPAGE_DIR)/Router/ttnet/boaroot $(APP_WEBPAGE_DIR).reserved/Router/ttnet/
endif
#endif/*TCSUPPORT_COMPILE*/

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_C1_NEW_GUI)
ifneq ($(strip $(TCSUPPORT_C1_NEW_GUI)),)
	mkdir -p $(APP_WEBPAGE_DIR).reserved/Router/ZYXEL/
	cp -rf  $(APP_WEBPAGE_DIR)/Router/ZYXEL/boaroot $(APP_WEBPAGE_DIR).reserved/Router/ZYXEL/
endif
#endif/*TCSUPPORT_COMPILE*/

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CD_NEW_GUI)
ifneq ($(strip $(TCSUPPORT_CD_NEW_GUI)),)
	mkdir -p $(APP_WEBPAGE_DIR).reserved/Router/dlink/
	cp -rf  $(APP_WEBPAGE_DIR)/Router/dlink/boaroot $(APP_WEBPAGE_DIR).reserved/Router/dlink/
endif
#endif/*TCSUPPORT_COMPILE*/

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT)
ifneq ($(strip $(TCSUPPORT_CT)),)
ifneq ($(strip $(TCSUPPORT_CT_WAN_PTM) $(TCSUPPORT_CT_E8B_ADSL)),)
ifneq ($(strip $(TCSUPPORT_GENERAL_MULTILANGUAGE)),)
	mkdir -p $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/e8c_ml
	cp -rf $(APP_WEBPAGE_DIR)/Router/e8c_ml/boaroot $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/e8c_ml
else
	mkdir -p $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/e8b_vd/
	cp -rf  $(APP_WEBPAGE_DIR)/Router/e8b_vd/boaroot  $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/e8b_vd/
endif	
endif
ifneq ($(strip $(TCSUPPORT_CT_PON)),)
ifneq ($(strip $(TCSUPPORT_GENERAL_MULTILANGUAGE)),)
	mkdir -p $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/e8c_ml
	cp -rf $(APP_WEBPAGE_DIR)/Router/e8c_ml/boaroot $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/e8c_ml
else
ifneq ($(strip $(TCSUPPORT_CT_GUI_ENGLISH)),)
	mkdir -p $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/e8c_en/
	cp -rf  $(APP_WEBPAGE_DIR)/Router/e8c_en/boaroot  $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/e8c_en/
else
ifneq ($(strip $(TCSUPPORT_CT_JOYME4)),)
	mkdir -p $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/e8c_joyme4/
	cp -rf  $(APP_WEBPAGE_DIR)/Router/e8c_joyme4/boaroot  $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/e8c_joyme4/
ifneq ($(strip $(TCSUPPORT_CT_JOYME4_UI2)),)
	cp -rf  $(APP_WEBPAGE_DIR)/Router/e8c_joyme4/style2  $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/e8c_joyme4/
endif
ifneq ($(strip $(TCSUPPORT_CT_JOYME4_UI3)),)
	cp -rf  $(APP_WEBPAGE_DIR)/Router/e8c_joyme4/style3  $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/e8c_joyme4/
endif
else
	mkdir -p $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/e8c/
	cp -rf  $(APP_WEBPAGE_DIR)/Router/e8c/boaroot  $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/e8c/
endif
endif
endif
endif
ifneq ($(strip $(TCSUPPORT_CMCC)),)
	mkdir -p $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/cmcc/
	cp -rf  $(APP_WEBPAGE_DIR)/Router/cmcc/boaroot  $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/cmcc/
endif
ifneq ($(strip $(TCSUPPORT_CMCCV2) $(TCSUPPORT_NP_CMCC)),)
	mkdir -p $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/cmccv2/
	cp -rf  $(APP_WEBPAGE_DIR)/Router/cmccv2/boaroot  $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/cmccv2/
endif
ifneq ($(strip $(JOYME_MULTI_SDK)),)
	mkdir -p $(JOYME_ISNTALL_CODE_TMP)/$(APP_WEBPAGE_DIR)/Router/
	cp -rf $(APP_WEBPAGE_DIR)/Router/cmccv2 $(JOYME_ISNTALL_CODE_TMP)/$(APP_WEBPAGE_DIR)/Router/
endif
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_PON_GD)
ifneq ($(strip $(TCSUPPORT_CT_PON_GD)),)
	mkdir -p $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/e8c_pon_gd/
	cp -rf  $(APP_WEBPAGE_DIR)/Router/e8c_pon_gd/boaroot  $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/e8c_pon_gd/
endif
#endif/*TCSUPPORT_COMPILE*/

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CUC_GUI)
ifneq ($(strip $(TCSUPPORT_CUC_GUI)),)
	mkdir -p $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/cucv2/
	cp -rf  $(APP_WEBPAGE_DIR)/Router/cucv2/boaroot  $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/cucv2/
endif
#endif/*TCSUPPORT_COMPILE*/

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CUC_SFU)
ifneq ($(strip $(TCSUPPORT_CUC_SFU)),)
	mkdir -p $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/sfu/
	cp -rf  $(APP_WEBPAGE_DIR)/Router/sfu/boaroot  $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/sfu/
endif
#endif/*TCSUPPORT_COMPILE*/

	mkdir -p $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/ct/
	cp -rf  $(APP_WEBPAGE_DIR)/Router/ct/boaroot  $(BIN_BAK_PATH)/apps/private/webPage.reserved/Router/ct/
endif
#endif/*TCSUPPORT_COMPILE*/

#copy led conf file, this "interface is left for future use"
release_led_conf:
	echo "copy led conf..."
	mkdir -p $(BIN_BAK_PATH)/apps/private/led_conf.reserved	

#release_chk:
#	if test -d $(APP_PRIVATE_DIR)/TR69_64/clmp; \
#	then echo "Origin Src Code"; \
#	else echo "Release Src Code, you can not release second time!"; exit 1;\
#	fi

ifneq ($(strip $(TCSUPPORT_FON)),)
release_fon:
	mkdir -p $(BIN_BAK_PATH)/apps/public/fonap-4ffea20/libs/fonsmcd/$(TCPLATFORM)
	cp -rf $(APP_FONSMCD_DIR)/src/fonsmcd/fon_conf $(APP_FONSMCD_DIR)/libs/fonsmcd/
	cp $(APP_FONSMCD_DIR)/src/fonsmcd/fonsmcd $(BIN_BAK_PATH)/apps/public/fonap-4ffea20/libs/fonsmcd/$(TCPLATFORM)/

	cd $(APP_FONSMCD_DIR)/src/coova && ./bootstrap && chmod 777 configure
	cd $(APP_FONSMCD_DIR)/src/coova && ./configure --host=mips-linux --prefix=/usr --sysconfdir=/etc --localstatedir=/var --enable-ipwhitelist --enable-uamdomainfile --enable-redirdnsreq --enable-binstatusfile --disable-accounting-onoff CFLAGS="-Os -Wall -mips1"
	$(MAKE) -C $(APP_FONSMCD_DIR)/src/coova
	$(STRIP) --strip-unneeded -R .comment $(APP_FONSMCD_DIR)/src/coova/src/.libs/chilli
	$(STRIP) --strip-unneeded -R .comment $(APP_FONSMCD_DIR)/src/coova/src/.libs/chilli_opt
	$(STRIP) --strip-unneeded -R .comment $(APP_FONSMCD_DIR)/src/coova/src/.libs/chilli_query
	$(STRIP) --strip-unneeded -R .comment $(APP_FONSMCD_DIR)/src/coova/src/.libs/chilli_radconfig
	$(STRIP) --strip-unneeded -R .comment $(APP_FONSMCD_DIR)/src/coova/src/.libs/chilli_response
	$(STRIP) --strip-unneeded -R .comment $(APP_FONSMCD_DIR)/src/coova/src/.libs/libchilli.so*
	$(STRIP) --strip-unneeded -R .comment $(APP_FONSMCD_DIR)/src/coova/bstring/.libs/libbstring.so*
	mkdir -p $(BIN_BAK_PATH)/apps/public/fonap-4ffea20/libs/coova/$(TCPLATFORM)/model_a/
	cp $(APP_FONSMCD_DIR)/src/coova/src/.libs/chilli $(BIN_BAK_PATH)/apps/public/fonap-4ffea20/libs/coova/$(TCPLATFORM)/model_a/
	cp $(APP_FONSMCD_DIR)/src/coova/src/.libs/chilli_opt $(BIN_BAK_PATH)/apps/public/fonap-4ffea20/libs/coova/$(TCPLATFORM)/model_a/
	cp $(APP_FONSMCD_DIR)/src/coova/src/.libs/chilli_query $(BIN_BAK_PATH)/apps/public/fonap-4ffea20/libs/coova/$(TCPLATFORM)/model_a/
	cp $(APP_FONSMCD_DIR)/src/coova/src/.libs/chilli_radconfig $(BIN_BAK_PATH)/apps/public/fonap-4ffea20/libs/coova/$(TCPLATFORM)/model_a/
	cp $(APP_FONSMCD_DIR)/src/coova/src/.libs/chilli_response $(BIN_BAK_PATH)/apps/public/fonap-4ffea20/libs/coova/$(TCPLATFORM)/model_a/
	cp $(APP_FONSMCD_DIR)/src/coova/src/.libs/libchilli.so* $(BIN_BAK_PATH)/apps/public/fonap-4ffea20/libs/coova/$(TCPLATFORM)/model_a/
	cp $(APP_FONSMCD_DIR)/src/coova/bstring/.libs/libbstring.so* $(BIN_BAK_PATH)/apps/public/fonap-4ffea20/libs/coova/$(TCPLATFORM)/
	mkdir -p $(APP_FONSMCD_DIR)/libs/coova/files_fon/
	cp $(APP_FONSMCD_DIR)/src/coova/files_fon/localusers $(APP_FONSMCD_DIR)/libs/coova/files_fon/

	cd $(APP_FONSMCD_DIR)/src/fon-api && ./bootstrap && chmod 777 configure
	cd $(APP_FONSMCD_DIR)/src/fon-api && ./configure --host=mips-linux CFLAGS="-Os -Wall -mips1"
	$(MAKE) -C $(APP_FONSMCD_DIR)/src/fon-api
	$(STRIP) --strip-unneeded -R .comment $(APP_FONSMCD_DIR)/src/fon-api/.libs/fonctl
	$(STRIP) --strip-unneeded -R .comment $(APP_FONSMCD_DIR)/src/fon-api/.libs/fon-api.so
	mkdir -p $(BIN_BAK_PATH)/apps/public/fonap-4ffea20/libs/fon-api/$(TCPLATFORM)/model_a
	cp $(APP_FONSMCD_DIR)/src/fon-api/.libs/fonctl $(BIN_BAK_PATH)/apps/public/fonap-4ffea20/libs/fon-api/$(TCPLATFORM)/model_a/
	cp $(APP_FONSMCD_DIR)/src/fon-api/.libs/fon-api.so $(BIN_BAK_PATH)/apps/public/fonap-4ffea20/libs/fon-api/$(TCPLATFORM)/model_a/
	$(MAKE) -C $(APP_FONSMCD_DIR)/src/coova clean
	$(MAKE) -C $(APP_FONSMCD_DIR)/src/fon-api clean
			
ifneq ($(strip $(TCSUPPORT_FON_MODEL_B)),)	
	cd $(APP_FONSMCD_DIR)/src/coova && ./bootstrap && chmod 777 configure
	cd $(APP_FONSMCD_DIR)/src/coova && ./configure --host=mips-linux --prefix=/usr --sysconfdir=/etc --localstatedir=/var --enable-ipwhitelist --enable-uamdomainfile --enable-redirdnsreq --enable-binstatusfile --disable-accounting-onoff CFLAGS="-DTCSUPPORT_FON_MODEL_B -Os -Wall -mips1" LDFLAGS="-lrt $(TRUNK_DIR)/apps/private/lib/libtcapi.so"
	$(MAKE) -C $(APP_FONSMCD_DIR)/src/coova
	$(STRIP) --strip-unneeded -R .comment $(APP_FONSMCD_DIR)/src/coova/src/.libs/chilli
	$(STRIP) --strip-unneeded -R .comment $(APP_FONSMCD_DIR)/src/coova/src/.libs/chilli_opt
	$(STRIP) --strip-unneeded -R .comment $(APP_FONSMCD_DIR)/src/coova/src/.libs/chilli_query
	$(STRIP) --strip-unneeded -R .comment $(APP_FONSMCD_DIR)/src/coova/src/.libs/chilli_radconfig
	$(STRIP) --strip-unneeded -R .comment $(APP_FONSMCD_DIR)/src/coova/src/.libs/chilli_response
	$(STRIP) --strip-unneeded -R .comment $(APP_FONSMCD_DIR)/src/coova/src/.libs/libchilli.so*
	$(STRIP) --strip-unneeded -R .comment $(APP_FONSMCD_DIR)/src/coova/bstring/.libs/libbstring.so*
	mkdir -p $(BIN_BAK_PATH)/apps/public/fonap-4ffea20/libs/coova/$(TCPLATFORM)/model_b/
	cp $(APP_FONSMCD_DIR)/src/coova/src/.libs/chilli $(BIN_BAK_PATH)/apps/public/fonap-4ffea20/libs/coova/$(TCPLATFORM)/model_b/
	cp $(APP_FONSMCD_DIR)/src/coova/src/.libs/chilli_opt $(BIN_BAK_PATH)/apps/public/fonap-4ffea20/libs/coova/$(TCPLATFORM)/model_b/
	cp $(APP_FONSMCD_DIR)/src/coova/src/.libs/chilli_query $(BIN_BAK_PATH)/apps/public/fonap-4ffea20/libs/coova/$(TCPLATFORM)/model_b/
	cp $(APP_FONSMCD_DIR)/src/coova/src/.libs/chilli_radconfig $(BIN_BAK_PATH)/apps/public/fonap-4ffea20/libs/coova/$(TCPLATFORM)/model_b/
	cp $(APP_FONSMCD_DIR)/src/coova/src/.libs/chilli_response $(BIN_BAK_PATH)/apps/public/fonap-4ffea20/libs/coova/$(TCPLATFORM)/model_b/
	cp $(APP_FONSMCD_DIR)/src/coova/src/.libs/libchilli.so* $(BIN_BAK_PATH)/apps/public/fonap-4ffea20/libs/coova/$(TCPLATFORM)/model_b/
	
	cd $(APP_FONSMCD_DIR)/src/fon-api && ./bootstrap && chmod 777 configure
	cd $(APP_FONSMCD_DIR)/src/fon-api && ./configure --host=mips-linux CFLAGS="-Os -Wall -mips1" LDFLAGS="-lrt $(TRUNK_DIR)/apps/private/lib/libtcapi.so" 
	$(MAKE) -C $(APP_FONSMCD_DIR)/src/fon-api
	$(STRIP) --strip-unneeded -R .comment $(APP_FONSMCD_DIR)/src/fon-api/.libs/fonctl
	$(STRIP) --strip-unneeded -R .comment $(APP_FONSMCD_DIR)/src/fon-api/.libs/fon-api.so
	mkdir -p $(BIN_BAK_PATH)/apps/public/fonap-4ffea20/libs/fon-api/$(TCPLATFORM)/model_b
	cp $(APP_FONSMCD_DIR)/src/fon-api/.libs/fonctl $(BIN_BAK_PATH)/apps/public/fonap-4ffea20/libs/fon-api/$(TCPLATFORM)/model_b/
	cp $(APP_FONSMCD_DIR)/src/fon-api/.libs/fon-api.so $(BIN_BAK_PATH)/apps/public/fonap-4ffea20/libs/fon-api/$(TCPLATFORM)/model_b/
	$(MAKE) -C $(APP_FONSMCD_DIR)/src/coova clean
	$(MAKE) -C $(APP_FONSMCD_DIR)/src/fon-api clean	
endif
endif

ifneq ($(strip $(TCSUPPORT_FON_V2)),)
release_fonV2:
	echo "Build hotspotd"
	mkdir -p $(BIN_BAK_PATH)/apps/public/hotspot/$(TCPLATFORM)
	$(MAKE) -C $(APP_FON_HOTSPOT_DIR) MODULES="fonsmc fonapi radius radconf fonctl tunnel" LDFLAGS="-lrt $(TRUNK_DIR)/apps/private/lib/libtcapi.so -lrt $(TRUNK_DIR)/apps/public/pcre-8.32/.libs/libpcre.so" 
	if test -e $(APP_FON_HOTSPOT_DIR)/hotspotd; \
	then \
		$(STRIP) --strip-unneeded -R .comment $(APP_FON_HOTSPOT_DIR)/hotspotd; \
		cp -rf $(APP_FON_HOTSPOT_DIR)/hotspotd $(BIN_BAK_PATH)/apps/public/hotspot/$(TCPLATFORM); \
	fi
	
	cp -rf $(APP_FON_HOTSPOT_DIR)/conf/ $(APP_PUBLIC_DIR)/hotspot
	$(MAKE) -C $(APP_FON_HOTSPOT_DIR) clean
endif

ifneq ($(strip $(TCSUPPORT_ECNT_MAP)),)
#release wifi map 
#ifneq ($(strip $(TCSUPPORT_ECNT_MAP_ENHANCE)),)
	#mkdir -p $(BIN_BAK_PATH)/apps/private/map_enhance/$(TCPLATFORM)/wappd/config_and_icon_files
#	mkdir -p $(BIN_BAK_PATH)/apps/private/map_enhance/$(TCPLATFORM)/1905daemon
#	mkdir -p $(BIN_BAK_PATH)/apps/private/map_enhance/$(TCPLATFORM)/libmapd/include
#	mkdir -p $(BIN_BAK_PATH)/apps/private/map_enhance/$(TCPLATFORM)/mapd	
	#mkdir -p $(BIN_BAK_PATH)/apps/private/map_enhance/$(TCPLATFORM)/wappd/config_and_icon_files
	
#	cp $(APP_MAP_LIBMAPD_DIR)/libmapd_interface_client.so $(BIN_BAK_PATH)/apps/private/map_enhance/$(TCPLATFORM)/libmapd/
#	cp $(APP_MAP_LIBMAPD_DIR)/include/mapd_interface_ctrl.h $(BIN_BAK_PATH)/apps/private/map_enhance/$(TCPLATFORM)/libmapd/include/
#	cp $(APP_MAP_LIBMAPD_DIR)/include/mapd_interface_cust.h $(BIN_BAK_PATH)/apps/private/map_enhance/$(TCPLATFORM)/libmapd/include/
	#cp $(APP_MAP_WAPPD_DIR)/libwapp_usr_intf_client.a $(BIN_BAK_PATH)/apps/private/map_enhance/$(TCPLATFORM)/wappd/
	#cp $(APP_MAP_WAPPD_DIR)/config_and_icon_files/* $(BIN_BAK_PATH)/apps/private/map_enhance/$(TCPLATFORM)/wappd/config_and_icon_files/
	#cp $(APP_MAP_WAPPD_DIR)/wapp $(BIN_BAK_PATH)/apps/private/map_enhance/$(TCPLATFORM)/wappd/
	#cp $(APP_MAP_WAPPD_DIR)/wappctrl $(BIN_BAK_PATH)/apps/private/map_enhance/$(TCPLATFORM)/wappd/
	#cp $(APP_MAP_WAPPD_DIR)/config_and_icon_files/* $(BIN_BAK_PATH)/apps/private/map_enhance/$(TCPLATFORM)/wappd/config_and_icon_files/
#	cp $(APP_MAP_1905DAEMON_DIR)/p1905_managerd $(BIN_BAK_PATH)/apps/private/map_enhance/$(TCPLATFORM)/1905daemon/
#	cp $(APP_MAP_1905DAEMON_DIR)/1905ctrl $(BIN_BAK_PATH)/apps/private/map_enhance/$(TCPLATFORM)/1905daemon/
#	cp $(APP_MAP_MAPD_DIR)/mapd $(BIN_BAK_PATH)/apps/private/map_enhance/$(TCPLATFORM)/mapd/
#	cp $(APP_MAP_MAPD_DIR)/mapd_cli $(BIN_BAK_PATH)/apps/private/map_enhance/$(TCPLATFORM)/mapd/
#else
#	mkdir -p $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/hal_map
#	mkdir -p $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/wappd
#	mkdir -p $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/1905daemon
#	mkdir -p $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/libmapd/include
#	mkdir -p $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/mapd	
	
#	cp $(APP_MAP_HAL_MAP_DIR)/libmap_hal.so $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/hal_map/
#	cp $(APP_MAP_LIBMAPD_DIR)/libmapd_interface_client.so $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/libmapd/
#	cp $(APP_MAP_LIBMAPD_DIR)/include/mapd_interface_ctrl.h $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/libmapd/include/
#	cp $(APP_MAP_WAPPD_DIR)/libwapp_usr_intf_client.a $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/wappd/
#	cp $(APP_MAP_WAPPD_DIR)/wapp $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/wappd/
#	cp $(APP_MAP_WAPPD_DIR)/wappctrl $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/wappd/
#	cp $(APP_MAP_1905DAEMON_DIR)/p1905_managerd $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/1905daemon/
#	cp $(APP_MAP_1905DAEMON_DIR)/1905ctrl $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/1905daemon/
#	cp $(APP_MAP_MAPD_DIR)/mapd $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/mapd/
#	cp $(APP_MAP_MAPD_DIR)/mapd_cli $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/mapd/
#endif

ifneq ($(strip $(TCSUPPORT_EASYMESH_R13)),)
	#mkdir -p $(BIN_BAK_PATH)/apps/private/map_v2/$(TCPLATFORM)/wappd/config_and_icon_files
	mkdir -p $(BIN_BAK_PATH)/apps/private/map_v2/$(TCPLATFORM)/1905daemon
	mkdir -p $(BIN_BAK_PATH)/apps/private/map_v2/$(TCPLATFORM)/1905daemon/ethernet
	mkdir -p $(BIN_BAK_PATH)/apps/private/map_v2/$(TCPLATFORM)/libmapd/include
	mkdir -p $(BIN_BAK_PATH)/apps/private/map_v2/$(TCPLATFORM)/mapd	
	#mkdir -p $(BIN_BAK_PATH)/apps/private/map_v2/$(TCPLATFORM)/wappd/config_and_icon_files
	
	cp $(APP_MAP_LIBMAPD_DIR)/libmapd_interface_client.so $(BIN_BAK_PATH)/apps/private/map_v2/$(TCPLATFORM)/libmapd/
	cp $(APP_MAP_LIBMAPD_DIR)/include/mapd_interface_ctrl.h $(BIN_BAK_PATH)/apps/private/map_v2/$(TCPLATFORM)/libmapd/include/
#	cp $(APP_MAP_LIBMAPD_DIR)/include/mapd_interface_cust.h $(BIN_BAK_PATH)/apps/private/map_v2/$(TCPLATFORM)/libmapd/include/
	#cp $(APP_MAP_WAPPD_DIR)/libwapp_usr_intf_client.a $(BIN_BAK_PATH)/apps/private/map_v2/$(TCPLATFORM)/wappd/
	#cp $(APP_MAP_WAPPD_DIR)/config_and_icon_files/* $(BIN_BAK_PATH)/apps/private/map_v2/$(TCPLATFORM)/wappd/config_and_icon_files/
	#cp $(APP_MAP_WAPPD_DIR)/wapp $(BIN_BAK_PATH)/apps/private/map_v2/$(TCPLATFORM)/wappd/
	#cp $(APP_MAP_WAPPD_DIR)/wappctrl $(BIN_BAK_PATH)/apps/private/map_v2/$(TCPLATFORM)/wappd/
	#cp $(APP_MAP_WAPPD_DIR)/config_and_icon_files/* $(BIN_BAK_PATH)/apps/private/map_v2/$(TCPLATFORM)/wappd/config_and_icon_files/
	cp $(APP_MAP_1905DAEMON_DIR)/p1905_managerd $(BIN_BAK_PATH)/apps/private/map_v2/$(TCPLATFORM)/1905daemon/
	cp $(APP_MAP_1905DAEMON_DIR)/1905ctrl $(BIN_BAK_PATH)/apps/private/map_v2/$(TCPLATFORM)/1905daemon/
	cp $(APP_MAP_1905DAEMON_DIR)/ethernet/libeth_1905ops.so $(BIN_BAK_PATH)/apps/private/map_v2/$(TCPLATFORM)/1905daemon/ethernet/	
	cp $(APP_MAP_MAPD_DIR)/mapd $(BIN_BAK_PATH)/apps/private/map_v2/$(TCPLATFORM)/mapd/
	cp $(APP_MAP_MAPD_DIR)/mapd_cli $(BIN_BAK_PATH)/apps/private/map_v2/$(TCPLATFORM)/mapd/
	cp $(APP_MAP_MAPD_DIR)/bs20 $(BIN_BAK_PATH)/apps/private/map_v2/$(TCPLATFORM)/mapd/
else
	mkdir -p $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/hal_map
	mkdir -p $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/wappd
	mkdir -p $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/1905daemon
	mkdir -p $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/libmapd/include
	mkdir -p $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/mapd	
	
	cp $(APP_MAP_HAL_MAP_DIR)/libmap_hal.so $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/hal_map/
	cp $(APP_MAP_LIBMAPD_DIR)/libmapd_interface_client.so $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/libmapd/
	cp $(APP_MAP_LIBMAPD_DIR)/include/mapd_interface_ctrl.h $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/libmapd/include/
	cp $(APP_MAP_WAPPD_DIR)/libwapp_usr_intf_client.a $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/wappd/
	cp $(APP_MAP_WAPPD_DIR)/wapp $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/wappd/
	cp $(APP_MAP_WAPPD_DIR)/wappctrl $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/wappd/
	cp $(APP_MAP_1905DAEMON_DIR)/p1905_managerd $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/1905daemon/
	cp $(APP_MAP_1905DAEMON_DIR)/1905ctrl $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/1905daemon/
	cp $(APP_MAP_MAPD_DIR)/mapd $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/mapd/
	cp $(APP_MAP_MAPD_DIR)/mapd_cli $(BIN_BAK_PATH)/apps/private/map/$(TCPLATFORM)/mapd/
endif
endif