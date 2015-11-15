#
# simple Makefile
#
# Copyright (C) 1999, 2000 by Daniel Balster <dbalster@psxdev.de>
#

WHERE_IS_GTK_DIR_BROWSER = ..

all:
	$(CC) \
	$(shell gtk-config --cflags) \
	$(shell gtk-config --libs) \
	-o dirbrowser \
	main.c \
	$(WHERE_IS_GTK_DIR_BROWSER)/gtkdirbrowser.c \
	-I $(WHERE_IS_GTK_DIR_BROWSER)
