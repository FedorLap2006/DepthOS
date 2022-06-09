BUILDDIR?=build
DESTDIR?=bin
# .PHONY: all apps install
APPS=init.bin autoload2-t.bin

all: install
$(BUILDDIR)/autoload2-t.bin: helloworld2.S
	$(CC) $(CEMU) -std=c11 -o $(BUILDDIR)/autoload2-t.bin helloworld2.S -MMD -MP -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-exceptions -fno-leading-underscore -fno-pic

include init.mk
$(DESTDIR)/%.bin: $(BUILDDIR)/%.bin
	install -p $< $@

build: $(BUILDDIR)/ $(addprefix $(BUILDDIR)/,$(APPS))
$(BUILDDIR)/:
	mkdir -p $(BUILDDIR)

install: build $(addprefix $(DESTDIR)/,$(APPS))

clean:
	@rm -rf $(BUILDDIR)/*
