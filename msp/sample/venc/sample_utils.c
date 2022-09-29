#include "sample_utils.h"

#include <assert.h>
#include <string.h>
#include "common_vin.h"
#include "common_arg_parse.h"

#define SAMPLE_LOG(str, arg...)    \
    do {    \
        printf("%s: %s:%d "str"\n", "sample_utils.c", __func__, __LINE__, ##arg); \
    } while(0)

#define SAMPLE_ERR_LOG(str, arg...)   \
    do{  \
        printf("%s: %s:%d Error! "str"\n", "sample_utils.c", __func__, __LINE__, ##arg); \
    }while(0)



void SetDefaultParameter(SAMPLE_VENC_CMD_PARA_T *pstPara)
{
    AX_S32 i;
    memset(pstPara, 0, sizeof(SAMPLE_VENC_CMD_PARA_T));
    pstPara->enCompressMode = AX_COMPRESS_MODE_NONE;
    pstPara->input      = "input.yuv";
    pstPara->output     = NULL;
    pstPara->inputFrameRate      = 30;
    pstPara->outputFrameRate      = 30;
    pstPara->horOffsetSrc = INVALID_DEFAULT;
    pstPara->verOffsetSrc = INVALID_DEFAULT;
    pstPara->syncType = -1;
    pstPara->rotation = 0;
    pstPara->inputFormat = 0;

    pstPara->nSrcWidth      = INVALID_DEFAULT;
    pstPara->nSrcHeight     = INVALID_DEFAULT;

    pstPara->picStride[0] = 0;
    pstPara->picStride[1] = 0;
    pstPara->picStride[2] = 0;

    pstPara->gopLength   = 30;
    pstPara->codecFormat = VENC_CODEC_NONE;

    pstPara->bitRate = 2000; //kbps
    pstPara->tolCtbRcIntra = INVALID_DEFAULT;
    pstPara->intraQpDelta = -2;

    pstPara->disableDeblocking = 0;
    pstPara->enablePerfTest = 0;
    pstPara->frameNum = -1;

    pstPara->qpHdr = INVALID_DEFAULT;
    pstPara->qpMin = 10;
    pstPara->qpMax = 51;
    pstPara->qpMinI = 10;
    pstPara->qpMaxI = 51;
    pstPara->picRc = INVALID_DEFAULT;
    pstPara->ctbRc = INVALID_DEFAULT; //CTB_RC
    pstPara->cpbSize = INVALID_DEFAULT;
    pstPara->bitrateWindow = INVALID_DEFAULT;
    pstPara->hrdConformance = 0;
    pstPara->rcMode = 0;
    pstPara->loopEncode = 0;

    pstPara->chnNum = 2;

    pstPara->enableSao = 1;

    pstPara->intraAreaLeft = pstPara->intraAreaRight = pstPara->intraAreaTop =
                            pstPara->intraAreaBottom = -1;  /* Disabled */
    pstPara->ipcm1AreaLeft = pstPara->ipcm1AreaRight = pstPara->ipcm1AreaTop =
                            pstPara->ipcm1AreaBottom = -1;  /* Disabled */
    pstPara->ipcm2AreaLeft = pstPara->ipcm2AreaRight = pstPara->ipcm2AreaTop =
                            pstPara->ipcm2AreaBottom = -1;  /* Disabled */

    pstPara->ipcm3AreaLeft = pstPara->ipcm3AreaRight = pstPara->ipcm3AreaTop =
                            pstPara->ipcm3AreaBottom = -1;  /* Disabled */
    pstPara->ipcm4AreaLeft = pstPara->ipcm4AreaRight = pstPara->ipcm4AreaTop =
                            pstPara->ipcm4AreaBottom = -1;  /* Disabled */
    pstPara->ipcm5AreaLeft = pstPara->ipcm5AreaRight = pstPara->ipcm5AreaTop =
                            pstPara->ipcm5AreaBottom = -1;  /* Disabled */
    pstPara->ipcm6AreaLeft = pstPara->ipcm6AreaRight = pstPara->ipcm6AreaTop =
                            pstPara->ipcm6AreaBottom = -1;  /* Disabled */
    pstPara->ipcm7AreaLeft = pstPara->ipcm7AreaRight = pstPara->ipcm7AreaTop =
                            pstPara->ipcm7AreaBottom = -1;  /* Disabled */
    pstPara->ipcm8AreaLeft = pstPara->ipcm8AreaRight = pstPara->ipcm8AreaTop =
                            pstPara->ipcm8AreaBottom = -1;  /* Disabled */
    pstPara->gdrDuration=0;

    pstPara->picSkip = 0;

    pstPara->sliceSize = 0;

    pstPara->enableCabac = 1;
    pstPara->enableDeblockOverride = 0;
    pstPara->deblockOverride = 0;

    pstPara->sei = 0;
    pstPara->videoRange = 0;
    pstPara->level = INVALID_DEFAULT;
    pstPara->profile = INVALID_DEFAULT;
    pstPara->tier = INVALID_DEFAULT;
    pstPara->blockRCSize= INVALID_DEFAULT;
    pstPara->rcQpDeltaRange = INVALID_DEFAULT;
    pstPara->rcBaseMBComplexity = INVALID_DEFAULT;
    pstPara->picQpDeltaMin = INVALID_DEFAULT;
    pstPara->picQpDeltaMax = INVALID_DEFAULT;
    pstPara->ctbRcRowQpStep = INVALID_DEFAULT;

    pstPara->gopType = 0;

    pstPara->roiMapDeltaQpBlockUnit=0;
    pstPara->roiMapDeltaQpEnable=0;
    pstPara->roiMapDeltaQpFile = NULL;
    pstPara->roiMapDeltaQpBinFile = NULL;
    pstPara->roiMapInfoBinFile        = NULL;
    pstPara->RoimapCuCtrlInfoBinFile  = NULL;
    pstPara->RoimapCuCtrlIndexBinFile = NULL;
    pstPara->RoiCuCtrlVer  = 0;
    pstPara->RoiQpDeltaVer = 1;
    pstPara->ipcmMapEnable = 0;
    pstPara->ipcmMapFile = NULL;

    pstPara->roiAttr[0].u32Index = 0;
    pstPara->roiAttr[1].u32Index = 1;
    pstPara->roiAttr[2].u32Index = 2;
    pstPara->roiAttr[3].u32Index = 3;
    pstPara->roiAttr[4].u32Index = 4;
    pstPara->roiAttr[5].u32Index = 5;
    pstPara->roiAttr[6].u32Index = 6;
    pstPara->roiAttr[7].u32Index = 7;

    /* constant chroma control */
    pstPara->constChromaEn = 0;
    pstPara->constCb = INVALID_DEFAULT;
    pstPara->constCr = INVALID_DEFAULT;

    pstPara->skip_frame_enabled_flag=0;
    pstPara->skip_frame_poc=0;

    /* skip mode */
    pstPara->skipMapEnable = 0;
    pstPara->skipMapFile = NULL;
    pstPara->skipMapBlockUnit = 0;

    /* mosaic */
    pstPara->mosaicEnables = 0;
    for(i = 0; i < MAX_MOSAIC_NUM; i++)
    {
        pstPara->mosHeight[i] = 0;
        pstPara->mosWidth[i] = 0;
        pstPara->mosXoffset[i] = 0;
        pstPara->mosYoffset[i] = 0;
    }

}

static AX_VOID VencHelp(AX_S8 *testApp)
{
    char help_info[] = {
        #include "help.dat"
    };

    fprintf(stdout, "Usage:  %s [options] -i inputfile\n\n", testApp);

    fprintf(stdout,
            "  -H --help                        This help\n\n");


    fprintf(stdout, "%s", help_info);

    exit(0);
}

AX_S32 VencParameterCheck(SAMPLE_VENC_CMD_PARA_T *pCml)
{
    return 0;
}

static SAMPLE_OPTION_T options[] =
{
    {"help",    'H', 2},
    {"nSrcWidth", 'w', 1},
    {"nSrcHeight", 'h', 1},
    {"picStride", '0', 1},                /* stride */

    {"enableCrop", '0', 1},
    {"cropWidth", 'x', 1},
    {"cropHeight", 'y', 1},
    {"horOffsetSrc", 'X', 1},
    {"verOffsetSrc", 'Y', 1},

    {"inputFormat", 'l', 1},        /* Input image format */
    {"colorConversion", 'O', 1},    /* RGB to YCbCr conversion type */
    {"rotation", 'r', 1},           /* Input image rotation */
    {"outputFrameRate", 'f', 1},
    {"inputLineBufferMode", '0', 1},
    {"inputLineBufferDepth", '0', 1},
    {"inputLineBufferAmountPerLoopback", '0', 1},
    {"inputFrameRate", 'j', 1},
    /*stride*/
    {"inputAlignmentExp", '0', 1},
    {"refAlignmentExp", '0', 1},
    {"refChromaAlignmentExp",  '0',  1},
    {"aqInfoAlignmentExp", '0', 1},

    {"input",   'i', 1},
    {"output",    'o', 1},
    {"loopEncode", 'p', 1},
    {"qp_size",   'Q', 1},
    {"qpMinI", '0', 1},             /* Minimum frame header qp for I picture */
    {"qpMaxI", '0', 1},             /* Maximum frame header qp for I picture */
    {"qpMin", 'n', 1},              /* Minimum frame header qp for any picture */
    {"qpMax", 'm', 1},              /* Maximum frame header qp for any picture */
    {"qpHdr",     'q', 1},
    {"hrdConformance", 'C', 1},     /* HDR Conformance (ANNEX C) */
    {"cpbSize", 'c', 1},            /* Coded Picture Buffer Size */
    {"rcMode", '0', 1},             /* 0: CBR, 1: VBR, 2: AVBR, 3: QPMAP, 4: FIXQP */
    {"intraQpDelta", 'A', 1},       /* QP adjustment for intra frames */
    {"fixedIntraQp", 'G', 1},       /* Fixed QP for all intra frames */
    {"bFrameQpDelta", 'V', 1},       /* QP adjustment for B frames */
    {"chnNum", 'N', 1},         /* Byte stream format (ANNEX B) */
    {"bitRate", 'B', 1},
    {"picRc",   'U', 1},
    {"ctbRc",   'u', 1},
    {"picSkip", 's', 1},            /* Frame skipping */
    {"profile",       'P', 1},   /* profile   (ANNEX A):support main and main still picture */
    {"level",         'L', 1},   /* Level * 30  (ANNEX A) */
    {"gopLength",  'R', 1},   /* IDR interval */

    {"bitrateWindow", 'g', 1},          /* bitrate window of pictures length */
    {"disableDeblocking", 'D', 1},
    {"enablePerfTest", 'W', 1},
    {"frameNum", 'E', 1},
    {"sliceSize", 'e', 1},
    {"enableSao", 'M', 1},          /* Enable or disable SAO */
    {"videoRange", 'k', 1},
    {"codecFormat", '0', 1},          /* select videoFormat: HEVC/H264/AV1 */
    {"enableCabac", 'K', 1},        /* H.264 entropy coding mode, 0 for CAVLC, 1 for CABAC */
    {"userData", 'z', 1},           /* SEI User data file */
    {"syncType", 'Z', 1},          /* video stabilization */

    /* Only long option can be used for all the following parameters because
    * we have no more letters to use. All shortOpt=0 will be identified by
    * long option. */
    {"cir", '0', 1},
    {"cpbMaxRate", '0', 1},         /* max bitrate for CPB VBR/CBR */
    {"intraArea", '0', 1},
    {"ipcm1Area", '0', 1},
    {"ipcm2Area", '0', 1},
    {"ipcm3Area", '0', 1},
    {"ipcm4Area", '0', 1},
    {"ipcm5Area", '0', 1},
    {"ipcm6Area", '0', 1},
    {"ipcm7Area", '0', 1},
    {"ipcm8Area", '0', 1},

    {"roi1Area", '0', 1},
    {"roi2Area", '0', 1},
    {"roi3Area", '0', 1},
    {"roi4Area", '0', 1},
    {"roi5Area", '0', 1},
    {"roi6Area", '0', 1},
    {"roi7Area", '0', 1},
    {"roi8Area", '0', 1},

    {"roi1Qp", '0', 1},
    {"roi2Qp", '0', 1},
    {"roi3Qp", '0', 1},
    {"roi4Qp", '0', 1},
    {"roi5Qp", '0', 1},
    {"roi6Qp", '0', 1},
    {"roi7Qp", '0', 1},
    {"roi8Qp", '0', 1},

    {"roi1Enable", '0', 1},
    {"roi2Enable", '0', 1},
    {"roi3Enable", '0', 1},
    {"roi4Enable", '0', 1},
    {"roi5Enable", '0', 1},
    {"roi6Enable", '0', 1},
    {"roi7Enable", '0', 1},
    {"roi8Enable", '0', 1},

    {"roi1AbsQp", '0', 1},
    {"roi2AbsQp", '0', 1},
    {"roi3AbsQp", '0', 1},
    {"roi4AbsQp", '0', 1},
    {"roi5AbsQp", '0', 1},
    {"roi6AbsQp", '0', 1},
    {"roi7AbsQp", '0', 1},
    {"roi8AbsQp", '0', 1},

    {"layerInRefIdc", '0', 1},	  /*H264 2bit nal_ref_idc*/

    {"ipcmFilterDisable", '0', 1},

    {"roiMapDeltaQpBlockUnit", '0', 1},
    {"roiMapDeltaQpEnable", '0', 1},
    {"ipcmMapEnable", '0', 1},

    {"constrainIntra", '0', 1},
    {"smoothingIntra", '0', 1},
    {"enableDeblockOverride", '0', 1},
    {"deblockOverride", '0', 1},
    {"enableScalingList", '0', 1},
    {"gopType", '0', 1},
    {"gopConfig", '0', 1},
    {"gopLowdelay", '0', 1},
    {"LTR", '0', 1},
    {"flexRefs", '0', 1},    /* flexible reference list */
    {"interlacedFrame", '0', 1},
    {"fieldOrder", '0', 1},
    {"tier", '0', 1},

    {"enCompressMode", 'd', 1},
    // FBDC_ENABLE
    {"UVheaderSize", '0', 1},
    {"UVpayloadSize", '0', 1},
    {"YheaderSize", '0', 1},
    {"YpayloadSize", '0', 1},
    {"CropX", '0', 1},
    {"CropY", '0', 1},

    {"gdrDuration", '0', 1},
    {"bitVarRangeI", '0', 1},
    {"bitVarRangeP", '0', 1},
    {"tolCtbRcInter", '0', 1},
    {"tolCtbRcIntra", '0', 1},
    {"ctbRowQpStep", '0', 1},

    {"roiMapDeltaQpFile", '0', 1},
    {"roiMapDeltaQpBinFile", '0', 1},
    {"ipcmMapFile", '0', 1},
    {"roiMapInfoBinFile", '0', 1},
    {"RoimapCuCtrlInfoBinFile", '0', 1},
    {"RoimapCuCtrlIndexBinFile", '0', 1},
    {"RoiCuCtrlVer", '0', 1},
    {"RoiQpDeltaVer", '0', 1 },

    {"rcQpDeltaRange",   '0', 1},   /* ctb rc qp delta range */
    {"rcBaseMBComplexity",   '0', 1},   /* ctb rc mb complexity base */
    {"picQpDeltaRange", '0', 1},

    /* constant chroma control */
    {"enableConstChroma", '0', 1}, /* enable constant chroma setting or not */
    {"constCb", '0', 1},           /* constant pixel value for CB */
    {"constCr", '0', 1},           /* constant pixel value for CR */

    {"skipFramePOC", '0', 1},
    {"POCConfig", '0', 1},

    /* skip map */
    {"skipMapEnable", '0', 1},
    {"skipMapFile", '0', 1},
    {"skipMapBlockUnit", '0', 1},

    /* sliceNode */
    {"sliceNode", '0', 1},

    /*Overlay*/
    {"overlayEnables", '0', 1},
    {"olInput1", '0', 1},
    {"olFormat1", '0', 1},
    {"olAlpha1", '0', 1},
    {"olWidth1", '0', 1},
    {"olHeight1", '0', 1},
    {"olXoffset1", '0', 1},
    {"olYoffset1", '0', 1},
    {"olYStride1", '0', 1},
    {"olUVStride1", '0', 1},
    {"olCropWidth1", '0', 1},
    {"olCropHeight1", '0', 1},
    {"olCropXoffset1", '0', 1},
    {"olCropYoffset1", '0', 1},
    {"olBitmapY1", '0', 1},
    {"olBitmapU1", '0', 1},
    {"olBitmapV1", '0', 1},
    {"olSuperTile1", '0', 1},
    {"olScaleWidth1", '0', 1},
    {"olScaleHeight1", '0', 1},

    {"olInput2", '0', 1},
    {"olFormat2", '0', 1},
    {"olAlpha2", '0', 1},
    {"olWidth2", '0', 1},
    {"olHeight2", '0', 1},
    {"olXoffset2", '0', 1},
    {"olYoffset2", '0', 1},
    {"olYStride2", '0', 1},
    {"olUVStride2", '0', 1},
    {"olCropWidth2", '0', 1},
    {"olCropHeight2", '0', 1},
    {"olCropXoffset2", '0', 1},
    {"olCropYoffset2", '0', 1},
    {"olBitmapY2", '0', 1},
    {"olBitmapU2", '0', 1},
    {"olBitmapV2", '0', 1},

    {"olInput3", '0', 1},
    {"olFormat3", '0', 1},
    {"olAlpha3", '0', 1},
    {"olWidth3", '0', 1},
    {"olHeight3", '0', 1},
    {"olXoffset3", '0', 1},
    {"olYoffset3", '0', 1},
    {"olYStride3", '0', 1},
    {"olUVStride3", '0', 1},
    {"olCropWidth3", '0', 1},
    {"olCropHeight3", '0', 1},
    {"olCropXoffset3", '0', 1},
    {"olCropYoffset3", '0', 1},
    {"olBitmapY3", '0', 1},
    {"olBitmapU3", '0', 1},
    {"olBitmapV3", '0', 1},

    {"olInput4", '0', 1},
    {"olFormat4", '0', 1},
    {"olAlpha4", '0', 1},
    {"olWidth4", '0', 1},
    {"olHeight4", '0', 1},
    {"olXoffset4", '0', 1},
    {"olYoffset4", '0', 1},
    {"olYStride4", '0', 1},
    {"olUVStride4", '0', 1},
    {"olCropWidth4", '0', 1},
    {"olCropHeight4", '0', 1},
    {"olCropXoffset4", '0', 1},
    {"olCropYoffset4", '0', 1},
    {"olBitmapY4", '0', 1},
    {"olBitmapU4", '0', 1},
    {"olBitmapV4", '0', 1},

    {"olInput5", '0', 1},
    {"olFormat5", '0', 1},
    {"olAlpha5", '0', 1},
    {"olWidth5", '0', 1},
    {"olHeight5", '0', 1},
    {"olXoffset5", '0', 1},
    {"olYoffset5", '0', 1},
    {"olYStride5", '0', 1},
    {"olUVStride5", '0', 1},
    {"olCropWidth5", '0', 1},
    {"olCropHeight5", '0', 1},
    {"olCropXoffset5", '0', 1},
    {"olCropYoffset5", '0', 1},
    {"olBitmapY5", '0', 1},
    {"olBitmapU5", '0', 1},
    {"olBitmapV5", '0', 1},

    {"olInput6", '0', 1},
    {"olFormat6", '0', 1},
    {"olAlpha6", '0', 1},
    {"olWidth6", '0', 1},
    {"olHeight6", '0', 1},
    {"olXoffset6", '0', 1},
    {"olYoffset6", '0', 1},
    {"olYStride6", '0', 1},
    {"olUVStride6", '0', 1},
    {"olCropWidth6", '0', 1},
    {"olCropHeight6", '0', 1},
    {"olCropXoffset6", '0', 1},
    {"olCropYoffset6", '0', 1},
    {"olBitmapY6", '0', 1},
    {"olBitmapU6", '0', 1},
    {"olBitmapV6", '0', 1},

    {"olInput7", '0', 1},
    {"olFormat7", '0', 1},
    {"olAlpha7", '0', 1},
    {"olWidth7", '0', 1},
    {"olHeight7", '0', 1},
    {"olXoffset7", '0', 1},
    {"olYoffset7", '0', 1},
    {"olYStride7", '0', 1},
    {"olUVStride7", '0', 1},
    {"olCropWidth7", '0', 1},
    {"olCropHeight7", '0', 1},
    {"olCropXoffset7", '0', 1},
    {"olCropYoffset7", '0', 1},
    {"olBitmapY7", '0', 1},
    {"olBitmapU7", '0', 1},
    {"olBitmapV7", '0', 1},

    {"olInput8", '0', 1},
    {"olFormat8", '0', 1},
    {"olAlpha8", '0', 1},
    {"olWidth8", '0', 1},
    {"olHeight8", '0', 1},
    {"olXoffset8", '0', 1},
    {"olYoffset8", '0', 1},
    {"olYStride8", '0', 1},
    {"olUVStride8", '0', 1},
    {"olCropWidth8", '0', 1},
    {"olCropHeight8", '0', 1},
    {"olCropXoffset8", '0', 1},
    {"olCropYoffset8", '0', 1},
    {"olBitmapY8", '0', 1},
    {"olBitmapU8", '0', 1},
    {"olBitmapV8", '0', 1},

    /* Mosaic */
    {"mosaicEnables", '0', 1},
    {"mosArea01", '0', 1},
    {"mosArea02", '0', 1},
    {"mosArea03", '0', 1},
    {"mosArea04", '0', 1},
    {"mosArea05", '0', 1},
    {"mosArea06", '0', 1},
    {"mosArea07", '0', 1},
    {"mosArea08", '0', 1},
    {"mosArea09", '0', 1},
    {"mosArea10", '0', 1},
    {"mosArea11", '0', 1},
    {"mosArea12", '0', 1},

    {NULL,      0,   0}        /* Format of last line */
};

AX_S32 VencParameterGet(AX_S32 argc, AX_S8 **argv, SAMPLE_VENC_CMD_PARA_T *pstPara)
{
    SAMPLE_PARAMETER_T prm;
    AX_S32 status = 0;
    AX_S32 ret, i;
    AX_S8 *p;
    AX_S32 bpsAdjustCount = 0;
    SAMPLE_PARAMETER_T *pPrm = &prm;
    SAMPLE_VENC_CMD_PARA_T *cml = pstPara;
    AX_CHAR *optarg;

    prm.cnt = 1;

    if (argc < 2) {
        VencHelp(argv[0]);
        exit(0);
    }

    while ((ret = SampleGetOption(argc, argv, options, &prm)) != -1)
    {
        if (ret == -2) {
        status = -1;
        }

        p = prm.argument;
        optarg = p;

        // SAMPLE_LOG("prm.short_opt:%c", prm.short_opt);
        switch (pPrm->short_opt)
        {
        case 'H':
            VencHelp(argv[0]);
            break;
        case 'i':
            pstPara->input = p;
            break;
        case 'o':
            pstPara->output = p;
            break;
        case 'w':
            pstPara->nSrcWidth = atoi(p);
            break;
        case 'h':
            pstPara->nSrcHeight = atoi(p);
            break;
        case 'z':
            pstPara->userData = p;
            break;
        case 'x':
            pstPara->cropWidth = atoi(p);
            break;
        case 'y':
            pstPara->cropHeight = atoi(p);
            break;
        case 'X':
            pstPara->horOffsetSrc = atoi(p);
            break;
        case 'Y':
            pstPara->verOffsetSrc = atoi(p);
            break;
        case 'l':
            pstPara->inputFormat = atoi(p);
            break;
        case 'f':
            pstPara->outputFrameRate = atoi(p);
            break;
        case 'j':
            pstPara->inputFrameRate = atoi(p);
            break;
        case 'p':
            pstPara->loopEncode = atoi(p);
            break;
        case 'K':
            pstPara->enableCabac = atoi(p);
            break;

        case 'q':
            pstPara->qpHdr = atoi(p);
            break;
        case 'n':
            pstPara->qpMin = atoi(p);
            break;
        case 'm':
            pstPara->qpMax = atoi(p);
            break;

        case 'B':
            pstPara->bitRate = atoi(p);
            break;
        case 'U':
            pstPara->picRc = atoi(p);
            break;
        case 'u':
            pstPara->ctbRc = atoi(p); //CTB_RC
            break;
        case 'C':
            pstPara->hrdConformance = atoi(p);
            break;
        case 'c':
            pstPara->cpbSize = atoi(p);
            break;
        case 's':
            pstPara->picSkip = atoi(p);
            break;
        case 'L':
            pstPara->level = atoi(p);
            break;
        case 'P':
            pstPara->profile = atoi(p);
            break;
        case 'r':
            pstPara->rotation = atoi(p);
            break;
        case 'R':
            pstPara->gopLength = atoi(p);
            break;
        case 'Z':
            pstPara->syncType = atoi(p);
            break;
        case 'A':
            pstPara->intraQpDelta = atoi(p);
            break;
        case 'D':
            pstPara->disableDeblocking = atoi(p);
            break;
        case 'W':
            pstPara->enablePerfTest = atoi(p);
            break;
        case 'E':
            pstPara->frameNum = atoi(p);
            break;
        case 'e':
            pstPara->sliceSize = atoi(p);
            break;

        case 'g':
            pstPara->bitrateWindow = atoi(p);
            break;
        case 'N':
            pstPara->chnNum = atoi(p);
            break;
        case 'M':
            pstPara->enableSao = atoi(p);
            break;

        case 'k':
            pstPara->videoRange = atoi(p);
            break;

        case 'S':
            pstPara->sei = atoi(p);
            break;

        case 'd':
            pstPara->enCompressMode = atoi(p);
            break;
        case '0':
        {
            // FBDC_ENABLE
            if (strcmp(pPrm->longOpt, "UVheaderSize") == 0) {
                cml->UVheaderSize = atoi(optarg);
                break;
            }
            if (strcmp(pPrm->longOpt, "UVpayloadSize") == 0) {
                cml->UVpayloadSize = atoi(optarg);
                break;
            }
            if (strcmp(pPrm->longOpt, "YheaderSize") == 0) {
                cml->YheaderSize = atoi(optarg);
                break;
            }
            if (strcmp(pPrm->longOpt, "YpayloadSize") == 0) {
                cml->YpayloadSize = atoi(optarg);
                break;
            }
            if (strcmp(pPrm->longOpt, "CropX") == 0) {
                cml->CropX = atoi(optarg);
                break;
            }
            if (strcmp(pPrm->longOpt, "CropY") == 0) {
                cml->CropY = atoi(optarg);
                break;
            }

            /* Check long option */
            if (strcmp(pPrm->longOpt, "tier") == 0)
                pstPara->tier = atoi(p);
            //roi area 1
            if (strcmp(pPrm->longOpt, "roi1Enable") == 0) {
                pstPara->roiAttr[0].bEnable = atoi(p);
                pstPara->roiAttr[0].u32Index = 0;
            }
            if (strcmp(pPrm->longOpt, "roi1AbsQp") == 0)
                pstPara->roiAttr[0].bAbsQp = atoi(p);
            if (strcmp(pPrm->longOpt, "roi1Qp") == 0)
                pstPara->roiAttr[0].s32RoiQp = atoi(p);
            //roi area 2
            if (strcmp(pPrm->longOpt, "roi2Enable") == 0) {
                pstPara->roiAttr[1].bEnable = atoi(p);
                pstPara->roiAttr[1].u32Index = 1;
            }
            if (strcmp(pPrm->longOpt, "roi2AbsQp") == 0)
                pstPara->roiAttr[1].bAbsQp = atoi(p);
            if (strcmp(pPrm->longOpt, "roi2Qp") == 0)
                pstPara->roiAttr[1].s32RoiQp = atoi(p);
            //roi area 3
            if (strcmp(pPrm->longOpt, "roi3Enable") == 0) {
                pstPara->roiAttr[2].bEnable = atoi(p);
                pstPara->roiAttr[2].u32Index = 2;
            }
            if (strcmp(pPrm->longOpt, "roi3AbsQp") == 0)
                pstPara->roiAttr[2].bAbsQp = atoi(p);
            if (strcmp(pPrm->longOpt, "roi3Qp") == 0)
                pstPara->roiAttr[2].s32RoiQp = atoi(p);
            //roi area 4
            if (strcmp(pPrm->longOpt, "roi4Enable") == 0) {
                pstPara->roiAttr[3].bEnable = atoi(p);
                pstPara->roiAttr[3].u32Index = 3;
            }
            if (strcmp(pPrm->longOpt, "roi4AbsQp") == 0)
                pstPara->roiAttr[3].bAbsQp = atoi(p);
            if (strcmp(pPrm->longOpt, "roi4Qp") == 0)
                pstPara->roiAttr[3].s32RoiQp = atoi(p);
            //roi area 5
            if (strcmp(pPrm->longOpt, "roi5Enable") == 0) {
                pstPara->roiAttr[4].bEnable = atoi(p);
                pstPara->roiAttr[4].u32Index = 4;
            }
            if (strcmp(pPrm->longOpt, "roi5AbsQp") == 0)
                pstPara->roiAttr[4].bAbsQp = atoi(p);
            if (strcmp(pPrm->longOpt, "roi5Qp") == 0)
                pstPara->roiAttr[4].s32RoiQp = atoi(p);
            //roi area 6
            if (strcmp(pPrm->longOpt, "roi6Enable") == 0) {
                pstPara->roiAttr[5].bEnable = atoi(p);
                pstPara->roiAttr[5].u32Index = 5;
            }
            if (strcmp(pPrm->longOpt, "roi6AbsQp") == 0)
                pstPara->roiAttr[5].bAbsQp = atoi(p);
            if (strcmp(pPrm->longOpt, "roi6Qp") == 0)
                pstPara->roiAttr[5].s32RoiQp = atoi(p);
            //roi area 7
            if (strcmp(pPrm->longOpt, "roi7Enable") == 0) {
                pstPara->roiAttr[6].bEnable = atoi(p);
                pstPara->roiAttr[6].u32Index = 6;
            }
            if (strcmp(pPrm->longOpt, "roi7AbsQp") == 0)
                pstPara->roiAttr[6].bAbsQp = atoi(p);
            if (strcmp(pPrm->longOpt, "roi7Qp") == 0)
                pstPara->roiAttr[6].s32RoiQp = atoi(p);
            //roi area 8
            if (strcmp(pPrm->longOpt, "roi8Enable") == 0) {
                pstPara->roiAttr[7].bEnable = atoi(p);
                pstPara->roiAttr[7].u32Index = 7;
            }
            if (strcmp(pPrm->longOpt, "roi8AbsQp") == 0)
                pstPara->roiAttr[7].bAbsQp = atoi(p);
            if (strcmp(pPrm->longOpt, "roi8Qp") == 0)
                pstPara->roiAttr[7].s32RoiQp = atoi(p);

            if (strcmp(pPrm->longOpt, "roiMapDeltaQpBlockUnit") == 0)
                pstPara->roiMapDeltaQpBlockUnit = atoi(p);

            if (strcmp(pPrm->longOpt, "roiMapDeltaQpEnable") == 0)
                pstPara->roiMapDeltaQpEnable = atoi(p);

            if (strcmp(pPrm->longOpt, "ipcmMapEnable") == 0)
                pstPara->ipcmMapEnable = atoi(p);

            if (strcmp(pPrm->longOpt, "codecFormat") == 0) {
                if (strcmp(p, "hevc") == 0)
                    pstPara->codecFormat = VENC_CODEC_HEVC;
                else if (strcmp(p, "h264") == 0)
                    pstPara->codecFormat = VENC_CODEC_H264;
                else {
                    assert(0 && "unknown codecFormat");
                }
            }

            if (strcmp(pPrm->longOpt, "enableCrop") == 0)
                pstPara->enableCrop = atoi(p);

            if (strcmp(pPrm->longOpt, "picStride") == 0) {
                /* Argument must be "xx:yy:XX".
                * xx is picStride[0], Y stride */
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->picStride[0] = atoi(p);
                /* yy is picStride[1], Cb stride */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->picStride[1] = atoi(p);
                /* yy is picStride[2], Cr stride */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->picStride[2] = atoi(p);
            }

            if (strcmp(pPrm->longOpt, "intraArea") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->intraAreaLeft = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->intraAreaTop = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->intraAreaRight = atoi(p);
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->intraAreaBottom = atoi(p);
                pstPara->intraAreaEnable = 1;
            }

            if (strcmp(pPrm->longOpt, "ipcm1Area") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1) break;
                pstPara->ipcm1AreaLeft = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1) break;
                pstPara->ipcm1AreaTop = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1) break;
                pstPara->ipcm1AreaRight = atoi(p);
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->ipcm1AreaBottom = atoi(p);
            }

            if (strcmp(pPrm->longOpt, "ipcm2Area") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1) break;
                pstPara->ipcm2AreaLeft = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1) break;
                pstPara->ipcm2AreaTop = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1) break;
                pstPara->ipcm2AreaRight = atoi(p);
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->ipcm2AreaBottom = atoi(p);
            }

            if (strcmp(pPrm->longOpt, "ipcm3Area") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1) break;
                pstPara->ipcm3AreaLeft = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1) break;
                pstPara->ipcm3AreaTop = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1) break;
                pstPara->ipcm3AreaRight = atoi(p);
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->ipcm3AreaBottom = atoi(p);
            }

            if (strcmp(pPrm->longOpt, "ipcm4Area") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1) break;
                pstPara->ipcm4AreaLeft = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1) break;
                pstPara->ipcm4AreaTop = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1) break;
                pstPara->ipcm4AreaRight = atoi(p);
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->ipcm4AreaBottom = atoi(p);
            }

            if (strcmp(pPrm->longOpt, "ipcm5Area") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1) break;
                pstPara->ipcm5AreaLeft = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1) break;
                pstPara->ipcm5AreaTop = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1) break;
                pstPara->ipcm5AreaRight = atoi(p);
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->ipcm5AreaBottom = atoi(p);
            }

            if (strcmp(pPrm->longOpt, "ipcm6Area") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->ipcm6AreaLeft = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->ipcm6AreaTop = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->ipcm6AreaRight = atoi(p);
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->ipcm6AreaBottom = atoi(p);
            }

            if (strcmp(pPrm->longOpt, "ipcm7Area") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->ipcm7AreaLeft = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->ipcm7AreaTop = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->ipcm7AreaRight = atoi(p);
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->ipcm7AreaBottom = atoi(p);
            }

            if (strcmp(pPrm->longOpt, "ipcm8Area") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->ipcm8AreaLeft = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->ipcm8AreaTop = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->ipcm8AreaRight = atoi(p);
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->ipcm8AreaBottom = atoi(p);
            }

            if (strcmp(pPrm->longOpt, "roi1Area") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->roiAttr[0].stRoiArea.u32X = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->roiAttr[0].stRoiArea.u32Y = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->roiAttr[0].stRoiArea.u32Width = atoi(p);
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->roiAttr[0].stRoiArea.u32Height = atoi(p);
            }

            if (strcmp(pPrm->longOpt, "roi2Area") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->roiAttr[1].stRoiArea.u32X = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->roiAttr[1].stRoiArea.u32Y = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->roiAttr[1].stRoiArea.u32Width = atoi(p);
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->roiAttr[1].stRoiArea.u32Height = atoi(p);
            }

            if (strcmp(pPrm->longOpt, "roi3Area") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->roiAttr[2].stRoiArea.u32X = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->roiAttr[2].stRoiArea.u32Y = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->roiAttr[2].stRoiArea.u32Width = atoi(p);
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->roiAttr[2].stRoiArea.u32Height = atoi(p);
            }

            if (strcmp(pPrm->longOpt, "roi4Area") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->roiAttr[3].stRoiArea.u32X = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->roiAttr[3].stRoiArea.u32Y = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->roiAttr[3].stRoiArea.u32Width = atoi(p);
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->roiAttr[3].stRoiArea.u32Height = atoi(p);
            }

            if (strcmp(pPrm->longOpt, "roi5Area") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->roiAttr[4].stRoiArea.u32X = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->roiAttr[4].stRoiArea.u32Y = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->roiAttr[4].stRoiArea.u32Width = atoi(p);
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->roiAttr[4].stRoiArea.u32Height = atoi(p);
            }

            if (strcmp(pPrm->longOpt, "roi6Area") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->roiAttr[5].stRoiArea.u32X = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->roiAttr[5].stRoiArea.u32Y = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->roiAttr[5].stRoiArea.u32Width = atoi(p);
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->roiAttr[5].stRoiArea.u32Height = atoi(p);
            }

            if (strcmp(pPrm->longOpt, "roi7Area") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->roiAttr[6].stRoiArea.u32X = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->roiAttr[6].stRoiArea.u32Y = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->roiAttr[6].stRoiArea.u32Width = atoi(p);
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->roiAttr[6].stRoiArea.u32Height = atoi(p);
            }

            if (strcmp(pPrm->longOpt, "roi8Area") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->roiAttr[7].stRoiArea.u32X = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->roiAttr[7].stRoiArea.u32Y = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->roiAttr[7].stRoiArea.u32Width = atoi(p);
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->roiAttr[7].stRoiArea.u32Height = atoi(p);
            }

            if (strcmp(pPrm->longOpt, "enableDeblockOverride") == 0)
                pstPara->enableDeblockOverride = atoi(p);
            if (strcmp(pPrm->longOpt, "deblockOverride") == 0)
                pstPara->deblockOverride = atoi(p);

            if (strcmp(pPrm->longOpt, "fieldOrder") == 0)
                pstPara->fieldOrder = atoi(p);
            if (strcmp(pPrm->longOpt, "gopType") == 0)
                pstPara->gopType = atoi(p);

            if (strcmp(pPrm->longOpt, "skipFramePOC") == 0) {
                pstPara->skip_frame_poc = atoi(p);
                pstPara->skip_frame_enabled_flag = (pstPara->skip_frame_poc !=0);
            }

            if (strcmp(pPrm->longOpt, "gdrDuration") == 0)
                pstPara->gdrDuration = atoi(p);

            if (strcmp(pPrm->longOpt, "tolCtbRcInter") == 0)
                pstPara->tolCtbRcInter = atof(p);
            if (strcmp(pPrm->longOpt, "tolCtbRcIntra") == 0)
                pstPara->tolCtbRcIntra = atof(p);

            if (strcmp(pPrm->longOpt, "roiMapDeltaQpFile") == 0)
                pstPara->roiMapDeltaQpFile = p;
            if (strcmp(pPrm->longOpt, "roiMapDeltaQpBinFile") == 0)
                pstPara->roiMapDeltaQpBinFile = p;
            if (strcmp(pPrm->longOpt, "ipcmMapFile") == 0)
                pstPara->ipcmMapFile = p;
            if (strcmp(pPrm->longOpt, "roiMapInfoBinFile") == 0)
                pstPara->roiMapInfoBinFile = p;
            if (strcmp(pPrm->longOpt, "RoimapCuCtrlInfoBinFile") == 0)
                pstPara->RoimapCuCtrlInfoBinFile = p;
            if (strcmp(pPrm->longOpt, "RoimapCuCtrlIndexBinFile") == 0)
                pstPara->RoimapCuCtrlIndexBinFile = p;
            if (strcmp(pPrm->longOpt, "RoiCuCtrlVer") == 0)
                pstPara->RoiCuCtrlVer = atoi(p);
            if (strcmp(pPrm->longOpt, "RoiQpDeltaVer") == 0)
                pstPara->RoiQpDeltaVer = atoi(p);

            if (strcmp(pPrm->longOpt, "blockRCSize") == 0)
                pstPara->blockRCSize = atoi(p);

            if (strcmp(pPrm->longOpt, "rcQpDeltaRange") == 0)
                pstPara->rcQpDeltaRange = atoi(p);

            if (strcmp(pPrm->longOpt, "rcBaseMBComplexity") == 0)
                pstPara->rcBaseMBComplexity = atoi(p);

            if (strcmp(pPrm->longOpt, "picQpDeltaRange") == 0) {
                if ((i = SampleParseDelim(p, ':')) == -1) break;
                pstPara->picQpDeltaMin = atoi(p);
                p += i + 1;
                pstPara->picQpDeltaMax = atoi(p);
            }

            if (strcmp(pPrm->longOpt, "ctbRowQpStep") == 0)
                pstPara->ctbRcRowQpStep = atoi(p);

            /* constant chroma control */
            if (strcmp(pPrm->longOpt, "enableConstChroma") == 0)
                pstPara->constChromaEn = atoi(p);
            if (strcmp(pPrm->longOpt, "constCb") == 0)
                pstPara->constCb = atoi(p);
            if (strcmp(pPrm->longOpt, "constCr") == 0)
                pstPara->constCr = atoi(p);

            /* qpMin/qpMax for I picture */
            if (strcmp(pPrm->longOpt, "qpMinI") == 0)
                pstPara->qpMinI = atoi(p);
            if (strcmp(pPrm->longOpt, "qpMaxI") == 0)
                pstPara->qpMaxI = atoi(p);

            /* vbr, controlled by qpMin */
            if (strcmp(pPrm->longOpt, "rcMode") == 0)
                pstPara->rcMode = atoi(p);

            /* skip map */
            if (strcmp(pPrm->longOpt, "skipMapEnable") == 0)
                pstPara->skipMapEnable = atoi(p);
            if (strcmp(pPrm->longOpt, "skipMapBlockUnit") == 0)
                pstPara->skipMapBlockUnit = atoi(p);
            if (strcmp(pPrm->longOpt, "skipMapFile") == 0)
                pstPara->skipMapFile = p;

            if(strcmp(pPrm->longOpt, "mosaicEnables") == 0)
                pstPara->mosaicEnables = atoi(p);

            if (strcmp(pPrm->longOpt, "mosArea01") == 0)
            {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosXoffset[0] = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosYoffset[0] = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosWidth[0] = atoi(p) - pstPara->mosXoffset[0];
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->mosHeight[0] = atoi(p) - pstPara->mosYoffset[0];
            }

            if (strcmp(pPrm->longOpt, "mosArea02") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosXoffset[1] = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosYoffset[1] = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosWidth[1] = atoi(p) - pstPara->mosXoffset[1];
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->mosHeight[1] = atoi(p) - pstPara->mosYoffset[1];
            }

            if (strcmp(pPrm->longOpt, "mosArea03") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosXoffset[2] = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosYoffset[2] = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosWidth[2] = atoi(p) - pstPara->mosXoffset[2];
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->mosHeight[2] = atoi(p) - pstPara->mosYoffset[2];
            }

            if (strcmp(pPrm->longOpt, "mosArea04") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosXoffset[3] = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosYoffset[3] = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosWidth[3] = atoi(p) - pstPara->mosXoffset[3];
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->mosHeight[3] = atoi(p) - pstPara->mosYoffset[3];
            }

            if (strcmp(pPrm->longOpt, "mosArea05") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosXoffset[4] = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosYoffset[4] = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosWidth[4] = atoi(p) - pstPara->mosXoffset[4];
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->mosHeight[4] = atoi(p) - pstPara->mosYoffset[4];
            }

            if (strcmp(pPrm->longOpt, "mosArea06") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosXoffset[5] = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosYoffset[5] = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosWidth[5] = atoi(p) - pstPara->mosXoffset[5];
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->mosHeight[5] = atoi(p) - pstPara->mosYoffset[5];
            }

            if (strcmp(pPrm->longOpt, "mosArea07") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosXoffset[6] = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosYoffset[6] = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosWidth[6] = atoi(p) - pstPara->mosXoffset[6];
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->mosHeight[6] = atoi(p) - pstPara->mosYoffset[6];
            }

            if (strcmp(pPrm->longOpt, "mosArea08") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosXoffset[7] = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosYoffset[7] = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosWidth[7] = atoi(p) - pstPara->mosXoffset[7];
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->mosHeight[7] = atoi(p) - pstPara->mosYoffset[7];
            }

            if (strcmp(pPrm->longOpt, "mosArea09") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosXoffset[8] = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosYoffset[8] = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosWidth[8] = atoi(p) - pstPara->mosXoffset[8];
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->mosHeight[8] = atoi(p) - pstPara->mosYoffset[8];
            }

            if (strcmp(pPrm->longOpt, "mosArea10") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosXoffset[9] = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosYoffset[9] = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosWidth[9] = atoi(p) - pstPara->mosXoffset[9];
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->mosHeight[9] = atoi(p) - pstPara->mosYoffset[9];
            }

            if (strcmp(pPrm->longOpt, "mosArea11") == 0) {
                /* Argument must be "xx:yy:XX:YY".
                * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosXoffset[10] = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosYoffset[10] = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosWidth[10] = atoi(p) - pstPara->mosXoffset[10];
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->mosHeight[10] = atoi(p) - pstPara->mosYoffset[10];
            }

            if (strcmp(pPrm->longOpt, "mosArea12") == 0) {
                /* Argument must be "xx:yy:XX:YY".
           * xx is left coordinate, replace first ':' with 0 */
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosXoffset[11] = atoi(p);
                /* yy is top coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosYoffset[11] = atoi(p);
                /* XX is right coordinate */
                p += i + 1;
                if ((i = SampleParseDelim(p, ':')) == -1)
                    break;
                pstPara->mosWidth[11] = atoi(p) - pstPara->mosXoffset[11];
                /* YY is bottom coordinate */
                p += i + 1;
                pstPara->mosHeight[11] = atoi(p) - pstPara->mosYoffset[11];
            }
        }
        break;

        default:
            SAMPLE_ERR_LOG("unknow options:%c.\n", prm.short_opt);
            break;
        }
    }

    if(VencParameterCheck(pstPara) != 0)
        status = -1;

    return status;
}
