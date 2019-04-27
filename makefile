
ifeq ($(OS),Windows_NT)
	BUILDOS ?= win
else
	BUILDOS ?= nix
endif
ARCH?=x86
DEBUG?=on
OSVER?=1.0
OSNAME?=DepthOS
CC=gcc
LD=ld
ASM=nasm -f elf32
CSTD=11
CEMU=-m32
CCFLAGS= -Iinclude -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-exceptions -fno-leading-underscore -fno-pic
ASFLAGS = -m32
ifeq ($(BUILDOS),win)
	LDEMU=-mi386pe
else
	LDEMU=-melf_i386
endif
LDFILE=link.ld

OUTBIN=$(OSNAME)-$(OSVER)
# CSOURCES ?=
# ASMSOURCES ?=
# NASMSOURCES ?=
CSOURCES +=kmain.c
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

clean:
	@rm -f build/*.o
	@rm -f build/*.bin
	@rm -f $(OSNAME)-*


build: kernel img iso


kernel: $(CSOURCES) $(ASMSOURCES) $(NASMSOURCES) $(LDFILE)
	@echo ---------- build kernel -----------
ifeq ($(DEBUG),on)
	$(CC) $(CEMU) -std=c$(CSTD) -W -Wall -g -c -DOSVER=\"$(OSVER)\" $(CSOURCES) $(CCFLAGS)
ifeq ($(DEBUG),true)
	$(CC) $(CEMU) -std=c$(CSTD) -W -Wall -g -c -DOSVER=\"$(OSVER)\" $(CSOURCES) $(CCFLAGS)
endif
else
	$(CC) $(CEMU) -std=c$(CSTD) -W -Wall -c -DOSVER=\"$(OSVER)\" $(CSOURCES) $(CCFLAGS)
endif
	@mv *.o build/
	$(ASM) $(NASMSOURCES)
	gcc -m32 -c $(ASMSOURCES)
	@mv *.o build/
	$(LD) $(LDEMU) --nmagic -T$(LDFILE) -o build/$(OUTBIN).bin build/*.o
ifeq ($(BUILDOS),win)	
	objcopy -O elf32-i386 build/$(OUTBIN).bin $(OUTBIN)
else
	cp build/$(OUTBIN).bin $(OUTBIN)
endif

img:

iso:

hex_info:
	@echo ---------- HEX INFO ----------
	@echo loader hex info
	hexdump -x build/loader.o
	@echo kernel hex info
	hexdump -x $(OUTBIN)
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
info: hex_info obj_info # dis_asm

test:
	@echo
	@echo ----------- testing os ------------
	@echo
	qemu-system-i386 -M pc-i440fx-2.8 -kernel $(OUTBIN) # -d cpu -D qemu_log.log # -d int,pcall,cpu,fpu # -nographic

