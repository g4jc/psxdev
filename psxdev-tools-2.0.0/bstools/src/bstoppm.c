/* $Id: bstoppm.c,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $ */

/*
	bstools - various tools to handle bitstream images

	Copyright (C) 1999, 2000 by these people, who contributed to this project

	  Daniel Balster <dbalster@psxdev.de>

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
#include <bs.h>

/* GNU long-options */

enum {
	OPTION_QUIET	= 'q' ,
	OPTION_HELP		= 'h' ,
	OPTION_VERBOSE	= 'v' ,
	OPTION_VERSION	= 'V' ,
	OPTION_OUTPUT	= 'o' ,
	OPTION_WIDTH	= 'W' ,
	OPTION_HEIGHT	= 'H' ,
};

static struct option long_options[] =
{
	{ "quiet" ,no_argument, NULL, OPTION_QUIET },
	{ "help" ,no_argument, NULL, OPTION_HELP },
	{ "verbose" ,no_argument, NULL, OPTION_VERBOSE },
	{ "version" ,no_argument, NULL, OPTION_VERSION },
	{ "output", required_argument, NULL, OPTION_OUTPUT },
	{ "width", required_argument, NULL, OPTION_WIDTH },
	{ "height", required_argument, NULL, OPTION_HEIGHT },
	{ NULL }
};

static char *option_help[] = {
	N_("be quiet"),
	N_("print this text"),
	N_("be verbose"),
	N_("print binary version"),
	N_("set output file"),
	N_("width of input image (required!)"),
	N_("height of input image (required!)"),
};

static char *version_text =
"1.0.0"
;

static char *copyright_text = N_(
"%s - BS to PPM image converter\n"
"Copyright (C) 2000 by Daniel Balster <dbalster@psxdev.de>\n"
"All Rights Reserved.\n"
"\n");

#define printf1(fmt,args...)	if (1<=verbose_level) fprintf(stderr,fmt, ## args)
#define printf2(fmt,args...)	if (2<=verbose_level) fprintf(stderr,fmt, ## args)
int verbose_level = 1;

int main (int argc, char **argv)
{
	volatile int opt, i,j;
	char *filename = NULL;
	char *outname = NULL;
	char option_chars [BUFSIZ];
	
	int width = -1;
	int height = -1;
	FILE *file = 0;
	struct stat st;
	FILE *fp = stdout;
	int rc;

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

			case OPTION_OUTPUT:
				outname = optarg;
				break;

			case OPTION_WIDTH:
				width = strtoul(optarg,0,0);
				break;

			case OPTION_HEIGHT:
				height = strtoul(optarg,0,0);
				break;

			default:
				exit(1);
		}
	}

	if (optind < argc) filename = argv[optind];

	if (getenv("PSXDEV_QUIET") && (verbose_level==1)) verbose_level = 0;

	bs_init();

	if (filename == NULL)
	{
		st.st_size = 0;
	
		file = tmpfile();
		
		while (!feof(stdin))
		{
			int c = fgetc (stdin);
			if (c != -1) fputc (c, file);
			st.st_size++;
		}
		
		rewind(file);
	}
	else
	{
		if (stat(filename,&st)==0)
		{
			file = fopen(filename,"rb");
		}
		else ioerrorf("stat(%s)",filename);
	}

	if (width!=-1 && height!=-1)
	{
		bs_header_t *image;
		unsigned char *outbuf = malloc (width*height*3);
		
		image = malloc (st.st_size);
		rc = fread (image,st.st_size,1,file);
	//	if (rc != 1) ioerrorf("fread");
	
		bs_decode_rgb24 (outbuf,image,width,height,0);

		if (outname) freopen(outname,"wb",fp);
		fprintf (fp,"P6\n%d %d\n255\n",width,height);
		fwrite(outbuf,1,width*height*3,fp);
		fflush (fp);
		if (outname) fclose (fp);
		
		free (outbuf);
	}
	else errorf(_("you must specify both, witdh and height!"));

	return EXIT_SUCCESS;
}

/*
** Local variables:
** c-indent-level: 4
** tab-width: 4
*/
