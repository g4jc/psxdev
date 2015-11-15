/* $Id: util.c,v 1.1.1.1 2001/05/17 11:50:27 dbalster Exp $ */

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

#include "config.h"
#include "common.h"
#include <psxdev.h>

/*
	slash exchange functions. they return the string length
*/

int psxdev_filename_unix_to_psx (char *filename)
{
	int i=0;
	
	for (i=0; filename[i] != 0; i++)
	{
		if (filename[i] == '\\') filename[i] = '/';
	}
	
	return i;
}

int psxdev_filename_psx_to_unix (char *filename)
{
	int i=0;
	
	for (i=0; filename[i] != 0; i++)
	{
		if (filename[i] == '/') filename[i] = '\\';
	}
	
	return i;
}

void psxdev_mc_result_to_string (u_short result, char *buffer, int length)
{
	snprintf (buffer,length,"%s%s%s%s%s%s%s%s%s"
		,(result & (1L<<0))  ? "CARD_DOES_NOT_EXIST " : ""
		,(result & (1L<<1))  ? "UNFORMATTED_CARD " : ""
		,(result & (1L<<2))  ? "STATE_TRANSITION " : ""
		,(result & (1L<<10)) ? "" : "INCONSISTENCY_CHECKSUM "
		,(result & (1L<<11)) ? "" : "INVALID_PARAMETER "
		,(result & (1L<<12)) ? "" : "FILE_EXISTS "
		,(result & (1L<<13)) ? "" : "COMMUNICATION_FAILURE "
		,(result & (1L<<14)) ? "" : "STATE_TRANSITION_DURING_LOCK "
		,(result & (1L<<15)) ? "" : "STATE_TRANSITION_WHILE_EXECUTING_COMMAND "
	);
}

int psxdev_mc_result_is_error (u_short result)
{
	return (result > 7) ? 1 : 0;
}
