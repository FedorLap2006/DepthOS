all: kernel

kernel: kmain.c loader.asm GRUBloader.asm
	gcc -m32 -S kmain.c -Ilibs -ffreestanding -nostdlib -nostdinc
	gcc -m32 -c kmain.s -o build/kmain.o
	nasm -f elf32 GRUBloader.asm -o build/gloader.o
	nasm -f elf32 loader.asm -o build/loader.o
	ld -m i386pe -T link.ld -o kernel.bin build/kmain.o build/loader.o build/gloader.o -build-id=none
#	objcopy -O elf32-i386 kernel kernel.elf
	objcopy -O pe-i386 kernel.bin kernel
