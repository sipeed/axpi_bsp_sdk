#ifndef _AXDL20DL_API_8ACF31F8_0EEF_4988_B71D_DFFFD8F213F2_H_
#define _AXDL20DL_API_8ACF31F8_0EEF_4988_B71D_DFFFD8F213F2_H_

#include "ax_base_type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  AXDL_SUCC = 0x00,                    /* success */
  AXDL_ERR_INVALID_PARAM = 0x01,       /* parameter is invalid, such as nil pointer */
  AXDL_ERR_NOT_SUPPORT = 0x02,         /* feature is not support */
  AXDL_ERR_MEM_ALLOC_FAIL = 0x03,      /* memory allocate failure */
  AXDL_ERR_TIMEOUT = 0x04,             /* timeout */
  AXDL_ERR_OPEN_FILE_FAIL = 0x05,      /* open file failure */
  AXDL_ERR_READ_FILE_FAIL = 0x06,      /* read file failure */
  AXDL_ERR_EMPTY_FILE = 0x07,          /* file size is 0 */
  AXDL_ERR_AUTH_PAC_FAIL = 0x08,       /* authenticate pac file failure */
  AXDL_ERR_INVALID_PAC_FILE = 0x09,    /* pac file is invalid */
  AXDL_ERR_DOWNLOAD_FAIL = 0x0A,       /* download firmware failure */
  AXDL_ERR_DEVICE_NOT_OPEN = 0x0C,     /* device is not open */
  AXDL_ERR_PULL_RESET_PIN_FAIL = 0x0D, /* pull reset pin failure */
  AXDL_ERR_SETUP_CLK_FAIL = 0x0E,      /* setup clk failure */
  AXDL_ERR_POWER_ONOFF_FAIL = 0x0F,    /* power on or off failure */
  AXDL_ERR_CHECK_DEVICE_FAIL = 0x10,   /* check device plug or remove status failure */
  AXDL_ERR_SCAN_DEVICE_FAIL = 0x11,    /* trigger to scan device failure */
  AXDL_ERR_BUTT,
} AXDL_ERR_E;

//////////////////////////////////////////////////////////////////////////////////////
/// AX_DL20
///
/// @brief Download slave firmware (.pac)
///
/// @param pszPacPath: Specifies the slave firmware (.pac) path to download
/// @param pszDevPath: Specifies the slave device path. If set NULL, "/dev/sdio_host" is replaced.
/// @param bEnterLowPwrMode: AX_TRUE: Control slave to enter low power mode after download.
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_DL20(const AX_CHAR *pszPacPath, const AX_CHAR *pszDevPath, AX_BOOL bEnterLowPwrMode);

#ifdef __cplusplus
}
#endif

#endif /* _AXDL20DL_API_8ACF31F8_0EEF_4988_B71D_DFFFD8F213F2_H_ */