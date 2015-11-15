/* $Id: error.c,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $ */

/*
	pcmtools - various tools to handle audio data

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

void error_printf (char *msg, ...)
{
	char buffer [BUFSIZ];
	va_list ap;

	va_start (ap, msg);
	vsnprintf (buffer, BUFSIZ, msg, ap);
	va_end (ap);
	
	fprintf (stderr, "%s: %s\n", program_invocation_short_name, buffer);

	exit(1);
}

void ioerror_printf (char *msg, ...)
{
	char buffer [BUFSIZ];
	va_list ap;

	va_start (ap, msg);
	vsnprintf (buffer, BUFSIZ, msg, ap);
	va_end (ap);
	
	fprintf (stderr, "%s: %s; %s\n", program_invocation_short_name, buffer, strerror(errno));

	exit(1);
}

void warn_printf (char *msg, ...)
{
	char buffer [BUFSIZ];
	va_list ap;

	va_start (ap, msg);
	vsnprintf (buffer, BUFSIZ, msg, ap);
	va_end (ap);
	
	fprintf (stderr, "%s: %s\n", program_invocation_short_name, buffer);
}

void iowarn_printf (char *msg, ...)
{
	char buffer [BUFSIZ];
	va_list ap;

	va_start (ap, msg);
	vsnprintf (buffer, BUFSIZ, msg, ap);
	va_end (ap);
	
	fprintf (stderr, "%s: %s; %s\n", program_invocation_short_name, buffer, strerror(errno));
}
