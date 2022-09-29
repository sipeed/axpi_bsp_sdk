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
#include <signal.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <getopt.h>

#include <tinyalsa/pcm.h>
#include <samplerate.h>
#include "ax_base_type.h"
#include "ax_aenc_api.h"
#include "ax_adec_api.h"
#include "ax_audio_process.h"
#include "ax_sys_api.h"
#include "wave_parser.h"
#include "pp_buffer.h"

#define FILE_NAME_SIZE      128
#define RESAMPLE_NUM        16*1024
//#define RESAMPLE_PERF

static unsigned int gCardNum = 0;
static unsigned int gDeviceNum = 0;
static unsigned int gChannels = 2;
static unsigned int gRate = 48000;
static unsigned int gBits = 16;
static const char *gEncoderType = "g711a";
static AX_S32 gWriteFrames = 0;
static AX_S32 gLoopExit = 0;

static unsigned int gPeriodSize = 1024;
static int gIsWave = 1;
static AUDIO_PROCESS_ATTR_S gAudioProcessAttr = {
    .s32SampleRate = 16000,
    .u32FrameSamples = 160,
    .stAecCfg = {
        .enAecMode = AEC_MODE_DISABLE,
    },
    .stNsCfg = {
        .bNsEnable = AX_FALSE,
        .enAggressivenessLevel = 2,
    },
    .stAgcCfg = {
        .bAgcEnable = AX_FALSE,
        .enAgcMode = AGC_MODE_FIXED_DIGITAL,
        .s16TargetLevel = -3,
        .s16Gain = 9,
    },
};
static unsigned int gAencChannels = 2;
static unsigned int gRef = 0;
static int gResample = 0;
static int gConverter = SRC_SINC_FASTEST;
static AAC_TYPE_E gAacType = AAC_TYPE_AAC_LC;
static AAC_TRANS_TYPE_E gTransType = AAC_TRANS_TYPE_ADTS;

int BitsToFormat(unsigned int bits, enum pcm_format* format)
{
    switch (bits) {
    case 32:
        *format = PCM_FORMAT_S32_LE;
        break;
    case 24:
        *format = PCM_FORMAT_S24_LE;
        break;
    case 16:
        *format = PCM_FORMAT_S16_LE;
        break;
    default:
        fprintf(stderr, "%u bits is not supported.\n", bits);
        return -1;
    }

    return 0;
}

int IsAudioProcessEnabled(const AUDIO_PROCESS_ATTR_S *pstAudioProcessAttr)
{
    int ret = ((gAudioProcessAttr.stAecCfg.enAecMode != AEC_MODE_DISABLE) ||
        (gAudioProcessAttr.stNsCfg.bNsEnable != AX_FALSE) ||
        (gAudioProcessAttr.stAgcCfg.bAgcEnable != AX_FALSE));

    return ret;
}

static int Resample(
        SRC_STATE *src_state,
        short     *input_short,
        short     *output_short,
        int        length_input_short,
        int        length_output_short,
        int        input_sample_rate,
        int        output_sample_rate,
        int        end_of_input
        )
{
    int error;
    SRC_DATA src_data;
    static float input[RESAMPLE_NUM];
    static float output[RESAMPLE_NUM];

#if 0
    printf("%s, length_input_short: %d, length_output_short: %d, "
           "input_sample_rate: %d, output_sample_rate: %d, end_of_input: %d\n",
           __func__, length_input_short, length_output_short,
           input_sample_rate, output_sample_rate, end_of_input);
#endif
    assert(length_input_short <= RESAMPLE_NUM);
    assert(length_output_short <= RESAMPLE_NUM);

    src_short_to_float_array(input_short, input, length_input_short);

    src_data.data_in = input;
    src_data.data_out = output;
    src_data.input_frames = length_input_short;
    src_data.output_frames = length_output_short;
    src_data.end_of_input = end_of_input;
    src_data.src_ratio = (float)output_sample_rate/input_sample_rate;

    #ifdef RESAMPLE_PERF
    struct timeval gTimebegin, gTimeend;
    gettimeofday(&gTimebegin, NULL);
    #endif
    error = src_process(src_state, &src_data);
    if (error) {
        printf("Error : %s\n", src_strerror (error));
    }
    #ifdef RESAMPLE_PERF
    gettimeofday(&gTimeend, NULL);
    AX_U32 total_usec = 1000000 * (gTimeend.tv_sec - gTimebegin.tv_sec) + gTimeend.tv_usec - gTimebegin.tv_usec;
    float total_msec = (float)total_usec / 1000.f;
    #endif

    #if 0
    printf("src_data.input_frames: %ld, src_data.input_frames_used: %ld, src_data.output_frames_gen: %ld\n",
        src_data.input_frames, src_data.input_frames_used, src_data.output_frames_gen);
    #endif

    #ifdef RESAMPLE_PERF
    printf("msec per frame: %.1f\n", total_msec);
    #endif

    assert(src_data.output_frames_gen <= length_output_short);
    src_float_to_short_array(output, output_short, src_data.output_frames_gen);

    return src_data.output_frames_gen;
}

static int Capture(void)
{
    unsigned int card = gCardNum;
    unsigned int device = gDeviceNum;
    int flags = PCM_IN;
    enum pcm_format format;

    if (BitsToFormat(gBits, &format))
        return -1;

    struct pcm_config config = {
        .channels = gChannels,
        .rate = gRate,
        .format = format, //PCM_FORMAT_S16_LE,
        .period_size = gPeriodSize,
        .period_count = 4, //2,
        .start_threshold = 0, //1024,
        .silence_threshold = 0, //1024 * 2,
        .stop_threshold = 0 //1024 * 2
    };

    struct pcm *pcm = pcm_open(card, device, flags, &config);
    if (pcm == NULL) {
        fprintf(stderr, "failed to allocate memory for PCM\n");
        return 0;
    } else if (!pcm_is_ready(pcm)) {
        pcm_close(pcm);
        fprintf(stderr, "failed to open PCM\n");
        return 0;
    }

    // init 3a if enabled
    gAudioProcessAttr.s32SampleRate = gRate;
    gAudioProcessAttr.u32FrameSamples = gPeriodSize;
    if (IsAudioProcessEnabled(&gAudioProcessAttr)) {
        if (AX_AUDIO_PROCESS_Init(&gAudioProcessAttr)) {
            pcm_close(pcm);
            return -1;
        }
    }

    unsigned int frame_size = pcm_frames_to_bytes(pcm, 1);
    unsigned int frames_per_sec = pcm_get_rate(pcm);

    // alloc frame buffer
    printf("frame_size: %u, frames_per_sec: %u\n", frame_size, frames_per_sec);
    printf("gPeriodSize: %u\n", gPeriodSize);
    void *frames = malloc(frame_size * gPeriodSize);
    assert(frames != NULL);
    short *in = NULL;
    short *ref = NULL;
    short *out = NULL;
    if (IsAudioProcessEnabled(&gAudioProcessAttr)) {
        in = malloc(sizeof(short) * gPeriodSize);
        ref = malloc(sizeof(short) * gPeriodSize);
        out = malloc(sizeof(short) * gPeriodSize);
        assert((in != NULL) && (ref != NULL) && (out != NULL));
    }

    // open file for write
    char output_file_name[FILE_NAME_SIZE];
    char in_file_name[FILE_NAME_SIZE];
    char ref_file_name[FILE_NAME_SIZE];
    char out_file_name[FILE_NAME_SIZE];
    snprintf(output_file_name, FILE_NAME_SIZE, "audio.%s", gIsWave ? "wav" : "raw");
    snprintf(in_file_name, FILE_NAME_SIZE, "in.%s", gIsWave ? "wav" : "raw");
    snprintf(ref_file_name, FILE_NAME_SIZE, "ref.%s", gIsWave ? "wav" : "raw");
    snprintf(out_file_name, FILE_NAME_SIZE, "out.%s", gIsWave ? "wav" : "raw");
    FILE *output_file = NULL;
    FILE *in_file = NULL;
    FILE *ref_file = NULL;
    FILE *out_file = NULL;
    if (gWriteFrames) {
        output_file = fopen(output_file_name, "wb");
        assert(output_file != NULL);
        if (gIsWave)
            LeaveWaveHeader(output_file);
        if (IsAudioProcessEnabled(&gAudioProcessAttr)) {
            in_file = fopen(in_file_name, "wb");
            assert(in_file != NULL);
            ref_file = fopen(ref_file_name, "wb");
            assert(ref_file != NULL);
            out_file = fopen(out_file_name, "wb");
            assert(out_file != NULL);

            if (gIsWave) {
                LeaveWaveHeader(in_file);
                LeaveWaveHeader(ref_file);
                LeaveWaveHeader(out_file);
            }
        }
    }

    unsigned int totalFrames = 0;
    while (!gLoopExit) {
        int read_count = pcm_readi(pcm, frames, gPeriodSize);

        size_t byte_count = pcm_frames_to_bytes(pcm, read_count);
        printf("read_count: %d, byte_count: %lu\n", read_count, byte_count);

        if (IsAudioProcessEnabled(&gAudioProcessAttr)) {
            AX_S16 *left = (gRef == 0) ? ref : in;
            AX_S16 *right = (gRef == 0) ? in : ref;
            AX_AUDIO_InterleavedToNoninterleaved16(frames, read_count, left, right);
            if (gWriteFrames) {
                fwrite(in, sizeof(short), read_count, in_file);
                fwrite(ref, sizeof(short), read_count, ref_file);
            }
            AX_AUDIO_PROCESS_Proc(in, ref, out);
            if (gWriteFrames)
                fwrite(out, sizeof(short), read_count, out_file);
        }

        if (gWriteFrames)
            fwrite(frames, 1, byte_count, output_file);

        totalFrames += read_count;
    }

    if (gWriteFrames) {
        if (IsAudioProcessEnabled(&gAudioProcessAttr)) {
            if (gIsWave) {
                WriteWaveHeader(in_file, 1, gRate, gBits, totalFrames);
                WriteWaveHeader(ref_file, 1, gRate, gBits, totalFrames);
                WriteWaveHeader(out_file, 1, gRate, gBits, totalFrames);
            }
            fclose(in_file);
            fclose(ref_file);
            fclose(out_file);
        }
        if (gIsWave)
            WriteWaveHeader(output_file, gChannels, gRate, gBits, totalFrames);
        fclose(output_file);
    }

    if (IsAudioProcessEnabled(&gAudioProcessAttr)) {
        free(in);
        free(ref);
        free(out);
    }
    free(frames);

    if (IsAudioProcessEnabled(&gAudioProcessAttr)) {
        AX_AUDIO_PROCESS_DeInit();
    }

    pcm_close(pcm);

    return 0;
}

static int Playback(void)
{
    unsigned int card = gCardNum;
    unsigned int device = gDeviceNum;
    int flags = PCM_OUT;
    enum pcm_format format;

    if (BitsToFormat(gBits, &format))
        return -1;

    struct pcm_config config = {
        .channels = gChannels,
        .rate = gRate,
        .format = format, //PCM_FORMAT_S16_LE,
        .period_size = gPeriodSize,
        .period_count = 2,
        .start_threshold = 1024,
        .silence_threshold = 1024 * 2,
        .stop_threshold = 1024 * 2
    };

    char input_file_name[FILE_NAME_SIZE];
    snprintf(input_file_name, FILE_NAME_SIZE, "audio.%s", gIsWave ? "wav" : "raw");
    FILE *input_file = fopen(input_file_name, "rb");
    if (input_file == NULL) {
        printf("failed to open '%s' for reading", input_file_name);
        return 0;
    }

    uint16_t num_channels = gChannels;
    uint32_t sample_rate;
    if (gIsWave) {
        uint16_t bits_per_sample;
        if (ParseWaveHeader(input_file_name, input_file, &num_channels, &sample_rate, &bits_per_sample)) {
            fprintf(stderr, "ParseWaveHeader '%s' error\n", input_file_name);
            fclose(input_file);
            return 0;
        }
        enum pcm_format wav_format;
        if (BitsToFormat(bits_per_sample, &wav_format)) {
            fclose(input_file);
            return -1;
        }
        config.channels = gChannels;
        if (!gResample)
            config.rate = sample_rate;
        config.format = wav_format;

        if (gResample && (sample_rate != gRate))
            printf("Resample to %u\n", gRate);
    }

    struct pcm *pcm = pcm_open(card, device, flags, &config);
    if (pcm == NULL) {
        fprintf(stderr, "failed to allocate memory for PCM\n");
        fclose(input_file);
        return -1;
    } else if (!pcm_is_ready(pcm)) {
        fclose(input_file);
        pcm_close(pcm);
        fprintf(stderr, "failed to open PCM\n");
        return -1;
    }

    char *buffer;
    int size;
    int num_read;

    size = pcm_frames_to_bytes(pcm, pcm_get_buffer_size(pcm)) / 2;

    int bufferSize = size;
    AX_S16 *mono = NULL;
    int monoSize = 0;
    AX_S16 *resampleBuff = NULL;
    int resampleBuffSize = 0;
    SRC_STATE *src_state = NULL;
    int error;
    int output_frames_gen;
    if (num_channels != gChannels) {
        monoSize = bufferSize / 2;
        printf("monoSize: %d\n", monoSize);
        mono = malloc(monoSize);
        assert(mono != NULL);
        if (gResample) {
            resampleBuffSize = (int)((float)monoSize * (float)gRate/(float)sample_rate);
            printf("resampleBuffSize: %d\n", resampleBuffSize);
            resampleBuff = malloc(resampleBuffSize);
            assert(resampleBuff != NULL);

            bufferSize = resampleBuffSize * 2;
            /* Initialize the sample rate converter. */
            if ((src_state = src_new(gConverter, num_channels, &error)) == NULL) {
                fclose(input_file);
                pcm_close(pcm);
                printf ("\n\nError : src_new() failed : %s.\n\n", src_strerror (error)) ;
                return -1;
            }
        }
    }

    printf("bufferSize: %d\n", bufferSize);
    buffer = malloc(bufferSize);
    assert(buffer != NULL);

    do {
        if (num_channels != gChannels) {
            num_read = fread(mono, 1, monoSize, input_file);
            if (gResample && (sample_rate != gRate)) {
                output_frames_gen = Resample(
                    src_state,
                    mono,
                    resampleBuff,
                    num_read/2,
                    resampleBuffSize/2,
                    sample_rate,
                    gRate,
                    num_read < monoSize ? 1 : 0);
                AX_AUDIO_MonoToStereo16(resampleBuff, output_frames_gen, (AX_S16 *)buffer);
                num_read = output_frames_gen*4;
            } else {
                AX_AUDIO_MonoToStereo16(mono, num_read / 2, (AX_S16 *)buffer);
                num_read *= 2;
            }
        } else {
            num_read = fread(buffer, 1, bufferSize, input_file);
        }
        if (num_read > 0) {
            if (pcm_writei(pcm, buffer,
                           pcm_bytes_to_frames(pcm, num_read)) < 0) {
                fprintf(stderr, "error playing sample\n");
                break;
            }
            printf("played %d bytes\n", num_read);
        }
    } while (!gLoopExit && num_read > 0);

    pcm_wait(pcm, -1);
    pcm_close(pcm);

    fclose(input_file);
    if (num_channels != gChannels) {
        free(mono);
        if (gResample) {
            free(resampleBuff);
            src_delete(src_state);
        }
    }
    free(buffer);
    return 0;
}

typedef struct axSAMPLE_AENC_ARGS_S {
    AENC_CHN aeChn;
    AX_PAYLOAD_TYPE_E payloadType;
    const char* fileExt;
} SAMPLE_AENC_ARGS_S;

static void *AencRecvThread(void *arg)
{
    AX_S32 ret = AX_SUCCESS;
    SAMPLE_AENC_ARGS_S *aencArgs = (SAMPLE_AENC_ARGS_S *)arg;
    FILE *fp_out = NULL;
    FILE *lengthHandle = NULL;
    if (gWriteFrames) {
        AX_CHAR file_path[128];
        snprintf(file_path, 128, "audio.%s", aencArgs->fileExt);
        printf("Write encoded audio to: %s\n", file_path);
        fp_out = fopen(file_path, "w");
        assert(fp_out != NULL);

        if (gWriteFrames && (aencArgs->payloadType == PT_AAC) && (gTransType == AAC_TRANS_TYPE_RAW)) {
            const char *lengthfile = "length.txt"; //record frame length
            lengthHandle = fopen(lengthfile, "wb");
            assert(lengthHandle != NULL);
        }
    }

    while (1) {
        AX_AUDIO_STREAM_S pstStream;
        ret = AX_AENC_GetStream(aencArgs->aeChn, &pstStream, -1);
        if (ret) {
            printf("AX_AENC_GetStream error: %d\n", ret);
            abort();
        }
        printf("after encode, u32Seq: %u, byte_count: %u\n", pstStream.u32Seq, pstStream.u32Len);
        if (gWriteFrames && pstStream.u32Len)
            fwrite(pstStream.pStream, 1, pstStream.u32Len, fp_out);
        if (gWriteFrames && (aencArgs->payloadType == PT_AAC) && (gTransType == AAC_TRANS_TYPE_RAW)) {
            fwrite(&(pstStream.u32Len), 1, 2, lengthHandle);
        }
        AX_AENC_ReleaseStream(aencArgs->aeChn, &pstStream);

        if (gLoopExit)
            break;
    }

    if (gWriteFrames)
        fclose(fp_out);
    if (gWriteFrames && (aencArgs->payloadType == PT_AAC) && (gTransType == AAC_TRANS_TYPE_RAW)) {
        fclose(lengthHandle);
    }
    return NULL;
}

static int StringToPayloadTypeFileExt(const char* str, AX_PAYLOAD_TYPE_E *pType, const char** fileExt) {
    int result = 0;
    if (!strcmp(str, "g711a")) {
        *pType = PT_G711A;
        *fileExt = "g711a";
    } else if (!strcmp(str, "g711u")) {
        *pType = PT_G711U;
        *fileExt = "g711u";
    } else if (!strcmp(str, "aac")) {
        *pType = PT_AAC;
        *fileExt = "aac";
    } else if (!strcmp(str, "lpcm")) {
        *pType = PT_LPCM;
        *fileExt = "lpcm";
    } else {
        result = -1;
    }

    return result;
}

static int AudioEncode(void)
{
    unsigned int card = gCardNum;
    unsigned int device = gDeviceNum;
    int flags = PCM_IN;
    enum pcm_format format;

    if (gChannels != 2) {
        printf("%u channels is not supported.\n", gChannels);
        return -1;
    }
    if (gBits != 16) {
        printf("%u bits is not supported.\n", gBits);
        return -1;
    }

    if (BitsToFormat(gBits, &format))
        return -1;

    struct pcm_config config = {
        .channels = gChannels,
        .rate = gRate,
        .format = format, //PCM_FORMAT_S32_LE,
        .period_size = gPeriodSize,
        .period_count = 4, //2,
        .start_threshold = 0, //1024,
        .silence_threshold = 0, //1024 * 2,
        .stop_threshold = 0 //1024 * 2
    };

    struct pcm *pcm = pcm_open(card, device, flags, &config);
    if (pcm == NULL) {
        fprintf(stderr, "failed to allocate memory for PCM\n");
        return 0;
    } else if (!pcm_is_ready(pcm)) {
        pcm_close(pcm);
        fprintf(stderr, "failed to open PCM\n");
        return 0;
    }

    // get encoder type
    AX_PAYLOAD_TYPE_E payloadType;
    const char* fileExt;
    if (StringToPayloadTypeFileExt(gEncoderType, &payloadType, &fileExt)) {
        printf("Unknown payload type\n");
        pcm_close(pcm);
        return -1;
    }

    AX_S32 ret = AX_SUCCESS;

    ret = AX_AENC_Init();
    if (ret) {
        printf("AX_AENC_Init error: %x\n", ret);
        pcm_close(pcm);
        return -1;
    }

    AENC_CHN aeChn = 0;
    AX_AENC_CHN_ATTR_S pstAttr;
    AX_AENC_AAC_ENCODER_ATTR_S aacEncoderAttr = {
        .enAacType = gAacType,
        .enTransType = gTransType,
        .enChnMode = (gAencChannels == 1) ? AAC_CHANNEL_MODE_1 : AAC_CHANNEL_MODE_2,
        .u32GranuleLength = (gAacType == AAC_TYPE_AAC_LC) ? 1024 : 480,
        .u32SampleRate = gRate,
        .u32BitRate = 128000
    };
    pstAttr.enType = payloadType;
    pstAttr.u32PtNumPerFrm = ((payloadType == PT_G711A) || (payloadType == PT_G711U) ||
        ((payloadType == PT_AAC) && (gAacType == AAC_TYPE_AAC_LC))) ? 1024 : 480;
    pstAttr.u32BufSize = 8;
    pstAttr.pValue = payloadType == PT_AAC ? &aacEncoderAttr : NULL;
    ret = AX_AENC_CreateChn(aeChn, &pstAttr);
    if (ret) {
        printf("AX_AENC_CreateChn error: %x\n", ret);
        pcm_close(pcm);
        return -1;
    }

    if (gPeriodSize > pstAttr.u32PtNumPerFrm) {
        printf("gPeriodSize should equal to or less than u32PtNumPerFrm\n");
        pcm_close(pcm);
        return -1;
    }

    if ((payloadType == PT_AAC) && (gTransType == AAC_TRANS_TYPE_RAW)) {
        FILE *ascHandle = NULL;
        const char *ascfile = "asc.txt"; //record Audio Specific Config
        ascHandle = fopen(ascfile, "wb");
        assert(ascHandle != NULL);
        fwrite(aacEncoderAttr.u8ConfBuf, 1, 64, ascHandle);
        fclose(ascHandle);
    }

    unsigned int frame_size = pcm_frames_to_bytes(pcm, 1);
    //unsigned int frames_per_sec = pcm_get_rate(pcm);

    // alloc raw audio buffer
    printf("frame_size: %u, u32PtNumPerFrm: %u\n", frame_size, pstAttr.u32PtNumPerFrm);
    void *frames = malloc(frame_size * pstAttr.u32PtNumPerFrm);
    if (frames == NULL) {
        fprintf(stderr, "failed to allocate frames\n");
        AX_AENC_DestroyChn(aeChn);
        pcm_close(pcm);
        return 0;
    }

    short *left = NULL;
    short *right = NULL;
    void* ppbuffer = NULL;
    uint8_t* p_buffer = NULL;
    int p_buffer_size = 0;
    if (gAencChannels == 1) {
        left = malloc(sizeof(short) * pstAttr.u32PtNumPerFrm);
        right = malloc(sizeof(short) * pstAttr.u32PtNumPerFrm);
        assert((left != NULL) && (right != NULL));

        if (gPeriodSize != pstAttr.u32PtNumPerFrm) {
            ppbuffer = PPBufferCreate(sizeof(short) * pstAttr.u32PtNumPerFrm);
            assert(ppbuffer != NULL);
        }
    }

    SAMPLE_AENC_ARGS_S aencArgs;
    aencArgs.aeChn = aeChn;
    aencArgs.payloadType = payloadType;
    aencArgs.fileExt = fileExt;
    pthread_t recvTid;
    pthread_create(&recvTid, NULL, AencRecvThread, (void *)&aencArgs);
    int seq = 1;
    while (1) {
        // read from PCM
        int read_count = pcm_readi(pcm, frames, gPeriodSize);
        size_t byte_count = pcm_frames_to_bytes(pcm, read_count);
        //printf("before encode, byte_count: %lu\n", byte_count);

        AX_U8 *frameAddr = frames;
        AX_U32 frameLen = byte_count;
        if (gAencChannels == 1) {
            AX_AUDIO_InterleavedToNoninterleaved16(frames, read_count, left, right);
            frameAddr = (AX_U8 *)((0 == gRef) ? right : left);
            frameLen = byte_count/2;

            if (gPeriodSize != pstAttr.u32PtNumPerFrm) {
                PPBufferUpdate(ppbuffer,
                               (uint8_t*)((0 == gRef) ? right : left),
                               byte_count/2,
                               &p_buffer,
                               &p_buffer_size);
                if (!p_buffer)
                    continue;

                frameAddr = p_buffer;
                frameLen = p_buffer_size;
            }
        }

        // encode
        AX_AUDIO_FRAME_S pstFrm;
        pstFrm.enBitwidth = AX_AUDIO_BIT_WIDTH_16;
        pstFrm.enSoundmode = (gAencChannels == 1) ? AX_AUDIO_SOUND_MODE_MONO : AX_AUDIO_SOUND_MODE_STEREO;
        pstFrm.u64VirAddr = frameAddr;
        pstFrm.u32Seq = seq++;
        pstFrm.u32Len = frameLen;

        while (1) {
            ret = AX_AENC_SendFrame(aeChn, &pstFrm);
            if (ret == AX_SUCCESS) {
                break;
            } else if (ret == AX_ERR_AENC_BUF_FULL) {
                usleep(20 * 1000);
                continue;
            } else {
                printf("AX_AENC_SendFrame error: %x\n", ret);
                abort();
            }
        }

        if (gLoopExit)
            break;
    }

    pthread_join(recvTid, NULL);
    if (gAencChannels == 1) {
        free(left);
        free(right);
        if (gPeriodSize != pstAttr.u32PtNumPerFrm)
            PPBufferDestory(ppbuffer);
    }
    free(frames);
    AX_AENC_DestroyChn(aeChn);
    AX_AENC_DeInit();
    pcm_close(pcm);
    return 0;
}

typedef struct axSAMPLE_ADEC_ARGS_S {
    ADEC_CHN adChn;
    struct pcm *pcm;
} SAMPLE_ADEC_ARGS_S;

static void *AdecRecvThread(void *arg)
{
    AX_S32 ret = AX_SUCCESS;
    SAMPLE_ADEC_ARGS_S *adecArgs = (SAMPLE_ADEC_ARGS_S *)arg;
    AX_AUDIO_FRAME_S stFrmInfo;

    AX_S16 *buffer = NULL;
    AX_U32 buffer_len = 0;
    if (gAencChannels == 1) {
        buffer = malloc(32 * 1024);
        assert(buffer);
    }
    while (1) {
        ret = AX_ADEC_GetFrame(adecArgs->adChn, &stFrmInfo, AX_TRUE);
        if (ret == AX_SUCCESS) {
            printf("after decode, u32Seq: %u, byte_count: %u, enSoundmode: %d\n", stFrmInfo.u32Seq, stFrmInfo.u32Len, stFrmInfo.enSoundmode);
            if (gAencChannels == 1) {
                AX_AUDIO_MonoToStereo16((AX_S16 *)stFrmInfo.u64VirAddr, stFrmInfo.u32Len / 2, (AX_S16 *)buffer);
                buffer_len = stFrmInfo.u32Len * 2;
            } else {
                buffer = (AX_S16 *)stFrmInfo.u64VirAddr;
                buffer_len = stFrmInfo.u32Len;
            }
            if (pcm_writei(adecArgs->pcm, buffer,
                           pcm_bytes_to_frames(adecArgs->pcm, buffer_len)) < 0) {
                fprintf(stderr, "error playing sample\n");
                break;
            }
            //printf("played %d bytes\n", stFrmInfo.u32Len);
            AX_ADEC_ReleaseFrame(adecArgs->adChn, &stFrmInfo);
        } else if (ret == AX_ERR_ADEC_END_OF_STREAM) {
            printf("AX_ADEC_GetFrame: AX_ERR_ADEC_END_OF_STREAM\n");
            break;
        } else {
            printf("AX_ADEC_GetFrame error: %d\n", ret);
            abort();
        }
    }

    if (gAencChannels == 1) {
        free(buffer);
    }
    return NULL;
}

static int AudioDecode(void)
{
    unsigned int card = gCardNum;
    unsigned int device = gDeviceNum;
    int flags = PCM_OUT;
    enum pcm_format format;

    if (gChannels != 2) {
        printf("%u channels is not supported.\n", gChannels);
        return -1;
    }
    if (gBits != 16) {
        printf("%u bits is not supported.\n", gBits);
        return -1;
    }

    if (BitsToFormat(gBits, &format))
        return -1;

    struct pcm_config config = {
        .channels = gChannels,
        .rate = gRate,
        .format = format, //PCM_FORMAT_S32_LE,
        .period_size = gPeriodSize,
        .period_count = 2,
        .start_threshold = 1024,
        .silence_threshold = 1024 * 2,
        .stop_threshold = 1024 * 2
    };

    struct pcm *pcm = pcm_open(card, device, flags, &config);
    if (pcm == NULL) {
        fprintf(stderr, "failed to allocate memory for PCM\n");
        return -1;
    } else if (!pcm_is_ready(pcm)) {
        pcm_close(pcm);
        fprintf(stderr, "failed to open PCM\n");
        return -1;
    }

    char *buffer;
    int size;
    int num_read;

    size = pcm_frames_to_bytes(pcm, pcm_get_buffer_size(pcm)) / 2;
    buffer = malloc(size);
    if (!buffer) {
        fprintf(stderr, "unable to allocate %d bytes\n", size);
        pcm_close(pcm);
        return -1;
    }

    // get encoder type
    AX_PAYLOAD_TYPE_E payloadType;
    const char* fileExt;
    if (StringToPayloadTypeFileExt(gEncoderType, &payloadType, &fileExt)) {
        printf("Unknown payload type\n");
        free(buffer);
        pcm_close(pcm);
        return -1;
    }

    AX_S32 ret = AX_SUCCESS;

    ret = AX_ADEC_Init();
    if (ret) {
        printf("AX_ADEC_Init error: %x\n", ret);
        pcm_close(pcm);
        return -1;
    }

    FILE *lengthHandle = NULL;
    AX_U8 raw_conf[64] = { 0x0 };
    AX_U8 *conf[] = { raw_conf };
    AX_U32 conf_len = sizeof(raw_conf);
    if ((payloadType == PT_AAC) && (gTransType == AAC_TRANS_TYPE_RAW)) {
        const char *lengthfile = "length.txt";
        lengthHandle = fopen(lengthfile, "rb");
        if (!lengthHandle) {
            perror(lengthfile);
            return 1;
        }
        FILE *ascHandle = NULL;
        const char *ascfile = "asc.txt"; //record Audio Specific Config
        ascHandle = fopen(ascfile, "rb");
        if (!ascHandle) {
            perror(ascfile);
            return 1;
        }
        int al = fread(raw_conf, 1, 64, ascHandle);
        if (al < 64) {
            printf("fread error\n");
            return 1;
        }
        fclose(ascHandle);
    }

    AX_ADEC_AAC_DECODER_ATTR_S aacDecoderAttr = {
        .enTransType = gTransType,
        .u8Conf = conf,
        .u32ConfLen = conf_len
    };

    AENC_CHN adChn = 0;
    AX_ADEC_CHN_ATTR_S pstAttr;
    pstAttr.enType = payloadType;
    pstAttr.u32BufSize = 8;
    pstAttr.pValue = (payloadType == PT_AAC) ? &aacDecoderAttr : NULL;
    ret = AX_ADEC_CreateChn(adChn, &pstAttr);
    if (ret) {
        printf("AX_ADEC_CreateChn error: %x\n", ret);
        free(buffer);
        pcm_close(pcm);
        return -1;
    }

    AX_CHAR file_path[128];
    snprintf(file_path, 128, "audio.%s", fileExt);
    FILE *input_file = fopen(file_path, "rb");
    if (input_file == NULL) {
        printf("failed to open '%s' for reading", file_path);
        AX_ADEC_DestroyChn(adChn);
        free(buffer);
        pcm_close(pcm);
        return 0;
    }

    SAMPLE_ADEC_ARGS_S adecArgs;
    adecArgs.adChn = adChn;
    adecArgs.pcm = pcm;
    pthread_t recvTid;
    pthread_create(&recvTid, NULL, AdecRecvThread, (void *)&adecArgs);
    int seq = 1;
    if ((pstAttr.enType == PT_G711A) || (pstAttr.enType == PT_G711U) ||
        (pstAttr.enType == PT_LPCM)) {
        do {
            num_read = fread(buffer, 1, size, input_file);
            if (num_read > 0) {
                AX_AUDIO_STREAM_S stStream;
                stStream.pStream = (AX_U8 *)buffer;
                stStream.u64PhyAddr = 0;
                stStream.u32Len = num_read;
                stStream.u32Seq = seq++;
                stStream.bEof = AX_FALSE;
                ret = AX_ADEC_SendStream(adChn, &stStream, AX_TRUE);
                if (ret != AX_SUCCESS) {
                    printf("AX_ADEC_SendStream error: %x\n", ret);
                    abort();
                }
            }
        } while (!gLoopExit && num_read > 0);
    } else {
        while (!gLoopExit) {
            AX_U8 packet[10240];
            int n;
            AX_U32 packet_size;
            if (gTransType == AAC_TRANS_TYPE_ADTS) {
                n = fread(packet, 1, 7, input_file);
                if (n != 7)
                    break;
                if (packet[0] != 0xff || (packet[1] & 0xf0) != 0xf0) {
                    fprintf(stderr, "Not an ADTS packet\n");
                    break;
                }
                packet_size = ((packet[3] & 0x03) << 11) | (packet[4] << 3) | (packet[5] >> 5);
                n = fread(packet + 7, 1, packet_size - 7, input_file);
                if (n != packet_size - 7) {
                    fprintf(stderr, "Partial packet\n");
                    break;
                }
            } else if (gTransType == AAC_TRANS_TYPE_RAW) {
                int ll = fread(&packet_size, 1, 2, lengthHandle);
                if (ll < 1)
                    break;
                //printf("packet_size %d\n", packet_size);
                n = fread(packet, 1, packet_size, input_file);
                if (n != packet_size) {
                    fprintf(stderr, "Partial packet\n");
                    break;
                }
            } else {
                fprintf(stderr, "unsupport trans type\n");
                break;
            }

            AX_AUDIO_STREAM_S stStream;
            stStream.pStream = (AX_U8 *)packet;
            stStream.u64PhyAddr = 0;
            stStream.u32Len = packet_size;
            stStream.u32Seq = seq++;
            stStream.bEof = AX_FALSE;
            ret = AX_ADEC_SendStream(adChn, &stStream, AX_TRUE);
            if (ret != AX_SUCCESS) {
                printf("AX_ADEC_SendStream error: %x\n", ret);
                abort();
            }
        }
    }

    AX_ADEC_SendEndOfStream(adChn, AX_FALSE);
    pthread_join(recvTid, NULL);
    AX_ADEC_DestroyChn(adChn);
    AX_ADEC_DeInit();
    pcm_wait(pcm, -1);
    pcm_close(pcm);

    if ((payloadType == PT_AAC) && (gTransType == AAC_TRANS_TYPE_RAW)) {
        fclose(lengthHandle);
    }
    fclose(input_file);
    free(buffer);
    return 0;
}

static void SigInt(int sigNo)
{
    printf("Catch signal %d\n", sigNo);
    gLoopExit = 1;
}

static void PrintHelp()
{
    printf("usage: sample_audio <command> <args>\n");
    printf("commands:\n");
    printf("  cap:              capture raw audio.\n");
    printf("  play:             play raw audio.\n");
    printf("  aenc:             encode audio.\n");
    printf("  adec:             decode audio.\n");
    printf("args:\n");
    printf("  -D:               card number.                (support 0), default: 0\n");
    printf("  -d:               device number.              (support 0,1), default: 0\n");
    printf("  -c:               channels.                   (support 2,4), default: 2\n");
    printf("  -r:               rate.                       (support 8000~48000), default: 48000\n");
    printf("  -b:               bits.                       (support 16,32), default: 16\n");
    printf("  -p:               period size.                (support 80~1024), default: 1024\n");
    printf("  -v:               is wave file.               (support 0,1), default: 1\n");
    printf("  -e:               encoder type.               (support g711a, g711u, aac, lpcm), default: g711a\n");
    printf("  -w:               write audio frame to file.  (support 0,1), default: 0\n");
    printf("  --aec-mode:       aec mode.                   (support 0,1,2), default: 0\n");
    printf("  --sup-level:      Suppression Level.          (support 0,1,2), default: 0\n");
    printf("  --routing-mode:   routing mode.               (support 0,1,2,3,4), default: 0\n");
    printf("  --aenc-chns:      encode channels.            (support 1,2), default: 2\n");
    printf("  --ref:            ref channel.                (support 0,1), default: 0\n");
    printf("  --ns:             ns enable.                  (support 0,1), default: 0\n");
    printf("  --ag-level:       aggressiveness level.       (support 0,1,2,3), default: 2\n");
    printf("  --agc:            agc enable.                 (support 0,1), default: 0\n");
    printf("  --target-level:   target level.               (support -31~0), default: -3\n");
    printf("  --gain:           compression gain.           (support 0~90), default: 9\n");
    printf("  --resample:       resample enable.            (support 0,1), default: 0\n");
    printf("  --converter:      converter type.             (support 0~4), default: 2\n");
    printf("  --aac-type:       aac type.                   (support 2,23,39), default: 2\n");
    printf("  --trans-type:     trans type.                 (support 0,2), default: 2\n");
}

enum LONG_OPTION {
    LONG_OPTION_AEC_MODE = 10000,
    LONG_OPTION_SUPPRESSION_LEVEL,
    LONG_OPTION_ROUTING_MODE,
    LONG_OPTION_AENC_CHANNELS,
    LONG_OPTION_REF,
    LONG_OPTION_NS_ENABLE,
    LONG_OPTION_AGGRESSIVENESS_LEVEL,
    LONG_OPTION_AGC_ENABLE,
    LONG_OPTION_TARGET_LEVEL,
    LONG_OPTION_GAIN,
    LONG_OPTION_RESAMPLE,
    LONG_OPTION_CONVERTER,
    LONG_OPTION_AAC_TYPE,
    LONG_OPTION_AAC_TRANS_TYPE,
    LONG_OPTION_BUTT
};

int main(int argc, char *argv[])
{
    extern int optind;
    AX_S32 c;
    AX_S32 isExit = 0;
    signal(SIGINT, SigInt);

    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
            {"aec-mode",            required_argument,  0, LONG_OPTION_AEC_MODE},
            {"sup-level",           required_argument,  0, LONG_OPTION_SUPPRESSION_LEVEL },
            {"routing-mode",        required_argument,  0, LONG_OPTION_ROUTING_MODE },
            {"aenc-chns",           required_argument,  0, LONG_OPTION_AENC_CHANNELS },
            {"ref",                 required_argument,  0, LONG_OPTION_REF },
            {"ns",                  required_argument,  0, LONG_OPTION_NS_ENABLE },
            {"ag-level",            required_argument,  0, LONG_OPTION_AGGRESSIVENESS_LEVEL },
            {"agc",                 required_argument,  0, LONG_OPTION_AGC_ENABLE },
            {"target-level",        required_argument,  0, LONG_OPTION_TARGET_LEVEL },
            {"gain",                required_argument,  0, LONG_OPTION_GAIN },
            {"resample",            required_argument,  0, LONG_OPTION_RESAMPLE },
            {"converter",           required_argument,  0, LONG_OPTION_CONVERTER },
            {"aac-type",            required_argument,  0, LONG_OPTION_AAC_TYPE },
            {"trans-type",          required_argument,  0, LONG_OPTION_AAC_TRANS_TYPE },
            {0,                     0,                  0, 0 }
        };

        c = getopt_long(argc, argv, "D:d:c:r:b:p:v:e:w:h",
                 long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 'D':
            gCardNum = atoi(optarg);
            break;
        case 'd':
            gDeviceNum = atoi(optarg);
            break;
        case 'c':
            gChannels = atoi(optarg);
            break;
        case 'r':
            gRate = atoi(optarg);
            break;
        case 'b':
            gBits = atoi(optarg);
            break;
        case 'p':
            gPeriodSize = atoi(optarg);
            break;
        case 'v':
            gIsWave = atoi(optarg);
            break;
        case 'e':
            gEncoderType = optarg;
            break;
        case 'w':
            gWriteFrames = atoi(optarg);
            break;
        case 'h':
            isExit = 1;
            break;
        case LONG_OPTION_AEC_MODE:
            gAudioProcessAttr.stAecCfg.enAecMode = atoi(optarg);
            break;
        case LONG_OPTION_SUPPRESSION_LEVEL:
            gAudioProcessAttr.stAecCfg.stAecFloatCfg.enSuppressionLevel = atoi(optarg);
            break;
        case LONG_OPTION_ROUTING_MODE:
            gAudioProcessAttr.stAecCfg.stAecFixedCfg.eRoutingMode = atoi(optarg);
            break;
        case LONG_OPTION_AENC_CHANNELS:
            gAencChannels = atoi(optarg);
            break;
        case LONG_OPTION_REF:
            gRef = atoi(optarg);
            break;
        case LONG_OPTION_NS_ENABLE:
            gAudioProcessAttr.stNsCfg.bNsEnable = atoi(optarg);
            break;
        case LONG_OPTION_AGGRESSIVENESS_LEVEL:
            gAudioProcessAttr.stNsCfg.enAggressivenessLevel = atoi(optarg);
            break;
        case LONG_OPTION_AGC_ENABLE:
            gAudioProcessAttr.stAgcCfg.bAgcEnable = atoi(optarg);
            break;
        case LONG_OPTION_TARGET_LEVEL:
            gAudioProcessAttr.stAgcCfg.s16TargetLevel = atoi(optarg);
            break;
        case LONG_OPTION_GAIN:
            gAudioProcessAttr.stAgcCfg.s16Gain = atoi(optarg);
            break;
        case LONG_OPTION_RESAMPLE:
            gResample = atoi(optarg);
            break;
        case LONG_OPTION_CONVERTER:
            gConverter = atoi(optarg);
            break;
        case LONG_OPTION_AAC_TYPE:
            gAacType = atoi(optarg);
            break;
        case LONG_OPTION_AAC_TRANS_TYPE:
            gTransType = atoi(optarg);
            break;
        default:
            isExit = 1;
            break;
        }
    }
    if (isExit || optind >= argc) {
        PrintHelp();
        exit(0);
    }

    if (!strncmp(argv[optind], "cap", 3)) {
        Capture();
    } else if (!strncmp(argv[optind], "play", 4)) {
        Playback();
    } else if (!strncmp(argv[optind], "aenc", 4)) {
        AudioEncode();
    } else if (!strncmp(argv[optind], "adec", 4)) {
        AudioDecode();
    } else {
        printf("Unknown command: %s\n", argv[optind]);
    }

    return 0;
}
