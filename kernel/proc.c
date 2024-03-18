#include <depthos/bitmap.h>
#include <depthos/elf.h>
#include <depthos/errno.h>
#include <depthos/fs.h>
#include <depthos/heap.h>
#include <depthos/logging.h>
#include <depthos/paging.h>
#include <depthos/proc.h>
#include <depthos/string.h>
#include <depthos/syscall.h>
#include <depthos/vmm.h>
#include <depthos/x86/asm/gdt.h>

static inline void alloc_kernel_stack(struct task *task) {
  task->kernel_stack = (uintptr_t)kmalloc(KSTACK_SIZE);
  task->kernel_esp = task->kernel_stack;
  task->kernel_esp += KSTACK_SIZE;
}

static inline void alloc_stack(struct task *task, uintptr_t stack, int size,
                               bool user) {
  klogf("allocating stack in region of %p...%p", stack,
        stack + PG_RND_UP(size));
  map_addr(task->pgd, stack, PG_RND_UP(size) / PAGE_SIZE, user,
           true); // TODO: use do_mmap
}

static struct task *create_dummy_task() {
  struct task *task = kmalloc(sizeof(struct task));
  task->gs_base = task->fs_base = 0x0;
  task->vm_areas = list_create();
  task->mmap_bump_idx = 0;
  alloc_kernel_stack(task);
  return task;
}

void _task_init_kstack(struct task *task, int selcode, int seldata, int selgs,
                       int selfs, uintptr_t esp) {
  uint32_t *kstack = (uint32_t *)task->kernel_esp;
  // memset(task->kernel_stack, 0, 0x1000 - 1);
#define PUSH(v) *--kstack = (uint32_t)v
  PUSH(seldata);           /* ss */
  PUSH(esp);               /* useresp */
  PUSH(0x202);             /* eflags */
  PUSH(selcode);           /* cs, 0x1b or 0x08 */
  PUSH(task->binfo.entry); /* eip */

  PUSH(0x0);
  PUSH(0);
  PUSH(0);

  PUSH(0x0);     /* eax */
  PUSH(0x0);     /* ecx */
  PUSH(0x0);     /* edx */
  PUSH(0x0);     /* ebx */
  PUSH(0x0);     /* esp */
  PUSH(0x0);     /* ebp */
  PUSH(0x0);     /* esi */
  PUSH(0x0);     /* edi */
  PUSH(seldata); /* ds */
  PUSH(seldata); /* es */
  PUSH(selgs);   /* gs */
  PUSH(selfs);   /* fs */
#undef PUSH
  // get_current_pgd()[pde_index(VIRT_BASE - 0x1000)] = 0;
  // activate_pgd(get_current_pgd());
  task->regs = (struct registers *)kstack;
  task->stack = (uintptr_t)kstack;
}

void task_init_kstack(struct task *task, uintptr_t esp) {
  _task_init_kstack(task, GDT_USER_CODE_SEL, GDT_USER_DATA_SEL,
                    GDT_GSBASE_SEL(3), GDT_FSBASE_SEL(3), esp);
}

void kernel_task_init_kstack(struct task *task, uintptr_t esp) {
  _task_init_kstack(task, GDT_KERNEL_CODE_SEL, GDT_KERNEL_DATA_SEL,
                    GDT_GSBASE_SEL(0), GDT_FSBASE_SEL(0), esp);
}

void task_init_stack(struct task *task, int argc, char **argv, char **envp) {
  pagedir_t pgd = get_current_pgd();
  activate_pgd(task->pgd);
  char *esp = (void *)task->regs->useresp;

#define PUSH(v, t)                                                             \
  esp -= sizeof(t);                                                            \
  *(t *)esp = (t)v;

#define PUSHP(p) PUSH(p, uintptr_t)
#define PUSHI(ip, l)                                                           \
  esp -= l;                                                                    \
  memcpy(ip, esp, l);
  for (int i = 0; envp[i] != NULL; i++) {
    esp -= strlen(envp[i]) + 1;
    memcpy(esp, envp[i], strlen(envp[i]) + 1);
    envp[i] = (char *)esp;
  }

  for (int i = 0; i < argc; i++) {
    esp -= strlen(argv[i]) + 1;
    memcpy(esp, argv[i], strlen(argv[i]) + 1);
    argv[i] = (char *)esp;
  }

  PUSH(0, uint64_t);
  PUSH(0, uint32_t);

  for (int i = 0; envp[i] != NULL; i++) {
    klogf("env=%s (%p)", envp[i], envp[i]);
    PUSHP(envp[i]);
  }

  PUSH(0, uint32_t);

  for (int i = argc - 1; i >= 0; i--) {
    klogf("argv[i]=%s (%p)", argv[i], argv[i]);
    PUSHP(argv[i]);
  }

  PUSH(argc, uint32_t);

  task->regs->useresp = (uint32_t)esp;
#undef PUSHP
#undef PUSH
  activate_pgd(pgd);
}

void task_setup_filetable(struct fs_node **ft) {
  memset(ft, 0, sizeof(struct fs_node *) * TASK_FILETABLE_MAX);
  struct fs_node *tty_file = vfs_open("/dev/tty0");
  ft[0] = tty_file;
  ft[1] = tty_file;
}

struct task *create_task(void *entry, pagedir_t pgd, bool kernel, uintptr_t esp,
                         size_t stack_size) {
  struct task *task = create_dummy_task();
  task->pgd = pgd;
  task->binfo.entry = entry;
  alloc_stack(task, esp - stack_size, stack_size, !kernel);
  if (kernel)
    kernel_task_init_kstack(task, esp);
  else {
    char *argv[] = {"argv1", "argv2"};
    char *envp[] = {"a=b", "c=d", NULL};
    // TODO: figure out envp and argv
    task_init_kstack(task, esp); // TODO: finish and refactor other declarations to use esp + size
    task_init_stack(task, 2, argv, envp);
  }
  return task;
}

struct task *fork_task(struct task *parent) {
  printk("============== forking task %s ================\n", parent->name);
  extern bitmap_t kheap_bitmap;
  struct task *task = create_dummy_task();
  bitmap_dump_compact(&kheap_bitmap, 0, -1, 32);
  task->name = strdup(parent->name);
  task->state = parent->state;
  task->thid = parent->thid;
  task->pgd = clone_pgd(parent->pgd);
  task->binfo = parent->binfo;
  task->process = NULL;
  task->sched_entry = NULL;
  task->vm_areas = parent->vm_areas; // TODO: clone pgd memes
  task->parent = parent;
  task->filetable = kmalloc(sizeof(struct fs_node *) * TASK_FILETABLE_MAX);
  printk("filetable: %p\n", task->filetable);
  bitmap_dump_compact(&kheap_bitmap, 0, -1, 32);
  printk("kernel_stack: %p\n", task->kernel_stack);
  printk("kernel_stack: *%p\n", &task);
  memcpy(task->filetable, parent->filetable,
         sizeof(struct fs_node *) * TASK_FILETABLE_MAX);
  printk("kernel_stack: %p\n", task->kernel_stack);
  bitmap_dump_compact(&kheap_bitmap, 0, -1, 32);
  memcpy(task->kernel_stack, parent->kernel_stack, 0x1000);
  task->regs =
      task->kernel_esp - (parent->kernel_esp - (uintptr_t)parent->regs);
  task->fs_base = parent->fs_base;
  task->gs_base = parent->gs_base;
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
  PUSH(0x0);                 /* eax */
  PUSH(0x0);                 /* ecx */
  PUSH(0x0);                 /* edx */
  PUSH(0x0);                 /* ebx */
  PUSH(tmp);                 /* esp */
  PUSH(0x0);                 /* ebp */
  PUSH(0x0);                 /* esi */
  PUSH(0x0);                 /* edi */
  PUSH(GDT_KERNEL_DATA_SEL); /* ds */
  PUSH(GDT_KERNEL_DATA_SEL); /* es */
  PUSH(GDT_GSBASE_SEL(0));   /* gs */
  PUSH(GDT_FSBASE_SEL(0));   /* fs */

#undef PUSH
  task->regs = (struct registers *)stack;
  task->stack = stack;
  task->gs_base = task->fs_base = 0x0;
  task->binfo.entry = entry;

  task->filetable = kmalloc(sizeof(struct fs_node *) * TASK_FILETABLE_MAX);
  task_setup_filetable(task->filetable);

  return task;
}

pid_t current_pid = 0;
static struct list free_pid;

void proc_manager_init() { list_init(&free_pid); }

pid_t generate_pid() {
  pid_t pid;
  if (free_pid.length) {
    pid = list_item(free_pid.first, pid_t);
    list_pop_front(&free_pid);
  } else
    pid = current_pid++;

  return pid;
}

struct task *thread_create(void *entry, void *stack, pagedir_t pgd,
                           struct fs_node **filetable) {
  struct task *thread =
      create_task(entry, pgd, false, stack + PAGE_SIZE, STACK_SIZE);
  thread->filetable = filetable;

  thread->thid =
      list_item(current_task->process->threads->last, struct task *)->thid + 1;
  list_push(current_task->process->threads, (list_value_t)thread);
  thread->process = current_task->process;
  return thread;
}

void thread_destroy(struct task *thread) {
  sched_remove(thread);
  kfree(thread->pgd, 4096); // TODO: implement pgd freeing
  kfree(thread, sizeof(*thread));
}

struct process *process_spawn(const char *filepath, struct process *parent) {
  struct fs_node *file = vfs_open(filepath);
  if (!file)
    return NULL;

  struct process *process = kmalloc(sizeof(struct process));
  process->pid = generate_pid();
  process->filepath = strdup(filepath);
  process->state = PROCESS_STARTING;
  process->parent = parent;

  struct task *main_thread = create_dummy_task();
  main_thread->thid = 0;
  // main_thread->parent = NULL; // TODO: remove
  main_thread->process = process;
  main_thread->parent = NULL;
  main_thread->sched_entry = NULL;
  main_thread->filetable = process->filetable =
      kmalloc(sizeof(struct fs_node *) * TASK_FILETABLE_MAX);
  task_setup_filetable(main_thread->filetable);
#if 0
  vfs_seek(file, 0);
  char *buffer = kmalloc(256);
  int n = vfs_read(file, buffer, 256);
  if (n > 2 && buffer[0] == '#' &&
      buffer[1] == '!') { /* TODO: implement shell bang */
  } else
	  elf_load(main_thread, filepath);
  kfree(buffer, 256);
#endif
  elf_loadf(main_thread, file);
  alloc_stack(
      main_thread, (uintptr_t)(VIRT_BASE - STACK_SIZE), STACK_SIZE,
      true); // TODO: convert api to work with both kernel and user tasks
  char *argv[] = {"argv1", "argv2"};
  char *envp[] = {"a=b", "c=d", NULL};

  // TODO: envp and argv
  task_init_kstack(main_thread, VIRT_BASE);
  task_init_stack(main_thread, 2, argv, envp);
  sched_add(main_thread);

  process->threads = list_create();
  list_push(process->threads, (list_value_t)main_thread);
  process->children = list_create();

  if (parent)
    process->parent_entry = list_push(parent->children, (list_value_t)process);
  return process;
}

void process_kill(struct process *process) {
  if (process->children)
    list_foreach(process->children, entry) {
      process_kill(list_item(entry, struct process *));
    }
  list_foreach(process->threads, entry) {
    struct task *thread = list_item(entry, struct task *);
    if (current_task == thread)
      current_task->state = TASK_DYING;
    else
      thread_destroy(thread);
  }
  if (!process->parent)
    panicf("Cannot kill init process");
  list_remove(process->parent->children, process->parent_entry);
  kfree(process->filepath, strlen(process->filepath));
  list_push(&free_pid, process->pid);
  kfree(process, sizeof(struct process));
  if (current_task->process == process)
    reschedule();
}

DECL_SYSCALL0(exit) { process_kill(current_task->process); }
DECL_SYSCALL0(fork) {
  idt_disable_hwinterrupts();
  struct task *main_thread = fork_task(current_task);
  struct process *process = kmalloc(sizeof(struct process));
  main_thread->process = process;
  process->pid = generate_pid();
  process->filetable = main_thread->filetable;
  process->threads = list_create();
  list_push(process->threads, main_thread);
  process->children = list_create();

  process->filepath = strdup(current_task->process->filepath);
  process->parent = current_task->process;
  process->parent_entry = list_push(process->parent->children, process);
  sched_add(main_thread);
  pagedir_t pgd = get_current_pgd();
  activate_pgd(main_thread->pgd);
  main_thread->regs->eax = 0;
  activate_pgd(pgd);
  return process->pid;
}

DECL_SYSCALL1(thcreate, struct sc_thcreate_params *, params) {
  if (!current_task->process)
    return;
  klogf("stack: 0x%x", params->stack);
  params = copy_userspace_ptr(params, sizeof(*params));
  idt_disable_hwinterrupts();
  struct task *thread =
      thread_create(params->entry, params->stack, current_task->pgd,
                    current_task->process->filetable);
  if (!params->join)
    sched_add(thread);
  else {
  } // TODO: implement
    // dump_tasks();
  kfree(params, sizeof(*params));
  return thread->thid;
}

DECL_SYSCALL1(thkill, thid_t, thid) {
  if (!current_task->process)
    return;
  idt_disable_hwinterrupts();
  if (current_task->process->threads->length == 1) {
    process_kill(current_task->process);
  }

  list_foreach(current_task->process->threads, entry) {
    if (list_item(entry, struct task *)->thid == thid) {
      list_remove(current_task->process->threads, entry);

      if (current_task->thid == thid) {
        current_task->state = TASK_DYING;
        reschedule();
      } else
        thread_destroy(list_item(entry, struct task *));
    }
  }
}

DECL_SYSCALL0(thjoin) {
  if (!current_task->process)
    return;

  idt_disable_hwinterrupts();
}

DECL_SYSCALL1(execve, const char *, file) {
  bool v = elf_probe(file);
  if (!v) {
    printk("file: %s\n", file);
    return -EINVAL;
  }
  extern struct task *current_task;
  // printk("AAAAAAAAAAAAAAAAA\n");
  task_setup_filetable(current_task->filetable);
  elf_load(current_task, file);
  current_task->process->filepath = current_task->name;
  task_init_kstack(current_task, (uintptr_t)VIRT_BASE);
  alloc_stack(current_task, VIRT_BASE - STACK_SIZE, STACK_SIZE, true);
  current_task->vm_areas = list_create();

  char **argv = kmalloc(sizeof(char *) * 3);
  argv[0] = current_task->process->filepath;
  argv[1] = "-test";
  argv[2] = "arg";
  char **envp = kmalloc(sizeof(char *) * 2);
  envp[0] = "LC_ALL=en_US";
  envp[1] = NULL;

  for (int i = 0; i < 3; i++) {
    klogf("REAL %s", argv[i]);
  }
  task_init_stack(current_task, 3, argv, envp);

  reschedule_to(current_task);
  // map_addr(current_task->pgd, VIRT_BASE - PAGE_SIZE, 1, true, false);
  // elf_exec(current_task);
}

DECL_SYSCALL2(sleep, unsigned int, seconds, unsigned int, nanos) {
  current_task->state = TASK_SLEEPING;
  extern uint32_t tick;
  current_task->wake_time = tick + seconds * 1e3 + nanos / 1e6;
  // printk("wake: %d cur: %d\n", current_task->wake_time, tick);
  reschedule();
}

#define PRCTL_SET_GS 0x1
#define PRCTL_SET_FS 0x2
DECL_SYSCALL2(prctl, int, option, void *, address) {
  switch (option) {
  case PRCTL_SET_GS:
    current_task->gs_base = address;
    reschedule_to(current_task);
  case PRCTL_SET_FS:
    current_task->fs_base = address;
    reschedule_to(current_task);
  }
}
