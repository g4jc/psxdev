# $Id: Makefile.am,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $

#
#	pcmtools - various tools to handle audio data
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

bin_PROGRAMS = pcmtoadpcm adpcmtopcm oss-record oss-play oss-query

pcmtoadpcm_SOURCES = pcmtoadpcm.c error.c common.h
pcmtoadpcm_LDADD = -lpsxdev

adpcmtopcm_SOURCES = adpcmtopcm.c error.c common.h
adpcmtopcm_LDADD = -lpsxdev

oss_record_SOURCES = record.c error.c common.h
oss_record_LDADD =

oss_play_SOURCES = play.c error.c common.h
oss_play_LDADD =

oss_query_SOURCES = query.c error.c common.h
oss_query_LDADD =

localedir = $(datadir)/locale

INCLUDES = \
	@INCLUDES@ \
	@psxdev_includes@ \
	-DLOCALEDIR=\"$(localedir)\" \
	-DBINDIR=\"$(bindir)\"
