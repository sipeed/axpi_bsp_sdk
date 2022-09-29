/**********************************************************************************
 *
 * Copyright (c) 2019-2022 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#pragma once
#include "ax_base_type.h"
#include "OsdConfig.h"
#include "OSDHandler.h"

class COSDHandlerWrapper
{
public:
    COSDHandlerWrapper(AX_VOID);
    virtual ~COSDHandlerWrapper(AX_VOID);

public:
    /* Get osdhandler instance
      @param - [IN] void
      @return - osdhandler instance pointer

      description:
      1. Get an osdhandler instance pointer
    */
    COSDHandler *NewInstance(AX_VOID);
    /* Release osdhandler instance
      @param - [IN]ppInstance: osdhandler instance pointer.
      @return - void

      description:
      1. Release osdhandler instance
    */
    AX_VOID ReleaseInstance(COSDHandler **ppInstance);
    /* Init OSDHanlder
      @param - [IN]pOSDHanlder: osdhandler instance pointer.
      @param - [IN]fontPath: font file path.
      @return - error code

      description:
      1. Init font file path and font file type
    */
    AX_BOOL InitHandler(COSDHandler *pOSDHandler, const AX_CHAR *fontFilePath);
    /* Generate argb data with the input text string
      @param - [IN]pOSDHanlder: osdhandler instance pointer.
      @param - [IN] pTextStr: text needs to be transformated,support freetype font.
      @param - [IN/OUT]     pArgbBuffer: buffer to store osd argb data
      @param - [IN]     u16OSDWidth: OSD width
      @param - [IN]     u16OSDHeight: OSD height
      @param - [IN]sX:x axis value in osd coordinate
      @param - [IN]sY:y axis value in osd coordinate
      @param - [IN]     uFontSize: font size
      @param - [IN]     bIsBrushSide: flag for brushing character side
      @param - [IN]     uFontColor: font color
      @param - [IN]     uBgColor: background color for osd,two bytes should be same and different from other color
      @param - [IN]     uSideColor: color for brushing side
      @return - argb data buffer pointer, nullptr if error

      description:
      1. Generate argb data with input text string, and set the font color and background color.
    */
    AX_VOID *GenARGB(COSDHandler *pOSDHandler, wchar_t *pTextStr, AX_U16 *pArgbBuffer, AX_U32 u32OSDWidth,
                                AX_U32 u32OSDHeight, AX_S16 sX, AX_S16 sY, AX_U16 uFontSize, AX_BOOL bIsBrushSide = AX_FALSE,
                                AX_U32 uFontColor = 0xFFFFFFFF, AX_U32 uBgColor = 0xFF0000, AX_U32 uSideColor = 0,
                                OSD_ALIGN_TYPE_E enAlign = OSD_ALIGN_TYPE_LEFT_TOP);

};
