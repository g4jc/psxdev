## Process this file with automake to produce Makefile.in

INCLUDES = \
	-I$(top_srcdir)/intl \
	$(GNOME_INCLUDEDIR) \
	-I.

bin_PROGRAMS = psxcdmaker

psxcdmaker_SOURCES = \
	main.c \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h \
	gtkdirbrowser.c gtkdirbrowser.h

psxcdmaker_LDFLAGS = `gnome-config --libs gtkxmhtml` `gnome-config --libs zvt`

psxcdmaker_LDADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(INTLLIBS)
