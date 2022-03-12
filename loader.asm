bits 32
 
section .bss

STACK_SIZE equ 60000 ; 4096 * 1024 * 1024 + 400
	
align 4 
stack_end:
	resb STACK_SIZE
stack_top:

section .multiboot

MB1_MAGIC          equ 0x1BADB002 ; MB 2 - 0xe85250d6
MB1_FLAGS_MEMINFO  equ 1<<1
MB1_FLAGS_MBALIGN  equ 1<<0
MB1_FLAGS          equ 0 | MB1_FLAGS_MBALIGN | MB1_FLAGS_MEMINFO
MB1_HEADER_LENGTH  equ multiboot_header_end - multiboot_header
MB1_CHECKSUM       equ 0x100000000 - (MB1_MAGIC + MB1_FLAGS + MB1_HEADER_LENGTH)

align 4 
multiboot_header:
	dd MB1_MAGIC
	dd MB1_FLAGS
	dd -(MB1_MAGIC + MB1_FLAGS)
	dd MB1_HEADER_LENGTH
	dd MB1_CHECKSUM
multiboot_header_end:

section .data
align 4096
 
VM_BASE   equ 0xC0000000
PDE_INDEX equ (VM_BASE >> 22)

global lowerkrnl_page_directory
lowerkrnl_page_directory:
    dd 0x00000083 
    times(PDE_INDEX - 1) dd 0
    dd 0x00000083
    times(1024 - PDE_INDEX - 1) dd 0


section .text
align 4

PSE_BIT equ 0x00000010
PG_BIT  equ 0x80000000


global _loader
_loader equ lower_loader - VM_BASE

lower_loader:
	; Update current page directory and prepare for jump
	mov ecx, (lowerkrnl_page_directory - VM_BASE)
	mov cr3, ecx
	; Enable 4mb pages
	mov ecx, cr4;
	or ecx, PSE_BIT
	mov cr4, ecx

	; Enable paging
	mov ecx, cr0
	or ecx, PG_BIT
	mov cr0, ecx

	; ; Just jumping to higher_loader is a relative jump, so it will just increase eip.
	; ; But we want eip to change to 0xc0000000 based addresses. 
	; ; So we're using doing jump here
	lea ecx, [higher_loader]
	jmp ecx

higher_loader:
	; Unmap first 4mb page, because we don't need it anymore
	; mov dword[lowerkrnl_page_directory], 0
	; invlpg[0]

	finit
	mov esp, stack_top

	push ebx
	push eax
 	
	extern kmain
	extern set_up_gdt
	call set_up_gdt
	call kmain
 
	cli
.stop:
	hlt
	jmp .stop
