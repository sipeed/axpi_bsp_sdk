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
** glyph.h: The common glyph related routines.
*/

#ifndef _MG_NEWGDI_GLYPH_H
    #define _MG_NEWGDI_GLYPH_H

typedef struct _BBOX {
    int x, y;
    int w, h;
} BBOX;

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

int _font_get_glyph_log_width (LOGFONT* logfont, Glyph32 gv);

int _font_get_glyph_advance (LOGFONT* logfont, DEVFONT* devfont,
        Glyph32 glyph_value, BOOL direction, int ch_extra,
        int x, int y, int* adv_x, int* adv_y, BBOX* bbox);

int _font_get_glyph_metrics(LOGFONT* logfont,
        Glyph32 gv, int* adv_x, int* adv_y, BBOX* bbox);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif // _MG_NEWGDI_GLYPH_H

