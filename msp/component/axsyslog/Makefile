CUR_PATH		:= $(shell pwd)
HOME_PATH		:= $(CUR_PATH)/../../..
OUT_PATH		:= $(HOME_PATH)/msp/out
SRC_PATH		:= $(CUR_PATH)
BASE_PATH		:= $(HOME_PATH)/msp/component
ROOTFS_TARGET_PATH 	?= $(OUT_PATH)

include $(HOME_PATH)/build/color.mk
include $(HOME_PATH)/build/config.mak

################################################################################
# build
################################################################################

.PHONY : all clean install lib
all: lib
	@echo -e $(GREEN)"\nBuild $@ success!!\n"  $(DONE)

clean:
	@echo -e $(GREEN)"\nClean $@ success!!\n"  $(DONE)

install:
	@echo -e $(GREEN)"install $@ ..."  $(DONE)

lib:
	@mkdir -p $(OUT_PATH)/lib
	@mkdir -p $(OUT_PATH)/include
	cp -r ./lib/* $(OUT_PATH)/lib
	cp -r ./include/* $(OUT_PATH)/include

export ROOTFS_TARGET_PATH
