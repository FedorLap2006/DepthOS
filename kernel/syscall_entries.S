
.data
.globl _syscall_entries

_syscall_entries:
  .long 0
  .long sys_write
  .long sys_exit
_syscall_entries_end:

.globl syscall_entries_len
syscall_entries_len:
  .int ((_syscall_entries_end - _syscall_entries) >> 2)