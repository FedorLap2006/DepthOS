#include <depthos/list.h>
#include <depthos/signal.h>
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
  vma_create(task, stack, PG_RND_UP(size) / PAGE_SIZE,
             0); // NOTE: pagefault will not work, as we have not yet
                 // rescheduled to the task
  map_addr(task->pgd, stack, PG_RND_UP(size) / PAGE_SIZE, user, true);
}

static struct task *create_dummy_task() {
  struct task *task = kmalloc(sizeof(struct task));
  task->gs_base = task->fs_base = 0x0;
  task->vm_areas = list_create();
  task->signal_queue = list_create();
  task->signal_mask = 0;
  task->signal_mask_current = 0;
  task->signal_handlers = (struct signal_handler*)kmalloc(sizeof(struct signal_handler) * NSIG);
  memset(task->signal_handlers, 0, sizeof(struct signal_handler) * NSIG);
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

  uintptr_t tmp = (uintptr_t)kstack;
  PUSH(0x0);     /* eax */
  PUSH(0x0);     /* ecx */
  PUSH(0x0);     /* edx */
  PUSH(0x0);     /* ebx */
  PUSH(tmp);     /* esp */
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

void task_init_stack(
    struct task *task, char const **argv,
    char const **envp) { // TODO: multiple architectures,
                         // currently compliant only with i386 sysv abi
  pagedir_t pgd = get_current_pgd();
  activate_pgd(task->pgd);
  char *esp = (void *)task->regs->useresp;
  klogf("esp: %p", esp);

#define PUSH(v, t)                                                             \
  esp -= sizeof(t);                                                            \
  *(t *)esp = (t)v;

#define PUSHP(p) PUSH(p, uintptr_t)
#define PUSHI(ip, l)                                                           \
  esp -= l;                                                                    \
  memcpy(ip, esp, l);
  if (envp) {
    for (int i = 0; envp[i] != NULL; i++) {
      klogf("=>> env='%s' (%p) strlen=%d", envp[i], envp[i], strlen(envp[i]));
      // PUSHI(envp[i], strlen(envp[i]) + 1);
      esp -= strlen(envp[i]) + 1;
      memcpy(esp, envp[i], strlen(envp[i]) + 1);
      klogf("memcpy(esp=%p, env='%s' (%p), l=%d)", esp, envp[i], envp[i],
            strlen(envp[i]) + 1);

      envp[i] = (char *)esp;
    }
  }

  int argc = 0;
  if (argv) {
    for (argc = 0; argv[argc] != NULL; argc++) {
      // esp -= (l = strlen(argv[i] + 1));
      klogf("=>> argv[i]='%s' (%p)", argv[argc], argv[argc]);
      esp -= strlen(argv[argc]) + 1;
      memcpy(esp, argv[argc], strlen(argv[argc]) + 1);
      // PUSHI(argv[i], strlen(argv[i]) + 1);
      klogf("memcpy(esp=%p, argv[i]=%s (%p), l=%d)", esp, argv[argc],
            argv[argc], strlen(argv[argc]) + 1);

      argv[argc] = (char *)esp;
    }
  }

  PUSH(0, uint64_t);
  PUSH(0, uint32_t);
  if (envp) {
    for (int i = 0; envp[i] != NULL; i++) {
      klogf("env=%s (%p)", envp[i], envp[i]);
      PUSHP(envp[i]);
    }
  } else {
    PUSHP(NULL); // XXX: ???
  }

  PUSH(0, uint32_t);

  for (int i = argc - 1; i >= 0; i--) {
    // esp -= (l = strlen(argv[i] + 1));
    klogf("argv[i]=%s (%p)", argv[i], argv[i]);
    PUSHP(argv[i]);
  }

  PUSH(argc, uint32_t);
  // PUSH(0x640046, uint64_t);

  for (int i = 1; i < 64; i++) {
    uint32_t *ptr = (uint32_t *)(task->regs->useresp - i);
    klogf("[0x%x] 0x%x (%d)", ptr, *ptr, esp == (char *)ptr);
  }
  task->regs->useresp = (uint32_t)esp;
  klogf("esp=%p", task->regs->useresp);
#undef PUSHP
#undef PUSH
  activate_pgd(pgd);
}

void task_setup_filetable(struct fs_node **ft) {
  memset(ft, 0, sizeof(struct fs_node *) * TASK_FILETABLE_MAX);
  struct fs_node *tty_file = vfs_open("/dev/tty0");
  ft[0] = tty_file;
  ft[1] = tty_file;
  ft[2] = tty_file;
}

void task_clone_filetable(struct fs_node **dst, struct fs_node **src) {
  for (int i = 0; src[i] != NULL; i++) {
    dst[i] = src[i];
  }
}

// TODO: refactor all usage to execute task_init_stack separately.
struct task *create_task(void *entry, pagedir_t pgd, bool kernel, uintptr_t esp,
                         size_t stack_size) {
  struct task *task = create_dummy_task();
  task->pgd = pgd;
  task->binfo.entry = entry;
  alloc_stack(task, esp - stack_size, stack_size, !kernel);
  if (kernel)
    kernel_task_init_kstack(task, esp);
  else {

    // char *argv[] = {"argv1", "argv2", NULL};
    // char *envp[] = {"a=b", "c=d", NULL};
    // TODO: envp and argv

    task_init_kstack(task, esp);
    // task_init_stack(task, argv, envp);
  }
  return task;
}

struct task *fork_task(struct task *parent) {
  // klogf("============== forking task %s ================\n", parent->name);
  klogf("forking task: %s", parent->name);
  extern bitmap_t kheap_bitmap;
  struct task *task = create_dummy_task();
  klogf("debug %s", parent->name);
  // bitmap_dump_compact(&kheap_bitmap, 0, -1, 32);
  task->name = strdup(parent->name);
  task->state = parent->state;
  task->thid = parent->thid; // TODO: increment thid? or nuke them?
  klogf("huh");
  task->pgd = clone_pgd(parent->pgd);
  klogf("duh");
  task->binfo = parent->binfo;
  task->process = parent->process; // TODO: NULL or keeping parent process?
  task->sched_entry = NULL;
  // task->vm_areas = parent->vm_areas; // TODO: clone pgd memes
  task->vm_areas = list_create();
  list_foreach(parent->vm_areas, item) {
    list_push(task->vm_areas, item->value);
  }
  task->mmap_bump_idx = parent->mmap_bump_idx;
  task->signal_mask = parent->signal_mask;
  task->signal_queue = parent->signal_queue;
  task->signal_mask_current = parent->signal_mask_current;
  memcpy(task->signal_handlers, parent->signal_handlers, sizeof(struct signal_handler) * NSIG);
  klogf("debug 2");

  task->parent = parent;
  task->filetable = kmalloc(sizeof(struct fs_node *) * TASK_FILETABLE_MAX);
  task_clone_filetable(task->filetable, parent->filetable);
  klogf("filetable: %p\n", task->filetable);
  // bitmap_dump_compact(&kheap_bitmap, 0, -1, 32);
  klogf("kernel_stack: %p\n", task->kernel_stack);
  klogf("kernel_stack: *%p\n", &task);
  memcpy(task->filetable, parent->filetable,
         sizeof(struct fs_node *) * TASK_FILETABLE_MAX);
  klogf("kernel_stack: %p\n", task->kernel_stack);
  // bitmap_dump_compact(&kheap_bitmap, 0, -1, 32);
  memcpy((void *)task->kernel_stack, (void *)parent->kernel_stack, KSTACK_SIZE);
  // TODO: remap userspace stack???
  task->regs =
      (struct registers *)(task->kernel_esp -
                           (parent->kernel_esp - (uintptr_t)parent->regs));
  task->fs_base = parent->fs_base;
  task->gs_base = parent->gs_base;
  task->regs->eax = 0;
  return task;
}

struct task *create_kernel_task(void *entry, bool do_stack) {
  struct task *task = create_dummy_task();
  task->pgd = kernel_pgd;
  task->gs_base = task->fs_base = 0x0;
  task->binfo.entry = entry;

  task->filetable = kmalloc(sizeof(struct fs_node *) * TASK_FILETABLE_MAX);
  task_setup_filetable(task->filetable);

  if (!do_stack)
    return;
  uint32_t *stack = (uint32_t *)task->kernel_esp;
  uint32_t tmp;
  // stack -= 4;

#define PUSH(v) *--stack = (uint32_t)v
  PUSH(0x10);  /* ss */
  PUSH(0x0);   /* useresp */
  PUSH(0x202); /* eflags */
  PUSH(0x08);  /* cs */
  PUSH(entry); /* eip */

  // TODO: implement tracing for kernel tasks?
  PUSH(0x0); /* trace_frame */
  PUSH(0);   /* err_code */
  PUSH(0);   /* int_num */

  tmp = (uint32_t)stack;
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
  task->stack = (uintptr_t)stack;
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
      create_task(entry, pgd, false, (uintptr_t)stack + PAGE_SIZE, STACK_SIZE);
  // task_init_stack(task, argv, envp);
  thread->filetable = filetable;

  thread->thid =
      list_item(current_task->process->threads->last, struct task *)->thid + 1;
  list_push(current_task->process->threads, (list_value_t)thread);
  thread->process = current_task->process;
  return thread;
}

void thread_destroy(struct task *thread) {
  sched_remove(thread);
  kfree(thread->name, strlen(thread->name));
  kfree(thread->pgd, 4096); // TODO: implement pgd freeing
  kfree(thread, sizeof(*thread));
}

struct process *
process_spawn(const char *filepath, struct process *parent, char const *argv[],
              char const *envp[]) { // TODO: char *[]argv in other places
  struct fs_node *file = vfs_open(filepath);
  if (!file)
    return NULL;

  struct process *process = kmalloc(sizeof(struct process));
  process->pid = generate_pid();
  process->filepath = strdup(filepath);
  process->cwd = strdup("/");
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
  // char *argv[] = {"argv1", "argv2"};
  // char *envp[] = {"a=b", "c=d", NULL};

  // TODO: envp and argv
  task_init_kstack(main_thread, VIRT_BASE);
  task_init_stack(main_thread, argv, envp);
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

DECL_SYSCALL1(exit, int, status) {
  if (!current_task->process) {
    klogf("we are dying");
    current_task->state = TASK_DYING;
    reschedule();
  }
  klogf("current process exited with %d status", status);
  process_kill(current_task->process); // TODO: status and waiters
}
DECL_SYSCALL0(fork) {
  idt_disable_hwinterrupts();
  struct task *main_thread = fork_task(current_task);
  struct process *process = kmalloc(sizeof(struct process));
  main_thread->process = process;
  process->pid = generate_pid();
  process->state = PROCESS_STARTING;
  process->filetable = main_thread->filetable;
  process->threads = list_create();
  list_push(process->threads, (list_value_t)main_thread);
  process->children = list_create();

  process->filepath = strdup(current_task->process->filepath);
  process->cwd = strdup(current_task->process->cwd);
  process->parent = current_task->process;
  process->parent_entry =
      list_push(process->parent->children, (list_value_t)process);
  sched_add(main_thread);
  pagedir_t pgd = get_current_pgd();
  // activate_pgd(main_thread->pgd);
  main_thread->regs->eax = 0;
  // activate_pgd(pgd);
  klogf("forked task with %d pid", process->pid);
  return process->pid;
}

DECL_SYSCALL1(thcreate, struct sc_thcreate_params *, params) {
  if (!current_task->process)
    return -1;
  klogf("stack: 0x%x", params->stack);
  params = copy_userspace_ptr(params, sizeof(*params));
  idt_disable_hwinterrupts();
  struct task *thread =
      thread_create(params->entry, params->stack, current_task->pgd,
                    current_task->process->filetable);

  // TODO: implement thread joining
  if (!params->join)
    sched_add(thread);

  // dump_tasks();
  kfree(params, sizeof(*params));
  return thread->thid;
}

DECL_SYSCALL1(thkill, thid_t, thid) {
  if (!current_task->process)
    return -1;
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
    return -1;

  idt_disable_hwinterrupts();
}

char const **copy_userspace_null_str_list(struct task *task, char const **src) {
  pagedir_t pgd = get_current_pgd();
  activate_pgd(task->pgd);

  // if src = 0 return a list with null
  // if length = 0 return a list with null
  // otherwise - copy

  if (!src)
    return NULL;
  int length;
  for (length = 0; src[length] != NULL;
       length++) // FIXME: we can go on forever, if userland does not specify a
                 // NULL
    ;
  length++;
  klogf("gotcha %d", length);
  char **dst = kmalloc(sizeof(char *) * length);
  for (int i = 0; i < length - 1; i++) {
    // klogf("%d %d", i, length);
    // klogf("[%d]: %p %p, length", i, src[i], dst[i], length);
    // klogf("[%d]: %s (%p) = %d", i, src[i], src[i], strlen(src[i]));
    klogf("[%d]: %p", i, src[i]);
    klogf("=> %s = %d", src[i],
          strlen(src[i])); // FIXME: for some reason we get here if userspace
                           // doesn't pass NULL
    // TODO: replace kmalloc(strlen) with kmalloc(strlen+1) in other places.
    dst[i] = kmalloc(strlen(src[i]) +
                     1); // HACK: strcpy uses +strlen byte. So, all strcpy
                         // calls with kmalloc(strlen) should be checked
    klogf("dst=%p", dst[i]);
    strcpy(dst[i], src[i]);
  }
  dst[length - 1] = src[length - 1];
  activate_pgd(pgd);
  return dst;
}

// TODO: what else do we need to nuke during an exec?

DECL_SYSCALL3(execve, const char *, file, char const **, argv, char const **,
              envp) {
  klogf("execve %s", file);
  bool v = elf_probe(file);
  if (!v) {
    klogf("probe failed, errno=%d", errno);
    // printk("elf probe failed: %s\n", file);
    if (errno)
      return -errno; // TODO: process does not terminate

    return -ENOEXEC;
  }

  if (argv == NULL || envp == NULL) { // TODO: maybe automatically fill argv?
    return -EFAULT;
  }

  extern struct task *current_task;
  argv = copy_userspace_null_str_list(current_task, argv);
  if (!argv) {
    // TODO: errno and cleanup
    panicf("NULL argv has been passed");
  }
  // FIXME: currently all process threads stay put if we execute another
  // program, how do we go about that in the ideal case ?
  current_task->process->argv = argv;

  klogf("argv: %p", current_task->process->argv[0]);

  for (int i = 0; current_task->process->argv[i] != NULL; i++)
    klogf("process args[%d]: %s", i, current_task->process->argv[i]);

  envp = copy_userspace_null_str_list(current_task, envp);
  if (!envp) {
    // TODO: errno and cleanup
    panicf("NULL envp has been passed");
  }
  current_task->process->envp = envp;

  klogf("envp");

  // printk("AAAAAAAAAAAAAAAAA\n");
  // task_setup_filetable(current_task->filetable);
  // TODO: FD_CLOEXEC

  elf_load(current_task, file);
  current_task->process->filepath =
      strdup(current_task->name); // TODO: what happens in threads?
  task_init_kstack(current_task, (uintptr_t)VIRT_BASE);
  alloc_stack(current_task, VIRT_BASE - STACK_SIZE, STACK_SIZE, true);
  // FIXME: nuke the old ones
  current_task->vm_areas = list_create();
  current_task->signal_queue = list_create();
  memset(current_task->signal_handlers, 0, sizeof(struct signal_handler) * NSIG);

  klogf("argv: %x envp: %x", argv, envp);
  klogf("task: %p process: %p argv: %p", current_task,
        current_task ? current_task->process : 0xC0FFEE,
        current_task->process ? current_task->process->argv : 0xC0FFEE);

  task_init_stack(current_task, argv, envp);

  reschedule_to(current_task);
  // map_addr(current_task->pgd, VIRT_BASE - PAGE_SIZE, 1, true, false);
  // elf_exec(current_task);
}

DECL_SYSCALL2(sleep, unsigned int, seconds, unsigned int, nanos) {
  // klogf("sleeping... sec=%d nano=%d", seconds, nanos);
  current_task->state = TASK_SLEEPING;
  extern uint32_t pit_ticks;
  current_task->wake_time = pit_ticks + seconds * 1e3 + nanos / 1e6;
  // printk("wake: %d cur: %d\n", current_task->wake_time, tick);
  reschedule();
}

#define PRCTL_SET_GS 0x1
#define PRCTL_SET_FS 0x2
DECL_SYSCALL2(prctl, int, option, uintptr_t, address) {
  switch (option) {
  case PRCTL_SET_GS:
    current_task->gs_base = address;
    reschedule_to(current_task);
    break;
  case PRCTL_SET_FS:
    current_task->fs_base = address;
    reschedule_to(current_task);
    break;
  }
}

DECL_SYSCALL0(getpid) { return current_task->process->pid; }
DECL_SYSCALL0(getppid) {
  // klogf("test test %d", current_task->process->parent->pid);
  if (!current_task->parent)
    return 0;
  return current_task->process->parent->pid;
}

DECL_SYSCALL1(chdir, const char*, path) {
  if (!current_task->process)
    return -EFAULT;

  klogf("chdir: %s", path);
  char *cwd = vfs_resolve(path, current_task->process->cwd);
  struct fs_node *f = vfs_open(cwd);
  if (!f)
    return -ENOENT;

  if (f->type != FS_DIR)
    return -ENOTDIR;

  vfs_close(f);

  current_task->process->cwd = cwd;
  return 0;
}

DECL_SYSCALL2(getcwd, char *, buf, size_t, size) {
  if (!current_task->process)
    return -EFAULT;
  if (!buf || (uintptr_t)buf >= VIRT_BASE || !size)
    return -EFAULT;

  if (strlen(current_task->process->cwd) + 1 > size) {
    return -ERANGE;
  }
  strcpy(buf, current_task->process->cwd);
  return 0;
}
