
#	@echo install files:{$(2)} in $(1) to $(3)
define AppsBinInstall
	$(foreach fname,$(strip $(2)), cp -rf $(1)/$(fname) $(3);)
endef

define AppBinClean
	@echo AppBinClean
	-cd $(2) && $(foreach fname, $(strip $(1)), find ./ -name '$(fname)' | xargs rm  -rf;) 
endef


define AppsPreUsrBinInstall
	@echo AppsPreUsrBinInstall files:{$(2)} in $(1) to $(APP_PRE_FS_USR_BIN)	
	$(call AppsBinInstall, $(1), $(2), $(APP_PRE_FS_USR_BIN))
endef

define AppsUsrBinClean	
	$(call AppBinClean, $(1), $(APP_PRE_FS_USR_BIN))
endef

define AppsPreUserFsBinInstall
	@echo AppsPreUserFsBinInstall files:{$(2)} in $(1) to $(APP_PRE_FS_USEFS_BIN)	
	$(call AppsBinInstall, $(1), $(2), $(APP_PRE_FS_USEFS_BIN))
endef

define AppsPreUserFsBinClean	
	$(call AppBinClean, $(1), $(APP_PRE_FS_USEFS_BIN))
endef

define AppsPreUsrEtcInstall
	@echo AppsPreUsrEtcInstall files:{$(2)} in $(1) to $(APP_PRE_FS_USR_ETC)	
	$(call AppsBinInstall, $(1), $(2), $(APP_PRE_FS_USR_ETC))
endef

define AppsPreUsrEtcClean	
	$(call AppBinClean, $(1), $(APP_PRE_FS_USR_ETC))
endef

define AppsPreUserFsInstall
	@echo AppsPreUserFsInstall files:{$(2)} in $(1) to $(APP_PRE_FS_USEFS)	
	$(call AppsBinInstall, $(1), $(2), $(APP_PRE_FS_USEFS))
endef

define AppsPreUserFsClean	
	$(call AppBinClean, $(1), $(APP_PRE_FS_USEFS))
endef

define AppsPreFsLibInstall
	@echo AppsPreFsLibInstall files:{$(2)} in $(1) to $(APP_PRE_FS_LIB)
	$(call AppsBinInstall, $(1), $(2), $(APP_PRE_FS_LIB))
endef

define EcntAPPReleaseInstall
	@echo @@@@@ EcntAPPReleaseInstall $(1)
	$(if $(strip $(SDKRelease)), \
		$(shell if [ ! -d $(3) ]; then install -d $(3); fi;) \
		$(foreach fname, $(strip $(2)), -cp -rf $(1)/$(fname) $(3);) )
endef

define EcntAPPReleaseUsrfsBinInstall
	@echo @@@@@ EcntBSPReleaseUsrfsBinInstall $(1)
	$(call EcntAPPReleaseInstall, $(1), $(2), $(RELEASE_APP_DIR)/$(TCPLATFORM)/filesystem/userfs/bin)
endef

define EcntAPPReleaseUsrfsInstall
	@echo @@@@@ EcntBSPReleaseUsrfsInstall $(1)
	$(call EcntAPPReleaseInstall, $(1), $(2), $(RELEASE_APP_DIR)/$(TCPLATFORM)/filesystem/userfs)
endef
define EcntAPPReleaseUsrBinInstall
	@echo @@@@@ EcntBSPReleaseUsrBinInstall $(1)
	$(call EcntAPPReleaseInstall, $(1), $(2), $(RELEASE_APP_DIR)/$(TCPLATFORM)/filesystem/usr/bin)
endef

define EcntAPPReleaseLibInstall
	@echo @@@@@ EcntBSPReleaseLibInstall $(1)
	$(call EcntAPPReleaseInstall, $(1), $(2), $(RELEASE_APP_DIR)/$(TCPLATFORM)/filesystem/lib)
endef

define EcntAPPReleaseLibModulesInstall
	@echo @@@@@ EcntBSPReleaseLibModulesInstall $(1)
	$(call EcntAPPReleaseInstall, $(1), $(2), $(RELEASE_APP_DIR)/$(TCPLATFORM)/filesystem/lib/modules)
endef
