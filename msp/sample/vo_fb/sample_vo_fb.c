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
#include <signal.h>
#include <linux/fb.h>

#include "ax_sys_api.h"
#include "ax_base_type.h"
#include "ax_vo_api.h"

#include "sample_comm_vo.h"
#include "sample_vo_pattern.h"

#define IMAGE_WIDTH 480
#define IMAGE_HEIGHT 854

typedef struct axSAMPLE_VO_CHN_THREAD_PARAM {
    pthread_t ThreadID;

    AX_U32 u32ThreadForceStop;

    AX_U32 u32LayerID;
    AX_U32 u32ChnID;
    AX_POOL u32UserPoolId;
} SAMPLE_VO_CHN_THREAD_PARAM_S;

#define SAMPLE_VO_DEV0 0

SAMPLE_VO_CONFIG_S stVoConf = {
    /* for device */
    .VoDev = SAMPLE_VO_DEV0,
    .enVoIntfType = VO_INTF_DSI0,
    .enIntfSync = VO_OUTPUT_USER,
    .stReso = {
        .u32Width = 480,
        .u32Height = 854,
        .u32RefreshRate = 60,
    },
    .u32LayerNr = 1,
    .stVoLayer = {
        {
            // layer0
            .VoLayer = 0,
            .stVoLayerAttr = {
                .stDispRect = {0, 0, 480, 854},
                .stImageSize = {480, 854},
                .enPixFmt = AX_YUV420_SEMIPLANAR,
                .enLayerSync = VO_LAYER_SYNC_NORMAL,
                .u32PrimaryChnId = 0,
                .u32FrameRate = 0,
                .u32FifoDepth = 0,
                .u32ChnNr = 16,
                .u32BkClr = 0,
                .enLayerBuf = VO_LAYER_OUT_BUF_POOL,
                .u32InplaceChnId = 0,
                .u32PoolId = 0,
                .enDispatchMode = VO_LAYER_OUT_TO_FIFO,
                .u64KeepChnPrevFrameBitmap = ~0x0,
            },
            .enVoMode = VO_MODE_1MUX,
        },
        {
            // layer1
            .VoLayer = 1,
            .stVoLayerAttr = {
                .stDispRect = {0, 0, 480, 854},
                .stImageSize = {480, 854},
                .enPixFmt = AX_YUV420_SEMIPLANAR,
                .enLayerSync = VO_LAYER_SYNC_NORMAL,
                .u32PrimaryChnId = 0,
                .u32FrameRate = 0,
                .u32FifoDepth = 0,
                .u32ChnNr = 16,
                .u32BkClr = 0,
                .enLayerBuf = VO_LAYER_OUT_BUF_POOL,
                .u32InplaceChnId = 0,
                .u32PoolId = 0,
                .enDispatchMode = VO_LAYER_OUT_TO_FIFO,
                .u64KeepChnPrevFrameBitmap = ~0x0,
            },
            .enVoMode = VO_MODE_1MUX,
        },
    },

	/* for graphic layer */
    .s32EnableGLayer = 1,
    .GraphicLayer = 0,

    /* for common pool */
    .u32CommonPoolCnt = 2,
    .stPoolFloorPlan = {
        .CommPool[0] = {512, 854 * 480 * 3 / 2, 5, POOL_CACHE_MODE_NONCACHE},
        .CommPool[1] = {512, 480 * 854 * 3 / 2, 5, POOL_CACHE_MODE_NONCACHE},
    },
};

static struct fb_bitfield g_r32 = {24, 8, 0};
static struct fb_bitfield g_g32 = {16, 8, 0};
static struct fb_bitfield g_b32 = {8, 8, 0};
static struct fb_bitfield g_a32 = {0, 8, 0};
static unsigned char *pShowScreen;

static int SAMPLE_VO_POOL_DESTROY(AX_U32 u32PoolID)
{
    return AX_POOL_MarkDestroyPool(u32PoolID);
}

static AX_S32 SAMPLE_VO_CREATE_POOL(AX_U32 u32BlkCnt, AX_U64 u64BlkSize, AX_U64 u64MetaSize, AX_U32 *pPoolID)
{
    AX_POOL_CONFIG_T stPoolCfg = {0};

    stPoolCfg.MetaSize = u64MetaSize;
    stPoolCfg.BlkCnt = u32BlkCnt;
    stPoolCfg.BlkSize = u64BlkSize;
    stPoolCfg.CacheMode = POOL_CACHE_MODE_NONCACHE;
    strcpy((char *)stPoolCfg.PartitionName, "anonymous");

    *pPoolID = AX_POOL_CreatePool(&stPoolCfg);
    if (*pPoolID == AX_INVALID_POOLID) {
        SAMPLE_PRT("AX_POOL_CreatePool failed, u32BlkCnt = %d, u64BlkSize = 0x%llx, u64MetaSize = 0x%llx\n", u32BlkCnt,
                   u64BlkSize, u64MetaSize);
        return -1;
    }

    SAMPLE_PRT("u32BlkCnt = %d, u64BlkSize = 0x%llx, pPoolID = %d\n", u32BlkCnt, u64BlkSize, *pPoolID);

    return 0;
}

static AX_S32 framebuffer()
{
    system("echo 0 > /sys/class/graphics/fbcon/cursor_blink");
	int i, fd;
    AX_S32 s32Ret = 0;
    struct fb_fix_screeninfo fix;
    struct fb_var_screeninfo var;
	// printf("richard %s:%d\n", __func__, __LINE__);
	/*2. open Framebuffer device overlay 0*/
	fd = open("/dev/fb0", O_RDWR);
	if(fd < 0) {
		printf("open fb0 failed!\n");
        goto exit3;
	}
	// printf("richard %s:%d\n", __func__, __LINE__);
	/*3. get the variable screen info*/
	s32Ret = ioctl(fd, FBIOGET_VSCREENINFO, &var);
	if (s32Ret < 0) {
		printf("get variable screen info failed!\n");
        goto exit4;
	}
	// printf("richard %s:%d\n", __func__, __LINE__);
	/*4. modify the variable screen info
	the screen size: IMAGE_WIDTH*IMAGE_HEIGHT
	the virtual screen size: IMAGE_WIDTH*(IMAGE_HEIGHT*2)
	the pixel format: ARGB8888
	*/
	var.xres = var.xres_virtual = IMAGE_WIDTH;
	var.yres = IMAGE_HEIGHT;
	var.yres_virtual = IMAGE_HEIGHT;
	var.transp= g_a32;
	var.red = g_r32;
	var.green = g_g32;
	var.blue = g_b32;
	var.bits_per_pixel = 32;

	/*5. set the variable screeninfo*/
	s32Ret = ioctl(fd, FBIOPUT_VSCREENINFO, &var);
	if (s32Ret < 0) {
		printf("put variable screen info failed!\n");
        goto exit4;
	}
	// printf("richard %s:%d\n", __func__, __LINE__);
	/*6. get the fix screen info*/
	s32Ret = ioctl(fd, FBIOGET_FSCREENINFO, &fix);
	if (s32Ret < 0) {
		printf("get fix screen info failed!\n");
        goto exit4;
	}
	// printf("richard %s:%d\n", __func__, __LINE__);
	/*7. map the physical video memory for user use*/
	pShowScreen = mmap(NULL, fix.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (pShowScreen == (char *)-1) {
		printf("smem map failed!\n");
        goto exit4;
    }
	// printf("richard %s:%d\n", __func__, __LINE__);
	SAMPLE_Fill_Color(AX_FORMAT_ARGB8888, IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_WIDTH * 4, pShowScreen);

    // for (int i = 0, s = 4 * IMAGE_HEIGHT * IMAGE_WIDTH; i < s; i += 4) pShowScreen[i + 3] = 0xff;

    // for (int x = 0; x < IMAGE_WIDTH; x++)
    // {
    //     for (int y = 0; y < IMAGE_HEIGHT; y++)
    //     {
    //         int offset = (y * IMAGE_WIDTH + x) * 4;
    //         pShowScreen[offset] = 255; // B G R A
    //         pShowScreen[offset + 1] = 255;
    //         pShowScreen[offset + 2] = 255;
    //         pShowScreen[offset + 3] = 255;
    //     }
    // }

/*
	for (i = 0; i < 100; i++) {
		if(i % 2)
			var.yoffset = 0;
		else
			var.yoffset = IMAGE_HEIGHT;

		s32Ret = ioctl(fd, FBIOPAN_DISPLAY, &var);
		if (s32Ret < 0) {
			printf("pan display failed, %s\n", strerror(errno));
        	goto exit5;
		}

		usleep(16666);
	}
*/
	// printf("richard %s:%d\n", __func__, __LINE__);
	return;
exit5:
    munmap(pShowScreen, fix.smem_len);
exit4:
    close(fd);
// exit43:
//     SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);
//     SAMPLE_COMM_VO_StopLayer(VoLayer);
//     AX_VO_UnBindVideoLayer(VoLayer, VoDev);
exit3:
    AX_VO_UnBindGraphicLayer(0, 0);
exit2:
    SAMPLE_COMM_VO_StopDev(0);
    // SAMPLE_COMM_VO_StopVO(pstVoConf);
exit1:
    AX_VO_Deinit();
exit0:
    printf("quit!\n");
    return s32Ret;
}

typedef struct COLOR_YUV {
    AX_U8 u8Y;
    AX_U8 u8U;
    AX_U8 u8V;
} COLOR_YUV_S;

#define MAKE_YUV_601_Y(u8R, u8G, u8B) \
    ((( 66 * (u8R) + 129 * (u8G) +  25 * (u8B) + 128) >> 8) + 16)
#define MAKE_YUV_601_U(u8R, u8G, u8B) \
    (((-38 * (u8R) -  74 * (u8G) + 112 * (u8B) + 128) >> 8) + 128)
#define MAKE_YUV_601_V(u8R, u8G, u8B) \
    (((112 * (u8R) -  94 * (u8G) -  18 * (u8B) + 128) >> 8) + 128)

#define MAKE_YUV_601(u8R, u8G, u8B) \
    { .u8Y = MAKE_YUV_601_Y(u8R, u8G, u8B), \
      .u8U = MAKE_YUV_601_U(u8R, u8G, u8B), \
      .u8V = MAKE_YUV_601_V(u8R, u8G, u8B) }

static AX_VOID Fill_Smpte_YUV(AX_U32 u32Width, AX_U32 u32Height, AX_U32 u32Stride,
                              AX_U8 *u8YMem, AX_U8 *u8UMem, AX_U8 *u8VMem)
{
    const COLOR_YUV_S stCororsTop[] = {
        MAKE_YUV_601(0, 0, 0),    /* zero */
        MAKE_YUV_601(0, 0, 0),    /* zero */
        MAKE_YUV_601(0, 0, 0),    /* zero */
        MAKE_YUV_601(0, 0, 0),    /* zero */
        MAKE_YUV_601(0, 0, 0),    /* zero */
        MAKE_YUV_601(0, 0, 0),    /* zero */
        MAKE_YUV_601(0, 0, 0),    /* zero */
    };
    const COLOR_YUV_S stCororsMiddle[] = {
        MAKE_YUV_601(0, 0, 0),    /* zero */
        MAKE_YUV_601(0, 0, 0),    /* zero */
        MAKE_YUV_601(0, 0, 0),    /* zero */
        MAKE_YUV_601(0, 0, 0),    /* zero */
        MAKE_YUV_601(0, 0, 0),    /* zero */
        MAKE_YUV_601(0, 0, 0),    /* zero */
        MAKE_YUV_601(0, 0, 0),    /* zero */
    };
    const COLOR_YUV_S stCororsBottom[] = {
        MAKE_YUV_601(0, 0, 0),    /* zero */
        MAKE_YUV_601(0, 0, 0),    /* zero */
        MAKE_YUV_601(0, 0, 0),    /* zero */
        MAKE_YUV_601(0, 0, 0),    /* zero */
        MAKE_YUV_601(0, 0, 0),    /* zero */
        MAKE_YUV_601(0, 0, 0),    /* zero */
        MAKE_YUV_601(0, 0, 0),    /* zero */
        MAKE_YUV_601(0, 0, 0),    /* zero */
    };
    AX_U32 u32CS = 2;
    AX_U32 u32XSub = 2;
    AX_U32 u32YSub = 2;
    AX_U32 u32X;
    AX_U32 u32Y;

    /* Luma */
    for (u32Y = 0; u32Y < u32Height * 6 / 9; ++u32Y) {
        for (u32X = 0; u32X < u32Width; ++u32X)
            u8YMem[u32X] = stCororsTop[u32X * 7 / u32Width].u8Y;
        u8YMem += u32Stride;
    }

    for (; u32Y < u32Height * 7 / 9; ++u32Y) {
        for (u32X = 0; u32X < u32Width; ++u32X)
            u8YMem[u32X] = stCororsMiddle[u32X * 7 / u32Width].u8Y;
        u8YMem += u32Stride;
    }

    for (; u32Y < u32Height; ++u32Y) {
        for (u32X = 0; u32X < u32Width * 5 / 7; ++u32X)
            u8YMem[u32X] = stCororsBottom[u32X * 4 / (u32Width * 5 / 7)].u8Y;
        for (; u32X < u32Width * 6 / 7; ++u32X)
            u8YMem[u32X] = stCororsBottom[(u32X - u32Width * 5 / 7) * 3
                                          / (u32Width / 7) + 4].u8Y;
        for (; u32X < u32Width; ++u32X)
            u8YMem[u32X] = stCororsBottom[7].u8Y;
        u8YMem += u32Stride;
    }

    /* Chroma */
    for (u32Y = 0; u32Y < u32Height / u32YSub * 6 / 9; ++u32Y) {
        for (u32X = 0; u32X < u32Width; u32X += u32XSub) {
            u8UMem[u32X * u32CS / u32XSub] = stCororsTop[u32X * 7 / u32Width].u8U;
            u8VMem[u32X * u32CS / u32XSub] = stCororsTop[u32X * 7 / u32Width].u8V;
        }
        u8UMem += u32Stride * u32CS / u32XSub;
        u8VMem += u32Stride * u32CS / u32XSub;
    }

    for (; u32Y < u32Height / u32YSub * 7 / 9; ++u32Y) {
        for (u32X = 0; u32X < u32Width; u32X += u32XSub) {
            u8UMem[u32X * u32CS / u32XSub] = stCororsMiddle[u32X * 7 / u32Width].u8U;
            u8VMem[u32X * u32CS / u32XSub] = stCororsMiddle[u32X * 7 / u32Width].u8V;
        }
        u8UMem += u32Stride * u32CS / u32XSub;
        u8VMem += u32Stride * u32CS / u32XSub;
    }

    for (; u32Y < u32Height / u32YSub; ++u32Y) {
        for (u32X = 0; u32X < u32Width * 5 / 7; u32X += u32XSub) {
            u8UMem[u32X * u32CS / u32XSub] =
                stCororsBottom[u32X * 4 / (u32Width * 5 / 7)].u8U;
            u8VMem[u32X * u32CS / u32XSub] =
                stCororsBottom[u32X * 4 / (u32Width * 5 / 7)].u8V;
        }
        for (; u32X < u32Width * 6 / 7; u32X += u32XSub) {
            u8UMem[u32X * u32CS / u32XSub] = stCororsBottom[(u32X - u32Width * 5 / 7) *
                                             3 / (u32Width / 7) + 4].u8U;
            u8VMem[u32X * u32CS / u32XSub] = stCororsBottom[(u32X - u32Width * 5 / 7) *
                                             3 / (u32Width / 7) + 4].u8V;
        }
        for (; u32X < u32Width; u32X += u32XSub) {
            u8UMem[u32X * u32CS / u32XSub] = stCororsBottom[7].u8U;
            u8VMem[u32X * u32CS / u32XSub] = stCororsBottom[7].u8V;
        }
        u8UMem += u32Stride * u32CS / u32XSub;
        u8VMem += u32Stride * u32CS / u32XSub;
    }
}

static AX_VOID *SAMPLE_VO_CHN_THREAD(AX_VOID *pData)
{
    AX_S32 s32Ret = 0;
    AX_VIDEO_FRAME_S stFrame = {0};
    AX_U32 u32FrameSize;
    AX_U32 u32LayerID, u32ChnID;
    AX_BLK BlkId;
    VO_CHN_ATTR_S stChnAttr;
    AX_U64 u64VirAddr, u64PhysAddr;
    SAMPLE_VO_CHN_THREAD_PARAM_S *pstChnThreadParam = (SAMPLE_VO_CHN_THREAD_PARAM_S *)pData;

    u32LayerID = pstChnThreadParam->u32LayerID;
    u32ChnID = pstChnThreadParam->u32ChnID;

    s32Ret = AX_VO_GetChnAttr(u32LayerID, u32ChnID, &stChnAttr);
    if (s32Ret) {
        SAMPLE_PRT("layer%d-chn%d AX_VO_GetChnAttr failed, s32Ret = 0x%x\n", u32LayerID, u32ChnID, s32Ret);
        goto exit;
    }
    SAMPLE_PRT("layer%d-chn%d u32Width = %d, u32Height = %d\n",
               u32LayerID, u32ChnID,
               stChnAttr.stRect.u32Width, stChnAttr.stRect.u32Height);

    stFrame.enImgFormat = AX_YUV420_SEMIPLANAR;
    stFrame.u32Width = stChnAttr.stRect.u32Width;
    stFrame.u32Height = stChnAttr.stRect.u32Height;
    stFrame.u32PicStride[0] = stChnAttr.stRect.u32Width;
    u32FrameSize = stFrame.u32PicStride[0] * stFrame.u32Height * 3 / 2;

    s32Ret = SAMPLE_VO_CREATE_POOL(4, u32FrameSize, 512, &pstChnThreadParam->u32UserPoolId);
    if (s32Ret) {
        SAMPLE_PRT("layer%d-chn%d SAMPLE_VO_CREATE_POOL failed, s32Ret = 0x%x\n", u32LayerID, u32ChnID, s32Ret);
        goto exit;
    }

    while (!pstChnThreadParam->u32ThreadForceStop) {
        BlkId = AX_POOL_GetBlock(pstChnThreadParam->u32UserPoolId, u32FrameSize, NULL);
        if (AX_INVALID_BLOCKID == BlkId) {
            // SAMPLE_PRT("layer%d-chn%d AX_POOL_GetBlock failed\n", u32LayerID, u32ChnID);
            usleep(10000);
            continue;
        }
        stFrame.u64PhyAddr[0] = AX_POOL_Handle2PhysAddr(BlkId);
        stFrame.u64VirAddr[0] = 0;

        u64PhysAddr = AX_POOL_Handle2PhysAddr(BlkId);

        if (!u64PhysAddr) {
            SAMPLE_PRT("AX_POOL_Handle2PhysAddr failed,BlkId=0x%x\n", BlkId);
            break;
        }

        u64VirAddr = AX_SYS_Mmap(u64PhysAddr, u32FrameSize);

        if (!u64VirAddr) {
            SAMPLE_PRT("AX_SYS_Mmap failed\n");
            break;
        }

        AX_IMG_FORMAT_E enPixFmt = stFrame.enImgFormat;
        AX_U32 u32Width = stFrame.u32Width;
        AX_U32 u32Height = stFrame.u32Height;
        AX_U32 u32Stride = stFrame.u32PicStride[0];
        AX_U8 *u8Mem = u64VirAddr;

        Fill_Smpte_YUV(u32Width, u32Height, u32Stride, u8Mem, u8Mem + u32Stride * u32Height, u8Mem + u32Stride * u32Height + 1);

        s32Ret = AX_SYS_Munmap(u64VirAddr, u32FrameSize);

        if (s32Ret) {
            SAMPLE_PRT("AX_SYS_Munmap failed,s32Ret=0x%x\n", s32Ret);
            break;
        }

        stFrame.u32BlkId[0] = BlkId;
        stFrame.u32BlkId[1] = AX_INVALID_BLOCKID;
        //SAMPLE_PRT("layer%d-chn%d start send frame, BlkId = 0x%x\n", u32LayerID, u32ChnID, BlkId);
        s32Ret = AX_VO_SendFrame(u32LayerID, u32ChnID, &stFrame, 0);
        if (s32Ret) {
            SAMPLE_PRT("layer%d-chn%d AX_VO_SendFrame failed, s32Ret = 0x%x\n", u32LayerID, u32ChnID, s32Ret);
            AX_POOL_ReleaseBlock(BlkId);
            usleep(16666);
            continue;
        }

        AX_POOL_ReleaseBlock(BlkId);

        usleep(20000);

		//framebuffer();

    }

exit:
    SAMPLE_PRT("layer%d-chn%d exit, s32Ret = 0x%x\n", u32LayerID, u32ChnID, s32Ret);

    return NULL;
}

static AX_S32 gLoopExit = 0;

static AX_VOID SigInt(AX_S32 s32SigNo)
{
    SAMPLE_PRT("Catch signal %d\n", s32SigNo);
    gLoopExit = 1;
}

static AX_VOID SigStop(AX_S32 s32SigNo)
{
    SAMPLE_PRT("Catch signal %d\n", s32SigNo);
    gLoopExit = 1;
}

static AX_U32 SAMPLE_VO_WIN_NUM(SAMPLE_VO_MODE_E enMode)
{
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
    case VO_MODE_BUTT:
        u32WndNum = 8;
        break;
    }

    SAMPLE_PRT("u32WndNum = %d\n", u32WndNum);

    return u32WndNum;
}

static AX_S32 ParseVoPubAttr(AX_CHAR *pStr, SAMPLE_VO_CONFIG_S *pstVoConf)
{
    AX_CHAR *p, *end;

    if (!pStr || !pstVoConf)
        return -EINVAL;

    p = pStr;

    if (strstr(p, "dpi")) {
        SAMPLE_PRT("dpi output\n");
        pstVoConf->enVoIntfType = VO_INTF_DPI;
    } else if (strstr(p, "dsi0")) {
        SAMPLE_PRT("dsi0 output\n");
        pstVoConf->enVoIntfType = VO_INTF_DSI0;
    } else if (strstr(p, "dsi1")) {
        SAMPLE_PRT("dsi1 output\n");
        pstVoConf->enVoIntfType = VO_INTF_DSI1;
    } else if (strstr(p, "2dsi")) {
        SAMPLE_PRT("dsi0 output\n");
        pstVoConf->enVoIntfType = VO_INTF_2DSI;
        pstVoConf->u32LayerNr = 2;
    } else if (strstr(p, "bt656")) {
        SAMPLE_PRT("bt656 output\n");
        pstVoConf->enVoIntfType = VO_INTF_BT656;
    } else if (strstr(p, "bt1120")) {
        SAMPLE_PRT("bt1120 output\n");
        pstVoConf->enVoIntfType = VO_INTF_BT1120;
    } else {
        SAMPLE_PRT("unsupported interface type, %s\n", p);
        return -EINVAL;
    }

    end = strstr(p, "@");
    p = end + 1;
    pstVoConf->stReso.u32Width = strtoul(p, &end, 10);
    end = strstr(p, "x");
    p = end + 1;
    pstVoConf->stReso.u32Height = strtoul(p, &end, 10);
    end = strstr(p, "@");
    p = end + 1;
    pstVoConf->stReso.u32RefreshRate = strtoul(p, &end, 10);

    SAMPLE_PRT("reso: %dx%d-%d\n", pstVoConf->stReso.u32Width,
               pstVoConf->stReso.u32Height, pstVoConf->stReso.u32RefreshRate);

    return 0;
}

static AX_VOID Usage(AX_CHAR *name)
{
    SAMPLE_PRT("command:\n");
    SAMPLE_PRT("\t-f: channel/layer fifo-depth set, rang: [0~8]\n");
    SAMPLE_PRT("\t-v: Interface@Resolution@VRefresh set:\n");
    SAMPLE_PRT("\t\tdpi@<w>x<h>@<refresh>: dpi output\n");
    SAMPLE_PRT("\t\tdsi0@<w>x<h>@<refresh>: mipi dsi0 output\n");
    SAMPLE_PRT("\t\tdsi1@<w>x<h>@<refresh>: mipi dsi1 output\n");
    SAMPLE_PRT("\t\t2dsi@<w>x<h>@<refresh>: dual mipi dsi output\n");
    SAMPLE_PRT("\t\tbt656@<w>x<h>@<refresh>: bt656 output\n");
    SAMPLE_PRT("\t\tbt1120@<w>x<h>@<refresh>: bt1120 output\n");

    SAMPLE_PRT("\t-m: style select:\n");
    SAMPLE_PRT("\t\t0: 1-MUX\n");
    SAMPLE_PRT("\t\t1: 2-MUX\n");
    SAMPLE_PRT("\t\t2: 4-MUX\n");
    SAMPLE_PRT("\t\t3: 8-MUX\n");
    SAMPLE_PRT("\t\t4: 9-MUX\n");
    SAMPLE_PRT("\t\t5: 16-MUX\n");

    SAMPLE_PRT("Example:\n");
    SAMPLE_PRT("\tsample_vo -v dsi0@1920x1080@60 -m 0\n");
    SAMPLE_PRT("\tsample_vo -v dsi0@480x360@60 -m 0\n");
}

AX_S32 main(AX_S32 argc, AX_CHAR *argv[])
{
    AX_S32 c;
    AX_S32 i, j, s32Ret = 0;
    AX_U32 u32LayerNr = 0, u32Mux = VO_MODE_2MUX, u32FifoDepth = 0;
    AX_U32 u32Count = 0;
    AX_U64 u64BlkSize = 0;
    SAMPLE_VO_CONFIG_S *pstVoConf = &stVoConf;
    SAMPLE_VO_LAYER_CONFIG_S *pstVoLayerConf;
    VO_VIDEO_LAYER_ATTR_S *pstVoLayerAttr;
    SAMPLE_VO_CHN_THREAD_PARAM_S stChnThreadParam[SAMPLE_VLAYER_MAX][64] = {0};

    SAMPLE_PRT("VO Sample. Build at %s %s\n", __DATE__, __TIME__);

    if (argc < 2) {
        //SAMPLE_PRT("parameters need to be specified\n");
        Usage(argv[0]);
        return 0;
    }

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, SigInt);
    signal(SIGTSTP, SigStop);

    opterr = 0;
    while ((c = getopt(argc, argv, "m:v:f:")) != -1) {
        switch (c) {
        case 'f':
            u32FifoDepth = strtoul(optarg, NULL, 10);
            break;
        case 'm':
            u32Mux = strtoul(optarg, NULL, 10);
            if (u32Mux > VO_MODE_16MUX)
                u32Mux = VO_MODE_16MUX;
            break;
        case 'v':
            s32Ret = ParseVoPubAttr(optarg, pstVoConf);
            if (s32Ret) {
                Usage(argv[0]);
                return s32Ret;
            }
            break;
        default:
            Usage(argv[0]);
            SAMPLE_PRT("use default mode\n");
            break;
        }
    }

    s32Ret = AX_SYS_Init();
    if (s32Ret) {
        SAMPLE_PRT("AX_SYS_Init failed, s32Ret = 0x%x\n", s32Ret);
        return -1;
    }

    s32Ret = AX_POOL_Exit();
    if (s32Ret) {
        SAMPLE_PRT("AX_POOL_Exit failed, s32Ret = 0x%x\n", s32Ret);
        goto exit0;
    }

    u32LayerNr = pstVoConf->u32LayerNr;
    SAMPLE_PRT("u32LayerNr = %d\n", u32LayerNr);

    for (i = 0; i < u32LayerNr; i++) {
        pstVoLayerConf = &pstVoConf->stVoLayer[i];
        pstVoLayerConf->enVoMode = u32Mux;
        pstVoLayerAttr = &pstVoLayerConf->stVoLayerAttr;
        pstVoLayerAttr->stImageSize.u32Width = (pstVoConf->stReso.u32Width + 0xF) & (~0xF);
        pstVoLayerAttr->stImageSize.u32Height = pstVoConf->stReso.u32Height;
        pstVoLayerAttr->stDispRect.u32Width = pstVoLayerAttr->stImageSize.u32Width;
        pstVoLayerAttr->stDispRect.u32Height = pstVoLayerAttr->stImageSize.u32Height;
        if (i == 1) {
            pstVoLayerAttr->stDispRect.u32X = pstVoConf->stReso.u32Width;
            pstVoLayerConf->enVoMode = VO_MODE_1MUX;
        }

        pstVoLayerAttr->u32ChnNr = SAMPLE_VO_WIN_NUM(pstVoLayerConf->enVoMode);
        pstVoConf->u32FifoDepth = u32FifoDepth;

        pstVoLayerAttr->u32PoolId = ~0;
        u64BlkSize = pstVoLayerAttr->stImageSize.u32Width * pstVoLayerAttr->stImageSize.u32Height * 3 / 2;
        s32Ret = SAMPLE_VO_CREATE_POOL(3, u64BlkSize, 512, &pstVoLayerAttr->u32PoolId);
        if (s32Ret) {
            SAMPLE_PRT("create layer%d failed, s32Ret = 0x%x\n", i, s32Ret);
            goto exit1;
        }

        SAMPLE_PRT("layer%d pool id = 0x%x\n", i, pstVoLayerAttr->u32PoolId);
    }

    s32Ret = AX_VO_Init();
    if (s32Ret) {
        SAMPLE_PRT("AX_VO_Init failed, s32Ret = 0x%x\n", s32Ret);
        goto exit2;
    }

    s32Ret = SAMPLE_COMM_VO_StartVO(pstVoConf);
    if (s32Ret) {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartVO failed, s32Ret = 0x%x\n", s32Ret);
        goto exit2;
    }

    for (i = 0; i < u32LayerNr; i++) {
        pstVoLayerConf = &pstVoConf->stVoLayer[i];
        for (j = 0; j < SAMPLE_VO_WIN_NUM(pstVoLayerConf->enVoMode); j++) {
            stChnThreadParam[i][j].u32LayerID = pstVoLayerConf->VoLayer;
            stChnThreadParam[i][j].u32ChnID = j;
            stChnThreadParam[i][j].u32ThreadForceStop = 0;
            stChnThreadParam[i][j].u32UserPoolId = ~0;
            pthread_create(&stChnThreadParam[i][j].ThreadID, NULL, SAMPLE_VO_CHN_THREAD, &stChnThreadParam[i][j]);
        }
    }

	framebuffer();

    while (!gLoopExit) {

        usleep(16666);
#if SLEEP_TEST_ENABLE
        if(++u32Count % 60 == 0)
            AX_SYS_Sleep();
        else
            SAMPLE_PRT("u32Count: %u\n", u32Count);
#endif
    }

    SAMPLE_PRT("gLoopExit = 0x%x\n", gLoopExit);

    for (i = 0; i < u32LayerNr; i++) {
        for (j = 0; j < SAMPLE_VO_WIN_NUM(pstVoLayerConf->enVoMode); j++) {
            if (stChnThreadParam[i][j].ThreadID) {
                stChnThreadParam[i][j].u32ThreadForceStop = 1;
                pthread_join(stChnThreadParam[i][j].ThreadID, NULL);
            }
        }
    }

    SAMPLE_COMM_VO_StopVO(pstVoConf);

    /* Destroy all chnnel-related pool */
    for (i = 0; i < u32LayerNr; i++) {
        for (j = 0; j < SAMPLE_VO_WIN_NUM(pstVoLayerConf->enVoMode); j++) {
            if (stChnThreadParam[i][j].u32UserPoolId != ~0)
                SAMPLE_VO_POOL_DESTROY(stChnThreadParam[i][j].u32UserPoolId);
        }
    }

exit2:
    AX_VO_Deinit();

exit1:
    /* Destroy all lyaer-related pool */
    for (i = 0; i < u32LayerNr; i++) {
        pstVoLayerConf = &pstVoConf->stVoLayer[i];
        if (pstVoLayerConf->stVoLayerAttr.u32PoolId != ~0)
            SAMPLE_VO_POOL_DESTROY(pstVoLayerConf->stVoLayerAttr.u32PoolId);
    }

exit0:
    AX_SYS_Deinit();

    SAMPLE_PRT("done, s32Ret = 0x%x\n", s32Ret);

    return s32Ret;
}
