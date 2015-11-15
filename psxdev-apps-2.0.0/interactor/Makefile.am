## Process this file with automake to produce Makefile.in

SUBDIRS = intl po macros src

install-data-local:
	test -d pixmaps \
	&& install -d -m 0755 @PACKAGE_PIXMAPS_DIR@ \
	&& install -m 0644 pixmaps/* @PACKAGE_PIXMAPS_DIR@

dist-hook:
	test -d pixmaps \
	&& mkdir $(distdir)/pixmaps \
	&& cp -p pixmaps/* $(distdir)/pixmaps

