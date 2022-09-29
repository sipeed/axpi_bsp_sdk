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

/************************************************************

Copyright 1989, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

Author:  Bob Scheifler, MIT X Consortium

********************************************************/
/* $XFree86: xc/programs/Xserver/mi/mifillarc.c,v 3.4 1999/04/11 13:11:20 dawes Exp $ */
/* $TOG: mifillarc.c /main/20 1998/02/09 14:46:52 kaleb $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "common.h"

#ifdef _MGHAVE_ADV_2DAPI

#include "minigui.h"
#include "gdi.h"
#include "window.h"
#include "cliprect.h"
#include "gal.h"
#include "internals.h"
#include "ctrlclass.h"
#include "dc.h"
#include "pixel_ops.h"

#include "mi.h"
#include "midc.h"
#include "mifillarc.h"

#define QUADRANT (90 * 64)
#define HALFCIRCLE (180 * 64)
#define QUADRANT3 (270 * 64)

#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

#define Dsin(d)        sin((double)d*(M_PI/11520.0))
#define Dcos(d)        cos((double)d*(M_PI/11520.0))

void miFillArcSetup (register miArc* arc, register miFillArcRec* info)
{
    info->y = arc->height >> 1;
    info->dy = arc->height & 1;
    info->yorg = arc->y + info->y;
    info->dx = arc->width & 1;
    info->xorg = arc->x + (arc->width >> 1) + info->dx;
    info->dx = 1 - info->dx;

    if (arc->width == arc->height) {
        /* (2x - 2xorg)^2 = d^2 - (2y - 2yorg)^2 */
        /* even: xorg = yorg = 0   odd:  xorg = .5, yorg = -.5 */
        info->ym = 8;
        info->xm = 8;
        info->yk = info->y << 3;

        if (!info->dx) {
            info->xk = 0;
            info->e = -1;
        }
        else {
            info->y++;
            info->yk += 4;
            info->xk = -4;
            info->e = - (info->y << 3);
        }
    }
    else {
        /* h^2 * (2x - 2xorg)^2 = w^2 * h^2 - w^2 * (2y - 2yorg)^2 */
        /* even: xorg = yorg = 0   odd:  xorg = .5, yorg = -.5 */
        info->ym = (arc->width * arc->width) << 3;
        info->xm = (arc->height * arc->height) << 3;
        info->yk = info->y * info->ym;

        if (!info->dy)
            info->yk -= info->ym >> 1;

        if (!info->dx) {
            info->xk = 0;
            info->e = - (info->xm >> 3);
        }
        else {
            info->y++;
            info->yk += info->ym;
            info->xk = -(info->xm >> 1);
            info->e = info->xk - info->yk;
        }
    }
}

void miFillArcDSetup (register miArc* arc, register miFillArcDRec* info)
{
    /* h^2 * (2x - 2xorg)^2 = w^2 * h^2 - w^2 * (2y - 2yorg)^2 */
    /* even: xorg = yorg = 0   odd:  xorg = .5, yorg = -.5 */
    info->y = arc->height >> 1;
    info->dy = arc->height & 1;
    info->yorg = arc->y + info->y;
    info->dx = arc->width & 1;
    info->xorg = arc->x + (arc->width >> 1) + info->dx;
    info->dx = 1 - info->dx;
    info->ym = ((double)arc->width) * (arc->width * 8);
    info->xm = ((double)arc->height) * (arc->height * 8);
    info->yk = info->y * info->ym;

    if (!info->dy)
        info->yk -= info->ym / 2.0;

    if (!info->dx) {
        info->xk = 0;
        info->e = - (info->xm / 8.0);
    }
    else {
        info->y++;
        info->yk += info->ym;
        info->xk = -info->xm / 2.0;
        info->e = info->xk - info->yk;
    }
}

static void miGetArcEdge (register miArc* arc, register miSliceEdgePtr edge, int k, BOOL top, BOOL left)
{
    register int xady, y;

    y = arc->height >> 1;
    if (!(arc->width & 1))
        y++;

    if (!top) {
        y = -y;
        if (arc->height & 1)
            y--;
    }

    xady = k + y * edge->dx;

    if (xady <= 0)
        edge->x = - ((-xady) / edge->dy + 1);
    else
        edge->x = (xady - 1) / edge->dy;

    edge->e = xady - edge->x * edge->dy;

    if ((top && (edge->dx < 0)) || (!top && (edge->dx > 0)))
        edge->e = edge->dy - edge->e + 1;

    if (left)
        edge->x++;

    edge->x += arc->x + (arc->width >> 1);

    if (edge->dx > 0) {
        edge->deltax = 1;
        edge->stepx = edge->dx / edge->dy;
        edge->dx = edge->dx % edge->dy;
    }
    else {
        edge->deltax = -1;
        edge->stepx = - ((-edge->dx) / edge->dy);
        edge->dx = (-edge->dx) % edge->dy;
    }

    if (!top) {
        edge->deltax = -edge->deltax;
        edge->stepx = -edge->stepx;
    }
}

void miEllipseAngleToSlope (int angle, int width, int height, int* dxp, int* dyp, double* d_dxp, double* d_dyp)
{
    int dx, dy;
    double d_dx, d_dy, scale;
    BOOL negative_dx, negative_dy;

    switch (angle) {
    case 0:
        *dxp = -1;
        *dyp = 0;
        if (d_dxp) {
            *d_dxp = width / 2.0;
            *d_dyp = 0;
        }
        break;

    case QUADRANT:
        *dxp = 0;
        *dyp = 1;
        if (d_dxp) {
            *d_dxp = 0;
            *d_dyp = - height / 2.0;
        }
        break;

    case HALFCIRCLE:
        *dxp = 1;
        *dyp = 0;
        if (d_dxp) {
            *d_dxp = - width / 2.0;
            *d_dyp = 0;
        }
        break;

    case QUADRANT3:
        *dxp = 0;
        *dyp = -1;
        if (d_dxp) {
            *d_dxp = 0;
            *d_dyp = height / 2.0;
        }
        break;

    default:
        d_dx = Dcos(angle) * width;
        d_dy = Dsin(angle) * height;
        if (d_dxp) {
            *d_dxp = d_dx / 2.0;
            *d_dyp = - d_dy / 2.0;
        }
        negative_dx = FALSE;
        if (d_dx < 0.0) {
            d_dx = -d_dx;
            negative_dx = TRUE;
        }
        negative_dy = FALSE;
        if (d_dy < 0.0) {
            d_dy = -d_dy;
            negative_dy = TRUE;
        }
        scale = d_dx;
        if (d_dy > d_dx)
            scale = d_dy;
        dx = floor ((d_dx * 32768) / scale + 0.5);
        if (negative_dx)
            dx = -dx;
        *dxp = dx;
        dy = floor ((d_dy * 32768) / scale + 0.5);
        if (negative_dy)
            dy = -dy;
        *dyp = dy;
        break;
    }
}

static void miGetPieEdge (register miArc* arc, register int angle, register miSliceEdgePtr edge, BOOL top, BOOL left)
{
    register int k;
    int dx, dy;

    miEllipseAngleToSlope (angle, arc->width, arc->height, &dx, &dy, 0, 0);

    if (dy == 0) {
        edge->x = left ? -65536 : 65536;
        edge->stepx = 0;
        edge->e = 0;
        edge->dx = -1;
        return;
    }

    if (dx == 0) {

        edge->x = arc->x + (arc->width >> 1);

        if (left && (arc->width & 1))
            edge->x++;
        else if (!left && !(arc->width & 1))
            edge->x--;

        edge->stepx = 0;
        edge->e = 0;
        edge->dx = -1;
        return;
    }

    if (dy < 0) {
        dx = -dx;
        dy = -dy;
    }

    k = (arc->height & 1) ? dx : 0;
    if (arc->width & 1)
        k += dy;

    edge->dx = dx << 1;
    edge->dy = dy << 1;
    miGetArcEdge (arc, edge, k, top, left);
}

void miFillArcSliceSetup (register miArc* arc, register miArcSliceRec* slice, PDC pdc)
{
    register int angle1, angle2;

    angle1 = arc->angle1;
    if (arc->angle2 < 0) {
        angle2 = angle1;
        angle1 += arc->angle2;
    }
    else
        angle2 = angle1 + arc->angle2;

    while (angle1 < 0)
        angle1 += FULLCIRCLE;

    while (angle1 >= FULLCIRCLE)
        angle1 -= FULLCIRCLE;

    while (angle2 < 0)
        angle2 += FULLCIRCLE;

    while (angle2 >= FULLCIRCLE)
        angle2 -= FULLCIRCLE;

    slice->min_top_y = 0;
    slice->max_top_y = arc->height >> 1;
    slice->min_bot_y = 1 - (arc->height & 1);
    slice->max_bot_y = slice->max_top_y - 1;
    slice->flip_top = FALSE;
    slice->flip_bot = FALSE;

#if 1
    {
        slice->edge1_top = (angle1 < HALFCIRCLE);
        slice->edge2_top = (angle2 <= HALFCIRCLE);
        if ((angle2 == 0) || (angle1 == HALFCIRCLE))
        {
            if (angle2 ? slice->edge2_top : slice->edge1_top)
                slice->min_top_y = slice->min_bot_y;
            else
                slice->min_top_y = arc->height;
            slice->min_bot_y = 0;
        }
        else if ((angle1 == 0) || (angle2 == HALFCIRCLE))
        {
            slice->min_top_y = slice->min_bot_y;
            if (angle1 ? slice->edge1_top : slice->edge2_top)
                slice->min_bot_y = arc->height;
            else
                slice->min_bot_y = 0;
        }
        else if (slice->edge1_top == slice->edge2_top)
        {
            if (angle2 < angle1)
            {
                slice->flip_top = slice->edge1_top;
                slice->flip_bot = !slice->edge1_top;
            }
            else if (slice->edge1_top)
            {
                slice->min_top_y = 1;
                slice->min_bot_y = arc->height;
            }
            else
            {
                slice->min_bot_y = 0;
                slice->min_top_y = arc->height;
            }
        }
        miGetPieEdge(arc, angle1, &slice->edge1,
                     slice->edge1_top, !slice->edge1_top);
        miGetPieEdge(arc, angle2, &slice->edge2,
                     slice->edge2_top, slice->edge2_top);
    }
#else
    {
        double w2, h2, x1, y1, x2, y2, dx, dy, scale;
        int signdx, signdy, y, k;
        BOOL isInt1 = TRUE, isInt2 = TRUE;

        w2 = (double)arc->width / 2.0;
        h2 = (double)arc->height / 2.0;

        if ((angle1 == 0) || (angle1 == HALFCIRCLE)) {
            x1 = angle1 ? -w2 : w2;
            y1 = 0.0;
        }
        else if ((angle1 == QUADRANT) || (angle1 == QUADRANT3)) {
            x1 = 0.0;
            y1 = (angle1 == QUADRANT) ? h2 : -h2;
        }
        else {
            isInt1 = FALSE;
            x1 = Dcos(angle1) * w2;
            y1 = Dsin(angle1) * h2;
        }

        if ((angle2 == 0) || (angle2 == HALFCIRCLE)) {
            x2 = angle2 ? -w2 : w2;
            y2 = 0.0;
        }
        else if ((angle2 == QUADRANT) || (angle2 == QUADRANT3)) {
            x2 = 0.0;
            y2 = (angle2 == QUADRANT) ? h2 : -h2;
        }
        else {
            isInt2 = FALSE;
            x2 = Dcos(angle2) * w2;
            y2 = Dsin(angle2) * h2;
        }

        dx = x2 - x1;
        dy = y2 - y1;

        if (arc->height & 1) {
            y1 -= 0.5;
            y2 -= 0.5;
        }

        if (arc->width & 1) {
            x1 += 0.5;
            x2 += 0.5;
        }

        if (dy < 0.0) {
            dy = -dy;
            signdy = -1;
        }
        else
            signdy = 1;

        if (dx < 0.0) {
            dx = -dx;
            signdx = -1;
        }
        else
            signdx = 1;

        if (isInt1 && isInt2) {
            slice->edge1.dx = dx * 2;
            slice->edge1.dy = dy * 2;
        }
        else {
            scale = (dx > dy) ? dx : dy;
            slice->edge1.dx = floor((dx * 32768) / scale + .5);
            slice->edge1.dy = floor((dy * 32768) / scale + .5);
        }

        if (!slice->edge1.dy) {
            if (signdx < 0) {
                y = floor(y1 + 1.0);
                if (y >= 0) {
                    slice->min_top_y = y;
                    slice->min_bot_y = arc->height;
                }
                else {
                    slice->max_bot_y = -y - (arc->height & 1);
                }
            }
            else {
                y = floor(y1);
                if (y >= 0)
                    slice->max_top_y = y;
                else {
                    slice->min_top_y = arc->height;
                    slice->min_bot_y = -y - (arc->height & 1);
                }
            }

            slice->edge1_top = TRUE;
            slice->edge1.x = 65536;
            slice->edge1.stepx = 0;
            slice->edge1.e = 0;
            slice->edge1.dx = -1;
            slice->edge2 = slice->edge1;
            slice->edge2_top = FALSE;
        }
        else if (!slice->edge1.dx) {
            if (signdy < 0)
                x1 -= 1.0;
            slice->edge1.x = ceil(x1);
            slice->edge1_top = signdy < 0;
            slice->edge1.x += arc->x + (arc->width >> 1);
            slice->edge1.stepx = 0;
            slice->edge1.e = 0;
            slice->edge1.dx = -1;
            slice->edge2_top = !slice->edge1_top;
            slice->edge2 = slice->edge1;
        }
        else {
            if (signdx < 0)
                slice->edge1.dx = -slice->edge1.dx;
            if (signdy < 0)
                slice->edge1.dx = -slice->edge1.dx;
            k = ceil(((x1 + x2) * slice->edge1.dy - (y1 + y2) * slice->edge1.dx) / 2.0);
            slice->edge2.dx = slice->edge1.dx;
            slice->edge2.dy = slice->edge1.dy;
            slice->edge1_top = signdy < 0;
            slice->edge2_top = !slice->edge1_top;
            miGetArcEdge(arc, &slice->edge1, k,
                         slice->edge1_top, !slice->edge1_top);
            miGetArcEdge(arc, &slice->edge2, k,
                         slice->edge2_top, slice->edge2_top);
        }
    }
#endif

}

#define ADDSPANS() \
    pts->x = xorg - x; \
    pts->y = yorg - y; \
    pts->width = slw; \
    pts++; \
    if (miFillArcLower(slw)) \
    { \
        pts->x = xorg - x; \
        pts->y = yorg + y + dy; \
        pts->width = slw; \
        pts++; \
    }

static void miFillEllipseI (PDC pdc, miArc* arc)
{
    register int x, y, e;
    int yk, xk, ym, xm, dx, dy, xorg, yorg;
    int slw;
    miFillArcRec info;
    Span* points;
    register Span* pts;

    points = (Span*)ALLOCATE_LOCAL(sizeof(Span) * arc->height);
    if (!points)
        return;

    miFillArcSetup(arc, &info);
    MIFILLARCSETUP();

    pts = points;
    while (y > 0) {
        MIFILLARCSTEP(slw);
        ADDSPANS();
    }

    if (pdc->brush_type == BT_SOLID) {
        pdc->cur_pixel = pdc->brushcolor;
        _dc_fill_spans (pdc, points, pts - points, FALSE);
    }
    else
        _dc_fill_spans_brush (pdc, points, pts - points, FALSE);

    DEALLOCATE_LOCAL(points);
}

static void miFillEllipseD (PDC pdc, miArc* arc)
{
    register int x, y;
    int xorg, yorg, dx, dy, slw;
    double e, yk, xk, ym, xm;
    miFillArcDRec info;
    Span* points;
    register Span* pts;

    points = (Span*)ALLOCATE_LOCAL(sizeof(Span) * arc->height);
    if (!points)
        return;

    miFillArcDSetup (arc, &info);
    MIFILLARCSETUP ();

    pts = points;
    while (y > 0) {
        MIFILLARCSTEP(slw);
        ADDSPANS();
    }

    if (pdc->brush_type == BT_SOLID) {
        pdc->cur_pixel = pdc->brushcolor;
        _dc_fill_spans (pdc, points, pts - points, FALSE);
    }
    else
        _dc_fill_spans_brush (pdc, points, pts - points, FALSE);

    DEALLOCATE_LOCAL(points);
}

#define ADDSPAN(l,r) \
    if (r >= l) \
    { \
        pts->x = l; \
        pts->y = ya; \
        pts->width = r - l + 1; \
        pts++; \
    }

#define ADDSLICESPANS(flip) \
    if (!flip) \
    { \
        ADDSPAN(xl, xr); \
    } \
    else \
    { \
        xc = xorg - x; \
        ADDSPAN(xc, xr); \
        xc += slw - 1; \
        ADDSPAN(xl, xc); \
    }

static void miFillArcSliceI(PDC pdc, miArc* arc)
{
    int yk, xk, ym, xm, dx, dy, xorg, yorg, slw;
    register int x, y, e;
    miFillArcRec info;
    miArcSliceRec slice;
    int ya, xl, xr, xc;
    Span* points;
    register Span* pts;

    miFillArcSetup (arc, &info);
    miFillArcSliceSetup (arc, &slice, pdc);
    MIFILLARCSETUP ();

    slw = arc->height;
    if (slice.flip_top || slice.flip_bot)
        slw += (arc->height >> 1) + 1;

    points = (Span*)ALLOCATE_LOCAL(sizeof(Span) * slw);
    if (!points)
        return;

    pts = points;
    while (y > 0) {
        MIFILLARCSTEP (slw);
        MIARCSLICESTEP (slice.edge1);
        MIARCSLICESTEP (slice.edge2);

        if (miFillSliceUpper (slice)) {
            ya = yorg - y;
            MIARCSLICEUPPER (xl, xr, slice, slw);
            ADDSLICESPANS (slice.flip_top);
        }

        if (miFillSliceLower(slice)) {
            ya = yorg + y + dy;
            MIARCSLICELOWER (xl, xr, slice, slw);
            ADDSLICESPANS (slice.flip_bot);
        }
    }

    if (pdc->brush_type == BT_SOLID) {
        pdc->cur_pixel = pdc->brushcolor;
        _dc_fill_spans (pdc, points, pts - points, FALSE);
    }
    else
        _dc_fill_spans_brush (pdc, points, pts - points, FALSE);

    DEALLOCATE_LOCAL(points);
}

static void miFillArcSliceD (PDC pdc, miArc* arc)
{
    register int x, y;
    int dx, dy, xorg, yorg, slw;
    double e, yk, xk, ym, xm;
    miFillArcDRec info;
    miArcSliceRec slice;
    int ya, xl, xr, xc;
    Span* points;
    register Span* pts;

    miFillArcDSetup (arc, &info);
    miFillArcSliceSetup (arc, &slice, pdc);
    MIFILLARCSETUP ();

    slw = arc->height;
    if (slice.flip_top || slice.flip_bot)
        slw += (arc->height >> 1) + 1;

    points = (Span*)ALLOCATE_LOCAL (sizeof(Span) * slw);
    if (!points)
        return;

    pts = points;
    while (y > 0) {
        MIFILLARCSTEP (slw);
        MIARCSLICESTEP (slice.edge1);
        MIARCSLICESTEP (slice.edge2);
        if (miFillSliceUpper (slice)) {
            ya = yorg - y;
            MIARCSLICEUPPER (xl, xr, slice, slw);
            ADDSLICESPANS (slice.flip_top);
        }
        if (miFillSliceLower (slice)) {
            ya = yorg + y + dy;
            MIARCSLICELOWER (xl, xr, slice, slw);
            ADDSLICESPANS (slice.flip_bot);
        }
    }

    pdc->cur_pixel = pdc->brushcolor;
    _dc_fill_spans (pdc, points, pts - points, FALSE);

    DEALLOCATE_LOCAL (points);
}

/* MIPOLYFILLARC -- The public entry for the PolyFillArc request.
 * Since we don't have to worry about overlapping segments, we can just
 * fill each arc as it comes.
 */
void miPolyFillArc (PDC pdc, int narcs, miArc* parcs)
{
    register int i;
    register miArc *arc;

    for (i = narcs, arc = parcs; --i >= 0; arc++) {
        if (miFillArcEmpty (arc))
            continue;

        if ((arc->angle2 >= FULLCIRCLE) || (arc->angle2 <= -FULLCIRCLE)) {
            if (miCanFillArc (arc))
                miFillEllipseI (pdc, arc);
            else
                miFillEllipseD (pdc, arc);
        }
        else {
            if (miCanFillArc (arc))
                miFillArcSliceI (pdc, arc);
            else
                miFillArcSliceD (pdc, arc);
        }
    }
}

#endif /* _MGHAVE_ADV_2DAPI */
