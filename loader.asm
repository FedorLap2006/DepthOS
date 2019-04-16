; GRUB MULTIBOOT 1
MAGIC equ 0x1BADB002 ; multiboot 2 - 0xe85250d6 ; multiboot 1 - 0x1BADB002
; ARCH equ 0 ; i386
MEMINFO equ 1<<1
MBALIGN equ 1<<0
FLAGS equ 0 | MBALIGN | MEMINFO
HLEN equ __boot_header_end - __boot_header
CHECKSUM equ 0x100000000 - (MAGIC + FLAGS + HLEN)

STACK_SIZE equ 600

bits 32

section .bss

align 4

stack_end:
	resb STACK_SIZE
stack_top:

section .boot

align 4

__boot_header:
	dd MAGIC
;	dd ARCH
	dd FLAGS
	dd -(MAGIC + FLAGS)
	dd HLEN
	dd CHECKSUM
	
;	dw 0
;	dw 0
;	dd 8
__boot_header_end:

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
