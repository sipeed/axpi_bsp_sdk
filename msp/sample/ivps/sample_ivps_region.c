/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include "ax_sys_api.h"
#include "sample_ivps_util.h"
#include "sample_ivps_object.h"
#include "sample_ivps_region.h"
#include "ivps_bitmap_text.h"

static SAMPLE_REGION_INFO_S gRegions[AX_IVPS_MAX_RGN_HANDLE_NUM] = {
    {
        .handle = AX_IVPS_INVALID_REGION_HANDLE,
        .IvpsGrp = 1,
        .IvpsFilter = 0x10, /* CHN0 FILTER0 */
    },
    {
        .handle = AX_IVPS_INVALID_REGION_HANDLE,
        .IvpsGrp = 1,
        .IvpsFilter = 0x10,
    },
    {
        .handle = AX_IVPS_INVALID_REGION_HANDLE,
        .IvpsGrp = 1,
        .IvpsFilter = 0x10,
    },
    {
        .handle = AX_IVPS_INVALID_REGION_HANDLE,
        .IvpsGrp = 1,
        .IvpsFilter = 0x10,
    },
};

AX_VOID IVPS_DrawSample(IVPS_RGN_HANDLE handle)
{
    AX_IVPS_RGN_CANVAS_INFO_S tCanvas;
    AX_IVPS_GDI_ATTR_S tAttr;

    //    AX_IVPS_RGN_GetCanvas(handle, &tCanvas);
    // ALOGI("nStride %d nW:%d nH:%d\n", tCanvas.nStride, tCanvas.nW, tCanvas.nH);

    tAttr.nColor = GREEN;
    tAttr.nAlpha = 0xFF;
    tAttr.nThick = (rand() % 16);
    tAttr.bSolid = AX_TRUE;

    AX_IVPS_POINT_S tLine[2];
    tLine[0].nX = 200;
    tLine[0].nY = 100;
    tLine[1].nX = 200 + ALIGN_UP((rand() % 100), 2);
    tLine[1].nY = 100 + ALIGN_UP((rand() % 100), 2);
    ALOGI("X0:%d Y0:%d\n", tLine[0].nX, tLine[0].nY);
    ALOGI("X1:%d Y1:%d\n", tLine[1].nX, tLine[1].nY);
    AX_IVPS_DrawLine(&tCanvas, tAttr, tLine, 2);

    tAttr.nColor = PALETURQUOISE;
    tAttr.nAlpha = 0xFF;
    tAttr.nThick = 4;
    tAttr.bSolid = AX_TRUE;

    AX_IVPS_POINT_S tPolygon[4];
    tPolygon[0].nX = 60;
    tPolygon[0].nY = 50;
    tPolygon[1].nX = 100;
    tPolygon[1].nY = 20;

    tPolygon[2].nX = 200;
    tPolygon[2].nY = 70;

    tPolygon[3].nX = 110;
    tPolygon[3].nY = 380;
    AX_IVPS_DrawPolygon(&tCanvas, tAttr, tPolygon, 4);

    tAttr.nColor = BLUE;
    tAttr.nAlpha = 0xFF;
    tAttr.nThick = 5;
    tAttr.bSolid = AX_FALSE;
    AX_IVPS_RECT_S tRect;

    tRect.nX = 500;
    tRect.nY = 150;
    tRect.nW = 100;
    tRect.nH = 50;
    AX_IVPS_DrawRect(&tCanvas, tAttr, tRect);

    tAttr.nColor = RED;
    tAttr.nAlpha = 0xFF;
    tAttr.nThick = 5;
    tAttr.bSolid = AX_TRUE;
    tRect.nX = 100;
    tRect.nY = 100;
    tRect.nW = 100;
    tRect.nH = 100;
    AX_IVPS_DrawRect(&tCanvas, tAttr, tRect);

    tAttr.nColor = LIGHTCYAN;
    tAttr.nAlpha = 0xFF;
    tAttr.nThick = 5;

    AX_IVPS_POINT_S tFoldLine[3];
    tFoldLine[0].nX = 200;
    tFoldLine[0].nY = 100;
    tFoldLine[1].nX = 500;
    tFoldLine[1].nY = 320;
    tFoldLine[2].nX = 300;
    tFoldLine[2].nY = 250;
    AX_IVPS_DrawLine(&tCanvas, tAttr, tFoldLine, 3);

    tAttr.nColor = DARKGREEN;
    tAttr.nAlpha = 0xFF;
    tAttr.nThick = 1;

    tRect.nX = 400;
    tRect.nY = 300;
    tRect.nW = 100;
    tRect.nH = 60;
    AX_IVPS_POINT_S CornerRect[3];
    CornerRect[1].nX = tRect.nX;
    CornerRect[1].nY = tRect.nY;
    CornerRect[0].nX = CornerRect[1].nX + tRect.nW / 5;
    CornerRect[0].nY = CornerRect[1].nY;
    CornerRect[2].nX = CornerRect[1].nX;
    CornerRect[2].nY = CornerRect[1].nY + tRect.nH / 5;
    AX_IVPS_DrawLine(&tCanvas, tAttr, CornerRect, 3);

    CornerRect[1].nX = tRect.nX + tRect.nW;
    CornerRect[1].nY = tRect.nY;
    CornerRect[0].nX = CornerRect[1].nX - tRect.nW / 5;
    CornerRect[0].nY = CornerRect[1].nY;
    CornerRect[2].nX = CornerRect[1].nX;
    CornerRect[2].nY = CornerRect[1].nY + tRect.nH / 5;
    AX_IVPS_DrawLine(&tCanvas, tAttr, CornerRect, 3);

    CornerRect[1].nX = tRect.nX;
    CornerRect[1].nY = tRect.nY + tRect.nH;
    CornerRect[0].nX = CornerRect[1].nX + tRect.nW / 5;
    CornerRect[0].nY = CornerRect[1].nY;
    CornerRect[2].nX = CornerRect[1].nX;
    CornerRect[2].nY = CornerRect[1].nY - tRect.nH / 5;
    AX_IVPS_DrawLine(&tCanvas, tAttr, CornerRect, 3);

    CornerRect[1].nX = tRect.nX + tRect.nW;
    CornerRect[1].nY = tRect.nY + tRect.nH;
    CornerRect[0].nX = CornerRect[1].nX - tRect.nW / 5;
    CornerRect[0].nY = CornerRect[1].nY;
    CornerRect[2].nX = CornerRect[1].nX;
    CornerRect[2].nY = CornerRect[1].nY - tRect.nH / 5;

    AX_IVPS_DrawLine(&tCanvas, tAttr, CornerRect, 3);

    // AX_IVPS_RGN_UpdateCanvas(handle);
}

AX_S32 IVPS_DrawWithCpuSample(AX_VIDEO_FRAME_S *pstVFrame)
{
    printf("AX_IVPS_DrawLine\n");
    AX_IVPS_RGN_CANVAS_INFO_S tCanvas;
    AX_IVPS_GDI_ATTR_S tAttr;

    tCanvas.eFormat = AX_YUV420_SEMIPLANAR;
    tCanvas.pVirAddr = (AX_VOID *)((AX_ULONG)pstVFrame->u64VirAddr[0]);
    tCanvas.nStride = pstVFrame->u32PicStride[0];
    tCanvas.nW = pstVFrame->u32Width;
    tCanvas.nH = pstVFrame->u32Height;

    AX_IVPS_POINT_S tPoint[10];
    tAttr.nThick = 5;
    tAttr.nColor = 0xFF0000;
    tPoint[0].nX = 10;
    tPoint[0].nY = 10;
    tPoint[1].nX = 100;
    tPoint[1].nY = 100;
    tPoint[2].nX = 50;
    tPoint[2].nY = 200;
    tPoint[3].nX = 440;
    tPoint[3].nY = 570;

    AX_IVPS_DrawLine(&tCanvas, tAttr, tPoint, 4);

    tAttr.nThick = 2;
    tAttr.nColor = 0xFF8000;
    tPoint[0].nX = 10 + 500;
    tPoint[0].nY = 10 + 500;
    tPoint[1].nX = 100 + 500;
    tPoint[1].nY = 100 + 600;
    tPoint[2].nX = 50 + 700;
    tPoint[2].nY = 200 + 400;
    tPoint[3].nX = 440;
    tPoint[3].nY = 570;

    AX_IVPS_DrawLine(&tCanvas, tAttr, tPoint, 4);

    // tAttr.nThick = 2;
    tAttr.nColor = 0xFF0080;
    tPoint[0].nX = 800;
    tPoint[0].nY = 800;
    tPoint[1].nX = 900;
    tPoint[1].nY = 900;
    tPoint[2].nX = 800;
    tPoint[2].nY = 1000;
    tPoint[3].nX = 700;
    tPoint[3].nY = 900;

    AX_IVPS_DrawPolygon(&tCanvas, tAttr, tPoint, 4);

    AX_IVPS_RECT_S tRect;

    // tAttr.nThick = 2;
    tAttr.bSolid = AX_FALSE;
    tAttr.nColor = 0xFFFF80;
    tRect.nX = 1000;
    tRect.nY = 900;
    tRect.nW = 600;
    tRect.nH = 300;

    AX_IVPS_DrawRect(&tCanvas, tAttr, tRect);

    SaveFileExt(pstVFrame, 0, 0, ".", "CPUDraw");
    return 0;
}
static AX_S32 Sample_ByteReverse(AX_U8 *ptBitmap, AX_S32 nSize)
{
    int idx;
    unsigned char data;
    for (idx = 0; idx < nSize; idx++)
    {
        data = *(ptBitmap + idx);
        data = (data & 0x55) << 1 | (data & 0xAA) >> 1; // swap 12,34,56,78
        data = (data & 0x33) << 2 | (data & 0xCC) >> 2; // swap (21 43),(65 87)
        data = (data & 0x0F) << 4 | (data & 0xF0) >> 4; // swap 4bit
        *(ptBitmap + idx) = data;
    }
    return IVPS_SUCC;
}

/* Line/Polygon config */
static AX_VOID IVPS_RegionCfg0(AX_IVPS_RGN_DISP_GROUP_S *ptRegion)
{

    /*
    nZindex: Required
    Indicates which layer to draw Line/Polygon/OSD on.
    If Filter engine type is TDP, nZindex is 0 ~ 4.
    If Filter engine type is VO, nZindex is 0 ~ 31.
    nAlpha: Required
            0 - 1024, 0: transparent, 1024: opaque.
    eFormat: Required
            TDP engine support AX_FORMAT_ARGB8888/AX_FORMAT_RGBA8888.
            VO engine support AX_FORMAT_ARGB8888.
            The sub arrDisp's format should be the same as the for format.
    */
    ptRegion->nNum = 4;
    ptRegion->tChnAttr.nZindex = 0;
    ptRegion->tChnAttr.nAlpha = 1024; /*0 - 1024, 0: transparent, 1024: opaque*/
    ptRegion->tChnAttr.eFormat = AX_FORMAT_ARGB8888;

    ptRegion->arrDisp[0].bShow = AX_TRUE;
    ptRegion->arrDisp[0].eType = AX_IVPS_RGN_TYPE_LINE;
    ptRegion->arrDisp[0].uDisp.tLine.nPointNum = 4;
    ptRegion->arrDisp[0].uDisp.tLine.tPTs[0].nX = 0;
    ptRegion->arrDisp[0].uDisp.tLine.tPTs[0].nY = 0;
    ptRegion->arrDisp[0].uDisp.tLine.tPTs[1].nX = 200;
    ptRegion->arrDisp[0].uDisp.tLine.tPTs[1].nY = 100;
    ptRegion->arrDisp[0].uDisp.tLine.tPTs[2].nX = 300;
    ptRegion->arrDisp[0].uDisp.tLine.tPTs[2].nY = 100;
    ptRegion->arrDisp[0].uDisp.tLine.tPTs[3].nX = 400;
    ptRegion->arrDisp[0].uDisp.tLine.tPTs[3].nY = 300;
    ptRegion->arrDisp[0].uDisp.tLine.nLineWidth = 4;
    ptRegion->arrDisp[0].uDisp.tLine.nColor = GREEN;
    ptRegion->arrDisp[0].uDisp.tLine.nAlpha = 200 /*0 - 255, 0: transparent, 255: opaque*/;

    ptRegion->arrDisp[1].bShow = AX_TRUE;
    ptRegion->arrDisp[1].eType = AX_IVPS_RGN_TYPE_POLYGON;
    ptRegion->arrDisp[1].uDisp.tPolygon.nPointNum = 4;
    ptRegion->arrDisp[1].uDisp.tPolygon.tPTs[0].nX = 0;
    ptRegion->arrDisp[1].uDisp.tPolygon.tPTs[0].nY = 0;
    ptRegion->arrDisp[1].uDisp.tPolygon.tPTs[1].nX = 200 + 50;
    ptRegion->arrDisp[1].uDisp.tPolygon.tPTs[1].nY = 100 + 50;
    ptRegion->arrDisp[1].uDisp.tPolygon.tPTs[2].nX = 300 + 50;
    ptRegion->arrDisp[1].uDisp.tPolygon.tPTs[2].nY = 100 + 50;
    ptRegion->arrDisp[1].uDisp.tPolygon.tPTs[3].nX = 400 + 50;
    ptRegion->arrDisp[1].uDisp.tPolygon.tPTs[3].nY = 300 + 50;
    ptRegion->arrDisp[1].uDisp.tPolygon.bSolid = AX_TRUE;
    ptRegion->arrDisp[1].uDisp.tPolygon.nLineWidth = 4;
    ptRegion->arrDisp[1].uDisp.tPolygon.nColor = YELLOW;
    ptRegion->arrDisp[1].uDisp.tPolygon.nAlpha = 200;

    ptRegion->arrDisp[2].bShow = AX_TRUE;
    ptRegion->arrDisp[2].eType = AX_IVPS_RGN_TYPE_RECT;
    ptRegion->arrDisp[2].uDisp.tPolygon.tRect.nX = 400;
    ptRegion->arrDisp[2].uDisp.tPolygon.tRect.nY = 200;
    ptRegion->arrDisp[2].uDisp.tPolygon.tRect.nW = 100;
    ptRegion->arrDisp[2].uDisp.tPolygon.tRect.nH = 200;
    ptRegion->arrDisp[2].uDisp.tPolygon.bSolid = AX_FALSE;
    ptRegion->arrDisp[2].uDisp.tPolygon.nLineWidth = 4;
    ptRegion->arrDisp[2].uDisp.tPolygon.nColor = RED;
    ptRegion->arrDisp[2].uDisp.tPolygon.nAlpha = 200;

    ptRegion->arrDisp[3].bShow = AX_TRUE;
    ptRegion->arrDisp[3].eType = AX_IVPS_RGN_TYPE_RECT;
    ptRegion->arrDisp[3].uDisp.tPolygon.tRect.nX = 600;
    ptRegion->arrDisp[3].uDisp.tPolygon.tRect.nY = 400;
    ptRegion->arrDisp[3].uDisp.tPolygon.tRect.nW = 100;
    ptRegion->arrDisp[3].uDisp.tPolygon.tRect.nH = 80;
    ptRegion->arrDisp[3].uDisp.tPolygon.bSolid = AX_FALSE;
    ptRegion->arrDisp[3].uDisp.tPolygon.bCornerRect = AX_TRUE;
    ptRegion->arrDisp[3].uDisp.tPolygon.nLineWidth = 2;
    ptRegion->arrDisp[3].uDisp.tPolygon.nColor = GREEN;
    ptRegion->arrDisp[3].uDisp.tPolygon.nAlpha = 200;
}

/* OSD config */
static AX_VOID IVPS_RegionCfg1(AX_IVPS_RGN_DISP_GROUP_S *ptRegion)
{
    char arrPath1[] = "/opt/data/ivps/OSD_754x70.argb1555";
    char arrPath2[] = "/opt/data/ivps/OSD_634x60.argb1555";

    /*
    nZindex: Required
            Indicates which layer to draw Line/Polygon/OSD on.
            If Filter engine type is TDP, nZindex is 0 ~ 4.
            If Filter engine type is VO, nZindex is 0 ~ 31.
    nAlpha: Required
            0 - 1024, 0: transparent, 1024: opaque.
    eFormat: Required
            TDP engine support AX_FORMAT_RGBA8888/AX_FORMAT_ARGB8888/AX_FORMAT_ARGB1555/AX_FORMAT_RGBA5551/AX_FORMAT_ARGB4444/AX_FORMAT_RGBA4444/AX_FORMAT_ARGB8565.
            VO engine  can only support AX_FORMAT_ARGB8888/AX_FORMAT_ARGB1555/AX_FORMAT_ARGB4444/AX_FORMAT_ARGB8565.
            The sub arrDisp's format should be the same as the for format.
    Note:   u64PhyAddr of tOSD is not required. But pBitmap is required.
    */
    ptRegion->nNum = 2;
    ptRegion->tChnAttr.nZindex = 1;
    ptRegion->tChnAttr.nAlpha = 1024; /*0 - 1024, 0: transparent, 1024: opaque*/
    ptRegion->tChnAttr.eFormat = AX_FORMAT_ARGB1555;
    ptRegion->arrDisp[0].bShow = AX_TRUE;
    ptRegion->arrDisp[0].eType = AX_IVPS_RGN_TYPE_OSD;
    ptRegion->arrDisp[0].uDisp.tOSD.u16Alpha = 512;
    ptRegion->arrDisp[0].uDisp.tOSD.enRgbFormat = AX_FORMAT_ARGB1555;
    ptRegion->arrDisp[0].uDisp.tOSD.u32ColorKey = 0x0;
    ptRegion->arrDisp[0].uDisp.tOSD.u32BgColorLo = 0xffffffff;
    ptRegion->arrDisp[0].uDisp.tOSD.u32BgColorHi = 0xffffffff;
    ptRegion->arrDisp[0].uDisp.tOSD.u32BmpWidth = 754;
    ptRegion->arrDisp[0].uDisp.tOSD.u32BmpHeight = 70;
    ptRegion->arrDisp[0].uDisp.tOSD.u32DstXoffset = 1000; // 128;
    ptRegion->arrDisp[0].uDisp.tOSD.u32DstYoffset = 150;  // 40;
    LoadImage(arrPath1, 0, (AX_U64 *)&ptRegion->arrDisp[0].uDisp.tOSD.u64PhyAddr, (AX_VOID **)&ptRegion->arrDisp[0].uDisp.tOSD.pBitmap);

    ptRegion->arrDisp[1].bShow = AX_TRUE;
    ptRegion->arrDisp[1].eType = AX_IVPS_RGN_TYPE_OSD;
    ptRegion->arrDisp[1].uDisp.tOSD.u16Alpha = 512;
    ptRegion->arrDisp[1].uDisp.tOSD.enRgbFormat = AX_FORMAT_ARGB1555;
    ptRegion->arrDisp[1].uDisp.tOSD.u32ColorKey = 0x0;
    ptRegion->arrDisp[1].uDisp.tOSD.u32BgColorLo = 0xffffffff;
    ptRegion->arrDisp[1].uDisp.tOSD.u32BgColorHi = 0xffffffff;
    ptRegion->arrDisp[1].uDisp.tOSD.u32BmpWidth = 634;
    ptRegion->arrDisp[1].uDisp.tOSD.u32BmpHeight = 60;
    ptRegion->arrDisp[1].uDisp.tOSD.u32DstXoffset = 800; // 300;
    ptRegion->arrDisp[1].uDisp.tOSD.u32DstYoffset = 50;  // 40;
    LoadImage(arrPath2, 0, (AX_U64 *)&ptRegion->arrDisp[1].uDisp.tOSD.u64PhyAddr, (AX_VOID **)&ptRegion->arrDisp[1].uDisp.tOSD.pBitmap);
}

static AX_VOID IVPS_RegionCfg3(AX_IVPS_RGN_DISP_GROUP_S *ptRegion)
{
    /*
    nZindex: Required
            Indicates which layer to draw Line/Polygon/OSD on.
            If Filter engine type is TDP, nZindex is 0 ~ 4.
            If Filter engine type is VO, nZindex is 0 ~ 31.
    nAlpha: Required
            0 - 1024, 0: transparent, 1024: opaque.
    eFormat: Required
            TDP engine support AX_FORMAT_RGBA8888/AX_FORMAT_ARGB8888/AX_FORMAT_ARGB1555/AX_FORMAT_RGBA5551/AX_FORMAT_ARGB4444/AX_FORMAT_RGBA4444/AX_FORMAT_ARGB8565.
            VO engine  can only support AX_FORMAT_ARGB8888/AX_FORMAT_ARGB1555/AX_FORMAT_ARGB4444/AX_FORMAT_ARGB8565.
            The sub arrDisp's format should be the same as the for format.
    Note:   u64PhyAddr of tOSD is not required. But pBitmap is required.
    */
    Sample_ByteReverse(&bitmap_cursor[0], sizeof(bitmap_cursor));
    ptRegion->nNum = 2;
    ptRegion->tChnAttr.nZindex = 3;
    ptRegion->tChnAttr.nAlpha = 1024; /*0 - 1024, 0: transparent, 1024: opaque*/
    ptRegion->tChnAttr.eFormat = AX_FORMAT_BITMAP;
    ptRegion->tChnAttr.nBitColor.nColor = 0xFF0000;
    ptRegion->tChnAttr.nBitColor.bEnable = AX_TRUE;
    ptRegion->tChnAttr.nBitColor.nColorInv = 0xFF;
    ptRegion->tChnAttr.nBitColor.nColorInvThr = 0xA0A0A0;
    ptRegion->arrDisp[0].bShow = AX_TRUE;
    ptRegion->arrDisp[0].eType = AX_IVPS_RGN_TYPE_OSD;
    ptRegion->arrDisp[0].uDisp.tOSD.u16Alpha = 512;
    ptRegion->arrDisp[0].uDisp.tOSD.enRgbFormat = AX_FORMAT_BITMAP;
    ptRegion->arrDisp[0].uDisp.tOSD.u32ColorKey = 0x0;
    ptRegion->arrDisp[0].uDisp.tOSD.u32BgColorLo = 0xffffffff;
    ptRegion->arrDisp[0].uDisp.tOSD.u32BgColorHi = 0xffffffff;
    ptRegion->arrDisp[0].uDisp.tOSD.u32BmpWidth = 240;
    ptRegion->arrDisp[0].uDisp.tOSD.u32BmpHeight = 227;
    ptRegion->arrDisp[0].uDisp.tOSD.u32DstXoffset = 512;
    ptRegion->arrDisp[0].uDisp.tOSD.u32DstYoffset = 400;
    ptRegion->arrDisp[0].uDisp.tOSD.pBitmap = &bitmap_cursor[0];

    ptRegion->arrDisp[1].bShow = AX_TRUE;
    ptRegion->arrDisp[1].eType = AX_IVPS_RGN_TYPE_OSD;
    ptRegion->arrDisp[1].uDisp.tOSD.u16Alpha = 512;
    ptRegion->arrDisp[1].uDisp.tOSD.enRgbFormat = AX_FORMAT_BITMAP;
    ptRegion->arrDisp[1].uDisp.tOSD.u32ColorKey = 0x0;
    ptRegion->arrDisp[1].uDisp.tOSD.u32BgColorLo = 0xffffffff;
    ptRegion->arrDisp[1].uDisp.tOSD.u32BgColorHi = 0xffffffff;
    ptRegion->arrDisp[1].uDisp.tOSD.u32BmpWidth = 240;
    ptRegion->arrDisp[1].uDisp.tOSD.u32BmpHeight = 227;
    ptRegion->arrDisp[1].uDisp.tOSD.u32DstXoffset = 653;
    ptRegion->arrDisp[1].uDisp.tOSD.u32DstYoffset = 303;
    ptRegion->arrDisp[1].uDisp.tOSD.pBitmap = &bitmap_cursor[0];
}

static AX_VOID IVPS_OsdBufRelease(AX_IVPS_RGN_DISP_GROUP_S *ptRegion)
{
    AX_SYS_MemFree((AX_U64)ptRegion->arrDisp[0].uDisp.tOSD.u64PhyAddr, (AX_VOID *)ptRegion->arrDisp[0].uDisp.tOSD.pBitmap);
    AX_SYS_MemFree((AX_U64)ptRegion->arrDisp[1].uDisp.tOSD.u64PhyAddr, (AX_VOID *)ptRegion->arrDisp[1].uDisp.tOSD.pBitmap);
}

/* Mosaic and Polygon config */
static AX_VOID IVPS_RegionCfg2(AX_IVPS_RGN_DISP_GROUP_S *ptRegion)
{
    /*
    nZindex:
            For Line/Polygon/OSD nZindex is required, It indicates which layer to draw Line/Polygon/OSD on.
            If Filter engine type is TDP, nZindex is 0 ~ 4.
            If Filter engine type is VO, nZindex is 0 ~ 31.
            But for mosaic nZindex is not required.
    nAlpha: Required
            0 - 1024, 0: transparent, 1024: opaque.
*/
    ptRegion->nNum = 5;
    ptRegion->tChnAttr.nZindex = 4;
    ptRegion->tChnAttr.nAlpha = 1024;
    ptRegion->tChnAttr.eFormat = AX_FORMAT_ARGB8888;
    ptRegion->arrDisp[0].bShow = AX_TRUE;
    ptRegion->arrDisp[0].eType = AX_IVPS_RGN_TYPE_MOSAIC;
    ptRegion->arrDisp[0].uDisp.tMosaic.tRect.nX = 1000;
    ptRegion->arrDisp[0].uDisp.tMosaic.tRect.nY = 900;
    ptRegion->arrDisp[0].uDisp.tMosaic.tRect.nW = 200;
    ptRegion->arrDisp[0].uDisp.tMosaic.tRect.nH = 300;
    ptRegion->arrDisp[0].uDisp.tMosaic.eBklSize = AX_IVPS_MOSAIC_BLK_SIZE_32;
    ptRegion->arrDisp[1].bShow = AX_TRUE;
    ptRegion->arrDisp[1].eType = AX_IVPS_RGN_TYPE_MOSAIC;
    ptRegion->arrDisp[1].uDisp.tMosaic.tRect.nX = 800;
    ptRegion->arrDisp[1].uDisp.tMosaic.tRect.nY = 600;
    ptRegion->arrDisp[1].uDisp.tMosaic.tRect.nW = 200;
    ptRegion->arrDisp[1].uDisp.tMosaic.tRect.nH = 150;
    ptRegion->arrDisp[1].uDisp.tMosaic.eBklSize = AX_IVPS_MOSAIC_BLK_SIZE_64;

    ptRegion->arrDisp[2].bShow = AX_TRUE;
    ptRegion->arrDisp[2].eType = AX_IVPS_RGN_TYPE_RECT;
    ptRegion->arrDisp[2].uDisp.tPolygon.tRect.nX = 400;
    ptRegion->arrDisp[2].uDisp.tPolygon.tRect.nY = 600;
    ptRegion->arrDisp[2].uDisp.tPolygon.tRect.nW = 100;
    ptRegion->arrDisp[2].uDisp.tPolygon.tRect.nH = 200;
    ptRegion->arrDisp[2].uDisp.tPolygon.bSolid = AX_FALSE;
    ptRegion->arrDisp[2].uDisp.tPolygon.nLineWidth = 4;
    ptRegion->arrDisp[2].uDisp.tPolygon.nColor = RED;
    ptRegion->arrDisp[2].uDisp.tPolygon.nAlpha = 200;

    ptRegion->arrDisp[3].bShow = AX_TRUE;
    ptRegion->arrDisp[3].eType = AX_IVPS_RGN_TYPE_RECT;
    ptRegion->arrDisp[3].uDisp.tPolygon.tRect.nX = 400;
    ptRegion->arrDisp[3].uDisp.tPolygon.tRect.nY = 800;
    ptRegion->arrDisp[3].uDisp.tPolygon.tRect.nW = 100;
    ptRegion->arrDisp[3].uDisp.tPolygon.tRect.nH = 100;
    ptRegion->arrDisp[3].uDisp.tPolygon.bSolid = AX_FALSE;
    ptRegion->arrDisp[3].uDisp.tPolygon.nLineWidth = 2;
    ptRegion->arrDisp[3].uDisp.tPolygon.nColor = RED;
    ptRegion->arrDisp[3].uDisp.tPolygon.nAlpha = 200;
    ptRegion->arrDisp[4].bShow = AX_TRUE;
    ptRegion->arrDisp[4].eType = AX_IVPS_RGN_TYPE_RECT;
    ptRegion->arrDisp[4].uDisp.tPolygon.tRect.nX = 100;
    ptRegion->arrDisp[4].uDisp.tPolygon.tRect.nY = 800;
    ptRegion->arrDisp[4].uDisp.tPolygon.tRect.nW = 100;
    ptRegion->arrDisp[4].uDisp.tPolygon.tRect.nH = 100;
    ptRegion->arrDisp[4].uDisp.tPolygon.bSolid = AX_TRUE;
    ptRegion->arrDisp[4].uDisp.tPolygon.nColor = PINK;
    ptRegion->arrDisp[4].uDisp.tPolygon.nAlpha = 200;
}

static AX_IVPS_RGN_DISP_GROUP_S tRegionGrp[8];

static AX_VOID RegionConfig(AX_S32 nNum)
{

    IVPS_RegionCfg1(&tRegionGrp[0]);
    IVPS_RegionCfg0(&tRegionGrp[1]);
    IVPS_RegionCfg2(&tRegionGrp[2]);
    IVPS_RegionCfg3(&tRegionGrp[3]);
}

static AX_S32 RegionUpdate(AX_U32 nRgnNum)
{
    AX_U32 ret, index;
    for (index = 0; index < nRgnNum; index++)
    {
        ret = AX_IVPS_RGN_Update(gRegions[index].handle, &tRegionGrp[index]);
        if (IVPS_SUCC != ret)
        {
            ALOGE("AX_IVPS_RGN_Update fail, ret=0x%x\n", ret);
        }
    }

    return ret;
}
static AX_VOID *UpdateCanvasThread(AX_VOID *pArg)
{
    AX_S32 nRegionNum = (AX_S32)pArg;
    if (!nRegionNum)
    {
        return (AX_VOID *)1;
    }

    while (!ThreadLoopStateGet())
    {
        RegionUpdate(nRegionNum);
        sleep(1);
    }

    return (AX_VOID *)0;
}

AX_S32 IVPS_StartRegion(AX_U32 nRegionNum)
{
    for (AX_S32 j = 0; j < sizeof(gRegions) / sizeof(gRegions[0]); j++)
    {
        gRegions[j].handle = AX_IVPS_INVALID_REGION_HANDLE;
    }
    for (AX_S32 i = 0; i < nRegionNum; i++)
    {

        gRegions[i].handle = AX_IVPS_RGN_Create();
        printf("gRegions[%d].handle:%d\n", i, gRegions[i].handle);
        if (AX_IVPS_INVALID_REGION_HANDLE == gRegions[i].handle)
        {
            ALOGE("AX_IVPS_CreateRegion(%d) fail\n", i);
            return -1;
        }
        AX_S32 ret = AX_IVPS_RGN_AttachToFilter(gRegions[i].handle, gRegions[i].IvpsGrp, gRegions[i].IvpsFilter);
        if (IVPS_SUCC != ret)
        {
            ALOGE("AX_IVPS_RGN_DetachFromFilter(handle %d => Grp %d Filter %x) fail, ret=0x%x\n", gRegions[i].handle, gRegions[i].IvpsGrp, gRegions[i].IvpsFilter, ret);
            /* detach and destroy overlay */
            for (AX_S32 j = 0; j < AX_IVPS_MAX_RGN_HANDLE_NUM; j++)
            {
                if (AX_IVPS_INVALID_REGION_HANDLE != gRegions[j].handle)
                {
                    AX_IVPS_RGN_DetachFromFilter(gRegions[j].handle, gRegions[j].IvpsGrp, gRegions[i].IvpsFilter);
                    AX_IVPS_RGN_Destroy(gRegions[j].handle);
                    gRegions[j].handle = AX_IVPS_INVALID_REGION_HANDLE;
                }
            }
            return -1;
        }
    }

    return 0;
}

AX_S32 IVPS_StopRegion(AX_VOID)
{
    ALOGI("IVPS_StopRegion num:%d\n", sizeof(gRegions) / sizeof(gRegions[0]));

    for (AX_U32 j = 0; j < sizeof(gRegions) / sizeof(gRegions[0]); ++j)
    {
        if (AX_IVPS_INVALID_REGION_HANDLE != gRegions[j].handle)
        {

            AX_IVPS_RGN_DetachFromFilter(gRegions[j].handle, gRegions[j].IvpsGrp, gRegions[j].IvpsFilter);
            AX_IVPS_RGN_Destroy(gRegions[j].handle);
            gRegions[j].handle = AX_IVPS_INVALID_REGION_HANDLE;
        }
    }

    return 0;
}

AX_S32 IVPS_ThreadStartRegion(AX_S32 nRegionNum)
{
    pthread_t tid = 0;

    RegionConfig(nRegionNum);
    printf("UPDATE nRegionNum:%d\n", nRegionNum);

    if (0 != pthread_create(&tid, NULL, UpdateCanvasThread, (AX_VOID *)nRegionNum))
    {
        return -1;
    }
    pthread_detach(tid);
    return 0;
}
