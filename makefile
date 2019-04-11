all: build test

build: kernel

kernel:
	gcc -m32 -std=c11 -c kmain.c -ffreestanding -nostdlib -nostdinc -o kmain.o
	nasm -f elf32 loader.asm -o loader.o
	ld -m i386pe -T link.ld -o kernel loader.o kmain.o -build-id=none
	objcopy -O pei-i386 kernel kernel
	objcopy -O pe-i386 kernel kernel.elf
test:
	#qemu-system-i386 -nographic -kernel kernel
	qemu-system-i386 -nographic -kernel kernel.elf
clean: kernel # kernel.bin
	rm *.o
	rm kernel
#	rm kernel.bin

