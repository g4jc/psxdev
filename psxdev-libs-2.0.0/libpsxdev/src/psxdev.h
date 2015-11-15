/* $Id: psxdev.h,v 1.1.1.1 2001/05/17 11:50:27 dbalster Exp $ */

/*
	libpsxdev - library of useful functions for PSX developers

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

#ifndef __LIB_PSXDEV_H
#define __LIB_PSXDEV_H

#ifndef __USE_BSD	/* for bsd types u_char, u_long, u_short, ... */
#define __USE_BSD
#endif

#include <sys/types.h>
#include <stdarg.h>

/*
** some PSX types
*/

typedef struct {
	u_long pc0;                  /* program entry point */
	u_long gp0;                  /* global pointer */
	u_long t_addr;               /* text segment, address + size */
	u_long t_size;
	u_long d_addr;               /* data segment */
	u_long d_size;
	u_long b_addr;               /* bss segment */
	u_long b_size;
	u_long s_addr;               /* stack */
	u_long s_size;
	u_long sp,fp,gp,ret,base;    /* for overlay loading */
} psxexec_t;

#define PSXEXE_MAGIC "PS-X EXE"

typedef struct {
	char magic[8];               /* binary magic */
	u_long text;                 /* must be NULL ? */
	u_long data;                 /* must be NULL ? */
	psxexec_t exec;              /* see above */
	char title[60];              /* descriptive title / area code */
} psxexe_header_t;

typedef struct {
	u_long task;
	u_long event;
	u_long stack;
} psxenv_t;

/* prototypes */

#ifdef __cplusplus
extern "C" {
#endif

/* currently does nothing, but you should call this very early (future!)*/
void psxdev_init (void);

/* attach a file & stdio server to ``dev'', stdin/stdout = sockets/fds*/
int psxdev_server (int dev, int sockin, int sockout);

/* Not used currently. I prefer UNIX style filenames, I never used a 8+3 OS! */
int psxdev_filename_unix_to_psx (char *filename);
int psxdev_filename_psx_to_unix (char *filename);

/* transfer memory as string or block */
int psxdev_pccl_get_string (int dev, char *buffer, int length);
int psxdev_pccl_put_string (int dev, char *buffer, int length);
int psxdev_pccl_put_memory (int dev, u_char *buffer, int length);
int psxdev_pccl_get_memory (int dev, u_char *buffer, int length);

void psxdev_mc_result_to_string (u_short result, char *buffer, int length);
int psxdev_mc_result_is_error (u_short result);

/* independent debugger memory exchange (open,transfer,close) */
void psxdev_debug_get_memory (char *devicename, u_long addr, void *ptr, u_long size);
void psxdev_debug_set_memory (char *devicename, u_long addr, void *ptr, u_long size);

#ifdef __cplusplus
}
#endif

#endif /* __LIB_PSXDEV_H */
