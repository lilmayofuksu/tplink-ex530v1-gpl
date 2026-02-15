APP_BSP_DIR=$(shell pwd)
APP_BSP_API_DIR=$(APP_BSP_DIR)/API

ifneq ($(strip $(TCSUPPORT_OPENWRT)),)
ifneq ($(strip $(TCSUPPORT_4_4_KERNEL)),)
KERNEL_DIR:=$(KERNEL_ROOT)/linux-4.4.115
KERNEL_HEADER:=$(KERNEL_DIR)/usr/include
endif
ifneq ($(strip $(TCSUPPORT_5_4_KERNEL)),)
KERNEL_DIR:=$(KERNEL_ROOT)/linux-5.4.55
KERNEL_HEADER:=$(KERNEL_DIR)/usr/include
endif
endif

ifneq ($(strip $(TCSUPPORT_OPENWRT)),)
COMPILEOPTION_CFLAGS := 
export COMPILEOPTION_CFLAGS
COMPILEOPTION_LDFLAGS := -lcompileoption
export COMPILEOPTION_LDFLAGS
endif

ifneq ($(strip $(TCSUPPORT_OPENWRT)),)
LIB_INSTALL_DIR:=$(APP_BSP_DIR)/bsp_lib
FILESYSTEM_DIR:=$(APP_BSP_DIR)/bsp_fs
LIBINSTALL_LDFLAGS:=-L$(LIB_INSTALL_DIR) -Wl,-rpath-link,$(LIB_INSTALL_DIR)
LIBINSTALL_CFLAGS:=-I$(LIB_INSTALL_DIR)
APP_BSP_LIB_DIR:=$(LIB_INSTALL_DIR)
APP_API_LIB_OUTPUT_DIR:=$(LIB_INSTALL_DIR)
export APP_API_LIB_OUTPUT_DIR
APP_BSP_API_LIB_OUTPUT_DIR:=$(APP_BSP_LIB_DIR)
export APP_BSP_API_LIB_OUTPUT_DIR
endif


ifneq ($(strip $(TCSUPPORT_OPENWRT)),)
export BSP_INT_LIB=$(LIB_INSTALL_DIR)
#export BSP_INT_INC=$(LIB_INSTALL_DIR)/inc
export BSP_INT_INC=$(BSP_INT_LIB)

export BSP_INT_FS=$(FILESYSTEM_DIR)
export BSP_INT_FS_USEFS=$(BSP_INT_FS)/userfs/bin

export BSP_EXT_INSTALL=$(APP_BSP_DIR)/install_bsp
#export BSP_EXT_LIB=$(BSP_EXT_INSTALL)/lib
#export BSP_EXT_INC=$(BSP_EXT_INSTALL)/inc
export BSP_EXT_LIB=$(BSP_INT_LIB)
export BSP_EXT_INC=$(BSP_INT_INC)
export BSP_EXT_BIN=$(BSP_EXT_INSTALL)/bin
export BSP_EXT_BIN_BIN=$(BSP_EXT_INSTALL)/bin/bin
export BSP_EXT_BIN_USR=$(BSP_EXT_INSTALL)/bin/usr/bin
export BSP_EXT_BIN_USEFS=$(BSP_EXT_INSTALL)/bin/userfs/bin
export BSP_EXT_FS=$(BSP_EXT_INSTALL)/fs
export BSP_EXT_FS_BIN=$(BSP_EXT_FS)/bin
export BSP_EXT_FS_USR=$(BSP_EXT_FS)/usr/bin
export BSP_EXT_FS_USEFS=$(BSP_EXT_FS)/userfs/bin
export BSP_EXT_MAKE=$(BSP_EXT_INSTALL)/make
export INSTALL_KERNELHEADER=$(BSP_EXT_INSTALL)/kernel_header

KERNELHEAD_CFLAGS=-I$(INSTALL_KERNELHEADER)
export KERNELHEAD_CFLAGS


################################
#BSP INT API for BSP internel api
export BSPAPI_INTLINK=-Wl,-rpath-link,$(BSP_INT_LIB):$(LIB_INSTALL_DIR)
BSPAPI_INTCFLAGS=-I$(LIB_INSTALL_DIR) -I$(BSP_INT_INC) -I$(BSP_INT_LIB)
export BSPAPI_INTCFLAGS

BSPAPI_INTLDFLAGS=-L$(LIB_INSTALL_DIR) -L$(BSP_INT_LIB) $(BSPAPI_INTLINK)
export BSPAPI_INTLDFLAGS

#BSP API for blapi/or other custom sdk
export BSPAPI_LINK=-Wl,-rpath-link,$(BSP_EXT_LIB)
BSPAPI_CFLAGS=-I$(BSP_INT_INC) -I$(BSP_EXT_INC) -I$(BSP_INT_LIB)
export BSPAPI_CFLAGS

BSPAPI_LDFLAGS=-L$(LIB_INSTALL_DIR) -L$(BSP_EXT_LIB) $(BSPAPI_LINK)
export BSPAPI_LDFLAGS

#BLAPI for ECNT APP
export BLAPI_LINK=-Wl,-rpath-link,$(BSP_EXT_LIB)
BLAPI_CFLAGS=-I$(BSP_EXT_INC)
export BLAPI_CFLAGS

BLAPI_LDFLAGS=-L$(LIB_INSTALL_DIR) -L$(BSP_EXT_LIB) $(BSPAPI_LINK)
export BLAPI_LDFLAGS
####################################

endif

export ECNT_EVENT_LIB_DIR=$(APP_BSP_API_DIR)/ecnt_event

export BSP_API_DBG_LIB_DIR=$(APP_BSP_API_DIR)/ecnt_lib_dbg
export BSP_API_ECNT_PRINTF_DIR=$(APP_BSP_API_DIR)/ecnt_lib_printf
export BSP_API_UTL_LIB_DIR=$(APP_BSP_API_DIR)/ecnt_lib_utility
export BSP_API_PORTBIND_LIB_DIR=$(APP_BSP_API_DIR)/ecnt_lib_portbind


BSP_API_ATM_LIB_DIR=$(APP_BSP_API_DIR)/hw_driver/atm/lib_src
BSP_API_ATM_CMD_DIR=$(APP_BSP_API_DIR)/hw_driver/atm/cmd
BSP_API_PTM_LIB_DIR=$(APP_BSP_API_DIR)/hw_driver/ptm/lib_src
BSP_API_PTM_CMD_DIR=$(APP_BSP_API_DIR)/hw_driver/ptm/cmd
BSP_API_XDSL_LIB_DIR=$(APP_BSP_API_DIR)/hw_driver/xdsl/lib_src
BSP_API_XDSL_CMD_DIR=$(APP_BSP_API_DIR)/hw_driver/xdsl/cmd
BSP_API_DBG_LIB_DIR=$(APP_BSP_API_DIR)/ecnt_lib_dbg
BSP_API_UTL_LIB_DIR=$(APP_BSP_API_DIR)/ecnt_lib_utility
BSP_API_PORTBIND_LIB_DIR=$(APP_BSP_API_DIR)/ecnt_lib_portbind
BSP_API_LIB_VOIP_DIR=$(APP_BSP_API_DIR)/hw_driver/voip/lib_src
BSP_API_FE_LIB_DIR=$(APP_BSP_API_DIR)/hw_driver/fe/lib_src
BSP_API_FE_LIBBSP_DIR=$(APP_BSP_API_DIR)/hw_driver/fe/libbsp_src
BSP_API_FE_CMD_DIR=$(APP_BSP_API_DIR)/hw_driver/fe/cmd
BSP_API_SWITCH_LIB_DIR=$(APP_BSP_API_DIR)/hw_driver/switch/lib_src
BSP_API_SWITCH_LIBBSP_DIR=$(APP_BSP_API_DIR)/hw_driver/switch/libbsp_src
BSP_API_SWITCH_CMD_DIR=$(APP_BSP_API_DIR)/hw_driver/switch/cmd
BSP_API_PPE_LIB_DIR=$(APP_BSP_API_DIR)/hw_driver/ppe/lib_src
BSP_API_PPE_LIBBSP_DIR=$(APP_BSP_API_DIR)/hw_driver/ppe/libbsp_src
BSP_API_PPE_CMD_DIR=$(APP_BSP_API_DIR)/hw_driver/ppe/cmd
BSP_API_QDMA_LIB_DIR=$(APP_BSP_API_DIR)/hw_driver/qdma/lib_src
BSP_API_QDMA_LIBBSP_DIR=$(APP_BSP_API_DIR)/hw_driver/qdma/libbsp_src
BSP_API_QDMA_CMD_DIR=$(APP_BSP_API_DIR)/hw_driver/qdma/cmd
BSP_API_HW_NAT_CMD_DIR=$(APP_BSP_API_DIR)/hw_driver/hw_nat/cmd
export BSP_API_QDMA_LIB_DIR
BSP_API_QOS_LIB_DIR = $(APP_BSP_API_DIR)/hw_driver/qos/lib_src
export BSP_API_QOS_LIB_DIR
BSP_API_QOS_CMD_DIR = $(APP_BSP_API_DIR)/hw_driver/qos/cmd
       
BSP_API_CPUTEMP_LIB_DIR = $(APP_BSP_API_DIR)/hw_driver/cpu/lib_src
export BSP_API_CPUTEMP_LIB_DIR
BSP_API_CPUTEMP_CMD_DIR = $(APP_BSP_API_DIR)/hw_driver/cpu/cmd
BSP_API_WIFI_LIB_DIR=$(APP_BSP_API_DIR)/hw_driver/wifi/lib_src
BSP_API_WIFI_CMD_DIR=$(APP_BSP_API_DIR)/hw_driver/wifi/cmd
BSP_API_CPU_INTERRUPT_LIB_DIR=$(APP_BSP_API_DIR)/hw_driver/cpu_interrupt/lib_src
BSP_API_CPU_INTERRUPT_CMD_DIR=$(APP_BSP_API_DIR)/hw_driver/cpu_interrupt/cmd
BSP_API_VOIP_DIR=$(APP_BSP_API_DIR)/hw_driver/voip/$(TCPLATFORM)
BSP_API_VOIP_SRC_DIR=$(APP_BSP_API_DIR)/hw_driver/voip
BSP_API_LIB_VDSL_DIR=$(APP_BSP_API_DIR)/hw_driver/vdsl/lib_src
BSP_API_LIB_DBG_ATM_DIR=$(APP_BSP_API_DIR)/hw_driver/dbgcmd/lib_src/atm/

export BSP_API_LIB_DBG_ATM_DIR
export BSP_API_LIB_VDSL_DIR
export BSP_API_VOIP_DIR
export BSP_API_VOIP_SRC_DIR
       
BSP_API_QOSRULE_LIB_DIR=$(APP_BSP_API_DIR)/hw_driver/qosrule/lib_src
export BSP_API_QOSRULE_LIB_DIR
BSP_API_QOSRULE_CMD_DIR=$(APP_BSP_API_DIR)/hw_driver/qosrule/cmd
export BSP_API_QOSRULE_CMD_DIR
BSP_API_IFC_LIB_DIR=$(APP_BSP_API_DIR)/hw_driver/ifc/lib_src
export BSP_API_IFC_LIB_DIR
BSP_API_IFC_CMD_DIR=$(APP_BSP_API_DIR)/hw_driver/ifc/cmd
export BSP_API_IFC_CMD_DIR
       
BSP_MESH_DIR=$(APP_BSP_API_DIR)/hw_driver/mesh
export BSP_MESH_DIR

BSP_BLAPI_DIR=$(APP_BSP_API_DIR)/ecnt_blapi
export BSP_BLAPI_DIR

#for xpon
export BSP_API_ECNT_IGMP_LIB_DIR=$(APP_BSP_API_DIR)/hw_driver/ecnt_igmp/lib_src
export BSP_API_ECNT_IGMP_CMD_DIR=$(APP_BSP_API_DIR)/hw_driver/ecnt_igmp/cmd

       