#
# simple Makefile
#
# Copyright (C) 1999, 2000 by Daniel Balster <dbalster@psxdev.de>
#

WHERE_IS_GTK_HEX_ENTRY = ..

all:
	$(CC) \
	$(shell gtk-config --cflags) \
	$(shell gtk-config --libs) \
	-o hexentry \
	main.c \
	$(WHERE_IS_GTK_HEX_ENTRY)/gtkhexentry.c \
	-I $(WHERE_IS_GTK_HEX_ENTRY)
