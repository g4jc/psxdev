# $Id: Makefile.am,v 1.2 2001/05/17 12:56:52 dbalster Exp $

#
#	libbs - library for bitstream image format
#
#	Copyright (C) 1999, 2000 by these people, who contributed to this project
#
#	  bero@geocities.co.jp
#	  Daniel Balster <dbalster@psxdev.de>
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

#	DCT code is based on Independent JPEG Group's sotfware

bsincludedir = $(includedir)
bsinclude_HEADERS = bs.h
lib_LTLIBRARIES = libbs.la
libbs_la_SOURCES = bs.c jfdctint.c idctfst.c vlc.c

libbs_la_LDFLAGS = \
	-export-dynamic -version-info @library_version@

noinst_HEADERS = table.h

localedir = $(datadir)/locale

INCLUDES = \
	@INCLUDES@ \
	@psxdev_includes@ \
	-DLOCALEDIR=\"$(localedir)\" \
	-DBINDIR=\"$(bindir)\"
