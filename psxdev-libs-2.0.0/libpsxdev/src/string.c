/* $Id: string.c,v 1.1.1.1 2001/05/17 11:50:27 dbalster Exp $ */

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
#include <pccl.h>
#include <stdio.h>

/*
	get a string from pccl:
	``length'' is the maximum buffer length
	returns <0 for and error and the string length else.
*/

int psxdev_pccl_get_string (int dev, char *buffer, int length)
{
	int err, i=0;
	u_char byte;

	do {
		byte = 0;
		err = ioctl(dev,CAEIOC_SWAP_8,&byte);
		if (err < 0) return err;
		
		buffer[i] = byte;
		i++;
	
	} while ((byte != 0) && (i < length));

	return i;
}

/*
	put a string to pccl:
	``length'' is the maximum number of characters to transfer (if <=0, all)
	returns <0 for and error and the string length else.
*/

int psxdev_pccl_put_string (int dev, char *buffer, int length)
{
	int err, i=0;
	u_char byte;

	do {
		byte = buffer[i];
	
		err = ioctl(dev,CAEIOC_SWAP_8,&byte);
		if (err < 0) return err;

		i++;
	
	} while ((byte != 0) && (i < length));

	if (byte != 0)	/* string truncated ? */
	{
		byte = 0;
		err = ioctl(dev,CAEIOC_SWAP_8,&byte);
		if (err < 0) return err;
	}

	return i;
}

/*
	here some memory functions. they return a checksum (cast to u_char)
*/

int psxdev_pccl_put_memory (int dev, u_char *buffer, int length)
{
	int err, i;
	u_char byte, chk=0;

	for (i=0; i<length; i++, chk+=byte)
	{
		byte = buffer[i];
	
		err = ioctl(dev,CAEIOC_SWAP_8,&byte);
		if (err < 0) return err;
	}

	return chk;
}

int psxdev_pccl_get_memory (int dev, u_char *buffer, int length)
{
	int err, i;
	u_char byte, chk=0;

	for (i=0; i<length; i++, chk+=byte)
	{
		byte = 0;
	
		err = ioctl(dev,CAEIOC_SWAP_8,&byte);
		if (err < 0) return err;

		buffer[i] = byte;
	}

	return chk;
}
