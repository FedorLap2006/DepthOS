#include <depthos/elf.h>
#include <depthos/heap.h>
#include <depthos/logging.h>
#include <depthos/pic.h>
#include <depthos/pmm.h>
#include <depthos/proc.h>
#include <depthos/string.h>
#include <depthos/syscall.h>
#include <depthos/tss.h>

struct tasklist {
  struct tasklist *next;
  struct tasklist *prev;
  struct task *task;
};

static struct tasklist *tasklist_head;
static struct tasklist *tasklist_top;
static struct tasklist *tasklist_current;
struct task *current_task;

bool __preempt_enabled = false;

void preempt_enable() { __preempt_enabled = true; }
void preempt_disable() { __preempt_enabled = false; }

void __idle_thread() {
  preempt_enable();
  // extern void init(void);
  // struct task *init_task = create_kernel_task(init, true);
  // init_task->name = "";
  // sched_add(init_task);
  // printk("================ YIELDING ================");

  for (;;) {
    // __asm__ volatile("int $0x80" : : "a"(4), "b"(1), "c"('a'));
    __asm__ volatile("int $0x30");
  }
  // __asm__ volatile("int $0x80" ::"a"(5));
}

void __th1() {
  for (;;)
    ;
}
void __th2() {
  for (;;)
    ;
}

void dump_task(struct task *t) {
#define PRINT(name, fmt, ...)                                                  \
  {                                                                            \
    char buffer[256];                                                          \
    snprintf(buffer, 256, fmt, __VA_ARGS__);                                   \
    printk("\t| %-10s | %-30s |\n", #name, buffer);                            \
    memset(buffer, '=', sizeof("\t| | |") + 10 + 30);                          \
    buffer[sizeof("\t| | |") + 10 + 30] = 0;                                   \
    printk("\t%s\n", buffer);                                                  \
  }

  printk("%s:\n", t->name);
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
#undef PRINT
}

void dump_tasks() {
  for (struct tasklist *task = tasklist_head; task != NULL; task = task->next) {
    dump_task(task->task);
  }
}

void _task_init(struct task *task) {
  task->kernel_stack = kmalloc(0x1000);
  task->kernel_esp = task->kernel_stack + 0x1000;
}

static struct task *create_dummy_task() {
  struct task *task = kmalloc(sizeof(struct task));
  _task_init(task);
  return task;
}

void sched_init() {
  idt_disable_hwinterrupts();
  tasklist_head = kmalloc(sizeof(struct tasklist));
  tasklist_top = tasklist_head;
  tasklist_current = tasklist_head;

  void sched_yield(regs_t * r);
  idt_register_interrupt(0x30, sched_yield); // yield
  // pagedir_t pgd = kmalloc(0x1000);
  struct task *idle_task = create_kernel_task(__idle_thread, true);
  idle_task->name = "idle";
  idle_task->pgd = kernel_pgd;
  tasklist_head->task = idle_task;
  void init();
  struct task *init_task = create_kernel_task(init, true);
  init_task->name = "init";
  // tasklist_head->task = init_task;
  sched_add(init_task);

  struct task *user_task = kmalloc(sizeof(struct task));
  printk("yolo: %d\n", elf_probe("/autoload2.bin"));
  elf_load(user_task, "/autoload2.bin");
  bootstrap_user_task(user_task, true);
  sched_add(user_task);

  struct task *th1 = create_kernel_task(__th1, true);
  th1->name = "kthread1";
  sched_add(th1);

  struct task *th2 = create_kernel_task(__th2, true);
  th2->name = "kthread2";
  sched_add(th2);

  current_task = tasklist_head->task;
  tasklist_top->next = NULL;

  extern bool ticker_sched_enable;
  ticker_sched_enable = true;

  // __asm__ volatile("movl %0, %%esp" ::"r"(idle_task->kernel_esp));
  // __idle_thread();
}

void reschedule_to(struct task *t) {
  current_task = t;
  current_task->running = 0;
  // printk("switching to %s\n", t->name);
  // dump_task(current_task);
  tss_set_stack(t->kernel_esp);

  if (t->pgd)
    activate_pgd(t->pgd);
  // printk("=============== switch task: %s\n", t->name);
  pic_eoi(0x20);
  __asm__ volatile("movl %0, %%esp;"
                   "jmp intr_exit;" ::"r"(current_task->regs));
}

static struct tasklist *pick_next_task() {
  if (tasklist_current->next)
    return tasklist_current->next;
  return tasklist_head;
}

void reschedule(void) {
  if (!__preempt_enabled)
    reschedule_to(current_task);

  struct tasklist *next = pick_next_task();
  tasklist_current = next;
  reschedule_to(next->task);
}

void bootstrap_user_task(struct task *task, bool do_stack) {
  _task_init(task);
  if (!do_stack)
    return;
  uintptr_t page = pmm_alloc(1);
  if (!page) {
    kfree(task, sizeof(struct task));
    return NULL;
  }
#if 0
  pmm_dump_compact();
  pagedir_t current_pgd = get_current_pgd();
  klogf("directory:");
  for (int j = 0; j < 1024; j++) {
    if (!current_pgd[j])
      continue;
    klogf("[0x%x]: %x:", j * 4096 * 1024, current_pgd[j]);
    for (int k = 0; k < 1024; k++) {
      pageinfo_t pg =
          parse_page(get_page(current_pgd, j * 4096 * 1024 + k * 4096));
      klogf("\t[0x%x]: 0x%x", j * 4096 * 1024 + k * 4096, pg.frame);
    }
  }
  map_addr_phys(current_pgd, VIRT_BASE - 0x1000, page, 1);
  klogf("directory:");
  for (int j = 0; j < 1024; j++) {
    if (!current_pgd[j])
      continue;
    klogf("[0x%x]: %x:", j * 4096 * 1024, current_pgd[j]);
    for (int k = 0; k < 1024; k++) {
      pageinfo_t pg =
          parse_page(get_page(current_pgd, j * 4096 * 1024 + k * 4096));
      klogf("\t[0x%x]: 0x%x", j * 4096 * 1024 + k * 4096, pg.frame);
    }
  }
#endif

  map_addr_phys(get_current_pgd(), VIRT_BASE - 0x1000, page, 1);
  activate_pgd(get_current_pgd());
  if (task->pgd)
    map_addr_phys(task->pgd, VIRT_BASE - 0x1000, page, 1);
  uint32_t *stack = VIRT_BASE;
  memset(VIRT_BASE - 0x1000, 0, 0x1000 - 1);
  uint32_t tmp = stack;
#define PUSH(v) *--stack = v
  PUSH(0x23);              /* ss */
  PUSH(tmp);               /* useresp */
  PUSH(0x202);             /* eflags */
  PUSH(0x1b);              /* cs */
  PUSH(task->binfo.entry); /* eip */

  PUSH(0x0);
  PUSH(0);
  PUSH(0);

  tmp = stack;
  PUSH(0x0);  /* eax */
  PUSH(0x0);  /* ecx */
  PUSH(0x0);  /* edx */
  PUSH(0x0);  /* ebx */
  PUSH(0x0);  /* esp */
  PUSH(0x0);  /* ebp */
  PUSH(0x0);  /* esi */
  PUSH(0x0);  /* edi */
  PUSH(0x23); /* ds */
  PUSH(0x23); /* es */
  PUSH(0x23); /* gs */
  PUSH(0x23); /* fs */
#undef PUSH
  // get_current_pgd()[pde_index(VIRT_BASE - 0x1000)] = 0;
  // activate_pgd(get_current_pgd());
  task->regs = (struct registers *)stack;
  task->stack = stack;
}

struct task *create_task(void *entry, pagedir_t pgd, bool do_stack) {
  struct task *task = create_dummy_task();
  task->pgd = pgd;
  bootstrap_user_task(task, do_stack);
  return task;
}

struct task *create_task_fork(struct task *parent) {
  struct task *task = kmalloc(sizeof(struct task));
  *task = *parent;
  task->parent = parent;
  task->pgd = clone_pgd(parent->pgd);
  _task_init(task);
  memcpy(task->kernel_stack, parent->kernel_stack, 0x1000);
  task->regs =
      task->kernel_esp - (parent->kernel_esp - (uintptr_t)parent->regs);
  task->regs->eax = 0;
  return task;
}

struct task *create_kernel_task(void *entry, bool do_stack) {
  struct task *task = create_dummy_task();
  task->pgd = kernel_pgd;
  if (!do_stack)
    return;
  uint32_t *stack = task->kernel_esp;
  uint32_t tmp;
  // stack -= 4;

#define PUSH(v) *--stack = v
  PUSH(0x10);  /* ss */
  PUSH(0x0);   /* useresp */
  PUSH(0x202); /* eflags */
  PUSH(0x08);  /* cs */
  PUSH(entry); /* eip */

  // TODO: implement tracing for kernel tasks?
  PUSH(0x0); /* trace_frame */
  PUSH(0);   /* err_code */
  PUSH(0);   /* int_num */

  tmp = stack;
  PUSH(0x0);  /* eax */
  PUSH(0x0);  /* ecx */
  PUSH(0x0);  /* edx */
  PUSH(0x0);  /* ebx */
  PUSH(tmp);  /* esp */
  PUSH(0x0);  /* ebp */
  PUSH(0x0);  /* esi */
  PUSH(0x0);  /* edi */
  PUSH(0x10); /* ds */
  PUSH(0x10); /* es */
  PUSH(0x10); /* gs */
  PUSH(0x10); /* fs */

#undef PUSH
  task->regs = (struct registers *)stack;
  task->stack = stack;
  return task;
}

void sched_add(struct task *task) {
  tasklist_top->next = kmalloc(sizeof(struct tasklist));
  tasklist_top->next->prev = tasklist_top;
  tasklist_top = tasklist_top->next;
  tasklist_top->next = NULL;
  tasklist_top->task = task;
}

void sched_remove(struct task *task) {
  if (!tasklist_head && !tasklist_top)
    return;
  for (struct tasklist *ptr = tasklist_head; ptr != NULL; ptr = ptr->next) {
    if (ptr->task == task) {
      if (!ptr->prev) { // beginning
        tasklist_head = ptr->next;
        if (ptr->next) // and the end
          ptr->next->prev = NULL;
      } else {
        ptr->prev->next = ptr->next;
        if (ptr->next)
          ptr->next->prev = ptr->prev;
      }

      if (!ptr->next)
        tasklist_top = ptr->prev;
      // if (!ptr->prev) {
      //   tasklist_head = ptr->next;
      // } else
      //   ptr->prev->next = ptr->next;
      // if (ptr->prev) {
      //   printk("ready player one\n");
      //   ptr->prev->next = ptr->next;
      // } else {
      //   printk("moving forward\n");
      //   tasklist_head = ptr->next;
      // }
    }
  }
}

void sched_yield(regs_t *r) {
  // printk("before yield: \n");
  // dump_task(current_task);
  current_task->regs = r;
  // printk("after yield: \n");
  // dump_task(current_task);
  reschedule();
}

void sched_ticker(regs_t *r) {
  // trace(0, -1);
  idt_disable_hwinterrupts();
  current_task->running++;
  sched_yield(r);
}
