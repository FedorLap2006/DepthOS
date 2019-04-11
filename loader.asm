; GRUB MULTIBOOT 2
MAGIC equ 0x1BADB002
MEMINFO equ 1<<1
FLAGS equ MEMINFO ; | ... | ...
; CHECKSUM equ -(MAGIC+FLAGS)


STACK_SIZE equ 600

section .bss
align 16
stack_end:
	resb STACK_SIZE
stack_start:
section .multiboot
align 4
grubBoot:
	dd MAGIC
	dd FLAGS
	dd end_grubBoot - grubBoot
	dd -(MAGIC + FLAGS + (end_grubBoot - grubBoot))

end_grubBoot:
section .text

global _loadkernel

extern _kmain

_loadkernel:
	finit
	mov esp,stack_start
	
	push ebx
	push eax
	

	call _kmain
	
	cli
	hlt
