/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/



#include "YuvHandler.h"
#include <fstream>
#include <string.h>
using namespace std;

///
static struct YUV_COLOR_T {
    AX_U8 Y;
    AX_U8 U;
    AX_U8 V;
} g_YuvColors[CYuvHandler::YUV_COLOR_MAX] = {
    {0x00, 0x00, 0x00}, // green
    {0x00, 0x00, 0xff}, // red
    {0x00, 0xff, 0x00}, // blue
    {0x00, 0xff, 0xff}, // purple
    {0xff, 0x00, 0x00}, // dark green
    {0xff, 0x00, 0xff}, // yellow
    {0xff, 0xff, 0x00}, // light blue
    {0xff, 0xff, 0xff}, // light purple
    {0x00, 0x80, 0x80}, // dark black
    {0x80, 0x80, 0x80}, // gray
    {0xff, 0x80, 0x80}  // white
};

//////////////////////////////////////////////////////////////////////////
CYuvHandler::CYuvHandler(AX_VOID)
    : m_pImage(nullptr)
    , m_bCopy(AX_FALSE)
    , m_nWidth(0)
    , m_nHeight(0)
    , m_stride(0)
    , m_u32Size(0)
    , m_eType(AX_YUV420_PLANAR)
{
}

CYuvHandler::CYuvHandler(const AX_U8 *pImage, AX_U16 width, AX_U16 height, AX_IMG_FORMAT_E eType, AX_S16 stride/* = 0 */, AX_BOOL bCopy/* = AX_FALSE */)
    : m_pImage(const_cast<AX_U8 *>(pImage))
    , m_bCopy(bCopy)
    , m_nWidth(width)
    , m_nHeight(height)
    , m_stride((0 == stride) ? width : stride)
    , m_eType(eType)
{
    m_u32Size = CalcImgSize(m_nWidth, height, eType, m_stride);
    if (bCopy) {
        m_pImage = (AX_U8 *)malloc(sizeof(AX_U8) * m_u32Size);
        if (m_pImage) {
            memcpy(m_pImage, pImage, m_u32Size);
        }
    }
}

CYuvHandler::~CYuvHandler(AX_VOID)
{
    FreeImage();
}

AX_VOID CYuvHandler::FreeImage(AX_VOID)
{
    if (m_bCopy && m_pImage) {
        free(m_pImage);
        m_pImage = nullptr;
    }
}

const AX_U8 *CYuvHandler::GetImageData(AX_VOID)const
{
    return m_pImage;
}

const AX_U32 CYuvHandler::GetImageSize(AX_VOID)const
{
    return m_u32Size;
}

const AX_U8 *CYuvHandler::LoadYuvFile(const AX_S8 *psFile, AX_U16 width, AX_U16 height, AX_IMG_FORMAT_E eType, AX_S16 stride/* = 0 */)
{
    FreeImage();

    m_bCopy   = AX_TRUE;
    m_nWidth  = width;
    m_stride  = (0 == stride) ? width : stride;
    m_nHeight = height;
    m_eType   = eType;
    m_u32Size = CalcImgSize(m_nWidth, height, eType, m_stride);
    ifstream f((char *)psFile, ios::in | ios::binary);
    if (f) {
        m_pImage = (AX_U8 *)malloc(sizeof(AX_U8) * m_u32Size);
        if (m_pImage) {
            f.read((char *)m_pImage, m_u32Size);
            if (!f) {
                free(m_pImage);
                m_pImage = nullptr;
            }
        }
        f.close();
    }

    return m_pImage;
}

AX_U32 CYuvHandler::CalcImgSize(AX_U16 w, AX_U16 h, AX_IMG_FORMAT_E eType, AX_S16 stride/* = 0 */)
{
    AX_U32 u32Size = 0;
    AX_U16 w0 = ((0 == stride) ? w : stride);
    switch (eType) {
    case AX_YUV420_PLANAR:
    case AX_YUV420_SEMIPLANAR:
    case AX_YUV420_SEMIPLANAR_VU:
        u32Size = (AX_U32)(w0 * h * 3) / 2;
        break;
    case AX_YUV422_INTERLEAVED_UYVY:
    case AX_YUV422_INTERLEAVED_YUYV:
        u32Size = (AX_U32)(w0 * h * 2);
        break;
    default:
        u32Size = 0;
        break;
    }

    return u32Size;
}

AX_VOID CYuvHandler::DrawPoint(AX_U8 *y, AX_U8 *u, AX_U8 *v, AX_U16 x0, AX_U16 y0, YUV_COLOR eColor)
{
    AX_U32 y_offset = 0;
    AX_U32 u_offset = 0;
    AX_U32 v_offset = 0;
    switch (m_eType) {
    case AX_YUV420_PLANAR: // YUV420 I420
        /* YYYY...UUUU...VVVV */
        y_offset = (AX_U32)(y0 * m_stride + x0);
        u_offset = (AX_U32)((y0 / 2) * (m_stride / 2) + x0 / 2);
        v_offset = u_offset;

        y[y_offset] = g_YuvColors[eColor].Y;
        u[u_offset] = g_YuvColors[eColor].U;
        v[v_offset] = g_YuvColors[eColor].V;
        break;

    case AX_YUV420_SEMIPLANAR: // YUV420SP NV12
        /* YYYY...UVUV */
        y_offset = (AX_U32)(y0 * m_stride + x0);
        u_offset = (AX_U32)((y0 / 2) * m_stride + x0 / 2 * 2);
        v_offset = u_offset + 1;

        if (g_YuvColors[eColor].Y == 0xFF) {
            y[y_offset] = g_YuvColors[eColor].Y;
        }
        else {
            u[u_offset] = g_YuvColors[eColor].U;
            v[v_offset] = g_YuvColors[eColor].V;
        }
        break;

    case AX_YUV420_SEMIPLANAR_VU: // YUV420SP NV21
        /* YYYY...VUVU */
        y_offset = (AX_U32)(y0 * m_stride + x0);
        v_offset = (AX_U32)((y0 / 2) * m_stride + x0 / 2 * 2);
        u_offset = v_offset + 1;

        y[y_offset] = g_YuvColors[eColor].Y;
        u[u_offset] = g_YuvColors[eColor].U;
        v[v_offset] = g_YuvColors[eColor].V;
        break;

    case AX_YUV422_INTERLEAVED_UYVY:
        /* UYVYUYVY */
        u_offset = (AX_U32)(y0 * m_stride * 2 + x0 / 2 * 4);
        v_offset = u_offset + 2;
        y_offset = u_offset + 1;
        y[y_offset]     = g_YuvColors[eColor].Y;
        y[y_offset + 2] = g_YuvColors[eColor].Y;
        y[u_offset]     = g_YuvColors[eColor].U;
        y[v_offset]     = g_YuvColors[eColor].V;
        break;

    case AX_YUV422_INTERLEAVED_YUYV:
        /* YUYVYUYV */
        y_offset = (AX_U32)(y0 * m_stride * 2 + x0 / 2 * 4);
        u_offset = y_offset + 1;
        v_offset = u_offset + 2;

        y[y_offset]     = g_YuvColors[eColor].Y;
        y[y_offset + 2] = g_YuvColors[eColor].Y;
        y[u_offset]     = g_YuvColors[eColor].U;
        y[v_offset]     = g_YuvColors[eColor].V;
        break;

    default:
        break;
    }
}

AX_VOID CYuvHandler::DrawPoint(AX_S16 x, AX_S16 y, AX_U8 nScale/* = 1*/, YUV_COLOR eColor/* = YUV_GREEN*/)
{
    if (!m_pImage) {
        return;
    }

    if (x < 0 || x > m_nWidth) {
        return;
    }

    if (y < 0 || y > m_nHeight) {
        return;
    }

    AX_U8 *pY = nullptr;
    AX_U8 *pU = nullptr;
    AX_U8 *pV = nullptr;
    switch (m_eType) {
    case AX_YUV420_PLANAR:
        pY = m_pImage;
        pU = m_pImage + m_stride * m_nHeight;
        pV = pU + m_stride * m_nHeight / 4;
        break;
    case AX_YUV420_SEMIPLANAR:
    case AX_YUV420_SEMIPLANAR_VU:
        pY = m_pImage;
        pU = m_pImage + m_stride * m_nHeight;
        pV = pU;
        break;
    case AX_YUV422_INTERLEAVED_UYVY:
    case AX_YUV422_INTERLEAVED_YUYV:
        pY = m_pImage;
        pU = pY;
        pV = pY;
        break;
    default:
        break;
    }

    for(uint32_t hScale = 0; hScale < nScale; hScale++)
    {
        for(uint32_t wScale = 0; wScale < nScale; wScale++)
        {
            DrawPoint(pY, pU, pV, x * nScale + hScale, y * nScale + wScale, eColor);
        }
    }
}

const AX_U8 *CYuvHandler::DrawLine(AX_S16 x0, AX_S16 y0, AX_S16 x1, AX_S16 y1, YUV_COLOR eColor/* = YUV_GREEN*/)
{
    if (!m_pImage) {
        return nullptr;
    }

    x0 = (x0 < 0) ? 0 : x0;
    y0 = (y0 < 0) ? 0 : y0;
    x1 = (x1 < 0) ? 0 : x1;
    y1 = (y1 < 0) ? 0 : y1;

    x0 = (x0 >= m_nWidth)  ? m_nWidth  - 1 : x0;
    y0 = (y0 >= m_nHeight) ? m_nHeight - 1 : y0;
    x1 = (x1 >= m_nWidth)  ? m_nWidth  - 1 : x1;
    y1 = (y1 >= m_nHeight) ? m_nHeight - 1 : y1;

    AX_U16 dx = (x0 > x1) ? (x0 - x1) : (x1 - x0);
    AX_U16 dy = (y0 > y1) ? (y0 - y1) : (y1 - y0);

    AX_S16 xstep = (x0 < x1) ? 1 : -1;
    AX_S16 ystep = (y0 < y1) ? 1 : -1;
    AX_S16 nstep = 0, eps = 0;

    AX_U8 *pY = nullptr;
    AX_U8 *pU = nullptr;
    AX_U8 *pV = nullptr;
    switch (m_eType) {
    case AX_YUV420_PLANAR:
        pY = m_pImage;
        pU = m_pImage + m_stride * m_nHeight;
        pV = pU + m_stride * m_nHeight / 4;
        break;
    case AX_YUV420_SEMIPLANAR:
    case AX_YUV420_SEMIPLANAR_VU:
        pY = m_pImage;
        pU = m_pImage + m_stride * m_nHeight;
        pV = pU;
        break;
    case AX_YUV422_INTERLEAVED_UYVY:
    case AX_YUV422_INTERLEAVED_YUYV:
        pY = m_pImage;
        pU = pY;
        pV = pY;
        break;
    default:
        break;
    }

    AX_U16 x = x0;
    AX_U16 y = y0;
    if (dx > dy) {
        while (nstep <= dx) {
            DrawPoint(pY, pU, pV, x, y, eColor);
            eps += dy;
            if ((eps << 1) >= dx) {
                y += ystep;
                eps -= dx;
            }
            x += xstep;
            nstep++;
        }
    } else {
        while (nstep <= dy) {
            DrawPoint(pY, pU, pV, x, y, eColor);
            eps += dx;
            if ((eps << 1) >= dy) {
                x += xstep;
                eps -= dy;
            }
            y += ystep;
            nstep++;
        }
    }

    return pY;
}

const AX_U8 *CYuvHandler::DrawRect(AX_S16 x0, AX_S16 y0, AX_U16 w, AX_U16 h, YUV_COLOR eColor/* = YUV_GREEN*/)
{
    if (!m_pImage) {
        return nullptr;
    }

    if (w > 0 && h > 0) {
        DrawLine(x0, y0, x0 + w, y0, eColor);
        DrawLine(x0, y0, x0, y0 + h, eColor);
        DrawLine(x0 + w, y0, x0 + w, y0 + h, eColor);
        DrawLine(x0, y0 + h, x0 + w, y0 + h, eColor);
    }

    return m_pImage;
}

AX_U32 CYuvHandler::GetClipImage(AX_S16 x0, AX_S16 y0, AX_U16 &w, AX_U16 &h, AX_U8 *pClipImage)
{
    AX_U32 clipSize = 0;

    if (!m_pImage || w == 0 || h == 0) {
        return clipSize;
    }

    x0 = (x0 < 0) ? 0 : x0;
    y0 = (y0 < 0) ? 0 : y0;
    x0 = (x0 + 1) / 2 * 2;
    y0 = (y0 + 1) / 2 * 2;
    /* 8 pixel padding */
    w  = ((w % 8) == 0) ?  w : ((w & 0xFFF8) + 8);
    h  = ((h % 8) == 0) ?  h : ((h & 0xFFF8) + 8);

    if (x0 + w > m_nWidth) {
        w = m_nWidth - x0;
        w = (w & 0xFFF8);
    }

    if (y0 + h > m_nHeight) {
        h = m_nHeight - y0;
        h = (h & 0xFFF8);
    }

    clipSize = CalcImgSize(w, h, m_eType);
    if (!pClipImage) {
        return clipSize;
    }

    AX_U8 *pY = nullptr;
    AX_U8 *pV = nullptr;
    AX_U8 *pO = pClipImage;

    switch(m_eType) {
    case AX_YUV420_SEMIPLANAR:
    case AX_YUV420_SEMIPLANAR_VU:
        /* YYYY UVUV or YYYY VUVU */
        /* Copy Y */
        pY = m_pImage + y0 * m_stride + x0;
        for (AX_U16 i = 0; i < h; ++i) {
            memcpy(pO, pY, w);
            pY += m_stride;
            pO += w;
        }

        /* Copy UV */
        pV = m_pImage + m_stride * m_nHeight + (y0 * m_stride/2 + x0);
        for (AX_U16 i = 0; i < (h / 2); ++i) {
            memcpy(pO, pV, w);
            pV += m_stride;
            pO += w;
        }
        break;
    default:
        break;
    }

    return clipSize;
}

AX_BOOL CYuvHandler::SaveYUV(const AX_S8 *psFile, const AX_U8 *pImage, AX_U16 w, AX_U16 h, AX_IMG_FORMAT_E eType, AX_S16 stride/* = 0*/)
{
    if (psFile && pImage && w > 0 && h > 0) {
        ofstream f((char *)psFile, ios::out | ios::binary);
        if (f) {
            f.write((const char *)pImage, CalcImgSize(w, h, eType, stride));
            f.close();
            return AX_TRUE;
        }
    }

    return AX_FALSE;
}
