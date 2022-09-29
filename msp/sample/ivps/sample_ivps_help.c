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
#include "ax_sys_api.h"
#include "sample_ivps_util.h"
#include "sample_ivps_region.h"
#include "sample_ivps_help.h"

AX_VOID RegionHelp(AX_VOID)
{
    printf("\t\t 7: support draw works,such as osd/line/rect/polygon\n");
}

AX_VOID UserModeHelp(AX_VOID)
{
    printf("IVPS user mode:\n");
    printf("Usage: /%s -v <IMG PATH> @ <FMT> @ <srcW> x <srcH> @ <cropW> x <cropH> [ + <srcX0> + <srcY0> ]>\n", SAMPLE_NAME);
    printf("\t\t   e.g: /%s -v /opt/bin/data/ivps/800x480logo.rgb24 @ 65 @ 800 x 480 @ 600 x 400 + 100 + 50 \n\n", SAMPLE_NAME);
    printf("\t\t\t   [-g] <IMG PATH> @ <FMT> @ <srcW> x <srcH> [ + <dstX0> + <dstY0> * <alpha> ]>\n");
    printf("\t\t   e.g: -g /opt/bin/data/ivps/rgb400x240.rgb24 @ 65 @ 400 x 240 + 100 + 50 * 150 \n\n");
    printf("\t\t\t   [-a] <stride> @ <dstW> x <dstH> @ <FMT> # <angle> - <mirror> + <flip> \n");
    printf("\t\t   e.g: -a 3840 @ 3840 x 2160 @ 3 # 3 - 1 + 1\n\n");
    printf("\t\t\t   [-c] <w x h + dst_x0 + dst_y0> @ <blk size> \n");
    printf("\t\t\t   [-n] <repeat num>]\n");
    printf("\t\t\t   [-r] <region num>]\n\n");
    printf("\t-v           : video frame input\n");
    printf("\t-g (optional): overlay input\n");
    printf("\t-a (optional): channel attr\n");
    printf("\t-n (optional): repeat number\n");
    printf("\t-r (optional): region config for each channel\n");

    printf("\t<IMG PATH> : image path\n");
    printf("\t<FMT>      : color format\n");
    printf("\t\t%d: NV12\n", AX_YUV420_SEMIPLANAR);
    printf("\t\t%d: NV21\n", AX_YUV420_SEMIPLANAR_VU);
    printf("\t\t%d: RGB888\n", AX_FORMAT_RGB888);
    printf("\t\t%d: RGB565\n", AX_FORMAT_RGB565);
    printf("\t\t%d: ARGB4444\n", AX_FORMAT_ARGB4444);
    printf("\t\t%d: RGBA4444\n", AX_FORMAT_RGBA4444);
    printf("\t\t%d: ARGB8888\n", AX_FORMAT_ARGB8888);
    printf("\t\t%d: RGBA8888\n", AX_FORMAT_RGBA8888);
    printf("\t\t%d: ARGB1555\n", AX_FORMAT_ARGB1555);
    printf("\t\t%d: RGBA5551\n", AX_FORMAT_RGBA5551);
    printf("\t\t%d: ARGB8565\n", AX_FORMAT_ARGB8565);
    printf("\t\t%d: BITMAP\n", AX_FORMAT_BITMAP);

    printf("\t<stride>                      : channel stride (NO limit)\n");
    printf("\t<srcW> x <srcH>               : input image width and height (NO limit)\n");
    printf("\t<dstW> x <dstH>               : channel width and height (dstW <= 4096 && dstH <= 4096)\n");
    printf("\t<cropW> x <cropH>             : crop rect width&height(cropW <= 4096 && cropH <= 4096)\n");
    printf("\t+ <srcX0> + <srcY0> (optional): crop rect coordinates\n");
    printf("\t+ <dstX0> + <dstY0> (optional): output position coordinates\n");
    printf("\t<alpha>             (optional): (0: transparent; 1024: opaque)\n");
    printf("\t<angle>             (optional): rotation angle: (0:no rotate; 1: 90 degree; 2:180 degree; 3:270 degree)\n");
    printf("\t<mirror>            (optional): (0:no mirror; 1: mirror)\n");
    printf("\t<flip>              (optional): (0:no flip; 1: flip)\n\n");
    printf("Example1(Crop):\n");
    printf("\t%s -v /opt/data/ivps/800x480logo.rgb24@65@800x480@800x480+100+50#3  -n 3 -r 3\n", SAMPLE_NAME);
    printf("Example2(No Crop):\n");
    printf("\t%s -v /opt/data/ivps/800x480logo.rgb24@65@800x480@800x480#3 -n 3 -r 3\n\n", SAMPLE_NAME);

    printf("\t The output image will be generated in the same level directory of the input image\n");
    printf("\t\t 1: support color space covert, please check the image starting with CSC\n");
    printf("\t\t 2: support rotation, please check the image starting with Rotate\n");
    printf("\t\t 3: support flip and mirror, please check the image starting with FlipMirrorRotate\n");
    printf("\t\t 4: support alpha blending, please check the image starting with AlphaBlend\n");
    printf("\t\t 5: support crop and resize, please check the image starting with CropResize\n");
    printf("\t\t 6: support OSD, please check the image starting with OSD\n");
}

AX_VOID DrawWorksHelp(AX_VOID)
{
    printf("Draw works(line, rect, polygon, osd):\n");
    printf("Usage: %sn", SAMPLE_NAME);
}

AX_VOID IVPS_ObjHelp(AX_VOID)
{
    printf("USER --> IVPS --> DPU\n");
    printf("Usage: /%s -p<sns type>@<run mode>@<hdr mode>@<fps>@<draw>\n", SAMPLE_NAME);
    printf("\tISP sensor type:\n");
    printf("\t\t0: SMARTSENSE_SC4210\n");
    printf("\t\t1: SONY_IMX485\n");
    printf("\t\t2: OMNIVISION_OS04A10 (default)\n");
    printf("\tISP mode:\n");
    printf("\t\t1: preisp->postisp (default)\n");
    printf("\t\t2: preisp->npu->postisp\n");
    printf("\tISP hdr mode\n");
    printf("\t\t1: linear mode (default)\n");
    printf("\t\t2: 2dol\n");
    printf("\t\t3: 3dol\n");
    printf("\tfps:  fps control, 0 means disable fps control (default 0)\n");
    printf("\tdraw: 1: draw line, rect, polygon, mosaic  0: no draw  (default: 1)\n");
    printf("Example: %s -p2@1@1@0\n", SAMPLE_NAME);
}

typedef AX_VOID (*pHelperFunc)(AX_VOID);

static struct
{
    pHelperFunc pHelper;
} gHelps[SAMPLE_HELP_MAX] = {
    {UserModeHelp},
    {RegionHelp},
};

AX_VOID ShowUsage(AX_S32 nHelpIdx)
{
    if (nHelpIdx > SAMPLE_HELP_MIN && nHelpIdx < SAMPLE_HELP_MAX)
    {
        pHelperFunc pHelper = gHelps[nHelpIdx].pHelper;
        if (pHelper)
        {
            (*pHelper)();
            return;
        }
    }
    printf("\tuser mode userguild help: /%s -h %d\n", SAMPLE_NAME, SAMPLE_HELP_USERMODE);
    printf("\tlink mode userguild help: /%s -h %d\n", SAMPLE_NAME, SAMPLE_HELP_LINKMODE);
    printf("\tregion    userguild help: /%s -h %d\n", SAMPLE_NAME, SAMPLE_HELP_REGION);
}
