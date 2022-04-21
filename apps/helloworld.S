.data
hello:
.string "\x1B[1;34mHello kernel world!\x1B[0m"
hello_end:

.text
.globl _start

_start:
  / pushl 0
  / movl $4, %ebx
  / movl $3, %ecx
  / movl $2, %eax
  / movl $1, %edx
  movl $4, %eax # sys_write
  movl $(hello_end - hello), %ebx
  movl $hello, %ecx
  int $0x80

  movl $1, %eax # sys_exit
  int $0x80