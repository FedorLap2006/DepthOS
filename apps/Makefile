BUILDDIR?=build
DESTDIR?=bin
# .PHONY: all apps install
APPS=init.bin nyancat.bin donut.bin

all: install
$(BUILDDIR)/autoload2-t.bin: helloworld2.S
	$(CC) $(CEMU) -std=c11 -o $(BUILDDIR)/autoload2-t.bin helloworld2.S -MMD -MP -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-exceptions -fno-leading-underscore -fno-pic

$(BUILDDIR)/nyancat.bin: nyancat.c
	$(CC) $(CEMU) -std=c11 -ggdb -o $(BUILDDIR)/nyancat.bin nyancat.c

$(BUILDDIR)/donut.bin: donut.c
	$(CC) $(CEMU) -std=c11 -ggdb -o $(BUILDDIR)/donut.bin donut.c


include init.mk
$(DESTDIR)/%.bin: $(BUILDDIR)/%.bin
	echo $(DESTDIR) $(BUILDDIR)
	install -p $< $@

build: $(BUILDDIR)/ $(addprefix $(BUILDDIR)/,$(APPS))
$(BUILDDIR)/:
	mkdir -p $(BUILDDIR)

install: build $(addprefix $(DESTDIR)/,$(APPS))

clean:
	@rm -rf $(BUILDDIR)/*
