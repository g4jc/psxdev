/* $Id: flash.c,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $ */

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
#include "flasher.h"
#include <psxdev.h>
#include <pccl.h>

/* GNU long-options */

enum {
	OPTION_QUIET	= 'q' ,
	OPTION_HELP		= 'h' ,
	OPTION_VERBOSE	= 'v' ,
	OPTION_VERSION	= 'V' ,
	OPTION_DEVICE	= 'd' ,
	OPTION_EMERGENCY= 'e' ,
};

static struct option long_options[] =
{
	{ "quiet" ,no_argument, NULL, OPTION_QUIET },
	{ "help" ,no_argument, NULL, OPTION_HELP },
	{ "verbose" ,no_argument, NULL, OPTION_VERBOSE },
	{ "version" ,no_argument, NULL, OPTION_VERSION },
	{ "device" ,required_argument, NULL, OPTION_DEVICE },
	{ "emergency" ,no_argument, NULL, OPTION_EMERGENCY },
	{ NULL }
};

static char *option_help[] = {
	N_("be quiet"),
	N_("print this text"),
	N_("be verbose"),
	N_("print binary version"),
	N_("use this device (default: /dev/pccl)"),
	N_("emergency mode (you have to boot with a special CD-ROM)"),
};

static char *version_text =
"1.0.0"
;

static char *copyright_text = N_(
"%s - flash a PAR rom image\n"
"Copyright (C) 1999 by Andrew Kieschnick <andrewk@mail.utexas.edu>\n"
"Copyright (C) 1997, 1998, 1999, 2000 by Daniel Balster <dbalster@psxdev.de>\n"
"All Rights Reserved.\n"
"\n");

#define printf1(fmt,args...)	if (1<=verbose_level) printf(fmt, ## args)
#define printf2(fmt,args...)	if (2<=verbose_level) printf(fmt, ## args)
int verbose_level = 1;

int main (int argc, char **argv)
{
	volatile int opt, i,j;
	char *device = "/dev/pccl";
	char option_chars [BUFSIZ];
	char *filename = NULL;
	int emergency = 0;

	int fd;

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

			case OPTION_EMERGENCY:
				emergency = 1;
				break;

			case OPTION_DEVICE:
				device = optarg;
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
			int bytes;
			
			if (st.st_size == 0) errorf(_("input file has zero length"));

			errno = 0;
			bytes = (int) mmap (NULL,st.st_size,PROT_READ,MAP_PRIVATE,fd,0);
			if (bytes != -1)
			{
				int pccl = open(device,O_RDWR);
				if (pccl != -1)
				{
					unsigned char arg,x;
					unsigned int tmp;
					unsigned char *p;
					unsigned short chk,stmp;
					int i;

// emergency mode:
// we have booted with a CD-ROM, which already started the update program
			if (emergency==0)
			{ 
					printf(_("WARNING: FLASH UPDATE - Turn PSX off and hit CTRL-C to abort!\n"));
					printf(_("Press RESET on PSX to continue!\n"));

				  arg = 'W';
				  while (arg != 'R')
					{
					  arg = 'W';
					  ioctl(pccl,CAEIOC_SWAP_8, &arg);
					}
				  printf1(_("Sending update program.\n"));
				  arg = 'B';
				  while (arg != 'W')
					{
					  arg = 'B';
					  ioctl(pccl,CAEIOC_SWAP_8, &arg);
					}
				  arg = 'X';
				  ioctl(pccl,CAEIOC_SWAP_8, &arg);
				  while (arg != 'X')
					{
					  arg = 'X';
					  ioctl(pccl,CAEIOC_SWAP_8, &arg);
					}
				  
				  tmp = 0x80020000;
				  ioctl(pccl,CAEIOC_SWAP_32, &tmp);
				  tmp = sizeof(flasher_rom);
				  ioctl(pccl,CAEIOC_SWAP_32, &tmp);
				  
				  p = (unsigned char *)flasher_rom;
				  chk = 0;
				  for (i=0;i<sizeof(flasher_rom);i++)
					{
					  x = *p++;
					  arg = x;
					  ioctl(pccl,CAEIOC_SWAP_8, &arg);
					  if (arg != x) printf(_("Transfer error!\n"));
					  chk += x;
					}
				  stmp = chk & 0xfff;
				  ioctl(pccl,CAEIOC_SWAP_16, &stmp);
				  
				  arg = 0;
				  while (arg != 'O')
					{
					  arg = 0;
					  ioctl(pccl,CAEIOC_SWAP_8,&arg);
					}
				  arg = 'W';
				  while (arg != 'K')
					{
					  arg = 'K';
					  ioctl(pccl,CAEIOC_SWAP_8,&arg);
					}
			}
				  printf1(_("Sending ROM image.\n")); 
				  arg = 'W';
				  while (arg != 'R')
					{
					  arg = 'W';
					  ioctl(pccl,CAEIOC_SWAP_8,&arg);
					}
				  arg = 'B';
				  while (arg != 'W')
					{
					  arg = 'B';
					  ioctl(pccl,CAEIOC_SWAP_8,&arg);
					}
				  arg = 'U';
				  ioctl(pccl,CAEIOC_SWAP_8,&arg);
				  while (arg != 'U')
					{
					  arg = 'U';
					  ioctl(pccl,CAEIOC_SWAP_8,&arg);
					}
				  
				  tmp=0x80040000;
				  ioctl(pccl,CAEIOC_SWAP_32, &tmp);
				  ioctl(pccl,CAEIOC_SWAP_32, &st.st_size);
				  
				  p = (unsigned char *) bytes;
				  chk = 0;
				  for (i=0;i<st.st_size;i++)
					{
					  x = *p++;
					  arg = x;
					  ioctl(pccl,CAEIOC_SWAP_8, &arg);
					  if (arg != x) printf(_("Transfer error!\n"));
					  chk += x;
					}
				  stmp = chk & 0xfff;
				  ioctl(pccl,CAEIOC_SWAP_16, &stmp);
				  
				  arg = 0;
				  while (arg != 'O')
					{
					  arg = 0;
					  ioctl(pccl,CAEIOC_SWAP_8,&arg);
					}
				  arg = 'W';
				  while (arg != 'K')
					{
					  arg = 'K';
					  ioctl(pccl,CAEIOC_SWAP_8,&arg);
					}

				  printf1(_("Update complete.\n"));
				  

					close (pccl);
				}
				else ioerrorf ("open(%s)",device);

				munmap ((void*)bytes,st.st_size);
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
