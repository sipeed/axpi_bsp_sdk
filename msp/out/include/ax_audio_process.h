/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/


#ifndef _AX_AUDIO_PROCESS_H__
#define _AX_AUDIO_PROCESS_H__

#include "ax_base_type.h"
#include "ax_global_type.h"

typedef enum axAEC_MODE_E {
    AEC_MODE_DISABLE = 0,
    AEC_MODE_FLOAT,     
    AEC_MODE_FIXED, 
} AEC_MODE_E;

typedef enum axSUPPRESSION_LEVEL_E {
  SUPPRESSION_LEVEL_LOW = 0,
  SUPPRESSION_LEVEL_MODERATE,
  SUPPRESSION_LEVEL_HIGH
} SUPPRESSION_LEVEL_E;

typedef struct axAEC_FLOAT_CONFIG_S {
    SUPPRESSION_LEVEL_E enSuppressionLevel;
} AEC_FLOAT_CONFIG_S;

// Recommended settings for particular audio routes. In general, the louder
// the echo is expected to be, the higher this value should be set. The
// preferred setting may vary from device to device.
typedef enum axROUTING_MODE_E {
  ROUTING_MODE_QUITE_EARPIECE_OR_HEADSET = 0,
  ROUTING_MODE_EARPIECE,
  ROUTING_MODE_LOUD_EARPIECE,
  ROUTING_MODE_SPEAKERPHONE,
  ROUTING_MODE_LOUD_SPEAKERPHONE
} ROUTING_MODE_E;

typedef struct axAEC_FIXED_CONFIG_S {
    ROUTING_MODE_E eRoutingMode;
} AEC_FIXED_CONFIG_S;

typedef struct axAEC_CONFIG_S {
    AEC_MODE_E enAecMode;
    union {
        /*0 ~ 2 default 0*/
        AEC_FLOAT_CONFIG_S stAecFloatCfg;
        /*0 ~ 4 default 3*/
        AEC_FIXED_CONFIG_S stAecFixedCfg;
    };
} AEC_CONFIG_S;

// Determines the aggressiveness of the suppression. Increasing the level
// will reduce the noise level at the expense of a higher speech distortion.
typedef enum axAGGRESSIVENESS_LEVEL {
  AGGRESSIVENESS_LEVEL_LOW = 0,
  AGGRESSIVENESS_LEVEL_MODERATE,
  AGGRESSIVENESS_LEVEL_HIGH,
  AGGRESSIVENESS_LEVEL_VERYHIGH
} AGGRESSIVENESS_LEVEL_E;

typedef struct axNS_CONFIG_S {
    AX_BOOL bNsEnable;
    /*0 ~ 3 default 2*/
    AGGRESSIVENESS_LEVEL_E enAggressivenessLevel;
} NS_CONFIG_S;

typedef enum axAGC_MODE_E {
  // Adaptive mode intended for use if an analog volume control is available
  // on the capture device. It will require the user to provide coupling
  // between the OS mixer controls and AGC through the |stream_analog_level()|
  // functions.
  //
  // It consists of an analog gain prescription for the audio device and a
  // digital compression stage.
  AGC_MODE_ADAPTIVE_ANALOG = 0,

  // Adaptive mode intended for situations in which an analog volume control
  // is unavailable. It operates in a similar fashion to the adaptive analog
  // mode, but with scaling instead applied in the digital domain. As with
  // the analog mode, it additionally uses a digital compression stage.
  AGC_MODE_ADAPTIVE_DIGITAL,

  // Fixed mode which enables only the digital compression stage also used by
  // the two adaptive modes.
  //
  // It is distinguished from the adaptive modes by considering only a
  // short time-window of the input signal. It applies a fixed gain through
  // most of the input level range, and compresses (gradually reduces gain
  // with increasing level) the input signal at higher levels. This mode is
  // preferred on embedded devices where the capture signal level is
  // predictable, so that a known gain can be applied.
  AGC_MODE_FIXED_DIGITAL
} AGC_MODE_E;

typedef struct axAGC_CONFIG_S {
    AX_BOOL bAgcEnable;
    AGC_MODE_E enAgcMode;
    /*-31 ~ 0 default -3*/
    AX_S16 s16TargetLevel;
    /*0 ~ 90 default 9*/
    AX_S16 s16Gain;
} AGC_CONFIG_S;

typedef struct axAUDIO_PROCESS_ATTR_S {
    AX_S32                                  s32SampleRate;
    AX_U32                                  u32FrameSamples;

    AEC_CONFIG_S                            stAecCfg;
    NS_CONFIG_S                             stNsCfg;
    AGC_CONFIG_S                            stAgcCfg;
} AUDIO_PROCESS_ATTR_S;

#ifdef __cplusplus
extern "C" {
#endif

AX_S32 AX_AUDIO_PROCESS_Init(const AUDIO_PROCESS_ATTR_S *pstAudioProcessAttr);
AX_S32 AX_AUDIO_PROCESS_Proc(AX_VOID *in_data, AX_VOID *ref_data, AX_VOID *out_data);
AX_S32 AX_AUDIO_PROCESS_DeInit();

AX_VOID AX_AUDIO_InterleavedToNoninterleaved16(AX_S16 *interleaved, AX_U32 frameCount, AX_S16 *left, AX_S16 *right);
AX_VOID AX_AUDIO_MonoToStereo16(AX_S16 *mono, AX_U32 frameCount, AX_S16 *stereo);

#ifdef __cplusplus
}
#endif

#endif /* _AX_AUDIO_PROCESS_H__ */
