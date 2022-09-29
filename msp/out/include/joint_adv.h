#ifndef _AX_JOINT_ADV_H_
#define _AX_JOINT_ADV_H_

#include "joint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _AX_JOINT_COMPONENT_TYPE_E {
  AX_JOINT_COMPONENT_TYPE_UNKNOWN = 0,
  AX_JOINT_COMPONENT_TYPE_NEU = 1,
  AX_JOINT_COMPONENT_TYPE_ONNX = 2,
  AX_JOINT_COMPONENT_TYPE_MAGMA = 3,
  AX_JOINT_COMPONENT_TYPE_AXE = 4
} AX_JOINT_COMPONENT_TYPE_T;

typedef struct _AX_JOINT_COMPONENT_PROFILE_T {
  AX_U32 nTotalUs;
  AX_U32 nCoreUs;  // XXX e.g. for neu, this records interpreter's consumption
  AX_U32 nInitUs;
  AX_U64 u64Reserved[6];
} AX_JOINT_COMPONENT_PROFILE_T;

typedef struct _ax630a_joint_component {
  AX_JOINT_COMPONENT_TYPE_T eType;
  const AX_CHAR *pName;
  AX_JOINT_COMPONENT_PROFILE_T tProfile;
  AX_NPU_SDK_EX_MODEL_TYPE_T eVNPUMode;
#if defined(__aarch64__)
  AX_U64 u64Reserved[5];
#elif defined(__arm__)
  AX_U64 u64Reserved[6];
#endif
} AX_JOINT_COMPONENT_T;

/*! experimental
 * \param components: output parameter, as list of component, whose memory is
 * owned by `context` \param size: size of `*components`
 */
AX_S32 AX_JOINT_ADV_GetComponents(AX_JOINT_EXECUTION_CONTEXT context,
                                  AX_JOINT_COMPONENT_T **pComponents,
                                  AX_U32 *nSize);

#ifdef __cplusplus
}
#endif

#endif
