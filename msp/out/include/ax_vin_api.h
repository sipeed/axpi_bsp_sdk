/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef __AX_VIN_API_H__
#define __AX_VIN_API_H__

#include "ax_base_type.h"
#include "ax_global_type.h"
#include "ax_isp_common.h"
#include "ax_sensor_struct.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum {
    AX_PIPE_SOURCE_MIN                      = -1,
    AX_PIPE_SOURCE_DEV_ONLINE               = 0,   /* device mode, online, less than 2 pipes used for ife */
    AX_PIPE_SOURCE_DEV_OFFLINE              = 1,   /* device mode, offline, more than 2 pipes used used for ife */
    AX_PIPE_SOURCE_USER_OFFLINE             = 2,   /* user mode, offline */
    AX_PIPE_SOURCE_MAX,
} AX_ISP_PIPE_SOURCE_E;

typedef enum {
    AX_RAW_SOURCE_ID_MIN                    = -1,
    AX_RAW_SOURCE_ID_SIF_SOURCE             = 0,   /* write out from sif, for user mode */
    AX_RAW_SOURCE_ID_IFE_SOURCE             = 1,   /* write out from ife, for user mode */
    AX_RAW_SOURCE_ID_EXT_SIF_SOURCE         = 2,   /* write out from ext sif */
    AX_RAW_SOURCE_ID_EXT_IFE_SOURCE         = 3,   /* write out from ext ife */
    AX_RAW_SOURCE_ID_EXT_NPU_SOURCE         = 4,   /* write out from ext npu */
    AX_RAW_SOURCE_ID_MAX
} AX_RAW_SOURCE_ID_E;

typedef enum {
    AX_OUTPUT_ID_MIN                    = -1,
    AX_RAW_SINK_IFE                     = 1,   /* write out to ife */
    AX_RAW_SINK_NPU                     = 2,   /* write out to npu */
    AX_OUTPUT_ID_MAX
} AX_RAW_SINK_ID_E;

typedef enum {
    AX_PIPE_INPUT_SOURCE_ID_MIN             = -1,
    AX_PIPE_INPUT_SOURCE_ID_SIF             = 0,
    AX_PIPE_INPUT_SOURCE_ID_ITP_RAW         = 1,
    AX_PIPE_INPUT_SOURCE_ID_ITP_RGB         = 2,
    AX_PIPE_INPUT_SOURCE_ID_ITP_YUV         = 3,
    AX_PIPE_INPUT_SOURCE_ID_MAX
} AX_PIPI_INPUT_SOURCE_ID_E;

typedef enum {
    AX_YUV_SOURCE_ID_INVALID                = -1,
    AX_YUV_SOURCE_ID_MAIN                   = 0,   /* main path */
    AX_YUV_SOURCE_ID_SUB1                   = 1,   /* sub path from downscaler0 */
    AX_YUV_SOURCE_ID_SUB2                   = 2,   /* sub path from downscaler1 */
    AX_YUV_SOURCE_ID_MAX
} AX_YUV_SOURCE_ID_E;

typedef enum {
    AX_PRE_OUTPUT_ID_INVALIED               = -1,
    AX_PRE_OUTPUT_FULL_MAIN                 = 0,    /* data output from ife full main path (ife_full drc)*/
    AX_PRE_OUTPUT_LITE_MAIN                 = 1,    /* data output from ife lite main path (ife_lite drc) */
    AX_PRE_OUTPUT_ID_MAX
} AX_PRE_OUTPUT_ID_E;

typedef enum {
    AX_MIPI_OUTPUT_SOURCE_SNS0              = 0,
    AX_MIPI_OUTPUT_SOURCE_SNS1              = 1,
    AX_MIPI_OUTPUT_SOURCE_RLTM              = 2,
    AX_MIPI_OUTPUT_SOURCE_SNS2              = 3,
} AX_OUTPUT_SOURCE_E;

typedef enum {
    AX_ISP_PIPELINE_NORMAL = 0,
    AX_ISP_PIPELINE_NONE_NPU,
} AX_RUN_MODE_E;

typedef enum {
    AX_ISP_FUSION_NONE = 0,
    AX_ISP_FUSION_DUAL_VISION,
} AX_FUSION_MODE_E;

typedef struct {
    AX_F32      fExpTime;
    AX_F32      fAGain;
    AX_F32      fDGain;
    AX_F32      fIspGain;
    AX_U32      nHcgLcgMode;        /* 0:HCG 1:LCG 2: Not Support*/
} AX_FRAME_EXP_INFO_T;

typedef struct {
    AX_RAW_TYPE_E       eRawType;
    AX_SNS_HDR_MODE_E   eHdrMode;
    AX_SNS_HDR_FRAME_E  nHdrFrame;
    AX_BAYER_PATTERN_E  eBayerPattern;
    AX_FRAME_EXP_INFO_T tExpInfo;
} AX_ISP_FRAME_T;

typedef struct {
    AX_VIDEO_FRAME_INFO_S   tFrameInfo;
    AX_ISP_FRAME_T          tIspInfo;
} AX_IMG_INFO_T;

typedef struct _AX_TX_IMG_INFO_T_ {
    AX_BOOL             bMipiTxEnable;
    AX_BOOL             bIspBypass;
    AX_U32              nWidth;
    AX_U32              nHeight;
    AX_IMG_FORMAT_E     eImgFormat;
} AX_TX_IMG_INFO_T;

typedef enum {
    AX_DATAFLOW_TYPE_INVALID                = -1,
    AX_DATAFLOW_TYPE_NORMAL                 = 0,
    AX_DATAFLOW_TYPE_MULTIPLEX              = 1,
    AX_DATAFLOW_TYPE_DISPATCH               = 2,
    AX_DATAFLOW_TYPE_MAX
} AX_DATAFLOW_TYPE_E;

typedef enum {
    AX_PIPE_FRAME_SOURCE_TYPE_MIN  = -1,
    AX_PIPE_FRAME_SOURCE_TYPE_DEV  = 0,
    AX_PIPE_FRAME_SOURCE_TYPE_USER = 1,
    AX_PIPE_FRAME_SOURCE_TYPE_MAX
} AX_ISP_PIPE_FRAME_SOURCE_TYPE_E;

typedef enum {
    AX_PIPE_FRAME_SOURCE_ID_MIN  = -1,
    AX_PIPE_FRAME_SOURCE_ID_IFE  = 0,
    AX_PIPE_FRAME_SOURCE_ID_NPU  = 1,
    AX_PIPE_FRAME_SOURCE_ID_YUV  = 2,
    AX_PIPE_FRAME_SOURCE_ID_MAX
} AX_ISP_PIPE_FRAME_SOURCE_ID_E;

typedef union {
    struct {
        AX_U32  bitIFEBypass                : 1 ;   /* RW; [0] */
        AX_U32  bitNPUBypass                : 1 ;   /* RW; [1] */
        AX_U32  bitITPBypass                : 1 ;   /* RW; [2] */
        AX_U32  bitRsv29                    : 29 ;  /* H ; [3:31] */
    };
    AX_U32  u32Key;
} AX_VIN_COMPONENT_CTRL_T;

typedef enum {
    AX_DEV_WORK_MODE_MIN            = -1,
    AX_DEV_WORK_MODE_NORMAL         = 0,        /* normal work mode */
    AX_DEV_WORK_MODE_MASTER         = 1,        /* master mode in cascade */
    AX_DEV_WORK_MODE_SLAVE          = 2,        /* slave mode in cascade */
    AX_DEV_WORK_MODE_MAX,
} AX_ISP_DEV_WORK_MODE_E;


/* Sequence of YUV data */
typedef enum _AX_VIN_YUV_DATA_SEQ_E_ {
    AX_VIN_DATA_SEQ_VUVU = 0,   /* The input sequence of the second component(only contains u and v) in BT.1120 mode is VUVU */
    AX_VIN_DATA_SEQ_UVUV,       /* The input sequence of the second component(only contains u and v) in BT.1120 mode is UVUV */

    AX_VIN_DATA_SEQ_UYVY,       /* The input sequence of YUV is UYVY */
    AX_VIN_DATA_SEQ_VYUY,       /* The input sequence of YUV is VYUY */
    AX_VIN_DATA_SEQ_YUYV,       /* The input sequence of YUV is YUYV */
    AX_VIN_DATA_SEQ_YVYU,       /* The input sequence of YUV is YVYU */

    AX_VIN_DATA_SEQ_MAX
} AX_VIN_YUV_DATA_SEQ_E;

/* Clock edge mode */
typedef enum _AX_VIN_CLK_EDGE_E_ {
    AX_VIN_CLK_EDGE_SINGLE_UP = 0,         /* single-edge mode: rising edge */
    AX_VIN_CLK_EDGE_SINGLE_DOWN,           /* single-edge mode: falling edge */

    AX_VIN_CLK_EDGE_MAX
} AX_VIN_CLK_EDGE_E;

typedef enum _AX_VIN_SCAN_MODE_E_ {
    AX_VIN_SCAN_INTERLACED  = 0,        /* interlaced mode */
    AX_VIN_SCAN_PROGRESSIVE,            /* progressive mode */
    AX_VIN_SCAN_MAX
} AX_VIN_SCAN_MODE_E;

/* Polarity of the horizontal synchronization signal */
typedef enum _AX_VIN_HSYNC_POLARITY_E_ {
    AX_VIN_SYNC_POLARITY_HIGH = 0,        /* the valid horizontal/vertical synchronization signal is high-level */
    AX_VIN_SYNC_POLARITY_LOW,             /* the valid horizontal/vertical synchronization signal is low-level */
    AX_VIN_SYNC_POLARITY_MAX
} AX_VIN_SYNC_POLARITY_E;

typedef enum _AX_VIN_LVDS_SYNC_MODE_E_ {
    AX_VIN_LVDS_SYNC_MODE_SOF = 0,         /* sensor SOL, EOL, SOF, EOF */
    AX_VIN_LVDS_SYNC_MODE_SAV,             /* SAV, EAV */
    AX_VIN_LVDS_SYNC_MODE_MAX
} AX_VIN_LVDS_SYNC_MODE_E;

typedef enum _AX_VIN_LVDS_BIT_ENDIAN_E_ {
    AX_VIN_LVDS_ENDIAN_LITTLE  = 0,
    AX_VIN_LVDS_ENDIAN_BIG     = 1,
    AX_VIN_LVDS_ENDIAN_MAX
} AX_VIN_LVDS_BIT_ENDIAN_E;

/* synchronization information about the BT or DVP timing */
typedef struct _AX_VIN_SYNC_CFG_T_ {
    AX_VIN_SYNC_POLARITY_E      eVsyncInv;                          /* the valid horizontal/vertical synchronization signal polarity */
    AX_VIN_SYNC_POLARITY_E      eHsyncInv;                          /* the valid horizontal/vertical synchronization signal polarity */
} AX_VIN_SYNC_CFG_T;

typedef struct _AX_VIN_DEV_BT_ATTR_T_ {
    AX_VIN_SCAN_MODE_E          eScanMode;                              /* Input scanning mode (progressive or interlaced) */
    AX_U32                      nComponentMask[AX_VIN_COMPMASK_NUM];    /* Component mask */
    AX_VIN_SYNC_CFG_T           tSyncCfg;
} AX_VIN_DEV_BT_ATTR_T;

typedef struct _AX_VIN_DEV_DVP_ATTR_T_ {
    AX_VIN_YUV_DATA_SEQ_E       eDataSeq;                   /* Input data sequence (only the YUV format is supported) */
    AX_U32                      nComponentMask[AX_VIN_COMPMASK_NUM];    /* Component mask */
    AX_VIN_SYNC_CFG_T           tSyncCfg;
} AX_VIN_DEV_DVP_ATTR_T;

typedef struct _AX_VIN_DEV_LVDS_ATTR_T_ {
    AX_VIN_LVDS_SYNC_MODE_E     eSyncMode;                  /* Input data sequence (only the YUV format is supported) */
    AX_VIN_LVDS_BIT_ENDIAN_E    eDataEndian;                /* data endian: little/big */
    AX_VIN_LVDS_BIT_ENDIAN_E    eSyncCodeEndian;            /* sync code endian: little/big */

    /* each vc has 4 params, sync code[i]:
       sync mode is SYNC_MODE_SOF: SOF, EOF, SOL, EOL
       sync mode is SYNC_MODE_SAV: invalid sav, invalid eav, valid sav, valid eav  */
    AX_U8                       szSyncCode[AX_VIN_LVDS_LANE_NUM][AX_HDR_CHN_NUM][AX_VIN_SYNC_CODE_NUM];
} AX_VIN_DEV_LVDS_ATTR_T;

typedef struct _AX_DEV_ATTR_T_ {
    AX_ISP_DEV_WORK_MODE_E                  eDevWorkMode;
    AX_BOOL                                 bImgDataEnable;     /* 1: image data enable, 0: disable */
    AX_SNS_TYPE_E                           eSnsType;
    AX_SNS_HDR_MODE_E                       eSnsMode;
    AX_SNS_GAIN_MODE_E                      eSnsGainMode;
    AX_BAYER_PATTERN_E                      eBayerPattern;
    AX_IMG_FORMAT_E                         ePixelFmt;          /* Pixel format */
    AX_WIN_AREA_T                           tDevImgRgn;         /* image region acquired by dev */
    AX_BOOL                                 bHMirror;           /* 0: No mirror, 1: horiontal direction mirror */
    AX_BOOL                                 bDolSplit;          /* 0: No split, 1: write the dol image in seprate frame */
    AX_SNS_SKIP_FRAME_E                     eSkipFrame;
    AX_SNS_OUTPUT_MODE_E                    eSnsOutputMode;

    union {
        AX_VIN_DEV_LVDS_ATTR_T              tLvdsIntfAttr;      /* input interface attr: LVDS */
        AX_VIN_DEV_DVP_ATTR_T               tDvpIntfAttr;       /* input interface attr: DVP */
        AX_VIN_DEV_BT_ATTR_T                tBtIntfAttr;        /* input interface attr: BT1120\BT656\BT601 */
    };

    AX_BOOL                                 bNonImgEnable;      /* 1: non image data enable, 0: disable */
    AX_U32                                  nNonImgWidth;
    AX_U32                                  nNonImgHeight;
    AX_IMG_FORMAT_E                         eNonPixelFmt;
    AX_U8                                   nNonImgDT;
    AX_U8                                   nNonImgVC;
    AX_U8                                   reserve[2];
    AX_BOOL                                 bIspBypass;
} AX_DEV_ATTR_T;


typedef struct {
    AX_U32                                  nWidth;
    AX_U32                                  nHeight;
    AX_U32                                  nWidthStride;
    AX_BAYER_PATTERN_E                      eBayerPattern;
    AX_IMG_FORMAT_E                         ePixelFmt;
    AX_SNS_HDR_MODE_E                       eSnsMode;
    AX_FUSION_MODE_E                        eFusionMode;
    AX_ISP_PIPE_SOURCE_E                    ePipeDataSrc;
    AX_DATAFLOW_TYPE_E                      eDataFlowType;
    AX_DEV_SOURCE_E                         eDevSource;         /* according to AX_DEV_SOURCE_E */
    AX_PRE_OUTPUT_ID_E                      ePreOutput;         /* according to AX_PRE_OUTPUT_ID_E */
    AX_VIN_COMPONENT_CTRL_T                 tCompCtrl;          /* according to AX_VIN_MODULE_CTRL_T */
} AX_PIPE_ATTR_T;

typedef struct {
    AX_U8                                   uPipeID;            /* 255: none */
    AX_BOOL                                 bOutEn;             /* TX output enable */
    AX_OUTPUT_SOURCE_E                      eOutSource;
} AX_VOUT_ATTR_T;

typedef enum _AX_DEV_STATE_E_ {
    AX_VIN_STATE_UNITED = 0,
    AX_VIN_STATE_INIT = 1,
    AX_VIN_STATE_RUNNING = 2,
} AX_DEV_STATE_E;

typedef struct {
    AX_DEV_STATE_E                          eStatus;
    AX_U64                                  nFrameCnt;
    AX_U32                                  nFrameRate;
    AX_U32                                  nLostCount;
    AX_U32                                  nImgWidth;
    AX_U32                                  nImgHight;
} AX_VIN_DEV_STATUS_T;

typedef struct {
    AX_BOOL                                 bEnable;
    AX_U32                                  nWidth;
    AX_U32                                  nHeight;
    AX_U32                                  nWidthStride;
    AX_IMG_FORMAT_E                         eImgFormat;
    AX_U32                                  nDepth;
} AX_VIN_CHN_DEV_T;

typedef struct {
    AX_VIN_CHN_DEV_T    tChnAttr[AX_YUV_SOURCE_ID_MAX];    /* support 3chn dev output */
} AX_VIN_CHN_ATTR_T;

typedef enum _AX_VIN_DUMP_TYPE_E_ {
    VIN_DUMP_SOURCE_INVLAID     = -1,   /* Invlaid type */
    VIN_DUMP_SOURCE_IFE         = 0,    /* write data from ife to ddr */
    VIN_DUMP_SOURCE_NPU         = 1,    /* write data from npu */
    VIN_DUMP_SOURCE_YUV_MAIN    = 2,    /* write data from main chn */
    VIN_DUMP_SOURCE_YUV_SUB1    = 3,    /* write data from sub1 chn */
    VIN_DUMP_SOURCE_YUV_SUB2    = 4,    /* write data from sub2 chn */
} AX_VIN_DUMP_TYPE_E;

typedef struct _AX_VIN_SNS_DUMP_ATTR_T_ {
    AX_BOOL             bEnable;            /* RW; Whether dump is enable */
    AX_U32              nDepth;             /* RW; frame buffer depth */
} AX_VIN_SNS_DUMP_ATTR_T;

typedef struct _AX_VIN_DUMP_ATTR_T_ {
    AX_BOOL             bEnable;            /* RW; Whether dump is enable */
    AX_U32              nDepth;             /* RW; frame buffer depth */
} AX_VIN_DUMP_ATTR_T;

/* List of pipe bind to dev */
typedef struct _AX_VIN_DEV_BIND_PIPE_T_ {
    AX_U32  nNum;                           /* RW; Range [1, DEF_VIN_PIPE_MAX_NUM] */
    AX_U32  nPipeId[DEF_VIN_PIPE_MAX_NUM];  /* RW; Array of pipe id */
} AX_VIN_DEV_BIND_PIPE_T;

typedef enum _AX_DAYNIGHT_MODE_E_ {
    AX_DAYNIGHT_MODE_DAY     = 0,
    AX_DAYNIGHT_MODE_NIGHT,
} AX_DAYNIGHT_MODE_E;

/************************************************************************************
 *  VIN API
 ************************************************************************************/
AX_S32 AX_VIN_Init();
AX_S32 AX_VIN_Deinit();

AX_S32 AX_VIN_RegisterSensor(AX_U8 pipe, AX_SENSOR_REGISTER_FUNC_T *ptSnsRegister);
AX_S32 AX_VIN_UnRegisterSensor(AX_U8 pipe);

AX_S32 AX_VIN_OpenSnsClk(AX_U8 pipe, AX_U8 clkIdx, AX_SNS_CLK_RATE_E eClkRate);
AX_S32 AX_VIN_CloseSnsClk(AX_U8 clkIdx);

AX_S32 AX_VIN_SetSnsAttr(AX_U8 pipe, AX_SNS_ATTR_T *pSnsAttr);
AX_S32 AX_VIN_GetSnsAttr(AX_U8 pipe, AX_SNS_ATTR_T *pSnsAttr);

AX_S32 AX_VIN_SetDevAttr(AX_U8 dev_id, AX_DEV_ATTR_T *pDevAttr);
AX_S32 AX_VIN_GetDevAttr(AX_U8 dev_id, AX_DEV_ATTR_T *pDevAttr);

AX_S32 AX_VIN_SetOutAttr(AX_U8 dev_id, AX_VOUT_ATTR_T *pOutAttr);
AX_S32 AX_VIN_GetOutAttr(AX_U8 dev_id, AX_VOUT_ATTR_T *pOutAttr);

AX_S32 AX_VIN_QueryDevState(AX_U8 dev_id, AX_VIN_DEV_STATUS_T *pDevStatus);

AX_S32 AX_VIN_Create(AX_U8 pipe);
AX_S32 AX_VIN_Destory(AX_U8 pipe);

AX_S32 AX_VIN_Start(AX_U8 pipe);
AX_S32 AX_VIN_Stop(AX_U8 pipe);

AX_S32 AX_VIN_SetPipeAttr(AX_U8 pipe, AX_PIPE_ATTR_T *pPipeAttr);
AX_S32 AX_VIN_GetPipeAttr(AX_U8 pipe, AX_PIPE_ATTR_T *pPipeAttr);

AX_S32 AX_VIN_SetPipeDumpAttr(AX_U8 pipe, AX_VIN_DUMP_TYPE_E  eDumpType, AX_VIN_DUMP_ATTR_T *ptDumpAttr);
AX_S32 AX_VIN_GetPipeDumpAttr(AX_U8 pipe, AX_VIN_DUMP_TYPE_E  eDumpType, AX_VIN_DUMP_ATTR_T *ptDumpAttr);

AX_S32 AX_VIN_SetChnAttr(AX_U8 pipe, AX_VIN_CHN_ATTR_T *pChnAttr);
AX_S32 AX_VIN_GetChnAttr(AX_U8 pipe, AX_VIN_CHN_ATTR_T *pChnAttr);

AX_S32 AX_VIN_EnableDev(AX_U8 dev_id);
AX_S32 AX_VIN_DisableDev(AX_U8 dev_id);

AX_S32 AX_VIN_StreamOn(AX_U8 pipe);
AX_S32 AX_VIN_StreamOff(AX_U8 pipe);

AX_S32 AX_VIN_SetSnsDumpAttr(AX_U8 dev_id, AX_VIN_SNS_DUMP_ATTR_T *ptSnsDumpAttr);
AX_S32 AX_VIN_GetSnsDumpAttr(AX_U8 dev_id, AX_VIN_SNS_DUMP_ATTR_T *ptSnsDumpAttr);

AX_S32 AX_VIN_SetNonImageAttr(AX_U8 dev_id);
AX_S32 AX_VIN_GetNonImageAttr(AX_U8 dev_id);

AX_S32 AX_VIN_SendFrame(AX_U8 pipe, AX_PIPI_INPUT_SOURCE_ID_E srcChId, AX_IMG_INFO_T *pImgInfo, AX_S32 timeOutMs);
AX_S32 AX_VIN_GetRawFrame(AX_U8 pipe, AX_RAW_SOURCE_ID_E rawChId, AX_SNS_HDR_FRAME_E eSnsFrame, AX_IMG_INFO_T *pImgInfo, AX_S32 timeOutMs);
AX_S32 AX_VIN_ReleaseRawFrame(AX_U8 pipe, AX_RAW_SOURCE_ID_E rawChId, AX_SNS_HDR_FRAME_E eSnsFrame, AX_IMG_INFO_T *pImgInfo);
AX_S32 AX_VIN_GetIfeRawFrame(AX_U8 pipe, AX_SNS_HDR_FRAME_E eSnsFrame, AX_IMG_INFO_T *pImgInfo, AX_S32 timeOutMs);
AX_S32 AX_VIN_ReleaseIfeRawFrame(AX_U8 pipe, AX_SNS_HDR_FRAME_E eSnsFrame, AX_IMG_INFO_T *pImgInfo);

AX_S32 AX_VIN_SendRawFrame(AX_U8 pipe, AX_RAW_SINK_ID_E rawChId, AX_S8 nFrameNum, AX_IMG_INFO_T *pImgInfo[], AX_S32 timeOutMs);

AX_S32 AX_VIN_GetSnsFrame(AX_U8 devId, AX_SNS_HDR_FRAME_E eSnsFrame, AX_IMG_INFO_T *pImgInfo, AX_S32 timeOutMs);
AX_S32 AX_VIN_ReleaseSnsFrame(AX_U8 devId, AX_SNS_HDR_FRAME_E eSnsFrame, AX_IMG_INFO_T *pImgInfo);

AX_S32 AX_VIN_GetYuvFrame(AX_U8 pipe, AX_YUV_SOURCE_ID_E yuvChId, AX_IMG_INFO_T *pImgInfo, AX_S32 timeOutMs);
AX_S32 AX_VIN_ReleaseYuvFrame(AX_U8 pipe,  AX_YUV_SOURCE_ID_E yuvChId, AX_IMG_INFO_T *pImgInfo);
AX_S32 AX_VIN_SetRunMode(AX_U8 pipe, AX_RUN_MODE_E run_mode);
AX_RUN_MODE_E AX_VIN_GetRunMode(AX_U8 pipe);

AX_S32 AX_VIN_SetPipeFrameSource(AX_U8 pipe, AX_ISP_PIPE_FRAME_SOURCE_ID_E eSrcId, AX_ISP_PIPE_FRAME_SOURCE_TYPE_E eSrcType);
AX_S32 AX_VIN_GetPipeFrameSource(AX_U8 pipe, AX_ISP_PIPE_FRAME_SOURCE_ID_E eSrcId, AX_ISP_PIPE_FRAME_SOURCE_TYPE_E *eSrcType);

AX_S32 AX_VIN_TxOpen(AX_U8 nMipiTxDev, AX_TX_IMG_INFO_T *pTxImgInfo);
AX_S32 AX_VIN_TxClose(AX_U8 nMipiTxDev);
AX_S32 AX_VIN_TxSendData(AX_U8 nMipiTxDev, AX_VIDEO_FRAME_S *pImgInfo, AX_S32 timeOutMs);

AX_S32 AX_VIN_SetDevBindPipe(AX_U8 dev_id, const AX_VIN_DEV_BIND_PIPE_T *ptDevBindPipe);
AX_S32 AX_VIN_GetDevBindPipe(AX_U8 dev_id, AX_VIN_DEV_BIND_PIPE_T *ptDevBindPipe);

AX_S32 AX_VIN_SetDevBindTx(AX_U8 dev_id, AX_U8 nMipiTxDev);
AX_S32 AX_VIN_GetDevBindTx(AX_U8 dev_id, AX_U8 *nMipiTxDev);

AX_S32 AX_VIN_SendYuvFrame(AX_U8 pipe, AX_VIDEO_FRAME_S *pImgInfo, AX_S32 timeOutMs);

AX_S32 AX_VIN_SetChnDayNightMode(AX_U8 pipe, AX_U8 chn, AX_DAYNIGHT_MODE_E eNightMode);
AX_S32 AX_VIN_GetChnDayNightMode(AX_U8 pipe, AX_U8 chn, AX_DAYNIGHT_MODE_E *eNightMode);
#ifdef __cplusplus
}
#endif

#endif //__AX_VIN_API_H__
