/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef __AI_TRACK_H__
#define __AI_TRACK_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _AI_TRACK_TYPE_E {
    AI_TRACK_TYPE_BODY, /* 0x01 */
    AI_TRACK_TYPE_VEHICLE, /* 0x02 */
    AI_TRACK_TYPE_CYCLE, /* 0x04 */
    AI_TRACK_TYPE_FACE, /* 0x08 */
    AI_TRACK_TYPE_PLATE, /* 0x10 */
    AI_TRACK_TYPE_BUTT
} AI_TRACK_TYPE_E;

//Default track type setting
#define AI_TRACK_BODY_ENABLE (0)
#define AI_TRACK_VEHICLE_ENABLE (0)
#define AI_TRACK_CYCLE_ENABLE (0)
#define AI_TRACK_FACE_ENABLE (1)
#define AI_TRACK_PLATE_ENABLE (1)

#define AI_TRACK_QPLEVEL_DEFAULT (90)

#ifdef __cplusplus
}
#endif

#endif

