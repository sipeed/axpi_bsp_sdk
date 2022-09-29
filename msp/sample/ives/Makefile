CUR_PATH			:= $(shell pwd)
HOME_PATH			:= $(CUR_PATH)/../../..
SRC_PATH			:= $(CUR_PATH)
BASE_PATH			:= $(HOME_PATH)/msp/component
OUT_PATH			:= $(HOME_PATH)/msp/out
LIB_PATH			:= $(OUT_PATH)/lib
THIRD_PARTY_PATH  	:= $(HOME_PATH)/third-party
ROOTFS_TARGET_PATH 	?= $(OUT_PATH)
include $(HOME_PATH)/build/config.mak

MOD_NAME = sample_ives
OUTPUT 				:= .obj
# source
SRCS                := $(wildcard $(SRC_PATH)/*.c) \
                       $(wildcard $(SRC_PATH)/md/*.c) \
                       $(wildcard $(SRC_PATH)/od/*.c)

CINCLUDE            := -I$(SRC_PATH) \
                       -I$(SRC_PATH)/md \
                       -I$(SRC_PATH)/od \
                       -I$(OUT_PATH)/include

OBJS                := $(SRCS:%.c=$(OUTPUT)/%.o)
DEPS                := $(OBJS:%.o=%.d)

# exec
TARGET				:= $(MOD_NAME)

# build flags
CFLAGS				:= -Wall -Wl,-rpath-link=$(LIB_PATH)
CFLAGS              += -DSAMPLE_IVES_BUILD_VERSION=\"$(SDK_VERSION)\"
CFLAGS              += -DSAMPLE_IVES_NAME=\"$(TARGET)\"

debug = no
ifeq ($(debug),yes)
CFLAGS				+= -O0 -ggdb3
else
CFLAGS				+= -O2
endif

# dependency
CLIB				:= -L$(LIB_PATH)
CLIB				+= -lax_sys
CLIB				+= -lax_ives
CLIB				+= -pthread

# install
INSTALL_TARGET		:= $(TARGET)
INSTALL_DIR			:= $(ROOTFS_TARGET_PATH)/bin/
INSTALL_DATA    	:= $(CUR_PATH)/data/ives

# link
LINK = $(CC)

include $(HOME_PATH)/build/rules.mak
