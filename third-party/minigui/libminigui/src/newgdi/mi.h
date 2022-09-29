///////////////////////////////////////////////////////////////////////////////
//
//                          IMPORTANT NOTICE
//
// The following open source license statement does not apply to any
// entity in the Exception List published by FMSoft.
//
// For more information, please visit:
//
// https://www.fmsoft.cn/exception-list
//
//////////////////////////////////////////////////////////////////////////////
/*
 *   This file is part of MiniGUI, a mature cross-platform windowing
 *   and Graphics User Interface (GUI) support system for embedded systems
 *   and smart IoT devices.
 *
 *   Copyright (C) 2002~2018, Beijing FMSoft Technologies Co., Ltd.
 *   Copyright (C) 1998~2002, WEI Yongming
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   Or,
 *
 *   As this program is a library, any link to this program must follow
 *   GNU General Public License version 3 (GPLv3). If you cannot accept
 *   GPLv3, you need to be licensed from FMSoft.
 *
 *   If you have got a commercial license of this program, please use it
 *   under the terms and conditions of the commercial license.
 *
 *   For more information about the commercial license, please refer to
 *   <http://www.minigui.com/blog/minigui-licensing-policy/>.
 */
/*
** mi.h: low level advanced two dimensions graphics interfaces.
*/

#ifndef MI_H
#define MI_H 1

#define PixmapBytePad(w, d) (w)
#define BitmapBytePad(w) (w)

#define SCRRIGHT(x, n) ((x)>>(n))

#include "mistruct.h"
#include "mifpoly.h"
#include "mifillarc.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

miDashPtr miDashLine (int npt, POINT* ppt, unsigned int nDash, unsigned char* pDash, unsigned int offset, int* pnseg);
void miStepDash (int dist, int *pDashIndex, const unsigned char *pDash, int numInDashList, int *pDashOffset);

void miWideDash (PDC pdc, register int npt, register POINT *pPts);
void miWideLine (PDC pdc, register int npt, register POINT *pPts);

void miPolyArc (PDC pdc, int narcs, miArc *parcs);
void miPolyFillArc (PDC pdc, int narcs, miArc *parcs);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif

