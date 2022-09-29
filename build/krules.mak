#  -----------------------------------------------------------------------------
#  Desc: TARGET
#  -----------------------------------------------------------------------------

ccflags-y += -DBUILD_AXVERSION=\"$(SDK_VERSION)\"

EXTRA_CFLAGS += -Wno-error=date-time
EXTRA_CFLAGS += -D$(CHIP_NAME)

ifeq ($(CASCADAE_TYPE), SLAVE)
DEBUG_OUT_PATH := $(ROOTFS_TARGET_PATH)/slave_debug
else
DEBUG_OUT_PATH := $(ROOTFS_TARGET_PATH)/debug
endif

all: modules

.PHONY: modules install clean

modules:
	@$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) KCFLAGS=$(KCFLAGS) -C $(KDIR) M=$(shell pwd) $@

install: modules
ifeq ($(debugko),yes)
	@$(CP) $(MODULE_NAME).ko $(MODULE_NAME).debug
ifneq ($(DEBUG_OUT_PATH), $(wildcard $(DEBUG_OUT_PATH)))
	$(VERB) $(MKDIR) $(DEBUG_OUT_PATH)
endif
	@$(CP) $(MODULE_NAME).debug $(DEBUG_OUT_PATH) -rf
endif
	$(CROSS)strip --strip-debug $(MODULE_NAME).ko
ifneq ($(ROOTFS_TARGET_PATH)/ko, $(wildcard $(ROOTFS_TARGET_PATH)/ko))
	$(VERB) $(MKDIR) $(ROOTFS_TARGET_PATH)/ko
endif
ifeq ($(CASCADAE_TYPE), SLAVE)
ifneq ($(ROOTFS_TARGET_PATH)/slave, $(wildcard $(ROOTFS_TARGET_PATH)/slave))
	@cp $(MODULE_NAME).ko $(ROOTFS_TARGET_PATH)/ko/ -rf
	@echo -e "\e[36;1m" "INSTALL  $(MODULE_NAME).ko to $(ROOTFS_TARGET_PATH)/ko" "\033[0m"
else
	@cp $(MODULE_NAME).ko $(ROOTFS_TARGET_PATH)/slave/ko/ -rf
	@echo -e "\e[36;1m" "INSTALL  $(MODULE_NAME).ko to $(ROOTFS_TARGET_PATH)/slave/ko" "\033[0m"
endif
else
	@cp $(MODULE_NAME).ko $(ROOTFS_TARGET_PATH)/ko/ -rf
	@echo -e "\e[36;1m" "INSTALL  $(MODULE_NAME).ko to $(ROOTFS_TARGET_PATH)/ko" "\033[0m"
endif



clean:
	@rm -rf $(clean-objs) *.o *~ .depend .*.cmd  *.mod.c .tmp_versions *.ko *.symvers modules.order $(MODULE_NAME).debug
ifneq ($(ROOTFS_TARGET_PATH)/slave, $(wildcard $(ROOTFS_TARGET_PATH)/slave))
	@rm -rf $(ROOTFS_TARGET_PATH)/ko/$(MODULE_NAME).ko
else
	@rm -rf $(ROOTFS_TARGET_PATH)/slave/ko/$(MODULE_NAME).ko
endif
ifeq ($(debugko),yes)
	@rm -rf $(DEBUG_OUT_PATH)
endif
