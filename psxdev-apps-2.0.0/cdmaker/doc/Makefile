psxcdmaker.doc.bz2:
	m4 online.in > psxcdmaker.doc
	bzip2 psxcdmaker.doc

install: psxcdmaker.doc.bz2
	mkdir -p $(prefix)/share/doc
	cp psxcdmaker.doc.bz2 $(prefix)/share/doc
