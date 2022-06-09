$(BUILDDIR)/init.bin: init.c

	$(CC) -std=c11 -o $(BUILDDIR)/init.bin init.c -ffreestanding -nostdlib
