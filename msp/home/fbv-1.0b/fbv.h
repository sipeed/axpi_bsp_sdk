/*
    fbv  --  simple image viewer for the linux framebuffer
    Copyright (C) 2000, 2001, 2003  Mateusz Golicz

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#define FH_ERROR_OK 0
#define FH_ERROR_FILE 1		/* read/access error */
#define FH_ERROR_FORMAT 2	/* file format error */

void fb_display(unsigned char *rgbbuff, unsigned char * alpha, int x_size, int y_size, int x_pan, int y_pan, int x_offs, int y_offs);
void getCurrentRes(int *x, int *y);

int fh_bmp_id(char *name);
int fh_bmp_load(char *name,unsigned char *buffer, unsigned char **alpha, int x,int y);
int fh_bmp_getsize(char *name,int *x,int *y);

int fh_jpeg_id(char *name);
int fh_jpeg_load(char *name,unsigned char *buffer, unsigned char **alpha, int x,int y);
int fh_jpeg_getsize(char *name,int *x,int *y);

int fh_png_id(char *name);
int fh_png_load(char *name,unsigned char *buffer, unsigned char **alpha, int x,int y);
int fh_png_getsize(char *name,int *x,int *y);

int fh_gif_id(char *name);
int fh_gif_load(char *name,unsigned char *buffer, unsigned char **alpha, int x,int y);
int fh_gif_getsize(char *name,int *x,int *y);

struct image
{
	int width, height;
	unsigned char *rgb;
	unsigned char *alpha;
	int do_free;
};

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif
unsigned char * simple_resize(unsigned char * orgin,int ox,int oy,int dx,int dy);
unsigned char * alpha_resize(unsigned char * alpha,int ox,int oy,int dx,int dy);
unsigned char * color_average_resize(unsigned char * orgin,int ox,int oy,int dx,int dy);
unsigned char * rotate(unsigned char *i, int ox, int oy, int rot);
unsigned char * alpha_rotate(unsigned char *i, int ox, int oy, int rot);
