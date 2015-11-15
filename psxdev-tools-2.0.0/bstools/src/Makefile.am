# $Id: Makefile.am,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $

#
#	bstools - various tools to handle BS images
#
#	Copyright (C) 1997, 1998, 1999, 2000 by these people, who contributed to this project
#
#	  Daniel Balster <dbalster@psxdev.de>
#	  Sergio Moreira <sergio@x-plorer.co.uk>
#	  Andrew Kieschnick <andrewk@cerc.utexas.edu>
#	  Kazuki Sakamoto <bsd-ps@geocities.co.jp>
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

bin_PROGRAMS = anytobs bstoppm

anytobs_SOURCES = anytobs.c error.c common.h
anytobs_LDADD = -lbs $(IMLIB_LIBS)

bstoppm_SOURCES = bstoppm.c error.c common.h
bstoppm_LDADD = -lbs

localedir = $(datadir)/locale

INCLUDES = \
	@INCLUDES@ \
	$(IMLIB_CFLAGS) \
	@psxdev_includes@ \
	-DLOCALEDIR=\"$(localedir)\" \
	-DBINDIR=\"$(bindir)\"
