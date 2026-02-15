export CC = $(TOOLPREFIX)gcc
export LD = $(TOOLPREFIX)ld
export AR = $(TOOLPREFIX)ar
export STRIP = $(TOOLPREFIX)strip

export APP_CMM_DIR = $(PRIVATE_APPS_PATH)/user
export APP_PJSIP_DIR = $(PUBLIC_APPS_PATH)/pjsip_1.10
export APP_VOIP_DIR = $(PRIVATE_APPS_PATH)/voip

VENDOR_CFLAGS := -DLINUX

ifneq ($(findstring BCM,$(SUPPLIER_SDK)),)
SUPPLIER=broadcom
endif

ifeq ($(strip $(SUPPLIER)),broadcom)
#include $(KERNELPATH)/$(MODEL)_config

SUPPLIER_VOIP_DIR := $(SDK_PATH)/userspace/private/apps/voice
#DSP_DYNAMIC_TARGET := $(SUPPLIER_VOIP_DIR)
#DSP_DYNAMIC_TARGET += $(SDK_PATH)/userspace/private/libs/xchg
#DSP_STATIC_LIB := $(SDK_PATH)/userspace/private/libs/xchg/libxchg.a

ifeq ($(BRCM_CHIP),63268)
DSP_CFLAGS := -O2 -Wall -Werror -Wno-unused-variable -march=mips32 -DBOS_OS_LINUXUSER -fomit-frame-pointer -fno-strict-aliasing -mabi=32 -G 0 -msoft-float -pipe -Wa,-mips32 -DBRCM_IQCTL
endif

ifeq ($(BRCM_CHIP),63138)
DSP_CFLAGS := -O2 -Wall -Wno-unused-variable -DBOS_OS_LINUXUSER -fomit-frame-pointer -fno-strict-aliasing -msoft-float -pipe -Wa, -DBRCM_IQCTL
#DSP_CFLAGS := -DNONE -DLINUX_FW_EXTRAVERSION=50204 -DSUPPORT_RDPA -DSUPPORT_HOSTMIPS_PWRSAVE -DSUPPORT_ETH_PWRSAVE -DSUPPORT_ENERGY_EFFICIENT_ETHERNET -DSUPPORT_ETH_DEEP_GREEN_MODE -DLINUX -DCHIP_63138 -DCONFIG_BCM963138 -Os -march=armv7-a -fomit-frame-pointer -mno-thumb-interwork -mabi=aapcs-linux -marm -fno-common -ffixed-r8 -msoft-float -D__ARM_ARCH_7A__ -Werror=return-type -Werror=uninitialized -Wno-date-time -Wall -Darm -g -fPIC \
			  -Wtype-limits
endif

ifeq ($(BRCM_CHIP),63146)
DSP_CFLAGS := -O2 -Wall -Wno-unused-variable -DBOS_OS_LINUXUSER -fomit-frame-pointer -fno-strict-aliasing -msoft-float -pipe 
#DSP_CFLAGS := -DNONE -DLINUX_FW_EXTRAVERSION=50204 -DSUPPORT_RDPA -DSUPPORT_HOSTMIPS_PWRSAVE -DSUPPORT_ETH_PWRSAVE -DSUPPORT_ENERGY_EFFICIENT_ETHERNET -DSUPPORT_ETH_DEEP_GREEN_MODE -DLINUX -DCHIP_63138 -DCONFIG_BCM963138 -Os -march=armv7-a -fomit-frame-pointer -mno-thumb-interwork -mabi=aapcs-linux -marm -fno-common -ffixed-r8 -msoft-float -D__ARM_ARCH_7A__ -Werror=return-type -Werror=uninitialized -Wno-date-time -Wall -Darm -g -fPIC \
			  -Wtype-limits
endif

ifeq ($(BRCM_CHIP),6856)
DSP_CFLAGS := -O2 -Wall -Wno-unused-variable -DBOS_OS_LINUXUSER -fomit-frame-pointer -fno-strict-aliasing -msoft-float -pipe -Wa, 
#DSP_CFLAGS := -DNONE -DLINUX_FW_EXTRAVERSION=50204 -DSUPPORT_RDPA -DSUPPORT_HOSTMIPS_PWRSAVE -DSUPPORT_ETH_PWRSAVE -DSUPPORT_ENERGY_EFFICIENT_ETHERNET -DSUPPORT_ETH_DEEP_GREEN_MODE -DLINUX -DCHIP_63138 -DCONFIG_BCM963138 -Os -march=armv7-a -fomit-frame-pointer -mno-thumb-interwork -mabi=aapcs-linux -marm -fno-common -ffixed-r8 -msoft-float -D__ARM_ARCH_7A__ -Werror=return-type -Werror=uninitialized -Wno-date-time -Wall -Darm -g -fPIC \
			  -Wtype-limits
endif

ifeq ($(BRCM_CHIP),63178)
DSP_CFLAGS := -Os -Wno-unused-variable -fno-strict-aliasing -march=armv7-a -fomit-frame-pointer -mno-thumb-interwork -mabi=aapcs-linux -marm -fno-common -ffixed-r8 -msoft-float -D__ARM_ARCH_7A__ -Werror=return-type -Werror=uninitialized -Wno-date-time -Wall -Darm -g -fPIC
#DSP_CFLAGS := -DNONE -DLINUX_FW_EXTRAVERSION=50204 -DSUPPORT_RDPA -DSUPPORT_HOSTMIPS_PWRSAVE -DSUPPORT_ETH_PWRSAVE -DSUPPORT_ENERGY_EFFICIENT_ETHERNET -DSUPPORT_ETH_DEEP_GREEN_MODE -DLINUX -DCHIP_63138 -DCONFIG_BCM963138 -Os -march=armv7-a -fomit-frame-pointer -mno-thumb-interwork -mabi=aapcs-linux -marm -fno-common -ffixed-r8 -msoft-float -D__ARM_ARCH_7A__ -Werror=return-type -Werror=uninitialized -Wno-date-time -Wall -Darm -g -fPIC \
			  -Wtype-limits
#include $(SDK_PATH)/make.voice
#DSP_CFLAGS += VOICE_CFLAGS
#DSP_CFLAGS += VOICE_LDFLAGS
endif

DSP_CFLAGS += -DBCM_DSL_RDP -DBCM_RDP \
			  -DCONFIG_BCM_MAX_GEM_PORTS=1 \
			  -DVRG_COUNTRY_CFG_AUSTRALIA=1 \
			  -DVRG_COUNTRY_CFG_BELGIUM=1 \
			  -DVRG_COUNTRY_CFG_BRAZIL=1 \
			  -DVRG_COUNTRY_CFG_CHILE=1 \
			  -DVRG_COUNTRY_CFG_CHINA=1 \
			  -DVRG_COUNTRY_CFG_CYPRUS=1 \
			  -DVRG_COUNTRY_CFG_CZECH=1 \
			  -DVRG_COUNTRY_CFG_DENMARK=1 \
			  -DVRG_COUNTRY_CFG_ETSI=1 \
			  -DVRG_COUNTRY_CFG_FINLAND=1 \
			  -DVRG_COUNTRY_CFG_FRANCE=1 \
			  -DVRG_COUNTRY_CFG_GERMANY=1 \
			  -DVRG_COUNTRY_CFG_HUNGARY=1 \
			  -DVRG_COUNTRY_CFG_INDIA=1 \
			  -DVRG_COUNTRY_CFG_ITALY=1 \
			  -DVRG_COUNTRY_CFG_JAPAN=1 \
			  -DVRG_COUNTRY_CFG_NETHERLANDS=1 \
			  -DVRG_COUNTRY_CFG_NEW_ZEALAND=1 \
			  -DVRG_COUNTRY_CFG_NORTH_AMERICA=1 \
			  -DVRG_COUNTRY_CFG_NORWAY=1 \
			  -DVRG_COUNTRY_CFG_SPAIN=1 \
			  -DVRG_COUNTRY_CFG_SWEDEN=1 \
			  -DVRG_COUNTRY_CFG_SWITZERLAND=1 \
			  -DVRG_COUNTRY_CFG_TAIWAN=1 \
			  -DVRG_COUNTRY_CFG_TR57=1 \
			  -DVRG_COUNTRY_CFG_UK=1 \
			  -DARCH_ENDIAN=little \
			  -DCMS_LOG3 -DMDM_SHARED_MEM -DCMS_MEM_DEBUG -DSUPPORT_SECURE_BOOT -DBRCM_WLAN -DWIRELESS \
			  -DDSLCPE -DWLAN_UNIFIED_WLMNGR -DDSLCPE_ENDIAN -DSUPPORT_TR181_WLMNGR
			  #-I/opt/toolchains/crosstools-arm-gcc-5.5-linux-4.1-glibc-2.26-binutils-2.28.1/usr/include\
			  #-L/opt/toolchains/crosstools-arm-gcc-5.5-linux-4.1-glibc-2.26-binutils-2.28.1/usr/lib \

BRCM_VOICE_GLOBAL_CFLAGS += CONFIG_TP_IMAGE=1
ifeq ($(strip $(BUILD_IPV6)),y)
DSP_CFLAGS += -DVOICE_IPV6_SUPPORT=1
DSP_CFLAGS += -DGLOBAL_CFG_IPv6_SUPPORT=1
endif

DSP_INCLUDE += -I$(SDK_PATH)/userspace/private/apps/voice/endpt/endpt/inc
DSP_INCLUDE += -I$(SDK_PATH)/userspace/private/apps/voice/inc
DSP_INCLUDE += -I$(SDK_PATH)/userspace/private/apps/voice/endpt/class
DSP_INCLUDE += -I$(SDK_PATH)/userspace/private/apps/voice/endpt/hdsp/inc
DSP_INCLUDE += -I$(SDK_PATH)/bcmdrivers/broadcom/include/bcm963xx
DSP_INCLUDE += -I$(SDK_PATH)/bcmdrivers/opensource/include/bcm963xx
DSP_INCLUDE += -I$(SDK_PATH)/shared/opensource/include/bcm963xx
DSP_INCLUDE += -I$(SDK_PATH)/userspace/private/apps/voice/endpt/ldx/ldx_apps/dsl/apps/hausware_libs_gateway_nodist_arm/lib/arm_glibc_risc.arm.armv7.asm
DSP_INCLUDE += -I$(SDK_PATH)/userspace/private/apps/voice/endpt/ldx/ldx_hausware/inc
DSP_INCLUDE += -I$(SDK_PATH)/userspace/private/apps/voice/endpt/hdsp/cfginc
DSP_INCLUDE += -I$(SDK_PATH)/userspace/private/apps/voice/endpt/cas/inc
DSP_INCLUDE += -I$(SDK_PATH)/userspace/private/apps/voice/endpt/hal/inc
DSP_INCLUDE += -I$(SDK_PATH)/userspace/private/apps/voice/endpt/codec
DSP_INCLUDE += -I$(SDK_PATH)/userspace/private/apps/voice/util/sme
DSP_INCLUDE += -I$(SDK_PATH)/userspace/private/libs/xchg/bos/publicInc
DSP_INCLUDE += -I$(SDK_PATH)/userspace/private/apps/voice/endpt/rtp
DSP_INCLUDE += -I$(SDK_PATH)/userspace/private/apps/voice/util/log
DSP_INCLUDE += -I$(SDK_PATH)/userspace/private/apps/voice/prov/tel_profiles
# SRTP support.
ifeq ($(strip $(INCLUDE_SRTP)),y)
DSP_INCLUDE += -I$(SDK_PATH)/userspace/private/apps/voice/sig/srtp
endif # $(INCLUDE_SRTP)
#DSP_OBJ := $(SUPPLIER_VOIP_DIR)/output/libvoip.o

VENDOR_CFLAGS += -DVOIP_BROADCOM
endif  # ifeq ($(strip $(SUPPLIER)),broadcom)

ifeq ($(strip $(SUPPLIER)),econet)
sinclude $(KERNELPATH)/.config 
#This is included in build/Makefile   
#sinclude $(TOP_PATH)/$(SUPPLIER)/Project/profile/RT/en7512_demo/en7512_demo.profile

ifeq ($(strip $(SDK_CONFIG)), UNION_EN7516_7915D_demo)
SUPPLIER_VOIP_DIR := $(SDK_PATH)/apps/private/voip/$(SDK_CONFIG)
else ifeq ($(findstring UNION_EN7523_GLIBC, $(SDK_CONFIG)), UNION_EN7523_GLIBC)
SUPPLIER_VOIP_DIR := $(SDK_PATH)/app_bsp/private/voip
else ifeq ($(strip $(SDK_CONFIG)), en7512_demo)
SUPPLIER_VOIP_DIR := $(SDK_PATH)/apps/private/voip
else
SUPPLIER_VOIP_DIR := $(SDK_PATH)/app_bsp/private/voip
endif

#DSP_DYNAMIC_TARGET := $(SUPPLIER_VOIP_DIR)
ifeq ($(strip $(SDK_CONFIG)), UNION_EN7516_7915D_demo)
DSP_DYNAMIC_LIB := -L$(SUPPLIER_VOIP_DIR)/eva/bin -ladam 
else ifeq ($(findstring UNION_EN7523_GLIBC, $(SDK_CONFIG)), UNION_EN7523_GLIBC)
DSP_DYNAMIC_LIB := -L$(RELEASE_BSP_DIR)/$(MPLATFORM)/filesystem/lib -ladam -lrt
else
DSP_STATIC_LIB := -L$(SUPPLIER_VOIP_DIR)/MTK_SIP/install/lib -lslic_user -lvdsp_user -lsyss -lbase
DSP_DYNAMIC_LIB := -L$(SUPPLIER_VOIP_DIR)/eva/bin -ladam -lgdi_mtk
endif

# SRTP support.
ifeq ($(strip $(INCLUDE_SRTP)),y)
DSP_STATIC_LIB += -L$(SUPPLIER_VOIP_DIR)/userspace/private/apps/voice/sig/srtp -lsrtp.arm.saved
endif # $(INCLUDE_SRTP)

ifeq ($(findstring UNION_EN7523_GLIBC, $(SDK_CONFIG)), UNION_EN7523_GLIBC)
DSP_CFLAGS := -O2 -Wall -msoft-float -fomit-frame-pointer -mglibc -DOSAL_PTHREADS -lrt -lm
else
DSP_CFLAGS := -O2 -mips32r2 -msoft-float -muclibc -DOSAL_PTHREADS -lrt -lm
endif
ifeq ($(strip $(SDK_CONFIG)), UNION_EN7516_7915D_demo)
DSP_INCLUDE += -I$(KERNELHEADER)/
else ifeq ($(findstring UNION_EN7523_GLIBC, $(SDK_CONFIG)), UNION_EN7523_GLIBC)
DSP_INCLUDE += -I$(KERNELHEADER)/
else
DSP_INCLUDE += -I$(KERNELPATH)/arch/mips/include/
DSP_INCLUDE += -I$(KERNELPATH)/include/
endif

ifeq ($(strip $(SDK_CONFIG)), UNION_EN7516_7915D_demo)
DSP_INCLUDE += -I$(SUPPLIER_VOIP_DIR)/eva/common
DSP_INCLUDE += -I$(SUPPLIER_VOIP_DIR)/eva/adam
DSP_INCLUDE += -I$(SUPPLIER_VOIP_DIR)/eva/gdi_mtk
DSP_INCLUDE += -I$(SUPPLIER_VOIP_DIR)/include
else ifeq ($(strip $(SDK_CONFIG)), en7512_demo)
DSP_INCLUDE += -I$(SUPPLIER_VOIP_DIR)/eva/common
DSP_INCLUDE += -I$(SUPPLIER_VOIP_DIR)/eva/adam
DSP_INCLUDE += -I$(SUPPLIER_VOIP_DIR)/eva/gdi_mtk
DSP_INCLUDE += -I$(MODULES_MTK_FXS3_DIR)/include -I$(MODULES_MTK_OVDSP_DIR)/include
else
DSP_INCLUDE += -I$(BSP_EXT_INC)
DSP_INCLUDE += -I$(SUPPLIER_VOIP_DIR)/voip_lib
#DSP_INCLUDE += -I$(SUPPLIER_VOIP_DIR)/eva/gdi_mtk
#DSP_INCLUDE += -I$(MODULES_MTK_FXS3_DIR)/include -I$(MODULES_MTK_OVDSP_DIR)/include
endif

VENDOR_CFLAGS += -DRALINK -DMTK

DSP_CFLAGS += -DVTSP_DEBUG_NETLOG -DDSPID_MTK
ifeq ($(strip $(INCLUDE_ECONET_MT7513)), y)
DSP_CFLAGS += -DMAX_NUM_FXS -DMAX_FXS_NUM=$(NUM_FXS_CHANNELS) -DMAX_STREAM_PER_CH=2
else
DSP_CFLAGS += -lgcc_s
endif
#PLATFORM wifi driver used. see "sdk/en7523/modules/private/wifi/MT7915_v7.4/embedded/Makefile"
#export PLATFORM:=tc3182 
export DSP:=mtk
export TCSUPPORT_VOIP=y
export VOIP_DSP=MTK

ifneq ($(strip $(TCSUPPORT_SDRAM_32M)),)
VENDOR_CFLAGS += -DSUPPORT_SDRAM_32M=1
endif

MODULES_MTK_FXS3_DIR:= $(SDK_PATH)/modules/private/voip_2.6.36/DSP/MTK/mod-fxs3
MODULES_MTK_OVDSP_DIR:= $(SDK_PATH)/modules/private/voip_2.6.36/DSP/MTK/mod-ovdsp
#ifeq ($(KERNELVERSION), 2.6.36)
#MODULES_MTK_FXS3_DIR:= $(MODULES_PATH)/voip_2.6.36/DSP/MTK/mod-fxs3
#MODULES_MTK_OVDSP_DIR:= $(MODULES_PATH)/voip_2.6.36/DSP/MTK/mod-ovdsp
#else
#MODULES_MTK_FXS3_DIR:= $(MODULES_PATH)/voip/DSP/MTK/mod-fxs3
#MODULES_MTK_OVDSP_DIR:= $(MODULES_PATH)/voip/DSP/MTK/mod-ovdsp
#endif

APP_MTKSIP_DIR:=$(SUPPLIER_VOIP_DIR)/MTK_SIP
LIB_DIR:=$(SDK_PATH)/apps/private/lib
ifeq ($(strip $(SDK_CONFIG)), UNION_EN7516_7915D_demo)
DSP_MTK_API_DIR := $(SDK_PATH)/apps/private/voip/$(SDK_CONFIG)
else ifeq ($(findstring UNION_EN7523_GLIBC, $(SDK_CONFIG)), UNION_EN7523_GLIBC)
DSP_MTK_API_DIR := $(SDK_PATH)/app_bsp/private/voip
else
DSP_MTK_API_DIR := $(BBA_PATH)/apps/private/voip
endif
DSP_MTK_INCLUDE = -I$(MODULES_MTK_FXS3_DIR)/include -I$(MODULES_MTK_OVDSP_DIR)/include
DSP_MTK_API_LIB := -L $(MODULES_MTK_FXS3_DIR)/ -lslic_user -L $(MODULES_MTK_OVDSP_DIR)/ -lvdsp_user -L$(DSP_MTK_API_DIR)/MTK_SIP/install/lib/ -lsyss -lbase
export MODULES_MTK_FXS3_DIR MODULES_MTK_OVDSP_DIR DSP_MTK_API_DIR DSP_MTK_INCLUDE DSP_MTK_API_LIB APP_MTKSIP_DIR

ifeq ($(strip $(SDK_CONFIG)), UNION_EN7516_7915D_demo)
CONFIG_NR_CPUS=4
else ifeq ($(findstring UNION_EN7523_GLIBC, $(SDK_CONFIG)), UNION_EN7523_GLIBC)
CONFIG_NR_CPUS=2
else
CONFIG_NR_CPUS=4
endif
endif  # ifeq ($(strip $(SUPPLIER)),econet)

# ----------------------- add mtk-d2 sdk ---------------------------
ifeq ($(INCLUDE_DSP_D2), y)

DSP_CFLAGS := -O2 -Wall -Wno-unused-variable -fomit-frame-pointer -nostdinc -DOSAL_PTHREADS
DSP_CFLAGS += -DMAX_NUM_FXS -DMAX_FXS_NUM=$(NUM_FXS_CHANNELS) -DMAX_STREAM_PER_CH=2

SUPPLIER_VOIP_DIR := $(MODEL_PATH)/voip_sdk
DSP_INCLUDE += -isystem $(OPENWRT_ROOT_PATH)/staging_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/include/ \
		-isystem $(OPENWRT_ROOT_PATH)/staging_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/include/linux/ \
		-isystem $(OPENWRT_ROOT_PATH)/staging_dir/target-aarch64_cortex-a53_musl/usr/include \

VENDOR_CFLAGS += -DRALINK -DMTK
VENDOR_CFLAGS += -DVOIP_MTK

SUPPLIER_VOIP_DIR := $(MODEL_PATH)/voip_sdk
DSP_INCLUDE += -I$(PRIVATE_APPS_PATH)/voip/server/environment_d2/include
DSP_STATIC_LIB := -L$(SUPPLIER_VOIP_DIR)/lib/ \
	-losal_user \
	-lve_vtsp 
	
endif
# --------------------------- end add --------------------------------

ifneq ($(strip $(CONFIG_NR_CPUS)), )
VENDOR_CFLAGS += -DCONFIG_NR_CPUS=$(CONFIG_NR_CPUS)
endif # CONFIG_NR_CPUS

ifeq ($(strip $(INCLUDE_DECT)),)
NUM_DECT_CHANNELS = 0
endif

export DSP_DYNAMIC_TARGET
export DSP_DYNAMIC_LIB
export DSP_STATIC_LIB
export DSP_OBJ
export DSP_CFLAGS
export DSP_INCLUDE

ifneq ($(strip $(INCLUDE_VOIP)),)
export INCLUDE_VOIP
export SUPPLIER

# now configuring voip locale settings

ifneq ($(strip $(VOIP_LOCALE_ALL)),)
VOIP_LOCALE := -DVOIP_CFG_ALL
ifeq ($(strip $(SUPPLIER)),broadcom)
export BRCM_VRG_COUNTRY_ALL_PROFILES=y
endif
else
VOIP_LOCALE := $(shell cat config/$(MODEL).config | sed -n 's/=y$$//p' | sed -n 's/^VOIP_CFG/-D&/p')
ifeq ($(strip $(SUPPLIER)),broadcom)
VENDOR_CFLAGS += -DBRCM_VRG_COUNTRY_CFG_CUSTOM_PROFILES
VENDOR_CFLAGS += $(shell cat config/$(MODEL).config | sed -n 's/=y$$//p' | sed -n 's/^VOIP_CFG/-DBRCM_VRG_COUNTRY_CFG/p')
export BRCM_VRG_COUNTRY_ALL_PROFILES=n
# for customized locale, it is defined as VOIP_CFG_xxx in the config
# to build modules, BCM source need BRCM_VRG_COUNTRY_CFG_xxx
$(warning *** You MUST export BRCM_VRG_COUNTRY_CFG_xxx to build voice modules ***)
endif
endif # VOIP_LOCALE_ALL

export VOIP_LOCALE += -I$(APP_VOIP_DIR)/inc

VOIP_DFLAGS := -DINCLUDE_VOIP

VOIP_CFLAGS := $(VOIP_LOCALE)

# wlm add
ifeq ($(strip $(INCLUDE_DECT)),y)
export INCLUDE_DECT
#VOIP_CFLAGS += -DINCLUDE_DECT
VOIP_DFLAGS += -DINCLUDE_DECT
endif

ifeq ($(strip $(INCLUDE_DECT_DSPG)),y)
export INCLUDE_DECT_DSPG
#VOIP_CFLAGS += -DINCLUDE_DECT_DSPG
VOIP_DFLAGS += -DINCLUDE_DECT_DSPG
endif

ifeq ($(strip $(INCLUDE_CALLTHROUGH)),y)
export INCLUDE_CALLTHROUGH=y
ifeq ($(strip $(INCLUDE_ECONET_MT7513)),y)
else
VOIP_CFLAGS += -DINCLUDE_CALLTHROUGH
VOIP_DFLAGS += -DINCLUDE_CALLTHROUGH
endif
WEBFLAGS += INCLUDE_CALLTHROUGH=1
else
WEBFLAGS += INCLUDE_CALLTHROUGH=0
endif

ifeq ($(strip $(INCLUDE_CALLFWD_THROUGH_DUT)),y)
export INCLUDE_CALLFWD_THROUGH_DUT=y
VOIP_DFLAGS += -DINCLUDE_CALLFWD_THROUGH_DUT
WEBFLAGS += INCLUDE_CALLFWD_THROUGH_DUT=1
else
WEBFLAGS += INCLUDE_CALLFWD_THROUGH_DUT=0
endif

ifeq ($(strip $(INCLUDE_SRTP)),y)
export INCLUDE_SRTP=y
VOIP_DFLAGS += -DINCLUDE_SRTP
WEBFLAGS += INCLUDE_SRTP=1
else
WEBFLAGS += INCLUDE_SRTP=0
endif # $(INCLUDE_SRTP)

VOIP_CFLAGS += -I$(APP_VOIP_DIR)/inc/client -I$(APP_VOIP_DIR)/inc/server 
VOIP_CFLAGS += -I$(APP_PJSIP_DIR)/pjlib/include
ifeq ($(strip $(SUPPLIER)),broadcom)
VOIP_CFLAGS += -DVOIP_BROADCOM
VOIP_CFLAGS += -I$(APP_VOIP_DIR)/server/broadcom/cmbs
endif
#VOIP_CFLAGS += -I$(OS_LIB_PATH)/include -I$(TP_MODULES_PATH)/voip
VOIP_CFLAGS += -I$(OS_LIB_PATH)/include -I$(PRIVATE_MODULES_PATH)/voip
#VOIP_CFLAGS += -I$(APP_VOIP_DIR)/common
VOIP_CFLAGS += -I$(APP_CMM_DIR)/include

export KER_FLAGS
ifeq ($(findstring -DCONFIG_IP_MULTIPLE_TABLES=y, $(KER_FLAGS)),-DCONFIG_IP_MULTIPLE_TABLES=y)
VOIP_CFLAGS += -DCONFIG_IP_MULTIPLE_TABLES
endif
#ifeq ($(strip $(CONFIG_IP_MULTIPLE_TABLES)), y)
#VOIP_CFLAGS += -DCONFIG_IP_MULTIPLE_TABLES
#endif
ifeq ($(findstring -DCONFIG_IPV6_MULTIPLE_TABLES=y, $(KER_FLAGS)),-DCONFIG_IPV6_MULTIPLE_TABLES=y)
VOIP_CFLAGS += -DCONFIG_IPV6_MULTIPLE_TABLES
endif

#ifeq ($(strip $(INCLUDE_DSP_SOCKET_OPEN)), y)
#VOIP_CFLAGS += -DINCLUDE_DSP_SOCKET_OPEN
#export INCLUDE_DSP_SOCKET_OPEN
#endif

export NUM_FXS_CHANNELS := $(NUM_FXS_CHANNELS)
VOIP_DFLAGS += -DINCLUDE_FXS_NUM=$(NUM_FXS_CHANNELS)
VOIP_CFLAGS += -DNUM_FXS_CHANNELS=$(NUM_FXS_CHANNELS)
ifeq ($(strip $(INCLUDE_CPU_AR9344)),y)
export CHANNEL = $(NUM_FXS_CHANNELS)
endif

# wlm add for DECT
export NUM_DECT_CHANNELS
VOIP_DFLAGS += -DINCLUDE_DECT_NUM=$(NUM_DECT_CHANNELS)
VOIP_CFLAGS += -DNUM_DECT_CHANNELS=$(NUM_DECT_CHANNELS)
# wlm end 

ifeq ($(strip $(INCLUDE_VOICEAPP)),y)
export INCLUDE_VOICEAPP
VOIP_DFLAGS += -DINCLUDE_VOICEAPP
VOIP_CFLAGS += -DINCLUDE_VOICEAPP
endif #  ifeq ($(strip $(INCLUDE_VOICEAPP)),y)

export NUM_VOICEAPP_CHANNELS
ifneq ($(strip $(NUM_VOICEAPP_CHANNELS)),)
VOIP_DFLAGS += -DINCLUDE_VOICEAPP_NUM=$(NUM_VOICEAPP_CHANNELS)
VOIP_CFLAGS += -DNUM_VOICEAPP_CHANNELS=$(NUM_VOICEAPP_CHANNELS)
else
VOIP_DFLAGS += -DINCLUDE_VOICEAPP_NUM=0
VOIP_CFLAGS += -DNUM_VOICEAPP_CHANNELS=0
endif


#ifneq ($(strip $(INCLUDE_DMZ)),)
#VOIP_CFLAGS += -DINCLUDE_DMZ
#endif

ifneq ($(strip $(INCLUDE_EMERGENCY_CALL)),)
VOIP_DFLAGS += -DINCLUDE_EMERGENCY_CALL
endif

ifneq ($(strip $(INCLUDE_P2P_CALL)),)
VOIP_DFLAGS += -DINCLUDE_P2P_CALL
endif

ifneq ($(strip $(INCLUDE_DIGITMAP)),)
export INCLUDE_DIGITMAP=y
VOIP_DFLAGS += -DINCLUDE_DIGITMAP
endif

ifneq ($(strip $(INCLUDE_USB_VOICEMAIL)),)
export INCLUDE_USB_VOICEMAIL=y
VOIP_DFLAGS += -DINCLUDE_USB_VOICEMAIL

ifneq ($(strip $(INCLUDE_USBVM_MODULE)),)
#ifeq ($(strip $(CONFIG_HZ)), $(shell echo $$[$(CONFIG_HZ) / 100 * 100]))
ifeq ($(strip $(CONFIG_HZ)), $(shell echo $(CONFIG_HZ) | awk '{v1=$$1/100*100; printf("%d", v1);}'))
export INCLUDE_USBVM_MODULE=y
ifeq ($(strip $(SUPPLIER)),broadcom)
VOIP_CFLAGS += -DINCLUDE_USBVM_MODULE
endif
endif
endif
endif

ifneq ($(strip $(INCLUDE_CALLLOG)),)
export INCLUDE_CALLLOG=y
VOIP_DFLAGS += -DINCLUDE_CALLLOG
endif

ifneq ($(strip $(INCLUDE_PSTN)),)
NUM_FXO_CHANNELS = 1
export INCLUDE_PSTN = y
VOIP_DFLAGS += -DINCLUDE_PSTN
ifneq ($(strip $(INCLUDE_PSTN_LIFELINE)),)
export INCLUDE_PSTN_LIFELINE=y
VOIP_DFLAGS += -DINCLUDE_PSTN_LIFELINE
endif
ifneq ($(strip $(INCLUDE_PSTN_POLREV)),)
export INCLUDE_PSTN_POLREV=y
VOIP_DFLAGS += -DINCLUDE_PSTN_POLREV
endif
ifneq ($(strip $(INCLUDE_PSTN_GATEWAY)),)
export INCLUDE_PSTN_GATEWAY=y
VOIP_DFLAGS += -DINCLUDE_PSTN_GATEWAY
endif
else
NUM_FXO_CHANNELS = 0
endif  # INCLUDE_PSTN

export NUM_FXO_CHANNELS
VOIP_DFLAGS += -DINCLUDE_FXO_NUM=$(NUM_FXO_CHANNELS)
VOIP_CFLAGS += -DNUM_FXO_CHANNELS=$(NUM_FXO_CHANNELS)

export NUM_CHANNELS=$(shell echo $(NUM_FXS_CHANNELS) $(NUM_DECT_CHANNELS) $(NUM_FXO_CHANNELS) | awk '{v1=$$1+$$2+$$3; printf("%d", v1);}')
VOIP_CFLAGS += -DNUM_CHANNELS=$(NUM_CHANNELS)

export VOIP_CFLAGS += $(VENDOR_CFLAGS)
export VOIP_DFLAGS

endif  # INCLUDE_VOIP

