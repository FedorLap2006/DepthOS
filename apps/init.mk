$(BUILDDIR)/init.bin: init.c
	$(CC) $(CEMU) -std=c11 -o $(BUILDDIR)/init.bin init.c -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-exceptions -fno-leading-underscore -fno-pic

