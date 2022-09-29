AM_CPPFLAGS = -I$(abs_top_srcdir)/src/include -I$(abs_top_srcdir)/include \
			  -I$(abs_top_srcdir)/src/newgal/ -I$(abs_top_srcdir)/../libdrm -I$(abs_top_srcdir)/../libdrm/include/drm

SRC_FILES = aichip_drmvideo.c
HDR_FILES = aichip_drmvideo.h

noinst_LTLIBRARIES = libvideo_aichipdrm.la
libvideo_aichipdrm_la_SOURCES = $(SRC_FILES) $(HDR_FILES)

EXTRA_DIST=

LIB_NAME=libvideo_aichipdrm
