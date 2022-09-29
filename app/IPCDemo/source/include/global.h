/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef _GLOBAL_2E66C191_E80B_4591_9ADD_A67605183B54_H_
#define _GLOBAL_2E66C191_E80B_4591_9ADD_A67605183B54_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <assert.h>
#include <vector>
#include <regex>
#include <thread>

extern "C" {
#include "ax_sys_api.h"
#include "ax_vin_api.h"
#include "ax_isp_api.h"
#include "ax_mipi_api.h"
#include "ax_sensor_struct.h"
#include "ax_nt_stream_api.h"
#include "ax_nt_ctrl_api.h"
#include "ax_venc_api.h"
#include "ax_ivps_api.h"
#include "ax_ivps_type.h"
#include "ax_base_type.h"
#include "ax_global_type.h"
#include "ax_comm_codec.h"
#include "ax_buffer_tool.h"
#include "ax_interpreter_external_api.h"
}

#include "AiDetection.h"
#include "AiTrack.h"
#include "AiSearch.h"
#include "AppLog.h"
#include "TimeUtil.h"

#ifdef AX_MEM_CHECK
#include <mcheck.h>
#define AX_MTRACE_ENTER(name) do { \
    if (!getenv("MALLOC_TRACE")) { \
        setenv("MALLOC_TRACE", ""#name"_mtrace.log", 1); \
    } \
    mtrace();\
}while(0)

#define AX_MTRACE_LEAVE do { \
    printf("please wait mtrace flush log to file...\n"); \
    sleep(30);\
}while(0)

#else
#define AX_MTRACE_ENTER(name)
#define AX_MTRACE_LEAVE
#endif

#define SDK_VERSION_PREFIX  "Ax_Version"

/* MAX: VDEC_MAX_CHN_NUM */
#define MAX_VIDEO_NUM       (2)
#define MAX_FIFO_NUM        (24)

/* NPU detection model resolution */
#define NPU_IMG_WIDTH       1920
#define NPU_IMG_HEIGHT      1080

/* Min and max resolution of input video */
#define MIN_IMG_WIDTH       1920
#define MIN_IMG_HEIGHT      1080

#define MAX_IMG_WIDTH       4096
#define MAX_IMG_HEIGHT      2160

/* DPU only support YUV420SP NV12 */
#define DPU_YUV_TYPE        AX_YUV420_SEMIPLANAR

#ifdef  AX_SDK_PASS
#undef  AX_SDK_PASS
#endif
#define AX_SDK_PASS         (0)

#ifndef AX_MAX
#define AX_MAX(a, b)        (((a) > (b)) ? (a) : (b))
#endif

#ifndef AX_MIN
#define AX_MIN(a, b)        (((a) < (b)) ? (a) : (b))
#endif

#ifndef ALIGN_UP
#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align)-1))
#endif

#ifndef ALIGN_DOWN
#define ALIGN_DOWN(x, align) ((x) & ~((align)-1))
#endif

#ifndef ALIGN_COMM_UP
#define ALIGN_COMM_UP(x, align) ((((x) + ((align) - 1)) / (align)) * (align))
#endif

#ifndef ALIGN_COMM_DOWN
#define ALIGN_COMM_DOWN(x, align) (((x) / (align)) * (align))
#endif

#ifndef ADAPTER_RANGE
#define ADAPTER_RANGE(v, min, max)    ((v) < (min)) ? (min) : ((v) > (max)) ? (max) : (v)
#endif

#ifndef AX_BIT_CHECK
#define AX_BIT_CHECK(v, b) (((AX_U32)(v) & (1 << (b))))
#endif

#ifndef AX_BIT_SET
#define AX_BIT_SET(v, b) ((v) |= (1 << (b)))
#endif

#ifndef AX_BIT_CLEAR
#define AX_BIT_CLEAR(v, b) ((v) &= ~(1 << (b)))
#endif

#define SAFE_DELETE_PTR(p) {if(p){delete p;p = nullptr;}}

#define ADAPTER_INT2BOOLSTR(val) (val == 1 ? "true" : "false")
#define ADAPTER_BOOLSTR2INT(val) (strcmp(val, "true") == 0 ? 1 : 0)

#define MAX_SNS_NUM          (1)
#define MAX_ISP_CHANNEL_NUM  (3)

#define MAX_VENC_CHANNEL_NUM (3)
#define MAX_WEB_CHANNEL_NUM  (3)
#define JENC_CHANNEL_ID      (1)
#define CAPTURE_VENC_CHANNEL_ID (9)
#define SNAPSHOT_VENC_CHANNEL_ID (10)

#define MAX_AI_CHANNEL_NUM   (1)

typedef enum
{
    E_END_POINT_NONE = 0,
    E_END_POINT_VENC,
    E_END_POINT_JENC,
    E_END_POINT_NPU,
    E_END_POINT_DET,
    E_END_POINT_MAX
} END_POINT_TYPE;

typedef struct  _END_POINT_OPTIONS {
    END_POINT_TYPE eEPType;
    AX_U8   nChannel;
    AX_U8   nInnerIndex;
} END_POINT_OPTIONS;

#define GDC_STRIDE_ALIGNMENT  (64)

#ifdef AX_SIMPLIFIED_MEM_VER

#define AX_RAW_BLOCK_COUNT_SDR (4)
#define AX_RAW_BLOCK_COUNT_HDR (6)
#define AX_RAW_BLOCK_COUNT AX_RAW_BLOCK_COUNT_HDR
#define AX_RAW_META_SIZE (5)
#define AX_NPU_BLOCK_COUNT (4)
#define AX_NPU_META_SIZE (5)
#define YUV_OS04A10_DEPTH (0)
#define YUV_GC4653_DEPTH (0)
#define YUV_SC1345_DEPTH (1)
#define YUV_SC530AI_DEPTH (0)
#define AX_YUV_META_SIZE (5)
#define YUV_BLOCK_COUNT_ADDATION_NORMAL (1) // Redundant buf count
#define YUV_BLOCK_COUNT_ADDATION_WBT_SDR_WNR 1    // Redundant buf count
#define VIN_IFE_DUMP_ENABLE (0) // will affect the block of RAW10 (AX_RAW_BLOCK_COUNT)
#define VIN_NPU_DUMP_ENABLE (0) // will affect the block of RAW16 (AX_NPU_BLOCK_COUNT)

#define AX_WEB_VENC_RING_BUFF_COUNT (5)
#define AX_WEB_JENC_RING_BUFF_COUNT (10)
#define AX_WEB_SNAPSHOT_RING_BUFF_COUNT (0)

// DETECTION
#define DETECTOR_ISP_GRP_NO (1) // use this channel to detect
#define DETECTOR_IVPS_CHANNEL_NO (1) // use this channel to detect
#define AX_AI_DETECT_FRAME_DEPTH (0)  // Frame depth
#define AX_AI_DETECT_CACHE_LIST_DEPTH (1) // Cache list depth
#define AX_AI_DETECT_FRAME_DEPTH_BLOCK_COUNT (AX_AI_DETECT_FRAME_DEPTH)
#define AX_AI_DETECT_CACHE_LIST_DEPTH_BLOCK_COUNT (AX_AI_DETECT_CACHE_LIST_DEPTH)
#define AX_AI_VENC0_USING_DETECTION (1) //0 or 1
#define AX_AI_VENC1_USING_DETECTION (1) //0 or 1
#define AX_AI_DETECT_BLOCK_COUNT (AX_AI_DETECT_FRAME_DEPTH_BLOCK_COUNT + AX_AI_DETECT_CACHE_LIST_DEPTH_BLOCK_COUNT)

#ifdef AX_620U_38BOARD_VER
#define YUV_BLOCK_COUNT_ADDATION_LINK (2 + 1)
#define AX_4653_MIPI_RX_PH_ID (AX_MIPI_RX_PHY1_SEL_LANE_0_1_2_3)
#define AX_PIPE_ID (1)
#define AX_DEV_SOURCE_SNS_ID (AX_DEV_SOURCE_SNS_1)
#define AX_SENSOR_ID (E_SENSOR_ID_1)
#else
#define YUV_BLOCK_COUNT_ADDATION_LINK (2 + 1)
#define AX_4653_MIPI_RX_PH_ID (AX_MIPI_RX_PHY0_SEL_LANE_0_1_2_3)
#define AX_DEV_SOURCE_SNS_ID (AX_DEV_SOURCE_SNS_0)
#define AX_PIPE_ID (0)
#define AX_SENSOR_ID (E_SENSOR_ID_0)
#endif //  AX_620U_38BOARD_VER

#define AX_IVPS_IN_FIFO_DEPTH (1)
#define AX_IVPS_OUT_FIFO_DEPTH (1) // if nonlink mode, out fifo depth should >= 1

#define AX_VENC_IN_FIFO_LINK_MODE_DEPTH (0) // 0: use system default(1)
#define AX_VENC_IN_FIFO_NONLINK_MODE_DEPTH (1) // 0: use system default(1)

#define AX_VENC_OUT_FIFO_LINK_MODE_DEPTH (1) // 0: use system default(4)
#define AX_VENC_OUT_FIFO_NONLINK_MODE_DEPTH (1) // 0: use system default(4)

#else

// if value is 0, we will set it but context code.
#define AX_RAW_BLOCK_COUNT_SDR (0)
#define AX_RAW_BLOCK_COUNT_HDR (0)
#define AX_RAW_BLOCK_COUNT AX_RAW_BLOCK_COUNT_HDR
#define AX_RAW_META_SIZE (10)
#define AX_NPU_BLOCK_COUNT (0)
#define AX_NPU_META_SIZE (10)
#define YUV_OS04A10_DEPTH (5)
#define YUV_GC4653_DEPTH (5)
#define YUV_SC1345_DEPTH (6)
#define YUV_SC530AI_DEPTH (5)
#define AX_YUV_META_SIZE (10)
#define YUV_BLOCK_COUNT_ADDATION_LINK (4 + 4 + 2) // (IVPS recommended buf count) + (VENC recommended buf count) + (Redundant buf count)
#define YUV_BLOCK_COUNT_ADDATION_NORMAL (2 + 2) // Redundant buf count
#define YUV_BLOCK_COUNT_ADDATION_WBT_SDR_WNR 0    // Redundant buf count
#define VIN_IFE_DUMP_ENABLE (1) // will affect the block of RAW10 (AX_RAW_BLOCK_COUNT)
#define VIN_NPU_DUMP_ENABLE (1) // will affect the block of RAW16 (AX_NPU_BLOCK_COUNT)

#define AX_WEB_VENC_RING_BUFF_COUNT (20)
#define AX_WEB_JENC_RING_BUFF_COUNT (12)
#define AX_WEB_SNAPSHOT_RING_BUFF_COUNT (0)

// DETECTION
#define DETECTOR_ISP_GRP_NO (1) // use this channel to detect
#define DETECTOR_IVPS_CHANNEL_NO (1) // use this channel to detect
#define AX_AI_DETECT_FRAME_DEPTH (10)  // Frame depth
#define AX_AI_DETECT_CACHE_LIST_DEPTH (20) // Cache list depth
#define AX_AI_DETECT_FRAME_DEPTH_BLOCK_COUNT (AX_AI_DETECT_FRAME_DEPTH)
#define AX_AI_DETECT_CACHE_LIST_DEPTH_BLOCK_COUNT (AX_AI_DETECT_CACHE_LIST_DEPTH)
#define AX_AI_VENC0_USING_DETECTION (1) //0 or 1
#define AX_AI_VENC1_USING_DETECTION (1) //0 or 1
#define AX_AI_DETECT_BLOCK_COUNT (AX_AI_DETECT_FRAME_DEPTH_BLOCK_COUNT + AX_AI_DETECT_CACHE_LIST_DEPTH_BLOCK_COUNT)

#define AX_4653_MIPI_RX_PH_ID (AX_MIPI_RX_PHY0_SEL_LANE_0_1_2_3)

#define AX_IVPS_IN_FIFO_DEPTH (2)
#define AX_IVPS_OUT_FIFO_DEPTH (2) // if nonlink mode, out fifo depth should >= 1

#define AX_VENC_IN_FIFO_LINK_MODE_DEPTH (0) // 0: use system default(4)
#define AX_VENC_IN_FIFO_NONLINK_MODE_DEPTH (0) // 0: use system default(4)

#define AX_VENC_OUT_FIFO_LINK_MODE_DEPTH (0) // 0: use system default(4)
#define AX_VENC_OUT_FIFO_NONLINK_MODE_DEPTH (0) // 0: use system default(4)

#define AX_DEV_SOURCE_SNS_ID (AX_DEV_SOURCE_SNS_0)
#define AX_PIPE_ID (0)
#define AX_SENSOR_ID (E_SENSOR_ID_0)

#endif //  AX_SIMPLIFIED_MEM_VER

#endif /* _GLOBAL_2E66C191_E80B_4591_9ADD_A67605183B54_H_ */
