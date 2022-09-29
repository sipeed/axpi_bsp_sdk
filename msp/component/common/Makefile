SUBDIRS = include

################################################################################
#	prepare param
################################################################################
LOCAL_PATH        := $(shell pwd)
HOME_PATH         := $(LOCAL_PATH)/../../..
ROOTFS_TARGET_PATH:= $(HOME_PATH)/msp/out

include $(HOME_PATH)/build/color.mk
include $(HOME_PATH)/build/config.mak
################################################################################
#	set task
################################################################################
SUBDIRS_CLEAN   = $(addsuffix .clean, $(SUBDIRS))
SUBDIRS_INSTALL = $(addsuffix .install, $(SUBDIRS))

INC_PATH := $(LOCAL_PATH)/include
INSTALL_TARGETS := $(wildcard $(INC_PATH)/*.h)


.PHONY: $(SUBDIRS) $(SUBDIRS_INSTALL) $(SUBDIRS_CLEAN)

all: install
	@$(ECHO) -e $(GREEN)"\nBuild All common Modules success!!\n"  $(DONE)

install: $(SUBDIRS_INSTALL)
	@$(ECHO) -e $(GREEN)"\nInstall common success!!\n"  $(DONE)

clean:	$(SUBDIRS_CLEAN)
	@$(ECHO) -e $(GREEN)"\nClean common directory!!\n"  $(DONE)

$(SUBDIRS):
	@$(ECHO)
	@$(ECHO) -e $(CYAN)"In subdir $@ ..." $(DONE)
	@$(MAKE) -C $(basename $@ )

$(SUBDIRS_INSTALL): $(SUBDIRS)
	@$(ECHO)
ifneq ($(ROOTFS_TARGET_PATH)/include, $(wildcard $(ROOTFS_TARGET_PATH)/include))
	$(VERB) $(MKDIR) $(ROOTFS_TARGET_PATH)/include
endif
	@$(CP) $(INSTALL_TARGETS)  $(ROOTFS_TARGET_PATH)/include
	@$(ECHO) -e $(CYAN)"In subdir $(basename $@ )..." $(DONE)
	@$(MAKE) -C $(basename $@ ) install

$(SUBDIRS_CLEAN):
	@$(ECHO) -e $(CYAN)"In subdir $(basename $@ )..." $(DONE)
	@$(MAKE) -C $(basename $@ ) clean
