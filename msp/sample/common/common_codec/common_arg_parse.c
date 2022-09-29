
#include <assert.h>
#include <string.h>
#include "common_arg_parse.h"



#define SAMPLE_LOG(str, arg...)    \
    do {    \
        printf("%s: %s:%d "str"\n", "common_arg_parse.c", __func__, __LINE__, ##arg); \
    } while(0)

#define SAMPLE_ERR_LOG(str, arg...)   \
    do{  \
        printf("%s: %s:%d Error! "str"\n", "common_arg_parse.c", __func__, __LINE__, ##arg); \
    }while(0)


#define CLIP3(x, y, z)  ((z) < (x) ? (x) : ((z) > (y) ? (y) : (z)))



static AX_S32 SampleGetNext(AX_S32 argc, AX_S8 **argv, SAMPLE_PARAMETER_T *parameter, AX_S8 **p)
{
    /* End of options */
    if ((parameter->cnt >= argc) || (parameter->cnt < 0))
    {
        // SAMPLE_ERR_LOG(" parameter->cnt:%d ", parameter->cnt);
        return -1;
    }
    *p = argv[parameter->cnt];
    parameter->cnt++;

    return 0;
}

static AX_S32 SampleParse(AX_S32 argc, AX_S8 **argv, SAMPLE_OPTION_T *option,
            SAMPLE_PARAMETER_T *parameter, AX_S8 **p, AX_U32 lenght)
{
    AX_S8 *arg;
    AX_U32 arg_len = 0;
    AX_S32 ret = 0;

    parameter->short_opt = option->short_opt;
    parameter->longOpt = option->long_opt;
    arg = *p + lenght;

    /* Argument and option are together */
    arg_len = strlen(arg);
    // SAMPLE_LOG("arg_len:%d lenght:%d ", arg_len, lenght);
    if (arg_len != 0)
    {
        /* There should be no argument */
        if (option->enable == 0)
        {
            SAMPLE_ERR_LOG(" ");
            return -1;
        }

        /* Remove = */
        if (strncmp("=", arg, 1) == 0)
        {
            arg++;
        }
        parameter->enable = 1;
        parameter->argument = arg;
        return 0;
    }

    /* Argument and option are separately */
    ret = SampleGetNext(argc, argv, parameter, p);
    // SAMPLE_LOG("ret:%d ", ret);
    if (ret)
    {
        /* There is no more parameters */
        if (option->enable == 1)
        {
            SAMPLE_ERR_LOG(" ");
            return -1;
        }
        return 0;
    }

    /* Parameter is missing if next start with "-" but next time this
    * option is OK so we must fix parameter->cnt */
    ret = strncmp("-", *p,  1);
    // SAMPLE_LOG("ret:%d ", ret);
    if (ret == 0)
    {
        parameter->cnt--;
        if (option->enable == 1)
        {
            SAMPLE_ERR_LOG(" ");
            return -1;
        }
        return 0;
    }

    /* There should be no argument */
    if (option->enable == 0)
    {
        SAMPLE_ERR_LOG(" *p:%s", *p);
        return -1;
    }

    parameter->enable = 1;
    parameter->argument = *p;

    return 0;
}

static AX_S32 SampleShortOption(AX_S32 argc, AX_S8 **argv, SAMPLE_OPTION_T *option,
                SAMPLE_PARAMETER_T *parameter, AX_S8 **p)
{
    AX_S32 i = 0;
    AX_S8 short_opt;

    if (strncmp("-", *p, 1) != 0)
    {
        return 1;
    }

    //strncpy(&short_opt, *p + 1, 1);
    short_opt = *(*p + 1);
    parameter->short_opt = short_opt;
    while (option[i].long_opt != NULL)
    {
        if (option[i].short_opt  == short_opt)
        {
            goto match;
        }
        i++;
    }
    return 1;

match:
    // SAMPLE_LOG("option[i:%d].short_opt:%c", i, option[i].short_opt);
    if (SampleParse(argc, argv, &option[i], parameter, p, 2) != 0)
    {
        SAMPLE_ERR_LOG(" ");
        return -2;
    }

    return 0;
}

static AX_S32 SampleLongOption(AX_S32 argc, AX_S8 **argv, SAMPLE_OPTION_T *option,
                SAMPLE_PARAMETER_T *parameter, AX_S8 **p)
{
    AX_S32 i = 0;
    AX_U32 lenght;
    // AX_U32 pLength;

    if (strncmp("--", *p, 2) != 0)
    {
        return 1;
    }

    // pLength = strlen(*p+2);
    while (option[i].long_opt != NULL)
    {
        lenght = strlen(option[i].long_opt);
        if (strncmp(option[i].long_opt, *p + 2, lenght) == 0)
        {
            goto match;
        }
        i++;
    }
    return 1;

match:
    lenght += 2;    /* Because option start -- */
    if (SampleParse(argc, argv, &option[i], parameter, p, lenght) != 0)
    {
        return -2;
    }

    return 0;
}

AX_S32 SampleGetOption(AX_S32 argc, AX_S8 **argv, SAMPLE_OPTION_T *option, SAMPLE_PARAMETER_T * parameter)
{
    AX_S8 *p = NULL;
    AX_S32 ret;

    parameter->argument = "?";
    parameter->short_opt = '?';
    parameter->enable = 0;

    if (SampleGetNext(argc, argv, parameter, &p)) {
        // SAMPLE_ERR_LOG(" ");
        return -1;  /* End of options */
    }

    /* Long option */
    ret = SampleLongOption(argc, argv, option, parameter, &p);
    if (ret != 1) {
        // SAMPLE_ERR_LOG(" ret:%d ", ret);
        return ret;
    }

    /* Short option */
    ret = SampleShortOption(argc, argv, option, parameter, &p);
    if (ret != 1) {
        // SAMPLE_ERR_LOG(" ret:%d ", ret);
        return ret;
    }

    /* This is unknow option but option anyway so argument must return */
    parameter->argument = p;

    return 1;
}


AX_S32 SampleParseDelim(AX_S8 *optArg, AX_S8 delim)
{
    AX_S32 i;

    for (i = 0; i < (AX_S32)strlen(optArg); i++)
        if (optArg[i] == delim)
        {
            optArg[i] = 0;
            return i;
        }

    return -1;
}
