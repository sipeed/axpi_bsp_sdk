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
MOD_NAME 		:= run_joint
OUTPUT 			:= .obj

# source
SRCS 			:= $(wildcard $(SRC_PATH)/*.cpp)

CINCLUDE		:= -I$(OUT_PATH)/include

OBJS 			:= $(addprefix $(OUTPUT)/,$(notdir $(patsubst %.cpp,%.o,$(SRCS))))
DEPS			:= $(OBJS:%.o=%.d)
# exec
TARGET			:= $(MOD_NAME)

# build flags
CFLAGS			:= -Wall -DAX620A -Wl,-rpath-link=$(LIB_PATH)
ifeq ($(debug),yes)
CFLAGS			+= -O0 -ggdb3
else
CFLAGS			+= -O2
endif

CFLAGS			+= -laxsyslog

# dependency
CLIB			+= -L$(LIB_PATH)
CLIB			+= -lm -lpthread -lax_run_joint -lax_interpreter_external -lax_interpreter -lax_sys -laxsyslog -lstdc++fs -ldl

# install
INSTALL_TARGET	:= $(TARGET)
INSTALL_DIR		:= $(ROOTFS_TARGET_PATH)/bin/

# link
LINK = $(CPP)

include $(HOME_PATH)/build/rules.mak
