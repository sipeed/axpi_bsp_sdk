#include "sample_utils.h"

#include <assert.h>
#include <string.h>
#ifdef __linux
#include <sys/syscall.h>
#endif

#define SAMPLE_NAME "Sample Jpegenc"

#define gettid() syscall(__NR_gettid)

#define SAMPLE_LOG(str, arg...)    \
    do {    \
        printf("tid:%d sample_utils.c %s:%d "str"\n", gettid(), __func__, __LINE__, ##arg); \
    } while(0)

#define SAMPLE_ERR_LOG(str, arg...)    \
    do{     \
        printf(" tid:%d sample_utils.c %s %d Error! "str"\n", gettid(), __func__, __LINE__, ##arg); \
    }while(0)

#define CLIP3(x, y, z)    ((z) < (x) ? (x) : ((z) > (y) ? (y) : (z)))

#ifndef ENCH2_DEFAULT_BURST_LENGTH
#define ENCH2_DEFAULT_BURST_LENGTH             0x10
#endif


static AX_VOID JencHelp(void)
{
    fprintf(stdout, "Usage:  %s [options] -i inputfile\n", "jpeg_testenc");
    fprintf(stdout,
            "  -H    --help              Display this help.\n\n"
           );

    fprintf(stdout,
            " Parameters affecting multi encoder channel:\n"
            "  -j[n] --EncChnNum        1..16, Multi encoder channel. [1]\n"
                );

    fprintf(stdout,
            " Parameters affecting input frame and encoded frame resolutions and cropping:\n"
            "  -i[s] --input             Read input from file. [input.yuv]\n"
            "        --filePath          Read input from file path. [null]\n"
            "                            Source file name should be like 1920x1080_nv12.yuv\n"
            "  -I[s] --inputThumb        Read thumbnail input from file. [thumbnail.jpg]\n"
            "  -o[s] --output            Write output to file. [stream.jpg]\n"
            "  -a[n] --firstPic          First picture of input file. [0]\n"
            "  -b[n] --lastPic           Last picture of input file. [0]\n"

            "  -w[n] --lumWidthSrc       Width of source image. [176]\n"
            "  -h[n] --lumHeightSrc      Height of source image. [144]\n"
        "  -x[n] --output_width             Width of output image. [--lumWidthSrc]\n"
        "  -y[n] --output_height            Height of output image. [--lumHeightSrc]\n"

            "  -X[n] --horOffsetSrc      Output image horizontal offset. [0]\n"
            "  -Y[n] --verOffsetSrc      Output image vertical offset. [0]\n"
            // "  -W[n] --write             0=NO, 1=YES write output. [1]\n"
        "        --chnWidth          Max width of channel. [default 8192]\n"
        "        --chnHeight         Max height of channel. [default 8192]\n"
        "        --strideAlign       Alignment of stride. [64]\n"
        "        --picStride         Stride of planes, set refer to Y:Cb:Cr\n"
                );

    fprintf(stdout,
            "\n Parameters for pre-processing frames before encoding:\n"
            "  -g[n] --enFrameFormat         Input YUV format. [0]\n"
            "                               0 - YUV420 planar CbCr (IYUV/I420)\n"
            "                               1 - YUV420 semi-planar CbCr (NV12)\n"
            "                               2 - YUV420 semi-planar CrCb (NV21)\n"
            "                               3 - YUYV422 interleaved (YUYV/YUY2)\n"
            "                               4 - UYVY422 interleaved (UYVY/Y422)\n"
            "  -v[n] --colorConversion   RGB to YCbCr color conversion type. [0]\n"
            "                               0 - ITU-R BT.601, RGB limited [16...235] (BT601_l.mat)\n"
            "                               1 - ITU-R BT.709, RGB limited [16...235] (BT709_l.mat)\n"
            "                               2 - User defined, coefficients defined in test bench.\n"
            "                               3 - ITU-R BT.2020\n"
            "                               4 - ITU-R BT.601, RGB full [0...255] (BT601_f.mat)\n"
            "                               5 - ITU-R BT.601, RGB limited [0...219] (BT601_219.mat)\n"
            "                               6 - ITU-R BT.709, RGB full [0...255] (BT709_f.mat)\n"
            "  -G[n] --rotation          Rotate input image. [0]\n"
            "                               0 - disabled\n"
            "                               1 - 90 degrees right\n"
            "                               2 - 90 degrees left\n"
            "                               3 - 180 degrees\n"
            "  -M[n]  --mirror           Mirror input image. [0]\n"
            "                               0 - disabled horizontal mirror\n"
            "                               1 - enable horizontal mirror\n"
            "  -Q[n]  --inputAlignmentExp Alignment value of input frame buffer. [4]\n"
            "                               0 = Disable alignment \n"
            "                               4..12 = Base address of input frame buffer and each line are aligned to 2^inputAlignmentExp \n"
        );
    fprintf(stdout,
            "  -d[n] --enableConstChroma   0..1 Enable/Disable setting chroma to a constant pixel value. [0]\n"
            "                                 0 = Disable. \n"
            "                                 1 = Enable. \n"
            "  -e[n] --constCb             0..255. The constant pixel value for Cb. [128]\n"
            "  -f[n] --constCr             0..255. The constant pixel value for Cr. [128]\n"
        );

    fprintf(stdout,
            "\n Parameters affecting the output stream and encoding tools:\n"
            "  -R[n] --restartInterval   Restart interval in MCU rows. [0]\n"
            "  -q[n] --qFactor           1..99, quantization scale. [90]\n"
            "        --qTableEnable      use user set qtable [0]\n"
            "  -p[n] --codingType        0=whole frame, 1=partial frame encoding. [0]\n"
            "  -m[n] --codingMode        0=YUV420, 1=YUV422, 2=Monochrome [0]\n"
            "  -t[n] --markerType        Quantization/Huffman table markers. [0]\n"
            "                               0 = Single marker\n"
            "                               1 = Multiple markers\n"
            "  -u[n] --units             Units type of x- and y-density. [0]\n"
            "                               0 = pixel aspect ratio\n"
            "                               1 = dots/inch\n"
            "                               2 = dots/cm\n"
            "  -k[n] --xdensity          Xdensity to APP0 header. [1]\n"
            "  -l[n] --ydensity          Ydensity to APP0 header. [1]\n"
            "        --streamBufChain    Enable two output stream buffers. [0]\n"
            "                               0 - Single output stream buffer.\n"
            "                               1 - Two output stream buffers chained together.\n"
            "                            Note the minimum allowable size of the first stream buffer is 1k bytes + thumbnail data size if any.\n"

            );
    fprintf(stdout,
            "  -T[n] --thumbnail         0=NO, 1=JPEG, 2=RGB8, 3=RGB24 Thumbnail to stream. [0]\n"
            "  -K[n] --widthThumb        Width of thumbnail output image. [32]\n"
            "  -L[n] --heightThumb       Height of thumbnail output image. [32]\n"
            );

    fprintf(stdout,
            "        --hashtype          Hash type for frame data hash. [0]\n"
            "                                 0=disable, 1=CRC32, 2=checksum32. \n"
         );

    fprintf(stdout,
            "\n Parameters affecting stream multi-segment output:\n"
            "        --streamMultiSegmentMode 0..2 Stream multi-segment mode control. [0]\n"
            "                                 0 = Disable stream multi-segment.\n"
            "                                 1 = Enable. No SW handshaking. Loop-back enabled.\n"
            "                                 2 = Enable. SW handshaking. Loop-back enabled.\n"
            "        --streamMultiSegmentAmount 2..16. the total amount of segments to control loopback/sw-handshake/IRQ. [4]\n"
        );

    fprintf(stdout,
            "\n");
    fprintf(stdout,
            "\n Parameters affecting lossless encoding:\n"
            "        --lossless          0=lossy, 1~7 Enalbe lossless with prediction select mode n [0]\n"
            "        --ptrans            0..7 Point transform value for lossless encoding. [0]\n");
#ifdef TB_DEFINED_COMMENT
    fprintf(stdout,
            "\n   Using comment values defined in testbench!\n");
#else
    fprintf(stdout,
            "  -c[n] --comLength         Comment header data length. [0]\n"
            "  -C[s] --comFile           Comment header data file. [com.txt]\n");
#endif
    fprintf(stdout,
           "\n Parameters affecting the low latency mode:\n"
            "  -S[n] --inputLineBufferMode 0..4. Input buffer mode control (Line-Buffer Mode). [0]\n"
            "                                 0 = Disable input line buffer. \n"
            "                                 1 = Enable. SW handshaking. Loopback enabled.\n"
            "                                 2 = Enable. HW handshaking. Loopback enabled.\n"
            "                                 3 = Enable. SW handshaking. Loopback disabled.\n"
            "                                 4 = Enable. HW handshaking. Loopback disabled.\n"
            "  -N[n] --inputLineBufferDepth 0..511 The number of MCU rows to control loop-back/handshaking [1]\n"
            "                                 Control loop-back mode if it is enabled:\n"
            "                                   There are two continuous ping-pong input buffers; each contains inputLineBufferDepth MCU rows.\n"
            "                                 Control hardware handshaking if it is enabled:\n"
            "                                   Handshaking signal is processed per inputLineBufferDepth CTB/MB rows.\n"
            "                                 Control software handshaking if it is enabled:\n"
            "                                   IRQ is sent and Read Count Register is updated every time inputLineBufferDepth MCU rows have been read.\n"
            "                                 0 is only allowed with inputLineBufferMode = 3, IRQ won't be sent and Read Count Register won't be updated.\n"
            "  -s[n] --inputLineBufferAmountPerLoopback 0..1023. Handshake sync amount for every loopback [0]\n");

    fprintf(stdout,
    "\n Parameters affecting Motion JPEG, rate control, JPEG bitsPerPic:\n"
        "  -J[n] --srcFrameRate             1..240 Input picture rate per second\n"
        "  -B[n] --bitRate      Target bit per second. [0]\n"
            "                               0 - RC OFF\n"
            "                               none zero - RC ON\n"
            "  -n[n] --frameRateNum      1..240 Output picture rate numerator. [30]\n"
            "  -r[n] --frameRateDenom    1..240 Output picture rate denominator. [1]\n"
            "  -V[n] --rcMode            1..3, MJPEG RC mode. [1]\n"
        "                               1 = MJPEG RC with CBR. \n"
        "                               2 = MJPEG RC with VBR. \n"
        "                               3 = MJPEG FixQp mode. \n"
            "  -U[n:m] --picQpDeltaRange Min:Max. Qp Delta range in picture-level rate control.\n"
            "                               Min: -10..-1 Minimum Qp_Delta in picture RC. [-2]\n"
            "                               Max:  1..10  Maximum Qp_Delta in picture RC. [3]\n"
            "                               This range only applies to two neighboring frames.\n"
            "  -E[n] --qpMin             0..51, Minimum frame qp. [0]\n"
            "  -F[n] --qpMax             0..51, Maxmum frame qp. [51]\n"
            "  -O[n] --fixedQP           -1..51, Fixed qp for every frame. [-1]\n"
            "                               -1 = disable fixed qp mode\n"
            "                               0-51 = value of fixed qp.\n"
           );

    fprintf(stdout,
            "\n Parameters for DEC400 compressed table(tile status):\n"
            "  --dec400TableInput            Read input DEC400 compressed table from file. [dec400CompTableinput.bin]\n"
            );
#if 0
    fprintf(stdout,
            "\nTesting parameters that are not supported for end-user:\n"
            "  -P[n] --trigger           Logic Analyzer trigger at picture <n>. [-1]\n"
            "                            -1 = Disable the trigger.\n"
            "  -D[n] --XformCustomerPrivateFormat    -1..4 Convert YUV420 to customer private format. [-1]\n"
            "                               -1 - No conversion to customer private format\n"
            "                               0 - customer private tile format for HEVC\n"
            "                               1 - customer private tile format for H.264\n"
            "                               2 - customer private YUV422_888\n"
            "                               3 - common data 8-bit tile 4x4\n"
            "                               4 - common data 10-bit tile 4x4\n"
            "                               5 - customer private tile format for JPEG\n"
            "\n");
#endif
    fprintf(stdout,
            "\n Parameters for OSD overlay controls (i should be a number from 1 to 8):\n"
            "  --overlayEnables             8 bits indicate enable for 8 overlay region. [0]\n"
            "                                   1: region 1 enabled\n"
            "                                   2: region 2 enabled\n"
            "                                   3: region 1 and 2 enabled\n"
            "                                   and so on.\n"
            "  --olInput[i]                   input file for overlay region i(1-8). [olInput[i].yuv]\n"
            "                                   for example --olInput1\n"
            "  --olFormat[i]                  0..1 Specify the overlay input format. [i]\n"
            "                                   0: ARGB8888\n"
            "                                   1: NV12\n"
            "  --olAlpha[i]                   0..255 Specify a global alpha value for NV12 overlay format. [i]\n"
            "  --olWidth[i]                   Width of overlay region. Must be set if region enabled. [i]\n"
            "  --olHeight[i]                  Height of overlay region. Must be set if region enabled. [i]\n"
            "  --olXoffset[i]                 Horizontal offset of overlay region top left pixel. [i]\n"
            "                                   must be under 2 aligned condition. [i]\n"
            "  --olYoffset[i]                 Vertical offset of overlay region top left pixel. [i]\n"
            "                                   must be under 2 aligned condition. [i]\n"
            "  --olYStride[i]                 Luma stride in bytes. Default value is based on format.\n"
            "                                   [olWidthi * 4] if ARGB888.\n"
            "                                   [olWidthi] if NV12.\n"
            "  --olUVStride[i]                Chroma stride in bytes. Default value is based on luma stride.\n"
            "  --olCropXoffset[i]             OSD cropping top left horizontal offset. [i]\n"
            "                                   must be under 2 aligned condition. [i]\n"
            "  --olCropYoffset[i]             OSD cropping top left vertical offset. [i]\n"
            "                                   must be under 2 aligned condition. [i]\n"
            "  --olCropWidth[i]               OSD cropping width. [olWidthi]\n"
            "  --olCropHeight[i]              OSD cropping height. [olHeighti]\n"
            "\n");
#if 0
    fprintf(stdout,
            "\n Parameters for AXI alignment:\n"
            "  --AXIAlignment               AXI alignment setting (in hexadecimal format). [0]\n"
            "                                   bit[31:28] AXI_burst_align_wr_common\n"
            "                                   bit[27:24] AXI_burst_align_wr_stream\n"
            "                                   bit[23:20] AXI_burst_align_wr_chroma_ref\n"
            "                                   bit[19:16] AXI_burst_align_wr_luma_ref\n"
            "                                   bit[15:12] AXI_burst_align_rd_common\n"
            "                                   bit[11: 8] AXI_burst_align_rd_prp\n"
            "                                   bit[ 7: 4] AXI_burst_align_rd_ch_ref_prefetch\n"
            "                                   bit[ 3: 0] AXI_burst_align_rd_lu_ref_prefetch\n"
            "\n");
        fprintf(stdout,
            "\n Parameters for AXI alignment:\n"
            "  --mmuEnable                  0=disable MMU if MMU exists, 1=enable MMU if MMU exists. [0]\n");
#endif
    fprintf(stdout,
            "\n Parameters for RoiMap:\n"
            "  --roimapfile                 Input file for roimap region. [jpeg_roimap.roi]\n"
            "  --nonRoiFilter               Input file for nonroimap region filter. [filter.txt]\n"
            "  --nonRoiLevel                0...9 nonRoiFliter Level\n"
            );

}

static AX_S32 JencParseDelim(AX_CHAR *optArg, AX_S8 delim)
{
    AX_S32 i;

    for (i = 0; i < (AX_S32)strlen(optArg); i++)
        if (optArg[i] == delim)
        {
            optArg[i] = 0;
            return i;
        }

    return -1;
}

static AX_S32 JencParameterCheck(SAMPLE_CMD_PARA_T * pCmdl)
{
    if (pCmdl->EncChnNum < 1 || pCmdl->EncChnNum > 40) {
        SAMPLE_ERR_LOG("pCmdl->EncChnNum:%d", pCmdl->EncChnNum);
        return -1;
    }

    if ((pCmdl->rcMode <= JPEGENC_RC_NONE) || (pCmdl->rcMode >= JPEGENC_RC_BUTT)) {
        SAMPLE_ERR_LOG("pCmdl->rcMode:%d", pCmdl->rcMode);
        return -1;
    }

    if ((pCmdl->qFactor < 1) || (pCmdl->qFactor > 99)) {
        SAMPLE_ERR_LOG("pCmdl->qFactor:%d", pCmdl->qFactor);
        return -1;
    }

    if ((pCmdl->picQpDeltaMin < -10) || (pCmdl->picQpDeltaMin > -1)) {
        SAMPLE_ERR_LOG("pCmdl->picQpDeltaMin:%d", pCmdl->picQpDeltaMin);
        return -1;
    }

    if ((pCmdl->picQpDeltaMax < 1) || (pCmdl->picQpDeltaMax > 10)) {
        SAMPLE_ERR_LOG("pCmdl->picQpDeltaMax:%d", pCmdl->picQpDeltaMax);
        return -1;
    }

    if ((pCmdl->srcFrameRate < 1) || (pCmdl->srcFrameRate > 240)) {
        SAMPLE_ERR_LOG("pCmdl->srcFrameRate:%d", pCmdl->srcFrameRate);
        return -1;
    }

    if ((pCmdl->frameRateNum < 1) || (pCmdl->frameRateNum > 240)) {
        SAMPLE_ERR_LOG("pCmdl->frameRateNum:%d", pCmdl->frameRateNum);
        return -1;
    }

    if ((pCmdl->frameRateDenom < 1) || (pCmdl->frameRateDenom > 240)) {
        SAMPLE_ERR_LOG("pCmdl->frameRateDenom:%d", pCmdl->frameRateDenom);
        return -1;
    }

    if ((pCmdl->frameRateNum / pCmdl->frameRateDenom) > pCmdl->srcFrameRate) {
        SAMPLE_ERR_LOG("pCmdl->frameRateNum:%d / pCmdl->frameRateDenom:%d = %d > pCmdl->srcFrameRate:%d",
            pCmdl->frameRateNum, pCmdl->frameRateDenom, pCmdl->frameRateNum / pCmdl->frameRateDenom, pCmdl->srcFrameRate);
        return -1;
    }

    if ((pCmdl->horOffsetSrc & (1)) != 0 ||
        (pCmdl->verOffsetSrc & (1)) != 0) {
        SAMPLE_ERR_LOG("pCmdl->horOffsetSrc:%d or pCmdl->verOffsetSrc:%d must be even.\n",
                        pCmdl->horOffsetSrc, pCmdl->verOffsetSrc);
        return -2;
    }

    if (pCmdl->horOffsetSrc > pCmdl->lumWidthSrc) {
        SAMPLE_ERR_LOG("pCmdl->horOffsetSrc:%d > pCmdl->verOffsetSrc:%d unsupport.\n",
                        pCmdl->horOffsetSrc, pCmdl->lumWidthSrc);
        return -2;
    }

    if (pCmdl->verOffsetSrc > pCmdl->lumHeightSrc) {
        SAMPLE_ERR_LOG("pCmdl->verOffsetSrc:%d > pCmdl->lumHeightSrc:%d unsupport.\n",
                        pCmdl->verOffsetSrc, pCmdl->lumHeightSrc);
        return -2;
    }

    if ((pCmdl->output_width != INVALID_DEFAULT) || (pCmdl->output_height != INVALID_DEFAULT)) {
        if (pCmdl->output_width <= 0) {
            SAMPLE_ERR_LOG("pCmdl->output_width:%d must > 0.\n", pCmdl->output_width);
            return -2;
        }

        if (pCmdl->output_width > pCmdl->lumWidthSrc) {
            SAMPLE_ERR_LOG("pCmdl->output_width:%d > pCmdl->lumWidthSrc:%d not support.\n",
                pCmdl->output_width, pCmdl->lumWidthSrc);
            return -2;
        }

        if (pCmdl->output_height <= 0) {
            SAMPLE_ERR_LOG("pCmdl->output_height:%d must > 0.\n", pCmdl->output_height);
            return -2;
        }

        if (pCmdl->output_height > pCmdl->lumHeightSrc) {
            SAMPLE_ERR_LOG("pCmdl->output_height:%d > pCmdl->lumHeightSrc:%d not support.\n",
                pCmdl->output_height, pCmdl->lumHeightSrc);
            return -2;
        }
    }

    return 0;
}

static SAMPLE_OPTION_T options[] =
{
    {"hashtype", 'A', 1},  /* hash frame data, 0--disable, 1--crc32, 2--checksum */
    {"firstPic", 'a', 1},
    {"lastPic", 'b', 1},
    {"bitRate", 'B', 1},
    {"comLength", 'c', 1},
    {"comFile", 'C', 1},
    {"XformCustomerPrivateFormat", 'D', 1},
    {"enableConstChroma", 'd', 1},
    {"constCb", 'e', 1},
    {"constCr", 'f', 1},
    {"qpMin", 'E', 1},
    {"qpMax", 'F', 1},
    {"enFrameFormat", 'g', 1},
    {"rotation", 'G', 1},
    {"help", 'H', 0},
    {"input", 'i', 1},
    {"inputThumb", 'I', 1},
    {"EncChnNum", 'j', 1},
    {"srcFrameRate", 'J', 1},
    {"xdensity", 'k', 1},
    {"ydensity", 'l', 1},
    {"widthThumb", 'K', 1},
    {"heightThumb", 'L', 1},
    {"mirror", 'M', 1},
    {"codingType", 'p', 1},
    {"codingMode", 'm', 1},
    {"trigger", 'P', 1},
    {"output", 'o', 1},
    {"fixedQP", 'O', 1},
    {"picQpDeltaRange", 'U', 1 },
    {"units", 'u', 1},
    {"colorConversion", 'v', 1},
    {"rcMode", 'V', 1 },
    {"inputLineBufferMode", 'S', 1},
    {"inputLineBufferDepth", 'N', 1},
    {"frameRateNum", 'n', 1},
    {"frameRateDenom", 'r', 1},
    {"restartInterval", 'R', 1},
    {"inputLineBufferAmountPerLoopback", 's', 1},
    {"inputAlignmentExp", 'Q', 1},
    {"qFactor", 'q', 1},
    {"qTableEnable", '0', 1},
    {"thumbnail", 'T', 1},
    {"markerType", 't', 1},
    {"lumWidthSrc", 'w', 1},
    {"lumHeightSrc", 'h', 1},
    {"chnWidth", '0', 1},
    {"chnHeight", '0', 1},
    // {"write", 'W', 1},
    {"output_width", 'x', 1},
    {"output_height", 'y', 1},
    {"strideAlign", '0', 1},
    {"picStride", '0', 1},
    {"horOffsetSrc", 'X', 1},
    {"verOffsetSrc", 'Y', 1},
    {"frameNum", 'Z', 1},
    {"eCompressMode", 'z', 2},

    {"lossless", '1', 1},
    {"ptrans", '2', 1},

    {"filePath", '0', 1},
    {"roimapFile", '0', 1},
    {"nonRoiFilter", '0', 1},
    {"nonRoiLevel", '0', 1},
    {"streamBufChain", '0', 1},
    {"streamMultiSegmentMode", '0', 1},
    {"streamMultiSegmentAmount", '0', 1},
    {"qTableFile", '0', 1},
    {"dec400TableInput", '0', 1},
    {"osdDec400TableInput", '0', 1},
    {"overlayEnables", '0', 1},
    {"olInput1", '0', 1},
    {"olFormat1", '0', 1},
    {"olAlpha1", '0', 1},
    {"olWidth1", '0', 1},
    {"olCropWidth1", '0', 1},
    {"olHeight1", '0', 1},
    {"olCropHeight1", '0', 1},
    {"olXoffset1", '0', 1},
    {"olCropXoffset1", '0', 1},
    {"olYoffset1", '0', 1},
    {"olCropYoffset1", '0', 1},
    {"olYStride1", '0', 1},
    {"olUVStride1", '0', 1},
    {"olSuperTile1", '0', 1},
    {"olScaleWidth1", '0', 1},
    {"olScaleHeight1", '0', 1},
    // FBDC_ENABLE
    {"UVheaderSize", '0', 1},
    {"UVpayloadSize", '0', 1},
    {"YheaderSize", '0', 1},
    {"YpayloadSize", '0', 1},
    {"CropX", '0', 1},
    {"CropY", '0', 1},

    {"olInput2", '0', 1},
    {"olFormat2", '0', 1},
    {"olAlpha2", '0', 1},
    {"olWidth2", '0', 1},
    {"olCropWidth2", '0', 1},
    {"olHeight2", '0', 1},
    {"olCropHeight2", '0', 1},
    {"olXoffset2", '0', 1},
    {"olCropXoffset2", '0', 1},
    {"olYoffset2", '0', 1},
    {"olCropYoffset2", '0', 1},
    {"olYStride2", '0', 1},
    {"olUVStride2", '0', 1},
    {"olInput3", '0', 1},
    {"olFormat3", '0', 1},
    {"olAlpha3", '0', 1},
    {"olWidth3", '0', 1},
    {"olCropWidth3", '0', 1},
    {"olHeight3", '0', 1},
    {"olCropHeight3", '0', 1},
    {"olXoffset3", '0', 1},
    {"olCropXoffset3", '0', 1},
    {"olYoffset3", '0', 1},
    {"olCropYoffset3", '0', 1},
    {"olYStride3", '0', 1},
    {"olUVStride3", '0', 1},
    {"olInput4", '0', 1},
    {"olFormat4", '0', 1},
    {"olAlpha4", '0', 1},
    {"olWidth4", '0', 1},
    {"olCropWidth4", '0', 1},
    {"olHeight4", '0', 1},
    {"olCropHeight4", '0', 1},
    {"olXoffset4", '0', 1},
    {"olCropXoffset4", '0', 1},
    {"olYoffset4", '0', 1},
    {"olCropYoffset4", '0', 1},
    {"olYStride4", '0', 1},
    {"olUVStride4", '0', 1},
    {"olInput5", '0', 1},
    {"olFormat5", '0', 1},
    {"olAlpha5", '0', 1},
    {"olWidth5", '0', 1},
    {"olCropWidth5", '0', 1},
    {"olHeight5", '0', 1},
    {"olCropHeight5", '0', 1},
    {"olXoffset5", '0', 1},
    {"olCropXoffset5", '0', 1},
    {"olYoffset5", '0', 1},
    {"olCropYoffset5", '0', 1},
    {"olYStride5", '0', 1},
    {"olUVStride5", '0', 1},
    {"olInput6", '0', 1},
    {"olFormat6", '0', 1},
    {"olAlpha6", '0', 1},
    {"olWidth6", '0', 1},
    {"olCropWidth6", '0', 1},
    {"olHeight6", '0', 1},
    {"olCropHeight6", '0', 1},
    {"olXoffset6", '0', 1},
    {"olCropXoffset6", '0', 1},
    {"olYoffset6", '0', 1},
    {"olCropYoffset6", '0', 1},
    {"olYStride6", '0', 1},
    {"olUVStride6", '0', 1},
    {"olInput7", '0', 1},
    {"olFormat7", '0', 1},
    {"olAlpha7", '0', 1},
    {"olWidth7", '0', 1},
    {"olCropWidth7", '0', 1},
    {"olHeight7", '0', 1},
    {"olCropHeight7", '0', 1},
    {"olXoffset7", '0', 1},
    {"olCropXoffset7", '0', 1},
    {"olYoffset7", '0', 1},
    {"olCropYoffset7", '0', 1},
    {"olYStride7", '0', 1},
    {"olUVStride7", '0', 1},
    {"olInput8", '0', 1},
    {"olFormat8", '0', 1},
    {"olAlpha8", '0', 1},
    {"olWidth8", '0', 1},
    {"olCropWidth8", '0', 1},
    {"olHeight8", '0', 1},
    {"olCropHeight8", '0', 1},
    {"olXoffset8", '0', 1},
    {"olCropXoffset8", '0', 1},
    {"olYoffset8", '0', 1},
    {"olCropYoffset8", '0', 1},
    {"olYStride8", '0', 1},
    {"olUVStride8", '0', 1},
    {"AXIAlignment", '0', 1},

    {"useVcmd", '3', 1},
    {"useMMU", '3', 1},
    {"useDec400", '3', 1},
    {"useL2Cache", '3', 1},

    {"sramPowerdownDisable", '0', 1},
    // {"mmuEnable", '0', 1},
    /*AXI max burst length */
    {"burstMaxLength", '0', 1},
    {NULL, 0, 0}
};


static AX_S32 JencGetNext(AX_S32 argc, AX_CHAR **argv, SAMPLE_PARAMETER_T *parameter, AX_CHAR **p)
{
    /* End of options */
    if ((parameter->cnt >= argc) || (parameter->cnt < 0)) {
        return -1;
    }
    *p = argv[parameter->cnt];
    parameter->cnt++;

    return 0;
}

static AX_S32 JencParse(AX_S32 argc, AX_CHAR **argv, SAMPLE_OPTION_T *option,
                    SAMPLE_PARAMETER_T *parameter, AX_CHAR **p, AX_U32 lenght)
{
    AX_CHAR *arg;

    parameter->short_opt = option->short_opt;
    parameter->long_opt = option->long_opt;
    arg = *p + lenght;

    /* Argument and option are together */
    if (strlen(arg) != 0) {
        /* There should be no argument */
        if (option->enable == 0) {
            return -1;
        }

        /* Remove = */
        if (strncmp("=", arg, 1) == 0) {
            arg++;
        }
        parameter->enable = 1;
        parameter->argument = arg;
        return 0;
    }

    /* Argument and option are separately */
    if (JencGetNext(argc, argv, parameter, p)) {
        /* There is no more parameters */
        if (option->enable == 1) {
            return -1;
        }
        return 0;
    }

    /* Parameter is missing if next start with "-" but next time this
     * option is OK so we must fix parameter->cnt */
    if (strncmp("-", *p,    1) == 0) {
        parameter->cnt--;
        if (option->enable == 1) {
            return -1;
        }
        return 0;
    }

    /* There should be no argument */
    if (option->enable == 0) {
        return -1;
    }

    parameter->enable = 1;
    parameter->argument = *p;

    return 0;
}

static AX_S32 JencShortOption(AX_S32 argc, AX_CHAR **argv, SAMPLE_OPTION_T *option,
                                SAMPLE_PARAMETER_T *parameter, AX_CHAR **p)
{
    AX_S32 i = 0;
    AX_S8 short_opt;

    if (strncmp("-", *p, 1) != 0) {
        return 1;
    }

    //strncpy(&short_opt, *p + 1, 1);
    short_opt = *(*p + 1);
    parameter->short_opt = short_opt;
    while (option[i].long_opt != NULL) {
        if (option[i].short_opt == short_opt) {
            goto match;
        }
        i++;
    }
    return 1;

match:
    if (JencParse(argc, argv, &option[i], parameter, p, 2) != 0) {
        return -2;
    }

    return 0;
}

static AX_S32 JencLongOption(AX_S32 argc, AX_CHAR **argv, SAMPLE_OPTION_T *option,
                                SAMPLE_PARAMETER_T *parameter, AX_CHAR **p)
{
    AX_S32 i = 0;
    AX_U32 lenght;
    // AX_U32 pLength;

    if (strncmp("--", *p, 2) != 0) {
        return 1;
    }

    // pLength = strlen(*p+2);
    while (option[i].long_opt != NULL) {
        lenght = strlen(option[i].long_opt);
        if (strncmp(option[i].long_opt, *p + 2, lenght) == 0) {
            goto match;
        }
        i++;
    }
    return 1;

match:
    lenght += 2;        /* Because option start -- */
    if (JencParse(argc, argv, &option[i], parameter, p, lenght) != 0) {
        return -2;
    }

    return 0;
}

static AX_S32 JencGetOption(AX_S32 argc, AX_CHAR **argv, SAMPLE_OPTION_T *option, SAMPLE_PARAMETER_T * parameter)
{
    AX_CHAR *p = NULL;
    AX_S32 ret;

    parameter->argument = "?";
    parameter->short_opt = '?';
    parameter->long_opt = "?";
    parameter->enable = 0;

    if (JencGetNext(argc, argv, parameter, &p)) {
        return -1;    /* End of options */
    }

    /* Long option */
    ret = JencLongOption(argc, argv, option, parameter, &p);
    if (ret != 1) {
        return ret;
    }

    /* Short option */
    ret = JencShortOption(argc, argv, option, parameter, &p);
    if (ret != 1) {
        return ret;
    }

    /* This is unknow option but option anyway so argument must return */
    parameter->argument = p;

    return 1;
}

AX_S32 JencSetDefaultParameter(SAMPLE_CMD_PARA_T *pstPara)
{
    SAMPLE_CMD_PARA_T * cml = pstPara;
    AX_S32 i;

    memset(cml, 0, sizeof(SAMPLE_CMD_PARA_T));

    strcpy(cml->input, "input.yuv");
    strcpy(cml->inputThumb, "thumbnail.jpg");
    strcpy(cml->com, "com.txt");
    strcpy(cml->output, "stream.jpg");
    strcpy(cml->qTablePath, "");
    cml->inputPath = NULL;
    cml->useVcmd = -1;
    cml->roimapFile = NULL;
    cml->nonRoiFilter = NULL;
    cml->nonRoiLevel = 10;
    cml->firstPic = 0;
    cml->lastPic = 0;
    cml->maxWidth = MAX_WIDTH_DEFAULT;
    cml->maxHeight = MAX_HEIGHT_DEFAULT;
    cml->lumWidthSrc = SRC_WIDTH_DEFAULT;
    cml->lumHeightSrc = SRC_HEIGHT_DEFAULT;
    cml->output_width = INVALID_DEFAULT;
    cml->output_height = INVALID_DEFAULT;
    cml->strideAlign = 64;
    cml->picStride[0] = 0;
    cml->picStride[1] = 0;
    cml->picStride[2] = 0;
    cml->horOffsetSrc = 0;
    cml->verOffsetSrc = 0;
    cml->qFactor = 90;
    cml->restartInterval = 0;
    cml->thumbnail = 0;
    cml->widthThumb = 32;
    cml->heightThumb = 32;
    cml->enFrameFormat = 0;
    cml->colorConversion = 0;
    cml->rotation = 0;
    cml->partialCoding = 0;
    cml->codingMode = 0;
    cml->markerType = 0;
    cml->unitsType = 0;
    cml->xdensity = 1;
    cml->ydensity = 1;
    cml->comLength = 0;
    cml->inputLineBufMode = 0;
    cml->inputLineBufDepth = 1;
    cml->amountPerLoopBack = 0;
    cml->hashtype = 0;
    cml->mirror = 0;
    cml->formatCustomizedType = -1;
    cml->constChromaEn = 0;
    cml->constCb = 0x80;
    cml->constCr = 0x80;
    cml->predictMode = 0;
    cml->ptransValue = 0;
    cml->bitRate = 2000;//kbps
    cml->srcFrameRate = 2;
    cml->frameRateNum = 2;
    cml->frameRateDenom = 1;
    cml->rcMode = JPEGENC_CBR;
    cml->picQpDeltaMin = -2;
    cml->picQpDeltaMax = 3;
    cml->qpmin = 22;
    cml->qpmax = 51;
    cml->fixedQP = -1;
    cml->exp_of_input_alignment = 4;
    cml->streamBufChain = 0;
    cml->streamMultiSegmentMode = 0;
    cml->streamMultiSegmentAmount = 4;
    strcpy(cml->dec400CompTableinput, "dec400CompTableinput.bin");
    cml->AXIAlignment = 0;
    // cml->mmuEnable = 0;
    /*Overlay*/
    cml->overlayEnables = 0;
    strcpy(cml->osdDec400CompTableInput, "osdDec400CompTableinput.bin");

    for(i = 0; i < MAX_OVERLAY_NUM; i++) {
        strcpy(cml->olInput[i], "olInput.yuv");
        cml->olFormat[i] = 0;
        cml->olAlpha[i] = 0;
        cml->olWidth[i] = 0;
        cml->olHeight[i] = 0;
        cml->olXoffset[i] = 0;
        cml->olYoffset[i] = 0;
        cml->olYStride[i] = 0;
        cml->olUVStride[i] = 0;
        cml->olSuperTile[i] = 0;
        cml->olScaleWidth[i] = 0;
        cml->olScaleHeight[i] = 0;
    }

    cml->sramPowerdownDisable = 0;
    cml->burstMaxLength = ENCH2_DEFAULT_BURST_LENGTH;

    cml->EncChnNum = 1;
    return 0;
}


static AX_S32 JencCheckLongOption0(SAMPLE_PARAMETER_T *prm, SAMPLE_CMD_PARA_T *cml, AX_CHAR *optarg)
{
    SAMPLE_PARAMETER_T *pPrm = prm;
    AX_S32 ret = 0;
    AX_S32 status = 0;
    AX_S32 i;

    do {
        /* Check long option */
        if (strcmp(pPrm->long_opt, "streamBufChain") == 0) {
            cml->streamBufChain = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "streamMultiSegmentMode") == 0) {
            cml->streamMultiSegmentMode = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "streamMultiSegmentAmount") == 0) {
            cml->streamMultiSegmentAmount = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "qTableFile") == 0) {
            strcpy(cml->qTablePath, optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "dec400TableInput") == 0) {
            strcpy(cml->dec400CompTableinput, optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "overlayEnables") == 0) {
            cml->overlayEnables = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "osdDec400TableInput") == 0)
            strcpy(cml->osdDec400CompTableInput, optarg);

        if (strcmp(pPrm->long_opt, "olInput1") == 0) {
            if(strlen(optarg) < MAX_PATH) {
                strcpy(cml->olInput[0], optarg);
            } else {
                SAMPLE_ERR_LOG("strlen(optarg):%d >= MAX_PATH:%d", strlen(optarg), MAX_PATH);
                status = -1;
            }
            break;
        }

        if (strcmp(pPrm->long_opt, "olFormat1") == 0) {
            cml->olFormat[0] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olAlpha1") == 0) {
            cml->olAlpha[0] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olWidth1") == 0) {
            cml->olWidth[0] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropWidth1") == 0) {
            cml->olCropWidth[0] = atoi(optarg);
            break;
        }


        if (strcmp(pPrm->long_opt, "olHeight1") == 0) {
            cml->olHeight[0] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropHeight1") == 0) {
            cml->olCropHeight[0] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olXoffset1") == 0) {
            cml->olXoffset[0] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropXoffset1") == 0) {
            cml->olCropXoffset[0] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olYoffset1") == 0) {
            cml->olYoffset[0] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropYoffset1") == 0) {
            cml->olCropYoffset[0] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olYStride1") == 0) {
            cml->olYStride[0] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olUVStride1") == 0) {
            cml->olUVStride[0] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olSuperTile1") == 0) {
            cml->olSuperTile[0] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olScaleWidth1") == 0) {
            cml->olScaleWidth[0] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olScaleHeight1") == 0) {
            cml->olScaleHeight[0] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olInput2") == 0) {
            if(strlen(optarg) < MAX_PATH) {
                strcpy(cml->olInput[1], optarg);
            } else {
                SAMPLE_ERR_LOG("strlen(optarg):%d >= MAX_PATH:%d", strlen(optarg), MAX_PATH);
                status = -1;
            }
            break;
        }

        if (strcmp(pPrm->long_opt, "olFormat2") == 0) {
            cml->olFormat[1] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olAlpha2") == 0) {
            cml->olAlpha[1] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olWidth2") == 0) {
            cml->olWidth[1] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropWidth2") == 0) {
            cml->olCropWidth[1] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olHeight2") == 0) {
            cml->olHeight[1] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropHeight2") == 0) {
            cml->olCropHeight[1] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olXoffset2") == 0) {
            cml->olXoffset[1] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropXoffset2") == 0) {
            cml->olCropXoffset[1] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olYoffset2") == 0) {
            cml->olYoffset[1] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropYoffset2") == 0) {
            cml->olCropYoffset[1] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olYStride2") == 0) {
            cml->olYStride[1] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olUVStride2") == 0) {
            cml->olUVStride[1] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olInput3") == 0) {
            if(strlen(optarg) < MAX_PATH) {
                strcpy(cml->olInput[2], optarg);
            } else {
                SAMPLE_ERR_LOG("strlen(optarg):%d >= MAX_PATH:%d", strlen(optarg), MAX_PATH);
                status = -1;
            }
            break;
        }

        if (strcmp(pPrm->long_opt, "olFormat3") == 0) {
            cml->olFormat[2] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olAlpha3") == 0) {
            cml->olAlpha[2] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olWidth3") == 0) {
            cml->olWidth[2] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropWidth3") == 0) {
            cml->olCropWidth[2] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olHeight3") == 0) {
            cml->olHeight[2] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropHeight3") == 0) {
            cml->olCropHeight[2] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olXoffset3") == 0) {
            cml->olXoffset[2] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropXoffset3") == 0) {
            cml->olCropXoffset[2] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olYoffset3") == 0) {
            cml->olYoffset[2] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropYoffset3") == 0) {
            cml->olCropYoffset[2] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olYStride3") == 0) {
            cml->olYStride[2] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olUVStride3") == 0) {
            cml->olUVStride[2] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olInput4") == 0) {
            if(strlen(optarg) < MAX_PATH) {
                strcpy(cml->olInput[3], optarg);
            } else {
                SAMPLE_ERR_LOG("strlen(optarg):%d >= MAX_PATH:%d", strlen(optarg), MAX_PATH);
                status = -1;
            }
            break;
        }

        if (strcmp(pPrm->long_opt, "olFormat4") == 0) {
            cml->olFormat[3] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olAlpha4") == 0) {
            cml->olAlpha[3] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olWidth4") == 0) {
            cml->olWidth[3] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropWidth4") == 0) {
            cml->olCropWidth[3] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olHeight4") == 0) {
            cml->olHeight[3] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropHeight4") == 0) {
            cml->olCropHeight[3] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olXoffset4") == 0) {
            cml->olXoffset[3] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropXoffset4") == 0) {
            cml->olCropXoffset[3] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olYoffset4") == 0) {
            cml->olYoffset[3] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropYoffset4") == 0) {
            cml->olCropYoffset[3] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olYStride4") == 0) {
            cml->olYStride[3] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olUVStride4") == 0) {
            cml->olUVStride[3] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olInput5") == 0) {
            if(strlen(optarg) < MAX_PATH) {
                strcpy(cml->olInput[4], optarg);
            } else {
                SAMPLE_ERR_LOG("strlen(optarg):%d >= MAX_PATH:%d", strlen(optarg), MAX_PATH);
                status = -1;
            }
            break;
        }

        if (strcmp(pPrm->long_opt, "olFormat5") == 0) {
            cml->olFormat[4] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olAlpha5") == 0) {
            cml->olAlpha[4] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olWidth5") == 0) {
            cml->olWidth[4] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropWidth5") == 0) {
            cml->olCropWidth[4] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olHeight5") == 0) {
            cml->olHeight[4] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropHeight5") == 0) {
            cml->olCropHeight[4] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olXoffset5") == 0) {
            cml->olXoffset[4] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropXoffset5") == 0) {
            cml->olCropXoffset[4] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olYoffset5") == 0) {
            cml->olYoffset[4] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropYoffset5") == 0) {
            cml->olCropYoffset[4] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olYStride5") == 0) {
            cml->olYStride[4] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olUVStride5") == 0) {
            cml->olUVStride[4] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olInput6") == 0) {
            if(strlen(optarg) < MAX_PATH) {
                strcpy(cml->olInput[5], optarg);
            } else {
                SAMPLE_ERR_LOG("strlen(optarg):%d >= MAX_PATH:%d", strlen(optarg), MAX_PATH);
                status = -1;
            }
            break;
        }

        if (strcmp(pPrm->long_opt, "olFormat6") == 0) {
            cml->olFormat[5] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olAlpha6") == 0) {
            cml->olAlpha[5] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olWidth6") == 0) {
            cml->olWidth[5] = atoi(optarg);
            break;
        }
        if (strcmp(pPrm->long_opt, "olCropWidth6") == 0) {
            cml->olCropWidth[5] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olHeight6") == 0) {
            cml->olHeight[5] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropHeight6") == 0) {
            cml->olCropHeight[5] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olXoffset6") == 0) {
            cml->olXoffset[5] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropXoffset6") == 0) {
            cml->olCropXoffset[5] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olYoffset6") == 0) {
            cml->olYoffset[5] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropYoffset6") == 0) {
            cml->olCropYoffset[5] = atoi(optarg);
            break;
        }
        if (strcmp(pPrm->long_opt, "olYStride6") == 0) {
            cml->olYStride[5] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olUVStride6") == 0) {
            cml->olUVStride[5] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olInput7") == 0) {
            if(strlen(optarg) < MAX_PATH) {
                strcpy(cml->olInput[6], optarg);
            } else {
                SAMPLE_ERR_LOG("strlen(optarg):%d >= MAX_PATH:%d", strlen(optarg), MAX_PATH);
                status = -1;
            }
            break;
        }

        if (strcmp(pPrm->long_opt, "olFormat7") == 0) {
            cml->olFormat[6] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olAlpha7") == 0) {
            cml->olAlpha[6] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olWidth7") == 0) {
            cml->olWidth[6] = atoi(optarg);
            break;
        }
        if (strcmp(pPrm->long_opt, "olCropWidth7") == 0) {
            cml->olCropWidth[6] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olHeight7") == 0) {
            cml->olHeight[6] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropHeight7") == 0) {
            cml->olCropHeight[6] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olXoffset7") == 0) {
            cml->olXoffset[6] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropXoffset7") == 0) {
            cml->olCropXoffset[6] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olYoffset7") == 0) {
            cml->olYoffset[6] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropYoffset7") == 0) {
            cml->olCropYoffset[6] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olYStride7") == 0) {
            cml->olYStride[6] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olUVStride7") == 0) {
            cml->olUVStride[6] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olInput8") == 0) {
            if(strlen(optarg) < MAX_PATH) {
                strcpy(cml->olInput[7], optarg);
            } else {
                SAMPLE_ERR_LOG("strlen(optarg):%d >= MAX_PATH:%d", strlen(optarg), MAX_PATH);
                status = -1;
            }
            break;
        }

        if (strcmp(pPrm->long_opt, "olFormat8") == 0) {
            cml->olFormat[7] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olAlpha8") == 0) {
            cml->olAlpha[7] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olWidth8") == 0) {
            cml->olWidth[7] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropWidth8") == 0) {
            cml->olCropWidth[7] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olHeight8") == 0) {
            cml->olHeight[7] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropHeight8") == 0) {
            cml->olCropHeight[7] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olXoffset8") == 0) {
            cml->olXoffset[7] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropXoffset8") == 0) {
            cml->olCropXoffset[7] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olYoffset8") == 0) {
            cml->olYoffset[7] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olCropYoffset8") == 0) {
            cml->olCropYoffset[7] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olYStride8") == 0) {
            cml->olYStride[7] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "olUVStride8") == 0) {
            cml->olUVStride[7] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "AXIAlignment") == 0) {
            cml->AXIAlignment = strtoul(optarg, NULL, 16);
            break;
        }

        if (strcmp(pPrm->long_opt, "roimapFile") == 0) {
            cml->roimapFile = optarg;
            break;
        }

        if (strcmp(pPrm->long_opt, "nonRoiFilter") == 0) {
            cml->nonRoiFilter = optarg;
            break;
        }

        if (strcmp(pPrm->long_opt, "nonRoiLevel") == 0) {
            cml->nonRoiLevel = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "sramPowerdownDisable") == 0) {
            cml->sramPowerdownDisable = atoi(optarg);
            break;
        }

        // if (strcmp(pPrm->long_opt, "mmuEnable") == 0) {
        //     cml->mmuEnable = atoi(optarg);
        //     break;
        // }

        // FBDC_ENABLE
        if (strcmp(pPrm->long_opt, "UVheaderSize") == 0) {
            cml->UVheaderSize = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "UVpayloadSize") == 0) {
            cml->UVpayloadSize = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "YheaderSize") == 0) {
            cml->YheaderSize = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "YpayloadSize") == 0) {
            cml->YpayloadSize = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "CropX") == 0) {
            cml->CropX = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "CropY") == 0) {
            cml->CropY = atoi(optarg);
            break;
        }

        /*AXI max burst length */
        if (strcmp(pPrm->long_opt, "burstMaxLength") == 0) {
            cml->burstMaxLength = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "chnWidth") == 0) {
            cml->maxWidth = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "chnHeight") == 0) {
            cml->maxHeight = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "strideAlign") == 0) {
            cml->strideAlign = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "qTableEnable") == 0) {
            cml->qTableEnable = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "picStride") == 0) {
            /* Argument must be "xx:yy:XX".
             * xx is picStride[0], Y stride */
            if ((i = JencParseDelim(optarg, ':')) == -1) break;
            cml->picStride[0] = atoi(optarg);
            /* yy is picStride[1], Cb stride */
            optarg += i + 1;
            if ((i = JencParseDelim(optarg, ':')) == -1) break;
            cml->picStride[1] = atoi(optarg);
            /* yy is picStride[2], Cr stride */
            optarg += i + 1;
            if ((i = JencParseDelim(optarg, ':')) == -1) break;
            cml->picStride[2] = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "filePath") == 0) {
            cml->inputPath = optarg;
            break;
        }

        ret = -1;
    } while(0);

    if (status) {
        ret = -1;
    }

    return ret;
}


static AX_S32 JencCheckLongOption3(SAMPLE_PARAMETER_T *prm, SAMPLE_CMD_PARA_T *cml, AX_CHAR *optarg)
{
    SAMPLE_PARAMETER_T *pPrm = prm;
    AX_S32 ret = 0;

    do {
        if (strcmp(pPrm->long_opt, "useVcmd") == 0) {
            cml->useVcmd = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "useMMU") == 0) {
            cml->useMMU = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "useDec400") == 0) {
            cml->useDec400 = atoi(optarg);
            break;
        }

        if (strcmp(pPrm->long_opt, "useL2Cache") == 0) {
            cml->useL2Cache = atoi(optarg);
            break;
        }
        ret = -1;
        SAMPLE_ERR_LOG("pPrm->long_opt:%s", pPrm->long_opt);
    } while(0);

    return ret;
}

AX_S32 JencParameterGet(AX_S32 argc, AX_CHAR **argv, SAMPLE_CMD_PARA_T *pstPara)
{
    SAMPLE_PARAMETER_T prm;
    AX_S32 status = 0;
    AX_S32 ret, i;
    AX_CHAR * optarg;
    SAMPLE_CMD_PARA_T *pCmdl = pstPara;

    prm.cnt = 1;

    if (argc < 2) {
        JencHelp();
        exit(0);
    }

    while ((ret = JencGetOption(argc, argv, options, &prm)) != -1) {
        if (ret == -2) {
            SAMPLE_ERR_LOG("ret == -2, prm.cnt:%d", prm.cnt);
            status = -1;
            goto ERR_RET;
        }

        optarg = prm.argument;
        switch (prm.short_opt) {
        case 'H':
            JencHelp();
            exit(0);
        case 'i':
            if (strlen(optarg) < MAX_PATH) {
                strcpy(pCmdl->input, optarg);
            } else {
                SAMPLE_ERR_LOG("strlen(optarg):%d >= MAX_PATH:%d", strlen(optarg), MAX_PATH);
                status = -1;
            }
            break;
        case 'I':
            if (strlen(optarg) < MAX_PATH) {
                strcpy(pCmdl->inputThumb, optarg);
            } else {
                SAMPLE_ERR_LOG("strlen(optarg):%d >= MAX_PATH:%d", strlen(optarg), MAX_PATH);
                status = -1;
            }
            break;
        case 'o':
            if (strlen(optarg) < MAX_PATH) {
                strcpy(pCmdl->output, optarg);
            } else {
                SAMPLE_ERR_LOG("strlen(optarg):%d >= MAX_PATH:%d", strlen(optarg), MAX_PATH);
                status = -1;
            }
            break;
        case 'C':
            if (strlen(optarg) < MAX_PATH) {
                strcpy(pCmdl->com, optarg);
            } else {
                SAMPLE_ERR_LOG("strlen(optarg):%d >= MAX_PATH:%d", strlen(optarg), MAX_PATH);
                status = -1;
            }
            break;
        case 'a':
            pCmdl->firstPic = atoi(optarg);
            break;
        case 'b':
            pCmdl->lastPic = atoi(optarg);
            break;
        case 'x':
            pCmdl->output_width = atoi(optarg);
            break;
        case 'y':
            pCmdl->output_height = atoi(optarg);
            break;
        case 'w':
            pCmdl->lumWidthSrc = atoi(optarg);
            break;
        case 'h':
            pCmdl->lumHeightSrc = atoi(optarg);
            break;
        case 'X':
            pCmdl->horOffsetSrc = atoi(optarg);
            break;
        case 'Y':
            pCmdl->verOffsetSrc = atoi(optarg);
            break;
        case 'R':
            pCmdl->restartInterval = atoi(optarg);
            break;
        case 'q':
            pCmdl->qFactor = atoi(optarg);
            break;
        case 'g':
            pCmdl->enFrameFormat = atoi(optarg);
            break;
        case 'v':
            pCmdl->colorConversion = atoi(optarg);
            break;
        case 'G':
            pCmdl->rotation = atoi(optarg);
            break;
        case 'p':
            pCmdl->partialCoding = atoi(optarg);
            break;
        case 'm':
            pCmdl->codingMode = atoi(optarg);
            break;
        case 't':
            pCmdl->markerType = atoi(optarg);
            break;
        case 'u':
            pCmdl->unitsType = atoi(optarg);
            break;
        case 'k':
            pCmdl->xdensity = atoi(optarg);
            break;
        case 'l':
            pCmdl->ydensity = atoi(optarg);
            break;
        case 'T':
            pCmdl->thumbnail = atoi(optarg);
            break;
        case 'K':
            pCmdl->widthThumb = atoi(optarg);
            break;
        case 'L':
            pCmdl->heightThumb = atoi(optarg);
            break;
        case 'W':
            // pCmdl->writeOut = atoi(optarg);
            break;
        case 'c':
            pCmdl->comLength = atoi(optarg);
            break;
        case 'P':
            // trigger_point = atoi(optarg);
            break;
        case 'S':
            pCmdl->inputLineBufMode = atoi(optarg);
            break;
        case 'N':
            pCmdl->inputLineBufDepth = atoi(optarg);
            break;
        case 's':
            pCmdl->amountPerLoopBack = atoi(optarg);
            break;
        case 'A':
            pCmdl->hashtype = atoi(optarg);
            break;
        case 'M':
            pCmdl->mirror = atoi(optarg);
            break;
        case 'D':
            pCmdl->formatCustomizedType = atoi(optarg);
            break;
        case 'd':
            pCmdl->constChromaEn = atoi(optarg);
            break;
        case 'e':
            pCmdl->constCb = atoi(optarg);
            break;
        case 'f':
            pCmdl->constCr = atoi(optarg);
            break;
        case '1':  /* --lossless [n] */
            pCmdl->predictMode = atoi(optarg);
            break;
        case '2':  /* --ptrans [n] */
            pCmdl->ptransValue = atoi(optarg);
            break;
        case 'B':
            pCmdl->bitRate = atoi(optarg);
            break;
        case 'J':
            pCmdl->srcFrameRate = atoi(optarg);
            break;
        case 'j':
            pCmdl->EncChnNum = atoi(optarg);
            break;
        case 'n':
            pCmdl->frameRateNum = atoi(optarg);
            break;
        case 'r':
            pCmdl->frameRateDenom = atoi(optarg);
            break;
        case 'V':
            pCmdl->rcMode = atoi(optarg);
            break;
        case 'E':
            pCmdl->qpmin = atoi(optarg);
            break;
        case 'F':
            pCmdl->qpmax = atoi(optarg);
            break;
        case 'U':
            if ((i = JencParseDelim(optarg, ':')) == -1)
                break;

            pCmdl->picQpDeltaMin = atoi(optarg);
            optarg += i + 1;
            pCmdl->picQpDeltaMax = atoi(optarg);
            break;
        case 'O':
            pCmdl->fixedQP = atoi(optarg);
            break;
        case 'Q':
            pCmdl->exp_of_input_alignment = atoi(optarg);
            break;
        case 'Z':
            pCmdl->frameNum = atoi(optarg);
            break;
        case 'z':
            pCmdl->eCompressMode = atoi(optarg);
            break;
        case '0':
            ret = JencCheckLongOption0(&prm, pCmdl, optarg);
            if (ret) {
                status = -1;
            }
            break;
        case '3':
            ret = JencCheckLongOption3(&prm, pCmdl, optarg);
            if (ret) {
                status = -1;
            }
            break;
        default:
            SAMPLE_ERR_LOG("Unsupport prm.short_opt:%c", prm.short_opt);
            status = -1;
            break;
        }
    }

    if (JencParameterCheck(pstPara) != 0) {
        status = -1;
    }

ERR_RET:
    return status;
}
