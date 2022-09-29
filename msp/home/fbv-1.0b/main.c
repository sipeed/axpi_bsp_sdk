/*
	fbv  --  simple image viewer for the linux framebuffer
	Copyright (C) 2000, 2001, 2003, 2004  Mateusz 'mteg' Golicz

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

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>
#include <signal.h>
#include "config.h"
#include "fbv.h"

#define PAN_STEPPING 20

static int opt_clear = 0,
	   opt_alpha = 0,
	   opt_hide_cursor = 1,
	   opt_image_info = 0,
	   opt_stretch = 0,
	   opt_delay = 0,
	   opt_enlarge = 0,
	   opt_ignore_aspect = 0;



void setup_console(int t)
{
	struct termios our_termios;
	static struct termios old_termios;

	if(t)
	{
		tcgetattr(0, &old_termios);
		memcpy(&our_termios, &old_termios, sizeof(struct termios));
		our_termios.c_lflag &= !(ECHO | ICANON);
		tcsetattr(0, TCSANOW, &our_termios);
	}
	else
		tcsetattr(0, TCSANOW, &old_termios);

}

static inline void do_rotate(struct image *i, int rot)
{
	if(rot)
	{
		unsigned char *image, *alpha = NULL;
		int t;

		image = rotate(i->rgb, i->width, i->height, rot);
		if(i->alpha)
			alpha = alpha_rotate(i->alpha, i->width, i->height, rot);
		if(i->do_free)
		{
			free(i->alpha);
			free(i->rgb);
		}

		i->rgb = image;
		i->alpha = alpha;
		i->do_free = 1;

		if(rot & 1)
		{
			t = i->width;
			i->width = i->height;
			i->height = t;
		}
	}
}


static inline void do_enlarge(struct image *i, int screen_width, int screen_height, int ignoreaspect)
{
	if(((i->width > screen_width) || (i->height > screen_height)) && (!ignoreaspect))
		return;
	if((i->width < screen_width) || (i->height < screen_height))
	{
		int xsize = i->width, ysize = i->height;
		unsigned char * image, * alpha = NULL;

		if(ignoreaspect)
		{
			if(i->width < screen_width)
				xsize = screen_width;
			if(i->height < screen_height)
				ysize = screen_height;

			goto have_sizes;
		}

		if((i->height * screen_width / i->width) <= screen_height)
		{
			xsize = screen_width;
			ysize = i->height * screen_width / i->width;
			goto have_sizes;
		}

		if((i->width * screen_height / i->height) <= screen_width)
		{
			xsize = i->width * screen_height / i->height;
			ysize = screen_height;
			goto have_sizes;
		}
		return;
have_sizes:
		image = simple_resize(i->rgb, i->width, i->height, xsize, ysize);
		if(i->alpha)
			alpha = alpha_resize(i->alpha, i->width, i->height, xsize, ysize);

		if(i->do_free)
		{
			free(i->alpha);
			free(i->rgb);
		}

		i->rgb = image;
		i->alpha = alpha;
		i->do_free = 1;
		i->width = xsize;
		i->height = ysize;
	}
}


static inline void do_fit_to_screen(struct image *i, int screen_width, int screen_height, int ignoreaspect, int cal)
{
	if((i->width > screen_width) || (i->height > screen_height))
	{
		unsigned char * new_image, * new_alpha = NULL;
		int nx_size = i->width, ny_size = i->height;

		if(ignoreaspect)
		{
			if(i->width > screen_width)
				nx_size = screen_width;
			if(i->height > screen_height)
				ny_size = screen_height;
		}
		else
		{
			if((i->height * screen_width / i->width) <= screen_height)
			{
				nx_size = screen_width;
				ny_size = i->height * screen_width / i->width;
			}
			else
			{
				nx_size = i->width * screen_height / i->height;
				ny_size = screen_height;
			}
		}

		if(cal)
			new_image = color_average_resize(i->rgb, i->width, i->height, nx_size, ny_size);
		else
			new_image = simple_resize(i->rgb, i->width, i->height, nx_size, ny_size);

		if(i->alpha)
			new_alpha = alpha_resize(i->alpha, i->width, i->height, nx_size, ny_size);

		if(i->do_free)
		{
			free(i->alpha);
			free(i->rgb);
		}

		i->rgb = new_image;
		i->alpha = new_alpha;
		i->do_free = 1;
		i->width = nx_size;
		i->height = ny_size;
	}
}


int show_image(char *filename)
{
	int (*load)(char *, unsigned char *, unsigned char **, int, int);

	unsigned char * image = NULL;
	unsigned char * alpha = NULL;

	int x_size, y_size, screen_width, screen_height;
	int x_pan, y_pan, x_offs, y_offs, refresh = 1, c, ret = 1;
	int delay = opt_delay, retransform = 1;

	int transform_stretch = opt_stretch, transform_enlarge = opt_enlarge, transform_cal = (opt_stretch == 2),
	    transform_iaspect = opt_ignore_aspect, transform_rotation = 0;

	struct image i;

#ifdef FBV_SUPPORT_GIF
	if(fh_gif_id(filename))
		if(fh_gif_getsize(filename, &x_size, &y_size) == FH_ERROR_OK)
		{
			load = fh_gif_load;
			goto identified;
		}
#endif

#ifdef FBV_SUPPORT_PNG
	if(fh_png_id(filename))
		if(fh_png_getsize(filename, &x_size, &y_size) == FH_ERROR_OK)
		{
			load = fh_png_load;
			goto identified;
		}
#endif

#ifdef FBV_SUPPORT_JPEG
	if(fh_jpeg_id(filename))
		if(fh_jpeg_getsize(filename, &x_size, &y_size) == FH_ERROR_OK)
		{
			load = fh_jpeg_load;
			goto identified;
		}
#endif

#ifdef FBV_SUPPORT_BMP
	if(fh_bmp_id(filename))
		if(fh_bmp_getsize(filename, &x_size, &y_size) == FH_ERROR_OK)
		{
			load = fh_bmp_load;
			goto identified;
		}
#endif
	fprintf(stderr, "%s: Unable to access file or file format unknown.\n", filename);
	return(1);

identified:

	if(!(image = (unsigned char*) malloc(x_size * y_size * 3)))
	{
		fprintf(stderr, "%s: Out of memory.\n", filename);
		goto error_mem;
	}

	if(load(filename, image, &alpha, x_size, y_size) != FH_ERROR_OK)
	{
		fprintf(stderr, "%s: Image data is corrupt?\n", filename);
		goto error_mem;
	}

	if(!opt_alpha)
	{
		free(alpha);
		alpha = NULL;
	}



	getCurrentRes(&screen_width, &screen_height);
	i.do_free = 0;
	while(1)
	{
		if(retransform)
		{
			if(i.do_free)
			{
				free(i.rgb);
				free(i.alpha);
			}
			i.width = x_size;
			i.height = y_size;
			i.rgb = image;
			i.alpha = alpha;
			i.do_free = 0;


			if(transform_rotation)
				do_rotate(&i, transform_rotation);

			if(transform_stretch)
				do_fit_to_screen(&i, screen_width, screen_height, transform_iaspect, transform_cal);

			if(transform_enlarge)
				do_enlarge(&i, screen_width, screen_height, transform_iaspect);

			x_pan = y_pan = 0;
			refresh = 1; retransform = 0;
			if(opt_clear)
			{
				printf("\033[H\033[J");
				fflush(stdout);
			}
			if(opt_image_info)
				printf("fbv - The Framebuffer Viewer\n%s\n%d x %d\n", filename, x_size, y_size);
		}
		if(refresh)
		{
			if(i.width < screen_width)
				x_offs = (screen_width - i.width) / 2;
			else
				x_offs = 0;

			if(i.height < screen_height)
				y_offs = (screen_height - i.height) / 2;
			else
				y_offs = 0;

			fb_display(i.rgb, i.alpha, i.width, i.height, x_pan, y_pan, x_offs, y_offs);
			refresh = 0;
		}
		if(delay)
		{
			struct timeval tv;
			fd_set fds;
			tv.tv_sec = delay / 10;
			tv.tv_usec = (delay % 10) * 100000;
			FD_ZERO(&fds);
			FD_SET(0, &fds);

			if(select(1, &fds, NULL, NULL, &tv) <= 0)
				break;
			delay = 0;
		}

		c = EOF; // 2022-09-06 getchar();
		switch(c)
		{
			case EOF:
			case 'q':
				ret = 0;
				goto done;
			case ' ': case 10: case 13:
				goto done;
			case '>': case '.':
				goto done;
			case '<': case ',':
				ret = -1;
				goto done;
			case 'r':
				refresh = 1;
				break;
			case 'a': case 'D':
				if(x_pan == 0) break;
				x_pan -= i.width / PAN_STEPPING;
				if(x_pan < 0) x_pan = 0;
				refresh = 1;
				break;
			case 'd': case 'C':
				if(x_offs) break;
				if(x_pan >= (i.width - screen_width)) break;
				x_pan += i.width / PAN_STEPPING;
				if(x_pan > (i.width - screen_width)) x_pan = i.width - screen_width;
				refresh = 1;
				break;
			case 'w': case 'A':
				if(y_pan == 0) break;
				y_pan -= i.height / PAN_STEPPING;
				if(y_pan < 0) y_pan = 0;
				refresh = 1;
				break;
			case 'x': case 'B':
				if(y_offs) break;
				if(y_pan >= (i.height - screen_height)) break;
				y_pan += i.height / PAN_STEPPING;
				if(y_pan > (i.height - screen_height)) y_pan = i.height - screen_height;
				refresh = 1;
				break;
			case 'f':
				transform_stretch = !transform_stretch;
				retransform = 1;
				break;
			case 'e':
				transform_enlarge = !transform_enlarge;
				retransform = 1;
				break;
			case 'k':
				transform_cal = !transform_cal;
				retransform = 1;
				break;
			case 'i':
				transform_iaspect = !transform_iaspect;
				retransform = 1;
				break;
			case 'p':
				transform_cal = 0;
				transform_iaspect = 0;
				transform_enlarge = 0;
				transform_stretch = 0;
				retransform = 1;
				break;
			case 'n':
				transform_rotation -= 1;
				if(transform_rotation < 0)
					transform_rotation += 4;
				retransform = 1;
				break;
			case 'm':
				transform_rotation += 1;
				if(transform_rotation > 3)
					transform_rotation -= 4;
				retransform = 1;
				break;

		}

	}

done:
	if(opt_clear)
	{
		printf("\033[H\033[J");
		fflush(stdout);
	}

error_mem:
	free(image);
	free(alpha);
	if(i.do_free)
	{
		free(i.rgb);
		free(i.alpha);
	}
	return(ret);

}

void help(char *name)
{
	printf("Usage: %s [options] image1 image2 image3 ...\n\n"
		   "Available options:\n"
		   " --help        | -h : Show this help\n"
		   " --alpha       | -a : Use the alpha channel (if applicable)\n"
		   " --dontclear   | -c : Do not clear the screen before and after displaying the image\n"
		   " --donthide    | -u : Do not hide the cursor before and after displaying the image\n"
		   " --noinfo      | -i : Supress image information\n"
		   " --stretch     | -f : Strech (using a simple resizing routine) the image to fit onto screen if necessary\n"
		   " --colorstretch| -k : Strech (using a 'color average' resizing routine) the image to fit onto screen if necessary\n"
		   " --enlarge     | -e : Enlarge the image to fit the whole screen if necessary\n"
		   " --ignore-aspect| -r : Ignore the image aspect while resizing\n"
                   " --delay <d>   | -s <delay> : Slideshow, 'delay' is the slideshow delay in tenths of seconds.\n\n"
		   "Keys:\n"
		   " r            : Redraw the image\n"
		   " a, d, w, x   : Pan the image\n"
		   " f            : Toggle resizing on/off\n"
		   " k            : Toggle resizing quality\n"
		   " e            : Toggle enlarging on/off\n"
		   " i            : Toggle respecting the image aspect on/off\n"
		   " n            : Rotate the image 90 degrees left\n"
		   " m            : Rotate the image 90 degrees right\n"
		   " p            : Disable all transformations\n"
		   "Copyright (C) 2000 - 2004 Mateusz Golicz, Tomasz Sterna.\n", name);
}

void sighandler(int s)
{
	if(opt_hide_cursor)
	{
		printf("\033[?25h");
		fflush(stdout);
	}
	setup_console(0);
	_exit(128 + s);

}

int main(int argc, char **argv)
{
	static struct option long_options[] =
	{
		{"help",	no_argument,	0, 'h'},
		{"noclear", 	no_argument, 	0, 'c'},
		{"alpha", 	no_argument, 	0, 'a'},
		{"unhide",  	no_argument, 	0, 'u'},
		{"noinfo",  	no_argument, 	0, 'i'},
		{"stretch", 	no_argument, 	0, 'f'},
		{"colorstrech", no_argument, 	0, 'k'},
		{"delay", 	required_argument, 0, 's'},
		{"enlarge",	no_argument,	0, 'e'},
		{"ignore-aspect", no_argument,	0, 'r'},
		{0, 0, 0, 0}
	};
	int c, i;

	if(argc < 2)
	{
		help(argv[0]);
		fprintf(stderr, "Error: Required argument missing.\n");
		return(1);
	}

	while((c = getopt_long_only(argc, argv, "hcauifks:er", long_options, NULL)) != EOF)
	{
		switch(c)
		{
			case 'a':
				opt_alpha = 1;
				break;
			case 'c':
				opt_clear = 0;
				break;
			case 's':
				opt_delay = atoi(optarg);
				break;
			case 'u':
				opt_hide_cursor = 0;
				break;
			case 'h':
				help(argv[0]);
				return(0);
			case 'i':
				opt_image_info = 0;
				break;
			case 'f':
				opt_stretch = 1;
				break;
			case 'k':
				opt_stretch = 2;
				break;
			case 'e':
				opt_enlarge = 1;
				break;
			case 'r':
				opt_ignore_aspect = 1;
				break;
		}
	}


	if(!argv[optind])
	{
		fprintf(stderr, "Required argument missing! Consult %s -h.\n", argv[0]);
		return(1);
	}

	signal(SIGHUP, sighandler);
	signal(SIGINT, sighandler);
	signal(SIGQUIT, sighandler);
	signal(SIGSEGV, sighandler);
	signal(SIGTERM, sighandler);
	signal(SIGABRT, sighandler);

	// if(opt_hide_cursor)
	// {
	// 	printf("\033[?25l");
	// 	fflush(stdout);
	// }

	// setup_console(1);

	for(i = optind; argv[i]; )
	{
		int r = show_image(argv[i]);

		if(!r) break;

		i += r;
		if(i < optind)
			i = optind;
	}

	// setup_console(0);

	// if(opt_hide_cursor)
	// {
	// 	printf("\033[?25h");
	// 	fflush(stdout);
	// }
	return(0);
}
