/* $Id: mcup.c,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $ */

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
"%s - upload file(s) to a memory card\n"
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

	int rc, fd, pccl;
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

			case OPTION_SLOT:
				slot = strtoul (optarg, 0, 16);
				break;

			case OPTION_FORCE:
				force = 1;
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
		caetla_mc_directory_t dir;
		caetla_device_info_t info;
		caetla_mc_file_create_t file;
		caetla_mc_result_t res;

		on_exit (on_exit_handler,(void*)pccl);

		rc = ioctl(pccl,CAEIOC_CONTROL,CAETLA_MCM);
		if (rc < 0) ioerrorf("ioctl(CAEIOC_CONTROL)");

		while (optind < argc)
		{
			filename = argv[optind++];

			printf2 (_("uploading %s\n"),filename);

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
						file.slot = slot;
						file.size = st.st_size - 0x36;
						memset (file.name,0,20);
						strcpy (file.name,bytes);
						
						dir.slot = slot;
						rc = ioctl(pccl,CAEIOC_MC_DIRECTORY, &dir);
						if (rc < 0) ioerrorf("ioctl(CAEIOC_MC_DIRECTORY)");

						for (i=0; i<dir.count; i++)
						{
							if (strcmp(dir.file[i].name,file.name)==0)
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
									res.slot = slot;
									res.fileno = i;

									printf2 (_("deleting %s\n"),dir.file[i].name);

									rc = ioctl(pccl,CAEIOC_MC_DELETE_FILE,&res);
									if (rc < 0) ioerrorf("ioctl(CAEIOC_MC_DELETE_FILE)");

									if (psxdev_mc_result_is_error(res.result))
									{
										char buf[BUFSIZ];
										psxdev_mc_result_to_string (res.result,buf,BUFSIZ);
										errorf (buf);
									}
									break;
								}
							}
						}
						
						rc = ioctl(pccl,CAEIOC_MC_CREATE_FILE,&file);
						if (rc < 0) ioerrorf("ioctl(CAEIOC_MC_CREATE_FILE)");

						rc = write (pccl,((char*)bytes)+0x36,file.blocks * 8192);
						if (rc < 0) ioerrorf("write()");

						rc = ioctl (pccl,CAEIOC_DEVICE_INFO,&info);
						if (rc == -1) ioerrorf("ioctl(CAEIOC_DEVICE_INFO)");

						rc = ioctl (pccl,CAEIOC_SWAP_16,&info.checksum2);
						if (rc == -1) ioerrorf("ioctl(CAEIOC_SWAP_16)");

						rc = ioctl (pccl,CAEIOC_SWAP_16,&file.result);
						if (rc == -1) ioerrorf("ioctl(CAEIOC_SWAP_16)");

						if (psxdev_mc_result_is_error(file.result))
						{
							char buf[BUFSIZ];
							psxdev_mc_result_to_string (file.result,buf,BUFSIZ);
							errorf (buf);
						}
					
						munmap (bytes,st.st_size);
					}
					else ioerrorf ("mmap()");
				}
				else ioerrorf ("fstat()");

				close (fd);
			}
			else ioerrorf ("open(%s)",filename);
		}
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
