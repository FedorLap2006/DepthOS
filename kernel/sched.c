#include <depthos/elf.h>
#include <depthos/heap.h>
#include <depthos/list.h>
#include <depthos/logging.h>
#include <depthos/pic.h>
#include <depthos/pmm.h>
#include <depthos/proc.h>
#include <depthos/string.h>
#include <depthos/syscall.h>
#include <depthos/tss.h>

static struct list *tasklist;
static struct list_entry *current_entry;
struct task *current_task;

static bool __sched_prof = false;
static bool __preempt_enabled = false;

void preempt_enable() { __preempt_enabled = true; }
void preempt_disable() { __preempt_enabled = false; }
void sched_prof_enable() { __sched_prof = true; }
void sched_prof_disable() { __sched_prof = false; }

void __idle_thread() {
  preempt_enable();

  for (;;)
    __asm__ volatile("int $0x30");
}

void dump_task(struct task *t) {
#define PRINT(name, fmt, ...)                                                  \
  {                                                                            \
    char buffer[256];                                                          \
    snprintf(buffer, 256, fmt, __VA_ARGS__);                                   \
    printk("\t| %-15s | %-30s |\n", #name, buffer);                            \
    memset(buffer, '=', sizeof("\t| | |") + 15 + 30);                          \
    buffer[sizeof("\t| | |") + 15 + 30] = 0;                                   \
    printk("\t%s\n", buffer);                                                  \
  }

  if (t->process)
    printk("thread %s [%d]: ", t->process->filepath, t->thid);

  if (t->name)
    printk("%s: ", t->name);
  printk("\n");

  if (t->binfo.entry)
    PRINT(entry, "0x%x", t->binfo.entry);
  PRINT(stack, "0x%x-0x%x", t->kernel_stack, t->kernel_esp);
  PRINT(regs, "0x%x-0x%x (+%d)", t->regs,
        (uintptr_t)t->regs + sizeof(struct registers),
        sizeof(struct registers));
  PRINT(regs->esp, "0x%x (+%d)", t->regs->esp, t->kernel_esp - t->regs->esp);
  struct kernel_symbol *pgdsym = ksymbols_lookup(t->pgd, true);
  if (pgdsym) {
    PRINT(pgd, "0x%x (%s)", t->pgd, pgdsym->name);
  } else {
    PRINT(pgd, "0x%x", t->pgd);
  }
  PRINT(regs->eip, "0x%x", t->regs->eip);
  PRINT(regs->useresp, "0x%x", t->regs->useresp);
#undef PRINT
}

void dump_tasks() {
  list_foreach(tasklist, task) { dump_task(list_item(task, struct task *)); }
}

void sched_init() {
  idt_disable_hwinterrupts();
  tasklist = list_create();

  void sched_yield(regs_t * r);
  idt_register_interrupt(0x30, sched_yield); // yield
  // pagedir_t pgd = kmalloc(0x1000);
  struct task *idle_task = create_kernel_task(__idle_thread, true);
  idle_task->name = "idle";
  idle_task->pgd = kernel_pgd;
  idle_task->process = NULL;
  idle_task->thid = 0;
  sched_add(idle_task);

  current_entry = tasklist->first;
  current_task = list_item(current_entry, struct task *);

  extern bool ticker_sched_enable;
  ticker_sched_enable = true;

  // __asm__ volatile("movl %0, %%esp" ::"r"(idle_task->kernel_esp));
  // __idle_thread();
}

void reschedule_to(struct task *t) {
  current_task->state = TASK_RUNNING;
  current_task = t;
  if (__sched_prof) {
    if (t->process)
      printk("switching to %s[%d]\n", t->process->filepath, t->thid);
    else
      printk("switching to %s\n", t->name);
    dump_task(current_task);
  }
  tss_set_stack(t->kernel_esp);

  if (t->pgd)
    activate_pgd(t->pgd);
  pic_eoi(0x20);
  __asm__ volatile("movl %0, %%esp;"
                   "jmp intr_exit;" ::"r"(current_task->regs));
}

static struct list_entry *pick_next_task() {
  if (current_entry->next)
    return current_entry->next;
  return tasklist->first;
}

void reschedule(void) {
  if (!__preempt_enabled)
    reschedule_to(current_task);

  struct list_entry *next = pick_next_task();
  if (current_task->state == TASK_DYING) {
    klogf("task %s is dying\n", current_task->name);
    thread_destroy(current_task);
  }

  current_entry = next;
  reschedule_to(list_item(current_entry, struct task *));
}

void sched_add(struct task *task) {
  struct list_entry *entry = list_push(tasklist, (list_value_t)task);
  task->state = TASK_STARTING;
  task->sched_entry = entry;
}

void sched_remove(struct task *task) {
  list_remove(tasklist, task->sched_entry);
  kfree(task->sched_entry, sizeof(struct list_entry));
  task->sched_entry = NULL;
}

void sched_yield(regs_t *r) {
  current_task->regs = r;
  reschedule();
}

void sched_ticker(regs_t *r) {
  idt_disable_hwinterrupts();
  current_task->running_time++;
  current_task->running_time_sched = 0;
  sched_yield(r);
}
