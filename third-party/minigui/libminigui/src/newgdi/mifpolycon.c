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

/***********************************************************
Copyright 1987, 1998  The Open Group

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


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $TOG: mifpolycon.c /main/13 1998/02/09 14:47:05 kaleb $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
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

static int GetFPolyYBounds(register SppPointPtr pts, int n, double yFtrans, int* by, int* ty);

#ifdef ICEILTEMPDECL
ICEILTEMPDECL
#endif

/*
 *        Written by Todd Newman; April. 1987.
 *
 *        Fill a convex polygon.  If the given polygon
 *        is not convex, then the result is undefined.
 *        The algorithm is to order the edges from smallest
 *        y to largest by partitioning the array into a left
 *        edge list and a right edge list.  The algorithm used
 *        to traverse each edge is digital differencing analyzer
 *        line algorithm with y as the major axis. There's some funny linear
 *        interpolation involved because of the subpixel postioning.
 *
 * count:               number of points
 * ptsIn:               the points
 * xTrans, yTrans:      Translate each point by this
 * xFtrans, yFtrans:    translate before conversion
 *                      by this amount.  This provides
 *                      a mechanism to match rounding
 *                      errors with any shape that must
 *                      meet the polygon exactly.
 */
void miFillSppPoly (PDC pdc, int count, SppPointPtr ptsIn, int xTrans, int yTrans, double xFtrans, double yFtrans)
{
    double          xl = 0, xr = 0, /* x vals of left and right edges */
                    ml = 0,         /* left edge slope */
                    mr = 0,         /* right edge slope */
                    dy,             /* delta y */
                    i;              /* loop counter */
    int             y,              /* current scanline */
                    j,
                    imin,           /* index of vertex with smallest y */
                    ymin,           /* y-extents of polygon */
                    ymax,
                    *Marked;        /* set if this vertex has been used */
    register int    left, right,    /* indices to first endpoints */
                    nextleft,
                    nextright;      /* indices to second endpoints */
    Span            *ptsOut, *FirstPoint;   /* output buffer */

    imin = GetFPolyYBounds(ptsIn, count, yFtrans, &ymin, &ymax);

    y = ymax - ymin + 1;
    if ((count < 3) || (y <= 0))
        return;

    ptsOut = FirstPoint = (Span*)ALLOCATE_LOCAL(sizeof(Span) * y);
    Marked = (int *) ALLOCATE_LOCAL(sizeof(int) * count);

    if(!ptsOut || !Marked)
    {
        if (Marked) DEALLOCATE_LOCAL(Marked);
        if (ptsOut) DEALLOCATE_LOCAL(ptsOut);
        return;
    }

    for(j = 0; j < count; j++)
        Marked[j] = 0;
    nextleft = nextright = imin;
    Marked[imin] = -1;
    y = ICEIL(ptsIn[nextleft].y + yFtrans);

    /*
     *  loop through all edges of the polygon
     */
    do
    {
        /* add a left edge if we need to */
        if ((y > (ptsIn[nextleft].y + yFtrans) ||
              ISEQUAL(y, ptsIn[nextleft].y + yFtrans)) &&
             Marked[nextleft] != 1)
        {
            Marked[nextleft]++;
            left = nextleft++;

            /* find the next edge, considering the end conditions */
            if (nextleft >= count)
                nextleft = 0;

            /* now compute the starting point and slope */
            dy = ptsIn[nextleft].y - ptsIn[left].y;
            if (dy != 0.0)
            {
                ml = (ptsIn[nextleft].x - ptsIn[left].x) / dy;
                dy = y - (ptsIn[left].y + yFtrans);
                xl = (ptsIn[left].x + xFtrans) + ml * MAX(dy, 0);
            }
        }

        /* add a right edge if we need to */
        if ((y > ptsIn[nextright].y + yFtrans) ||
              (ISEQUAL(y, ptsIn[nextright].y + yFtrans)
              && Marked[nextright] != 1))
        {
            Marked[nextright]++;
            right = nextright--;

            /* find the next edge, considering the end conditions */
            if (nextright < 0)
                nextright = count - 1;

            /* now compute the starting point and slope */
            dy = ptsIn[nextright].y - ptsIn[right].y;
            if (dy != 0.0)
            {
                mr = (ptsIn[nextright].x - ptsIn[right].x) / dy;
                dy = y - (ptsIn[right].y + yFtrans);
                xr = (ptsIn[right].x + xFtrans) + mr * MAX(dy, 0);
            }
        }


        /*
         *  generate scans to fill while we still have
         *  a right edge as well as a left edge.
         */
        i = (MIN(ptsIn[nextleft].y, ptsIn[nextright].y) + yFtrans) - y;

        if (i < EPSILON)
        {
            if(Marked[nextleft] && Marked[nextright])
            {
                /* Arrgh, we're trapped! (no more points)
                 * Out, we've got to get out of here before this decadence saps
                 * our will completely! */
                break;
            }
            continue;
        }
        else
        {
                j = (int) i;
                if(!j)
                    j++;
        }
        while (j > 0)
        {
            int cxl, cxr;

            ptsOut->y = (y) + yTrans;

            cxl = ICEIL(xl);
            cxr = ICEIL(xr);
            /* reverse the edges if necessary */
            if (xl < xr)
            {
              ptsOut->width = cxr - cxl;
              (ptsOut++)->x = cxl + xTrans;
            }
            else
            {
              ptsOut->width = cxl - cxr;
              (ptsOut++)->x = cxr + xTrans;
            }
            y++;

            /* increment down the edges */
            xl += ml;
            xr += mr;
            j--;
        }
    }  while (y <= ymax);

    /* Finally, fill the spans we've collected */
    _dc_fill_spans (pdc, FirstPoint, ptsOut-FirstPoint, FALSE);
    DEALLOCATE_LOCAL(Marked);
    DEALLOCATE_LOCAL(FirstPoint);
}


/* Find the index of the point with the smallest y.also return the
 * smallest and largest y */
static int GetFPolyYBounds(register SppPointPtr pts, int n, double yFtrans, int* by, int* ty)
{
    register SppPointPtr        ptMin;
    double                         ymin, ymax;
    SppPointPtr                        ptsStart = pts;

    ptMin = pts;
    ymin = ymax = (pts++)->y;

    while (--n > 0) {
        if (pts->y < ymin)
        {
            ptMin = pts;
            ymin = pts->y;
        }
        if(pts->y > ymax)
            ymax = pts->y;

        pts++;
    }

    *by = ICEIL(ymin + yFtrans);
    *ty = ICEIL(ymax + yFtrans - 1);
    return(ptMin-ptsStart);
}

#endif /* _MGHAVE_ADV_2DAPI */
