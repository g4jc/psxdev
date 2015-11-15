/* $Id: query.c,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $ */

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

#include "common.h"
#include <psxdev.h>
#include <math.h>

#include <sys/soundcard.h>
#include <sys/file.h>

/* GNU long-options */

enum {
	OPTION_QUIET	= 'q' ,
	OPTION_HELP		= 'h' ,
	OPTION_VERBOSE	= 'v' ,
	OPTION_VERSION	= 'V' ,
	OPTION_DEVICE	= 'd' ,
};

static struct option long_options[] =
{
	{ "quiet" ,no_argument, NULL, OPTION_QUIET },
	{ "help" ,no_argument, NULL, OPTION_HELP },
	{ "verbose" ,no_argument, NULL, OPTION_VERBOSE },
	{ "version" ,no_argument, NULL, OPTION_VERSION },
	{ "device" ,required_argument, NULL, OPTION_DEVICE },
	{ NULL }
};

static char *option_help[] = {
	N_("be quiet"),
	N_("print this text"),
	N_("be verbose"),
	N_("print binary version"),
	N_("which audio device (default: /dev/dsp)"),
};

typedef struct {
	char *name;
	int value;
} format_options_t;

format_options_t format_options[] = {
	{ "MU_LAW", AFMT_MU_LAW },
	{ "A_LAW", AFMT_A_LAW },
	{ "IMA_ADPCM", AFMT_IMA_ADPCM },
	{ "U8", AFMT_U8 },
	{ "S16_LE", AFMT_S16_LE },
	{ "S16_BE", AFMT_S16_BE },
	{ "S8", AFMT_S8 },
	{ "U16_LE", AFMT_U16_LE },
	{ "U16_BE", AFMT_U16_BE },
	{ "MPEG", AFMT_MPEG },
	{ NULL }
};

static char *version_text =
"1.0.0"
;

static char *copyright_text = N_(
"%s - query capabilities of OSS compatible audio device\n"
"Copyright (C) 1997, 1998, 1999, 2000 by Daniel Balster <dbalster@psxdev.de>\n"
"All Rights Reserved.\n"
"\n");

#define printf1(fmt,args...)	if (1<=verbose_level) printf(fmt, ## args)
#define printf2(fmt,args...)	if (2<=verbose_level) printf(fmt, ## args)
int verbose_level = 1;

int main (int argc, char **argv)
{
	volatile int opt, i,j;
	char option_chars [BUFSIZ];

	char *device_name;
	int device;

#if ENABLE_NLS
#if HAVE_SETLOCALE
   setlocale (LC_ALL, "");
#endif
   bindtextdomain (PACKAGE, LOCALEDIR);
   bindtextdomain (program_invocation_short_name, LOCALEDIR);
   textdomain (PACKAGE);
#endif

	device_name = "/dev/dsp";

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
				device_name = optarg;
				break;

			default:
				exit(1);
		}
	}

	if (getenv("PSXDEV_QUIET") && (verbose_level==1)) verbose_level = 0;

	device = open (device_name,O_RDWR);
	if (device != -1)
	{
		int caps, rc, formats;

		rc = ioctl (device,SNDCTL_DSP_GETCAPS,&caps);
		if (rc == -1) ioerrorf ("ioctl(SNDCTL_DSP_GETCAPS)");

		printf1(_("%s has the following capabilities:\n"),device_name);
		if (caps & DSP_CAP_DUPLEX) printf1(_("- Full duplex record/playback\n"));
		if (caps & DSP_CAP_REALTIME) printf1(_("- Real time capability\n"));
		if (caps & DSP_CAP_BATCH) printf1(_("- Device has some kind of internal buffers which may cause some delays and decrease precision of timing\n"));
		if (caps & DSP_CAP_COPROC) printf1(_("- Has a coprocessor (Sometimes it's a DSP but usually not)\n"));
		if (caps & DSP_CAP_TRIGGER) printf1(_("- Supports SETTRIGGER\n"));
		if (caps & DSP_CAP_MMAP) printf1(_("- Supports mmap()\n"));

			
		rc = ioctl (device,SNDCTL_DSP_GETFMTS,&formats);
		if (rc == -1) ioerrorf ("ioctl(SNDCTL_DSP_GETFMTS)");

		printf1(_("%s supports 0x%08lx: "), device_name, formats);
		for (i=0; format_options[i].name != NULL; i++)
		{
			if (format_options[i].value & formats)
			{
				printf1("%s ",format_options[i].name);
			}
		}
		printf1("\n");

		close (device);
	}
	else ioerrorf ("open(%s)",device_name);

	return EXIT_SUCCESS;
}

/*
** Local variables:
** c-indent-level: 4
** tab-width: 4
*/
