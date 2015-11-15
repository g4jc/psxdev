/* $Id: timmontage.c,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $ */

/*
	timtools - various tools to handle TIM images

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
#include <tim.h>

/* GNU long-options */

enum {
	OPTION_QUIET	= 'q' ,
	OPTION_HELP		= 'h' ,
	OPTION_VERBOSE	= 'v' ,
	OPTION_VERSION	= 'V' ,
	OPTION_WIDTH	= 'W' ,
	OPTION_HEIGHT	= 'H' ,
	OPTION_MODE		= 'm' ,
	OPTION_OUTPUT	= 'o' ,
	OPTION_NCLUTS	= 'n' ,
};

static struct option long_options[] =
{
	{ "quiet" ,no_argument, NULL, OPTION_QUIET },
	{ "help" ,no_argument, NULL, OPTION_HELP },
	{ "verbose" ,no_argument, NULL, OPTION_VERBOSE },
	{ "version" ,no_argument, NULL, OPTION_VERSION },
	{ "width", required_argument, NULL, OPTION_WIDTH },
	{ "height", required_argument, NULL, OPTION_HEIGHT },
	{ "mode", required_argument, NULL, OPTION_MODE },
	{ "output", required_argument, NULL, OPTION_OUTPUT },
	{ "numcluts", required_argument, NULL, OPTION_NCLUTS },
	{ NULL }
};

static char *option_help[] = {
	N_("be quiet"),
	N_("print this text"),
	N_("be verbose"),
	N_("print binary version"),
	N_("width"),
	N_("height"),
	N_("mode (0..3)"),
	N_("output"),
	N_("number of cluts"),
};

static char *version_text =
"1.0.0"
;

static char *copyright_text = N_(
"%s - montage of multiple TIM images to one new image\n"
"Copyright (C) 1997, 1998, 1999, 2000 by Daniel Balster <dbalster@psxdev.de>\n"
"All Rights Reserved.\n"
"\n");

#define printf1(fmt,args...)	if (1<=verbose_level) fprintf(stderr,fmt, ## args)
#define printf2(fmt,args...)	if (2<=verbose_level) fprintf(stderr,fmt, ## args)
int verbose_level = 1;

int main (int argc, char **argv)
{
	volatile int opt, i,j;
	char *filename = NULL;
	char *outname  = NULL;
	char option_chars [BUFSIZ];

	int rc, fd;
	struct stat st;
	tim_t *tim, *xtim;
	
	int width, height, mode,ncluts;

	width = 256;
	height = 256;
	mode = 0;
	ncluts = 1;

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

			case OPTION_WIDTH:
				width = strtoul(optarg,NULL,0);
				break;

			case OPTION_HEIGHT:
				height = strtoul(optarg,NULL,0);
				break;

			case OPTION_MODE:
				mode = strtoul(optarg,NULL,0);
				break;

			case OPTION_OUTPUT:
				outname = optarg;
				break;

			case OPTION_NCLUTS:
				ncluts = strtoul(optarg,NULL,0);
				break;

			default:
				exit(1);
		}
	}

	if (getenv("PSXDEV_QUIET") && (verbose_level==1)) verbose_level = 0;

	if (outname == NULL) outname = "/dev/stdout";

	xtim = tim_new (mode,width,height,ncluts);
	if (xtim != NULL)
	{
		for (i=0;i<ncluts;i++)
		{
			filename = NULL;
			if (optind < argc) filename = argv[optind++];
			if (filename == NULL) errorf (_("input filename missing"));

			fd = open (filename,O_RDWR);
			if (fd != -1)
			{
				rc = fstat (fd,&st);
				if (rc != -1)
				{
					if (st.st_size == 0) errorf(_("input file has zero length"));

					tim = (tim_t*) mmap (NULL,st.st_size,PROT_WRITE|PROT_READ,MAP_SHARED,fd,0);
					if (1)
					{
						tim_chunk_t *chunk, *pchunk;
						u_short *psrc, *pdst;
						int x,y,k;

						pchunk = tim_clut_addr (xtim);
						chunk = tim_clut_addr (tim);
						if (chunk != NULL)
						{
							printf2 ("clut entries: %d\n",chunk->width);

							psrc = (u_short*) tim_clut (tim,0,0);
							pdst = (u_short*) tim_clut (xtim,0,0);

							for (x=0;x<chunk->width;x++)
							{
								pdst[(pchunk->width*i)+x] = psrc[x];
							}
						}

						pchunk = tim_pixel_addr (xtim);
						chunk = tim_pixel_addr (tim);
						if (chunk != NULL)
						{
							int X,Y,MD=0;
							printf2 ("data at: %d,%d (%d,%d)\n"
								,chunk->x,chunk->y,chunk->width,chunk->height);
						
							psrc = (u_short*) tim_pixel (tim,0,0);
							pdst = (u_short*) tim_pixel (xtim,0,0);
							k = 0;

							switch (mode)
							{
								case 0: MD = 4; break;
								case 1: MD = 2; break;
								case 2: MD = 1; break;
								
								/* 24-bit causes division by zero */
								
							}

							for (y=0;y<chunk->height;y++)
							{
								Y = y + (chunk->y);
								for (x=0;x<chunk->width;x++)
								{
									X = x + (chunk->x/MD);
									pdst[(pchunk->width*Y)+X] = psrc[k];

									k++;
								}
							}
						}

						munmap ((void*)tim,st.st_size);
					}
					else ioerrorf ("mmap()");
				}
				else ioerrorf ("fstat()");
				close (fd);
			}
			else ioerrorf ("open(%s)",filename);
		}
	
		{
			int f;

			f = open(outname,O_WRONLY|O_CREAT);
			if (f != -1)
			{
				write (f,xtim,tim_sizeof(xtim));
				close (f);
			}
		}

		tim_free (xtim);
	}


	return EXIT_SUCCESS;
}

/*
** Local variables:
** c-indent-level: 4
** tab-width: 4
*/
