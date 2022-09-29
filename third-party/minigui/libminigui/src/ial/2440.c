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
** 2410.c: Low Level Input Engine for SMDK2410 Dev Board.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#ifdef _MGIAL_2440

#include <unistd.h>
#include <fcntl.h>
//#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/kd.h>

#include "ial.h"
#include "2440.h"

/* for data reading from /dev/ts */
typedef struct {
    unsigned short pressure;
    unsigned short x;
    unsigned short y;
    unsigned short pad;
} TS_EVENT;

static unsigned char state [NR_KEYS];
static int ts = -1;
static int mousex = 0;
static int mousey = 0;
static TS_EVENT ts_event;


/************************  Low Level Input Operations **********************/
/*
 * Mouse operations -- Event
 */
static int mouse_update(void)
{
    return 1;
}

static void mouse_getxy(int *x, int* y)
{
#ifdef _DEBUG
    printf ("mousex = %d, mousey = %d\n", mousex, mousey);
#endif

    if (mousex < 0) mousex = 0;
    if (mousey < 0) mousey = 0;
    if (mousex > 799) mousex = 799;
    if (mousey > 479) mousey = 479;

    *x = mousex;
    *y = mousey;
}

static int mouse_getbutton(void)
{
    return ts_event.pressure;
}

static int wait_event (int which, int maxfd, fd_set *in, fd_set *out, fd_set *except,
                struct timeval *timeout)
{
    fd_set rfds;
    int    retvalue = 0;
    int    e;

    if (!in) {
        in = &rfds;
        FD_ZERO (in);
    }

    if ((which & IAL_MOUSEEVENT) && ts >= 0) {
        FD_SET (ts, in);
        if (ts > maxfd) maxfd = ts;
    }
    e = select (maxfd + 1, in, out, except, timeout) ;

    if (e > 0) {
        if (ts >= 0 && FD_ISSET (ts, in)) {
            int rs;
            FD_CLR (ts, in);
            ts_event.x=0;
            ts_event.y=0;

            rs = read (ts, &ts_event, sizeof (TS_EVENT));
            if (rs != sizeof(TS_EVENT))
                return -1;

            if (ts_event.pressure > 0) {
                mousex = ts_event.x;
                mousey = ts_event.y;
            }
#ifdef _DEBUG
            if (ts_event.pressure > 0) {
                printf ("mouse down: ts_event.x = %d, ts_event.y = %d\n", ts_event.x, ts_event.y);
            }
#endif
            ts_event.pressure = ( ts_event.pressure > 0 ? IAL_MOUSE_LEFTBUTTON:0);
            retvalue |= IAL_MOUSEEVENT;
        }

    }
    else if (e < 0) {
        return -1;
    }

    return retvalue;
}

BOOL Init2440Input (INPUT* input, const char* mdev, const char* mtype)
{
    /* mdev should be /dev/ts */
    ts = open (mdev, O_RDONLY);
    if (ts < 0) {
        fprintf (stderr, "2410: Can not open touch screen: %s!\n", mdev);
        return FALSE;
    }

    input->update_mouse = mouse_update;
    input->get_mouse_xy = mouse_getxy;
    input->set_mouse_xy = NULL;
    input->get_mouse_button = mouse_getbutton;
    input->set_mouse_range = NULL;

    input->wait_event = wait_event;
    mousex = 0;
    mousey = 0;
    ts_event.x = ts_event.y = ts_event.pressure = 0;

    return TRUE;
}

void Term2440Input(void)
{
    if (ts >= 0)
        close(ts);
}

#endif /* _SMDK2410_IAL */

