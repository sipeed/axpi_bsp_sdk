CUR_PATH		:= $(shell pwd)
HOME_PATH		:= $(CUR_PATH)/../../../..
SRC_PATH		:= $(CUR_PATH)
ROOTFS_TARGET_PATH ?= $(HOME_PATH)/msp/out

include $(HOME_PATH)/build/config.mak

# output
MOD_NAME 		:=
OUTPUT 			:= .obj

# source
SRCS 			:=
CINCLUDE		:=	-I$(HOME_PATH)/msp/component/sys
CINCLUDE		:=	-I$(HOME_PATH)/msp/component/codec/common

OBJS 			:= $(SRCS:%.c=$(OUTPUT)/%.o)
DEPS			:= $(OBJS:%.o=%.d)

# build flags
CFLAGS			:= -shared -fPIC
ifeq ($(debug),yes)
CFLAGS			+= -Wall -O0 -g
else
CFLAGS			+= -Wall -O2
endif

# dependency
CLIB			+=

# install
INSTALL_INC		:= $(SRC_PATH)/ax_base_type.h
INSTALL_INC		+= $(SRC_PATH)/ax_global_type.h
# link
LINK = $(CC)

include $(HOME_PATH)/build/rules.mak
