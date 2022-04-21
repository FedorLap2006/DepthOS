#include <depthos/kconfig.h>
#include <depthos/logging.h>
#include <depthos/syscall.h>
#include <depthos/tools.h>

DECL_SYSCALL2(write, long, count, char *, message) {
  for (int i = 0; i < count; i++) {
    printk("%c", *(message + i));
  }
  return 2;
}

DECL_SYSCALL0(exit) { __asm__ volatile("call init"); }

uintptr_t resolve_syscall_no(long no) {
#ifndef CONFIG_SYS_LINUX_COMPAT
  extern void _syscall_entries;
  static uintptr_t *syscall_entries = &_syscall_entries;
  extern int syscall_entries_len;

  if (no >= syscall_entries_len) {
    return NULL;
  }
  return syscall_entries[no];
#else
  switch (no) {
  case 1:
    return sys_exit;
  case 4:
    return sys_write;
  }
  return NULL;
#endif
}
void syscall_interrupt_handler(regs_t *r) {
  uintptr_t function = resolve_syscall_no(r->eax);
  if (!function)
    return;
  long ret;
  __asm__ volatile("pushl %1;"
                   "pushl %2;"
                   "pushl %3;"
                   "pushl %4;"
                   "pushl %5;"
                   "call *%6;"
                   "popl %%ebx;"
                   "popl %%ebx;"
                   "popl %%ebx;"
                   "popl %%ebx;"
                   "popl %%ebx;"
                   : "=a"(ret)
                   : "r"(r->edi), "r"(r->esi), "r"(r->edx), "r"(r->ecx),
                     "r"(r->ebx), "r"(function));
  r->eax = ret;
}