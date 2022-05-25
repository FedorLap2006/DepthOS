
.data
.globl _syscall_entries

_syscall_entries:
#define SC(name, num) .long name
#include "syscall_decl.inc"
#undef SC
_syscall_entries_end:

.globl syscall_entries_len
syscall_entries_len:
  .int ((_syscall_entries_end - _syscall_entries) >> 2)
