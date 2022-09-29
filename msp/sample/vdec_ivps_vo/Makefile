CUR_PATH		:= $(shell pwd)
HOME_PATH		:= $(CUR_PATH)/../../..
SRC_PATH		:= $(CUR_PATH)
THIRD_PARTY_PATH        := $(HOME_PATH)/third-party
BASE_PATH		:= $(HOME_PATH)/msp/component
OUT_PATH		:= $(HOME_PATH)/msp/out
LIB_PATH		:= $(OUT_PATH)/lib
SAMPLE_PATH		:= $(CUR_PATH)/..
ROOTFS_TARGET_PATH ?= $(OUT_PATH)
include $(HOME_PATH)/build/config.mak

MOD_NAME = sample_vdec_ivps_vo
OUTPUT 			:= .obj
# source
SRCS 			:= $(wildcard $(SRC_PATH)/*.c) \
				$(wildcard $(SAMPLE_PATH)/common/vo/*.c)
vpath %.c $(addsuffix :,$(dir $(patsubst %.c,%.o,$(SRCS)))))


CINCLUDE += -I$(SRC_PATH) \
		-I$(OUT_PATH)/include \
		-I$(BASE_PATH)/codec/common \
		-I$(BASE_PATH)/sys \
		-I$(BASE_PATH)/sys/cmm \
		-I$(HOME_PATH)/third-party/openssl/include \
		-I$(SAMPLE_PATH)/common/vo

OBJS 			:= $(addprefix $(OUTPUT)/,$(notdir $(patsubst %.c,%.o,$(SRCS))))
DEPS			:= $(OBJS:%.o=%.d)
# exec
TARGET			:= $(MOD_NAME)

# build flags
CFLAGS			:= -Wall -Wl,-rpath-link=$(LIB_PATH)
debug = yes
ifeq ($(debug),yes)
CFLAGS			+= -O0 -ggdb3
else
CFLAGS			+= -O2
endif

# dependency
CLIB			+= -L$(LIB_PATH) -lax_vdec
CLIB			+= -lax_ivps
CLIB			+= -lax_vo
CLIB			+= -L$(HOME_PATH)/third-party/drm/lib
CLIB			+= -ldrm
CLIB			+= -lax_sys
CLIB			+= -lm -lpthread
CLIB			+= -laxsyslog
CLIB			+= -L$(THIRD_PARTY_PATH)/openssl/lib
CLIB			+= -lssl -lcrypto

# install
INSTALL_BIN	:= $(TARGET)
#INSTALL_DATA    := $(CUR_PATH)/data/vdec

# link
LINK = $(CC)

include $(HOME_PATH)/build/rules.mak
