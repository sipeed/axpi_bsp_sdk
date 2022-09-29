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
**  ecos_generic.c: A subdriver of CommonLCD NEWGAL engine for eCos.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#ifdef _MGGAL_COMMLCD

#ifdef __ECOS__

#include "minigui.h"
#include "newgal.h"
#include "sysvideo.h"
#include "pixels_c.h"
#include "commlcd.h"

#include <cyg/hal/drv_api.h>
#include <cyg/io/io.h>
#include <cyg/hal/lcd_support.h>

static int a_init (void)
{
    lcd_init (16);
    return 0;
}

static int a_getinfo (struct commlcd_info *li, int width, int height, int bpp)
{
    struct lcd_info ecos_lcd_info;

    lcd_getinfo (&ecos_lcd_info);

    li->height = ecos_lcd_info.height;
    li->width = ecos_lcd_info.width;
    li->fb = ecos_lcd_info.fb;
    li->bpp = ecos_lcd_info.bpp;
    li->pitch = ecos_lcd_info.pitch;
    li->async_update = 0;
    return 0;
}

struct commlcd_ops __mg_commlcd_ops = {
    a_init,
    a_getinfo,
    NULL,
    NULL,
    NULL
};

#endif /* __ECOS__ */

#endif /* _MGGAL_COMMLCD */
