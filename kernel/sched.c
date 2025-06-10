#include <depthos/signal.h>
#include <depthos/assert.h>
#include <depthos/elf.h>
#include <depthos/heap.h>
#include <depthos/list.h>
#include <depthos/logging.h>
#include <depthos/pic.h>
#include <depthos/pit.h>
#include <depthos/pmm.h>
#include <depthos/proc.h>
#include <depthos/string.h>
#include <depthos/syscall.h>
#include <depthos/tss.h>
#if defined(__i386__) || defined(__x86_64__)
#include <depthos/x86/gsfsbase.h>
#endif

static struct list *tasklist;
static struct list_entry *current_entry;
struct task *current_task = NULL;



static bool sched_debug = false;
// #define SCHED_PROF_COND(CT) strcmp(CT->name, "/bin/game3d") == 0
static bool sched_preempt = false;
bool sched_initialized = false;

void preempt_enable() { sched_preempt = true; }
void preempt_disable() { sched_preempt = false; }
void sched_debug_enable() { sched_debug = true; }
void sched_debug_disable() { sched_debug = false; }

void __idle_thread() {
  // printk("====================================================================="
  //        "== lol!\n");
  printk("lol");
  preempt_enable();

  for (;;) {
    // klogf("lol");
    // while (true)
    //   ;
    // sched_yield();
    // printk("LOL");
    // printk("lol!!\n");
    __asm__ volatile("hlt");
  }
}

void dump_task(struct task *t) {
#define PRINT(name, fmt, ...)                                                  \
  {                                                                            \
    char buffer[512];                                                          \
    snprintf(buffer, 512, fmt, __VA_ARGS__);                                   \
    printk("\t| %-15s | %-50s |\n", name, buffer);                             \
    memset(buffer, '=', sizeof("\t| | |") + 15 + 50);                          \
    buffer[sizeof("\t| | |") + 15 + 50] = 0;                                   \
    printk("\t%s\n", buffer);                                                  \
  }

  if (t->process)
    printk("thread %s [%d]: ", t->process->filepath, t->thid);

  if (t->name)
    printk("%s: ", t->name);
  printk("\n");

  if (t->binfo.entry)
    PRINT("entry", "0x%x", t->binfo.entry);
  PRINT("stack", "0x%x-0x%x", t->kernel_stack, t->kernel_esp);
  PRINT("regs", "0x%x-0x%x (+%d)", t->regs,
        (uintptr_t)t->regs + sizeof(struct registers),
        sizeof(struct registers));
  PRINT("regs->esp", "0x%x (+%d)", t->regs->esp, t->kernel_esp - t->regs->esp);
  struct kernel_symbol *pgdsym = ksymbols_lookup((uintptr_t)t->pgd, true);
  if (pgdsym) {
    PRINT("pgd", "0x%x (%s)", t->pgd, pgdsym->name);
  } else {
    PRINT("pgd", "0x%x", t->pgd);
  }
  PRINT("regs->eip", "0x%x", t->regs->eip);
  PRINT("regs->useresp", "0x%x", t->regs->useresp);
  // PRINT("segments", "cs=%lx ds=%lx ss=%lx es=%lx gs=%lx fs=%lx", t->regs->cs,
  //       t->regs->ds, t->regs->ss, t->regs->es, t->regs->gs, t->regs->fs);
#undef PRINT
}

void dump_tasks() {
  list_foreach(tasklist, task) { dump_task(list_item(task, struct task *)); }
}

void sched_init() {
  idt_disable_hwinterrupts();
  tasklist = list_create();

  void _sched_yield(regs_t * r);
  idt_register_interrupt(0x30, _sched_yield); // yield
  // pagedir_t pgd = kmalloc(0x1000);
  struct task *idle_task = create_kernel_task(__idle_thread, true);
  idle_task->name = "idle";
  idle_task->pgd = kernel_pgd;
  idle_task->process = NULL;
  idle_task->thid = 0;
  // printk("IDLE TASK:\n");
  // dump_task(idle_task);
  sched_add(idle_task);

  current_entry = tasklist->first;
  current_task = list_item(current_entry, struct task *);

  pit_sched_enable = true;

  sched_initialized = true;
  // __asm__ volatile("movl %0, %%esp" ::"r"(idle_task->kernel_esp));
  // __idle_thread();
}

void reschedule_to(struct task *t) {
  // current_task->state = TASK_RUNNING;
  if (sched_debug && sched_preempt && strcmp(t->name, "idle") != 0) {
    if (current_task->process)
      printk("switching from %s[%ld]\n", current_task->process->filepath,
             current_task->thid);
    else
      printk("switching from %s\n", current_task->name);

    if (t->process)
      printk("switching to %s[%ld]\n", t->process->filepath, t->thid);
    else
      printk("switching to %s\n", t->name);

    if (t->regs->esp != 0)
      printk("esp: %lx\n", t->regs->esp);
    printk("previous task:\n");
    printk("------------------------\n");
    if (current_task->regs)
      dump_registers(*current_task->regs);
    dump_task(current_task);
    printk("\nnew task:\n");
    printk("------------------------\n");
    if (t->regs)
      dump_registers(*t->regs);
    dump_task(t);
  }
  current_task = t;

#if defined(__i386__) || defined(__x86_64__)
  // x86_gdt_set_base(6, t->gs_base);
  // x86_gdt_set_base(7, t->fs_base);
  x86_set_gsbase(t->gs_base);
  x86_set_fsbase(t->fs_base);
#endif

  // assert(t->kernel_esp != NULL);
  tss_set_stack((void *)t->kernel_esp);

  if (t->pgd)
    activate_pgd(t->pgd);

  if (t->signal_queue->length > 0) {
    struct signal *s = signal_fetch(t);
    if (s)
      signal_exec(t, *s);
    
    // FIXME: deallocate
  }

  // assert(t->regs->esp != 0);
  __asm__ volatile("movl %0, %%esp;"
                   "jmp intr_exit;" ::"r"(current_task->regs));
}

static struct list_entry *pick_next_task() {
  struct list_entry *current = current_entry;
  struct task *task;
retry:
  current = current->next;

  if (!current)
    current = tasklist->first;

  extern uint32_t pit_ticks;
  task = list_item(current, struct task *);
  if (task->state == TASK_SLEEPING && task->wake_time <= pit_ticks) {
    task->state = TASK_RUNNING;
    return current;
  } else if (task->state == TASK_SLEEPING)
    goto retry;

  return current;
}

void reschedule(void) {
  if (!sched_preempt)
    reschedule_to(current_task);

  struct list_entry *next = pick_next_task();
  if (current_task->state == TASK_DYING) {
    klogf("task %s is dying", current_task->name);
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

void _sched_yield(regs_t *r) {
  current_task->regs = r;
  reschedule();
}

void sched_yield() {
  if (!sched_initialized)
    return;
  // TODO: proper context switch for internal yielding
  __asm__ volatile("int $0x30");
}

uint32_t sched_skip_counter = 0;

void sched_ticker(regs_t *r) {
  idt_disable_hwinterrupts();
  current_task->running_time++;
  pic_eoi(0x20); // XXX: was in reschedule_to, right before the jump. If
                 // anything breaks, bring it back.

  // sched_skip_counter++;
  // if (sched_skip_counter == 1000) {
  //   sched_skip_counter = 0;
  // }
  _sched_yield(r);
}
