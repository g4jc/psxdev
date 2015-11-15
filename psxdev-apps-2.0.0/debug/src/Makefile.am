## Process this file with automake to produce Makefile.in

INCLUDES = \
	-I$(top_srcdir)/intl \
	$(GNOME_INCLUDEDIR) \
	@psxdev_includes@ \
	-I.

bin_PROGRAMS = psxdebug

psxdebug_SOURCES = \
	main.c \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h \
	gtkmemory.c gtkmemory.h \
	r3k_disasm.c r3k_disasm.h \
	gtkhexentry.c gtkheyentry.h \
	gtkr3kdisasm.c gtkr3kdisasm.h \
	gtkbreakpoints.c gtkbreakpoints.h

psxdebug_LDADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(INTLLIBS) \
	@psxdev_libraries@ -lpsxdev -lpthread

