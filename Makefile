include config.mk

NASMSOURCES +=loader.asm
SOURCEDIRS=kernel lib mm io fs drivers arch/$(ARCH) tests
CSOURCES += $(filter-out $(wildcard apps/*.*),$(wildcard */*.c))
ASMSOURCES +=$(filter-out $(wildcard apps/*.*),$(wildcard */*.S))
NASMSOURCES +=$(wildcard */*.asm)
OBJS=$(notdir $(CSOURCES:%.c=%.o) $(ASMSOURCES:%.S=%.o) $(NASMSOURCES:%.asm=%.o))
DEPS=$(OBJS:.o=.d)

# .PHONY: all clean

CSOURCES+=$(wildcard */$(ARCH)/*.c)
ASMSOURCES+=$(wildcard */$(ARCH)/*.S)
NASMSOURCES+=$(wildcard */$(ARCH)/*.asm)

APPS_BINARIES=$(foreach name,$(APPS),$(APPS_INSTALLDIR)/$(name).bin)

ifndef TARGET_PROGRESS
HIT_TOTAL != ${MAKE} ${MAKECMDGOALS} --dry-run TARGET_PROGRESS="HIT_MARK" | grep -c "HIT_MARK"
HIT_COUNT = $(eval HIT_N != expr ${HIT_N} + 1)${HIT_N}
TARGET_PROGRESS = echo Targets remaining [${HIT_COUNT}/${HIT_TOTAL}]
endif


.PHONY: all hostinfo
all: hostinfo test

hostinfo:
	@echo Building $(OSNAME) v$(OSVER)
	@echo Hosted on $(BUILDOS)
	@echo

.PHONY: format clean

format:
	find -iname "*.h" -o -iname "*.c" | xargs clang-format -i

clean:
	@rm -f build/*.o
	@rm -f build/*.bin
	@rm -f $(OUTBIN)
	@rm -f $(INITRD_FILE)
	@rm -f $(KERNEL_MAP_FILE)

.PHONY: build checks kernel

build: checks kernel $(KERNEL_MAP_FILE) $(INITRD_FILE)

checks:
kernel: $(OUTBIN)

vpath %.c $(subst $(eval) ,:,$(SOURCEDIRS))
vpath %.S $(subst $(eval) ,:,$(SOURCEDIRS))
vpath %.asm .:$(subst $(eval) ,:,$(SOURCEDIRS))
$(BUILDDIR)/%.o: %.c
	@echo CC $< $@
ifeq ($(DEBUG),$(filter $(DEBUG),on true))
	@$(CC) $(CEMU) -std=c$(CSTD) $(CCFLAGS) -g -c $(BUILDDEFS) -o $@ $<
else
	@$(CC) $(CEMU) -std=c$(CSTD) $(CCFLAGS) -c $(BUILDDEFS) -o $@ $<
endif
	@$(TARGET_PROGRESS)

$(BUILDDIR)/%.o: %.S
	@echo CC $< $@
ifeq ($(DEBUG),$(filter $(DEBUG),on true))
	@$(CC) $(CEMU) -std=c$(CSTD) $(CCFLAGS) -g -c $(BUILDDEFS) -o $@ $<
else
	@$(CC) $(CEMU) -std=c$(CSTD) $(CCFLAGS) -c $(BUILDDEFS) -o $@ $<
endif
	@$(TARGET_PROGRESS)

$(BUILDDIR)/%.o: %.asm
	@echo ASM $<
	@$(ASM) -o $@ $<
	@$(TARGET_PROGRESS)

-include $(addprefix $(BUILDDIR)/,$(DEPS))

$(BUILDDIR)/:
	@mkdir -p $(BUILDDIR)

$(OUTBIN): $(addprefix $(BUILDDIR)/,$(OBJS)) | $(BUILDDIR)/
	@echo ---------- build kernel -----------
	$(LD) -o $(BUILDDIR)/$(OUTBIN).bin $(LDEMU) -T$(LDFILE) -O2 -nostdlib -g $(BUILDDIR)/*.o --build-id=none
	cp build/$(OUTBIN).bin $(OUTBIN)

$(KERNEL_MAP_FILE): $(OUTBIN)
	nm --demangle=gnu-v3 -n $(OUTBIN) > $(KERNEL_MAP_FILE)

.PHONY: apps
apps:
	@$(MAKE) -C apps DESTDIR=$(APPS_ROOTPATH)/$(APPS_INSTALLDIR) BUILDDIR=$(APPS_BUILDDIR)


# kernel-map: $(KERNEL_MAP_FILE)
$(INITRD_FILE): $(KERNEL_MAP_FILE) $(foreach name,$(APPS),$(APPS_INSTALLDIR)/$(name).bin) $(findstring apps,$(MAKECMDGOALS)) 
	python3 tools/initrd.py $(INITRD_ROOT)

.PHONY: iso
iso: $(OUTBIN) $(INITRD_FILE)
	cp DepthOS-1.0 iso/boot/
	cp initrd.img iso/boot/

.PHONY: test
test: build $(OUTBIN) $(INITRD_FILE)
	@echo
	@echo ----------- testing os ------------
	@echo
	qemu-system-i386 -M pc-i440fx-2.8 -kernel $(OUTBIN) -initrd $(INITRD_FILE) $(QEMU_ARGS) # -d int,pcall
	@# -d int,pcall,cpu,fpu -D qemu_log.log # -S -s # -nographic

.PHONY: hexdump elfinfo objdump info
hexdump:
	@echo ---------- HEX INFO ----------
	@echo Loader hexdump
	hexdump -x build/loader.o
	@echo Kernel hexdump
	hexdump -x $(OUTBIN)

elfinfo:
	@echo ---------- ELF INFO ----------
	readelf -l $(OUTBIN)

objdump:
	@echo ---------- OBJ INFO ----------
	@echo Loader object info
	objdump -f -h build/loader.o
	@echo Kernel object info
	objdump -f -h $(OUTBIN)

# dis_asm:
# 	@echo ---------- DIS ASM ----------
# 	@echo loader disasm
# 	ndisasm -b 32 build/loader.o
# 	@echo kernel disasm
# 	ndisasm -b 32 $(OUTBIN)

info: hexdump objdump elfinfo # dis_asm
