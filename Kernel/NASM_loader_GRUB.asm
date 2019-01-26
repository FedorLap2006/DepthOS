set BALIGN,	1<<0
set MEMINFO,	1<<1
set GRAPH,     1<<2
set BFLAGS,	0x0 ;BALIGN | MEMINFO | GRAPH
set MAGIC,	0x1BADB002
set HEADER_LEN head_start - head_end
set CHECKSUM, 	-(MAGIC + BFLAGS)

section .text

dd MAGIC
dd BFLAGS
dd CHECKSUM

; BFLAGS equ -( MALIGN  | VB_MODE | MEMINFO )

; CHSUM equ -( MAGIC | ARCH | HEADER_LEN )

; align 4

; section multiboot_header

; head_start:
    
;     ; dd 0, 0, 0, 0, 0
;     ; dd 0 ; 0 = set graphics mode
;     ; dd 1024, 768, 32 ; Width, height, depth
;     ; optional flags
;     ; dd BFLAGS
;     ; end tag 
;     dw 0
;     dw 0
;     dd 8
; head_end:

global _start
_start:
    cli
    finit
    mov esp,kernel_stack
    push eax
    push ebx
    extern kmain
    call kmain
    
    cli
    hlt
; section .bootstrap_stack, nobits
kernel_stack: