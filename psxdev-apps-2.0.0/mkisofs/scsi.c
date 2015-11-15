/* @(#)scsi.c	1.5 98/04/17 Copyright 1997 J. Schilling */
#ifndef lint
static	char sccsid[] =
	"@(#)scsi.c	1.5 98/04/17 Copyright 1997 J. Schilling";
#endif
/*
 *	Copyright (c) 1997 J. Schilling
 */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifdef	USE_SCG
#include <mconfig.h>

#include <stdio.h>
#include <standard.h>
#include <stdxlib.h>
#include <unixstd.h>

#include "mkisofs.h"
#include "scsireg.h"
#include "cdrecord.h"

int
readsecs(startsecno, buffer, sectorcount)
	int	startsecno;
	void	*buffer;
	int	sectorcount;
{
	int	f;

	if (in_image == NULL) {
		if (read_scsi(buffer, startsecno, sectorcount) < 0)
			return (-1);
		return (SECTOR_SIZE * sectorcount);
	}

	f = fileno(in_image);

	if (lseek(f, (off_t)startsecno * SECTOR_SIZE, 0) == (off_t)-1) {
		fprintf(stderr," Seek error on old image\n");
		exit(10);
	}
	return (read(f, buffer, sectorcount * SECTOR_SIZE));
}

int
scsidev_open(path)
	char	*path;
{
/*	return (open_scsi(path, -1, 1));*/
	return (open_scsi(path, -1, 0));
}

#endif	/* USE_SCG */
