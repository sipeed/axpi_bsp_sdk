/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef _IVPS_STAGE_H_
#define _IVPS_STAGE_H_

#include "global.h"
#include "Stage.h"
#include "BaseSensor.h"
#include "JpgEncoder.h"
#include "VideoEncoder.h"
#include "DetectStage.h"
#include "OSDHandlerWrapper.h"

class CIVPSStage;
class CVideoEncoder;
class CJpgEncoder;

#define IVPS_MAX_CHANNEL_PER_GROUP (3)
#define IVPS_GROUP_NUM (3)
#define AX_IVPS_SUCC (0)
#define EPOLL_MAXUSERS (128)
#define OSD_ATTACH_NUM (4)
typedef AX_S32 IVPS_GRP;
typedef AX_S32 EP_HANDLE;
typedef AX_S32 AX_IVPS_FILTER;


enum {
    YUV_COLOR_GREEN         = 0x000000,
    YUV_COLOR_RED           = 0x0000FF,
    YUV_COLOR_BLUE          = 0x00FF00,
    YUV_COLOR_PURPLE        = 0x00FFFF,
    YUV_COLOR_DARK_GREEN    = 0xFF0000,
    YUV_COLOR_YELLOW        = 0xFF00FF,
    YUV_COLOR_DARK_BLACK    = 0x008080,
    YUV_COLOR_GRAY          = 0x808080,
    YUV_COLOR_WHITE         = 0xFF8080,
    YUV_COLOR_NUM           = 9
};

typedef enum {
    OSD_CHN_TYPE_ARGB1555 = 0,
    OSD_CHN_TYPE_RECT,
    OSD_CHN_TYPE_MOSAIC,
    OSD_CHN_TYPE_MAX
} OSD_CHN_TYPE;

typedef struct _IVPS_GROUP_CFG_T {
    AX_U8               nGrpChnNum;
    AX_IVPS_ENGINE_E    eGrpEngineType;
    AX_IVPS_ENGINE_E    arrChnEngineType[IVPS_MAX_CHANNEL_PER_GROUP];
    AX_S16              arrFramerate[IVPS_MAX_CHANNEL_PER_GROUP][2]; /* Config src framerate & dst framerate */
    AX_S16              arrOutResolution[IVPS_MAX_CHANNEL_PER_GROUP][3];
    AX_U8               arrLinkModeFlag[IVPS_MAX_CHANNEL_PER_GROUP];
} IVPS_GROUP_CFG_T, *IVPS_GROUP_CFG_PTR;

typedef struct _IVPS_GRP_ATTR_T
{
	AX_U16 nX;
	AX_U16 nY;
	AX_U16 nW;
	AX_U16 nH;
	AX_U32 nStride;
	AX_IMG_FORMAT_E eFormat;
	IVPS_GRP IvpsGrp;
	AX_MOD_INFO_S tLink;
	AX_S32 nFrameRate;
	AX_U16 nChnNum;
} IVPS_GRP_ATTR_T;

typedef struct _IVPS_CHN_ATTR_T
{
	AX_U32 nWidth;
    AX_U32 nHeight;
    AX_U32 nStride;
	AX_S32 nFD;
} IVPS_CHN_ATTR_T;

typedef struct _IVPS_GRP_T {
    IVPS_GRP            nIvpsGrp;
    EP_HANDLE           hEpool;
    IVPS_RGN_HANDLE     hGrpRgn;
    AX_S32              nIspChn;
    AX_IVPS_GRP_ATTR_S  tIvpsGrp;
    AX_MOD_INFO_S       tPrevMod;
    AX_MOD_INFO_S       tCurrMod;
    AX_MOD_INFO_S       tNextMod;
    AX_IVPS_PIPELINE_ATTR_S tPipelineAttr;
    IVPS_CHN_ATTR_T     arrIvpsChns[IVPS_MAX_CHANNEL_PER_GROUP];
} IVPS_GRP_T, *IVPS_GRP_PTR;

typedef struct _IVPS_REGION_PARAM {
    IVPS_RGN_HANDLE hChnRgn;
    IVPS_GRP nGroup;
    AX_IVPS_FILTER nFilter;
    OSD_CHN_TYPE eOsdType;
    AX_BOOL bExit;

    _IVPS_REGION_PARAM() {
        hChnRgn = AX_IVPS_INVALID_REGION_HANDLE;
        nGroup = -1;
        nFilter = -1;
        eOsdType = OSD_CHN_TYPE_ARGB1555;
        bExit = AX_TRUE;
    }
} IVPS_REGION_PARAM_T, *IVPS_REGION_PARAM_PTR;

typedef struct _IVPS_GET_THREAD_PARAM
{
    AX_BOOL bValid;
    AX_U8 nIvpsGrp;
    AX_U8 nIvpsChn;
    AX_U8 nIvpsChnIndex;
    CIVPSStage* pReleaseStage;
    AX_BOOL bExit;

    _IVPS_GET_THREAD_PARAM() {
        bValid = AX_FALSE;
        nIvpsGrp = 0;
        nIvpsChn = 0;
        nIvpsChnIndex = 0;
        pReleaseStage = nullptr;
        bExit = AX_TRUE;
    }
} IVPS_GET_THREAD_PARAM_T, *IVPS_GET_THREAD_PARAM_PTR;


typedef struct _MESH_INFO_T
{
    AX_U64    nPhyAddr;
    AX_VOID*  pVirAddr;
    AX_U32    nSize;
    AX_U32    nSrcWidth;
    AX_U32    nSrcHeight;
    AX_U32    nDstWidth;
    AX_U32    nDstHeight;
} MESH_INFO_S;

class CCamera;
class CIVPSStage : public CStage, public IFrameRelease
{
public:
    CIVPSStage(AX_VOID);
    virtual ~CIVPSStage(AX_VOID);

    virtual AX_BOOL Init();
    virtual AX_VOID DeInit();
    virtual AX_BOOL ProcessFrame(CMediaFrame *pFrame);
    virtual AX_VOID MediaFrameRelease(CMediaFrame *pMediaFrame);

    AX_VOID SetVENC(vector<CVideoEncoder*>* vecEncoders) { m_pVecEncoders = vecEncoders; };
    AX_VOID SetJENC(vector<CJpgEncoder*>* vecEncoders) { m_pVecJecEncoders = vecEncoders; };
    AX_VOID SetDetect(CDetectStage *pStage) { m_pDetectStage = pStage; };
    AX_VOID FrameGetThreadFunc(IVPS_GET_THREAD_PARAM_PTR pThreadParam);
    AX_VOID RgnThreadFunc(IVPS_REGION_PARAM_PTR pThreadParam);
    AX_BOOL FillCameraAttr(CCamera* pCameraInstance);
    AX_BOOL UpdateFramerate(AX_U32 nSnsFramerate);

    AX_VOID RefreshOSD(AX_U32 nOsdIndex);

    static AX_U8 GetIvpsChnNum();
    static AX_U8 GetIspChnIndex(AX_U8 nChannel);
    static AX_U8 GetIvpsChnIndex(AX_U8 nChannel);
    static AX_VOID AttrChangeNotify(AX_IVPS_ROTATION_E eOriRotation, AX_IVPS_ROTATION_E eNewRotation, AX_BOOL bMirror, AX_BOOL bFlip);
    static AX_BOOL ChangeChnAttr(IVPS_GRP nGrp, IVPS_CHN nChn, AX_IVPS_ROTATION_E eRotation, AX_BOOL bMirror, AX_BOOL bFlip, VIDEO_ATTR_T tNewAttr);
    static AX_BOOL IsResChangedForTdpRotation(IVPS_GRP nGrp, IVPS_CHN nChn, AX_IVPS_ROTATION_E eNewRotation);

private:
    AX_BOOL InitPPL();
    AX_BOOL StartIVPS();
    AX_BOOL StopIVPS();
    AX_BOOL StopWorkThread();

    /* OSD Functions */
    AX_BOOL InitOsd();
    AX_BOOL StartOSD();
    AX_BOOL StopOSD();
    AX_BOOL SetOSD(OSD_ATTR_INFO* pOsdAttr);
    AX_BOOL SetLogo(AX_U32 nIvpsGrp, AX_S32 hLogoHandle, string strPicPath, AX_U32 nPicWidth, AX_U32 nPicHeight);
    AX_BOOL IsOSDChannel(IVPS_GRP nGrp, IVPS_CHN nChn);

public:
    static IVPS_GRP_T m_arrIvpsGrp[IVPS_GROUP_NUM];

private:
    AX_VIN_CHN_ATTR_T m_tChnAttr;

    static vector<CVideoEncoder*>* m_pVecEncoders;
    static vector<CJpgEncoder*>* m_pVecJecEncoders;
    static CDetectStage *m_pDetectStage;

    COSDHandlerWrapper m_osdWrapper;

    IVPS_GET_THREAD_PARAM_T m_tGetThreadParam[MAX_VENC_CHANNEL_NUM];
    std::thread m_hGetThread[MAX_VENC_CHANNEL_NUM];

    IVPS_REGION_PARAM_T m_arrRgnThreadParam[OSD_ATTACH_NUM];
    std::thread m_arrRgnThread[OSD_ATTACH_NUM];
    OSD_ATTR_INFO m_arrOsdAttr[OSD_ATTACH_NUM];
};

#endif // _IVPS_STAGE_H_