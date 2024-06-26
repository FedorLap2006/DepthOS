bits 32
 
section .bss

STACK_SIZE equ 60000 ; 4096 * 1024 * 1024 + 400
	
align 4 
global stack_top
global stack_end
stack_end:
	resb STACK_SIZE
stack_top:

section .multiboot

MB1_MAGIC          equ 0x1BADB002 ; MB 2 - 0xe85250d6
MB1_FLAGS_MEMINFO  equ 1<<1
MB1_FLAGS_MBALIGN  equ 1<<0
MB1_FLAGS_GRAPHICS equ 1<<2
MB1_FLAGS          equ MB1_FLAGS_MBALIGN  \
  | MB1_FLAGS_MEMINFO \
   | MB1_FLAGS_GRAPHICS

align 4 
multiboot_header:
	dd MB1_MAGIC
	dd MB1_FLAGS
	dd -(MB1_MAGIC + MB1_FLAGS)
  dd 0
  dd 0
  dd 0
  dd 0
  dd 0
  dd 0
  dd 0
  dd 0
  dd 24
multiboot_header_end:

section .bootstrap.data
align 4096
 
VM_BASE   equ 0xC0000000
PDE_INDEX equ (VM_BASE >> 22)


global lowerkrnl_page_directory
lowerkrnl_page_directory:
    dd 0x00000083 
    times(PDE_INDEX - 1) dd 0
    dd 0x00000083
    times(1024 - PDE_INDEX - 1) dd 0


section .bootstrap.text
align 4

PSE_BIT equ 0x00000010
PG_BIT  equ 0x80000000


global _loader
; _loader equ lower_loader - VM_BASE
_loader equ lower_loader
global lower_loader
lower_loader:
  cli
  mov ax, ds
  mov es, ax
  mov ss, ax
  cld


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
 
	mov esp, stack_top
	
	finit

	; extern set_up_gdt
	; call set_up_gdt
	extern x86_gdt_init
	call x86_gdt_init
	push ebx
	push eax

  mov eax, 0x1
  cpuid
  test edx, 1<<25
  jz .noSSE
  
  ; SSE setup
  mov eax, cr0
  and ax, 0xFFFB
  or ax, 0x2
  mov cr0, eax
  mov eax, cr4
  or ax, 3 << 9
  mov cr4, eax

.noSSE:

	xor ebp, ebp
	extern kmain
	call kmain
 
	cli
.stop:
	hlt
	jmp .stop
