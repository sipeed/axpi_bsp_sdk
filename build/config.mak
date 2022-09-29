TOPDIR 			:= $(PWD)/../../
BASEDIR			:= $(TOPDIR)src/
INCLUDE_PATH 	:= $(TOPDIR)include
LIBS_PATH		:= $(TOPDIR)libs
KERNEL_DIR		:= linux-4.19.125

CROSS	:= arm-linux-gnueabihf-
CC	= $(CROSS)gcc
CPP	= $(CROSS)g++
LD	= $(CROSS)ld
AR	= $(CROSS)ar -rcs
OBJCPOY = $(CROSS)objcopy
STRIP = $(CROSS)strip

VERB    = @
RM	    = rm -f
MKDIR   = mkdir -p
ECHO    = echo
MV		= mv
LN      = ln -sf
CP      = cp -f
TAR           = tar
ARCH	= arm
STATIC_FLAG  := -fPIC
DYNAMIC_FLAG := -shared -fPIC

include $(shell dirname $(lastword $(MAKEFILE_LIST)))/project.mak
include $(shell dirname $(lastword $(MAKEFILE_LIST)))/version.mak

-include $(shell dirname $(lastword $(MAKEFILE_LIST)))/projects/$(PROJECT).mak