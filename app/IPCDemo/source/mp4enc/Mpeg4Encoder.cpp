/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include "Mpeg4Encoder.h"
#include "OptionHelper.h"
#include "global.h"
#include <atomic>
#include <unistd.h>
#include <time.h>
#include <cstdio>
#include <sys/vfs.h>
#include <dirent.h>
#include <regex>
#include <string>
#include <algorithm>
#include<fstream>
#include<iostream>


extern COptionHelper gOptions;
#define MPEG4 "MPEG4"

typedef struct _MP4ENC_NaluUnit {
    AX_U32 type;
    AX_U32 size;
    unsigned char *data;
} MP4ENC_NaluUnit;

AX_S32 ReadOneNaluFromBuf(const unsigned char *buffer, AX_S32 nBufferSize, AX_S32 offSet, MP4ENC_NaluUnit& nalu) {
    AX_S32 i = offSet;
    while (i < nBufferSize) {
        if (buffer[i++] == 0x00 &&
            buffer[i++] == 0x00 &&
            buffer[i++] == 0x00 &&
            buffer[i++] == 0x01 ) {
            AX_S32 pos = i;
            while (pos < nBufferSize) {
                if (buffer[pos++] == 0x00 &&
                    buffer[pos++] == 0x00 &&
                    buffer[pos++] == 0x00 &&
                    buffer[pos++] == 0x01) {
                    break;
                }
            }
            if (pos == nBufferSize) {
               nalu.size = pos - i;
            } else {
                nalu.size = (pos - 4) - i;
            }

            nalu.type = buffer[i]&0x1f;
            nalu.data =(unsigned char*)&buffer[i];
            return (nalu.size + i - offSet);
        }
    }
    return 0;
}


CMPEG4Encoder::CMPEG4Encoder()
    : m_pWriteFrameThread(nullptr)
    , m_bThreadRunning(AX_FALSE)
    , m_bLoopCoverRecord(AX_TRUE)
    , m_nFrameRate(MPEG4_DEFAULT_FRAME_RATE)
{
}

CMPEG4Encoder::~CMPEG4Encoder()
{
    for (int i = 0; i < MAX_MPEG4_CHANNEL_NUM; i++) {
        SAFE_DELETE_PTR(m_pRingBufRawFrame[i]);
    }
}

AX_VOID* CMPEG4Encoder::WriteFrameThreadFunc(AX_VOID* __this)
{
    prctl(PR_SET_NAME, "IPC_MP4_Write");

    AX_U64 nTotalSize = 0;
    AX_U64 nMaxFileSize = 0;
    std::string strFullName;

    CMPEG4Encoder *pThis = (CMPEG4Encoder *)__this;
    pThis->m_bThreadRunning = AX_TRUE;
    pThis->m_cvMp4.notify_all();

    LOG_M(MPEG4, "+++");

    // Make sure mp4 saved path exist
    if (access(pThis->m_sMp4SavedPath.c_str(), 0) != 0) { //path not exist, try make dir
        if (mkdir(pThis->m_sMp4SavedPath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) != 0) {  // mkdir fail, exit
            LOG_M(MPEG4, "Could not create mp4 saved path: %s", pThis->m_sMp4SavedPath.c_str());
            return nullptr;
        }
    }

    nMaxFileSize = pThis->m_nMaxFileSize << 20;
    while (pThis->m_bThreadRunning) {
        if (!pThis->m_bLoopCoverRecord && (pThis->m_sMp4RecordFiles.size() >= pThis->m_nMaxFileNum)) {
            break;
        }

        nTotalSize = 0;
        AX_CHAR szFileName[MAX_NAME_LEN] = {0};
        AX_U64 nCurrAvailSpace = pThis->GetAvailableSpace();
        strFullName = pThis->m_sMp4SavedPath + pThis->GenFileName(szFileName);

        if ((pThis->m_sMp4RecordFiles.size() >= pThis->m_nMaxFileNum)
            || (nCurrAvailSpace < nMaxFileSize)) {
            if (!pThis->RemoveFilesForSpace(nCurrAvailSpace)) {
                break;
            }
        }

        pThis->m_sMp4RecordFiles.insert(std::make_pair(strFullName, nMaxFileSize));
        LOG_M(MPEG4, "New record file name: %s", strFullName.c_str());
        const AX_S32 MP4_TIMESCALE = 90000;
        MP4FileHandle fileHandle = pThis->CreateMP4File(strFullName.c_str(), MP4_TIMESCALE);
        AX_U64 videoIndex = 0;
        AX_U64 nBasePts = 0;
        while (pThis->m_bThreadRunning) {
            CAXRingElement* pElement = nullptr;
            pElement = pThis->m_pRingBufRawFrame[0]->Get();
            if (!pElement)
            {
                CTimeUtils::msSleep(10);
                continue;
            }

            if (nTotalSize >= nMaxFileSize && pElement->bIFrame) {
                LOG_M(MPEG4, "Reach file max size, nTotalSize: %llu", nTotalSize);
                break;
            }

            if (0 == videoIndex) {
                nBasePts = pElement->nPts - 1000000 / pThis->m_nFrameRate;
            }
            ++videoIndex;

            nTotalSize += pElement->nSize;
            if (pThis->WriteH264Data(fileHandle, pElement, nBasePts) < 0) {
                break;
            }
            pThis->m_pRingBufRawFrame[0]->Pop();

            nBasePts = pElement->nPts;
        }
        LOG_M(MPEG4, "write record file: %s trailer, videoIndex: %llu", strFullName.c_str(), videoIndex);
        pThis->SetRecordFileSizeInfo(strFullName, nTotalSize + MP4_HEAD_TAIL_SIZE);
        pThis->CloseMP4File(fileHandle);
        LOG_M(MPEG4, "write record file: %s done, videoIndex: %llu", strFullName.c_str(), videoIndex);
    }

    LOG_M(MPEG4, "---");

    return nullptr;
}

AX_BOOL CMPEG4Encoder::Start()
{
    std::unique_lock<std::mutex> lck(m_mtxMp4);

    LOG_M(MPEG4, "+++");

    if (!m_pWriteFrameThread) {
        // update recording strategy params
        UpdateRecordStrategyParams();

        if (0 == m_nMaxFileNum || 0 == m_nMaxFileSize) {
            LOG_M_E(MPEG4, "CMPEG4Encoder start failed for m_nMaxFileNum: %d or m_nMaxFileSize: %dMB is zero!!!", m_nMaxFileNum, m_nMaxFileSize);
            return AX_FALSE;
        }

        m_bThreadRunning = AX_FALSE;
        m_pWriteFrameThread = new thread(WriteFrameThreadFunc, this);

        // cv wait thread start
        m_cvMp4.wait_for(lck, std::chrono::milliseconds(SECOND_5S), [this]() { return (m_bThreadRunning ? true : false); });
    }

    LOG_M(MPEG4, "---");
    return (m_pWriteFrameThread && m_bThreadRunning) ? AX_TRUE: AX_FALSE;
}

AX_VOID CMPEG4Encoder::Stop()
{
    std::unique_lock<std::mutex> lck(m_mtxMp4);

    LOG_M(MPEG4, "+++");

    m_bThreadRunning = AX_FALSE;
    if (m_pWriteFrameThread) {
        if (m_pWriteFrameThread->joinable()) {
            m_pWriteFrameThread->join();
        }

        delete m_pWriteFrameThread;
        m_pWriteFrameThread = nullptr;
    }

    DropFrames();

    LOG_M(MPEG4, "---");
    return;
}

AX_VOID CMPEG4Encoder::SendRawFrame(AX_U8 nChn, AX_VOID* data, AX_U32 size, AX_U64 nPts/*=0*/, AX_BOOL bIFrame/*=AX_FALSE*/)
{
    if (0 != nChn || !m_bThreadRunning) {
        return;
    }

    CAXRingElement ele((AX_U8*)data, size, nChn, nPts, bIFrame);
    m_pRingBufRawFrame[0]->Put(ele);
}

AX_BOOL CMPEG4Encoder::Init()
{
    /*
        global init process, only be invoked once by "Singleton" template class
    */
    if (!gOptions.IsEnableMp4Record()) {
        LOG_M(MPEG4, "MP4 recorder is disabled.");
        return AX_TRUE;
    }

    AX_U32 nMaxFileSize = 0, nMaxFileNum = 0;
    AX_U64 nRecordFilesSize = 0;

    gOptions.GetMp4FileInfo(nMaxFileSize, nMaxFileNum);

    m_nMaxFileNum = (0 == nMaxFileNum) ? MP4_DEFAULT_RECORD_FILE_NUM : nMaxFileNum;
    m_nMaxFileSize = (0 == nMaxFileSize) ? MP4_DEFAULT_FILE_SIZE : (nMaxFileSize * 1024 * 1024);
    m_sMp4SavedPath = gOptions.GetMp4SavedPath();
    if (m_sMp4SavedPath[m_sMp4SavedPath.length() - 1] != '/') {
        m_sMp4SavedPath += '/';
    }

    LOG_M(MPEG4, "Param set info - (nMaxFileNum, nMaxFileSize, sMp4SavedPath):(%d, %dMB, %s)", nMaxFileNum, nMaxFileSize, m_sMp4SavedPath.c_str());

    m_sMp4RecordFiles = GetRecorderFiles(m_sMp4SavedPath, MPEG4_FILE_NAME_PATTERN, nRecordFilesSize);
    m_nFreeSpace = GetFreeSpaceForMp4Record(m_sMp4SavedPath, nRecordFilesSize);
    LOG_M(MPEG4, "Exist record file info: (number:%d, size:%lluB), Max space for record: %lluMB", m_sMp4RecordFiles.size(), nRecordFilesSize, (m_nFreeSpace >> 20));

    // Calculate max file number according to free space
    nMaxFileNum = m_nFreeSpace / m_nMaxFileSize;
    m_nMaxFileNum = (nMaxFileNum < m_nMaxFileNum) ? nMaxFileNum : m_nMaxFileNum;
    LOG_M(MPEG4, "m_nMaxFileNum: %d, m_nMaxFileSize: %dMB", m_nMaxFileNum, (m_nMaxFileSize >> 20));

    gOptions.SetMp4FileInfo((m_nMaxFileSize >> 20), m_nMaxFileNum);
    gOptions.SetMp4TotalSpace(m_nFreeSpace >> 20);
    LOG_M(MPEG4, "Param actual info - (nMaxFileNum, nMaxFileSize, sMp4SavedPath):(%d, %dMB, %s)", m_nMaxFileNum, (m_nMaxFileSize >> 20), m_sMp4SavedPath.c_str());

    return AX_TRUE;
}

AX_VOID CMPEG4Encoder::InitParam(const MPEG4EC_INFO_T& stMpeg4Info)
{
    if (gOptions.IsEnableMp4Record()) {
        m_stMpeg4EncInfo = stMpeg4Info;
        m_nFrameRate = (m_stMpeg4EncInfo.nFrameRate < MPEG4_DEFAULT_FRAME_RATE) ? MPEG4_DEFAULT_FRAME_RATE : m_stMpeg4EncInfo.nFrameRate;
#ifdef AX_SIMPLIFIED_MEM_VER
        AX_U32 nRingbufSize = m_stMpeg4EncInfo.nfrWidth * m_stMpeg4EncInfo.nfrHeight / 8;
#else
        AX_U32 nRingbufSize = m_stMpeg4EncInfo.nfrWidth * m_stMpeg4EncInfo.nfrHeight * 3 / 16; // (YUV SIZE / 8)
#endif
        LOG_M(MPEG4, "Frame width: %d, height: %d, Ring buffer size: %d, Ring buffer Num:%d",
                    m_stMpeg4EncInfo.nfrWidth, m_stMpeg4EncInfo.nfrHeight, nRingbufSize, MP4_RINGBUF_DATA_SIZE);
        for (int i = 0; i < MAX_MPEG4_CHANNEL_NUM; i++) {
            m_pRingBufRawFrame[i] = new CAXRingBuffer(nRingbufSize, MP4_RINGBUF_DATA_SIZE, MPEG4);
        }
    }
}

AX_CHAR* CMPEG4Encoder::GenFileName(AX_CHAR* szFileName) {
    time_t rawtime;
    struct tm *ptminfo;

    time(&rawtime);
    ptminfo = localtime(&rawtime);
    snprintf((char *)szFileName, MAX_NAME_LEN, "%s_%02d-%02d-%02d-%02d-%02d-%02d.%s", \
              MPEG4_FILE_NAME_TAG, ptminfo->tm_year + 1900, ptminfo->tm_mon + 1, \
              ptminfo->tm_mday, ptminfo->tm_hour, ptminfo->tm_min, ptminfo->tm_sec, \
              MP4_FORMAT_NAME);
    return szFileName;
}

std::map<std::string, AX_U64> CMPEG4Encoder::GetRecorderFiles(std::string path, std::string suffix, AX_U64 &totalSize)
{
    std::map<std::string, AX_U64> files;
    DIR* dp = nullptr;
    struct dirent *dirp;
    struct stat stStat;
    totalSize = 0;

    if ((dp = opendir(path.c_str())) == NULL) {
        LOG_M(MPEG4, "Can not open: %s", path.c_str());
        return files;
    }

    regex reg_obj(suffix);
    while ((dirp = readdir(dp)) != NULL)
    {
        if (dirp->d_type == 8) {
            // 4 means catalog; 8 means file; 0 means unknown
            if (regex_match(dirp->d_name, reg_obj)) {
                string all_path = path + dirp->d_name;
                stat(all_path.c_str(), &stStat);
                if (stStat.st_size <= 0) {  // remove empty file
                    remove(all_path.c_str());
                } else {
                    totalSize += stStat.st_size;
                    files.insert(std::make_pair(all_path, stStat.st_size));
                }
            }
        }
    }
    closedir(dp);

    return files;
}

AX_U64 CMPEG4Encoder::GetFreeSpaceForMp4Record(const std::string& path, const AX_U64 nRecordFilesSize)
{
    struct statfs diskInfo = {0};
    AX_U64 nBlockSize = 0;
    AX_U64 nFreeSpace = 0;

    if (statfs(path.c_str(), &diskInfo)) {
        LOG_M_E(MPEG4, "Error, Get free space for path: %s failed!", path.c_str());
        return 0;
    }

    nBlockSize = diskInfo.f_bsize;
    nFreeSpace = diskInfo.f_bfree * nBlockSize;

    LOG_M(MPEG4, "nFreeSpace [%llu(B)], reserved [%d(MB)], path: %s", nFreeSpace, (MP4_RECORD_SPACE_RESERVED >> 20), path.c_str());

    return (nFreeSpace + nRecordFilesSize > MP4_RECORD_SPACE_RESERVED) ? (nFreeSpace + nRecordFilesSize - MP4_RECORD_SPACE_RESERVED) : 0;
}

AX_U64 CMPEG4Encoder::GetAvailableSpace()
{
    AX_U64 nTotalFileSize = 0;
    AX_U64 nAvailSize = 0;
    for (auto &file : m_sMp4RecordFiles) {
        nTotalFileSize += file.second;
    }

    nAvailSize = m_nFreeSpace - nTotalFileSize + MP4_RECORD_SPACE_MARGIN;

    return nAvailSize;
}

AX_BOOL CMPEG4Encoder::RemoveFilesForSpace(const AX_U64& nCurrAvailableSpace)
{
    if (m_sMp4RecordFiles.empty()) {
        LOG_M(MPEG4, "Error, m_sMp4RecordFiles empty");
        return AX_FALSE;
    }

    AX_U64 nAvailSize = nCurrAvailableSpace;
    while (!m_sMp4RecordFiles.empty()) {
        if (remove(m_sMp4RecordFiles.begin()->first.c_str())) {
            LOG_M(MPEG4, "Remove file: %s failed, errno: %s", m_sMp4RecordFiles.begin()->first.c_str(), strerror(errno));
            return AX_FALSE;
        }
        LOG_M(MPEG4, "Remove file: %s, size: %llu", m_sMp4RecordFiles.begin()->first.c_str(), m_sMp4RecordFiles.begin()->second);
        nAvailSize += m_sMp4RecordFiles.begin()->second;
        m_sMp4RecordFiles.erase(m_sMp4RecordFiles.begin());

        if (nAvailSize >= m_nMaxFileSize && (m_sMp4RecordFiles.size() < m_nMaxFileNum)) {
            break;
        }
    }

    return AX_TRUE;
}

AX_VOID CMPEG4Encoder::DropFrames()
{
    CAXRingElement *pEle = nullptr;

    do {
        pEle = m_pRingBufRawFrame[0]->Get();
        if (!pEle) {
            LOG_M(MPEG4, "Drop frames done!");
            break;
        }

        m_pRingBufRawFrame[0]->Pop();

    } while(pEle);
}

AX_VOID CMPEG4Encoder::SetRecordFileSizeInfo(const std::string& strFileName, const AX_U64& nFileSize)
{
    std::map<std::string, AX_U64>::iterator iter;

    iter = m_sMp4RecordFiles.find(strFileName);

    if (iter != m_sMp4RecordFiles.end()) {
        iter->second = nFileSize;
    } else {
        LOG_M(MPEG4, "Can not find file: %s", strFileName.c_str());
    }
}

AX_VOID CMPEG4Encoder::UpdateRecordStrategyParams()
{
    m_bLoopCoverRecord = gOptions.IsLoopCoverMp4Record();
    gOptions.GetMp4FileInfo(m_nMaxFileSize, m_nMaxFileNum);
}

MP4FileHandle CMPEG4Encoder::CreateMP4File(const char* pFileName, AX_S32 timeScale/* = 90000*/)
{
    if(pFileName == NULL) {
        return nullptr;
    }
    // create mp4 file
    MP4FileHandle hMp4file = MP4Create(pFileName);
    if (hMp4file == MP4_INVALID_FILE_HANDLE) {
        return nullptr;
    }

    m_nTimeScale = timeScale;
    MP4SetTimeScale(hMp4file, m_nTimeScale);
    m_videoId = MP4_INVALID_TRACK_ID;
    return hMp4file;
}

AX_S32 CMPEG4Encoder::WriteH264Data(MP4FileHandle hMp4File, CAXRingElement* pData, AX_U64 nBasePts)
{
    if ((hMp4File == NULL) || (pData == NULL)) {
        return -1;
    }
    MP4ENC_NaluUnit nalu;
    AX_S32 pos = 0;
    AX_S32 len = 0;
    while ((len = ReadOneNaluFromBuf(pData->pBuf, pData->nSize, pos, nalu)) > 0) {
       if (nalu.type == 0x07) {
            // sps
            if (m_videoId == MP4_INVALID_TRACK_ID) {
                m_videoId = MP4AddH264VideoTrack(hMp4File,
                    m_nTimeScale,
                    m_nTimeScale / m_nFrameRate,
                    m_stMpeg4EncInfo.nfrWidth,     // width
                    m_stMpeg4EncInfo.nfrHeight,    // height
                    nalu.data[1], // sps[1] AVCProfileIndication
                    nalu.data[2], // sps[2] profile_compat
                    nalu.data[3], // sps[3] AVCLevelIndication
                    3);           // 4 bytes length before each NAL unit
                if (m_videoId == MP4_INVALID_TRACK_ID)  {
                    return -1;
                }
            }

            MP4SetVideoProfileLevel(hMp4File, 1); //  Simple Profile @ Level 3
            MP4AddH264SequenceParameterSet(hMp4File, m_videoId, nalu.data, nalu.size);
        } else if (nalu.type == 0x08) {
             // pps
            MP4AddH264PictureParameterSet(hMp4File, m_videoId, nalu.data, nalu.size);
        } else {
            if (m_videoId == MP4_INVALID_TRACK_ID) {
                pos += len;
                continue;
            }

            AX_S32 datalen = nalu.size + 4;
            unsigned char *data = new unsigned char[datalen];
            data[0] = nalu.size>>24;
            data[1] = nalu.size>>16;
            data[2] = nalu.size>>8;
            data[3] = nalu.size&0xff;
            memcpy(data+4, nalu.data, nalu.size);
            if (!MP4WriteSample(hMp4File, m_videoId, data, datalen, (pData->nPts - nBasePts) * m_nTimeScale / 1000000, 0, 1)) {
                delete[] data;
                return -1;
            }
            delete[] data;
        }
        pos += len;
    }
    return pos;
}

void CMPEG4Encoder::CloseMP4File(MP4FileHandle& hMp4File)
{
    if (hMp4File) {
        MP4Close(hMp4File);
        hMp4File = nullptr;
    }
}
