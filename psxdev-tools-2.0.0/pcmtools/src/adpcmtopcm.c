/* $Id: adpcmtopcm.c,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $ */

/*
	pcmtools - various tools to handle audio data

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

/*
	Credit notice: this converter is based on VAGPACK.C:
	"PSX VAG-Packer, hacked by bITmASTER@bigfoot.com"
*/

#include "common.h"
#include <math.h>

/* GNU long-options */

enum {
	OPTION_QUIET	= 'q' ,
	OPTION_HELP		= 'h' ,
	OPTION_VERBOSE	= 'v' ,
	OPTION_VERSION	= 'V' ,
	OPTION_OUTPUT	= 'o' ,
};

static struct option long_options[] =
{
	{ "quiet" ,no_argument, NULL, OPTION_QUIET },
	{ "help" ,no_argument, NULL, OPTION_HELP },
	{ "verbose" ,no_argument, NULL, OPTION_VERBOSE },
	{ "version" ,no_argument, NULL, OPTION_VERSION },
	{ "output", required_argument, NULL, OPTION_OUTPUT },
	{ NULL }
};

static char *option_help[] = {
	N_("be quiet"),
	N_("print this text"),
	N_("be verbose"),
	N_("print binary version"),
	N_("set output file"),
};

static char *version_text =
"1.0.0"
;

static char *copyright_text = N_(
"%s - ADPCM to PCM packer\n"
"Copyright (C) 1997, 1998, 1999, 2000 by Daniel Balster <dbalster@psxdev.de>\n"
"based on VAG-Depacker, hacked by bITmASTER@bigfoot.com\n"
"All Rights Reserved.\n"
"\n");

#define printf1(fmt,args...)	if (1<=verbose_level) fprintf(stderr,fmt, ## args)
#define printf2(fmt,args...)	if (2<=verbose_level) fprintf(stderr,fmt, ## args)
int verbose_level = 1;

static double f[5][2] = {
	{ 0.0, 0.0 },
	{   60.0 / 64.0,  0.0 },
	{  115.0 / 64.0, -52.0 / 64.0 },
	{   98.0 / 64.0, -55.0 / 64.0 },
	{  122.0 / 64.0, -60.0 / 64.0 }
};

double samples[28];

int main (int argc, char **argv)
{
	volatile int opt, i,j;
	char option_chars [BUFSIZ];
	char *filename = NULL;
	char *outname = NULL;
	FILE *fin, *fout;

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

			default:
				exit(1);
		}
	}

	if (optind < argc) filename = argv[optind++];

	if (getenv("PSXDEV_QUIET") && (verbose_level==1)) verbose_level = 0;

	if (filename == NULL) fin = stdin;
	else fin = fopen (filename,"rb");
	if (fin != NULL)
	{
		int predict_nr, shift_factor, flags;
		int d, s;
		static double s_1 = 0.0;
		static double s_2 = 0.0;

		if (outname == NULL) fout = stdout;
		else fout = fopen (outname,"rb");
		if (fout != NULL)
		{
    		while( 1 )
			{
        		predict_nr = fgetc( fin );
        		shift_factor = predict_nr & 0xf;
        		predict_nr >>= 4;
        		flags = fgetc( fin );
        		if ( flags == 7 )
            		break;              
        		for ( i = 0; i < 28; i += 2 )
				{
            		d = fgetc( fin );
            		s = ( d & 0xf ) << 12;
            		if ( s & 0x8000 )
                		s |= 0xffff0000;
            		samples[i] = (double) ( s >> shift_factor  );
            		s = ( d & 0xf0 ) << 8;
            		if ( s & 0x8000 )
                		s |= 0xffff0000;
            		samples[i+1] = (double) ( s >> shift_factor  );
        		}
        		for ( i = 0; i < 28; i++ )
				{
            		samples[i] = samples[i] + s_1 * f[predict_nr][0] + s_2 * f[predict_nr][1];
            		s_2 = s_1;
            		s_1 = samples[i];
            		d = (int) ( samples[i] + 0.5 );
            		fputc( d & 0xff, fout );
            		fputc( d >> 8, fout );
        		}
    		}
		if (filename != NULL) fclose (fout);
		}
		else ioerrorf ("fopen()");
	if (filename != NULL) fclose (fin);
	}
	else ioerrorf ("fopen()");

	return EXIT_SUCCESS;
}

/*
** Local variables:
** c-indent-level: 4
** tab-width: 4
*/
