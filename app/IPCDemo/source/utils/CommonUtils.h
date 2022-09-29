/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef _COMMON_UTILS_H_
#define _COMMON_UTILS_H_

#include "global.h"
#include "OsdConfig.h"

class CCommonUtils
{
public:
    CCommonUtils(void);
    virtual ~CCommonUtils(void);

public:
    static AX_U32 CalcImgSize(AX_U32 nW, AX_U32 nH, AX_IMG_FORMAT_E eType, AX_U32 nStride, AX_U32 nAlign);
    static AX_BOOL GetIP(const std::vector<std::string>& vNetType, AX_CHAR* pOutIPAddress);
    static AX_BOOL LoadImage(const AX_CHAR *pszImge, AX_U64 *pPhyAddr, AX_VOID **ppVirAddr, AX_U32 nImgSize);
    static AX_S32 CreateImage(AX_VIDEO_FRAME_S *pImage, AX_U32 nW, AX_U32 nH, AX_U32 nStride);
    // static AX_BOOL DumpAiKitImage(const AX_AI_KIT_Image *pImage, std::string strOutPath);
    static AX_BOOL SaveAsBin(std::string strPath, AX_VOID* pData, AX_U32 nSize, AX_BOOL bAppend = AX_FALSE);
    static AX_BOOL SaveAsBin(FILE* pFile, AX_VOID* pData, AX_U32 nSize, AX_BOOL bAppend = AX_FALSE);
    static AX_BOOL FrameSkipCtrl(AX_S32 nSrcFrameRate, AX_S32 nDstFrameRate, AX_U32 nFrameSeqNum);
    static AX_U32  CalOsdOffsetX(AX_S32 nWidth, AX_S32 nOsdWidth, AX_S32 nXMargin, OSD_ALIGN_TYPE_E eAlign);
    static AX_U32  CalOsdOffsetY(AX_S32 nHeight, AX_S32 nOSDHeight, AX_S32 nYMargin, OSD_ALIGN_TYPE_E eAlign);
};

#endif // _COMMON_UTILS_H_
