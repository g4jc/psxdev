/* $Id: upload.c,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $ */

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

/* GNU long-options */

enum {
	OPTION_QUIET	= 'q' ,
	OPTION_HELP		= 'h' ,
	OPTION_VERBOSE	= 'v' ,
	OPTION_VERSION	= 'V' ,
	OPTION_DEVICE	= 'd' ,
	OPTION_ADDRESS	= 'a' ,
};

static struct option long_options[] =
{
	{ "quiet" ,no_argument, NULL, OPTION_QUIET },
	{ "help" ,no_argument, NULL, OPTION_HELP },
	{ "verbose" ,no_argument, NULL, OPTION_VERBOSE },
	{ "version" ,no_argument, NULL, OPTION_VERSION },
	{ "device" ,required_argument, NULL, OPTION_DEVICE },
	{ "address", required_argument, NULL, OPTION_ADDRESS },
	{ NULL }
};

static char *option_help[] = {
	N_("be quiet"),
	N_("print this text"),
	N_("be verbose"),
	N_("print binary version"),
	N_("use this device (default: /dev/pccl)"),
	N_("set address (default: 0x80010000)"),
};

static char *version_text =
"1.0.0"
;

static char *copyright_text = N_(
"%s - upload memory to the PSX\n"
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
	char *filename = NULL;

	int rc, fd;
	u_long address = 0x80010000;

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

			case OPTION_ADDRESS:
				address = strtoul (optarg,NULL,16);
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
					caetla_memory_t mem;
					caetla_device_info_t info;
					u_char checksum;
					
					mem.address = address;
					mem.size    = st.st_size;
				
					on_exit (on_exit_handler,(void*)pccl);

					rc = ioctl (pccl,CAEIOC_WRITE_MEMORY,&mem);
					if (rc == -1) ioerrorf("ioctl(CAEIOC_WRITE_MEMORY)");

					rc = write (pccl,bytes,st.st_size);
					if (rc < 0) ioerrorf("write()");
					
					rc = ioctl (pccl,CAEIOC_DEVICE_INFO,&info);
					if (rc == -1) ioerrorf("ioctl(CAEIOC_DEVICE_INFO)");
					
					checksum = info.checksum;
					rc = ioctl (pccl,CAEIOC_SWAP_8,&checksum);
					if (rc == -1) ioerrorf("ioctl(CAEIOC_SWAP_8)");

					if (checksum != info.checksum)
					{
						warnf (_("invalid checksum (%02x/%02x)"),checksum,info.checksum);
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
