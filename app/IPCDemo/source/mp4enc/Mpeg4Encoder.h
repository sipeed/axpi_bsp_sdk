/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef MPEG4_ENCODER_H
#define MPEG4_ENCODER_H

#include "global.h"
#include <iostream>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <condition_variable>
#include <mutex>
#include <thread>
#include "AXRingBuffer.h"
#include "Singleton.h"
#include "mp4v2/mp4v2.h"

using namespace std;

#define MAX_MPEG4_CHANNEL_NUM (1)
#define MPEG4_DEFAULT_FRAME_RATE (25)
#define MP4_DEFAULT_FILE_SIZE (268435456) //256*1024*1024
#define MP4_DEFAULT_RECORD_FILE_NUM (10)
#define MP4_FORMAT_NAME "mp4"
#ifdef AX_SIMPLIFIED_MEM_VER
#define MP4_RINGBUF_DATA_SIZE (5)
#else
#define MP4_RINGBUF_DATA_SIZE (10)
#endif //AX_SIMPLIFIED_MEM_VER
#define SECOND_5S   (5000)
#define SECOND_15S  (15000)
#define MPEG4_FILE_NAME_TAG "ipcdemo"
#define MPEG4_FILE_NAME_PATTERN "ipcdemo_(\\d{4}-\\d{2}-\\d{2}-\\d{2}-\\d{2}-\\d{2}).mp4"
#define MP4_RECORD_SPACE_RESERVED   (134217728) //128MB:1024*1024*128
#define MP4_RECORD_SPACE_MARGIN     (8388608) //8MB:1024*1024*8
#define MP4_HEAD_TAIL_SIZE          (262)

#define MAX_NAME_LEN (32)  //ipcdemo_Year-Month-Day-Hour-Min-Sec.mp4, eg. ipcdemo_2021-07-12-15-00-00.mp4

typedef struct _stMPEG4ECInfo
{
    AX_U32  nFrameRate;
    AX_S32  nfrWidth;
    AX_S32  nfrHeight;
    AX_S32  nBitrate;
    AX_BOOL bIsH264;
} MPEG4EC_INFO_T;


class CMPEG4Encoder: public CSingleton<CMPEG4Encoder>
{
    friend class CSingleton<CMPEG4Encoder>;
public:
    AX_BOOL Start();
    AX_VOID Stop();
    AX_BOOL Init();
    AX_VOID InitParam(const MPEG4EC_INFO_T& stMpeg4Info);
    AX_VOID SendRawFrame(AX_U8 nChn,  AX_VOID* data, AX_U32 size, AX_U64 nPts=0, AX_BOOL bIFrame=AX_FALSE);
    AX_CHAR* GenFileName(AX_CHAR* szFileName);
    std::map<std::string, AX_U64> GetRecorderFiles(std::string path, std::string suffix, AX_U64 &totalSize);
    AX_U64 GetFreeSpaceForMp4Record(const std::string& path, const AX_U64 nRecordFilesSize);
    AX_U64 GetAvailableSpace();
    AX_BOOL RemoveFilesForSpace(const AX_U64& nCurrAvailableSpace);
    AX_VOID DropFrames();
    AX_VOID SetRecordFileSizeInfo(const std::string& strFileName, const AX_U64& nFileSize);
    AX_VOID UpdateRecordStrategyParams();

private:
    static AX_VOID* WriteFrameThreadFunc(AX_VOID* __this);

private:
    // open or creat a mp4 file.
    MP4FileHandle CreateMP4File(const char* fileName, AX_S32 timeScale = 90000);
    // wirte 264 data, data can contain  multiple frame.
    int WriteH264Data(MP4FileHandle hMp4File, CAXRingElement* pData, AX_U64 nBasePts);
    // close mp4 file.
    void CloseMP4File(MP4FileHandle& hMp4File);

public:
    thread* m_pWriteFrameThread;
    CAXRingBuffer* m_pRingBufRawFrame[MAX_MPEG4_CHANNEL_NUM];
    MPEG4EC_INFO_T m_stMpeg4EncInfo;
    AX_BOOL m_bThreadRunning;
    AX_BOOL m_bLoopCoverRecord;
    AX_U32  m_nFrameRate;
    AX_U32  m_nMaxFileNum;
    AX_U32  m_nMaxFileSize;
    AX_U64  m_nFreeSpace;

    std::string m_sMp4SavedPath;
    std::map<std::string, AX_U64> m_sMp4RecordFiles;

    std::mutex m_mtxMp4;
    std::condition_variable m_cvMp4;

    AX_U32 m_nTimeScale;
    MP4TrackId m_videoId;
    MP4FileHandle m_fileHanele;

private:
    CMPEG4Encoder();
    virtual ~CMPEG4Encoder();
};

#endif // MPEG4_ENCODER_H