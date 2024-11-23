CC?=gcc
CEMU?=-m32

BUILDDIR?=build
DESTDIR?=../disk-fs
APPS=init

$(BUILDDIR)/init: mdinit.c
	$(CC) $(CEMU) -std=c11 -ggdb -o $(BUILDDIR)/init mdinit.c

$(DESTDIR)/bin/init: $(BUILDDIR)/init
	install -p $(BUILDDIR)/init $(DESTDIR)/bin

.PHONY: init
init: $(DESTDIR)/bin/init

install: $(APPS)

