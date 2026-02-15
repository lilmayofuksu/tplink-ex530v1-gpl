#**************************************************************************** 
# 
#  Copyright(c) 2010-2017 Shenzhen TP-LINK Technologies Co.Ltd. 
#  All Rights Reserved 
#  YangXv <yangxu@tp-link.com.cn> 
#  Double <double.syu@tp-link.com>
# 
#***************************************************************************
####################################################
#
# General targets
#
####################################################
ifeq ($(BBA_BUILD),)
.PHONY: sdk_init
all: sdk_init
clean:

else
ALL_TARGET =
ALL_TARGET += env
ALL_TARGET += boot
ALL_TARGET += kernel
ALL_TARGET += modules
ALL_TARGET += apps
ALL_TARGET += fs
ALL_TARGET += image
ALL_TARGET += check

ALL_TARGET_BUILD = $(addsuffix _build,$(ALL_TARGET))
ALL_TARGET_CLEAN = $(addsuffix _clean,$(ALL_TARGET))

.PHONY: $(ALL_TARGET_BUILD) $(ALL_TARGET_CLEAN)

all: $(ALL_TARGET_BUILD)
clean: $(ALL_TARGET_CLEAN)
endif

####################################################
#
# Top-level Makefile for all linux platforms
#
####################################################
PLATFORM_BUILD_DIR:=$(PWD)
export TOP_PATH=$(shell cd $(PLATFORM_BUILD_DIR)/../.. && pwd)


export BBA_PATH = $(TOP_PATH)/platform
export BASH := $(shell which bash)

# BBA Root Level
export BUILD_PATH		= $(BBA_PATH)/build
export WEB_PATH			= $(BBA_PATH)/web/en
export DEF_DM_PATH		= $(BBA_PATH)/dataModel
export PUBLIC_APPS_PATH		= $(BBA_PATH)/apps/public
export PUBLIC_MODULES_PATH	= $(BBA_PATH)/modules/public
export TARGETS_PATH		= $(BBA_PATH)/targets
export TOOLS_PATH		= $(BBA_PATH)/host_tools
export FAKEROOT_PATH		= $(TOOLS_PATH)/fakeroot

# BBA Build Level
MAKES_PATH		= $(BUILD_PATH)/makes
MENU_PATH		= $(BUILD_PATH)/menu
export MODEL_PATH	= $(BUILD_PATH)/model/$(MODEL)/$(SPEC)
export OBJ_TG_PATH	= $(MODEL_PATH)/objs

#TG means target
export BOOT_TG_PATH     = $(TARGETS_PATH)/$(MODEL)/$(SPEC)/boot
export KERNEL_TG_PATH   = $(TARGETS_PATH)/$(MODEL)/$(SPEC)/kernel
export DM_TG_PATH    = $(TARGETS_PATH)/$(MODEL)/$(SPEC)/dataModel
export FS_TG_PATH       = $(TARGETS_PATH)/$(MODEL)/$(SPEC)/filesystem
export IMG_TG_PATH      = $(TARGETS_PATH)/$(MODEL)/$(SPEC)/image
export TEST_TG_PATH     = $(TARGETS_PATH)/$(MODEL)/$(SPEC)/test
export RELEASE_IMG_PATH = $(TARGETS_PATH)/$(MODEL)/$(SPEC)/release

# all modules version
export BBA_PRODUCT_VER_FILE = $(FS_TG_PATH)/etc/platform_ver

# Import Makefile library
include $(MAKES_PATH)/Makefile.common

# Export all of flag
$(eval $(call checkFile,$(MODEL_PATH)/config.bba))
$(eval $(call exportFlagsFromFile,$(MODEL_PATH)/config.bba))

export INCLUDE_$(subst -,_,$(MODEL))=y

ifdef INCLUDE_CUSTOMIZED_SP
ifneq ($(INCLUDE_CUSTOMIZED_SP), "")
export CSP=$(subst ",,$(INCLUDE_CUSTOMIZED_SP))
export INCLUDE_$(CSP)=y
export INCLUDE_$(subst -,_,$(MODEL))_$(SPEC)_$(CSP)=y
endif
endif

SDK_PATH = $(TOP_PATH)/sdk/$(SDK_DIR)

# Import SDK config to get chipset infomation by project
ifeq ($(INCLUDE_MTK)$(INCLUDE_QUALCOMM),)
$(eval $(call checkFile,$(MODEL_PATH)/config.sdk))
$(eval $(call exportFlagsFromFile,$(MODEL_PATH)/config.sdk))
endif
export BUILD = $(shell gcc -dumpmachine)
export ARCH=$(INCLUDE_ARCH)

# Check Supplier from config.bba
$(eval $(call checkDefine,SUPPLIER))
ifeq ($(INCLUDE_PON),y)
include $(MAKES_PATH)/Makefile.sdk.$(SUPPLIER).pon
else ifeq ($(INCLUDE_SPEC_XX530v),y)
include $(MAKES_PATH)/Makefile.sdk.$(SUPPLIER).pon
else ifeq ($(INCLUDE_SPEC_EX530v),y)
include $(MAKES_PATH)/Makefile.sdk.$(SUPPLIER).pon
else
include $(MAKES_PATH)/Makefile.sdk.$(SUPPLIER)
endif
ifeq ($(INCLUDE_QUALCOMM), y)
include $(MAKES_PATH)/qca.image.mk
endif
ifeq ($(INCLUDE_QUECTEL), y)
include $(MAKES_PATH)/qtl.image.mk
endif

# Check DataModel for this product
$(eval $(call checkFile,$(MODEL_PATH)/reduced_data_model.xml))

$(eval $(call checkDefine,CC_TARBALL))
$(eval $(call checkDefine,HOST))
$(eval $(call checkDefine,TOOLPREFIX))
$(eval $(call checkDefine,TOOLROOTPATH))
$(eval $(call checkDefine,SLIB_PATH))

$(eval $(call checkDefine,PROFILE))
$(eval $(call checkDefine,SDK_CONFIG_FILE))
$(eval $(call checkDefine,KERNELPATH))
$(eval $(call checkDefine,KERNELHEADER))

include $(MAKES_PATH)/Makefile.prepare
include $(MAKES_PATH)/Makefile.kmod.public
include $(MAKES_PATH)/Makefile.apps.public
include $(MAKES_PATH)/Makefile.rootfs
include $(MAKES_PATH)/Makefile.image
include $(MAKES_PATH)/Makefile.check

####################################################
# 
# Export Compile Flags
#
####################################################
# Compile Flag from config.bba, config.kernel
$(eval $(call compileFlagsFromFile,BBA_FLAGS,$(MODEL_PATH)/config.bba))
$(eval $(call compileFlagsFromFile,KER_FLAGS,$(MODEL_PATH)/config.kernel))

# prepare environment for VoIP
ifeq ($(INCLUDE_VOIP), y)
ifeq ($(INCLUDE_ECONET), y)
export ENVIRONMENT = environment_mtk
endif
ifeq ($(INCLUDE_BROADCOM), y)
export ENVIRONMENT = broadcom
endif
ifeq ($(INCLUDE_DSP_D2), y)
export ENVIRONMENT = environment_d2
endif
include voip.mak
endif

# For WebGUI
$(eval $(call webFlagsFromFile,WEBFLAGS,$(MODEL_PATH)/config.bba))

BBA_FLAGS += -DINCLUDE_$(subst -,_,$(MODEL))

ifdef INCLUDE_CUSTOMIZED_SP
ifneq ($(INCLUDE_CUSTOMIZED_SP), "")
export CSP=$(subst ",,$(INCLUDE_CUSTOMIZED_SP))
BBA_FLAGS += -DINCLUDE_$(CSP)
BBA_FLAGS += -DINCLUDE_$(subst -,_,$(MODEL))_$(SPEC)_$(CSP)
endif
endif

BBA_FLAGS += $(SUPPLIER_FLAGS)

# For SDK
SDK_FLAGS += $(BBA_FLAGS)
export SDK_FLAGS

export BBA_FLAGS

# For BBA Application build
DF_FLAGS = $(BBA_FLAGS)
DF_FLAGS += $(VOIP_CFLAGS)
export DF_FLAGS

####################################################
#
# BBA Menuconfig         
#
####################################################
.PHONY: menuconfig defaultconfig

$(MENU_PATH)/scripts/config/mconf:
	make -C $(MENU_PATH)/scripts/config
	
menuconfig: $(MENU_PATH)/scripts/config/mconf
	cp -pfT $(MODEL_PATH)/config.bba $(BUILD_PATH)/.config
	cd $(BUILD_PATH) && $(MENU_PATH)/scripts/config/mconf $(MENU_PATH)/sysdeps/Config.in
	cp -pfT $(BUILD_PATH)/.config  $(MODEL_PATH)/config.bba

defaultconfig: $(MENU_PATH)/scripts/config/mconf
	cp -pfT $(MODEL_PATH)/config.bba $(BUILD_PATH)/.config
	cd $(BUILD_PATH) && $(MENU_PATH)/scripts/config/mconf $(MENU_PATH)/sysdeps/Config.in --default
	cp -pfT $(BUILD_PATH)/.config  $(MODEL_PATH)/config.bba

####################################################
#
# Compilation environment
# build/makes/Makefile.prepare
# build/makes/Makefile.sdk.$(SUPPLIER)
#
####################################################
.PHONY: \
	model_init \
	supplier_env_build \
	tools_build

env_build: model_init supplier_env_build tools_build
	
####################################################
#
# Build bootloader
# build/makes/Makefile.sdk.$(SUPPLIER)
#
####################################################
.PHONY: \
	supplier_boot_build \
	supplier_boot_clean

boot_build: supplier_boot_build
boot_clean: supplier_boot_clean
	rm -rf $(BOOT_TG_PATH)/*

####################################################
#
# Build linux kernel
# build/makes/Makefile.sdk.$(SUPPLIER)
#
####################################################
.PHONY: \
	kernel_menuconfig

.PHONY: \
	supplier_kernel_menuconfig \
	supplier_kernel_build \
	supplier_kernel_clean

kernel_menuconfig: supplier_kernel_menuconfig
kernel_build: supplier_kernel_build
ifeq ($(INCLUDE_ECONET), y)
	if [ ! -f $(KERNEL_TG_PATH)/linux.7z.origin ]; then \
		cp -pf $(KERNELPATH)/linux.7z $(KERNEL_TG_PATH)/; \
		cp -pf $(KERNELPATH)/vmlinux $(KERNEL_TG_PATH)/;	\
	fi
endif
kernel_clean: supplier_kernel_clean
	rm -Rf $(KERNEL_TG_PATH)/*

####################################################
#
# Kernel Modules
#
# build/makes/Makefile.kmod.public
#
####################################################
ALL_KMOD =
ALL_KMOD += public_modules


ALL_KMOD_BUILD = $(addsuffix _build,$(ALL_KMOD))
ALL_KMOD_INSTALL = $(addsuffix _install,$(ALL_KMOD))
ALL_KMOD_CLEAN = $(addsuffix _clean,$(ALL_KMOD))

.PHONY: $(ALL_KMOD_BUILD) $(ALL_KMOD_INSTALL) $(ALL_KMOD_CLEAN)

modules_build: $(ALL_KMOD_BUILD)
modules_install: $(ALL_KMOD_INSTALL)
modules_clean: $(ALL_KMOD_CLEAN)

####################################################
#
# Application
#
# build/makes/Makefile.apps.public
# build/makes/Makefile.sdk.$(SUPPLIER)
#
####################################################
ALL_APPS =
ALL_APPS += supplier_apps
ALL_APPS += public_apps

ALL_APPS_BUILD = $(addsuffix _build,$(ALL_APPS))
ALL_APPS_INSTALL = $(addsuffix _install,$(ALL_APPS))
ALL_APPS_CLEAN = $(addsuffix _clean,$(ALL_APPS))

.PHONY: $(ALL_APPS_BUILD) $(ALL_APPS_INSTALL) $(ALL_APPS_CLEAN)

apps_build: $(ALL_APPS_BUILD)
apps_clean: $(ALL_APPS_CLEAN)

####################################################
#
# Build image
# build/makes/Makefile.rootfs
# build/makes/Makefile.image
# build/makes/Makefile.sdk.$(SUPPLIER)
#
####################################################

ALL_IMAGE_BUILD += bba_mkkernel_build
ALL_IMAGE_BUILD += bba_mkimage_build

.PHONY: $(ALL_IMAGE_BUILD)

mkkernel_build: bba_mkkernel_build
mkkernel_clean:

ifeq ($(INCLUDE_QUALCOMM), y)
image_build:  ipq50xx_image_build
else
ifeq ($(INCLUDE_QUECTEL), y)
image_build:  t750_image_build
else
ifeq ($(INCLUDE_REALTEK), y)
image_build:  rtl819x_image_build
else
image_build:  $(ALL_IMAGE_BUILD)
endif
endif
endif

image_clean:

