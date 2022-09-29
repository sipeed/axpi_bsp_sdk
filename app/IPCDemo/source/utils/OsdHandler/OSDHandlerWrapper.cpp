/**********************************************************************************
 *
 * Copyright (c) 2019-2022 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include "OSDHandlerWrapper.h"
#include "OSDHandler.h"


COSDHandlerWrapper::COSDHandlerWrapper(AX_VOID)
{

}

COSDHandlerWrapper::~COSDHandlerWrapper(AX_VOID)
{

}

COSDHandler* COSDHandlerWrapper::NewInstance(AX_VOID) {
    return new COSDHandler();
}

AX_VOID COSDHandlerWrapper::ReleaseInstance(COSDHandler **ppInstance) {
    delete *ppInstance;
    *ppInstance = 0;
}

AX_BOOL COSDHandlerWrapper::InitHandler(COSDHandler *pOSDHandler, const AX_CHAR *fontFilePath) {
    return pOSDHandler->InitOSDHandler(fontFilePath);
}

AX_VOID* COSDHandlerWrapper::GenARGB(COSDHandler *pOSDHandler, wchar_t *pTextStr, AX_U16 *pArgbBuffer, AX_U32 u32OSDWidth, AX_U32 u32OSDHeight,
                       AX_S16 sX, AX_S16 sY, AX_U16 uFontSize, AX_BOOL bIsBrushSide, AX_U32 uFontColor, AX_U32 uBgColor, AX_U32 uSideColor,
                       OSD_ALIGN_TYPE_E enAlign) {
    return pOSDHandler->GenARGB(pTextStr, pArgbBuffer, u32OSDWidth, u32OSDHeight, sX, sY, uFontSize, bIsBrushSide, uFontColor,
                                           uBgColor, uSideColor, enAlign);
}
