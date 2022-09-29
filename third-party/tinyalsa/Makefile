CUR_PATH		   := $(shell pwd)
HOME_PATH		   := $(CUR_PATH)/../..
ROOTFS_TARGET_PATH ?= $(HOME_PATH)/rootfs/rootfs/usr/local

include $(HOME_PATH)/build/color.mk
include $(HOME_PATH)/build/config.mak

all:
	@echo -e $(GREEN)"\nBuild tinyalsa success!!\n"  $(DONE)


install:
	@mkdir -p $(ROOTFS_TARGET_PATH)/bin
	@cp -rf $(CUR_PATH)/bin/* $(ROOTFS_TARGET_PATH)/bin
	@mkdir -p $(ROOTFS_TARGET_PATH)/lib
	@cp -rf $(CUR_PATH)/lib/* $(ROOTFS_TARGET_PATH)/lib
	@echo -e $(GREEN)"\nInstall tinyalsa success!!\n"  $(DONE)

clean:
	@rm -f $(ROOTFS_TARGET_PATH)/bin/tinycap
	@rm -f $(ROOTFS_TARGET_PATH)/bin/tinymix
	@rm -f $(ROOTFS_TARGET_PATH)/bin/tinypcminfo
	@rm -f $(ROOTFS_TARGET_PATH)/bin/tinyplay
	@rm -f $(ROOTFS_TARGET_PATH)/bin/tinywavinfo
	@rm -f $(ROOTFS_TARGET_PATH)/lib/libtinyalsa.so
	@rm -f $(ROOTFS_TARGET_PATH)/lib/libtinyalsa.so.2
	@rm -f $(ROOTFS_TARGET_PATH)/lib/libtinyalsa.so.2.0.0

	@echo -e $(GREEN)"\nClean tinyalsa success!!\n"  $(DONE)







