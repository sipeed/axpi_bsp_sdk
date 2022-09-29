/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef __AX_OSD_API_H__
#define __AX_OSD_API_H__

#include "global.h"
#include "Singleton.h"
#include <mutex>

#define AX_MAX_VO_NUM        (32)
#define OSD_ALIGN_X_OFFSET   (64)
#define OSD_ALIGN_Y_OFFSET   (2)
#define OSD_ALIGN_WIDTH      (64)
#define OSD_ALIGN_HEIGHT     (2)
#define IS_VALID_CHANNEL(chn) ((chn) >= 0 && (chn) < MAX_WEB_CHANNEL_NUM)
#define IS_VALID_OSD_NUM(num) ((num) >= 0 && (num) <= AX_MAX_VO_NUM)

#ifndef RANGE_CHECK
#define RANGE_CHECK(v, min, max) (((v) < (min)) ? 0 : ((v) > (max)) ? 0 : 1)
#endif

#define CHECK_POINTER(p)                   \
    if (!p) {                              \
        return OSD_ERR_NULL_POINTER;       \
    }

#define CHECK_CHANNEL(chn)                          \
    if (!IS_VALID_CHANNEL(chn)) {                   \
        return OSD_ERR_INVALID_VIDEO_CHANNEL;       \
    }

#define CHECK_OSD_NUMBER(num)                       \
    if (!IS_VALID_OSD_NUM(num)) {                   \
        return OSD_ERR_INVALID_PARAM;               \
    }

#define CHECK_OSD_HANDLE(chn, handle)                       \
    if (!IS_VALID_OSD_NUM(handle)) {                        \
        return OSD_ERR_INVALID_HANDLE;                      \
    }                                                       \
    if (!RANGE_CHECK(handle, 0, m_nOsdCfgNum[chn] - 1)) {    \
        return OSD_ERR_INVALID_HANDLE;                      \
    }

/* OSD ERROR */
typedef enum
{
    OSD_SUCCESS = 0,
    OSD_ERR_NULL_POINTER,
    OSD_ERR_INVALID_HANDLE,
    OSD_ERR_INVALID_VIDEO_CHANNEL,
    OSD_ERR_INVALID_PARAM,
    OSD_ERR_MAX
} OSD_ERR_CODE_E;

/* OSD Align Type */
typedef enum
{
	OSD_ALIGN_TYPE_LEFT_TOP,				/* 左上角对齐 */
	OSD_ALIGN_TYPE_RIGHT_TOP,				/* 右上角对齐 */
	OSD_ALIGN_TYPE_LEFT_BOTTOM,				/* 左下角对齐 */
	OSD_ALIGN_TYPE_RIGHT_BOTTOM,			/* 右下角对齐 */
	OSD_ALIGN_TYPE_MAX
} OSD_ALIGN_TYPE_E;

/* OSD Type */
typedef enum
{
	OSD_TYPE_PICTURE,						/* 图片 */
	OSD_TYPE_STRING,						/* 字符串 */
	OSD_TYPE_TIME,							/* 时间 */
    OSD_TYPE_PRIVACY,                       /* 矩形遮挡 */
	OSD_TYPE_MAX
} OSD_TYPE_E;

/* 隐私遮挡类型 */
typedef enum
{
	OSD_PRIVACY_TYPE_RECT,						/* 矩形遮挡 */
	OSD_PRIVACY_TYPE_MOSAIC,						/* 马赛克遮挡 */
	OSD_PRIVACY_TYPE_MAX
} OSD_PRIVACY_TYPE_E;

/* 时间类型OSD时间显示格式 */
typedef enum
{
	OSD_DATE_FORMAT_YYMMDD1,				/* XXXX-XX-XX (年月日) */
	OSD_DATE_FORMAT_MMDDYY1,				/* XX-XX-XXXX (月日年) */
	OSD_DATE_FORMAT_DDMMYY1,				/* XX-XX-XXXX (日月年) */
	OSD_DATE_FORMAT_YYMMDD2,				/* XXXX年XX月XX日 */
	OSD_DATE_FORMAT_MMDDYY2,				/* XX月XX日XXXX年 */
	OSD_DATE_FORMAT_DDMMYY2,				/* XX日XX月XXXX年 */
	OSD_DATE_FORMAT_YYMMDD3,				/* XXXX/XX/XX (年月日) */
	OSD_DATE_FORMAT_MMDDYY3,				/* XXXX/XX/XX (年月日) */
	OSD_DATE_FORMAT_DDMMYY3,				/* XXXX/XX/XX (年月日) */
	OSD_DATE_FORMAT_YYMMDDWW1,				/* XXXX-XX-XX XXX (年月日 星期几) */
    OSD_DATE_FORMAT_HHmmSS,				    /* XX:XX:XX (时分秒) */
	OSD_DATE_FORMAT_YYMMDDHHmmSS,			/* XXXX-XX-XX XX:XX:XX (年月日 时分秒) */
    OSD_DATE_FORMAT_YYMMDDHHmmSSWW,			/* XXXX-XX-XX XX:XX:XX XXX (年月日 时分秒 星期几) */
	OSD_OSD_DATE_FORMAT_MAX
} OSD_DATE_FORMAT_E;

/* 图片类型OSD属性 */
typedef struct _OSD_PIC_ATTR_T
{
	AX_S32  nWidth;							/* 图片分辨率 */
	AX_S32  nHeight;
	AX_CHAR szFileName[128];				/* 图片所在位置 /xxx/xxx/xxx.bmp */
	AX_U8*  pData;							/* 图片缓存， fileName和pPicData二选一设置图片OSD, 若fileName是空的就必须填充pPicData */
	AX_S32  nDataLen;						/* 图片缓存长度 */

    _OSD_PIC_ATTR_T() {
        memset(this, 0, sizeof(_OSD_PIC_ATTR_T));
    }
} OSD_PIC_ATTR_T;

/* 字符串类型OSD属性 */
typedef struct _OSD_STR_ATTR_T
{
	AX_S32  nFontSize;						/* 字体大小，-1：默认大小 */
	AX_CHAR szStr[128];						/* 字符串内容 */

    _OSD_STR_ATTR_T() {
        memset(this, 0, sizeof(_OSD_STR_ATTR_T));
    }
} OSD_STR_ATTR_T;

/* 时间串类型OSD属性 */
typedef struct _OSD_TIME_ATTR_T
{
	AX_U32              nFontSize;			/* 字体大小，-1：默认大小 */
	OSD_DATE_FORMAT_E   eFormat;			/* 格式 */

    _OSD_TIME_ATTR_T() {
        memset(this, 0, sizeof(_OSD_TIME_ATTR_T));
    }
} OSD_TIME_ATTR_T;

/* 隐私遮挡类型OSD属性 */
typedef struct _OSD_PRIVACY_ATTR_T
{
    OSD_PRIVACY_TYPE_E eType;       /* 遮挡类型(0:矩形; 1:马赛克) */
	AX_U32  nX;			            /* 左上角X坐标 */
    AX_U32  nY;			            /* 左上角Y坐标 */
    AX_U32  nW;			            /* 矩形宽 */
    AX_U32  nH;			            /* 矩形高 */
    AX_U32  nArgb;                  /* 填充色 */

    _OSD_PRIVACY_ATTR_T() {
        memset(this, 0, sizeof(_OSD_PRIVACY_ATTR_T));
    }
} OSD_PRIVACY_ATTR_T;

/* OSD Config */
typedef struct _OSD_CFG_T
{
	AX_BOOL             bEnable;			/* 是否显示 */
	AX_U32              nMatchWidth;		/* 匹配的分辨率，若匹配当前视频分辨率，显示该OSD */
	AX_U32              nMatchHeight;
	OSD_ALIGN_TYPE_E    eAlign;				/* 对齐方式 */
	AX_S32              nBoundaryX;			/* X轴上，OSD显示位置与边界距离，OSD_ALIGN_LEFT_TOP/OSD_ALIGN_LEFT_BOTTOM对齐方式，则是与左边界距离，否则右边界 */
	AX_S32              nBoundaryY;			/* Y轴上，OSD显示位置与边界距离，OSD_ALIGN_LEFT_TOP/OSD_ALIGN_RIGHT_TOP对齐方式，则是与上边界距离，否则下边界 */
	OSD_TYPE_E          eType;				/* 类型 */
	AX_U32              nARGB;				/* 字符串或时间颜色，A,R,G,B各一个字节 */

	union {
		OSD_PIC_ATTR_T      tPicAttr;	    /* 图片OSD属性 */
		OSD_STR_ATTR_T      tStrAttr;		/* 字符串OSD属性 */
		OSD_TIME_ATTR_T     tTimeAttr;		/* 时间OSD属性 */
        OSD_PRIVACY_ATTR_T  tPrivacyAttr;   /* 隐私遮挡OSD属性 */
	};

	AX_BOOL             bAllMatch;			/* 是否匹配所有分辨率，true-忽略matchWidth/matchHeight，切所有分辨率都显示该OSD */

    _OSD_CFG_T() {
        memset(this, 0, sizeof(_OSD_CFG_T));
    }
} OSD_CFG_T;

typedef struct _OSD_ATTR_INFO {
    AX_S32      nHandle;
    IVPS_GRP    nIvpsGrp;
    AX_U32      nFilter;
    OSD_TYPE_E  eOsdType;
    AX_S32      nZIndex;
    AX_BOOL     bThreadUpdate;
} OSD_ATTR_INFO;

class CAXOsdCfg: public CSingleton<CAXOsdCfg>
{
    friend class CSingleton<CAXOsdCfg>;

public:
    CAXOsdCfg(AX_VOID);
    ~CAXOsdCfg(AX_VOID);

    AX_S32 SetOsd(AX_U32 nChn, const OSD_CFG_T *pOsdArr, const AX_S32 nOsdSize);
    AX_S32 GetOsd(AX_U32 nChn, const OSD_CFG_T **pOsdArr, AX_S32 *pOsdSize);

    AX_S32 AddOsd(AX_U32 nChn, const OSD_CFG_T *pOsd, AX_S32 *pOsdHandle);
    AX_S32 RemoveOsd(AX_U32 nChn, AX_S32 nOsdHandle);
    AX_S32 UpdateOsd(AX_U32 nChn, AX_S32 nOsdHandle, const OSD_CFG_T *pOsd);

    AX_S32 TypeOn(AX_U32 nChn, AX_U32 nFlag);
    AX_S32 TypeOff(AX_U32 nChn, AX_U32 nFlag);

private:
    AX_BOOL Init();

private:
    OSD_CFG_T   m_tOsdCfgWrite[MAX_WEB_CHANNEL_NUM][AX_MAX_VO_NUM];
    OSD_CFG_T   m_tOsdCfgRead[MAX_WEB_CHANNEL_NUM][AX_MAX_VO_NUM];
    AX_S32      m_nOsdCfgNum[MAX_WEB_CHANNEL_NUM];

    std::mutex  m_mtxOsd;
};

#endif
