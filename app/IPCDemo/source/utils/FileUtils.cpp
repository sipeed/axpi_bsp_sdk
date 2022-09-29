/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include "FileUtils.h"
#define FILE_UTILS "FileUtils"


CFileUtils::CFileUtils(AX_VOID)
{
}

CFileUtils::~CFileUtils(AX_VOID)
{
}

AX_S32 CFileUtils::MkDir(AX_CHAR* pszDir)
{
    DIR *mydir = NULL;
    if((mydir = opendir(pszDir)) == NULL)
    {
        AX_S32 ret = mkdir(pszDir, MODE);
        if (ret != 0)
        {
            LOG_M_E(FILE_UTILS, "mkdir failed, target path:%s, err: %d", pszDir, errno);
            return -1;
        }
    }
    return 0;
}

AX_S32 CFileUtils::CheckAndMkDir(AX_CHAR* pszDir)
{
    AX_S32  nRet  = 0;
    AX_CHAR *pDir = pszDir;

    while (1)
    {
        AX_CHAR *pIndex = index(pDir, '/');
        if (pIndex != NULL && pIndex != pszDir)
        {
            AX_CHAR buffer[64] = {0};
            AX_S32  size = pIndex - pszDir;

            memcpy(buffer, pszDir, size);
            AX_S32 ret = MkDir(buffer);
            if (ret < 0)
            {
                nRet = -1;
                break;
            }
        }
        else if (pIndex == NULL && pDir == pszDir)
        {
            // target path is file
            nRet = -1;
            break;
        }
        else if (pIndex == NULL && pDir != pszDir)
        {
            int ret = MkDir(pszDir);
            if (ret < 0)
            {
                nRet = -1;
            }
            break;
        }
        else if (pIndex == (pszDir + (strlen(pszDir) - 1)))
        {
            // target path ends with '/'
            LOG_M_E(FILE_UTILS, "target path ends with \'/\'");
            break;
        }
        pDir = pIndex + 1;
    }

    return nRet;
}
