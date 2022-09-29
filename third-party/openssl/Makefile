CUR_PATH		   := $(shell pwd)
HOME_PATH		   := $(CUR_PATH)/../..
ROOTFS_TARGET_PATH ?= $(HOME_PATH)/rootfs/rootfs/usr/local

include $(HOME_PATH)/build/color.mk
include $(HOME_PATH)/build/config.mak

all:
	@echo -e $(GREEN)"\nBuild openssl success!!\n"  $(DONE)


install:
	@mkdir -p $(ROOTFS_TARGET_PATH)/lib
	@cp -rf $(CUR_PATH)/lib/* $(ROOTFS_TARGET_PATH)/lib
	@echo -e $(GREEN)"\nInstall openssl success!!\n"  $(DONE)

clean:
	@rm -f $(ROOTFS_TARGET_PATH)/lib/libcrypto.so
	@rm -f $(ROOTFS_TARGET_PATH)/lib/libcrypto.so.1.1
	@rm -f $(ROOTFS_TARGET_PATH)/lib/libssl.so
	@rm -f $(ROOTFS_TARGET_PATH)/lib/libssl.so.1.1
	@rm -f $(ROOTFS_TARGET_PATH)/lib/pkgconfig/libcrypto.pc
	@rm -f $(ROOTFS_TARGET_PATH)/lib/pkgconfig/libssl.pc
	@rm -f $(ROOTFS_TARGET_PATH)/lib/pkgconfig/openssl.pc
	@rm -f $(ROOTFS_TARGET_PATH)/lib/engines-1.1/capi.so
	@rm -f $(ROOTFS_TARGET_PATH)/lib/engines-1.1/padlock.so
	@echo -e $(GREEN)"\nClean openssl success!!\n"  $(DONE)







