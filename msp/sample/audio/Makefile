CUR_PATH		:= $(shell pwd)
HOME_PATH		:= $(CUR_PATH)/../../..
SRC_PATH		:= $(CUR_PATH)
BASE_PATH		:= $(HOME_PATH)/msp/component
SAMPLE_PATH		:= $(CUR_PATH)/..
OUT_PATH        := $(HOME_PATH)/msp/out
COMMON_PATH     := $(HOME_PATH)/msp/component/common

ROOTFS_TARGET_PATH ?= $(OUT_PATH)

include $(HOME_PATH)/build/config.mak

# output
MOD_NAME 		:= sample_audio
OUTPUT 			:= .obj

# source
SRCS 			:= $(wildcard $(SRC_PATH)/*.c)

CINCLUDE		+= -I$(SRC_PATH) \
				-I$(OUT_PATH)/include \
				-I$(BASE_PATH)/sys \
				-I$(COMMON_PATH)/include \
				-I$(HOME_PATH)/third-party/tinyalsa/include \
				-I$(HOME_PATH)/third-party/libsamplerate/include

OBJS			:= $(addprefix $(OUTPUT)/,$(notdir $(patsubst %.c,%.o,$(SRCS))))
DEPS			:= $(OBJS:%.o=%.d)
# exec
TARGET			:= $(MOD_NAME)

# build flags
ifeq ($(debug),yes)
CFLAGS			+= -Wall -O0 -ggdb3
else
CFLAGS			+= -Wall -O2
endif

# dependency
#CLIB			+= -Wl,-rpath-link=$(OUT_PATH)/lib
CLIB			+= -L$(OUT_PATH)/lib
CLIB			+= -L$(HOME_PATH)/third-party/tinyalsa/lib
CLIB			+= -ltinyalsa
CLIB			+= -L$(HOME_PATH)/third-party/libsamplerate/lib
CLIB			+= -lsamplerate
CLIB			+= -lax_audio
CLIB			+= -lax_audio_3a
CLIB			+= -lax_sys
CLIB			+= -lm -lpthread
CLIB			+= -laxsyslog


# install
INSTALL_BIN	:= $(TARGET)

# link
LINK = $(CC)

include $(HOME_PATH)/build/rules.mak
