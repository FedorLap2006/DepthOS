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
FS_ROOT=disk-fs
KERNEL_MAP_FILE?=$(INITRD_ROOT)/kernel.map
QEMU_ARGS=-nographics -display none
# QEMU_ARGS=-no-reboot
QEMU_DEBUG?=false
ifeq ($(QEMU_DEBUG),$(call on_check,$(QEMU_DEBUG)))
	QEMU_ARGS += -s -S
	QEMU_TTY=on
endif
QEMU_APPEND?=
ifeq ($(QEMU_TTY),$(call on_check,$(QEMU_TTY)))
	QEMU_APPEND += console=ttyS0
	QEMU_ARGS += -monitor none -serial stdio
endif
ifeq ($(NO_COLOR),$(call on_check,$(NO_COLOR)))
	QEMU_APPEND += console_no_color
	
endif
# QEMU_APPEND += console_no_color shutdown_on_panic

QEMU_ARGS += -append "$(QEMU_APPEND)" -m 4G


CC?=$(BINCPATH)/gcc
LD?=$(BINCPATH)/ld
ASM?=nasm
export CC
export LD
export ASM


CSTD=11
export CEMU=-m32
CCFLAGS  = -Iinclude -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-exceptions -fno-leading-underscore -fno-pic -MP -MD -fno-pie -fno-PIC
# TODO: resolve int-to-pointer-cast in list.h (to and from macros)
CCFLAGS += -W -Wall -Wno-unused-parameter -Wno-type-limits -Wno-parentheses -Wno-unused-variable -Wno-maybe-uninitialized -Wno-return-local-addr -Wno-return-type -Wno-int-to-pointer-cast
ASFLAGS  = -m32



NASMFLAGS= -f elf32

export LDEMU=-melf_i386
LDFILE=link.ld
LDFLAGS=-L
OUTBIN=$(OSNAME)-v$(OSVER)-$(ARCH)

BUILDDEFS=-DOSVER=\"$(OSVER)\" -DCONFIG_EMULATOR # -DCONFIG_EMULATOR_QEMU
ifeq ($(DEBUG), $(call on_check,$(DEBUG)))
	BUILDDEFS += -DDEBUG
endif
# APPS=init nyancat donut # test-gcc cat
# APPS=init donut # test-gcc
APPS=mdinit nyancat music-player
# INITRD_APPS=mdinit

APPS_BUILDDIR=build
APPS_ROOTPATH=..
# APPS_INSTALLDIR=$(INITRD_ROOT)
APPS_INSTALLDIR=$(FS_ROOT)/bin
