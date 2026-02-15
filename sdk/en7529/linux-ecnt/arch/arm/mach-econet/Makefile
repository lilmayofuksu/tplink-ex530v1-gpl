obj-y := sgmii/

obj-y += \
	../../../kernel/libcompileoption.o \
	ecnt_hook/ecnt_hook.o \
	ecnt_hook/ecnt_hook_nf.o \
	ecnt_kernel_ext.o \
	ecnt_scu.o \
	tcwdog.o \
	prom.o \
	ecnt_sram.o \
	ecnt_gdma.o \
	ecnt_frame_engine.o \
	ecnt_xsi.o \
	ecnt_gpio.o \
	ecnt_rbus.o \
	ecnt_npu.o \
	ecnt_xpon.o \
	ecnt_pon_phy.o \
	ecnt_i2c.o \
	ecnt_crypto_k.o \
	arm_v7_pmu.o \
	ecnt_trng.o \
	ecnt_pcm.o \
	ecnt_cpu_top.o \
	ecnt_bootargs.o \
	ecnt_timer.o \
	ecnt_i2c_slave.o \
	ecnt_gdump.o \
	ecnt_i2s.o \
        ecnt_scu_phy.o \
        ecnt_thermal.o \
	ecnt_avs.o \
	ecnt_irqbind.o 
	
obj-$(TCSUPPORT_KPROFILE)	+= kprofile_hook.o
ifneq ($(strip $(RTP_RX_SHORTCUT)),)
obj-$(TCSUPPORT_VOIP)	+= voip_hook.o
endif
