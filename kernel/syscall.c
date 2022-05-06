#include <depthos/elf.h>
#include <depthos/errno.h>
#include <depthos/kconfig.h>
#include <depthos/logging.h>
#include <depthos/proc.h>
#include <depthos/syscall.h>
#include <depthos/tools.h>

DECL_SYSCALL2(write, long, count, char *, message) {
  for (int i = 0; i < count; i++) {
    printk("%c", *(message + i));
  }
  // klogf("lmao");
  // trace(0, -1);
  return 2;
}

DECL_SYSCALL0(exit) {
  // extern struct task *current_task;
  // printk("name: %s\n", current_task->name);
  // dump_tasks();
  extern struct task *current_task;
  // dump_tasks();
  sched_remove(current_task);
  // dump_tasks();
  reschedule();
  // for (;;)
  //   __asm__ volatile("int $0x30");
  // for (;;)
  //   sys_sched_yield();
  // __asm__ volatile("hlt");
  // sys_sched_yield();
}
DECL_SYSCALL0(fork) {
  struct task *tsk = create_task_fork(current_task);
  sched_add(tsk);
  // reschedule();
}

DECL_SYSCALL1(execve, const char *, file) {
  // printk("AAAAAAAAAAAAAAAAAAAAA");
  bool v = elf_probe(file);
  if (!v) {
    printk("file: %s\n", file);
    return -EINVAL;
  }
  extern struct task *current_task;
  // printk("AAAAAAAAAAAAAAAAA\n");
  elf_load(current_task, file);
  elf_exec(current_task);
}
__noinline uintptr_t resolve_syscall_no(long no) {
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
  case 2:
    return sys_fork;
  case 4:
    return sys_write;
  // case 5:
  //   return sys_sched_yield;
  case 11:
    return sys_execve;
  }
  return NULL;
#endif
}
__noinline void syscall_interrupt_handler(regs_t *r) {
  // uintptr_t esp;
  // if (r->eax != 5) {
  //   printk("\nsyscall %d\n", r->eax);
  //   printk("eflags=0x%x\n", r->eflags);
  //   __asm__ volatile("movl %%esp, %0" : "=r"(esp));
  //   printk("esp = 0x%x\n", esp);
  // }

  uintptr_t function = resolve_syscall_no(r->eax);
  if (!function) {
    printk("syscall not found\n");
    return;
  }
  if (current_task)
    current_task->regs = r;
#ifdef CONFIG_SYS_TRACE
  printk("\nsyscall %d\n", r->eax);
  trace(0, -1);
#endif

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