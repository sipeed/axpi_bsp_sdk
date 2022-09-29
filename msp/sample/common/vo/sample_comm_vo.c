/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "sample_comm_vo.h"

AX_S32 SAMPLE_COMM_VO_StartDev(VO_DEV VoDev, VO_PUB_ATTR_S *pstPubAttr)
{
    AX_S32 s32Ret = 0;

    s32Ret = AX_VO_SetPubAttr(VoDev, pstPubAttr);
    if (s32Ret) {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = AX_VO_Enable(VoDev);
    if (s32Ret) {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    return s32Ret;
}

AX_S32 SAMPLE_COMM_VO_StopDev(VO_DEV VoDev)
{
    AX_S32 s32Ret = 0;

    s32Ret = AX_VO_Disable(VoDev);
    if (s32Ret) {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    return s32Ret;
}

AX_S32 SAMPLE_COMM_VO_StartLayer(VO_LAYER VoLayer, const VO_VIDEO_LAYER_ATTR_S *pstLayerAttr)
{
    AX_S32 s32Ret = 0;

    s32Ret = AX_VO_SetVideoLayerAttr(VoLayer, pstLayerAttr);
    if (s32Ret) {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = AX_VO_EnableVideoLayer(VoLayer);
    if (s32Ret) {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    return s32Ret;
}

AX_S32 SAMPLE_COMM_VO_StopLayer(VO_LAYER VoLayer)
{
    AX_S32 s32Ret = 0;

    s32Ret = AX_VO_DisableVideoLayer(VoLayer);
    if (s32Ret) {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    return s32Ret;
}

AX_S32 SAMPLE_COMM_VO_StartChn(VO_LAYER VoLayer, SAMPLE_VO_MODE_E enMode, AX_U32 u32FifoDepth)
{
    AX_S32 i;
    AX_S32 s32Ret    = 0;
    AX_U32 u32WndNum = 0;
    AX_U32 u32Square = 0;
    AX_U32 u32Row    = 0;
    AX_U32 u32Col    = 0;
    AX_U32 u32Width  = 0;
    AX_U32 u32Height = 0;
    VO_CHN_ATTR_S         stChnAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;

    switch (enMode) {
    case VO_MODE_1MUX:
        u32WndNum = 1;
        u32Square = 1;
        break;
    case VO_MODE_2MUX:
        u32WndNum = 2;
        u32Square = 2;
        break;
    case VO_MODE_4MUX:
        u32WndNum = 4;
        u32Square = 2;
        break;
    case VO_MODE_8MUX:
        u32WndNum = 8;
        u32Square = 3;
        break;
    case VO_MODE_9MUX:
        u32WndNum = 9;
        u32Square = 3;
        break;
    case VO_MODE_16MUX:
        u32WndNum = 16;
        u32Square = 4;
        break;
    case VO_MODE_25MUX:
        u32WndNum = 25;
        u32Square = 5;
        break;
    case VO_MODE_36MUX:
        u32WndNum = 36;
        u32Square = 6;
        break;
    case VO_MODE_49MUX:
        u32WndNum = 49;
        u32Square = 7;
        break;
    case VO_MODE_64MUX:
        u32WndNum = 64;
        u32Square = 8;
        break;
    case VO_MODE_2X4:
        u32WndNum = 8;
        u32Square = 3;
        u32Row    = 4;
        u32Col    = 2;
        break;
    default:
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return -1;
    }

    s32Ret = AX_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
    if (s32Ret) {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return s32Ret;
    }
    u32Width  = stLayerAttr.stImageSize.u32Width;
    u32Height = stLayerAttr.stImageSize.u32Height;
    SAMPLE_PRT("layer%d u32Width:%d, u32Height:%d, u32Square:%d\n", VoLayer, u32Width, u32Height, u32Square);
    for (i = 0; i < u32WndNum; i++) {
        if (enMode == VO_MODE_1MUX  ||
            enMode == VO_MODE_2MUX  ||
            enMode == VO_MODE_4MUX  ||
            enMode == VO_MODE_8MUX  ||
            enMode == VO_MODE_9MUX  ||
            enMode == VO_MODE_16MUX ||
            enMode == VO_MODE_25MUX ||
            enMode == VO_MODE_36MUX ||
            enMode == VO_MODE_49MUX ||
            enMode == VO_MODE_64MUX) {

            stChnAttr.stRect.u32X       = ALIGN_DOWN((u32Width / u32Square) * (i % u32Square), 16);
            stChnAttr.stRect.u32Y       = ALIGN_DOWN((u32Height / u32Square) * (i / u32Square), 16);
            stChnAttr.stRect.u32Width   = ALIGN_DOWN(u32Width / u32Square, 16);
            stChnAttr.stRect.u32Height  = ALIGN_DOWN(u32Height / u32Square, 2);

        } else if (enMode == VO_MODE_2X4) {
            stChnAttr.stRect.u32X       = ALIGN_DOWN((u32Width / u32Col) * (i % u32Col), 16);
            stChnAttr.stRect.u32Y       = ALIGN_DOWN((u32Height / u32Row) * (i / u32Col), 16);
            stChnAttr.stRect.u32Width   = ALIGN_DOWN(u32Width / u32Col, 16);
            stChnAttr.stRect.u32Height  = ALIGN_DOWN(u32Height / u32Row, 2);
        }

        stChnAttr.u32FifoDepth = u32FifoDepth;

        s32Ret = AX_VO_SetChnAttr(VoLayer, i, &stChnAttr);
        if (s32Ret) {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return s32Ret;
        }

        s32Ret = AX_VO_EnableChn(VoLayer, i);
        if (s32Ret) {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return s32Ret;
        }
    }

    return 0;
}

AX_S32 SAMPLE_COMM_VO_StopChn(VO_LAYER VoLayer, SAMPLE_VO_MODE_E enMode)
{
    AX_S32 i;
    AX_S32 s32Ret    = 0;
    AX_U32 u32WndNum = 0;

    switch (enMode) {
    case VO_MODE_1MUX:
        u32WndNum = 1;
        break;
    case VO_MODE_2MUX:
        u32WndNum = 2;
        break;
    case VO_MODE_4MUX:
        u32WndNum = 4;
        break;
    case VO_MODE_8MUX:
        u32WndNum = 8;
        break;
    case VO_MODE_9MUX:
        u32WndNum = 9;
        break;
    case VO_MODE_16MUX:
        u32WndNum = 16;
        break;
    case VO_MODE_25MUX:
        u32WndNum = 25;
        break;
    case VO_MODE_36MUX:
        u32WndNum = 36;
        break;
    case VO_MODE_49MUX:
        u32WndNum = 49;
        break;
    case VO_MODE_64MUX:
        u32WndNum = 64;
        break;
    case VO_MODE_2X4:
        u32WndNum = 8;
        break;
    default:
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return -1;
    }

    for (i = 0; i < u32WndNum; i++) {
        s32Ret = AX_VO_DisableChn(VoLayer, i);
        if (s32Ret) {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return s32Ret;
        }
    }

    return s32Ret;
}

AX_S32 SAMPLE_COMM_VO_StartVO(SAMPLE_VO_CONFIG_S *pstVoConfig)
{
    VO_RECT_S                 stDefDispRect  = {0, 0, 1920, 1080};
    VO_SIZE_S                 stDefImageSize = {1920, 1080};

    /*******************************************
    * VO device VoDev# information declaration.
    ********************************************/
    VO_DEV                 VoDev          = 0;
    VO_LAYER               VoLayer        = 0;
    SAMPLE_VO_MODE_E       enVoMode       = 0;
    VO_INTF_TYPE_E         enVoIntfType   = VO_INTF_DSI0;
    VO_INTF_SYNC_E         enIntfSync     = VO_OUTPUT_1080P30;
    VO_PUB_ATTR_S          stVoPubAttr    = {0};
    VO_VIDEO_LAYER_ATTR_S  *pstLayerAttr  = NULL;
    SAMPLE_VO_LAYER_CONFIG_S *pstVoLayerConf = NULL;
    AX_S32                  sEnableGLayer  = 0;
    GRAPHIC_LAYER          GraphicLayer   = 0;
    AX_S32                 s32Ret         = 0;
    AX_U32                 u32FifoDepth   = 0;
    AX_S32                 i;

    if (!pstVoConfig) {
        SAMPLE_PRT("Error:argument can not be NULL\n");
        return -1;
    }

    VoDev          = pstVoConfig->VoDev;
    enVoIntfType   = pstVoConfig->enVoIntfType;
    enIntfSync     = pstVoConfig->enIntfSync;
    sEnableGLayer  = pstVoConfig->s32EnableGLayer;
    GraphicLayer   = pstVoConfig->GraphicLayer;
    u32FifoDepth   = pstVoConfig->u32FifoDepth;

    /********************************
    * Set and start VO device VoDev#.
    *********************************/
    stVoPubAttr.enIntfType  = enVoIntfType;
    stVoPubAttr.enIntfSync  = enIntfSync;
    stVoPubAttr.stReso      = pstVoConfig->stReso;

    //stVoPubAttr.u32BgColor  = pstVoConfig->u32BgColor;

    s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);
    if (s32Ret) {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartDev failed!\n");
        return s32Ret;
    }

    for (i = 0; i < pstVoConfig->u32LayerNr; i++) {
        pstVoLayerConf = &pstVoConfig->stVoLayer[i];
        VoLayer        = pstVoLayerConf->VoLayer;
        pstLayerAttr   = &pstVoLayerConf->stVoLayerAttr;
        enVoMode       = pstVoLayerConf->enVoMode;

        /******************************
        * Set and start layer VoDev#.
        ********************************/
        pstLayerAttr->u32FifoDepth = pstVoConfig->u32FifoDepth;
        s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, pstLayerAttr);
        if (s32Ret) {
            SAMPLE_PRT("SAMPLE_COMM_VO_Start failed, i = %d\n", i);
            goto exit;
        }

        /******************************
        * start vo channels.
        ********************************/
        s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode, u32FifoDepth);
        if (s32Ret) {
            SAMPLE_PRT("SAMPLE_COMM_VO_StartChn failed, i = %d\n", i);
            SAMPLE_COMM_VO_StopLayer(VoLayer);
            goto exit;
        }

        s32Ret = AX_VO_BindVideoLayer(VoLayer, VoDev);
        if (s32Ret) {
            SAMPLE_PRT("AX_VO_BindVideoLayer failed, i = %d\n", i);
            SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);
            SAMPLE_COMM_VO_StopLayer(VoLayer);
            goto exit;
        }
    }

    if (sEnableGLayer) {
        s32Ret = AX_VO_BindGraphicLayer(GraphicLayer, VoDev);
        if (s32Ret)
            SAMPLE_PRT("AX_VO_BindGraphicLayer failed, s32Ret = 0x%x\n", s32Ret);
    }

exit:
    if (s32Ret) {
        for (i -= 1; i >= 0; i--) {
            pstVoLayerConf = &pstVoConfig->stVoLayer[i];
            VoLayer   = pstVoLayerConf->VoLayer;
            enVoMode  = pstVoLayerConf->enVoMode;
            SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);
            SAMPLE_COMM_VO_StopLayer(VoLayer);
            AX_VO_UnBindVideoLayer(VoLayer, VoDev);
        }

        SAMPLE_COMM_VO_StopDev(VoDev);
    }

    SAMPLE_PRT("done, s32Ret = 0x%x\n", s32Ret);

    return 0;
}

AX_S32 SAMPLE_COMM_VO_StopVO(SAMPLE_VO_CONFIG_S *pstVoConfig)
{
    AX_S32                i  = 0;
    VO_DEV                VoDev     = 0;
    VO_LAYER              VoLayer   = 0;
    SAMPLE_VO_MODE_E      enVoMode  = VO_MODE_BUTT;
    AX_S32                 sEnableGLayer  = 0;
    GRAPHIC_LAYER         GraphicLayer   = 0;
    SAMPLE_VO_LAYER_CONFIG_S *pstVoLayerConf = NULL;

    if (!pstVoConfig) {
        SAMPLE_PRT("Error:argument can not be NULL\n");
        return -1;
    }

    VoDev     = pstVoConfig->VoDev;
    sEnableGLayer  = pstVoConfig->s32EnableGLayer;
    GraphicLayer   = pstVoConfig->GraphicLayer;

    for (i = 0; i < pstVoConfig->u32LayerNr; i++) {
        pstVoLayerConf = &pstVoConfig->stVoLayer[i];
        VoLayer   = pstVoLayerConf->VoLayer;
        enVoMode  = pstVoLayerConf->enVoMode;
        SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);
        SAMPLE_COMM_VO_StopLayer(VoLayer);
        AX_VO_UnBindVideoLayer(VoLayer, VoDev);
    }

    if (sEnableGLayer)
        AX_VO_UnBindGraphicLayer(GraphicLayer, VoDev);

    SAMPLE_COMM_VO_StopDev(VoDev);

    return 0;
}

