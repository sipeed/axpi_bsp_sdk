#ifndef __SAMPLE_UTILS_H__
#define __SAMPLE_UTILS_H__

#include "common_venc.h"


#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

#define DEFAULT -255




#if 0
/* SVC-T Configure */
static AX_U32 gSvcTGopSize = 4;
/*SVC-T GOP4*/
static char *gSvcTCfg[] = {
  "Frame1:  P      1      0       0.4624        2        1           -1          1",
  "Frame2:  P      2      0       0.4624        1        1           -2          1",
  "Frame3:  P      3      0       0.4624        2        2           -1 -3       1 0",
  "Frame4:  P      4      0       0.4624        0        1           -4          1",
  NULL,
};
#endif


AX_S32 VencParameterGet(AX_S32 argc, AX_S8 **argv, SAMPLE_VENC_CMD_PARA_T *pCml) ;
AX_VOID SetDefaultParameter(SAMPLE_VENC_CMD_PARA_T *pstPara);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif /* End of __AX_COMM_ENC_H__ */
