ifeq ($(OS),Windows_NT)
	BUILDOS ?= win
else
	BUILDOS ?= nix
endif

OS_NAME = DepthOS
OS_VERSION = 1.0\(Beta\)
ARCH ?= x86
OUTBINR ?= .
OUTBIN = $(OUTBINR)/$(OS_NAME)-$(OS_VERSION)
DEBUG ?= -g

BINCPATH ?= /bin
FROOT ?= ./

BUILDDIR ?= $(FROOT)build

ifeq ($(BUILDOS),win)
	CC=$(BINCPATH)/i686-elf-gcc
	LD=$(BINCPATH)/i686-elf-ld
	# ASM=$(BINCPATH)/i686-elf-as
else
	CC=$(BINCPATH)/gcc
	LD=$(BINCPATH)/ld
endif
ASM=/bin/nasm

CCSTD?=11
LDEMU?=elf_i386
CEMU?=32

LDEPF ?= $(FROOT)/env/linker.ld

# ASMFLAGS=-m$(CEMU)
ASMFLAGS=-f elf32 $(DEBUG)
CCFLAGS = -m$(CEMU) -std=c$(CCSTD) -I$(FROOT)inc -I$(FROOT)mdls $(DEBUG)
CCFLAGS += -c -Iinclude -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-exceptions -fno-leading-underscore -fno-pic
CCFLAGS += -W -Wall -Wno-unused-parameter -Wno-type-limits -Wno-parentheses -Wno-unused-variable -Wno-maybe-uninitialized -Wno-return-local-addr -Wno-return-type
LDFLAGS = -m$(LDEMU) -T$(LDEPF)

# CSOURCES=$(wildcard $(FROOT)*/*.c)
# CSOURCES+=$(wildcard $(FROOT)*/*.cc)
# ASMSOURCES=$(wildcard $(FROOT)*/*.asm)
# CSOURCES+=$(wildcard $(FROOT)*/$(ARCH)/*.c)
# CSOURCES+=$(wildcard $(FROOT)*/$(ARCH)/*.cc)
# ASMSOURCES+=$(wildcard $(FROOT)*/$(ARCH)/*.asm)
# CSOURCES+=$(wildcard $(FROOT)mods/*/$(ARCH)/*.c)
# CSOURCES+=$(wildcard $(FROOT)mods/*/$(ARCH)/*.cc)
# ASMSOURCES+=$(wildcard $(FROOT)mods/*/$(ARCH)/*.asm)
# CSOURCES+=$(wildcard $(FROOT)drivers/*/$(ARCH)/*.c)
# CSOURCES+=$(wildcard $(FROOT)drivers/*/$(ARCH)/*.cc)
# ASMSOURCES+=$(wildcard $(FROOT)drivers/*/$(ARCH)/*.asm)
# CSOURCES+=$(wildcard $(FROOT)mods/*/*.c)
# CSOURCES+=$(wildcard $(FROOT)mods/*/*.cc)
# ASMSOURCES+=$(wildcard $(FROOT)mods/*/*.asm)
# CSOURCES+=$(wildcard $(FROOT)drivers/*/*.c)
# CSOURCES+=$(wildcard $(FROOT)drivers/*/*.cc)
# ASMSOURCES+=$(wildcard $(FROOT)drivers/*/*.asm)
# CSOURCES=$(wildcard $(FROOT)kernel/*/*.c)
# CSOURCES+=$(wildcard $(FROOT)kernel/*/*.cc)
CSOURCES = $(shell find .. -name "*.c" -o -name "*.cc")
ASMSOURCES = $(shell find .. -name "*.asm" -o -name "*.as")



TESTING_EMU ?= qemu
TESTING_EMU_ARCH?=i386
TESTING_EMU_BIN ?= $(TESTING_EMU)-system-$(TESTING_EMU_ARCH)
TESTING_EMU_CONFIG=	-M pc-i440fx-2.8 -kernel $(OUTBIN)

.PHONY: clean
all: os_info build testing

os_info:
	@echo ---------- project $(OS_NAME) ----------
# 	@echo -------- os version is $(OS_VERSION) --------
# 	@echo ---------- build for $(BUILDOS) ----------
# 	@echo

clean:
	@echo ---------- clean $(OS_NAME) ----------
	@rm -f build/*.o
	@rm -f build/*.bin
	@rm -f $(OS_NAME)-*


%.o: %.c
	@echo $(CC) $(CCFLAGS) -DOSVER=\"$(OS_VERSION)1\" $< -o $@
	$(CC) $(CCFLAGS) -DOSVER=\"$(OS_VERSION)1\" $< -o $@
%.o: %.asm
	@echo $(ASM) $(ASMFLAGS) $<
	$(ASM) $(ASMFLAGS) $<

build: $(CSOURCES:.c=.o) $(ASMSOURCES:.asm=.o)
	@echo ---------- building $(OS_NAME) ------------
	$(LD) $(LDFLAGS) -o $(BUILDDIR)/$(OUTBIN).bin $(CSOURCES:.c=.o) $(ASMSOURCES:.asm=.o) --build-id=none
	@cp $(BUILDDIR)/$(OUTBIN).bin $(OUTBIN)
#	 ifeq ($(DEBUG),on)
# 	$(CC) $(CCFLAGS) -g -DOSVER=\"$(OS_VERSION)\" $(CSOURCES)
# else
# 	$(CC) $(CCFLAGS) -DOSVER=\"$(OS_VERSION)\" $(CSOURCES)
# endif
# 		@mkdir -p $(BUILDDIR)
# 		@mv *.o $(BUILDDIR)
#		$(CC) $(ASMFLAGS) -c $(ASMSOURCES)
#		@mv *.o $(BUILDDIR)
# 		$(foreach SOURCE, $(ASMSOURCES), $(ASM) $(ASMFLAGS) -o $(SOURCE:.asm=.o) $(SOURCE))


testing:
	@echo --------- testing $(OS_NAME) ------------
	$(TESTING_EMU_BIN) $(TESTING_EMU_CONFIG)






