/* $Id: rm.c,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $ */

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

#include <fnmatch.h>

/* GNU long-options */

enum {
	OPTION_QUIET	= 'q' ,
	OPTION_HELP		= 'h' ,
	OPTION_VERBOSE	= 'v' ,
	OPTION_VERSION	= 'V' ,
	OPTION_DEVICE	= 'd' ,
	OPTION_SLOT		= 's' ,
	OPTION_FORCE	= 'f' ,
};

static struct option long_options[] =
{
	{ "quiet" ,no_argument, NULL, OPTION_QUIET },
	{ "help" ,no_argument, NULL, OPTION_HELP },
	{ "verbose" ,no_argument, NULL, OPTION_VERBOSE },
	{ "version" ,no_argument, NULL, OPTION_VERSION },
	{ "device" ,required_argument, NULL, OPTION_DEVICE },
	{ "slot" ,required_argument, NULL, OPTION_SLOT },
	{ "force" ,no_argument, NULL, OPTION_FORCE },
	{ NULL }
};

static char *option_help[] = {
	N_("be quiet"),
	N_("print this text"),
	N_("be verbose"),
	N_("print binary version"),
	N_("use this device (default: /dev/pccl)"),
	N_("memory card slot (default: 0)"),
	N_("force operation (default: ask)"),
};

static char *version_text =
"1.0.0"
;

static char *copyright_text = N_(
"%s - delete a file from a memory card\n"
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
	
	int slot = 0;
	int force = 0;

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

			case OPTION_FORCE:
				force = 1;
				break;

			case OPTION_SLOT:
				slot = strtoul (optarg, 0, 16);
				break;

			default:
				exit(1);
		}
	}

	if (getenv("PSXDEV_QUIET") && (verbose_level==1)) verbose_level = 0;

	if (optind >= argc) return EXIT_SUCCESS;

	psxdev_init();

	{
		int pccl = open(device,O_RDWR);
		if (pccl != -1)
		{
			caetla_mc_result_t arg;
			caetla_mc_directory_t dir;
			int err, i;

			on_exit (on_exit_handler,(void*)pccl);

			err = ioctl(pccl,CAEIOC_CONTROL,CAETLA_MCM);
			if (err < 0) ioerrorf("ioctl(CAEIOC_CONTROL)");

			while (optind < argc)
			{
				int found = 1;
			
				filename = argv[optind++];
				
				while (found)
				{
					for (i=0; i<14; i++)
					{
						found = 0;
					
						arg.slot = slot;

						err = ioctl(pccl,CAEIOC_MC_RESCAN_FILES,&arg);
						if (err < 0) ioerrorf("ioctl(CAEIOC_RESCAN_FILES)");
						if (psxdev_mc_result_is_error(arg.result))
						{
							char buf[BUFSIZ];
							psxdev_mc_result_to_string (arg.result,buf,BUFSIZ);
							errorf (buf);
						}

						dir.slot = slot;
						err = ioctl(pccl,CAEIOC_MC_DIRECTORY, &dir);
						if (err < 0) ioerrorf("ioctl(CAEIOC_MC_DIRECTORY)");

						if (i >= dir.count) break;

						if (0==fnmatch(filename,dir.file[i].name,0))
						{
							int ok = 0;

							if (force==1)
							{
								ok = 1;
							}
							else
							{
								char buf[BUFSIZ];
								printf(_("delete file \"%s\" [yes/no]? :"),dir.file[i].name);
								fgets (buf,BUFSIZ,stdin);
								if (strncmp(buf,_("yes"),3)==0) ok = 1;
							}
							
							if (ok == 1)
							{
								arg.slot = slot;
								arg.fileno = i;

								printf2 (_("deleting \"%s\"...\n"),dir.file[i].name);

								err = ioctl(pccl,CAEIOC_MC_DELETE_FILE,&arg);
								if (err < 0) ioerrorf("ioctl(CAEIOC_MC_DELETE_FILE)");

								if (psxdev_mc_result_is_error(arg.result))
								{
									char buf[BUFSIZ];
									psxdev_mc_result_to_string (arg.result,buf,BUFSIZ);
									errorf (buf);
								}
								
								found = 1;
								break;
							}
						}
					}
				}
			}

			err = ioctl(pccl,CAEIOC_RESUME);
			if (err < 0) ioerrorf("ioctl(CAEIOC_RESUME)");

			close (pccl);
		}
		else ioerrorf ("open(%s)",device);
	}

	return EXIT_SUCCESS;
}

/*
** Local variables:
** c-indent-level: 4
** tab-width: 4
*/
