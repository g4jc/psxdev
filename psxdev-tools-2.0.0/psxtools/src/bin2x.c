/* $Id: bin2x.c,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $ */

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

/* GNU long-options */

enum {
	OPTION_QUIET	= 'q' ,
	OPTION_HELP		= 'h' ,
	OPTION_VERBOSE	= 'v' ,
	OPTION_VERSION	= 'V' ,
	OPTION_OUTPUT	= 'o' ,
	OPTION_SCEE		= 'E' ,
	OPTION_SCEA		= 'A' ,
	OPTION_SCEI		= 'I' ,
	OPTION_STACK	= 's' ,
	OPTION_ADDRESS	= 'a' ,
	OPTION_PC		= 'P' ,
	OPTION_GP		= 'G' ,
	OPTION_TITLE	= 't' ,
};

static struct option long_options[] =
{
	{ "quiet" ,no_argument, NULL, OPTION_QUIET },
	{ "help" ,no_argument, NULL, OPTION_HELP },
	{ "verbose" ,no_argument, NULL, OPTION_VERBOSE },
	{ "version" ,no_argument, NULL, OPTION_VERSION },
	{ "output", required_argument, NULL, OPTION_OUTPUT },
	{ "SCEE" ,no_argument, NULL, OPTION_SCEE },
	{ "SCEA" ,no_argument, NULL, OPTION_SCEA },
	{ "SCEI" ,no_argument, NULL, OPTION_SCEI },
	{ "address", required_argument, NULL, OPTION_ADDRESS },
	{ "stack", required_argument, NULL, OPTION_STACK },
	{ "pc", required_argument, NULL, OPTION_PC },
	{ "gp", required_argument, NULL, OPTION_GP },
	{ "title" ,required_argument, NULL, OPTION_TITLE },
	{ NULL }
};

static char *option_help[] = {
	N_("be quiet"),
	N_("print this text"),
	N_("be verbose"),
	N_("print binary version"),
	N_("set output file"),
	N_("set area code for europe"),
	N_("set area code for north america"),
	N_("set area code for japan"),
	N_("set stack pointer (SP, default: 0x801ffff0)"),
	N_("set base address (default: 0x80010000)"),
	N_("set program counter (PC, default: address)"),
	N_("set global pointer (GP, default: 0)"),
	N_("set a custom area code string"),
};

static char *version_text =
"1.0.0"
;

static char *copyright_text = N_(
"%s - RAW BINARY to PS-X EXE converter\n"
"Copyright (C) 1997, 1998, 1999, 2000 by Daniel Balster <dbalster@psxdev.de>\n"
"All Rights Reserved.\n"
"\n");

#define printf1(fmt,args...)	if (1<=verbose_level) printf(fmt, ## args)
#define printf2(fmt,args...)	if (2<=verbose_level) printf(fmt, ## args)
int verbose_level = 1;

int main (int argc, char **argv)
{
	volatile int opt, i,j;
	char *filename = NULL;
	char *outname = NULL;
	char option_chars [BUFSIZ];
	char area = 0;
	u_long pc = 0x0;
	u_long gp = 0x0;
	u_long stack = 0x801ffff0;
	u_long size,base = 0x80010000;
	int fd;

	char buffer[2048];
	psxexe_hdr_t *psx = (psxexe_hdr_t *) buffer;
	memset (buffer,0,2048);

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

			case OPTION_STACK:
				stack = strtoul (optarg,NULL,16);
				break;

			case OPTION_SCEE:
				area = 'e';
				break;

			case OPTION_SCEI:
				area = 'i';
				break;

			case OPTION_SCEA:
				area = 'a';
				break;

			case OPTION_ADDRESS:
				base = strtoul (optarg,NULL,16);
				break;

			case OPTION_PC:
				pc = strtoul (optarg,NULL,16);
				break;

			case OPTION_GP:
				gp = strtoul (optarg,NULL,16);
				break;

			case OPTION_TITLE:
				strcpy(psx->title,optarg);
				break;

			default:
				exit(1);
		}
	}

	if (optind < argc) filename = argv[optind];

	if (getenv("PSXDEV_QUIET") && (verbose_level==1)) verbose_level = 0;


	if (filename == NULL) errorf (_("input filename missing"));
	printf2 (_("Reading from \"%s\"\n"),filename);

	printf1 (_("Object file format is \"RAW BINARY\"\n"));

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
			
			errno = 0;
			bytes = (int) mmap (NULL,st.st_size,PROT_READ,MAP_PRIVATE,fd,0);
			if (bytes != -1)
			{
				FILE *file;
				u_char *mem = (u_char *) bytes;

				// setup PS-X header

				strncpy (psx->key,"PS-X EXE",8);

				switch (area)
				{
					case 'i':
						strncpy(psx->title,"Sony Computer Entertainment Inc. for Japan area",60);
						break;
					case 'a':
						strncpy(psx->title,"Sony Computer Entertainment Inc. for North America area",60);
						break;
					case 'e':
						strncpy(psx->title,"Sony Computer Entertainment Inc. for Europe area",60);
						break;
				}

				if (pc == 0)  pc = base;

				size = st.st_size;
				size = ((size+0x7ff) >> 11) << 11;

				psx->exec.pc0 = pc;
				psx->exec.gp0 = gp;
				psx->exec.t_size = size;
				psx->exec.t_addr = base;
				psx->exec.s_addr = stack;
				psx->exec.s_size = 0;

				printf2 (_("PC=0x%08lx GP=0x%08lx Base=%08lx Size=0x%08lx\n"),psx->exec.pc0,psx->exec.gp0,base,size);
				printf2 (_("Section [   .text] Address [%08lx] Length [%06lx] *\n"),base,size);

				if (outname == NULL) outname = "psx.exe";
				printf2 (_("Writing to \"%s\"\n"),outname);

				file = fopen (outname,"wb");
				if (file)
				{
					if (1 != fwrite (buffer,2048,1,file))
						iowarnf ("fwrite()");

					if (1 != fwrite (mem,size,1,file))
						iowarnf ("fwrite()");

					fclose (file);
				}
				else iowarnf ("open(%s)",outname);

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
