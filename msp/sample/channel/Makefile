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
MOD_NAME        := sample_channel
OUTPUT          := .obj

# source
SRCCPPS         := $(wildcard $(SRC_PATH)/*.cpp) \
                   $(wildcard $(SRC_PATH)/master/tasks/*.cpp) \
                   $(wildcard $(SRC_PATH)/master/*.cpp) \
                   $(wildcard $(SRC_PATH)/slave/procs/*.cpp) \
                   $(wildcard $(SRC_PATH)/slave/*.cpp)

CINCLUDE       	:= -I$(OUT_PATH)/include \
                   -I$(SRC_PATH) \
                   -I$(SRC_PATH)/master \
                   -I$(SRC_PATH)/master/tasks \
                   -I$(SRC_PATH)/slave \
                   -I$(SRC_PATH)/slave/procs

OBJS			:= $(SRCCPPS:%.cpp=$(OUTPUT)/%.o)
DEPS			:= $(OBJS:%.o=%.d)


# target
TARGET			:= $(MOD_NAME)

# build flags
CFLAGS			:= -Wl,-rpath=./
ifeq ($(debug),yes)
CFLAGS          += -O0 -ggdb3
else
CFLAGS          += -O2
endif

CPPFLAGS	    := --std=c++11 $(CFLAGS)

# dependency
CLIB			:= -pthread -lm -lrt -lstdc++
CLIB			+= -L$(LIB_PATH) -lax_channel


# install
INSTALL_TARGET	:= $(TARGET)
INSTALL_DIR		:= $(ROOTFS_TARGET_PATH)/bin/

# link
LINK = $(CPP)

include $(HOME_PATH)/build/rules.mak
