/* $Id: show_registers.c,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $ */

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
	N_("use this device (default: /dev/pccl)"),
};

static char *version_text =
"1.0.0"
;

static char *copyright_text = N_(
"%s - show the contents of the MIPS CPU registers\n"
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
	int rc;

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

			default:
				exit(1);
		}
	}

	if (getenv("PSXDEV_QUIET") && (verbose_level==1)) verbose_level = 0;


	psxdev_init();

	{
		int pccl = open(device,O_RDWR);
		if (pccl != -1)
		{
			caetla_registers_t reg;
			u_char st;
		
			on_exit (on_exit_handler,(void*)pccl);

			rc = ioctl (pccl,CAEIOC_GET_STATUS_PSPAR,&st);
			if (rc == -1) ioerrorf("ioctl(CAEIOC_GET_STATUS_PSPAR)");

			if (st == 0x00) errorf (_("no program is running"));

			rc = ioctl (pccl,CAEIOC_CONTROL,CAETLA_DU);
			if (rc == -1) ioerrorf("ioctl(CAEIOC_CONTROL)");
			
			rc = ioctl (pccl,CAEIOC_DU_GET_REGISTERS,&reg);
			if (rc == -1) ioerrorf("ioctl(CAEIOC_DU_GET_REGISTERS)");
		
			printf1 (
				"at:%08lx  v0:%08lx  v1:%08lx\n"
				"a0:%08lx  a1:%08lx  a2:%08lx\n"
				"a3:%08lx  t0:%08lx  t1:%08lx\n"
				"t2:%08lx  t3:%08lx  t4:%08lx\n"
				"t5:%08lx  t6:%08lx  t7:%08lx\n"
				"s0:%08lx  s1:%08lx  s2:%08lx\n"
				"s3:%08lx  s4:%08lx  s5:%08lx\n"
				"s6:%08lx  s7:%08lx  t8:%08lx\n"
				"t9:%08lx  k0:%08lx  k1:%08lx\n"
				"gp:%08lx  sp:%08lx  fp:%08lx\n"
				"ra:%08lx  pc:%08lx  hi:%08lx\n"
				"lo:%08lx  sr:%08lx  cr:%08lx\n"
				,reg.data[ 0],reg.data[ 1],reg.data[ 2]
				,reg.data[ 3],reg.data[ 4],reg.data[ 5]
				,reg.data[ 6],reg.data[ 7],reg.data[ 8]
				,reg.data[ 9],reg.data[10],reg.data[11]
				,reg.data[12],reg.data[13],reg.data[14]
				,reg.data[15],reg.data[16],reg.data[17]
				,reg.data[18],reg.data[19],reg.data[20]
				,reg.data[21],reg.data[22],reg.data[23]
				,reg.data[24],reg.data[25],reg.data[26]
				,reg.data[27],reg.data[28],reg.data[29]
				,reg.data[30],reg.data[31],reg.data[32]
				,reg.data[33],reg.data[34],reg.data[35]
			);

			rc = ioctl (pccl,CAEIOC_RESUME);
			if (rc == -1) ioerrorf("ioctl(CAEIOC_RESUME)");

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
