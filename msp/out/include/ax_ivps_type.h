/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#ifndef _AX_IVPS_TYPE_H_
#define _AX_IVPS_TYPE_H_
#include "ax_global_type.h"

/* When the common error code of AX_ERR_CODE_E cannot meet the requirements,
 the module error code can be extended in the following places,
 and the value should start with ox80 */
typedef enum
{
    IVPS_ERR_BUTT
} AX_IVPS_ERR_CODE_E;

#define IVPS_SUCC 0x00

#define SUB_ID_IVPS          0x01
#define SUB_ID_RGN      0x02
#define SUB_ID_GDC      0x03

#define AX_ERROR_IVPS(e) AX_DEF_ERR(AX_ID_IVPS, SUB_ID_IVPS, (e))
#define AX_ERROR_RGN(e)  AX_DEF_ERR(AX_ID_IVPS, SUB_ID_RGN, (e))
#define AX_ERROR_GDC(e)  AX_DEF_ERR(AX_ID_IVPS, SUB_ID_GDC, (e))


/*******************************IVPS ERROR CODE************************************/
/* IVPS module is not exist, maybe IVPS module is not initial */
#define AX_ERR_IVPS_INVALID_MODID                 AX_ERROR_IVPS(AX_ERR_INVALID_MODID)
/* device node is invalid, plz check /dev/tdp* */
#define AX_ERR_IVPS_INVALID_DEVID                 AX_ERROR_IVPS(AX_ERR_INVALID_DEVID)
/* this channel is not exist */
#define AX_ERR_IVPS_INVALID_CHNID                 AX_ERROR_IVPS(AX_ERR_INVALID_CHNID)
/* at lease one parameter is illagal ,eg. an illegal enumeration value */
#define AX_ERR_IVPS_ILLEGAL_PARAM                 AX_ERROR_IVPS(AX_ERR_ILLEGAL_PARAM)
/* used a NULL point */
#define AX_ERR_IVPS_NULL_PTR                      AX_ERROR_IVPS(AX_ERR_NULL_PTR)
/* bad address, eg. used for copy_from_user & copy_to_user */
#define AX_ERR_IVPS_BAD_ADDR                      AX_ERROR_IVPS(AX_ERR_BAD_ADDR)
/* system is not ready,had not initialed or loaded*/
#define AX_ERR_IVPS_SYS_NOTREADY                  AX_ERROR_IVPS(AX_ERR_SYS_NOTREADY)
/* ivps channel is busy */
#define AX_ERR_IVPS_BUSY                          AX_ERROR_IVPS(AX_ERR_BUSY)
/* initilize error */
#define AX_ERR_IVPS_NOT_INIT                      AX_ERROR_IVPS(AX_ERR_NOT_INIT)
/* try to enable or initialize system, device or channel, before configing attribute */
#define AX_ERR_IVPS_NOT_CONFIG                    AX_ERROR_IVPS(AX_ERR_NOT_CONFIG)
/* operation is not supported by NOW */
#define AX_ERR_IVPS_NOT_SUPPORT                   AX_ERROR_IVPS(AX_ERR_NOT_SUPPORT)
/* operation is not permitted, eg. try to change stati attribute */
#define AX_ERR_IVPS_NOT_PERM                      AX_ERROR_IVPS(AX_ERR_NOT_PERM)
/* channel exists */
#define AX_ERR_IVPS_EXIST                         AX_ERROR_IVPS(AX_ERR_EXIST)
/* the channle is not existed */
#define AX_ERR_IVPS_UNEXIST                       AX_ERROR_IVPS(AX_ERR_UNEXIST)
/* failure caused by malloc memory */
#define AX_ERR_IVPS_NOMEM                         AX_ERROR_IVPS(AX_ERR_NOMEM)
/* failure caused by getting block from pool */
#define AX_ERR_IVPS_NOBUF                         AX_ERROR_IVPS(AX_ERR_NOBUF)
/* buffer contains no data */
#define AX_ERR_IVPS_BUF_EMPTY                     AX_ERROR_IVPS(AX_ERR_BUF_EMPTY)
/* buffer contains fresh data */
#define AX_ERR_IVPS_BUF_FULL                      AX_ERROR_IVPS(AX_ERR_BUF_FULL)
/* failed to read as fifo is empty */
#define AX_ERR_IVPS_QUEUE_EMPTY                   AX_ERROR_IVPS(AX_ERR_QUEUE_EMPTY)
/* failed to write as fifo is full */
#define AX_ERR_IVPS_QUEUE_FULL                    AX_ERROR_IVPS(AX_ERR_QUEUE_FULL)
/* wait timeout failed */
#define AX_ERR_IVPS_TIMED_OUT                     AX_ERROR_IVPS(AX_ERR_TIMED_OUT)
/* process termination */
#define AX_ERR_IVPS_FLOW_END                      AX_ERROR_IVPS(AX_ERR_FLOW_END)
/* for ivps unknown error */
#define AX_ERR_IVPS_UNKNOWN                       AX_ERROR_IVPS(AX_ERR_UNKNOWN)


/*******************************IVPS RGN ERROR CODE*********************************/
/* this region handle id invalid */
#define AX_ERR_IVPS_RGN_INVALID_GRPID             AX_ERROR_RGN(AX_ERR_INVALID_GRPID)
/* this region has been attached and in busy state */
#define AX_ERR_IVPS_RGN_BUSY                      AX_ERROR_RGN(AX_ERR_BUSY)
/* this region has been destroy */
#define AX_ERR_IVPS_RGN_UNEXIST                   AX_ERROR_RGN(AX_ERR_UNEXIST)
/* at lease one parameter is illagal */
#define AX_ERR_IVPS_RGN_ILLEGAL_PARAM             AX_ERROR_RGN(AX_ERR_ILLEGAL_PARAM)
/* failure caused by getting block from pool */
#define AX_ERR_IVPS_RGN_NOBUF                     AX_ERROR_RGN(AX_ERR_NOBUF)

#define AX_IVPS_MIN_IMAGE_WIDTH 32
#define AX_IVPS_MAX_IMAGE_WIDTH 4096

#define AX_IVPS_MIN_IMAGE_HEIGHT 32
#define AX_IVPS_MAX_IMAGE_HEIGHT 4096

#define AX_IVPS_MAX_HW_CHN_NUM 32

#define TDP_CHN_NUM_MAX AX_IVPS_MAX_HW_CHN_NUM
#define GDC_CHN_NUM_MAX AX_IVPS_MAX_HW_CHN_NUM
#define VO_CHN_NUM_MAX AX_IVPS_MAX_HW_CHN_NUM

#define TDP_MAX_OSD_NUM  5
#define TDP_MAX_MOSAIC_NUM 16

/*mosiac*/
typedef enum
{
    AX_IVPS_MOSAIC_BLK_SIZE_2 = 0, /* block size 2*2 of MOSAIC */
    AX_IVPS_MOSAIC_BLK_SIZE_4,     /* block size 4*4 of MOSAIC */
    AX_IVPS_MOSAIC_BLK_SIZE_8,     /* block size 8*8 of MOSAIC */
    AX_IVPS_MOSAIC_BLK_SIZE_16,    /* block size 16*16 of MOSAIC */
    AX_IVPS_MOSAIC_BLK_SIZE_32,    /* block size 32*32 of MOSAIC */
    AX_IVPS_MOSAIC_BLK_SIZE_64,    /* block size 64*64 of MOSAIC */
    AX_IVPS_MOSAIC_BLK_SIZE_BUTT
} AX_IVPS_MOSAIC_BLK_SIZE_E;

typedef enum
{
    AX_IVPS_ROTATION_0   = 0,
    AX_IVPS_ROTATION_90  = 1,
    AX_IVPS_ROTATION_180 = 2,
    AX_IVPS_ROTATION_270 = 3,
    AX_IVPS_ROTATION_BUTT
} AX_IVPS_ROTATION_E;

typedef enum
{
    AX_IVPS_ASPECT_RATIO_STRETCH = 0,  /* fill according to output image size */
    AX_IVPS_ASPECT_RATIO_AUTO = 1, /* aspect ration according to horizontal and vertical alignment specified by eAligns param of IVPS_ASPECT_RATIO_S structure */
    AX_IVPS_ASPECT_RATIO_MANUAL = 2,
    AX_IVPS_ASPECT_RATIO_BUTT
} AX_IVPS_ASPECT_RATIO_E;

typedef enum
{
    /*
if (src_w / src_h < dst_w / dst_h)
w_ratio = h_ratio = dst_h * 1.0 / src_h
else
w_ratio = h_ratio = dst_w * 1.0 / src_w

Example1:
src: 1920x1080  dst: 1920x1140
IVPS_ASPECT_RATIO_HORIZONTAL_CENTER
IVPS_ASPECT_RATIO_VERTICAL_BOTTOM
then output image:
|   fill area (0, 0, 1920, 60)      |  0
|-----------------------------------| 60
|                                   |
|   Image area(0, 60, 1920, 1140)   |
|                                   |
|                                   | 1140

Example2:
src: 1920x1080  dst: 1980x1080
IVPS_ASPECT_RATIO_HORIZONTAL_LEFT
IVPS_ASPECT_RATIO_VERTICAL_CENTER
then output image:
0                            1920   1980
|                              |      |
|      Image Area              | Fill |
|      (0, 0, 1920, 1080)      | Area |
|                              |      |
*/
    AX_IVPS_ASPECT_RATIO_HORIZONTAL_CENTER = 0,
    AX_IVPS_ASPECT_RATIO_HORIZONTAL_LEFT = 1,
    AX_IVPS_ASPECT_RATIO_HORIZONTAL_RIGHT = 2,
    AX_IVPS_ASPECT_RATIO_VERTICAL_CENTER = AX_IVPS_ASPECT_RATIO_HORIZONTAL_CENTER,
    AX_IVPS_ASPECT_RATIO_VERTICAL_TOP = AX_IVPS_ASPECT_RATIO_HORIZONTAL_LEFT,
    AX_IVPS_ASPECT_RATIO_VERTICAL_BOTTOM = AX_IVPS_ASPECT_RATIO_HORIZONTAL_RIGHT,
} AX_IVPS_ASPECT_RATIO_ALIGN_E;

#endif
