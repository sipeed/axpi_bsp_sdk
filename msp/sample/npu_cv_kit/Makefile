CUR_PATH		:= $(shell pwd)
HOME_PATH		:= $(CUR_PATH)/../../..
SRC_PATH		:= $(CUR_PATH)
BASE_PATH		:= $(HOME_PATH)/msp/component
OUT_PATH		:= $(HOME_PATH)/msp/out
LIB_PATH		:= $(HOME_PATH)/msp/out/lib
BIN_DIR		:= $(HOME_PATH)/msp/out/bin/sample_npu_cv_kit
COMMON_PATH		:= $(BASE_PATH)/common
ROOTFS_TARGET_PATH ?= $(OUT_PATH)

include $(HOME_PATH)/build/config.mak

CXX = $(CROSS)g++

CXXFLAGS = --std=c++11 -I$(OUT_PATH)/include -I$(OUT_PATH)/include/npu_cv_kit  -g
LDFLAGS = -fPIC -Wl,-rpath,$(LIB_PATH) -L$(LIB_PATH)

SRC_DIR		:= $(SRC_PATH)
SHD_LIBS    := -lax_npu_cv_kit -lax_sys -lax_interpreter_external -laxsyslog

ALL_SAMPLE_SRCS = $(shell find . -maxdepth 1 -name "npu_*.cpp")
all: $(patsubst %.cpp, %, $(ALL_SAMPLE_SRCS))

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

%: $(BIN_DIR) %.cpp
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/$@.cpp -o $(BIN_DIR)/$@.ax620a $(SHD_LIBS) $(LDFLAGS)

clean:
	rm -rf $(BIN_DIR)

install:

.PHONY: all clean install

