#include <depthos/bitmap.h>
#include <depthos/elf.h>
#include <depthos/errno.h>
#include <depthos/kconfig.h>
#include <depthos/logging.h>
#include <depthos/proc.h>
#include <depthos/syscall.h>
#include <depthos/tools.h>

void *copy_userspace_ptr(void *ptr, size_t size) {
  void *kptr = kmalloc(size);
  memcpy(kptr, ptr, size);
  return kptr;
}

// DECL_SYSCALL2(write, long, count, char *, message) {
//   for (int i = 0; i < count; i++) {
//     printk("%c", *(message + i));
//   }
//   // klogf("lmao");
//   // trace(0, -1);
//   return 2;
// }

static inline uintptr_t resolve_posix_syscall_no(long no) {
  switch (no) {
  case 0x1:
    return (uintptr_t)sys_exit;
  case 0x2:
    return (uintptr_t)sys_fork;
  case 0x4:
    return (uintptr_t)sys_write;
  case 0xB:
    return (uintptr_t)sys_execve;
  }
  return NULL;
}

__noinline void posix_syscall_handler(regs_t *r) {
#ifdef CONFIG_SYS_TRACE
  printk("\nposix syscall %d\n", r->eax);
  trace(0, -1);
#endif

  exec_syscall(r, resolve_posix_syscall_no(r->eax));
}

__noinline void syscall_handler(regs_t *r) {
#ifdef CONFIG_SYS_TRACE
  printk("\nsyscall %d\n", r->eax);
  trace(0, -1);
#endif
  extern void _syscall_entries;
  static uintptr_t *syscall_entries = &_syscall_entries;
  exec_syscall(r, syscall_entries[r->eax]);
}
void exec_syscall(regs_t *r, uintptr_t function) {
  // uintptr_t esp;
  // if (r->eax != 5) {
  //   printk("\nsyscall %d\n", r->eax);
  //   printk("eflags=0x%x\n", r->eflags);
  //   __asm__ volatile("movl %%esp, %0" : "=r"(esp));
  //   printk("esp = 0x%x\n", esp);
  // }

  if (current_task)
    current_task->regs = r;

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

  // if (r->eax != 5) {
  //   __asm__ volatile("movl %%esp, %0" : "=r"(esp));
  //   printk("\nsyscall %d\n", r->eax);
  //   printk("esp after = 0x%x\n", esp);
  // }
  r->eax = ret;
  // TODO: deal with return
}
