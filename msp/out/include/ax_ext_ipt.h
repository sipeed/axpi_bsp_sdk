#ifndef _DEF_AX_EXT_IPT_H_
#define _DEF_AX_EXT_IPT_H_

typedef struct {
    AX_U8 nEnable;    /* GDC Enable. Accuracy: U1.0 Range: [0, 1] */
    AX_S64 nCameraMatrix[9];
    AX_S64 nDistortionCoeff[8];
} AX_EXT_GDC_CFG_T;

#endif