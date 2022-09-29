/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/



#ifndef _YUV_HANDLER_189DD52E_543A_4E0E_89E0_A0204C7CE0AA_H_
#define _YUV_HANDLER_189DD52E_543A_4E0E_89E0_A0204C7CE0AA_H_

#include "global.h"

/*
    2020-08-28:
        - Add stride param, stride: https://blog.csdn.net/z827997640/article/details/81366251
*/
//////////////////////////////////////////////////////////////////////////
class CYuvHandler final
{
public:
    enum YUV_COLOR {
        YUV_GREEN = 0,
        YUV_RED,
        YUV_BLUE,
        YUV_PURPLE,
        YUV_DARK_GREEN,
        YUV_YELLOW,
        YUV_LIGHT_BLUE,
        YUV_LIGHT_PURPLE,
        YUV_DARK_BLACK,
        YUV_GRAY,
        YUV_WHITE,
        YUV_COLOR_MAX,
    };

    CYuvHandler(AX_VOID);
    CYuvHandler(const AX_U8 *pImage, AX_U16 width, AX_U16 height, AX_IMG_FORMAT_E eType, AX_S16 stride = 0, AX_BOOL bCopy = AX_FALSE);
    ~CYuvHandler(AX_VOID);

    static AX_U32  CalcImgSize(AX_U16 w, AX_U16 h, AX_IMG_FORMAT_E eType, AX_S16 stride = 0);
    static AX_BOOL SaveYUV(const AX_S8 *psFile, const AX_U8 *pImage, AX_U16 w, AX_U16 h, AX_IMG_FORMAT_E eType, AX_S16 stride = 0);

    const AX_U8 *LoadYuvFile(const AX_S8 *psFile, AX_U16 width, AX_U16 height, AX_IMG_FORMAT_E eType, AX_S16 stride = 0);
    const AX_U8 *GetImageData(AX_VOID)const;
    const AX_U32 GetImageSize(AX_VOID)const;

    /* Clip ROI Image, if pClipImage is nullptr, return real ROI size */
    AX_U32 GetClipImage(AX_S16 x0, AX_S16 y0, AX_U16 &w, AX_U16 &h, AX_U8 *pClipImage);

    const AX_U8 *DrawLine(AX_S16 x0, AX_S16 y0, AX_S16 x1, AX_S16 y1, YUV_COLOR eColor = YUV_GREEN);
    const AX_U8 *DrawRect(AX_S16 x0, AX_S16 y0, AX_U16 w, AX_U16 h, YUV_COLOR eColor = YUV_GREEN);

    AX_VOID DrawPoint(AX_S16 x, AX_S16 y, AX_U8 nScale = 1, YUV_COLOR eColor = YUV_GREEN);

private:
    AX_VOID DrawPoint(AX_U8 *y, AX_U8 *u, AX_U8 *v, AX_U16 x0, AX_U16 y0, YUV_COLOR eColor);
    AX_VOID FreeImage(AX_VOID);

private:
    AX_U8  *m_pImage;
    AX_BOOL  m_bCopy;
    AX_U16  m_nWidth;
    AX_U16 m_nHeight;
    AX_S16  m_stride;
    AX_U32 m_u32Size;
    AX_IMG_FORMAT_E m_eType;
};

#endif /* _YUV_HANDLER_189DD52E_543A_4E0E_89E0_A0204C7CE0AA_H_ */
