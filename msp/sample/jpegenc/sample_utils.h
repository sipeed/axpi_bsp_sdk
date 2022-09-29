#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "ax_global_type.h"
#include "ax_comm_venc.h"

#ifndef MAX_PATH
#define MAX_PATH   256
#endif

#define INVALID_DEFAULT -255

#define MAX_OVERLAY_NUM 8
#define ALIGN_UP(x, align) (((x) + ((align)-1)) & ~((align)-1))

#define MAX_WIDTH_DEFAULT 16384 // 8192
#define MAX_HEIGHT_DEFAULT 16384  // 8192
#define SRC_WIDTH_DEFAULT 1920
#define SRC_HEIGHT_DEFAULT 1080


/** Jpeg Format */
typedef enum
{
    JPEGENC_JPEG,
    JPEGENC_MJPEG = 1,
} SAMPLE_JPEGENC_FORMAT_E;

typedef enum
{
    JPEGENC_RC_NONE,
    JPEGENC_CBR = 1,  /* CBR RC mode */
    JPEGENC_VBR = 2,  /* VBR RC mode */
    JPEGENC_FIXQP,
    JPEGENC_RC_BUTT
} SAMPLE_JPEGENC_RCMODE;

/* Structure for command line options */
typedef struct _SAMPLE_CMD_PARA
{
    AX_CHAR input[MAX_PATH];
    AX_CHAR output[MAX_PATH];
    AX_CHAR *inputPath;
    AX_CHAR inputThumb[MAX_PATH];
    AX_CHAR *roimapFile;
    AX_CHAR *nonRoiFilter;
    AX_S32 firstPic;
    AX_S32 lastPic;
    // FBDC_ENABLE
    AX_S32 UVheaderSize;
    AX_S32 UVpayloadSize;
    AX_S32 YheaderSize;
    AX_S32 YpayloadSize;
    AX_S32 CropX;
    AX_S32 CropY;

    AX_S32 maxWidth;
    AX_S32 maxHeight;
    AX_S32 output_width;
    AX_S32 output_height;
    AX_S32 lumWidthSrc;
    AX_S32 lumHeightSrc;
    AX_S32 strideAlign;
    AX_S32 picStride[3];
    AX_S32 horOffsetSrc;
    AX_S32 verOffsetSrc;
    AX_S32 restartInterval;

    AX_IMG_FORMAT_E enFrameFormat;
    AX_S32 colorConversion;
    AX_S32 rotation;
    AX_S32 partialCoding;
    AX_S32 codingMode;
    AX_S32 markerType;
    AX_S32 qFactor;
    AX_BOOL qTableEnable;
    AX_S32 nonRoiLevel;
    AX_CHAR qTablePath[MAX_PATH];
    AX_S32 unitsType;
    AX_S32 xdensity;
    AX_S32 ydensity;
    AX_S32 thumbnail;
    AX_S32 widthThumb;
    AX_S32 heightThumb;
    AX_S32 lumWidthSrcThumb;
    AX_S32 lumHeightSrcThumb;
    AX_S32 horOffsetSrcThumb;
    AX_S32 verOffsetSrcThumb;
    // AX_S32 writeOut;
    AX_S32 comLength;
    AX_CHAR com[MAX_PATH];
    AX_S32 inputLineBufMode;
    AX_S32 inputLineBufDepth;
    AX_U32 amountPerLoopBack;
    AX_U32 hashtype;
    AX_S32 mirror;
    AX_S32 formatCustomizedType;
    AX_U32 constCb;
    AX_S32 constChromaEn;
    AX_U32 constCr;
    AX_S32 predictMode;
    AX_S32 ptransValue;
    AX_U32 bitRate;
    AX_U32 srcFrameRate; /* RW; Range:[1, 240]; the input frame rate of the venc chnnel */
    AX_U32 frameRateDenom;/* RW; frameRateNum/frameRateDenom Range:[1/64, 240]; frameRateNum/frameRateDenom can not be larger than srcFrameRate */
    AX_U32 frameRateNum; /* RW; Range:[1, 240] */
    AX_S32 picQpDeltaMax;
    SAMPLE_JPEGENC_RCMODE rcMode;
    AX_S32 picQpDeltaMin;
    AX_U32 qpmin;
    AX_U32 qpmax;
    AX_S32 fixedQP;
    AX_U32 exp_of_input_alignment;
    AX_U32 streamBufChain;
    AX_U32 streamMultiSegmentMode;
    AX_U32 streamMultiSegmentAmount;
    AX_CHAR dec400CompTableinput[MAX_PATH];
    // AX_U64 dec400FrameTableSize;

    /* AXI alignment */
    AX_U32 AXIAlignment;

    AX_CHAR olInput[MAX_OVERLAY_NUM][MAX_PATH];
    AX_U32 overlayEnables;
    AX_U32 olFormat[MAX_OVERLAY_NUM];
    AX_U32 olAlpha[MAX_OVERLAY_NUM];
    AX_U32 olWidth[MAX_OVERLAY_NUM];
    AX_U32 olCropWidth[MAX_OVERLAY_NUM];
    AX_U32 olHeight[MAX_OVERLAY_NUM];
    AX_U32 olCropHeight[MAX_OVERLAY_NUM];
    AX_U32 olXoffset[MAX_OVERLAY_NUM];
    AX_U32 olCropXoffset[MAX_OVERLAY_NUM];
    AX_U32 olYoffset[MAX_OVERLAY_NUM];
    AX_U32 olCropYoffset[MAX_OVERLAY_NUM];
    AX_U32 olYStride[MAX_OVERLAY_NUM];
    AX_U32 olUVStride[MAX_OVERLAY_NUM];
    AX_U32 olSuperTile[MAX_OVERLAY_NUM];
    AX_U32 olScaleWidth[MAX_OVERLAY_NUM];
    AX_U32 olScaleHeight[MAX_OVERLAY_NUM];
    AX_CHAR osdDec400CompTableInput[MAX_PATH];

    /* SRAM power down mode disable */
    AX_U32 sramPowerdownDisable;

    AX_S32 useVcmd;
    AX_S32 useMMU;
    AX_S32 useDec400;
    AX_S32 useL2Cache;

    /*AXI max burst length */
    AX_U32 burstMaxLength;

    AX_U32 frameNum;
    // AX_S32 picture_cnt;
    AX_U32 EncChnNum;          /* encoder channel number */
    // AX_S32 syncType;
    AX_COMPRESS_MODE_E eCompressMode;
} SAMPLE_CMD_PARA_T;

typedef struct _SAMPLE_OPTION
{
    AX_CHAR *long_opt;
    AX_S8 short_opt;
    AX_S32 enable;
} SAMPLE_OPTION_T;

typedef struct _SAMPLE_PARAMETER
{
    AX_S32 cnt;
    AX_CHAR *argument;
    AX_S8 short_opt;
    AX_CHAR *long_opt;
    AX_S32 enable;
} SAMPLE_PARAMETER_T;



AX_S32 JencParameterGet(AX_S32 argc, AX_CHAR **argv, SAMPLE_CMD_PARA_T *pCml) ;


AX_S32 JencSetDefaultParameter(SAMPLE_CMD_PARA_T *pstPara);

