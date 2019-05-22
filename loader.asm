; GRUB MULTIBOOT 1
MAGIC equ 0x1BADB002 ; multiboot 2 - 0xe85250d6 ; multiboot 1 - 0x1BADB002
; ARCH equ 0 ; i386
MEMINFO equ 1<<1
MBALIGN equ 1<<0
FLAGS equ 0 | MBALIGN | MEMINFO
HLEN equ __boot_header_end - __boot_header
CHECKSUM equ 0x100000000 - (MAGIC + FLAGS + HLEN)
 
STACK_SIZE equ 60000 ;4096 * 1024 * 1024 + 400
 
; mem
 
VM_BASE equ 0xC0000000
PDE_INDEX   equ (VM_BASE >> 22)
PSE_BIT     equ 0x00000010
PG_BIT      equ 0x80000000
 
 
 
 
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
;       dd ARCH
	dd FLAGS
	dd -(MAGIC + FLAGS)
	dd HLEN
	dd CHECKSUM
 
;       dw 0
;       dw 0
;       dd 8
__boot_header_end:
 
section .text

global TEMP_PG_DIR
 
align 4096
TEMP_PG_DIR:
	; Map the first 4mb physical memory to first 4mb virtual memory. Otherwise, when paging is enabled, eip points to, 0x100004 for example, and MMU is not gonna know how to translate
    ; this address into phsyical mem address, because our PDE doesn't tell MMU how to find it.
    dd 0x00000083
    times(PDE_INDEX - 1) dd 0
    dd 0x00000083
    times(1024 - PDE_INDEX - 1) dd 0
 
align 4
 
global _loader
 
extern kmain
extern set_up_gdt
 
_loader:
    ; update page directory address, since eax and ebx is in use, have to use ecx or other register
    mov ecx, TEMP_PG_DIR
    mov cr3, ecx
 
    ; Enable 4mb pages
    mov ecx, cr4;
    or ecx, PSE_BIT
    mov cr4, ecx
 
    ; Set PG bit, enable paging
    mov ecx, cr0
    or ecx, PG_BIT
    mov cr0, ecx
 
 
 
	finit
	; sti - DO NOT enable interrupts until IDT is created!
	mov esp,stack_top
 
	push ebx
	push eax
 
	call set_up_gdt
	call kmain
 
	cli
.stop:
	hlt
	jmp .stop
