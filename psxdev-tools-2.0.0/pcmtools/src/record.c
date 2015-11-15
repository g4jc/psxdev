/* $Id: record.c,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $ */

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
	OPTION_STEREO	= 'S' ,
	OPTION_MONO		= 'M' ,
	OPTION_SPEED	= 's' ,
	OPTION_FORMAT	= 'f' ,
	OPTION_LENGTH	= 'l' ,
	OPTION_TIME		= 't' ,
	OPTION_BUFSIZE	= 'b' ,
	OPTION_CHANNEL	= 'c' ,
	OPTION_WAVE		= 'w' ,
};

static struct option long_options[] =
{
	{ "quiet" ,no_argument, NULL, OPTION_QUIET },
	{ "help" ,no_argument, NULL, OPTION_HELP },
	{ "verbose" ,no_argument, NULL, OPTION_VERBOSE },
	{ "version" ,no_argument, NULL, OPTION_VERSION },
	{ "device" ,required_argument, NULL, OPTION_DEVICE },
	{ "stereo" ,no_argument, NULL, OPTION_STEREO },
	{ "mono" ,no_argument, NULL, OPTION_MONO },
	{ "speed" ,required_argument, NULL, OPTION_SPEED },
	{ "format" ,required_argument, NULL, OPTION_FORMAT },
	{ "length" ,required_argument, NULL, OPTION_LENGTH },
	{ "time" ,required_argument, NULL, OPTION_TIME },
	{ "buffer-size" ,required_argument, NULL, OPTION_BUFSIZE },
	{ "channels" ,required_argument, NULL, OPTION_CHANNEL },
	{ "wave" ,no_argument, NULL, OPTION_WAVE },
	{ NULL }
};

static char *option_help[] = {
	N_("be quiet"),
	N_("print this text"),
	N_("be verbose"),
	N_("print binary version"),
	N_("which audio device (default: /dev/dsp)"),
	N_("enable stereo"),
	N_("enable mono"),
	N_("set speed (default: 44100)"),
	N_("set audio format (default: system default)"),
	N_("set length (default: 0 bytes)"),
	N_("set time (default: 1 second)"),
	N_("buffer size (default: 4096 bytes)"),
	N_("set number of channels (default: 1)"),
	N_("use RIFF-WAVE file format (default: raw pcm)"),
};

typedef struct {
	unsigned long main_chunk;
	unsigned long length;
	unsigned long chunk_type;
	unsigned long sub_chunk;
	unsigned long sc_len;
	unsigned short format;
	unsigned short modus;
	unsigned long sample_fq;
	unsigned long byte_p_sec;
	unsigned short byte_p_spl;
	unsigned short bit_p_spl;
	unsigned long data_chunk;
	unsigned long data_length;
} wave_header_t;

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
"%s - record digital audio with OSS\n"
"Copyright (C) 1997, 1998, 1999, 2000 by Daniel Balster <dbalster@psxdev.de>\n"
"All Rights Reserved.\n"
"\n");

#define printf1(fmt,args...)	if (1<=verbose_level) fprintf(stderr,fmt, ## args)
#define printf2(fmt,args...)	if (2<=verbose_level) fprintf(stderr,fmt, ## args)
int verbose_level = 1;

int main (int argc, char **argv)
{
	volatile int opt, i,j;
	char option_chars [BUFSIZ];
	char *filename = NULL;

	char *device_name;
	int device;
	u_long  speed;
	u_long length;
	u_long seconds;
	u_long channels;
	int stereo;
	int format;
	int bufsize;
	void *buffer;
	char *format_string = 0;
	FILE *file = 0;
	int use_wave = 0;

	wave_header_t header;
	char *riff = "RIFF";
	char *wave = "WAVE";
	char *fmt  = "fmt ";
	char *data = "data";

#if ENABLE_NLS
#if HAVE_SETLOCALE
   setlocale (LC_ALL, "");
#endif
   bindtextdomain (PACKAGE, LOCALEDIR);
   bindtextdomain (program_invocation_short_name, LOCALEDIR);
   textdomain (PACKAGE);
#endif

	device_name = "/dev/dsp";
	stereo = 1;
	speed = 44100;
	length = 0;
	seconds = 1;
	format = AFMT_S16_LE;
	bufsize = 4096;
	channels = 1;

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

			case OPTION_STEREO:
				stereo = 1;
				break;

			case OPTION_MONO:
				stereo = 0;
				break;

			case OPTION_BUFSIZE:
				bufsize = strtoul (optarg,NULL,10);
				break;

			case OPTION_SPEED:
				speed = strtoul (optarg,NULL,10);
				break;

			case OPTION_CHANNEL:
				channels = strtoul (optarg,NULL,10);
				break;

			case OPTION_LENGTH:
				length = strtoul (optarg,NULL,10);
				break;

			case OPTION_FORMAT:
				for (j=0; format_options[j].name != NULL; j++)
				{
					if (strcmp(format_options[j].name,optarg) == 0)
						format = format_options[j].value;
				}
				if (format == 0) errorf (_("unsupported format \"%s\""),optarg);
				break;

			case OPTION_TIME:
				seconds = strtoul (optarg,NULL,10);
				break;

			case OPTION_WAVE:
				use_wave = 1;
				break;

			default:
				exit(1);
		}
	}

	if (optind < argc) filename = argv[optind++];

	if (getenv("PSXDEV_QUIET") && (verbose_level==1)) verbose_level = 0;

	for (j=0; format_options[j].name != NULL; j++)
	{
		if (format_options[j].value == format)
		{
			format_string = format_options[j].name;
		}
	}

	if (filename == NULL)
	{
		file = stdout;
	}

	device = open (device_name,O_RDONLY);
	if (device != -1)
	{
		int caps, rc, bits;

		rc = ioctl (device, SNDCTL_DSP_RESET,NULL);
		if (rc == -1) ioerrorf ("ioctl(SNDCTL_DSP_RESET)");

		rc = ioctl (device, SNDCTL_DSP_GETCAPS, &caps);
		if (rc == -1) ioerrorf ("ioctl(SNDCTL_DSP_GETCAPS)");

		buffer = malloc (bufsize);
		if (buffer == NULL) ioerrorf ("malloc(%d)",bufsize);

		/* set attributes */

		rc = ioctl (device, SNDCTL_DSP_CHANNELS, &channels);
		if (rc == -1) iowarnf ("ioctl(SNDCTL_DSP_CHANNELS)");

		rc = ioctl (device, SNDCTL_DSP_STEREO, &stereo);
		if (rc == -1) iowarnf ("ioctl(SNDCTL_DSP_STEREO)");

		rc = ioctl (device, SNDCTL_DSP_SPEED, &speed);
		if (rc == -1) iowarnf ("ioctl(SNDCTL_DSP_SPEED)");

		rc = ioctl (device, SNDCTL_DSP_SETFMT, &format);
		if (rc == -1) iowarnf ("ioctl(SNDCTL_DSP_SETFMT)");

		rc = ioctl (device, SOUND_PCM_READ_BITS, &bits);
		if (rc == -1) iowarnf ("ioctl(SOUND_PCM_READ_BITS)");

		printf2("recording file \"%s\" [%d Hz, %s, %d Bits, %s]\n", filename, (int) speed, stereo?"Stereo":"Mono", bits, format_string);

		if (length == 0)
		{
			length = seconds * speed * (stereo ? 2 : 1) * (bits/8);
		}

		if (filename != NULL) file = fopen (filename,"wb");

		if (file != NULL)
		{
			int n, k = length;

			// spit out RIFF-WAVE header

			if (use_wave==1)
			{
				memcpy(&(header.main_chunk), riff, 4);
				header.length = sizeof(wave_header_t) - 8 + length;
				memcpy(&(header.chunk_type), wave, 4);
				memcpy(&(header.sub_chunk), fmt, 4);
				header.sc_len = 16;
				header.format = 1;
				header.modus = stereo + 1;
				header.sample_fq = speed;
				header.byte_p_sec = ((bits > 8)? 2:1)*(stereo+1)*speed;
				header.byte_p_spl = (bits > 8)? 2:1;
				header.bit_p_spl = bits;
				memcpy(&(header.data_chunk), data, 4);
				header.data_length = length;
				rc = fwrite(&header, sizeof(wave_header_t),1,file);
				if (rc != 1) iowarnf ("write()");
			}

			while (k>0)
			{
				n = (k>bufsize) ? bufsize : k;

				rc = read (device,buffer,n);
				if (rc == -1) iowarnf ("read()");

				rc = fwrite (buffer,n,1,file);
				if (rc != 1) iowarnf ("write()");

				k -= n;
			}

			if (filename != NULL) fclose (file);
		}
		else iowarnf("open(%s)",filename);

		free (buffer);
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
