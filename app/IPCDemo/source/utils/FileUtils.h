/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef _FILE_UTILS_H_
#define _FILE_UTILS_H_

#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<dirent.h>
#include<sys/types.h>
#include<sys/stat.h>

#include "global.h"

extern int errno;
#define MODE (S_IRWXU | S_IRWXG | S_IRWXO)


class CFileUtils
{
public:
    static AX_S32 CheckAndMkDir(AX_CHAR* pszDir);

private:
    CFileUtils(AX_VOID);
    ~CFileUtils(AX_VOID);

    static AX_S32 MkDir(AX_CHAR* pszDir);
};

#endif // _FILE_UTILS_H_