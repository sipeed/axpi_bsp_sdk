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
#define dbg() printf("%s %d\n", __FUNCTION__, __LINE__)

/*
**  $Id: aichip_fbvideo.c 12768 2020-06-2 05:28:30Z wanhu $
**
**  Copyright (C) 2003 ~ 2007 Feynman Software.
**  Copyright (C) 2001 ~ 2002 Wei Yongming.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "minigui.h"
#include "newgal.h"
#include "sysvideo.h"
#include "pixels_c.h"

#ifdef _MGGAL_AICHIP

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
//#include <drm/drm.h>
//#include <drm/drm_fourcc.h>
//#include <xf86drm.h>
//#include <xf86drmMode.h>

#include "aichip_drmvideo.h"

#define AICHIPVID_DRIVER_NAME "aichip"

/* Initialization/Query functions */
static int AICHIP_VideoInit(_THIS, GAL_PixelFormat *vformat);
static GAL_Rect **AICHIP_ListModes(_THIS, GAL_PixelFormat *format, Uint32 flags);
static GAL_Surface *AICHIP_SetVideoMode(_THIS, GAL_Surface *current, int width, int height, int bpp, Uint32 flags);
static void AICHIP_VideoQuit(_THIS);
static void AICHIP_UpdateRects (_THIS, int numrects, GAL_Rect *rects);
static BOOL AICHIP_SyncUpdate (_THIS);

static inline void round_rect_to_even (RECT* rc)
{
    if (rc->left % 2) rc->left--;
    if (rc->right % 2) rc->right++;
    if (rc->top % 2) rc->top--;
    if (rc->bottom % 2) rc->bottom++;
}

static __inline BYTE rgb2y (BYTE r, BYTE g, BYTE b)
{
    DWORD y = 257*(DWORD)r + 504*(DWORD)g + 98*(DWORD)b + 16000;
    if (y > 255000)
        y = 255000;
    y /= 1000;
    return (BYTE)y;
}

static __inline BYTE rgb2u (BYTE r, BYTE g, BYTE b)
{
    DWORD u = -148*(DWORD)r - 291*(DWORD)g + 449*(DWORD)b + 128000;
    if (u > 255000)
        u = 255000;
    u /= 1000;
    return (BYTE)u;
}

static __inline BYTE rgb2v (BYTE r, BYTE g, BYTE b)
{
    DWORD v = 439*(DWORD)r - 368*(DWORD)g - 71*(DWORD)b + 128000;
    if (v > 255000)
        v = 255000;
    v /= 1000;
    return (BYTE)v;
}

//Convert two rows from RGB to two Y rows, and one row of interleaved U,V.
//I0 and I1 points two sequential source rows.
//I0 -> rgbrgbrgbrgbrgbrgb...
//I1 -> rgbrgbrgbrgbrgbrgb...
//Y0 and Y1 points two sequential destination rows of Y plane.
//Y0 -> yyyyyy
//Y1 -> yyyyyy
//UV0 points destination rows of interleaved UV plane.
//UV0 -> uvuvuv
static void rgb2nv12tworows(const BYTE I0[], const BYTE I1[], int step, const int w, BYTE Y0[], BYTE Y1[], BYTE UV0[])
{
    int x;

    for (x = 0; x < w; x += 2) {
        BYTE r00 = I0[x*step + 0];
        BYTE g00 = I0[x*step + 1];
	    BYTE b00 = I0[x*step + 2];

		BYTE r01 = I0[x*step + step+0];
		BYTE g01 = I0[x*step + step+1];
		BYTE b01 = I0[x*step + step+2];

		BYTE r10 = I1[x*step + 0];
		BYTE g10 = I1[x*step + 1];
		BYTE b10 = I1[x*step + 2];

		BYTE r11 = I1[x*step + step+0];
		BYTE g11 = I1[x*step + step+1];
		BYTE b11 = I1[x*step + step+2];

		BYTE y00 = rgb2y(r00, g00, b00);
		BYTE y01 = rgb2y(r01, g01, b01);
		BYTE y10 = rgb2y(r10, g10, b10);
		BYTE y11 = rgb2y(r11, g11, b11);

		BYTE u00 = rgb2u(r00, g00, b00);
		BYTE u01 = rgb2u(r01, g01, b01);
		BYTE u10 = rgb2u(r10, g10, b10);
		BYTE u11 = rgb2u(r11, g11, b11);

		BYTE v00 = rgb2v(r00, g00, b00);
		BYTE v01 = rgb2v(r01, g01, b01);
		BYTE v10 = rgb2v(r10, g10, b10);
		BYTE v11 = rgb2v(r11, g11, b11);

		BYTE u0 = (u00 + u01 + u10 + u11) / 4;
		BYTE v0 = (v00 + v01 + v10 + v11) / 4;

		Y0[x + 0] = y00;
		Y0[x + 1] = y01;
		Y1[x + 0] = y10;
		Y1[x + 1] = y11;

		UV0[x + 0] = u0;
		UV0[x + 1] = v0;
	}
}
 
 
//Convert image I from pixel ordered RGB to NV12 format.
//I - Input image in pixel ordered RGB format
//image_width - Number of columns of I
//image_height - Number of rows of I
//J - Destination "image" in NV12 format.
 
//I is pixel ordered RGB color format (size in bytes is image_width*image_height*3):
//RGBRGBRGBRGBRGBRGB
//RGBRGBRGBRGBRGBRGB
//RGBRGBRGBRGBRGBRGB
//RGBRGBRGBRGBRGBRGB
//
//J is in NV12 format (size in bytes is image_width*image_height*3/2):
//YYYYYY
//YYYYYY
//UVUVUV
//Each element of destination U is average of 2x2 "original" U elements
//Each element of destination V is average of 2x2 "original" V elements
//
//Limitations:
//1. image_width must be a multiple of 2.
//2. image_height must be a multiple of 2.
//3. I and J must be two separate arrays (in place computation is not supported). 
#if 1
void rgb2nv12(int w, int h, const BYTE *rgb, int step, int rgb_stride, BYTE *y, BYTE *uv, int yuv_stride)
{
	const unsigned char *r0;
	const unsigned char *r1;

	unsigned char *y0;
	unsigned char *y1;

	unsigned char *uv0;
	int row;

	for (row = 0; row < h; row += 2) {
		r0 = &rgb[row * step * rgb_stride];
		r1 = &rgb[(row + 1) * step * rgb_stride];

		y0 = &y[row * yuv_stride];
		y1 = &y[(row + 1) * yuv_stride];

		uv0 = &uv[(row/2) * yuv_stride];

		rgb2nv12tworows(r0, r1, step, w, y0, y1, uv0);
	}
}
#else
void rgb2nv12(const BYTE *rgb, int step, const int width, const int height, BYTE *nv12)
{
	//In NV12 format, UV plane starts below Y plane.
	unsigned char *UV = &nv12[width * height];

	//I0 and I1 points two sequential source rows.
	const unsigned char *I0;	//I0 -> rgbrgbrgbrgbrgbrgb...
	const unsigned char *I1;	//I1 -> rgbrgbrgbrgbrgbrgb...

	//Y0 and Y1 points two sequential destination rows of Y plane.
	unsigned char *Y0;	//Y0 -> yyyyyy
	unsigned char *Y1;	//Y1 -> yyyyyy

	//UV0 points destination rows of interleaved UV plane.
	unsigned char *UV0;	//UV0 -> uvuvuv

	int y;	//Row index

	//In each iteration: process two rows of Y plane, and one row of interleaved UV plane.
	for (y = 0; y < height; y += 2) {
		I0 = &rgb[y * width * step];	//Input row width is width*3 bytes (each pixel is R,G,B).
		I1 = &rgb[(y + 1) * width * step];

		Y0 = &nv12[y * width];	//Output Y row width is width bytes (one Y element per pixel).
		Y1 = &nv12[(y + 1) * width];

		UV0 = &UV[(y/2) * width];	//Output UV row - width is same as Y row width.

		//Process two source rows into: Two Y destination row, and one destination interleaved U,V row.
		rgb2nv12tworows(I0, I1, step, width, Y0, Y1, UV0);
	}
}
#endif

static int AICHIP_Available(void)
{
    return 1;
}

static void AICHIP_DeleteDevice(GAL_VideoDevice *device)
{
    free(device->hidden);
    free(device);
}

static GAL_VideoDevice *AICHIP_CreateDevice(int devindex)
{
    GAL_VideoDevice *device;

    /* Initialize all variables that we clean on shutdown */
    device = (GAL_VideoDevice *)malloc(sizeof(GAL_VideoDevice));
    if ( device ) {
        memset(device, 0, (sizeof *device));
        device->hidden = (struct GAL_PrivateVideoData *)
                malloc((sizeof *device->hidden));
    }
    if ( (device == NULL) || (device->hidden == NULL) ) {
        GAL_OutOfMemory();
        if ( device ) {
            free(device);
        }
        return(0);
    }
    memset(device->hidden, 0, (sizeof *device->hidden));

    /* Set the function pointers */
    device->VideoInit = AICHIP_VideoInit;
    device->ListModes = AICHIP_ListModes;
    device->SetVideoMode = AICHIP_SetVideoMode;
    device->VideoQuit = AICHIP_VideoQuit;
    device->UpdateRects = AICHIP_UpdateRects;
    device->SyncUpdate = AICHIP_SyncUpdate;

    device->free = AICHIP_DeleteDevice;

	sem_init(&device->hidden->sem, 0, 0);
    pthread_mutex_init (&device->hidden->lock, NULL);

    return device;
}

VideoBootStrap AICHIP_bootstrap = {
    AICHIPVID_DRIVER_NAME, "AICHIP video driver",
    AICHIP_Available, AICHIP_CreateDevice
};

char GAL_fbdev[] = "/dev/fb0";

static void print_vinfo(struct fb_var_screeninfo *vinfo)
{
    fprintf(stderr, "Printing vinfo:\n");
    fprintf(stderr, "txres: %d\n", vinfo->xres);
    fprintf(stderr, "tyres: %d\n", vinfo->yres);
    fprintf(stderr, "txres_virtual: %d\n", vinfo->xres_virtual);
    fprintf(stderr, "tyres_virtual: %d\n", vinfo->yres_virtual);
    fprintf(stderr, "txoffset: %d\n", vinfo->xoffset);
    fprintf(stderr, "tyoffset: %d\n", vinfo->yoffset);
    fprintf(stderr, "tbits_per_pixel: %d\n", vinfo->bits_per_pixel);
    fprintf(stderr, "tgrayscale: %d\n", vinfo->grayscale);
    fprintf(stderr, "tnonstd: %d\n", vinfo->nonstd);
    fprintf(stderr, "tactivate: %d\n", vinfo->activate);
    fprintf(stderr, "theight: %d\n", vinfo->height);
    fprintf(stderr, "twidth: %d\n", vinfo->width);
    fprintf(stderr, "taccel_flags: %d\n", vinfo->accel_flags);
    fprintf(stderr, "tpixclock: %d\n", vinfo->pixclock);
    fprintf(stderr, "tleft_margin: %d\n", vinfo->left_margin);
    fprintf(stderr, "tright_margin: %d\n", vinfo->right_margin);
    fprintf(stderr, "tupper_margin: %d\n", vinfo->upper_margin);
    fprintf(stderr, "tlower_margin: %d\n", vinfo->lower_margin);
    fprintf(stderr, "thsync_len: %d\n", vinfo->hsync_len);
    fprintf(stderr, "tvsync_len: %d\n", vinfo->vsync_len);
    fprintf(stderr, "tsync: %d\n", vinfo->sync);
    fprintf(stderr, "tvmode: %d\n", vinfo->vmode);
    fprintf(stderr, "tred: %d/%d\n", vinfo->red.length, vinfo->red.offset);
    fprintf(stderr, "tgreen: %d/%d\n", vinfo->green.length, vinfo->green.offset);
    fprintf(stderr, "tblue: %d/%d\n", vinfo->blue.length, vinfo->blue.offset);
    fprintf(stderr, "talpha: %d/%d\n", vinfo->transp.length, vinfo->transp.offset);
}

static int AICHIP_VideoInit(_THIS, GAL_PixelFormat *vformat)
{
    struct GAL_PrivateVideoData *hidden = this->hidden;
    struct fb_var_screeninfo vinfo;

    hidden->fd = open(GAL_fbdev, O_RDWR, 0);
    if (hidden->fd < 0) {
        GAL_SetError("NEWGAL>FBCON: Unable to open %s\n", GAL_fbdev);
        return(-1);
    }

#if 0
    if (ioctl(hidden->fd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
        GAL_SetError("NEWGAL>FBCON: Couldn't get console pixel format\n");
        AICHIP_VideoQuit(this);
        return(-1);
    }

	print_vinfo(&vinfo);
#endif

    vformat->BitsPerPixel = 24;
    vformat->BytesPerPixel = 3;
	vformat->Rshift = 16;
	vformat->Gshift = 8;
	vformat->Bshift = 0;
	vformat->Rmask = 0xFF0000;
	vformat->Gmask = 0xFF00;
	vformat->Bmask = 0xFF;
	vformat->Amask = 0;


    this->hidden->dirty = FALSE;
    SetRect (&hidden->update, 0, 0, 0, 0);

    return(0);

}

static GAL_Rect **AICHIP_ListModes(_THIS, GAL_PixelFormat *format, Uint32 flags)
{
    if (format->BitsPerPixel < 24) {
        return NULL;
    }

    /* any size is ok */
    return (GAL_Rect**) -1;
}


static inline void update_rect_to_nv12(_THIS, RECT* rect)
{
    struct GAL_PrivateVideoData *hidden = this->hidden;
	int step = ((hidden->bpp + 7) / 8);
	int reso_w = hidden->width;
	int nv12_stride = (hidden->width + 0xff) & (~0xff);
	int w = rect->right - rect->left;
	int h = rect->bottom - rect->top;
#if 1
	BYTE * rgb = this->screen->pixels + rect->top * hidden->pitch + rect->left * step;
	BYTE * y = hidden->real_screen + rect->top * nv12_stride + rect->left;
	BYTE * uv = y + hidden->height * nv12_stride + (rect->top * nv12_stride >> 1) + rect->left;

	printf("%s rgb = %p, step = %d, w = %d, h = %d, reso_w = %d, y = %p, uv = %p\n",
		__func__, rgb, step, w, h, reso_w, y, uv);
	rgb2nv12(w, h, rgb, step, reso_w, y, uv, nv12_stride);
#else
	BYTE * rgb = this->screen->pixels;
	BYTE * y = hidden->real_screen;
	BYTE * uv = y + hidden->height * reso_w;

	printf("%s rgb = %p, step = %d, w = %d, h = %d, reso_w = %d, y = %p, uv = %p\n",
		__func__, rgb, step, w, h, reso_w, y, uv);
	rgb2nv12(rgb, step, hidden->width, hidden->height, reso_w, y, uv);
#endif
}

static BOOL do_update (_THIS)
{
    struct GAL_PrivateVideoData *hidden = this->hidden;
    struct fb_var_screeninfo vinfo;

	if (ioctl(hidden->fd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
		_ERR_PRINTF ("FBIOGET_VSCREENINFO failed\n");
		return FALSE;
	}

	if (ioctl(hidden->fd, FBIOPAN_DISPLAY, &vinfo) < 0) {
		_ERR_PRINTF ("FBIOPAN_DISPLAY failed\n");
		return FALSE;
	}

	return TRUE;
}

static void* thread_do_update (void* data)
{
	int policy;
    GAL_VideoDevice *this = data;
	pthread_attr_t attr;
    struct fb_var_screeninfo vinfo;

	if (pthread_attr_init(&attr)) {
		printf("%s pthread attr init failed\n", __func__);
		return NULL;
	}

	if (pthread_attr_getschedpolicy (&attr, &policy)) {
		printf("%s pthread attr getschedpolicy failed\n", __func__);
		return NULL;
	}
	printf("%s policy = %d\n", __func__, policy);

	policy = SCHED_FIFO;
	if (pthread_attr_setschedpolicy(&attr, policy)) {
		printf("%s pthread attr setschedpolicy failed\n", __func__);
		return NULL;
	}

    while (this->hidden->real_screen) {
		sem_wait(&this->hidden->sem);
		printf("%s start update\n", __func__);
		pthread_mutex_lock (&this->hidden->lock);
        if (this->hidden->dirty) {
            RECT bound;

            bound = this->hidden->update;
            SetRect (&this->hidden->update, 0, 0, 0, 0);
            this->hidden->dirty = FALSE;
            round_rect_to_even (&bound);

			printf("%s start update to nv12\n", __func__);
            update_rect_to_nv12(this, &bound);
			printf("%s update to nv12 done\n", __func__);
			do_update(this);
        }
		pthread_mutex_unlock (&this->hidden->lock);
    }

    return 0;
}

static GAL_Surface *AICHIP_SetVideoMode(_THIS, GAL_Surface *current,
                int width, int height, int bpp, Uint32 flags)
{
    int fd, pitch;
    struct GAL_PrivateVideoData *hidden = this->hidden;
    Uint32 Rmask = 0xFF;
    Uint32 Gmask = 0xFF00;
    Uint32 Bmask = 0xFF0000;
    Uint32 Amask = 0xFF000000;

    pitch = width * ((bpp + 7) / 8);
    pitch = (pitch + 3) & ~3;

	printf("%s bpp = %d, width = %d, height = %d, pitch = %d\n", __func__, bpp, width, height, pitch);

	current->pixels = malloc (pitch * height);
	if (!current->pixels) {
		printf("%s malloc pixels buffer failed\n", __func__);
	}

	hidden->real_screen = mmap(NULL, ((width + 0xff) & ~(0xff)) * height * 3 / 2,
                      PROT_READ|PROT_WRITE, MAP_SHARED, hidden->fd, 0);
	if (hidden->real_screen == MAP_FAILED) {
		_ERR_PRINTF ("NEWGAL>DUMMY: "
                "Couldn't allocate buffer for requested mode\n");
        return NULL;
    }
	printf("%s hidden->buffer = %p\n", __func__, current->pixels);
	printf("%s hidden->real_screen = %p\n", __func__, hidden->real_screen);

    memset (current->pixels, 0, pitch * height);

    /* Allocate the new pixel format for the screen */
    if (!GAL_ReallocFormat (current, bpp, Rmask, Gmask, Bmask, Amask)) {
        free(current->pixels);
        _ERR_PRINTF ("NEWGAL>DUMMY: "
                "Couldn't allocate new pixel format for requested mode\n");
        return(NULL);
    }

    /* Set up the new mode framebuffer */
    current->flags = GAL_FULLSCREEN;
    current->w = width;
    current->h = height;
    current->pitch = pitch;

	hidden->width = current->w;
	hidden->height = current->h;
	hidden->bpp = bpp;
	hidden->pitch = current->pitch;

    if(pthread_create (&this->hidden->th, NULL, thread_do_update, this))
        goto err;

    return current;

err:
	return NULL;
}

/* Note:  If we are terminated, this could be called in the middle of
   another video routine -- notably UpdateRects.
*/
static void AICHIP_VideoQuit(_THIS)
{
    struct GAL_PrivateVideoData *hidden = this->hidden;

    pthread_mutex_destroy (&hidden->lock);
	sem_destroy(&hidden->sem);

    if (this->screen->pixels != NULL) {
        free (this->screen->pixels);
        this->screen->pixels = NULL;
    }

    if (hidden->fd > 0) {
        /* Unmap the video framebuffer and I/O registers */
        if (hidden->real_screen) {
            munmap(hidden->real_screen, ((hidden->width + 0xff) & 0xff) * hidden->height);
            hidden->real_screen = NULL;
        }
    }

	free(hidden);
}

static void AICHIP_UpdateRects (_THIS, int numrects, GAL_Rect *rects)
{
    int i;
    RECT bound;

	printf("%s numrects = %d\n", __func__, numrects);

    pthread_mutex_lock (&this->hidden->lock);

	printf("%s locked\n", __func__);

    bound = this->hidden->update;
    for (i = 0; i < numrects; i++) {
        RECT rc;

        SetRect (&rc, rects[i].x, rects[i].y,
                 rects[i].x + rects[i].w, rects[i].y + rects[i].h);
        if (IsRectEmpty (&bound))
            bound = rc;
        else
            GetBoundRect (&bound, &bound, &rc);
    }

    if (!IsRectEmpty (&bound)) {
        RECT rcScr = GetScreenRect();

        if (IntersectRect (&bound, &bound, &rcScr)) {
            this->hidden->update = bound;
            this->hidden->dirty = TRUE;
        }
    }

	printf("%s bound {(%d, %d),(%d, %d)}\n", __func__,
		bound.left, bound.top, bound.right, bound.bottom);

    pthread_mutex_unlock (&this->hidden->lock);

	sem_post(&this->hidden->sem);

}

static BOOL AICHIP_SyncUpdate (_THIS)
{
    struct GAL_PrivateVideoData *hidden = this->hidden;
    struct fb_var_screeninfo vinfo;
#if 0
	if (ioctl(hidden->fd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
		_ERR_PRINTF ("FBIOGET_VSCREENINFO failed\n");
		return FALSE;
	}

	if (ioctl(hidden->fd, FBIOPAN_DISPLAY, &vinfo) < 0) {
		_ERR_PRINTF ("FBIOPAN_DISPLAY failed\n");
		return FALSE;
	}
#endif
	return TRUE;
}
#endif /* _MGGAL_AICHIP */
