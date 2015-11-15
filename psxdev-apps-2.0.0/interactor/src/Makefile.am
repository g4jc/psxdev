## Process this file with automake to produce Makefile.in

INCLUDES = \
	-I$(top_srcdir)/intl \
	@psxdev_includes@ \
	$(GNOME_INCLUDEDIR)

bin_PROGRAMS = psxinteractor

psxinteractor_SOURCES = \
	main.c \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h

psxinteractor_LDADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(INTLLIBS) \
	@psxdev_libraries@ -lpsxdev -lbfd -liberty

