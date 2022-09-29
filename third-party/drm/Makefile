CUR_PATH		   := $(shell pwd)
HOME_PATH		   := $(CUR_PATH)/../..
ROOTFS_TARGET_PATH ?= $(HOME_PATH)/rootfs/rootfs/usr/local

include $(HOME_PATH)/build/color.mk
include $(HOME_PATH)/build/config.mak

all:
	@echo -e $(GREEN)"\nBuild drm success!!\n"  $(DONE)


install:
	@mkdir -p $(ROOTFS_TARGET_PATH)/bin
	@cp -rf $(CUR_PATH)/bin/* $(ROOTFS_TARGET_PATH)/bin
	@mkdir -p $(ROOTFS_TARGET_PATH)/lib
	@cp -rf $(CUR_PATH)/lib/* $(ROOTFS_TARGET_PATH)/lib
	@echo -e $(GREEN)"\nInstall drm success!!\n"  $(DONE)

clean:
	@rm -f $(ROOTFS_TARGET_PATH)/bin/modeprint
	@rm -f $(ROOTFS_TARGET_PATH)/bin/modetest
	@rm -f $(ROOTFS_TARGET_PATH)/bin/proptest
	@rm -f $(ROOTFS_TARGET_PATH)/bin/vbltest
	@rm -f $(ROOTFS_TARGET_PATH)/lib/libdrm.so
	@rm -f $(ROOTFS_TARGET_PATH)/lib/libdrm.so.2

	@echo -e $(GREEN)"\nClean drm success!!\n"  $(DONE)







