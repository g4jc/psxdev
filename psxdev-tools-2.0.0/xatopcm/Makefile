# $Id: Makefile,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $

# idiot Makefile for ppmtotim
# Copyright (C) 1997,1998,1999,2000 by Daniel Balster <dbalster@psxdev.de>

CFLAGS  := $(CFLAGS)  -I.
LDFLAGS := $(LDFLAGS) -s

xatopcm : xatopcm.o xadecode.o

clean::
	rm -f *.o xatopcm
