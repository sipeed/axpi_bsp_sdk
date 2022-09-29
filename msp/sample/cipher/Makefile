CUR_PATH		:= $(shell pwd)
HOME_PATH		:= $(CUR_PATH)/../../..
SRC_PATH		:= $(CUR_PATH)
BASE_PATH		:= $(HOME_PATH)/msp/component
OUT_PATH		:= $(HOME_PATH)/msp/out
LIB_PATH		:= $(HOME_PATH)/msp/out/lib
COMMON_PATH		:= $(BASE_PATH)/common
ROOTFS_TARGET_PATH ?= $(OUT_PATH)

include $(HOME_PATH)/build/config.mak

# output
MOD_NAME                := sample_cipher
OUTPUT                  := obj

# source
SRCS                    := $(wildcard $(SRC_PATH)/*.c)
OBJS                    := $(SRCS:%.c=$(OUTPUT)/%.o)
CINCLUDE                := -I$(SRC_PATH) \
                           -I$(OUT_PATH)/include

# exec
TARGET                  := $(MOD_NAME)

# build flags
CFLAGS			:= -Wall -Wl,-rpath-link=$(LIB_PATH)
ifeq ($(debug),yes)
CFLAGS			+= -O0 -ggdb3
else
CFLAGS			+= -O2
endif

# dependency
CLIB			+= -L$(LIB_PATH)
CLIB			+= -lax_sys -lax_cipher
CLIB			+= -laxsyslog

# install
INSTALL_TARGET	:= $(TARGET)
INSTALL_DIR		:= $(ROOTFS_TARGET_PATH)/bin/

# link
LINK = $(CC)

include $(HOME_PATH)/build/rules.mak

