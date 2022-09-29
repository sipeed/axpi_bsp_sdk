/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef _OPTION_HELPER_C8C7426B_2B0E_45F4_AA65_BCB2834187DC_H_
#define _OPTION_HELPER_C8C7426B_2B0E_45F4_AA65_BCB2834187DC_H_

#include "global.h"
#include <string>
#include <mutex>

/* HVCFP Detection */
#define MAX_DECT_BOX_COUNT 10

/* Donot use memset/memcpy */
typedef struct _DETECT_RESULT_T {
    AX_U32 nFrameId;
    AI_Detection_FaceResult_t tFaces[MAX_DECT_BOX_COUNT];
    AX_U32 nFaceSize;
    AI_Detection_BodyResult_t tBodys[MAX_DECT_BOX_COUNT];
    AX_U32 nBodySize;
    AI_Detection_VehicleResult_t tVehicles[MAX_DECT_BOX_COUNT];
    AX_U32 nVehicleSize;
    AI_Detection_PlateResult_t tPlates[MAX_DECT_BOX_COUNT];
    AX_U32 nPlateSize;
    AI_Detection_CycleResult_t tCycles[MAX_DECT_BOX_COUNT];
    AX_U32 nCycleSize;

    _DETECT_RESULT_T() {
        Clear();
    }

    AX_VOID Clear() {
        nFrameId = 0;
        nFaceSize = 0;
        nBodySize = 0;
        nVehicleSize = 0;
        nPlateSize = 0;
        nCycleSize = 0;
    }

    AX_VOID Print() {
        #define ObjectPrint(Obj) \
            do { \
                printf("%s size: %d\n", #Obj, n##Obj##Size); \
                for (AX_U32 i = 0; i < n##Obj##Size; ++i) { \
                    printf("%s[%d]: (%.2f, %.2f, %.2f, %.2f)\n", #Obj, i, t##Obj##s[i].tBox.fX, t##Obj##s[i].tBox.fY, t##Obj##s[i].tBox.fW, t##Obj##s[i].tBox.fH); \
                } \
            } while(0)

        printf("FrameId: %d detection result:\n", nFrameId);
        ObjectPrint(Face);
        ObjectPrint(Body);
        ObjectPrint(Vehicle);
        ObjectPrint(Plate);
        ObjectPrint(Cycle);
    }
} DETECT_RESULT_T;

/**
 * Load configuration
 */
class COptionHelper final
{
public:
    COptionHelper(void);
    ~COptionHelper(void);

    AX_BOOL ParseArgs(AX_S32 argc, AX_CHAR const *argv[]);

    /* json config */
    const std::string &GetJsonFile(void)const;

    /* log control */
    AX_U32  GetLogTarget(void)const;
    AX_U32  GetLogLevel(void)const;
    AX_BOOL IsPrintFPS() const;

    /* yuv detection */
    AX_BOOL IsActivedDetect(void) const;
    AX_BOOL IsActivedDetectFromWeb(void) const;
    AX_BOOL IsActivedTrack(void) const;
    AX_BOOL IsActivedSearch(void) const;
    AX_BOOL IsActivedSearchFromWeb(void) const;
    AX_BOOL IsLinkMode(void)const;
    AX_BOOL IsEnableDewarp(void)const;
    AX_BOOL IsActivedOcclusionDetect(void) const;
    AX_BOOL IsActivedMotionDetect(void) const;
    AX_U32  GetDetectInputStragy(void)const;

    AX_BOOL IsEnableAutoSleep(void) const;
    AX_U64  GetAutoSleepFrameNum(void) const;
    AX_U64  GetVenc0SeqNum() const;
    void    SetVenc0SeqNum(AX_U64 nSeqNum);

    AX_VOID SetDetectActived(AX_BOOL bActive);
    AX_VOID SetTrackActived(AX_BOOL bActive);
    AX_VOID SetNrEnabled(AX_BOOL bEnable);
    AX_VOID SetSearchActived(AX_BOOL bActive);
    AX_VOID ActiveOcclusionDetect(AX_BOOL bActive);
    AX_VOID ActiveMotionDetect(AX_BOOL bActive);

    AX_VOID SetDetectResult(AX_U32 nPipeID, AI_Detection_Result_t *pResult);
    DETECT_RESULT_T GetDetectResult(AX_U32 nPipeID);

    const std::string &GetDetectionModelPath(void) const;
    const std::string &GetDetectionAlgoType(void) const;
    const std::string &GetDetectionConfigPath(void) const;
    const std::string &GetSearchConfigPath(void) const;

    /* recording(mp4 format) */
    AX_BOOL IsEnableMp4Record() const;
    AX_BOOL IsActiveMp4Record() const;
    AX_VOID SetMp4RecordActived(AX_BOOL bActive);
    AX_BOOL IsLoopCoverMp4Record() const;
    AX_VOID SetLoopCoverMp4Record(AX_BOOL bLoopCover);
    AX_VOID GetMp4FileInfo(AX_U32 & nMaxFileSize, AX_U32 & nMaxFileNum) const;
    AX_VOID SetMp4FileInfo(AX_U32 nMaxFileSize, AX_U32 nMaxFileNum);
    AX_U64  GetMp4TotalSpace(void)const;
    AX_VOID SetMp4TotalSpace(AX_U64 nTotalSpace);
    const std::string &GetMp4SavedPath(void)const;
    AX_VOID SetMp4SavedPath(AX_CHAR* pszPath);
    const std::string &GetTtfPath(void) const;

    /* rtsp config */
    AX_U32  GetRTSPMaxFrmSize(void)const;

    /* web config */
    AX_F32  GetWebFrmSizeRatio(void)const;

    /* EIS */
    AX_BOOL IsEnableEIS(void) const;
    AX_VOID SetEISEnable(AX_BOOL bEnable);
    AX_U32  GetEISDelayNum() const;
    AX_U32  GetEISCropW() const;
    AX_U32  GetEISCropH() const;
    AX_BOOL IsEnableEISEffectComp(void) const;
    AX_BOOL IsEISSupport(void) const;
    AX_VOID SetEISSupport(AX_BOOL bSupport);

    AX_IVPS_ROTATION_E GetRotation() const;
    AX_BOOL GetMirror() const;
    AX_BOOL GetFlip() const;

    /* etc. */
    AX_U32  GetSnsNum()const;
    AX_U32  GetSensorID()const;
    AX_BOOL IsEnableOSD(void)const;

private:
    AX_U32  m_nLogTarget;
    AX_U32  m_nLogLevel;
    AX_U32  m_nRTSPMaxFrmSize;
    AX_F32  m_nWebFrmSizeRatio;
    AX_BOOL m_bActiveDetect;
    AX_BOOL m_bActiveDetectFromWeb;
    AX_BOOL m_bActiveTrack;
    AX_BOOL m_bActiveSearch;
    AX_BOOL m_bActiveSearchFromWeb;
    AX_BOOL m_bActiveOD;
    AX_BOOL m_bActiveMD;
    AX_BOOL m_bNrEnable;
    AX_BOOL m_bPrintFPS;
    AX_BOOL m_bEnableDetect;
    AX_BOOL m_bUseWebDeamon;
    AX_BOOL m_bMp4Record;
    AX_BOOL m_bActiveMp4Record;
    AX_BOOL m_bLoopCoverMp4Record;
    AX_BOOL m_bEnableDewarp;
    AX_BOOL m_bEnableOSD;
    AX_U32  m_nSensorID;
    AX_BOOL m_bLinkMode;
    AX_U32  m_nMp4MaxFileNum;
    AX_U32  m_nMp4MaxFileSize;
    AX_U64  m_nMp4TotalSpace;
    AX_U32  m_nDetectInputStrgy;
    AX_BOOL m_bEnableAutoSleep;
    AX_U64  m_nAutoSleepFrameNum;
    AX_U64  m_nVenc0SeqNum;
    AX_BOOL m_nEnableEIS;
    AX_U32  m_nEISDelayNum;
    AX_U32  m_nEISCropW;
    AX_U32  m_nEISCropH;
    AX_BOOL m_nEnableEISEffectComp;
    AX_BOOL m_nEISSupport;
    AX_IVPS_ROTATION_E m_eRotation;
    AX_BOOL m_bMirror;
    AX_BOOL m_bFlip;
    std::string m_strJsonCfgFile;
    std::string m_strDetectionModel;
    std::string m_strMp4SavedPath;
    std::string m_strDetectAlgoType;
    std::string m_strDetectConfigPath;
    std::string m_strTtfFile;

    DETECT_RESULT_T m_arrDetectResult[MAX_SNS_NUM];

    std::mutex m_mtxOption;
};


#endif /* _OPTION_HELPER_C8C7426B_2B0E_45F4_AA65_BCB2834187DC_H_ */
