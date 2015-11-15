# $Id: Makefile.am,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $

#
#	psxtools - communication and data convert tools for the unix shell
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

bin_PROGRAMS = \
	psx-bin2x psx-bfd2x \
	psx-run psx-reset psx-resume psx-halt psx-mode \
	psx-upload psx-download psx-server psx-flash  \
	psx-codedown psx-codeup psx-exe2mexe \
	psx-xaldown psx-xalup psx-xaldump \
	mc-ls mc-mkfs mc-rm mc-down mc-up \
	tim-down psx-fbmode tim-up \
	mc-geticon \
	psx-show-registers

psx_bin2x_SOURCES = bin2x.c error.c common.h
psx_bin2x_LDADD = @psxdev_libraries@ -lpsxdev

psx_bfd2x_SOURCES = bfd2x.c error.c common.h
psx_bfd2x_LDADD = @psxdev_libraries@ -lpsxdev -lbfd -liberty

psx_run_SOURCES = run.c error.c common.h
psx_run_LDADD = @psxdev_libraries@ -lpsxdev

psx_reset_SOURCES = reset.c error.c common.h
psx_reset_LDADD = @psxdev_libraries@ -lpsxdev

psx_mode_SOURCES = mode.c error.c common.h
psx_mode_LDADD = @psxdev_libraries@ -lpsxdev

psx_halt_SOURCES = halt.c error.c common.h
psx_halt_LDADD = @psxdev_libraries@ -lpsxdev

psx_resume_SOURCES = resume.c error.c common.h
psx_resume_LDADD = @psxdev_libraries@ -lpsxdev

psx_upload_SOURCES = upload.c error.c common.h
psx_upload_LDADD = @psxdev_libraries@ -lpsxdev

psx_download_SOURCES = download.c error.c common.h
psx_download_LDADD = @psxdev_libraries@ -lpsxdev

psx_server_SOURCES = server.c error.c common.h
psx_server_LDADD = @psxdev_libraries@ -lpsxdev

psx_flash_SOURCES = flash.c error.c common.h flasher.h
psx_flash_LDADD = @psxdev_libraries@ -lpsxdev

psx_codedown_SOURCES = codedown.c error.c common.h
psx_codedown_LDADD = @psxdev_libraries@ -lpsxdev

psx_codeup_SOURCES = codeup.c error.c common.h
psx_codeup_LDADD = @psxdev_libraries@ -lpsxdev

psx_xaldown_SOURCES = xaldown.c error.c common.h
psx_xaldown_LDADD = @psxdev_libraries@ -lpsxdev

psx_xalup_SOURCES = xalup.c error.c common.h
psx_xalup_LDADD = @psxdev_libraries@ -lpsxdev

psx_xaldump_SOURCES = xaldump.c error.c common.h
psx_xaldump_LDADD =

mc_ls_SOURCES = ls.c error.c common.h
mc_ls_LDADD = @psxdev_libraries@ -lpsxdev

mc_mkfs_SOURCES = mkfs.c error.c common.h
mc_mkfs_LDADD = @psxdev_libraries@ -lpsxdev

mc_rm_SOURCES = rm.c error.c common.h
mc_rm_LDADD = @psxdev_libraries@ -lpsxdev

mc_down_SOURCES = mcdown.c error.c common.h
mc_down_LDADD = @psxdev_libraries@ -lpsxdev

mc_up_SOURCES = mcup.c error.c common.h
mc_up_LDADD = @psxdev_libraries@ -lpsxdev

psx_exe2mexe_SOURCES = exe2mexe.c error.c common.h
psx_exe2mexe_LDADD = @psxdev_libraries@ -lpsxdev -ltim

mc_geticon_SOURCES = geticon.c error.c common.h
mc_geticon_LDADD = @psxdev_libraries@ -lpsxdev -ltim

tim_down_SOURCES = timdown.c error.c common.h
tim_down_LDADD = @psxdev_libraries@ -lpsxdev @tim_libraries@ -ltim

tim_up_SOURCES = timup.c error.c common.h
tim_up_LDADD = @psxdev_libraries@ -lpsxdev @tim_libraries@ -ltim

psx_fbmode_SOURCES = fbmode.c error.c common.h
psx_fbmode_LDADD = @psxdev_libraries@ -lpsxdev

psx_show_registers_SOURCES = show_registers.c error.c common.h
psx_show_registers_LDADD = @psxdev_libraries@ -lpsxdev

localedir = $(datadir)/locale

INCLUDES = \
	@INCLUDES@ \
	@psxdev_includes@ \
	@tim_includes@ \
	-DLOCALEDIR=\"$(localedir)\" \
	-DBINDIR=\"$(bindir)\"
