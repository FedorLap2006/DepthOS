CC=gcc
LD=ld
ASM=nasm -f elf32
CSTD=11
CEMU=-m32
LDEMU=i386pe # elf_i386
LDFILE=link.ld
OUTBIN=kernel
CSOURCES ?=
ASMSOURCES ?= 
NASMSOURCES ?= 
CSOURCES += $(shell find . -name "*.c" -type f -print )
ASMSOURCES += $(shell find . -name "*.s" -type f -print )
NASMSOURCES += $(shell find . -name "*.asm" -type f -print )


all: build test

build: kernel info

kernel:
	$(CC) $(CEMU) -std=c$(CSTD) -c $(CSOURCES) -ffreestanding -nostdlib -nostdinc -fno-pic
	$(ASM) $(NASMSOURCES)
	mv *.o build/
	$(LD) -m$(LDEMU) --nmagic -T$(LDFILE) -o build/$(OUTBIN).bin build/*.o
	objcopy -O elf32-i386 build/$(OUTBIN).bin $(OUTBIN)
hex_info:
	@echo --HEX INFO--
	@echo loader hex info
	hexdump -x build/loader.o
	@echo kernel hex info
	hexdump -x $(OUTBIN)
dis_asm:
	@echo --DIS ASM--
	@echo loader disasm
	ndisasm -b 32 build/loader.o
	@echo kernel disasm
	ndisasm -b 32 $(OUTBIN)
obj_info:
	@echo --OBJ INFO--
	@echo loader obj info
	objdump -f -h build/loader.o
	@echo kernel obj info
	objdump -f -h $(OUTBIN)
info: dis_asm hex_info obj_info

test:
	qemu-system-i386 -nographic -kernel $(OUTBIN)
clean: build/*.bin $(OUTBIN)
	rm build/*.o
	rm build/*.bin
	rm $(OUTBIN)
