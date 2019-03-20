set STACK_SIZE, 64

align 4

section .bss

ss_end:
  resb STACK_SIZE ; reserve stack
ss_begin:

section .text

global _loadkernel

_loadkernel:
  
;  extern loadGRUB ; loadGRUB in another file
;  call loadGRUB ; call loadGRUB function
  
  push ebx
  push eax

  finit ; init FPU ( math coprocessor )
  mov esp, ss_begin ; use stack
  
  ; load kernel main function
  
  extern kmain ; kmain in another file
  call kmain
  
  ; halt CPU and disable interrputs
  
  cli ; disable interrputs
  hlt ; CPU halt


; section .end
