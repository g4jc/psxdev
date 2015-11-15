/* $Id: debugger.c,v 1.1.1.1 2001/05/17 11:50:27 dbalster Exp $ */

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

#include <sys/file.h>
#include <errno.h>

void psxdev_debug_get_memory (char *devicename, u_long addr, void *ptr, u_long size)
{
	caetla_memory_t mem;
	caetla_device_info_t info;
	u_char checksum;
	u_long x;
	int pccl, rc;

	mem.address = addr;
	mem.size    = size;
/*
	pccl = open (devicename,O_RDWR);
	if (pccl == -1) ioerrorf ("open()");

	rc = ioctl (pccl,CAEIOC_DU_READ_MEMORY,&mem);
	if (rc <= -1) ioerrorf ("ioctl(CAEIOC_DU_READ_MEMORY)");
	
	rc = read (pccl,ptr,size);
	if (rc < 0) ioerrorf ("write()");

	rc = ioctl (pccl,CAEIOC_SWAP_8,&checksum);
	if (rc <= -1) ioerrorf ("ioctl(CAEIOC_SWAP_8)");

	rc = ioctl (pccl,CAEIOC_DEVICE_INFO,&info);
	if (rc <= -1) ioerrorf ("ioctl(CAEIOC_DEVICE_INFO)");

	if (checksum != info.checksum)
	{
		ioerrorf ("invalid checksum (%02x/%02x)",checksum,info.checksum);
	}

	x = 0;
	rc = ioctl (pccl,CAEIOC_SWAP_32,&x);
	if (rc <= -1) ioerrorf ("ioctl(CAEIOC_SWAP_32)");
	x = 0;
	rc = ioctl (pccl,CAEIOC_SWAP_32,&x);
	if (rc <= -1) ioerrorf ("ioctl(CAEIOC_SWAP_32)");

	rc = ioctl (pccl,CAEIOC_SWAP_8,&checksum);
	if (rc <= -1) ioerrorf ("ioctl(CAEIOC_SWAP_8)");
	rc = ioctl (pccl,CAEIOC_SWAP_8,&checksum);
	if (rc <= -1) ioerrorf ("ioctl(CAEIOC_SWAP_8)");

	rc = ioctl (pccl,CAEIOC_RESUME);
	if (rc == -1) ioerrorf ("ioctl(CAEIOC_RESUME)");
	close (pccl);
*/
}

void psxdev_debug_set_memory (char *devicename, u_long addr, void *ptr, u_long size)
{
	caetla_memory_t mem;
	int pccl, rc;

	mem.address = addr;
	mem.size    = size;
/*
	pccl = open(devicename,O_RDWR);
	if (pccl == -1) ioerrorf ("open()");

	rc = ioctl (pccl,CAEIOC_DU_WRITE_MEMORY,&mem);
	if (rc <= -1) ioerrorf ("ioctl(CAEIOC_DU_WRITE_MEMORY)");
	
	rc = write (pccl,ptr,size);
	if (rc < 0) ioerrorf ("write()");

	rc = ioctl (pccl,CAEIOC_RESUME);
	if (rc == -1) ioerrorf ("ioctl(CAEIOC_RESUME)");
	close (pccl);
*/
}
