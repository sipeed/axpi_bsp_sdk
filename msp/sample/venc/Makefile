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
MOD_NAME 		:= sample_venc
OUTPUT 			:= .obj

# source
SRCS 			:= $(wildcard $(SRC_PATH)/*.c) \
				$(wildcard $(SAMPLE_PATH)/common/common_codec/*.c)
vpath %.c $(addsuffix :,$(dir $(patsubst %.c,%.o,$(SRCS)))))

CINCLUDE		+= -I$(SRC_PATH) \
				-I$(OUT_PATH)/include \
				-I$(SAMPLE_PATH)/common \
				-I$(SAMPLE_PATH)/common/common_codec 

OBJS 			:= $(addprefix $(OUTPUT)/,$(notdir $(patsubst %.c,%.o,$(SRCS))))
DEPS			:= $(OBJS:%.o=%.d)
DEPS			+= $(shell awk '{gsub(/```/,"");print}' help.md | xxd -i > help.dat)
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

