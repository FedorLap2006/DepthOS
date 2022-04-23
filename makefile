on_check=$(filter $(1),on true 1)

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
INITRD_FILE?=initrd.img
INITRD_ROOT=./initrd
KERNEL_MAP_FILE?= $(INITRD_ROOT)/kernel.map
QEMU_ARGS=
QEMU_DEBUG?=false
ifeq ($(QEMU_DEBUG),$(call on_check,$(QEMU_DEBUG)))
	QEMU_ARGS += -s -S
endif
QEMU_APPEND?=
ifeq ($(QEMU_TTY),$(call on_check,$(QEMU_TTY)))
	QEMU_APPEND += console=ttyS0
	QEMU_ARGS += -monitor none -serial stdio
endif
ifeq ($(NO_COLOR),$(call on_check,$(NO_COLOR)))
	QEMU_APPEND += console_no_color
endif
QEMU_ARGS += -append "$(QEMU_APPEND)"


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
NASMSOURCES +=loader.asm
#CSOURCES += $(shell find . -name "*.c" -type f -print )
#ASMSOURCES += $(shell find . -name "*.s" -type f -print )
#NASMSOURCES += $(shell find . -name "*.asm" -type f -print )
CSOURCES +=$(filter-out $(wildcard apps/*.*),$(wildcard */*.c))
ASMSOURCES +=$(filter-out $(wildcard apps/*.*),$(wildcard */*.S))
ASMSOURCES +=$(filter-out $(wildcard apps/*.*),$(wildcard */*.s))
NASMSOURCES +=$(wildcard */*.asm)
# OBJS=build/*.o
# .PHONY: all clean

CSOURCES+=$(wildcard */$(ARCH)/*.c)
ASMSOURCES+=$(wildcard */$(ARCH)/*.S)
ASMSOURCES+=$(wildcard */$(ARCH)/*.s)
NASMSOURCES+=$(wildcard */$(ARCH)/*.asm)

KCONFIG_LOG_ENABLE?=1

KCONFIG_DEF=-DOSVER=\"$(OSVER)\" 
ifeq ($(KCONFIG_LOG_ENABLE),$(call on_check,$(KCONFIG_LOG_ENABLE)))
	KCONFIG_DEF+=-DKLOG_ENABLED=1
endif


.PHONY: build
.PHONY: clean
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
	@rm -f $(INITRD_FILE)


build: checks apps kernel initrd

checks:
kernel: $(OUTBIN) 
$(OUTBIN): $(CSOURCES) $(NASMSOURCES) $(ASMSOURCES) $(LDFILE) 
	@echo ---------- build kernel -----------
ifeq ($(DEBUG),$(filter $(DEBUG),on true))
	$(CC) $(CEMU) -std=c$(CSTD) -g -c $(KCONFIG_DEF) $(CSOURCES) $(ASMSOURCES) $(CCFLAGS)
else
	$(CC) $(CEMU) -std=c$(CSTD) -c $(KCONFIG_DEF) $(CSOURCES) $(CCFLAGS)
endif
	@mkdir -p build
	@mv *.o build/
	$(ASM) $(NASMSOURCES)
	@mv *.o build/
	$(LD) $(LDEMU) -T$(LDFILE) -O2 -nostdlib -g -ggdb -o build/$(OUTBIN).bin build/*.o --build-id=none 
	cp build/$(OUTBIN).bin $(OUTBIN)

kernel-map: $(KERNEL_MAP_FILE)
$(KERNEL_MAP_FILE): $(OUTBIN)
	nm --demangle=gnu-v3 -n $(OUTBIN) > $(KERNEL_MAP_FILE)

iso: $(OUTBIN) $(INITRD_FILE)
	cp DepthOS-1.0 iso/boot/
	cp initrd.img iso/boot/

initrd: $(INITRD_FILE)
$(INITRD_FILE): $(INITRD_ROOT)/
	python3 tools/initrd.py $(INITRD_ROOT)

test: $(OUTBIN) $(INITRD_FILE) 
	@echo
	@echo ----------- testing os ------------
	@echo
	qemu-system-i386 -M pc-i440fx-2.8 -kernel $(OUTBIN) -initrd $(INITRD_FILE) $(QEMU_ARGS)
	@# -d int,pcall,cpu,fpu -D qemu_log.log # -S -s # -nographic


apps: initrd/autoload.bin
initrd/autoload.bin: apps/helloworld.S
	$(CC) $(CEMU) -std=c$(CSTD) -o initrd/autoload.bin apps/helloworld.S  -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-exceptions -fno-leading-underscore -fno-pic

hex_info:
	@echo ---------- HEX INFO ----------
	@echo loader hex info
	hexdump -x build/loader.o
	@echo kernel hex info
	hexdump -x $(OUTBIN)

elf_info:
	@echo ---------- ELF INFO ----------
	readelf -l $(OUTBIN)

obj_info:
	@echo ---------- OBJ INFO ----------
	@echo loader obj info
	objdump -f -h build/loader.o
	@echo kernel obj info
	objdump -f -h $(OUTBIN)

# dis_asm:
# 	@echo ---------- DIS ASM ----------
# 	@echo loader disasm
# 	ndisasm -b 32 build/loader.o
# 	@echo kernel disasm
# 	ndisasm -b 32 $(OUTBIN)

info: hex_info obj_info elf_info # dis_asm

