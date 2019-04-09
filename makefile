# binary specification
ARCH=i386
ASMFORMAT=elf32
BINFORMAT=elf_$(ARCH)
OUTBINARY=kernel.elf

# folders

CROSSFOLDER=cross
ENTRYFOLDER=kernel
BUILDFOLDER=build
MODSFOLDER=$(ENTRYFOLDER)/mods


# specification files

LOADER=$(ENTRYFOLDER)/loader.asm
SUPERLOADERTYPE=GRUB
SUPERLOADER=$(ENTRYFOLDER)/$(SUPERLOADERTYPE)loader.asm
ENTRYFILE=$(ENTRYFOLDER)/kmain.c
LINKFILE=$(ENTRYFOLDER)/link.ld


LDFLAGS = -T $(LINKFILE) -m $(BINFORMAT) -o $(OUTBINARY)

CCSTD=c11
CCFLAGS= -m32 -std=$(CCSTD) -ffreestanding -nostdlib -nostdinc

CC=gcc

ASM=nasm -f $(ASMFORMAT)


QEMU=qemu-system-i386

#BU-TARGET=i686-elf
#BU-ROOT="cross/gcc"

all: kernel


#build/%.o: libs/%.c
#	$(CC) -c $< -Iinclude -o 

loader: # loader/GRUBloader.asm loader/loader.asm
	$(ASM) $(SUPERLOADER) -o $(BUILDFOLDER)/$(SUPERLOADERTYPE)loader.o
	
	$(ASM) $(LOADER) -o $(BUILDFOLDER)/$(LOADER).o

kernel: loader
	$(CC) $(CCFLAGS) $(ENTRYFILE) fs/*.c arch/*.c kernel/*.c lib/*.c memory/*.c -Iinclude -Iinclude/x86 -Llib -Lmemory -Lfs -Larch -Larch/x86 -Lexec -Ldrivers -o $(BUILDFOLDER)/kernel/kmain.o

	# $(CC) $(CCFLAGS) $(LDFLAGS) $(BUILDFOLDER)/*.o -L$(LIBFOLDER) -o $(OUTBINARY)
	ld -T $(LINKFILE) -o pre_$(OUTBINARY) -build-id=none

	objcopy -O elf-i386 pre_$(OUTBINARY) $(OUTBINARY)
	rm pre_$(OUTBINARY)


test:
	$(QEMU) -kernel $(OUTBINARY)
#	ld-i386 -m elf_i386 -T link.ld -o kernel.bin build/kmain.o build/loader.o build/gloader.o -build-id=none
#	objcopy -O elf32-i386 kernel.bin kernel
#	objcopy -O pe-i386 kernel.bin kernel
