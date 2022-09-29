/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef __AI_DETECTION_H__
#define __AI_DETECTION_H__

#ifdef __cplusplus
extern "C" {
#endif

#define DETECT_DEFAULT_WIDTH 1280
#define DETECT_DEFAULT_HEIGHT 720
#define DETECT_DEFAULT_IVES_FRAMERATE_CTRL 1
#define DETECT_DEFAULT_AI_FRAMERATE_CTRL 5
#define DETECT_DEFAULT_FRAMERATE_CTRL 15

#define DETECT_DEFAULT_HOTBALANCE_IVES_FRAMERATE_CTRL 1
#define DETECT_DEFAULT_HOTBALANCE_AI_FRAMERATE_CTRL 2
#define DETECT_DEFAULT_HOTBALANCE_FRAMERATE_CTRL 6

#define AI_MAX_TRACK_HUMAN_SIZE (0) //no limit
#define AI_MAX_TRACK_VEHICLE_SIZE (0) //no limit
#define AI_MAX_TRACK_CYCLE_SIZE (0) //no limit

#define DETECT_MARGIN_GAP_TIME 5 // 5ms

typedef enum _AI_DRAW_RECT_TYPE_E {
    AI_DRAW_RECT_TYPE_BODY, /* 0x01 */
    AI_DRAW_RECT_TYPE_VEHICLE, /* 0x02 */
    AI_DRAW_RECT_TYPE_CYCLE, /* 0x04 */
    AI_DRAW_RECT_TYPE_FACE, /* 0x08 */
    AI_DRAW_RECT_TYPE_PLATE, /* 0x10 */
    AI_DRAW_RECT_TYPE_BUTT
} AI_DRAW_RECT_TYPE_E;

//Default draw rect type setting
#define AI_DRAW_RECT_BODY_ENABLE (0)
#define AI_DRAW_RECT_VEHICLE_ENABLE (1)
#define AI_DRAW_RECT_CYCLE_ENABLE (1)
#define AI_DRAW_RECT_FACE_ENABLE (1)
#define AI_DRAW_RECT_PLATE_ENABLE (1)

//Default body fliter attribute
#define AI_BODY_FLITER_DEFAULT_WIDTH 0
#define AI_BODY_FLITER_DEFAULT_HEIGHT 0
#define AI_BODY_FLITER_DEFAULT_CONFIDENCE 0

//Default vehicle fliter attribute
#define AI_VEHICLE_FLITER_DEFAULT_WIDTH 0
#define AI_VEHICLE_FLITER_DEFAULT_HEIGHT 0
#define AI_VEHICLE_FLITER_DEFAULT_CONFIDENCE 0

//Default cycle fliter attribute
#define AI_CYCLE_FLITER_DEFAULT_WIDTH 0
#define AI_CYCLE_FLITER_DEFAULT_HEIGHT 0
#define AI_CYCLE_FLITER_DEFAULT_CONFIDENCE 0

//Default face fliter attribute
#define AI_FACE_FLITER_DEFAULT_WIDTH 0
#define AI_FACE_FLITER_DEFAULT_HEIGHT 0
#define AI_FACE_FLITER_DEFAULT_CONFIDENCE 0

//Default plate fliter attribute
#define AI_PLATE_FLITER_DEFAULT_WIDTH 0
#define AI_PLATE_FLITER_DEFAULT_HEIGHT 0
#define AI_PLATE_FLITER_DEFAULT_CONFIDENCE 0

/* Not Support */
typedef struct _AI_Body_Attr_t {
    AX_BOOL bExist;
    std::string strSafetyCap;
    std::string strHairLength;

    _AI_Body_Attr_t() {
        bExist = AX_FALSE;
        strSafetyCap = "";
        strHairLength = "";
    }
} AI_Body_Attr_t;

/* Not Support */
typedef struct _AI_Vehicle_Attr_t {
    AX_BOOL bExist;
    std::string strVehicleColor;
    std::string strVehicleSubclass;

    _AI_Vehicle_Attr_t() {
        bExist = AX_FALSE;
        strVehicleColor = "";
        strVehicleSubclass = "";
    }
} AI_Vehicle_Attr_t;

/* Not Support */
typedef struct _AI_Cycle_Attr_t {
    AX_BOOL bExist;
    std::string strCycleSubclass;

    _AI_Cycle_Attr_t() {
        bExist = AX_FALSE;
        strCycleSubclass = "";
    }
} AI_Cycle_Attr_t;

typedef struct _AI_Face_Attr_t {
    AX_BOOL bExist;
    AX_U8 nAge;
    /* 0: female 1: male */
    AX_U8 nGender;
    /*
    string:
        no_respirator
        surgical
        anti-pollution
        common
        kitchen_transparent
        unknown
    */
    std::string strRespirator;

    _AI_Face_Attr_t() {
        bExist = AX_FALSE;
        nAge = 0;
        nGender = 0;
        strRespirator = "";
    }
} AI_Face_Attr_t;

typedef struct _AI_Plat_Attr_t {
    AX_BOOL bExist;
    AX_BOOL bValid;
    /*
    string:
        blue
        yellow
        black
        white
        green
        small_new_energy
        large_new_energy
        absence
        unknown
    */
    std::string strPlateColor;
    /*
    string:
        one_row
        two_rows
        unknown
    */
    std::string strPlateType;
    /* string: UTF8*/
    std::string strPlateCode;

    _AI_Plat_Attr_t() {
        bExist = AX_FALSE;
        bValid = AX_FALSE;
        strPlateColor = "";
        strPlateType = "";
        strPlateCode = "";
    }
} AI_Plat_Attr_t;

typedef struct _AI_Detection_Box_t {
    AX_F32 fX, fY, fW, fH;
} AI_Detection_Box_t;

typedef struct _AI_Detection_BodyResult_t {
    AI_Detection_Box_t tBox;  // 0-1 relative
    AI_Body_Attr_t tBodyAttr;
    AX_F32 fConfidence;
    AX_U64 u64TrackId;

    _AI_Detection_BodyResult_t() {
        memset(&tBox, 0x00, sizeof(tBox));
        fConfidence = 0;
        u64TrackId = 0;
    }
} AI_Detection_BodyResult_t;

typedef struct _AI_Detection_VehicleResult_t {
    AI_Detection_Box_t tBox;  // 0-1 relative
    AI_Vehicle_Attr_t tVehicleAttr;
    AX_F32 fConfidence;
    AX_U64 u64TrackId;

    _AI_Detection_VehicleResult_t() {
        memset(&tBox, 0x00, sizeof(tBox));
        fConfidence = 0;
        u64TrackId = 0;
    }
} AI_Detection_VehicleResult_t;

typedef struct _AI_Detection_CycleResult_t {
    AI_Detection_Box_t tBox;  // 0-1 relative
    AI_Cycle_Attr_t tCycleAttr;
    AX_F32 fConfidence;
    AX_U64 u64TrackId;

    _AI_Detection_CycleResult_t() {
        memset(&tBox, 0x00, sizeof(tBox));
        fConfidence = 0;
        u64TrackId = 0;
    }
} AI_Detection_CycleResult_t;

typedef struct _AI_Detection_FaceResult_t {
    AI_Detection_Box_t tBox;  // 0-1 relative
    AI_Face_Attr_t tFaceAttr;
    AX_F32 fConfidence;
    AX_U64 u64TrackId;

    _AI_Detection_FaceResult_t() {
        memset(&tBox, 0x00, sizeof(tBox));
        fConfidence = 0;
        u64TrackId = 0;
    }
} AI_Detection_FaceResult_t;

typedef struct _AI_Detection_PlateResult_t {
    AI_Detection_Box_t tBox;  // 0-1 relative
    AI_Plat_Attr_t tPlateAttr;
    AX_F32 fConfidence;
    AX_U64 u64TrackId;

    _AI_Detection_PlateResult_t() {
        memset(&tBox, 0x00, sizeof(tBox));
        fConfidence = 0;
        u64TrackId = 0;
    }
} AI_Detection_PlateResult_t;

/* Donot use memset/memcpy */
typedef struct _AI_Detection_Result_t {
    AX_S32 nErrorCode;
    AX_U32 nFrameId;
    AI_Detection_BodyResult_t* pBodys;
    AX_U32 nBodySize;
    AI_Detection_VehicleResult_t* pVehicles;
    AX_U32 nVehicleSize;
    AI_Detection_CycleResult_t* pCycles;
    AX_U32 nCycleSize;
    AI_Detection_FaceResult_t* pFaces;
    AX_U32 nFaceSize;
    AI_Detection_PlateResult_t* pPlates;
    AX_U32 nPlateSize;

    _AI_Detection_Result_t() {
        nErrorCode = 0;
        nFrameId = 0;
        nBodySize = 0;
        pBodys = nullptr;
        nVehicleSize = 0;
        pVehicles = nullptr;
        nCycleSize = 0;
        pCycles = nullptr;
        nFaceSize = 0;
        pFaces = nullptr;
        nPlateSize = 0;
        pPlates = nullptr;
    }
} AI_Detection_Result_t;

typedef struct _AI_CONFIG_T
{
    AX_U32 nWidth;
    AX_U32 nHeight;
    AX_U32 nDetectFps;
    AX_U32 nAiFps;
    AX_U32 nIvesFps;
} AI_CONFIG_T, *AI_CONFIG_PTR;

#ifdef __cplusplus
}
#endif

#endif
