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
** dummy.c: The dummy IAL engine.
**
** Created by Wei Yongming, 2001/09/13
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#ifdef _MGIAL_DUMMY

#include "misc.h"
#include "ial.h"
#include "dummy.h"

static int mouse_x, mouse_y, mouse_button;

typedef struct tagPOS
{
    short x;
    short y;
    short b;
} POS;

/************************  Low Level Input Operations **********************/
/*
 * Mouse operations -- Event
 */
static int mouse_update(void)
{
    return 1;
}

static void mouse_getxy (int* x, int* y)
{
    *x = mouse_x;
    *y = mouse_y;
}

static int mouse_getbutton(void)
{
    return mouse_button;
}

static int keyboard_update(void)
{
    return 0;
}

static const char * keyboard_get_state (void)
{
    return NULL;
}

static int wait_event (int which, int maxfd, fd_set *in, fd_set *out, fd_set *except,
                struct timeval *timeout)
{
#ifdef _MGRM_THREADS
    __mg_os_time_delay (timeout->tv_sec * 1000 + timeout->tv_usec/1000);
    timeout->tv_sec  = 0;
    timeout->tv_usec = 0;
#else
    fd_set rfds;
    int    e;

    if (!in) {
        in = &rfds;
        mg_fd_zero (in);
    }

    e = mg_select (maxfd + 1, in, out, except, timeout);

    if (e < 0) {
        return -1;
    }
#endif

    return 0;
}

BOOL ial_InitDummyInput (INPUT* input, const char* mdev, const char* mtype)
{
    input->update_mouse = mouse_update;
    input->get_mouse_xy = mouse_getxy;
    input->set_mouse_xy = NULL;
    input->get_mouse_button = mouse_getbutton;
    input->set_mouse_range = NULL;

    input->update_keyboard = keyboard_update;
    input->get_keyboard_state = keyboard_get_state;
    input->set_leds = NULL;

    input->wait_event = wait_event;
    mouse_x = 0;
    mouse_y = 0;
    mouse_button= 0;
    return TRUE;
}

void TermDummyInput (void)
{
}

#endif /* _MGIAL_DUMMY */

