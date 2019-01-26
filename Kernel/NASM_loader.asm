bits 32			;директива nasm - 32 bit
section .text

global start
extern kmain	        ;kmain определена в C-файле

start:
  cli 			;блокировка прерываний
  mov esp, stack_space	;установка указателя стека
  call kmain
  hlt		 	;остановка процессора

section .bss
;resb 8192		;8KB на стек
stack_space: