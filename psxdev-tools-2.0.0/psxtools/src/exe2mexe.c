/* $Id: exe2mexe.c,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $ */

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
#include <tim.h>

/* GNU long-options */

enum {
	OPTION_QUIET	= 'q' ,
	OPTION_HELP		= 'h' ,
	OPTION_VERBOSE	= 'v' ,
	OPTION_VERSION	= 'V' ,
	OPTION_ICON		= 'i' ,
	OPTION_TITLE	= 't' ,
	OPTION_OUTPUT	= 'o' ,
};

static struct option long_options[] =
{
	{ "quiet" ,no_argument, NULL, OPTION_QUIET },
	{ "help" ,no_argument, NULL, OPTION_HELP },
	{ "verbose" ,no_argument, NULL, OPTION_VERBOSE },
	{ "version" ,no_argument, NULL, OPTION_VERSION },
	{ "icon" ,required_argument, NULL, OPTION_ICON },
	{ "title" ,required_argument, NULL, OPTION_TITLE },
	{ "output", required_argument, NULL, OPTION_OUTPUT },
	{ NULL }
};

static char *option_help[] = {
	N_("be quiet"),
	N_("print this text"),
	N_("be verbose"),
	N_("print binary version"),
	N_("specify icon file(s) (16x16 4-Bit TIM)"),
	N_("specify memory card title"),
	N_("set output file"),
};

static char *version_text =
"1.0.0"
;

static char *copyright_text = N_(
"%s - PS-X EXE to MEXE converter\n"
"Copyright (C) 1997, 1998, 1999, 2000 by Daniel Balster <dbalster@psxdev.de>\n"
"All Rights Reserved.\n"
"\n");

#define printf1(fmt,args...)	if (1<=verbose_level) printf(fmt, ## args)
#define printf2(fmt,args...)	if (2<=verbose_level) printf(fmt, ## args)
int verbose_level = 1;

#define ICONSIZE ((16*16)/2)

u_char tux_tim[] = {
#include "tux_tim.h"
};

int main (int argc, char **argv)
{
	volatile int opt, i,j;
	char *filename = NULL;
	char *outname = NULL;
	char *iconname[3];
	int nicons = 0, use_builtin = 0;
	char option_chars [BUFSIZ];
	char *title = "created with PSXDEV+linux";

	int rc, fd;

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

			case OPTION_ICON:
				if (nicons>2) errorf(_("a maximum of three icons is allowed"));
				iconname[nicons] = optarg;
				nicons++;
				break;

			case OPTION_TITLE:
				title = optarg;
				break;

			default:
				exit(1);
		}
	}

	if (optind < argc) filename = argv[optind++];

	if (getenv("PSXDEV_QUIET") && (verbose_level==1)) verbose_level = 0;

	if (filename == NULL) errorf (_("input filename missing"));

	psxdev_init();

	if (nicons == 0)
	{
		nicons = 1;
		use_builtin = 1;
	}

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
				psxexe_header_t *psx = (psxexe_header_t *) bytes;
				caetla_mc_file_header_t mcfile;
				caetla_mc_mexe_header_t mexe;
				u_char *data = (u_char*) (bytes+2048);
				u_long datasize, fullsize;
				char name[0x36];
				FILE *file;
				
				if (strncmp(psx->magic,PSXEXE_MAGIC,sizeof(PSXEXE_MAGIC)))
				{
					errorf(_("file is not a PSX executable"));
				}
				
				datasize = st.st_size - 2048;
				
				fullsize = -((3-nicons)*ICONSIZE) + sizeof(name) + sizeof(mexe) + sizeof(mcfile) + datasize;
				
				// create filename header
				
				memset (name,0,sizeof(name));
				strcpy (name,"E00-EXE PSXDEV+LINUX");

				// create FILE header

				memset (&mcfile,0,sizeof(mcfile));
				mcfile.magic = MCFILE_MAGIC;
				mcfile.type = 0x10 + nicons;
				mcfile.size = fullsize / 8192;
				strncpy (mcfile.name,title,64);

				tim_init();

				if (use_builtin)	// use builtin icon
				{
					u_short *pixel, *clut;
					tim_t *tim = (tim_t*) tux_tim;

					printf2 (_("using builtin icon of tux the penguin\n"));

					pixel = (u_short*) tim_pixel (tim,0,0);
					clut = (u_short*) tim_clut (tim,0,0);
							
					memcpy (mcfile.clut,clut,16*2);
					memcpy (mcfile.icon[0],pixel,ICONSIZE);
				}
				else for (j=0;j<nicons;j++) if (iconname[j] != NULL)
				{
					int ifd = open (iconname[j],O_RDONLY);
					if (ifd != -1)
					{
						int status;
						struct stat ist;
	
						status = fstat (ifd,&ist);
						if (status != -1)
						{
							tim_t *tim;
			
							if (ist.st_size == 0) errorf(_("icon %s has zero length"),iconname[j]);

							tim = (tim_t*) mmap (NULL,ist.st_size,PROT_READ,MAP_PRIVATE,ifd,0);
							if (((int)tim) != -1)
							{
								u_short *pixel, *clut;
							
								printf2 (_("using \"%s\" as icon #%d\n"),iconname[j],j);

								if (tim->magic != TIM_MAGIC)
								errorf(_("icon %s is not a TIM image"),iconname[j]);
							
								if (
									(tim_width(tim,0) != 16) ||
									(tim_height(tim,0) != 16) ||
									(tim->flags.mode != 0) ||
									(tim->flags.clut == 0)
								) errorf(_("icon %s is not a 16x16 4-Bit TIM image"),iconname[j]);
							
								pixel = (u_short*) tim_pixel (tim,0,0);
								clut = (u_short*) tim_clut (tim,0,0);
							
								memcpy (mcfile.clut,clut,16*2);
								memcpy (mcfile.icon[j],pixel,ICONSIZE);
							
								munmap ((void*)tim,ist.st_size);
							}
							else ioerrorf ("mmap()");
						}
						else ioerrorf ("fstat()");
						close (ifd);
					}
					else ioerrorf ("open(%s)",iconname[j]);
				}

				// create MEXE header

				memset (&mexe,0,sizeof(mexe));
				mexe.magic  = MEXE_MAGIC;
				mexe.pc0	= psx->exec.pc0;
				mexe.gp0	= psx->exec.gp0;
				mexe.t_addr	= psx->exec.t_addr;
				mexe.t_size	= psx->exec.t_size;
				mexe.d_addr	= psx->exec.d_addr;
				mexe.d_size	= psx->exec.d_size;
				mexe.b_addr	= psx->exec.b_addr;
				mexe.b_size	= psx->exec.b_size;
				mexe.s_addr	= psx->exec.s_addr;
				mexe.s_size	= psx->exec.s_size;

				// chewed it enough, now spit it out

				if (outname == NULL) outname = "psx.mexe";
				printf2 (_("Writing to \"%s\"\n"),outname);

				file = fopen(outname,"wb");
				if (file)
				{
					rc = fwrite (name,sizeof(name),1,file);
					if (rc != 1) ioerrorf ("fwrite()");
					rc = fwrite (&mcfile,sizeof(mcfile)-((3-nicons)*ICONSIZE),1,file);
					if (rc != 1) ioerrorf ("fwrite()");
					rc = fwrite (&mexe,sizeof(mexe),1,file);
					if (rc != 1) ioerrorf ("fwrite()");
					rc = fwrite (data,datasize,1,file);
					if (rc != 1) ioerrorf ("fwrite()");
				
					fclose (file);
				}
				else ioerrorf ("fopen()");

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
