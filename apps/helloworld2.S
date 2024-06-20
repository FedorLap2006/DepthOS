.data
hello:
.string "\x1B[1;34mUserapp start!\x1B[0m"
hello_end:

after:
.string "\x1B[1;34mUserapp middle!\x1B[0m"
after_end:

after2:
.string "\x1B[1;34mUserapp end!\x1B[0m"
after_end2:

.text
.globl _start

something:
  pushl %ebp
  movl %esp, %ebp

  // movl $0, %ecx
  // movl $1, %edi
  // movl $2, %esi
  // movl $3, %ebp
  // // movl $4, %esp
  // movl $5, %ebx
  // movl $6, %edx
  // movl $7, %ecx
  movl $(hello_end - hello), %ebx
  movl $hello, %ecx
  movl $2, %eax
  int $0x64

  int $0x30
  
  movl $(after_end - after), %ebx
  movl $after, %ecx
  movl $2, %eax
  int $0x64
  movl $(after_end2 - after2), %ebx
  movl $after2, %ecx
  movl $2, %eax
  int $0x64

  // movl $4, %eax
  // int $0x80
  add $4, %esp
  ret

_start:
  / pushl 0
  / movl $4, %ebx
  / movl $3, %ecx
  / movl $2, %eax
  / movl $1, %edx
  // movl $4, %eax # sys_write

  call something

  // movl $(hello_end - hello), %ebx
  // movl $hello, %ecx
  // movl $4, %eax
  // int $0x80

// a:
//   jmp a
  movl $1, %eax # sys_exit
  int $0x64
