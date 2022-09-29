CUR_PATH				:= $(shell pwd)
HOME_PATH				:= $(CUR_PATH)/../../..
SRC_PATH				:= $(CUR_PATH)
SAMPLE_PATH				:= $(CUR_PATH)/..
BASE_PATH				:= $(HOME_PATH)/msp/component
OUT_PATH				:= $(HOME_PATH)/msp/out
THIRD_PARTY_PATH		:= $(HOME_PATH)/third-party
LIB_PATH				:= $(HOME_PATH)/msp/out/lib
INC_PATH				:= $(HOME_PATH)/msp/out/include
COMMON_PATH				:= $(BASE_PATH)/common
NPU_PATH				:= $(BASE_PATH)/npu
ROOTFS_TARGET_PATH		?= $(OUT_PATH)

include $(HOME_PATH)/build/config.mak

# output
MOD_NAME 		:= sample_jpegenc
OUTPUT 			:= .obj

# source
SRCS 			:= $(wildcard $(SRC_PATH)/*.c)
#SRCS 			+= $(wildcard $(SAMPLE_PATH)/common/*.c)

CINCLUDE        := -I$(SRC_PATH) \
					-I$(INC_PATH)

#CINCLUDE        += -I$(SAMPLE_PATH)/common

OBJS 			:= $(SRCS:%.c=$(OUTPUT)/%.o)
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

CLIB            := -L$(LIB_PATH) \
					-lax_venc \
					-lax_sys \
					-lax_interpreter_external \
					-lm -lpthread \
					-laxsyslog

#CLIB            += -lax_mipi
#CLIB            += -lax_isp
#CLIB            += -ldpu
#CLIB            += -lax_sensor_interface


# install
INSTALL_BIN	:= $(TARGET)
# INSTALL_DATA    := $(CUR_PATH)/data/jpegenc
# link
LINK = $(CC)

include $(HOME_PATH)/build/rules.mak

