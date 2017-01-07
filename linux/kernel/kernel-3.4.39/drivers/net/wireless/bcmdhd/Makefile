# bcmdhd
# 1. WL_IFACE_COMB_NUM_CHANNELS must be added if Android version is 4.4 with Kernel version 3.0~3.4,
#    otherwise please remove it.

TARGET = bcmdhd

#CONFIG_BCMDHD_OOB = y
CONFIG_BCMDHD_SDIO = y
CONFIG_CFG80211 = y
CONFIG_ARCH_S5P4418 = y

KDIR			:= ../../../../../../kernel/kernel-3.4.39
PWD     := $(shell pwd)

#modified by ddanggzi
#obj-m			:= $(TARGET).o
obj-$(CONFIG_BCMDHD) += bcmdhd.o
# ddanggzi end

$(TARGET)-objs	+=

KBUILD_CFLAGS += -Wall -Wstrict-prototypes -Dlinux -DBCMDRIVER -DSDTEST       \
	-DBCMDONGLEHOST -DUNRELEASEDCHIP -DBCMDMA32 -DBCMFILEIMAGE            \
	-DDHDTHREAD -DDHD_DEBUG -DSHOW_EVENTS -DBCMDBG -DGET_OTP_MAC_ENABLE   \
	-DWIFI_ACT_FRAME -DARP_OFFLOAD_SUPPORT -DSUPPORT_PM2_ONLY             \
	-DKEEP_ALIVE -DPKT_FILTER_SUPPORT -DPNO_SUPPORT -DDHDTCPACK_SUPPRESS  \
	-DDHD_DONOT_FORWARD_BCMEVENT_AS_NETWORK_PKT -DRXFRAME_THREAD          \
	-DENABLE_INSMOD_NO_FW_LOAD		       \
	-Idrivers/net/wireless/bcmdhd -Idrivers/net/wireless/bcmdhd/include

$(TARGET)-objs += aiutils.o siutils.o sbutils.o bcmutils.o bcmwifi_channels.o \
	dhd_linux.o dhd_linux_platdev.o dhd_linux_sched.o dhd_pno.o \
	dhd_common.o dhd_ip.o dhd_linux_wq.o dhd_custom_gpio.o \
	bcmevent.o hndpmu.o linux_osl.o wldev_common.o wl_android.o \
	hnd_pktq.o hnd_pktpool.o dhd_config.o

ifneq ($(CONFIG_BCMDHD_SDIO),)
KBUILD_CFLAGS += \
	-DBCMSDIO -DMMC_SDIO_ABORT -DBCMLXSDMMC -DUSE_SDIOFIFO_IOVAR          \
	-DBDC -DPROP_TXSTATUS -DDHD_USE_IDLECOUNT -DBCMSDIOH_TXGLOM           \
	-DCUSTOM_SDIO_F2_BLKSIZE=128

$(TARGET)-objs += bcmsdh.o bcmsdh_linux.o bcmsdh_sdmmc.o bcmsdh_sdmmc_linux.o \
	dhd_sdio.o dhd_cdc.o dhd_wlfc.o

ifeq ($(CONFIG_BCMDHD_OOB),y)
KBUILD_CFLAGS += -DOOB_INTR_ONLY -DHW_OOB -DCUSTOMER_OOB
ifeq ($(CONFIG_BCMDHD_DISABLE_WOWLAN),y)
KBUILD_CFLAGS += -DDISABLE_WOWLAN
endif
else
KBUILD_CFLAGS += -DSDIO_ISR_THREAD
endif
endif

ifneq ($(CONFIG_BCMDHD_PCIE),)
KBUILD_CFLAGS += \
	-DPCIE_FULL_DONGLE -DBCMPCIE -DSHOW_LOGTRACE -DDPCIE_TX_DEFERRAL      \
	-DCUSTOM_DPC_PRIO_SETTING=-1

$(TARGET)-objs += dhd_pcie.o dhd_pcie_linux.o pcie_core.o dhd_flowring.o \
	dhd_msgbuf.o
endif



#ifeq ($(CONFIG_MACH_ODROID_4210),y)
$(TARGET)-objs += dhd_gpio.o
KBUILD_CFLAGS += -DCUSTOMER_HW -DDHD_OF_SUPPORT
#KBUILD_CFLAGS += -DBCMWAPI_WPI -DBCMWAPI_WAI
#endif

#ifeq ($(CONFIG_ARCH_S5P4418),y)
$(TARGET)-objs += dhd_gpio.o
KBUILD_CFLAGS += -DCUSTOMER_HW -DDHD_OF_SUPPORT
#KBUILD_CFLAGS += -DBCMWAPI_WPI -DBCMWAPI_WAI
#endif

ifeq ($(CONFIG_BCMDHD_AG),y)
KBUILD_CFLAGS += -DBAND_AG
endif

ifeq ($(CONFIG_DHD_USE_STATIC_BUF),y)
# add dhd_static_buf to kernel image build
#$(TARGET)-objs += dhd_static_buf.o
KBUILD_CFLAGS += -DSTATIC_WL_PRIV_STRUCT -DENHANCED_STATIC_BUF
endif


ifneq ($(CONFIG_CFG80211),)
$(TARGET)-objs += wl_cfg80211.o wl_cfgp2p.o wl_linux_mon.o wl_cfg_btcoex.o
$(TARGET)-objs += dhd_cfg80211.o dhd_cfg_vendor.o
KBUILD_CFLAGS += -DWL_CFG80211 -DWLP2P -DWL_CFG80211_STA_EVENT -DWL_ENABLE_P2P_IF
KBUILD_CFLAGS += -DWL_IFACE_COMB_NUM_CHANNELS
KBUILD_CFLAGS += -DCUSTOM_ROAM_TRIGGER_SETTING=-65
KBUILD_CFLAGS += -DCUSTOM_ROAM_DELTA_SETTING=15
KBUILD_CFLAGS += -DCUSTOM_KEEP_ALIVE_SETTING=28000
KBUILD_CFLAGS += -DCUSTOM_PNO_EVENT_LOCK_xTIME=7
KBUILD_CFLAGS += -DWL_SUPPORT_AUTO_CHANNEL
KBUILD_CFLAGS += -DWL_SUPPORT_BACKPORTED_KPATCHES
KBUILD_CFLAGS += -DESCAN_RESULT_PATCH
KBUILD_CFLAGS += -DVSDB -DWL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST
KBUILD_CFLAGS += -DWLTDLS -DMIRACAST_AMPDU_SIZE=8 -DPROP_TXSTATUS_VSDB
endif
EXTRA_CFLAGS = $(KBUILD_CFLAGS)

KBUILD_CFLAGS += -DMULTIPLE_SUPPLICANT
EXTRA_LDFLAGS += --strip-debug


KBUILD_CFLAGS += -I../../platform/s5p4418/modules/bcmdhd/include
KBUILD_CFLAGS += -I../../platform/s5p4418/modules/bcmdhd

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	@find . -type f \( -name '*.ko' -o -name '*.mod.*'		\
					-o -name '*.o' -o -name '.*.cmd'		\
					-o -name '*.order' -o -name '*.symvers'	\
					-o -name '*.' \) -print | xargs rm -f
	rm -rf .tmp_versions

