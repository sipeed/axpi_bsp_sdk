#ifndef __COMMON_ARG_PARSE_H__
#define __COMMON_ARG_PARSE_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "ax_global_type.h"


#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */


typedef struct _SAMPLE_OPTION
{
    AX_S8 *long_opt;
    AX_S8 short_opt;
    AX_S32 enable;
} SAMPLE_OPTION_T;

typedef struct _SAMPLE_PARAMETER
{
    AX_S32 cnt;
    AX_S8 *argument;
    AX_S8 short_opt;
    AX_S8 *longOpt;
    AX_S32 enable;
} SAMPLE_PARAMETER_T;

AX_S32 SampleGetOption(AX_S32 argc, AX_S8 **argv, SAMPLE_OPTION_T *option, SAMPLE_PARAMETER_T * parameter);
AX_S32 SampleParseDelim(AX_S8 *optArg, AX_S8 delim);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif