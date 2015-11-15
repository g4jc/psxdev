/* $Id: common.h,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $ */

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

#ifndef __COMMON_H
#define __COMMON_H

#define _GNU_SOURCE
#define _USE_GNU

#include "config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <stdarg.h>

#if ENABLE_NLS
#if HAVE_LOCALE_H
#include <locale.h>
#endif
#if HAVE_LIBINTL_H
#include <libintl.h>
#endif
#define _(string) gettext(string)
#define N_(string) (string)
#else
#define _(string) (string)
#define N_(string) (string)
#endif

/*
** some PSX types
*/

typedef struct {
	u_long pc0;
	u_long gp0;
	u_long t_addr;
	u_long t_size;
	u_long d_addr;
	u_long d_size;
	u_long b_addr;
	u_long b_size;
	u_long s_addr;
	u_long s_size;
	u_long sp,fp,gp,ret,base;
} exec_header_t;

typedef struct {
	char key[8];
	u_long text;
	u_long data;
	exec_header_t exec;
	char title[60];
} psxexe_hdr_t;

#define errorf(args...)		error_printf( ## args)
#define warnf(args...)		warn_printf( ## args)
#define ioerrorf(args...)	ioerror_printf( ## args)
#define iowarnf(args...)	iowarn_printf( ## args)

/* prototypes */

#ifdef __cplusplus
extern "C" {
#endif

void error_printf (char *, ...);
void ioerror_printf (char *, ...);
void warn_printf (char *, ...);
void iowarn_printf (char *, ...);

#ifdef __cplusplus
}
#endif

#endif
