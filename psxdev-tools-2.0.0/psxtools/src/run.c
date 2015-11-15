/* $Id: run.c,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $ */

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
	OPTION_TASK		= 't' ,
	OPTION_EVENT	= 'e' ,
	OPTION_STACK	= 's' ,
	OPTION_SERVER	= 'S' ,
	OPTION_CONSOLE	= 'C' ,
	OPTION_NORMAL	= 'n' ,
	OPTION_RESET	= 'R' ,
	OPTION_BREAK	= 'b' ,
	OPTION_POLL		= 'p' ,
	OPTION_BASE		= 'B' ,
};

static struct option long_options[] =
{
	{ "quiet" ,no_argument, NULL, OPTION_QUIET },
	{ "help" ,no_argument, NULL, OPTION_HELP },
	{ "verbose" ,no_argument, NULL, OPTION_VERBOSE },
	{ "version" ,no_argument, NULL, OPTION_VERSION },
	{ "device" ,required_argument, NULL, OPTION_DEVICE },
	{ "task" ,required_argument, NULL, OPTION_TASK },
	{ "event" ,required_argument, NULL, OPTION_EVENT },
	{ "stack" ,required_argument, NULL, OPTION_STACK },
	{ "server" ,no_argument, NULL, OPTION_SERVER },
	{ "console" ,no_argument, NULL, OPTION_CONSOLE },
	{ "normal" ,no_argument, NULL, OPTION_NORMAL },
	{ "reset" ,no_argument, NULL, OPTION_RESET },
	{ "break" ,no_argument, NULL, OPTION_BREAK },
	{ "poll" ,no_argument, NULL, OPTION_POLL },
	{ "argvbase" ,required_argument, NULL, OPTION_BASE },
	{ NULL }
};

static char *option_help[] = {
	N_("be quiet"),
	N_("print this text"),
	N_("be verbose"),
	N_("print binary version"),
	N_("use this device (default: /dev/pccl)"),
	N_("number of tasks allowed (default: 4)"),
	N_("number of events allowed (default: 16)"),
	N_("initial stack address (default: 0x801ffff0)"),
	N_("run with full stdio and file server"),
	N_("run without server, but with console mode activated"),
	N_("normal start"),
	N_("reset PSX before upload"),
	N_("break execution at main() function"),
	N_("poll until the game stops"),
	N_("where to place argv/argc arguments in memory"),
};

static char *version_text =
"1.0.0"
;

static char *copyright_text = N_(
"%s - upload and start PS-X EXE files\n"
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
	char *filename = NULL;
	char *device = "/dev/pccl";
	char option_chars [BUFSIZ];

	int rc, fd;

	u_long task = 4;
	u_long event = 0x10;
	u_long stack = 0x801ffff0;
	u_long argvbase = 0xE800;	// this is a hack...
	
	int server = 0;
	int console = 0;
	int normal = 0;
	int reset = 0;
	int mainbreak = 0;
	int polling = 0;
	
	u_long breakpoint_instruction = 0x0A0D;
	u_long saved_instruction = 0;

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

			case OPTION_NORMAL:
				normal = 1;
				break;

			case OPTION_RESET:
				reset = 1;
				break;

			case OPTION_SERVER:
				server = 1;
			case OPTION_CONSOLE:
				console = 1;
				break;

			case OPTION_POLL:
				polling = 1;
				break;

			case OPTION_STACK:
				stack = strtoul(optarg,0,16);
				break;

			case OPTION_EVENT:
				event = strtoul(optarg,0,16);
				break;

			case OPTION_TASK:
				task = strtoul(optarg,0,16);
				break;

			case OPTION_BASE:
				argvbase = strtoul(optarg,0,16);
				break;

			case OPTION_BREAK:
				mainbreak = 1;
				break;

			default:
				exit(1);
		}
	}

	if (optind < argc) filename = argv[optind++];

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
					// ..._psxexe_t is the header type for "PS-X EXE" binaries.
					psxexe_header_t *p = (psxexe_header_t *) bytes;
					caetla_execute_t exe; // execute environment
					caetla_memory_t mem;  // memory transfer
					caetla_device_info_t info;
					u_char checksum;
					
					if (strncmp(p->magic,PSXEXE_MAGIC,sizeof(PSXEXE_MAGIC)))
					{
						errorf(_("file is not a PSX executable"));
					}
					
					on_exit (on_exit_handler,(void*)pccl);

					if (reset)
					{
						rc = ioctl (pccl,CAEIOC_RESET);
						if (rc == -1) ioerrorf("ioctl(CAEIOC_RESET)");
					}
			
					// extract exec_t from "PS-X EXE" header
					memcpy (&(exe),&(p->exec),sizeof(psxexec_t));

					// setup memory transfer
					mem.address = exe.t_addr;
					mem.size    = exe.t_size;
					
					rc = ioctl (pccl,CAEIOC_WRITE_MEMORY,&mem);
					if (rc == -1) ioerrorf("ioctl(CAEIOC_WRITE_MEMORY)");

					rc = write (pccl,bytes+2048,exe.t_size);
					if (rc < 0) ioerrorf("write()");
					
					rc = ioctl (pccl,CAEIOC_SWAP_8,&checksum);
					if (rc == -1) ioerrorf("ioctl(CAEIOC_SWAP_8)");

					rc = ioctl (pccl,CAEIOC_DEVICE_INFO,&info);
					if (rc == -1) ioerrorf("ioctl(CAEIOC_DEVICE_INFO)");
					
					if (checksum != info.checksum)
					{
						warnf (_("invalid checksum (%02x/%02x)"),checksum,info.checksum);
					}
				
					if (optind < argc)
					{
						caetla_arguments_t args;
						char *p, *q;
						int n;
						
						n = strlen(filename) + 1;
						
						for (i=optind; i<argc; ++i)
						{
							n += strlen(argv[i]) + 1;
						}
						
						q = alloca (n); p = q;

						strcpy (p,filename);
						p += strlen(filename) + 1;
						*p = 0;

						for (i=optind; i<argc; ++i)
						{
							strcpy (p,argv[i]);
							p += strlen(argv[i]) + 1;
							*p = 0;
						}
						
						args.argc    = argc - optind + 1;
						args.argv    = argvbase;
						args.address = argvbase+((1+args.argc)*4);

						mem.address = args.address;
						mem.size    = n;

						rc = ioctl (pccl,CAEIOC_WRITE_MEMORY,&mem);
						if (rc == -1) ioerrorf("ioctl(CAEIOC_WRITE_MEMORY)");

						rc = write (pccl,q,n);
						if (rc < 0) ioerrorf("write()");

						rc = ioctl (pccl,CAEIOC_SWAP_8,&checksum);
						if (rc == -1) ioerrorf("ioctl(CAEIOC_SWAP_8)");

						rc = ioctl (pccl,CAEIOC_DEVICE_INFO,&info);
						if (rc == -1) ioerrorf("ioctl(CAEIOC_DEVICE_INFO)");

						if (checksum != info.checksum)
						{
							warnf (_("invalid checksum (%02x/%02x)"),checksum,info.checksum);
						}

						rc = ioctl (pccl,CAEIOC_ARGUMENTS,&args);
						if (rc == -1) ioerrorf("ioctl(CAEIOC_ARGUMENTS)");
					}
				
					// setup console mode
					if (console)
					{
						rc = ioctl (pccl,CAEIOC_CONSOLE,console);
						if (rc == -1) ioerrorf("ioctl(CAEIOC_CONSOLE)");
					}

					// default environment
					exe.task  = task;
					exe.event = event;
					exe.stack = stack;

					exe.t_size += 2048 - (exe.t_size - (exe.t_size/2048)*2048);
					
					exe.mode = normal;
					
					if (mainbreak==1)
					{
						mem.address = exe.pc0;
						mem.size    = 4;
					
						rc = ioctl (pccl,CAEIOC_READ_MEMORY,&mem);
						if (rc == -1) ioerrorf("ioctl(CAEIOC_READ_MEMORY)");

						rc = read (pccl,&saved_instruction,4);
						if (rc < 0) ioerrorf("read()");
					
						rc = ioctl (pccl,CAEIOC_SWAP_8,&checksum);
						if (rc == -1) ioerrorf("ioctl(CAEIOC_SWAP_8)");

						rc = ioctl (pccl,CAEIOC_DEVICE_INFO,&info);
						if (rc == -1) ioerrorf("ioctl(CAEIOC_DEVICE_INFO)");
					
						if (checksum != info.checksum)
						{
							warnf (_("invalid checksum (%02x/%02x)"),checksum,info.checksum);
						}
						
						printf1 ("Setting temporary breakpoint at EPC=0x%08lx\n",exe.pc0);

						rc = ioctl (pccl,CAEIOC_WRITE_MEMORY,&mem);
						if (rc == -1) ioerrorf("ioctl(CAEIOC_WRITE_MEMORY)");

						rc = write (pccl,&breakpoint_instruction,4);
						if (rc < 0) ioerrorf("write()");
					
						rc = ioctl (pccl,CAEIOC_SWAP_8,&checksum);
						if (rc == -1) ioerrorf("ioctl(CAEIOC_SWAP_8)");

						rc = ioctl (pccl,CAEIOC_DEVICE_INFO,&info);
						if (rc == -1) ioerrorf("ioctl(CAEIOC_DEVICE_INFO)");
					
						if (checksum != info.checksum)
						{
							warnf (_("invalid checksum (%02x/%02x)"),checksum,info.checksum);
						}
					}
				
					// send data and launch !
					rc = ioctl (pccl,CAEIOC_EXECUTE,&exe);
					if (rc == -1) ioerrorf("ioctl(CAEIOC_EXECUTE)");

					// attach  server. 0 and 1 are stdin/stdout sockets
					if (server==1)
					{
						pspar_status_t pst;
						caetla_status_t cst;

						do {
							rc = psxdev_server(pccl,0,1);
							if (rc == -1) ioerrorf("psxdev_server");

							cst = (caetla_status_t) rc;

							if (
								(cst & CAESTF_HBP_STOP) ||
								(cst & CAESTF_BRK_STOP) ||
								(cst & CAESTF_IN_CONTROL) )
							{
								if (cst & CAESTF_HBP_STOP) printf1 ("break on hardware breakpoint!\n");
								if (cst & CAESTF_BRK_STOP) printf1 ("break on break instruction!\n");
								if (cst & CAESTF_IN_CONTROL) printf1 ("PC is in control!\n");
							
								polling = 0;
							}
							else
							{
								rc = ioctl (pccl,CAEIOC_GET_STATUS_PSPAR,&pst);
								if (rc == -1) ioerrorf("ioctl(CAEIOC_GET_STATUS_PSPAR)");

								if (pst == PSPAR_IN_MENU)
								{
									polling = 0;
									printf1 ("PC is in control!\n");
								}
							}

						} while (polling);

// 						if (mainbreak!=1)
// 						{
// 							// switch off console mode
// 							rc = ioctl (pccl,CAEIOC_CONSOLE,0);
// 							if (rc == -1) ioerrorf("ioctl(CAEIOC_CONSOLE)");
// 						}
					}
					
					if (mainbreak==1)
					{
						caetla_word_t word;
						
// 						do {
// 							rc = ioctl (pccl,CAEIOC_GET_STATUS_CAETLA,&cst);
// 							if (rc == -1) ioerrorf("ioctl(CAEIOC_GET_STATUS_CAETLA)");
// 						} while (!((cst & CAESTF_IN_CONTROL) || (cst & CAESTF_BRK_STOP)));
						
						word.address = exe.pc0;
						word.data    = saved_instruction;
						rc = ioctl (pccl,CAEIOC_DU_WRITE_WORD,&word);
						if (rc == -1) ioerrorf("ioctl(CAEIOC_DU_WRITE_WORD)");
						rc = ioctl (pccl,CAEIOC_DU_FLUSH_ICACHE,0);
						if (rc == -1) ioerrorf("ioctl(CAEIOC_DU_FLUSH_ICACHE)");

						printf1 ("Temporary breakpoint removed from EPC=0x%08lx\n",exe.pc0);
					}
					else
					{
						caetla_device_mode_t mode;
						int oldmode;
					
						mode.mode = PCCL_MODE_AUTO_RESUME;

						rc = ioctl (pccl,CAEIOC_DEVICE_GET_MODE,&mode);
						if (rc == -1) ioerrorf("ioctl(CAEIOC_DEVICE_GET_MODE)");
						oldmode = mode.value;

						mode.value = 1;
						rc = ioctl (pccl,CAEIOC_DEVICE_SET_MODE,&mode);
						if (rc == -1) ioerrorf("ioctl(CAEIOC_DEVICE_SET_MODE)");

						// resume (give control back to caetla
						rc = ioctl (pccl,CAEIOC_RESUME);
						if (rc == -1) ioerrorf("ioctl(CAEIOC_RESUME)");

						mode.value = oldmode;
						rc = ioctl (pccl,CAEIOC_DEVICE_SET_MODE,&mode);
						if (rc == -1) ioerrorf("ioctl(CAEIOC_DEVICE_SET_MODE)");
					}

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
