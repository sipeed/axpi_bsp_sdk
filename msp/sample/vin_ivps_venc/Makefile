CUR_PATH		:= $(shell pwd)
HOME_PATH		:= $(CUR_PATH)/../../..
SRC_PATH		:= $(CUR_PATH)
SAMPLE_PATH		:= $(CUR_PATH)/..
BASE_PATH		:= $(HOME_PATH)/msp/component
OUT_PATH		:= $(HOME_PATH)/msp/out

LIB_PATH			:= $(HOME_PATH)/msp/out/lib
#INC_PATH			:= $(HOME_PATH)/msp/out/include
#INC_PATH			+= $(HOME_PATH)/msp/sample/common_codec
COMMON_PATH			:= $(BASE_PATH)/common
COMMON_PATH			:= $(BASE_PATH)/common/common_codec
ROOTFS_TARGET_PATH	?= $(OUT_PATH)

include $(HOME_PATH)/build/config.mak

# output
MOD_NAME 		:= sample_vin_ivps_venc
OUTPUT 			:= .obj

# source
SRCS 			:= $(wildcard $(SRC_PATH)/*.c) \
				$(wildcard $(SAMPLE_PATH)/common/*.c) \
				$(wildcard $(SAMPLE_PATH)/common/common_codec/*.c)
vpath %.c $(addsuffix :,$(dir $(patsubst %.c,%.o,$(SRCS)))))

CINCLUDE		+= -I$(SRC_PATH) \
				-I$(OUT_PATH)/include \
				-I$(SAMPLE_PATH)/common \
				-I$(SAMPLE_PATH)/common/common_codec

OBJS 			:= $(addprefix $(OUTPUT)/,$(notdir $(patsubst %.c,%.o,$(SRCS))))
DEPS			:= $(OBJS:%.o=%.d)
# exec
TARGET			:= $(MOD_NAME)

# build flags
#CFLAGS			:= -DDISABLE_NPU
ifeq ($(debug),yes)
CFLAGS			+= -Wall -O0 -ggdb3
else
CFLAGS			+= -Wall -O2
endif

CFLAGS			+= -DISP_PIPELINE_INVOLE_NPU

CFLAGS			+= -ldl -laxsyslog

# dependency
CLIB			+= -Wl,-rpath-link=$(OUT_PATH)/lib
CLIB			+= -L$(OUT_PATH)/lib
CLIB			+= -lax_venc
CLIB			+= -lax_ivps
CLIB			+= -lax_sys
CLIB			+= -lax_3a
CLIB			+= -lax_proton
CLIB			+= -lax_interpreter_external
CLIB			+= -lax_mipi
CLIB			+= -lm -lpthread
CLIB			+= -laxsyslog

#This position need add all sensor's libxxx.so
CLIB			+= -lsns_os04a10_master
CLIB			+= -lsns_os04a10_slave
CLIB			+= -lsns_os04a10
# CLIB			+= -lsns_imx334
CLIB			+= -lsns_gc4653
# CLIB			+= -lsns_os08a20

ifneq ($(preview), no)
CLIB			+= -lax_nt_stream
CLIB			+= -lax_nt_ctrl
CFLAGS			+= -DTUNING_CTRL
endif


# install
INSTALL_BIN	:= $(TARGET)

# link
LINK = $(CC)

include $(HOME_PATH)/build/rules.mak

export ROOTFS_TARGET_PATH
