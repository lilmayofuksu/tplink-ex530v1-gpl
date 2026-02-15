release_makefile_bsp:
#copy makefiles
	cp $(MODULES_PRIV_SRC_DIR)/tc3162l2hp2h/Makefile.release $(MODULES_PRIV_SRC_DIR)/tc3162l2hp2h/Makefile
ifeq ($(strip $(TCSUPPORT_UBOOT))),)
	cp $(BOOTROM_DIR)/ddr_cal/Makefile.release $(BOOTROM_DIR)/ddr_cal/Makefile
	cp $(BOOTROM_DIR)/ddr_cal_mt7505/Makefile.release $(BOOTROM_DIR)/ddr_cal_mt7505/Makefile
	cp $(BOOTROM_DIR)/ddr_cal_en7512/Makefile.release $(BOOTROM_DIR)/ddr_cal_en7512/Makefile
	cp $(BOOTROM_DIR)/ram_init/Makefile.release $(BOOTROM_DIR)/ram_init/Makefile
endif
#########################################################################################
ifneq ($(strip $(TCSUPPORT_NP)),)
release_bsp: release_bootrom release_drivers release_app_bsp 
else
release_bsp: release_bootrom release_drivers release_app_bsp release_xpon
endif
	

release_bootrom:
#ifneq ($(strip $(TCSUPPORT_CPU_MT7510)),)
ifneq ($(strip $(TCSUPPORT_UBOOT))),)
	echo "nothing to do!";
else
ifneq ($(strip $(TCSUPPORT_CPU_EN7580)),)
	@echo "release Bootrom for EN7580"
	mkdir -p $(BIN_BAK_PATH)/bootrom/unopen_img/$(TCPLATFORM)/ram_init/output
	cp -Rf $(BOOTROM_DIR)/ram_init/output/* $(BIN_BAK_PATH)/bootrom/unopen_img/$(TCPLATFORM)/ram_init/output/
	cp -Rf $(BOOTROM_DIR)/ram_init/spram.c $(BIN_BAK_PATH)/bootrom/unopen_img/$(TCPLATFORM)/ram_init/
	cp -Rf $(BOOTROM_DIR)/spram_ext/system.o $(BIN_BAK_PATH)/bootrom/unopen_img/$(TCPLATFORM)/ram_init/output/
else
ifneq ($(strip $(TCSUPPORT_CPU_EN7512) $(TCSUPPORT_CPU_EN7521)),)
	@echo "release Bootrom for MT751221"
	mkdir -p $(BIN_BAK_PATH)/bootrom/unopen_img/$(TCPLATFORM)/ddr_cal_en7512/output
	cp -Rf $(BOOTROM_DIR)/ddr_cal_en7512/output/* $(BIN_BAK_PATH)/bootrom/unopen_img/$(TCPLATFORM)/ddr_cal_en7512/output/
	cp -Rf $(BOOTROM_DIR)/ddr_cal_en7512/spram.c $(BIN_BAK_PATH)/bootrom/unopen_img/$(TCPLATFORM)/ddr_cal_en7512/
	cp -Rf $(BOOTROM_DIR)/spram_ext/system.o $(BIN_BAK_PATH)/bootrom/unopen_img/$(TCPLATFORM)/ddr_cal_en7512/output/
else
ifneq ($(strip $(TCSUPPORT_CPU_MT7510) $(TCSUPPORT_CPU_MT7520)),)
	@echo "release Bootrom for MT751020"
	mkdir -p $(BOOTROM_DIR)/ddr_cal/reserved
	cp -Rf $(BOOTROM_DIR)/ddr_cal/output/* $(BOOTROM_DIR)/ddr_cal/reserved/
	cp -Rf $(BOOTROM_DIR)/ddr_cal/spram.c $(BOOTROM_DIR)/ddr_cal/reserved/
else
ifneq ($(strip $(TCSUPPORT_CPU_MT7505)),)
	@echo "release Bootrom for MT7505"
	mkdir -p $(BOOTROM_DIR)/ddr_cal_mt7505/reserved
	cp -Rf $(BOOTROM_DIR)/ddr_cal_mt7505/output/* $(BOOTROM_DIR)/ddr_cal_mt7505/reserved/
	cp -Rf $(BOOTROM_DIR)/ddr_cal_mt7505/spram.c $(BOOTROM_DIR)/ddr_cal_mt7505/reserved/
else
	echo "nothing to do!";
endif
endif
endif
endif
endif
release_drivers:
	mkdir -p $(BIN_BAK_PATH)/modules/private/ko/modules/$(TCPLATFORM)
	mkdir -p $(BIN_BAK_PATH)/modules/private/ko/modules/$(TCPLATFORM)/inc
ifneq ($(strip $(TCSUPPORT_HWNAT_V3)),)	
	cp $(MODULES_RA_HWNAT_V3_DIR)/hwnat_ioctl.h $(BIN_BAK_PATH)/modules/private/ko/modules/$(TCPLATFORM)/inc/
else
	cp $(MODULES_RA_HWNAT_7510_DIR)/hwnat_ioctl.h $(BIN_BAK_PATH)/modules/private/ko/modules/$(TCPLATFORM)/inc/
endif
	cp -rf $(BSP_EXT_MODULE)/* $(BIN_BAK_PATH)/modules/private/ko/modules/$(TCPLATFORM)/
#	rm -rf $(MODULES_PRIV_SRC_DIR)/tcci/version.c
	mkdir -p $(BIN_BAK_PATH)/modules/private/obj/$(TCPLATFORM)
ifneq ($(strip $(TCSUPPORT_SECURE_BOOT)),)
	cp -rf $(MODULES_PRIV_SRC_DIR)/secure_upgrade/secure_upgrade.o $(BIN_BAK_PATH)/modules/private/obj/$(TCPLATFORM)
endif
	cp -rf $(MODULES_PRIV_SRC_DIR)/tcci/version.o $(BIN_BAK_PATH)/modules/private/obj/$(TCPLATFORM)/
ifneq ($(strip $(TCSUPPORT_HW_CRYPTO)),)
ifeq ($(strip $(TCSUPPORT_2_6_36_KERNEL)),)
	cp -rf $(MODULES_CRYPTO_DRIVER)/source/utils/*.o $(BIN_BAK_PATH)/modules/private/obj/$(TCPLATFORM)/
	cp -rf $(MODULES_CRYPTO_DRIVER)/source/mtk_eip93Init.o $(BIN_BAK_PATH)/modules/private/obj/$(TCPLATFORM)/
	cp -rf $(MODULES_CRYPTO_DRIVER)/source/mtk_init.o $(BIN_BAK_PATH)/modules/private/obj/$(TCPLATFORM)/
	cp -rf $(MODULES_CRYPTO_DRIVER)/source/mtk_interruptHelper.o $(BIN_BAK_PATH)/modules/private/obj/$(TCPLATFORM)/
	cp -rf $(MODULES_CRYPTO_DRIVER)/source/mtk_pecInit.o $(BIN_BAK_PATH)/modules/private/obj/$(TCPLATFORM)/
	cp -rf $(MODULES_CRYPTO_DRIVER)/source/mtk_pktProcess_k318.o $(BIN_BAK_PATH)/modules/private/obj/$(TCPLATFORM)/
endif
endif
ifneq ($(strip $(TCSUPPORT_IFC_EN)),)
	cp -rf $(MODULES_IFC_DIR)/ifc_api.o $(BIN_BAK_PATH)/modules/private/obj/$(TCPLATFORM)/
	cp -rf $(MODULES_IFC_DIR)/ifc_dev.o $(BIN_BAK_PATH)/modules/private/obj/$(TCPLATFORM)/
	cp -rf $(MODULES_IFC_DIR)/ifc_util.o $(BIN_BAK_PATH)/modules/private/obj/$(TCPLATFORM)/
endif
ifeq ($(strip $(TCSUPPORT_NP)),)
	cp -rf $(MODULES_PRIV_SRC_DIR)/speedtest/speed_test_unopen.o $(BIN_BAK_PATH)/modules/private/obj/$(TCPLATFORM)/
endif
ifneq ($(strip $(TCSUPPORT_DS_HWNAT_OFFLOAD)),)
ifneq ($(strip $(TCSUPPORT_CPU_EN7512) $(TCSUPPORT_CPU_EN7521) $(TCSUPPORT_CPU_EN7516) $(TCSUPPORT_CPU_EN7527)),)
	cp -rf $(MODULES_ETHER_DIR)/soft_qdma/soft_qdma.o $(BIN_BAK_PATH)/modules/private/obj/$(TCPLATFORM)/
else	
	cp -rf $(MODULES_RAETH_DIR)/soft_qdma/soft_qdma.o $(BIN_BAK_PATH)/modules/private/obj/$(TCPLATFORM)/
endif
endif
ifneq ($(strip $(TCSUPPORT_FORWARD_LEFT_TO_RIGHT)),)
	cp -rf $(MODULES_PRIV_SRC_DIR)/forward_left_to_right_7516/qdma_forward_left_to_right.o $(BIN_BAK_PATH)/modules/private/obj/$(TCPLATFORM)/
	rm -rf $(MODULES_PRIV_SRC_DIR)/forward_left_to_right_7516/
endif
ifneq ($(strip $(TCSUPPORT_CMCCV2)),)
	-rm -f $(BIN_BAK_PATH)/modules/private/ko/modules/$(TCPLATFORM)/tcfullcone.ko
	-rm -f $(BIN_BAK_PATH)/modules/private/ko/modules/$(TCPLATFORM)/maxnetdpi.ko
endif
ifneq ($(strip $(JOYME_MULTI_SDK)),)
	# drivers
	mkdir -p $(JOYME_ISNTALL_CODE_TMP)/$(BIN_BAK_PATH)/modules/private/ko/modules/$(TCPLATFORM)
	cp $(BIN_BAK_PATH)/modules/private/ko/modules/$(TCPLATFORM)/wlanshare.ko $(JOYME_ISNTALL_CODE_TMP)/$(BIN_BAK_PATH)/modules/private/ko/modules/$(TCPLATFORM)
endif

release_app_bsp:
	echo "release app bsp"
	
ifeq ("$(IS_APPS_DIR_EXIST)", "N")
	mkdir -p $(BIN_BAK_PATH)/app_bsp/bsp_exclusive/binary/$(TCPLATFORM)
	#cp && delete smuxctl
ifneq ($(strip $(TCSUPPORT_SMUX)),)
	cp -rf $(APP_BSP_SMUX_DIR)/smuxctl $(BIN_BAK_PATH)/app_bsp/bsp_exclusive/binary/$(TCPLATFORM)
endif
endif

ifneq ($(strip $(TCSUPPORT_CT_CUSTOMCODE)),)
	mkdir -p $(OPEN_CODE) && mkdir -p $(OPEN_CODE_TMP)
# modules
	mkdir -p $(OPEN_CODE_TMP)/$(MODULES_ACCESSLIMIT_DIR)
	cp $(MODULES_ACCESSLIMIT_DIR)/* $(OPEN_CODE_TMP)/$(MODULES_ACCESSLIMIT_DIR)
	mkdir -p $(OPEN_CODE_TMP)/$(MODULES_URL_FILTER_DIR)
	cp $(MODULES_URL_FILTER_DIR)/* $(OPEN_CODE_TMP)/$(MODULES_URL_FILTER_DIR)
	mkdir -p $(OPEN_CODE_TMP)/$(MODULES_RA_HWNAT_7510_DIR)
	cp $(MODULES_RA_HWNAT_7510_DIR)/* $(OPEN_CODE_TMP)/$(MODULES_RA_HWNAT_7510_DIR)
	mkdir -p $(OPEN_CODE_TMP)/$(MODULES_RAETH_DIR)
	cp $(MODULES_RAETH_DIR)/* $(OPEN_CODE_TMP)/$(MODULES_RAETH_DIR)
	mkdir -p $(OPEN_CODE_TMP)/$(MODULES_CT_VLAN_TAG_DIR)
	cp $(MODULES_CT_VLAN_TAG_DIR)/* $(OPEN_CODE_TMP)/$(MODULES_CT_VLAN_TAG_DIR)

	mkdir -p $(OPEN_CODE_TMP)/$(MODULES_IGMPSNOOP_DIR)
	cp $(MODULES_IGMPSNOOP_DIR)/* $(OPEN_CODE_TMP)/$(MODULES_IGMPSNOOP_DIR)
	mkdir -p $(OPEN_CODE_TMP)/$(MODULES_SWQOS_DIR)
	cp $(MODULES_SWQOS_DIR)/* $(OPEN_CODE_TMP)/$(MODULES_SWQOS_DIR)

	mkdir -p $(OPEN_CODE_TMP)/$(MODULES_LOOP_DETECT_DIR)
	cp $(MODULES_LOOP_DETECT_DIR)/* $(OPEN_CODE_TMP)/$(MODULES_LOOP_DETECT_DIR)
	mkdir -p $(OPEN_CODE_TMP)/$(MODULES_PRIV_SRC_DIR)/tcci
	cp $(MODULES_PRIV_SRC_DIR)/tcci/* $(OPEN_CODE_TMP)/$(MODULES_PRIV_SRC_DIR)/tcci
	
	mkdir -p $(OPEN_CODE_TMP)/$(MODULES_PORTBIND_DIR)
	cp $(MODULES_PORTBIND_DIR)/* $(OPEN_CODE_TMP)/$(MODULES_PORTBIND_DIR)	
	
	mkdir -p $(OPEN_CODE_TMP)/$(MODULES_FE_7512_DIR)
	cp $(MODULES_FE_7512_DIR)/* $(OPEN_CODE_TMP)/$(MODULES_FE_7512_DIR)

	mkdir -p $(OPEN_CODE_TMP)/$(MODULES_QDMA_7512_DIR)
	cp $(MODULES_QDMA_7512_DIR)/* $(OPEN_CODE_TMP)/$(MODULES_QDMA_7512_DIR)

	mkdir -p $(OPEN_CODE_TMP)/$(MODULES_ETHER_DIR)
	cp -a $(MODULES_ETHER_DIR)/* $(OPEN_CODE_TMP)/$(MODULES_ETHER_DIR)
endif

	mkdir -p $(BIN_BAK_PATH)/app_bsp/binary/$(TCPLATFORM)
	cp -rf $(BSP_EXT_FS_USEFS)/cputemp  $(BIN_BAK_PATH)/app_bsp/binary/$(TCPLATFORM)

ifneq ($(strip $(TCSUPPORT_ECNT_MAP)),)
#release wifi map wappd
#ifneq ($(strip $(TCSUPPORT_ECNT_MAP_ENHANCE)),)
ifneq ($(strip $(TCSUPPORT_EASYMESH_R13)),)
	mkdir -p $(BIN_BAK_PATH)/app_bsp/binary/$(TCPLATFORM)/wappd/config_and_icon_files
	cp $(BSP_API_MAP_WAPPD_DIR)/libwapp_usr_intf_client.a $(BIN_BAK_PATH)/app_bsp/binary/$(TCPLATFORM)/wappd/
	cp $(BSP_API_MAP_WAPPD_DIR)/config_and_icon_files/* $(BIN_BAK_PATH)/app_bsp/binary/$(TCPLATFORM)/wappd/config_and_icon_files/
	cp $(BSP_API_MAP_WAPPD_DIR)/wapp $(BIN_BAK_PATH)/app_bsp/binary/$(TCPLATFORM)/wappd/
	cp $(BSP_API_MAP_WAPPD_DIR)/wappctrl $(BIN_BAK_PATH)/app_bsp/binary/$(TCPLATFORM)/wappd/
endif
endif
	
#Release tcwdog	
	cp -rf $(BSP_EXT_FS_USR)/tcwdog  $(BIN_BAK_PATH)/app_bsp/binary/$(TCPLATFORM)

#Release smuxctl
ifneq ($(strip $(TCSUPPORT_SMUX)),)
	cp -rf $(BSP_EXT_FS_USR)/smuxctl  $(BIN_BAK_PATH)/app_bsp/binary/$(TCPLATFORM)
endif

#Release hw_nat
ifneq ($(strip $(TCSUPPORT_RA_HWNAT)),)
#ifeq ($(strip $(TCSUPPORT_MT7510_FE)),)
#	cp -rf $(APP_RA_HWNAT_DIR)/ac  $(BIN_BAK_PATH)/apps/binary/$(TCPLATFORM)
#	cp -rf $(APP_RA_HWNAT_DIR)/acl  $(BIN_BAK_PATH)/apps/binary/$(TCPLATFORM)
#	cp -rf $(APP_RA_HWNAT_DIR)/mtr  $(BIN_BAK_PATH)/apps/binary/$(TCPLATFORM)
#	cp -rf $(APP_RA_HWNAT_DIR)/hw_nat  $(BIN_BAK_PATH)/apps/binary/$(TCPLATFORM)
#else
	cp -rf $(BSP_EXT_FS_USEFS)/hw_nat  $(BIN_BAK_PATH)/app_bsp/binary/$(TCPLATFORM)
#endif
endif
	
release_chk:
	if test -d $(BIN_BAK_PATH)/modules/private/ko/modules/$(TCPLATFORM); \
	then echo "Release Src Code, you can not release second time!"; exit 1;\
	else echo "Origin Src Code"; \
	fi
