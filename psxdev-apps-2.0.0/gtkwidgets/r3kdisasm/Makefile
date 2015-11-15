#
# simple Makefile
#
# Copyright (C) 1999, 2000 by Daniel Balster <dbalster@psxdev.de>
#

WHERE_IS_IT = ..

all:
	$(CC) \
	$(shell gtk-config --cflags) \
	$(shell gtk-config --libs) \
	-o r3kdisasm \
	main.c \
	$(WHERE_IS_IT)/gtkr3kdisasm.c \
	$(WHERE_IS_IT)/gtkmemory.c \
	$(WHERE_IS_IT)/gtkbreakpoints.c \
	$(WHERE_IS_IT)/r3k_disasm.c \
	-I $(WHERE_IS_IT)
