#include <depthos/kernel.h>
#include <depthos/logging.h>
#include <depthos/proc.h>
#include <depthos/trace.h>
#include <depthos/kconfig.h>

void trace(unsigned skip, unsigned max) {
  struct trace_stackframe *ebp;
  __asm__ volatile("movl %%ebp, %0" : "=r"(ebp));
  for (unsigned i = 0; i < skip && ebp != NULL; i++)
    ebp = ebp->next;
  printk("Call trace:\n");
  struct kernel_symbol *s;
  for (unsigned i = 0; i < max && ebp != NULL; i++) {
    printk("\t");
    if (ebp->eip >= VIRT_BASE) {
      s = ksymbols_lookup(ebp->eip, false);
      if (s && (s->type == 'T' || s->type == 't')) {
        printk("%s+0x%x\n", s->name, ebp->eip - s->address);
        ebp = ebp->next;
        continue;
      }
    }

    printk("? 0x%x\n", ebp->eip);
    ebp = ebp->next;
  }
#ifdef CONFIG_TRACE_TASK_INFO
	if (current_task->process)
		printk("thread: %s [%d]", current_task->process->filepath, current_task->thid);
	else printk("kernel thread: %s", current_task->name);
	printk("\n");
#endif
}
