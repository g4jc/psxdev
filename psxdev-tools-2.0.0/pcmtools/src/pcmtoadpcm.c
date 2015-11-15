/* $Id: pcmtoadpcm.c,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $ */

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
"%s - PCM to ADPCM packer\n"
"Copyright (C) 1997, 1998, 1999, 2000 by Daniel Balster <dbalster@psxdev.de>\n"
"based on VAG-Packer, hacked by bITmASTER@bigfoot.com\n"
"All Rights Reserved.\n"
"\n");

#define printf1(fmt,args...)	if (1<=verbose_level) fprintf(stderr,fmt, ## args)
#define printf2(fmt,args...)	if (2<=verbose_level) fprintf(stderr,fmt, ## args)
int verbose_level = 1;

static double f[5][2] = {
	{ 0.0, 0.0 },
	{  -60.0 / 64.0, 0.0 },
	{ -115.0 / 64.0, 52.0 / 64.0 },
	{  -98.0 / 64.0, 55.0 / 64.0 },
	{ -122.0 / 64.0, 60.0 / 64.0 }
};

void find_predict( short *samples, double *d_samples, int *predict_nr, int *shift_factor )
{
	int i, j;
	double buffer[28][5];
	double min = 1e10;
	double max[5];
	double ds;
	int min2;
	int shift_mask;
	static double _s_1 = 0.0;
	static double _s_2 = 0.0;
	double s_0, s_1, s_2;

	for ( i = 0; i < 5; i++ )
	{
		max[i] = 0.0;
		s_1 = _s_1;
		s_2 = _s_2;
		for ( j = 0; j < 28; j ++ )
		{
			s_0 = (double) samples[j];
			if ( s_0 > 30719.0 ) s_0 = 30719.0;
            if ( s_0 < - 30720.0 ) s_0 = -30720.0;
			ds = s_0 + s_1 * f[i][0] + s_2 * f[i][1];
			buffer[j][i] = ds;
			if ( fabs( ds ) > max[i] ) max[i] = fabs( ds );
			s_2 = s_1;
			s_1 = s_0;
        }

		if ( max[i] < min )
		{
			min = max[i];
			*predict_nr = i;
		}
		if ( min <= 7 )
		{
			*predict_nr = 0;
			break;
		}
    }

	_s_1 = s_1;
	_s_2 = s_2;
    
	for ( i = 0; i < 28; i++ ) d_samples[i] = buffer[i][*predict_nr];

//  if ( min > 32767.0 )
//      min = 32767.0;
        
	min2 = ( int ) min;
	shift_mask = 0x4000;
	*shift_factor = 0;
    
	while( *shift_factor < 12 )
	{
		if ( shift_mask  & ( min2 + ( shift_mask >> 3 ) ) ) break;
		(*shift_factor)++;
		shift_mask = shift_mask >> 1;
	}
}

void pack( double *d_samples, short *four_bit, int predict_nr, int shift_factor )
{
    double ds;
    int di;
    double s_0;
    static double s_1 = 0.0;
    static double s_2 = 0.0;
    int i;

    for ( i = 0; i < 28; i++ ) {
        s_0 = d_samples[i] + s_1 * f[predict_nr][0] + s_2 * f[predict_nr][1];
        ds = s_0 * (double) ( 1 << shift_factor );

        di = ( (int) ds + 0x800 ) & 0xfffff000;

        if ( di > 32767 )
            di = 32767;
        if ( di < -32768 )
            di = -32768;
            
        four_bit[i] = (short) di;

        di = di >> shift_factor;
        s_2 = s_1;
        s_1 = (double) di - s_0;

    }
}

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
		#define BUFFER_SIZE 128*28
		int sample_len;
		short wave[BUFFER_SIZE];

		if (outname == NULL) fout = stdout;
		else fout = fopen (outname,"rb");
		if (fout != NULL)
		{
    		short *ptr;
    		double d_samples[28];
    		short four_bit[28];
    		int predict_nr;
    		int shift_factor;
    		int flags;
    		int size;
    		int i, j, k;    
    		unsigned char d;

    		flags = 0;
			size = 0;

			while (1)
			{
				size = fread (wave,2,BUFFER_SIZE,fin);
				if (size == 0) break;
				sample_len = size;

        		i = size / 28;
        		if ( size % 28 )
				{
            		for ( j = size % 28; j < 28; j++ )
                		wave[28*i+j] = 0;
            		i++;
        		}

        		for ( j = 0; j < i; j++ )
				{
            		ptr = wave + j * 28;
            		find_predict( ptr, d_samples, &predict_nr, &shift_factor );
            		pack( d_samples, four_bit, predict_nr, shift_factor );
            		d = ( predict_nr << 4 ) | shift_factor;
            		fputc( d, fout );
            		fputc( flags, fout );
            		for ( k = 0; k < 28; k += 2 ) {
                		d = ( ( four_bit[k+1] >> 8 ) & 0xf0 ) | ( ( four_bit[k] >> 12 ) & 0xf );
                		fputc( d, fout );
            		}
            		sample_len -= 28;
            		if ( sample_len < 28 )
                		flags = 1;
        		}
    		}

    		fputc( ( predict_nr << 4 ) | shift_factor, fout );
    		fputc( 7, fout );
    		for ( i = 0; i < 14; i++ ) fputc( 0, fout );

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
