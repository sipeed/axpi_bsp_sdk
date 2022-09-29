CUR_PATH		:= $(shell pwd)
HOME_PATH		:= $(CUR_PATH)/../../..
SRC_PATH		:= $(CUR_PATH)
BASE_PATH		:= $(HOME_PATH)/msp/component
OUT_PATH                := $(HOME_PATH)/msp/out
LIB_PATH                := $(HOME_PATH)/msp/out/lib
INC_PATH                := $(HOME_PATH)/msp/out/include
COMMON_PATH		:= $(BASE_PATH)/common
ROOTFS_TARGET_PATH      ?= $(OUT_PATH)

include $(HOME_PATH)/build/config.mak

# output
MOD_NAME 		:= sample_resolution_venc
OUTPUT 			:= .obj

# source
SRCS 			:= $(wildcard $(SRC_PATH)/*.c)

CINCLUDE                := -I$(SRC_PATH) \
				-I$(INC_PATH)


OBJS 			:= $(addprefix $(OUTPUT)/,$(notdir $(patsubst %.c,%.o,$(SRCS))))
DEPS			:= $(OBJS:%.o=%.d)
# exec
TARGET			:= $(MOD_NAME)

# build flags
CFLAGS			:= -Wall -Wl,-rpath-link=$(LIB_PATH)

ifeq ($(debug),yes)
CFLAGS			+= -O0 -ggdb3
else
CFLAGS			+= -O2
endif

CFLAGS			+= -Wno-unused -Wno-pointer-sign

CLIB                    := -L$(LIB_PATH) -lax_venc \
				-lax_sys \
				-lm -lpthread \
				-laxsyslog

# install
INSTALL_BIN	:= $(TARGET)
#INSTALL_DATA	:= $(CUR_PATH)/data/venc
# link
LINK = $(CC)

include $(HOME_PATH)/build/rules.mak

