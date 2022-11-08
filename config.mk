on_check=$(filter $(1),on true 1)

ifeq ($(OS),Windows_NT)
	BUILDOS ?= win
else
	BUILDOS ?= nix
endif
export BUILDOS
export ARCH?=x86
DEBUG?=on
OSVER?=1.0
OSNAME?=DepthOS
export BINCPATH?=/bin
BUILDDIR=build
INITRD_FILE?=initrd.img
INITRD_ROOT=initrd
KERNEL_MAP_FILE?=$(INITRD_ROOT)/kernel.map
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


CC?=$(BINCPATH)/gcc
LD?=$(BINCPATH)/ld
ASM?=nasm
export CC
export LD
export ASM

CSTD=11
export CEMU=-m32
CCFLAGS  = -Iinclude -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-exceptions -fno-leading-underscore -fno-pic -MP -MD
CCFLAGS += -W -Wall -Wno-unused-parameter -Wno-attribute-alias -Wno-type-limits -Wno-parentheses -Wno-unused-variable -Wno-maybe-uninitialized -Wno-return-local-addr -Wno-return-type
ASFLAGS  = -m32
NASMFLAGS= -f elf32
export LDEMU=-melf_i386
LDFILE=link.ld
OUTBIN=$(OSNAME)-v$(OSVER)-$(ARCH)

BUILDDEFS=-DOSVER=\"$(OSVER)\"
ifeq ($(DEBUG), $(call on_check,$(DEBUG)))
	BUILDDEFS += -DDEBUG
endif
APPS=init test-gcc
APPS_ROOTPATH=..
APPS_BUILDDIR=apps-build
APPS_INSTALLDIR=$(INITRD_ROOT)
