# $Id: Makefile.am,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $

#
#	timtools - various tools to handle TIM images
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

bin_PROGRAMS = anytotim timtoppm timsetpos timmontage

timtoppm_SOURCES = timtoppm.c error.c common.h
timtoppm_LDADD = @tim_libraries@ -ltim

anytotim_SOURCES = anytotim.c error.c common.h
anytotim_LDADD = @tim_libraries@ -ltim $(IMLIB_LIBS)

timsetpos_SOURCES = timsetpos.c error.c common.h
timsetpos_LDADD = @tim_libraries@ -ltim

timmontage_SOURCES = timmontage.c error.c common.h
timmontage_LDADD = @tim_libraries@ -ltim

localedir = $(datadir)/locale

INCLUDES = \
	@INCLUDES@ \
	$(IMLIB_CFLAGS) \
	@psxdev_includes@ \
	@tim_includes@ \
	-DLOCALEDIR=\"$(localedir)\" \
	-DBINDIR=\"$(bindir)\"
