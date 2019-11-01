BL_0LH_MAGIC equ 0x1BADB002 ; multiboot v1
BL_0LH_FLAGS equ 0 | 1 << 1 | 1 << 0
BL_0LH_CHECKSUM equ -(BL_0LH_MAGIC + BL_0LH_FLAGS)


KRNL_STACK_SIZE equ 80000

; .long MAGIC
; .long FLAGS
; .long CHECKSUM
; .long 0 # header_addr
; .long 0 # load_addr
; .long 0 # load_end_addr
; .long 0 # bss_end_addr
; .long 0 # entry_addr
; .long 0 # mode_type
; .long 0 # width
; .long 0 # height
; .long 0 # depth

;  push %ebx
;  push %eax

section .bl_0l_header
dd BL_0LH_MAGIC
dd BL_0LH_FLAGS
dd BL_0LH_CHECKSUM

section .bss
global __krnl_stack_top
global __krnl_stack_root

__krnl_stack_top:
  resb KRNL_STACK_SIZE
__krnl_stack_root:

section .krnl_data

section .text

global krnl_bootloader

extern _kernel

krnl_bootloader:
  ;jmp 0x08:.krnl
.krnl:
  mov esp, __krnl_stack_top
  finit

  push ebx
  push eax
  call _kernel
.stop:
  hlt
  jmp .stop
