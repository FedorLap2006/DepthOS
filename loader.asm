; GRUB MULTIBOOT 2
MAGIC equ 0x1BADB002
MEMINFO equ 1<<1
MBALIGN equ 1<<0
FLAGS equ MBALIGN | MEMINFO

STACK_SIZE equ 600

align 4

bits 32

section .bss
stack_end:
	resb STACK_SIZE
stack_top:
section .multiboot

grubBoot:
	dd MAGIC
	dd FLAGS
	dd (end_grubBoot - grubBoot)
	dd -(MAGIC+FLAGS+(end_grubBoot - grubBoot))
end_grubBoot:
section .text

global _loadkernel

extern _kmain


_loadkernel:
	finit
	mov esp,stack_top
	
	push ebx
	push eax

	call _kmain

	cli
	hlt

