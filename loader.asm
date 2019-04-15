; GRUB MULTIBOOT 2
MAGIC equ 0x1BADB002
MEMINFO equ 1<<1
MBALIGN equ 1<<0
FLAGS equ 0 | MBALIGN | MEMINFO

STACK_SIZE equ 600

bits 32

section .bss

align 4

stack_end:
	resb STACK_SIZE
stack_top:

section .boot

align 4

grubBoot:
	dd MAGIC
	dd FLAGS
	dd 0
	dd (end_grubBoot - grubBoot)
	dd -(MAGIC + FLAGS + (end_grubBoot - grubBoot))
;	dd -(MAGIC + 0 + (end_grubBoot - grubBoot))
	
	dw 0
	dw 0
	dd 8
end_grubBoot:

section .text

align 4

global _loader
; global _loadkernel

extern kmain

_loadkernel:
	finit
	sti
	mov esp,stack_top
	
	push ebx
	push eax

	call kmain

.stop:
	cli
	hlt
	jmp .stop

_loader:
	jmp 08h:_loadkernel ; protected mode
