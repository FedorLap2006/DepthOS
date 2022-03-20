
ifeq ($(OS),Windows_NT)
	BUILDOS ?= win
else
	BUILDOS ?= nix
endif
ARCH?=x86
DEBUG?=on
OSVER?=1.0
OSNAME?=DepthOS
BINCPATH?=/bin
ifeq ($(BUILDOS),win)
	CC=$(BINCPATH)/i686-elf-gcc
	LD=$(BINCPATH)/i686-elf-ld
else
	CC=$(BINCPATH)/gcc
	LD=$(BINCPATH)/ld
endif
ASM=nasm -f elf32
CSTD=11
CEMU=-m32

CCFLAGS = -Iinclude -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-exceptions -fno-leading-underscore -fno-pic
CCFLAGS += -W -Wall -Wno-unused-parameter -Wno-type-limits -Wno-parentheses -Wno-unused-variable -Wno-maybe-uninitialized -Wno-return-local-addr -Wno-return-type

ASFLAGS = -m32
ifeq ($(BUILDOS),win)
	LDEMU=-melf_i386
endif
LDFILE=link.ld

OUTBIN=$(OSNAME)-$(OSVER)
# CSOURCES ?=
# ASMSOURCES ?=
# NASMSOURCES ?=
CSOURCES +=kmain.c
CSOURCES +=shell.c
NASMSOURCES +=loader.asm
#CSOURCES += $(shell find . -name "*.c" -type f -print )
#ASMSOURCES += $(shell find . -name "*.s" -type f -print )
#NASMSOURCES += $(shell find . -name "*.asm" -type f -print )
CSOURCES +=$(wildcard */*.c)
ASMSOURCES +=$(wildcard */*.S)
ASMSOURCES +=$(wildcard */*.s)
NASMSOURCES +=$(wildcard */*.asm)
# OBJS=build/*.o
# .PHONY: all clean

CSOURCES+=$(wildcard */$(ARCH)/*.c)
ASMSOURCES+=$(wildcard */$(ARCH)/*.S)
ASMSOURCES+=$(wildcard */$(ARCH)/*.s)
NASMSOURCES+=$(wildcard */$(ARCH)/*.asm)

all: os_info clean build test
	

os_info:
	@echo ---------- build $(OSNAME) ----------
	@echo -------- os version is $(OSVER) --------
	@echo ---------- build for $(BUILDOS) ----------
	@echo

format:
	find -iname "*.h" -o -iname "*.c" | xargs clang-format -i

clean:
	@rm -f build/*.o
	@rm -f build/*.bin
	@rm -f $(OSNAME)-*


build: checks kernel img iso

checks:
kernel: $(CSOURCES) $(ASMSOURCES) $(NASMSOURCES) $(LDFILE)
	@echo ---------- build kernel -----------
ifeq ($(DEBUG),$(filter $(DEBUG),on true))
	$(CC) $(CEMU) -std=c$(CSTD) -g -c -DOSVER=\"$(OSVER)\" -DKLOG_ENABLED=1 $(CSOURCES) $(CCFLAGS)
else
	$(CC) $(CEMU) -std=c$(CSTD) -c -DOSVER=\"$(OSVER)\" $(CSOURCES) $(CCFLAGS)
endif
	@mkdir -p build
	@mv *.o build/
	$(ASM) $(NASMSOURCES)
	$(CC) $(CEMU) -c $(ASMSOURCES)
	@mv *.o build/
	$(LD) $(LDEMU) -T$(LDFILE) -O2 -nostdlib -g -ggdb -o build/$(OUTBIN).bin build/*.o --build-id=none 
	cp build/$(OUTBIN).bin $(OUTBIN)

img:

iso:

hex_info:
	@echo ---------- HEX INFO ----------
	@echo loader hex info
	hexdump -x build/loader.o
	@echo kernel hex info
	hexdump -x $(OUTBIN)
elf_info:
	@echo ---------- ELF INFO ----------
	readelf -l $(OUTBIN)
#dis_asm:
#	@echo ---------- DIS ASM ----------
#	@echo loader disasm
#	ndisasm -b 32 build/loader.o
#	@echo kernel disasm
#	ndisasm -b 32 $(OUTBIN)
obj_info:
	@echo ---------- OBJ INFO ----------
	@echo loader obj info
	objdump -f -h build/loader.o
	@echo kernel obj info
	objdump -f -h $(OUTBIN)
info: hex_info obj_info elf_info # dis_asm

test:
	@echo
	@echo ----------- testing os ------------
	@echo
ifeq ($(DEBUG), on)
	qemu-system-i386 -M pc-i440fx-2.8 -kernel $(OUTBIN) $(QEMU_ARGS) # -S -s # -nographic
else ifeq ($(DEBUG), true)
	qemu-system-i386 -M pc-i440fx-2.8 -kernel $(OUTBIN) $(QEMU_ARGS) # -S -s # -nographic
else
	qemu-system-i386 -M pc-i440fx-2.8 -kernel $(OUTBIN) $(QEMU_ARGS) # -d int,pcall,cpu,fpu -D qemu_log.log # -S -s # -nographic
endif
