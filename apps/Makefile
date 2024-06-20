BUILDDIR?=build
DESTDIR?=bin
CCFLAGS=$(CEMU) -std=c11 $(APPS_CCFLAGS)
# .PHONY: all apps install
# APPS=init.bin test-gcc.bin nyancat.bin cat.bin donut.bin
# APPS=init.bin nyancat.bin donut.bin inputtest.bin test-gcc.bin
APPS=mdinit nyancat sound-beep png-viewer wav-player

all: install

### APPS ###

$(BUILDDIR)/cat.bin: cat.c
	$(CC) $(CEMU) -std=c11 -ggdb -o $(BUILDDIR)/cat.bin cat.c

$(BUILDDIR)/nyancat.bin: nyancat.c
	$(CC) $(CEMU) -std=c11 -ggdb -o $(BUILDDIR)/nyancat.bin nyancat.c

$(BUILDDIR)/donut.bin: donut.c
	$(CC) $(CEMU) -std=c11 -ggdb -o $(BUILDDIR)/donut.bin donut.c

$(BUILDDIR)/inputtest.bin: inputtest.c
	$(CC) $(CEMU) -std=c11 -ggdb -o $(BUILDDIR)/inputtest.bin inputtest.c

$(BUILDDIR)/mdinit.bin: mdinit.c
	$(CC) $(CEMU) -std=c11 -ggdb -o $(BUILDDIR)/mdinit.bin mdinit.c

$(BUILDDIR)/sound-beep.bin: sound-beep.c
	$(CC) $(CCFLAGS) -o $(BUILDDIR)/sound-beep.bin sound-beep.c

$(BUILDDIR)/png-viewer.bin: png-viewer.c
	$(CC) $(CCFLAGS) -o $(BUILDDIR)/png-viewer.bin png-viewer.c -lpng -lz 

$(BUILDDIR)/wav-player.bin: wav-player.c
	$(CC) -std=c11 wav-player.c -o $(BUILDDIR)/wav-player.bin


### GENERIC TARGETS ###

$(DESTDIR)/%: $(BUILDDIR)/%.bin
	# echo $(DESTDIR) $(BUILDDIR)
	install -p $< $@

build: $(BUILDDIR)/ $(addprefix $(BUILDDIR)/,$(foreach name,$(APPS),$(name).bin))
$(BUILDDIR)/:
	mkdir -p $(BUILDDIR)

install: build $(addprefix $(DESTDIR)/,$(APPS))

clean:
	@rm -rf $(BUILDDIR)/*


