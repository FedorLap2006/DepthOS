ARCH=i386

QEMU=qemu-system-i386

LDFLAGS = -T link.ld -o kernel.elf

CC=/bin/x86_64-pc-cygwin-gcc -m32 -std=c11

ASM=nasm -f elf32

BU-TARGET=i686-elf
BU-ROOT="cross/gcc"

all: kernel

bin-u:
	mkdir -p cross
	
	
cross-c:
	

kernel: kmain.c loader.asm GRUBloader.asm
	$(CC) kmain.c -Ilibs -ffreestanding -nostdlib -nostdinc -o build/kmain.o
		
	$(ASM) GRUBloader.asm -o build/gloader.o
	
	$(ASM) -f elf32 loader.asm -o build/loader.o
	
	$(CC) $(LDFLAGS) build/*.o -ffreestanding -nostdlib -nostdinc -o kernel.elf
	


test:
	$(QEMU) -kernel kernel.elf
#	ld-i386 -m elf_i386 -T link.ld -o kernel.bin build/kmain.o build/loader.o build/gloader.o -build-id=none
#	objcopy -O elf32-i386 kernel.bin kernel
#	objcopy -O pe-i386 kernel.bin kernel
