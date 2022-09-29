CUR_PATH		   := $(shell pwd)
HOME_PATH		   := $(CUR_PATH)/../..
ROOTFS_TARGET_PATH := $(HOME_PATH)/msp/out

include $(HOME_PATH)/build/color.mk
include $(HOME_PATH)/build/config.mak

all:
	@echo -e $(GREEN)"\nBuild ai-sdk success!!\n"  $(DONE)


install:
	@mkdir -p $(ROOTFS_TARGET_PATH)/lib
	@mkdir -p $(ROOTFS_TARGET_PATH)/etc/models

	@cp -rf $(CUR_PATH)/sdk/lib/*.so $(ROOTFS_TARGET_PATH)/lib

ifeq ($(findstring AX620U,${PROJECT}), AX620U)
	@cp -rf $(CUR_PATH)/sdk/models/*ax620u*.pkg $(ROOTFS_TARGET_PATH)/etc/models
	@cp -rf $(CUR_PATH)/sdk/models/lite/ $(ROOTFS_TARGET_PATH)/etc/models
	@echo -e $(GREEN)"\nInstall ai-sdk 620U success!!\n"  $(DONE)
else
	@cp -rf $(CUR_PATH)/sdk/models/*ax620a*.pkg $(ROOTFS_TARGET_PATH)/etc/models
	@echo -e $(GREEN)"\nInstall ai-sdk success!!\n"  $(DONE)
endif

clean:
	@rm -f $(ROOTFS_TARGET_PATH)/lib/libmcvsdk_*.so
	@rm -f $(ROOTFS_TARGET_PATH)/lib/libmegsearch.so
	@rm -f $(ROOTFS_TARGET_PATH)/etc/models/*.pkg
	@echo -e $(GREEN)"\nClean ai-sdk success!!\n"  $(DONE)







