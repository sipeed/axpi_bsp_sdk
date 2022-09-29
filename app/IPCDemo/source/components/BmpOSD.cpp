/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/



#include "BmpOSD.h"
#include <stdlib.h>

#ifdef WIN32
#else
#pragma pack(2)
typedef struct tagBITMAPFILEHEADER {
    AX_U16 bfType;
    AX_U32 bfSize;
    AX_U16 bfReserved1;
    AX_U16 bfReserved2;
    AX_U32 bfOffBits;
} BITMAPFILEHEADER;
#pragma pack()

typedef struct tagBITMAPINFOHEADER {
    AX_U32 biSize;
    AX_S32 biWidth;
    AX_S32 biHeight;
    AX_U16 biPlanes;
    AX_U16 biBitCount;
    AX_U32 biCompression;
    AX_U32 biSizeImage;
    AX_S32 biXPelsPerMeter;
    AX_S32 biYPelsPerMeter;
    AX_U32 biClrUsed;
    AX_U32 biClrImportant;
} BITMAPINFOHEADER;
#endif

#define MIN_BITMAP_FILE_SIZE    (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER))

static AX_U8 reverse8(AX_U8 c)
{
    c = (c & 0x55) << 1 | (c & 0xAA) >> 1;
    c = (c & 0x33) << 2 | (c & 0xCC) >> 2;
    c = (c & 0x0F) << 4 | (c & 0xF0) >> 4;
    return c;
}

static AX_U32 FONT_MAP[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // space
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
    2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 0, 0, 0, 0, 0,  //0-9 :
    0, 0, 0, 0, 0, 0, 13, 0, 14, 0, 0, 0, 0, 0, 0, 0,  // F H
    15, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // P V
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

//////////////////////////////////////////////////////////////////////////
CBmpOSD::CBmpOSD(AX_VOID)
    : m_pData(nullptr),
      m_pFont(nullptr),
      m_uWidth(0),
      m_uHight(0)
{
}

CBmpOSD::~CBmpOSD(AX_VOID)
{
    Free();
}

AX_VOID CBmpOSD::Free(AX_VOID)
{
    if (m_pData) {
        free(m_pData);
        m_pData = nullptr;
    }
}

AX_U8 *CBmpOSD::LoadBmp(const AX_CHAR *pBmpFile, AX_U16 &u16w, AX_U16 &u16h, AX_U32 &u32Size)
{
    Free();

    FILE *fp = fopen((char *)pBmpFile, "rb");
    if (NULL == fp) {
        return nullptr;
    }

    fseek(fp, 0, SEEK_END);
    AX_U32 fsize = ftell(fp);
    if (fsize <= MIN_BITMAP_FILE_SIZE) {
        fclose(fp);
        return nullptr;
    }

    m_pData = (AX_U8 *)malloc(fsize);
    if (NULL == m_pData) {
        fclose(fp);
        return nullptr;
    }

    rewind(fp);
    if (fread(m_pData, 1, fsize, fp) != fsize) {
        Free();
        fclose(fp);
        return nullptr;
    }

    fclose(fp);

    BITMAPFILEHEADER *phd = (BITMAPFILEHEADER *)&m_pData[0];
    BITMAPINFOHEADER *pif = (BITMAPINFOHEADER *)&m_pData[sizeof(BITMAPFILEHEADER)];
    if (phd->bfType != 0x4D42) {
        /* Not a bitmap */
        Free();
        return nullptr;
    }

    if (pif->biBitCount != 1) {
        /* VO only support single-bit bitmap */
        Free();
        return nullptr;
    }

    u16w = (AX_U16)pif->biWidth;
    u16h = (AX_U16)pif->biHeight;
    m_uWidth = u16w;
    m_uHight = u16h;
    u32Size = fsize - phd->bfOffBits;
    AX_U8 *p = &m_pData[phd->bfOffBits];
    for (AX_U32 i = 0; i < u32Size; ++i) {
        p[i] = reverse8(p[i]);
    }
    m_pFont = p;

    return p;
}

AX_U8  *CBmpOSD::GetString(const AX_CHAR *pNumStri, AX_U16 &u16w, AX_U16 &u16h, AX_U32 &u32Size)
{
    if (!m_pFont || !pNumStri) {
        return nullptr;
    }
    AX_U16 uLen = strlen((char *)pNumStri);
    if (uLen == 0) {
        return nullptr;
    }

    AX_U16 uSymbolWidth = m_uWidth / 17;
    AX_U16 uSymbolWidthBytes = (uSymbolWidth + 7) / 8;
    u16w = 128;//uSymbolWidth * uLen;
    u16h = m_uHight;
    AX_U16 uStrWidthBytes = (((u16w + 7) / 8 + 3) / 4) * 4;
    u32Size = uStrWidthBytes * u16h;
    AX_U8 *pBuf = (AX_U8 *)malloc(u32Size);
    memset(pBuf, 0, u32Size);
    AX_U16 uFontWidthBytes = (((m_uWidth + 7) / 8 + 3) / 4) * 4;

    for (AX_U16 i = 0; i < uLen; i++) {
        AX_U8 *dst = pBuf + i * uSymbolWidthBytes;
        AX_U8 *src = m_pFont + FONT_MAP[(AX_U8)pNumStri[i]] * uSymbolWidthBytes;
        for (AX_U16 j = 0; j < m_uHight; j++) {
            for (AX_U16 k = 0; k < uSymbolWidthBytes; k++) {
                dst[j * uStrWidthBytes + k] = src[ j * uFontWidthBytes + k];
            }
        }
    }
    return pBuf;
}

AX_VOID CBmpOSD::FreeString(AX_U8 *pStr)
{
    if (pStr) {
        free(pStr);
    }
}

/*
 * pBmp: input
 * x,y:  number string pos, x must be 8 times
 * u16w:  bmp width
 * u16h:  bmp height;
 */
AX_BOOL CBmpOSD::FillString(const AX_CHAR *pNumStri, AX_S16 x, AX_S16 y, AX_U8 *pBmp, AX_U16 u16w, AX_U16 u16h)
{
    if (!m_pFont || !pNumStri) {
        return AX_FALSE;
    }

    AX_U16 uLen = strlen((char *)pNumStri);
    if (uLen == 0) {
        return AX_FALSE;
    }

    if (x < 0) {
        x = 0;
    }
    if (y < 0) {
        y = 0;
    }

    AX_U16 uSymbolWidth = m_uWidth / 17;
    AX_U16 uSymbolWidthBytes = (uSymbolWidth + 7) / 8;

    AX_U16 uBmpWidthBytes = (((u16w + 7) / 8 + 3) / 4) * 4;
    AX_U16 uFontWidthBytes = (((m_uWidth + 7) / 8 + 3) / 4) * 4;

    if ((x + uSymbolWidth * uLen) > u16w) {
        x = u16w - uSymbolWidth * uLen;
    }
    if ((y + m_uHight) > u16h) {
        y = u16h - m_uHight;
    }

    x = (x + 4) / 8;

    // printf("x:%d, y:%d, len:%d\n", x, y, uLen);

    for (AX_U16 i = 0; i < uLen; i++) {
        //AX_U8 uDigit = pNumStri[i] - '0';
        AX_U8 *dst = pBmp + y * uBmpWidthBytes + (x + i) * uSymbolWidthBytes;
        AX_U8 *src = m_pFont + FONT_MAP[(AX_U8)pNumStri[i]] * uSymbolWidthBytes;
        for (AX_U16 j = 0; j < m_uHight; j++) {
            for (AX_U16 k = 0; k < uSymbolWidthBytes; k++) {
                dst[j * uBmpWidthBytes + k] = src[j * uFontWidthBytes + k];
                printf("des pos:%d, src pos:%d\n", j * uBmpWidthBytes + k, j * uFontWidthBytes + k);
            }
        }
    }

    return AX_TRUE;
}

AX_BOOL CBmpOSD::FillString(const AX_CHAR *pNumStri, AX_S16 x, AX_S16 y, CYuvHandler *yuvHandler, AX_U16 u16w, AX_U16 u16h, AX_U8 nScale/* = 1*/)
{
    if (!m_pFont || !pNumStri || !yuvHandler) {
        return AX_FALSE;
    }

    AX_U16 uLen = strlen((char *)pNumStri);
    if (uLen == 0) {
        return AX_FALSE;
    }

    if (x < 0) {
        x = 0;
    }
    if (y < 0) {
        y = 0;
    }

    AX_U16 uSymbolWidth = m_uWidth / 17;
    AX_U16 uSymbolWidthBytes = (uSymbolWidth + 7) / 8;
    AX_U16 uFontWidthBytes = (((m_uWidth + 7) / 8 + 3) / 4) * 4;

    if ((x + uSymbolWidthBytes * uLen * 8 ) > u16w) {
        x = u16w - uSymbolWidthBytes * uLen * 8;
    }
    if ((y + m_uHight) > u16h) {
        y = u16h - m_uHight;
    }

    if (x < 0) {
        x = 0;
    }
    if (y < 0) {
        y = 0;
    }

    for (AX_U16 i = 0; i < uLen; i++) {
        AX_U8 *src = m_pFont + FONT_MAP[(AX_U8)pNumStri[i]] * uSymbolWidthBytes;
        for (AX_U16 j = 0; j < m_uHight; j++) {
            for (AX_U16 k = 0; k < uSymbolWidthBytes; k++) {
                AX_U8 dot8 = src[ j * uFontWidthBytes + k];
                for (AX_U16 dot = 0; dot < 8; dot++){
                    if ((dot8 & 0x1) == 1) {
                        yuvHandler->DrawPoint(x + i * uSymbolWidthBytes * 8 + k * 8 + dot, y + j, nScale, CYuvHandler::YUV_WHITE);
                    }

                    dot8 = dot8 >> 1;
                }
            }
        }
    }

    return AX_TRUE;
}
