# $Id: Makefile.am,v 1.2 2001/05/17 12:57:00 dbalster Exp $

#
#	libtim - library for the TIM image format
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

timincludedir = $(includedir)
timinclude_HEADERS = tim.h
lib_LTLIBRARIES = libtim.la
libtim_la_SOURCES = init.c config.h

libtim_la_LDFLAGS = \
	-export-dynamic -version-info @library_version@

noinst_HEADERS = common.h

localedir = $(datadir)/locale

INCLUDES = \
	@INCLUDES@ \
	@psxdev_includes@ \
	-DLOCALEDIR=\"$(localedir)\" \
	-DBINDIR=\"$(bindir)\"
