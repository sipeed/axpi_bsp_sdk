#ifndef _AX_JOINT_SHORTCUT_H_
#define _AX_JOINT_SHORTCUT_H_

#include "joint.h"

#ifdef __cplusplus
extern "C" {
#endif

AX_S32 AX_JOINT_ShortcutRun(const AX_VOID* pJoint, AX_U32 nJointSize,
                            const AX_VOID* pInput, AX_U32 nInputSize,
                            AX_JOINT_IOMETA_T** pOutputInfos,
                            AX_JOINT_IO_BUFFER_T** pOutputs,
                            AX_U32* pOutputSize);
AX_VOID AX_JOINT_DestroyShortcutRunOutput(AX_JOINT_IOMETA_T* pOutputInfos,
                                          AX_JOINT_IO_BUFFER_T* pOutput,
                                          AX_U32 nOutputSize);

#ifdef __cplusplus
}
#endif

#endif
