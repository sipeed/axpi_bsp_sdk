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
** resource.c: This file include some functions for system resource loading.
**           some functions are from misc.c.
**
** Create date: 2003/09/06
**
** Current maintainer: Wei Yongming.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _MGRM_THREADS
#include <pthread.h>
#endif

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "window.h"
#include "cursor.h"
#include "icon.h"
#include "sysres.h"
#include "misc.h"

#ifdef _MGINCORE_RES

#include "../font/rawbitmap.h"
#include "../font/varbitmap.h"
#include "../font/qpf.h"
#include "../font/upf.h"

#include "bmp/_bmp_inner_res.c"
#include "icon/_icon_inner_res.c"
#include "font/_font_inner_res.c"

#endif

BOOL __sysres_init_inner_resource()
{
#ifdef _MGINCORE_RES
    AddInnerRes((INNER_RES*)__mgir_bmp_inner_res, TABLESIZE(__mgir_bmp_inner_res), FALSE);
    AddInnerRes((INNER_RES*)__mgir_icon_inner_res, TABLESIZE(__mgir_icon_inner_res),FALSE);
    AddInnerRes((INNER_RES*)__mgir_font_inner_res, TABLESIZE(__mgir_font_inner_res),FALSE);
#endif
    return TRUE;
}

#ifdef _MGHAVE_CURSOR
#ifdef _MGINCORE_RES
extern const int __mg_cursors_offset [];
extern const unsigned char __mg_cursors_data[];
#endif

HCURSOR __mg_load_cursor_from_res (int i)
{
    HCURSOR hCursor = 0;
    int nr_cursors = 0;
    char key [32];
    char filename [MAX_NAME + 1];
#ifdef _MGSCHEMA_COMPOSITING
    const char* ext;
    BOOL is_png = FALSE;
    int hotspot[2] = { 0, 0 };
#endif

    if (GetMgEtcIntValue ("cursorinfo", "cursornumber", &nr_cursors) < 0 )
        return 0;

    if (i >= nr_cursors || i < 0)
        return 0;

    sprintf (key, "cursor%d", i);
    if (GetMgEtcValue (CURSORSECTION, key, filename, MAX_NAME) < 0)
        return 0;

#ifdef _MGSCHEMA_COMPOSITING
    ext = __mg_get_extension (filename);
    if (ext && strcasecmp (ext, "png") == 0) {
        sprintf (key, "hotspot%d", i);
        if (GetMgEtcValue (CURSORSECTION, key, filename, MAX_NAME) < 0)
            return 0;

        __mg_extract_integers (filename, ',', hotspot, 2);
        is_png = TRUE;
    }
#endif

#ifdef _MGINCORE_RES
#   ifdef _MGSCHEMA_COMPOSITING
    if (is_png) {
        size_t size;

        /* For LoadCursorFromPNGMem, the array of offsets
         * must have NR_CURSORS + 1 units. */
        size = __mg_cursors_offset [i + 1] - __mg_cursors_offset [i];

        hCursor = LoadCursorFromPNGMem (__mg_cursors_data +
                __mg_cursors_offset [i], size, hotspot[0], hotspot[1]);
    }
    else
#   endif
    hCursor = LoadCursorFromMem (__mg_cursors_data + __mg_cursors_offset [i]);
#endif

    if (!hCursor) {
        char path [MAX_PATH + 1];

        strncpy (path, __sysres_get_system_res_path(), MAX_PATH);
        path [MAX_PATH] = 0;
        strcat (path, "cursor/");
        path [MAX_PATH] = 0;
        strcat (path, filename);
        path [MAX_PATH] = 0;

#ifdef _MGSCHEMA_COMPOSITING
        if (is_png)
            hCursor = LoadCursorFromPNGFile (path, hotspot[0], hotspot[1]);
        else
#endif
        hCursor = LoadCursorFromFile (path);
    }

    return hCursor;
}
#endif

