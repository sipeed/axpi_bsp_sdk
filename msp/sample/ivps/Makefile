CUR_PATH			:= $(shell pwd)
HOME_PATH			:= $(CUR_PATH)/../../..
SRC_PATH			:= $(CUR_PATH)
BASE_PATH			:= $(HOME_PATH)/msp/component
OUT_PATH			:= $(HOME_PATH)/msp/out
LIB_PATH			:= $(OUT_PATH)/lib
THIRD_PARTY_PATH  	:= $(HOME_PATH)/third-party
ROOTFS_TARGET_PATH 	?= $(OUT_PATH)
include $(HOME_PATH)/build/config.mak

MOD_NAME = sample_ivps
OUTPUT 				:= .obj
# source
SRCS 				:= $(wildcard $(SRC_PATH)/*.c)
    				#   $(wildcard $(SRC_PATH)/../common/*.c)


CINCLUDE 			:= -I$(SRC_PATH) \
						-I$(OUT_PATH)/include \
						-I$(SRC_PATH)/../common/include

OBJS 				:= $(addprefix $(OUTPUT)/,$(notdir $(patsubst %.c,%.o,$(SRCS))))
DEPS				:= $(OBJS:%.o=%.d)

# exec
TARGET				:= $(MOD_NAME)

# build flags
CFLAGS				:= -Wall -Wl,-rpath-link=$(LIB_PATH)

debug = no
ifeq ($(debug),yes)
CFLAGS				+= -O0 -ggdb3
else
CFLAGS				+= -O2
endif


# ihal 3a 3a+ sensor mipi calibdb pre_isp post_isp utils isp_firmware
# dependency
CLIB				+= -L$(LIB_PATH)
CLIB				+= -lax_ivps
CLIB				+= -lax_sys
CLIB				+= -lm -lpthread -lrt -ldl
CLIB				+= -laxsyslog
CLIB			+= -lax_venc
# install
INSTALL_TARGET		:= $(TARGET)
INSTALL_DIR			:= $(ROOTFS_TARGET_PATH)/bin/
INSTALL_DATA    	:= $(CUR_PATH)/data/ivps

# link
LINK = $(CC)

include $(HOME_PATH)/build/rules.mak
