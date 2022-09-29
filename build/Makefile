include color.mk
include config.mak
############################################################################
#	prepare param
############################################################################
HOME_PATH 			:= $(shell pwd)/..

COMPILE_TIME 		 = $(shell date +"%Y%m%d%H%M")
ifeq ($(SUPPORT_RAMDISK), TRUE)
ROOTFS_TARGET_PATH  ?= $(BUILD_PATH)/out/$(PROJECT)/rootfs/opt
else
ifeq ($(620U_LITTLE_FLASH), TRUE)
ROOTFS_TARGET_PATH	?= $(BUILD_PATH)/out/$(PROJECT)/rootfs/opt
else
ROOTFS_TARGET_PATH	?= $(HOME_PATH)/rootfs/rootfs/opt
endif
endif

# Base dir
BL1_PATH			:= $(HOME_PATH)/boot/bl1
UBOOT_PATH			:= $(HOME_PATH)/boot/uboot
KERNEL_PATH			:= $(HOME_PATH)/kernel/linux
OSDRV_PATH			:= $(HOME_PATH)/kernel/osdrv
MSP_PATH			:= $(HOME_PATH)/msp
ifeq ($(SUPPORT_RAMDISK), TRUE)
ROOTFS_PATH			:= $(HOME_PATH)/rootfs/ramdisk
else
ifeq ($(620U_LITTLE_FLASH), TRUE)
ROOTFS_PATH			:= $(HOME_PATH)/rootfs/rootfs_620u
else
ROOTFS_PATH			:= $(HOME_PATH)/rootfs
endif
endif
APP_PATH			:= $(HOME_PATH)/app
BUILD_PATH			:= $(HOME_PATH)/build
TOOLS_PATH			:= $(HOME_PATH)/tools
OSAL_PATH			:= $(HOME_PATH)/osal
THIRD_PARTY_PATH	:= $(HOME_PATH)/third-party

ifeq ($(PROJECT), AX620_slave)
SUBDIRS	  			:=$(BL1_PATH) \
					$(KERNEL_PATH) \
					$(OSDRV_PATH)/private_drv \
					$(OSDRV_PATH)/ext_drv \
					$(MSP_PATH)/component/common \
					$(MSP_PATH)/component/isp_proton \
					$(MSP_PATH)/sample \
					$(HOME_PATH)/osal/linux \
					$(THIRD_PARTY_PATH)/ai-sdk \
					$(APP_PATH) \
					$(ROOTFS_PATH)
else
SUBDIRS	  			:=$(BL1_PATH) \
					$(UBOOT_PATH)  \
					$(ROOTFS_PATH) \
					$(KERNEL_PATH) \
					$(OSDRV_PATH)/private_drv \
					$(OSDRV_PATH)/ext_drv \
					$(MSP_PATH)/component/common \
					$(MSP_PATH)/component/isp_proton \
					$(MSP_PATH)/sample \
					$(HOME_PATH)/osal/linux \
					$(THIRD_PARTY_PATH)/ai-sdk \
					$(APP_PATH)
endif

############################################################################
#	build
############################################################################
SUBDIRS_ALL     = $(addsuffix .all, $(SUBDIRS))
SUBDIRS_CLEAN   = $(addsuffix .clean, $(SUBDIRS))
SUBDIRS_INSTALL = $(addsuffix .install, $(SUBDIRS))

.PHONY: $(SUBDIRS) $(SUBDIRS_ALL) $(SUBDIRS_INSTALL) $(SUBDIRS_CLEAN)

default: $(SUBDIRS)

ifeq ($(FORCE_CLEAN), TRUE)
all: clean $(SUBDIRS_ALL)
	@echo -e $(GREEN)"\nBuild All Modules success!!\n"  $(DONE)
else
all: $(SUBDIRS_ALL)
	@echo -e $(GREEN)"\nBuild All Modules success!!\n"  $(DONE)
endif

install: $(SUBDIRS_INSTALL)
	@echo -e $(CYAN)"\nInstall MSP and OSDRV out to rootfs!!\n"  $(DONE)
	@cp -rf $(MSP_PATH)/out/bin/*	        $(ROOTFS_TARGET_PATH)/bin
	@cp -rf $(MSP_PATH)/out/etc/*	        $(ROOTFS_TARGET_PATH)/etc
	@cp -rf $(MSP_PATH)/out/lib/*.so*	    $(ROOTFS_TARGET_PATH)/lib
ifeq ($(PROJECT), AX620_slave)
	@cp -rf $(OSDRV_PATH)/out/slave/ko/*	$(ROOTFS_TARGET_PATH)/ko
else
	@cp -rf $(OSDRV_PATH)/out/ko/*	        $(ROOTFS_TARGET_PATH)/ko
endif
	@cp -rf $(MSP_PATH)/out/data/*	        $(ROOTFS_TARGET_PATH)/data

	@$(MAKE) -C $(ROOTFS_PATH) image install
	@echo -e $(GREEN)"\nInstall success!!\n"  $(DONE)

clean:	$(SUBDIRS_CLEAN)
	@rm -rf $(BUILD_PATH)/out/$(PROJECT)
	@echo -e $(GREEN)"\nClean success!!\n"  $(DONE)


axp:
	@bash axp_make.sh $(PROJECT) $(SDK_VERSION)
	@echo -e $(GREEN)"\nMake AXP success!!\n"  $(DONE)

$(SUBDIRS):
	@$(ECHO)
	@$(ECHO) -e $(CYAN)"In subdir $@ ..." $(DONE)
	@$(MAKE) -C $(basename $@ )

$(SUBDIRS_ALL):
	@$(ECHO) -e $(CYAN)"Making $(notdir $@)" $(DONE)
	@$(ECHO) -e $(CYAN)"In subdir $(basename $(notdir $@) )..." $(DONE)
	@$(MAKE) -C $(basename $@ ) all

$(SUBDIRS_INSTALL):
	@$(ECHO)
	@$(ECHO) -e $(CYAN)"In subdir $(basename $(notdir $@) )..." $(DONE)
	@$(MAKE) -C $(basename $@ ) install

$(SUBDIRS_CLEAN):
	@$(ECHO) -e $(CYAN)"In subdir $(basename $(notdir $@) )..." $(DONE)
	@$(MAKE) -C $(basename $@ ) clean



