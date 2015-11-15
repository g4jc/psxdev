/* $Id: fbmode.c,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $ */

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
	OPTION_WIDTH	= 'W' ,
	OPTION_HEIGHT	= 'H' ,
	OPTION_MODE		= 'M' ,
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
	{ "width", required_argument, NULL, OPTION_WIDTH },
	{ "height", required_argument, NULL, OPTION_HEIGHT },
	{ "mode", required_argument, NULL, OPTION_MODE },
	{ NULL }
};

static char *option_help[] = {
	N_("be quiet"),
	N_("print this text"),
	N_("be verbose"),
	N_("print binary version"),
	N_("use this device (default: /dev/pccl)"),
	N_("left edge of image area (default: 0)"),
	N_("top edge of image area (default: 0)"),
	N_("width of image area (default: 256)"),
	N_("height of image area (default: 240)"),
	N_("pixel mode (0:15-bit, 1:24-bit, default:0)"),
};

static char *version_text =
"1.0.0"
;

static char *copyright_text = N_(
"%s - set PSX frame buffer mode\n"
"Copyright (C) 1997, 1998, 1999, 2000 by Daniel Balster <dbalster@psxdev.de>\n"
"All Rights Reserved.\n"
"\n");

#define printf1(fmt,args...)	if (1<=verbose_level) printf(fmt, ## args)
#define printf2(fmt,args...)	if (2<=verbose_level) printf(fmt, ## args)
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

	int pccl;
	int x,y,w,h,m;
	
	x = y = 0;
	w = 256;
	h = 240;
	m = 0;

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
				x = strtoul (optarg,NULL,0);
				break;

			case OPTION_TOP:
				y = strtoul (optarg,NULL,0);
				break;

			case OPTION_WIDTH:
				w = strtoul (optarg,NULL,0);
				break;

			case OPTION_HEIGHT:
				h = strtoul (optarg,NULL,0);
				break;

			case OPTION_MODE:
				m = strtoul (optarg,NULL,0);
				break;

			default:
				exit(1);
		}
	}

	if (getenv("PSXDEV_QUIET") && (verbose_level==1)) verbose_level = 0;


	psxdev_init();

	pccl = open(device,O_RDWR);
	if (pccl != -1)
	{
		caetla_fb_mode_t fbm;
		int rc;
		
		fbm.x = x;
		fbm.y = y;
		fbm.width = w;
		fbm.height = h;
		fbm.mode = m;

		on_exit (on_exit_handler,(void*)pccl);

		rc = ioctl (pccl,CAEIOC_FB_SET_MODE,&fbm);
		if (rc == -1) ioerrorf("ioctl(CAEIOC_FB_SET_MODE)");

		rc = ioctl (pccl,CAEIOC_RESUME);
		if (rc == -1) ioerrorf("ioctl(CAEIOC_RESUME)");

		close (pccl);
	}
	else ioerrorf ("open(%s)",device);

	return EXIT_SUCCESS;
}

/*
** Local variables:
** c-indent-level: 4
** tab-width: 4
*/
