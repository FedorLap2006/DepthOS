STACK_SIZE equ 64

align 4

section .bss

ss_end:
  resb STACK_SIZE ; reserve stack
ss_begin:

section .text

global _loadkernel
extern _kmain ; kmain in another file

_loadkernel:
  
;  extern loadGRUB ; loadGRUB in another file
;  call loadGRUB ; call loadGRUB function

  finit ; init FPU ( math coprocessor )
  mov esp, ss_begin ; use stack

  ; push ebx ; multiboot ptr
  ; push eax ; magic number
  
  ; load kernel main function
 
  call _kmain
  
  ; halt CPU and disable interrputs
  
  cli ; disable interrputs
  hlt ; CPU halt


; section .end
