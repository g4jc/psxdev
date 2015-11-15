# $Id: Makefile,v 1.1.1.1 2001/05/17 11:51:04 dbalster Exp $

#
#	pccl - a driver for PCcommslink board (connected to actionreplay/caetla)
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

# you should add your IO port(s) here, to force detection.
# autodetection works too, but is not recommended (all systems
# need to be online during module probing for that)
#
PCCL_INSMOD_OPTIONS = "io=0x320"

# this is a preliminary major device number. I have requested an official
# major device number for /dev/pccl, but still waiting for it
#
MAJOR = 240

# /usr/src/linux should be default
#
VERSION=$(shell uname -r)
KERNEL=/usr/src/linux
#KERNEL=/usr/src/linux-$(VERSION)

# you probably don't want to change this
#
ETC_CONF_MODULES = /etc/conf.modules

M_OBJS = pccl.o
EXTRA_CFLAGS = -DPCCL_WITH_PROCFS

install: remove_from_etc_conf_modules add_to_etc_conf_modules
	-@DIR=`pwd` && (cd $(KERNEL) && make SUBDIRS=$$DIR modules) 2>&1 > /dev/null
	@install -m 644 -g 0 -o 0 $(M_OBJS) /lib/modules/$(VERSION)/misc/$(M_OBJS)
	@install -m 644 -g 0 -o 0 pccl.h $(PSXDEV)/include/pccl.h 
	-@for dev in 0 1 2 3; do \
		rm -f /dev/pccl$$dev ; \
		mknod /dev/pccl$$dev c $(MAJOR) $$dev ; \
		chmod 666 /dev/pccl$$dev ; \
		done
	-@rm -f /dev/pccl
	-@ln -s /dev/pccl0 /dev/pccl
	-@rm -f $(M_OBJS) .*.o.flags .etc.conf.modules *~
	-@depmod -a
	-@/etc/rc.d/init.d/psxdev restart

uninstall: remove_from_etc_conf_modules
	-@rm -f /dev/pccl?
	-@rm -f /dev/pccl
	-@rm -f /lib/modules/$(VERSION)/misc/$(M_OBJS)
	-@rm -f $(PSXDEV)/include/pccl.h
	-@find /etc/rc.d/ -name *psxdev | xargs rm

remove_from_etc_conf_modules::
	@if [ -f $(ETC_CONF_MODULES) ] ; then \
		rm -f .etc.conf.modules ; \
		grep -v pccl $(ETC_CONF_MODULES) > .etc.conf.modules ; \
		cp .etc.conf.modules $(ETC_CONF_MODULES) ; \
		rm -f .etc.conf.modules ; \
	fi

add_to_etc_conf_modules::
	@echo "## begin of \"/dev/pccl\"" >> $(ETC_CONF_MODULES)
	@echo "alias char-major-$(MAJOR) pccl" >>$(ETC_CONF_MODULES)
	@echo "options pccl debug=0 $(PCCL_INSMOD_OPTIONS)" >>$(ETC_CONF_MODULES)
	@echo "## end of \"/dev/pccl\"" >>$(ETC_CONF_MODULES)

include $(KERNEL)/Rules.make
