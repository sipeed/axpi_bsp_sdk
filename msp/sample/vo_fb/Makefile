CUR_PATH		:= $(shell pwd)
HOME_PATH		:= $(CUR_PATH)/../../..
SRC_PATH		:= $(CUR_PATH)
SAMPLE_PATH		:= $(CUR_PATH)/..
BASE_PATH		:= $(HOME_PATH)/msp/component
OUT_PATH		:= $(HOME_PATH)/msp/out
LIB_PATH		:= $(HOME_PATH)/msp/out/lib
COMMON_PATH		:= $(BASE_PATH)/common
ROOTFS_TARGET_PATH ?= $(OUT_PATH)

include $(HOME_PATH)/build/config.mak

# output
MOD_NAME 		:= sample_vo_fb
OUTPUT 			:= .obj

# source
SRCS 			:= $(wildcard $(SRC_PATH)/*.c) \
				   $(wildcard $(SAMPLE_PATH)/common/vo/*.c)

CINCLUDE		:= -I$(SRC_PATH) \
				-I$(OUT_PATH)/include \
				-I$(SAMPLE_PATH)/common/vo \

OBJS 			:= $(SRCS:%.c=$(OUTPUT)/%.o)
DEPS			:= $(OBJS:%.o=%.d)
# exec
TARGET			:= $(MOD_NAME)

# build flags
CFLAGS			:= -Wall -Wl,-rpath-link=$(LIB_PATH)
CFLAGS			+= -O0 -ggdb3

# dependency
CLIB			+= -L$(HOME_PATH)/third-party/drm/lib
CLIB			+= -ldrm
CLIB			+= -L$(LIB_PATH)
CLIB			+= -lax_vo
CLIB			+= -lax_sys
CLIB			+= -laxsyslog
CLIB			+= -lm -lpthread

# install
INSTALL_TARGET	:= $(TARGET)
INSTALL_DIR	:= $(ROOTFS_TARGET_PATH)/bin/
INSTALL_DATA := $(CUR_PATH)/data/vo
# link
LINK = $(CC)

include $(HOME_PATH)/build/rules.mak
