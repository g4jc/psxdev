/* $Id: timup.c,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $ */

/*
	psxtools - communication and data convert tools for the unix shell

	Copyright (C) 1997, 1998, 1999, 2000 by these people, who contributed to this project

	  Daniel Balster <dbalster@psxdev.de>
	  Sergio Moreira <sergio@x-plorer.co.uk>
	  Andrew Kieschnick <andrewk@cerc.utexas.edu>
	  Kazuki Sakamoto <bsd-ps@geocities.co.jp>

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

#include "common.h"
#include <psxdev.h>
#include <pccl.h>
#include <tim.h>

/* GNU long-options */

enum {
	OPTION_QUIET	= 'q' ,
	OPTION_HELP		= 'h' ,
	OPTION_VERBOSE	= 'v' ,
	OPTION_VERSION	= 'V' ,
	OPTION_DEVICE	= 'd' ,
	OPTION_LEFT		= 'X' ,
	OPTION_TOP		= 'Y' ,
	OPTION_CLEFT	= 'x' ,
	OPTION_CTOP		= 'y' ,
};

static struct option long_options[] =
{
	{ "quiet" ,no_argument, NULL, OPTION_QUIET },
	{ "help" ,no_argument, NULL, OPTION_HELP },
	{ "verbose" ,no_argument, NULL, OPTION_VERBOSE },
	{ "version" ,no_argument, NULL, OPTION_VERSION },
	{ "device" ,required_argument, NULL, OPTION_DEVICE },
	{ "left", required_argument, NULL, OPTION_LEFT },
	{ "top", required_argument, NULL, OPTION_TOP },
	{ "clutx", required_argument, NULL, OPTION_CLEFT },
	{ "cluty", required_argument, NULL, OPTION_CTOP },
	{ NULL }
};

static char *option_help[] = {
	N_("be quiet"),
	N_("print this text"),
	N_("be verbose"),
	N_("print binary version"),
	N_("use this device (default: /dev/pccl)"),
	N_("left edge of pixel (default: TIM)"),
	N_("top edge of pixel (default: TIM)"),
	N_("left edge of clut (default: TIM)"),
	N_("top edge of clut (default: TIM)"),
};

static char *version_text =
"1.0.0"
;

static char *copyright_text = N_(
"%s - upload a TIM image to the framebuffer\n"
"Copyright (C) 1997, 1998, 1999, 2000 by Daniel Balster <dbalster@psxdev.de>\n"
"All Rights Reserved.\n"
"\n");

#define printf1(fmt,args...)	if (1<=verbose_level) fprintf(stderr,fmt, ## args)
#define printf2(fmt,args...)	if (2<=verbose_level) fprintf(stderr,fmt, ## args)
int verbose_level = 1;

void on_exit_handler (int exitcode, void* pccl)
{
	ioctl ((int)pccl,CAEIOC_RESUME);
	close ((int)pccl);
}

int main (int argc, char **argv)
{
	volatile int opt, i,j;
	char *device = "/dev/pccl";
	char option_chars [BUFSIZ];
	char *filename = NULL;

	int rc, fd;
	int px, py, cx, cy;
	
	px = py = cx = cy = -1;

#if ENABLE_NLS
#if HAVE_SETLOCALE
   setlocale (LC_ALL, "");
#endif
   bindtextdomain (PACKAGE, LOCALEDIR);
   bindtextdomain (program_invocation_short_name, LOCALEDIR);
   textdomain (PACKAGE);
#endif

	/*
	**	build the option-char string from the option struct.
	*/

	for (i=j=0; long_options[i].name; i++)
	{
		switch (long_options[i].has_arg)
		{
			case no_argument:
				option_chars[j++] = (char) long_options[i].val;
				break;
			case required_argument:
				option_chars[j++] = (char) long_options[i].val;
				option_chars[j++] = ':';
				break;
		}
	}
	option_chars[j] = 0;

	while ((opt = getopt_long(argc,argv,option_chars,long_options,NULL)) != EOF)
	{
		switch (opt)
		{
			case OPTION_HELP:
				printf1 (copyright_text,program_invocation_short_name);
				printf1 (_("Usage: %s [options] <filename>\n"),program_invocation_short_name);
				for (i=0; long_options[i].name; i++)
				{
					switch (long_options[i].has_arg)
					{
						case no_argument:
							printf1 ("--%s,-%c \t%s\n",long_options[i].name,long_options[i].val,_(option_help[i]));
							break;
						case required_argument:
							printf1 ("--%s=x,-%c\t%s\n",long_options[i].name,long_options[i].val,_(option_help[i]));
							break;
					}
				}
				exit (EXIT_SUCCESS);

			case OPTION_VERSION:
				printf1 (copyright_text,program_invocation_short_name);
				printf1 ("%s version %s\n",program_invocation_short_name,version_text);
				exit (EXIT_SUCCESS);

			case OPTION_QUIET:
				verbose_level = 0;
				break;

			case OPTION_VERBOSE:
				verbose_level = 2;
				break;

			case OPTION_DEVICE:
				device = optarg;
				break;

			case OPTION_LEFT:
				px = strtoul (optarg,NULL,0);
				break;

			case OPTION_TOP:
				py = strtoul (optarg,NULL,0);
				break;

			case OPTION_CLEFT:
				cx = strtoul (optarg,NULL,0);
				break;

			case OPTION_CTOP:
				cy = strtoul (optarg,NULL,0);
				break;

			default:
				exit(1);
		}
	}

	if (optind < argc) filename = argv[optind];

	if (getenv("PSXDEV_QUIET") && (verbose_level==1)) verbose_level = 0;

	if (filename == NULL) errorf (_("input filename missing"));

	psxdev_init();

	errno = 0;
	fd = open (filename,O_RDONLY);
	if (fd != -1)
	{
		int status;
		struct stat st;
	
		errno = 0;
		status = fstat (fd,&st);
		if (status != -1)
		{
			void *bytes;
			
			if (st.st_size == 0) errorf(_("input file has zero length"));

			errno = 0;
			bytes = mmap (NULL,st.st_size,PROT_READ,MAP_PRIVATE,fd,0);
			if (((int)bytes) != -1)
			{
				int pccl = open(device,O_RDWR);
				if (pccl != -1)
				{
					caetla_fb_mode_t fbm;
					tim_t *tim = (tim_t*) bytes;
					tim_chunk_t *tc;
					
					on_exit (on_exit_handler,(void*)pccl);

					if (tim->magic != TIM_MAGIC) errorf(_("not a TIM image!"));

					tc = tim_clut_addr(tim);
					if (tc != 0)
					{
						fbm.x = (cx == -1) ? tc->x : cx;
						fbm.y = (cy == -1) ? tc->y : cy;
						fbm.width = tc->width;
						fbm.height = tc->height;

						rc = ioctl (pccl,CAEIOC_FB_WRITE_MEMORY,&fbm);
						if (rc == -1) ioerrorf("ioctl(CAEIOC_FB_WRITE_MEMORY)");

						rc = write (pccl,tim_clut(tim,0,0),tc->size-sizeof(tim_chunk_t));
						if (rc < 0) ioerrorf("write()");
					}

					tc = tim_pixel_addr(tim);
					if (tc != 0)
					{
						fbm.x = (px == -1) ? tc->x : px;
						fbm.y = (py == -1) ? tc->y : py;
						fbm.width = tc->width;
						fbm.height = tc->height;

						rc = ioctl (pccl,CAEIOC_FB_WRITE_MEMORY,&fbm);
						if (rc == -1) ioerrorf("ioctl(CAEIOC_FB_WRITE_MEMORY)");

						rc = write (pccl,tim_pixel(tim,0,0),tc->size-sizeof(tim_chunk_t));
						if (rc < 0) ioerrorf("write()");
					}

					rc = ioctl (pccl,CAEIOC_RESUME);
					if (rc == -1) ioerrorf("ioctl(CAEIOC_RESUME)");

					close (pccl);
				}
				else ioerrorf ("open(%s)",device);

				munmap (bytes,st.st_size);
			}
				else ioerrorf ("mmap()");
		}
		else ioerrorf ("fstat()");

		close (fd);
	}
	else ioerrorf ("open(%s)",filename);

	return EXIT_SUCCESS;
}

/*
** Local variables:
** c-indent-level: 4
** tab-width: 4
*/
