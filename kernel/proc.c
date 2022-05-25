#include <depthos/elf.h>
#include <depthos/errno.h>
#include <depthos/fs.h>
#include <depthos/heap.h>
#include <depthos/proc.h>
#include <depthos/string.h>
#include <depthos/syscall.h>
#include <depthos/logging.h>

pid_t current_pid = 0;
static struct list free_pid;

void proc_manager_init() { list_init(&free_pid); }

struct task *thread_create(void *entry, void *stack, pagedir_t pgd) {
  struct task *thread = create_task(entry, pgd, true, stack);
  thread->thid =
      list_item(current_task->process->threads->last, struct task *)->thid + 1;
  list_push(current_task->process->threads, (list_value_t)thread);
	thread->process = current_task->process;
  return thread;
}

struct process *process_spawn(const char *filepath, struct process *parent) {
  struct fs_node *file = vfs_open(filepath);
  if (!file)
    return NULL;

  struct process *process = kmalloc(sizeof(struct process));
  if (free_pid.length) {
    process->pid = list_item(free_pid.first, pid_t);
    list_pop_front(&free_pid);
  } else
    process->pid = current_pid++;

  process->filepath = strdup(filepath);
  process->state = PROCESS_STARTING;
  process->parent = parent;

  struct task *main_thread = kmalloc(sizeof(struct task)); // TODO
  main_thread->thid = 0;
  // main_thread->parent = NULL; // TODO: remove
  main_thread->process = process;
	main_thread->parent = NULL;
	main_thread->sched_entry = NULL;
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
	_task_init(main_thread);
  elf_loadf(main_thread, file);
  bootstrap_user_task(main_thread, true, VIRT_BASE-0x1000);
  sched_add(main_thread);

  process->threads = list_create();
  list_push(process->threads, main_thread);
  process->children = list_create();

  if (parent)
    process->parent_entry = list_push(parent->children, (list_value_t)process);
}

void process_kill(struct process *process) {
  list_foreach(process->children, entry) {
    process_kill(list_item(entry, struct process *));
  }
  list_foreach(process->threads, entry) {
    struct task *thread = list_item(entry, struct task *);
    if (current_task->process == process && thread->thid == current_task->thid)
      current_task->state = TASK_DYING;
    else
      sched_remove(list_item(entry, struct task *));
    kfree(thread, sizeof(*thread));
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

DECL_SYSCALL0(exit) {
  // extern struct task *current_task;
  // printk("name: %s\n", current_task->name);
  // dump_tasks();
  extern struct task *current_task;
  // dump_tasks();
  current_task->state = TASK_DYING;
  // process_kill(current_task->process);
  // current_task->state = TASK_DYING;
  reschedule();
  // dump_tasks();
  // for (;;)
  //   __asm__ volatile("int $0x30");
  // for (;;)
  //   sys_sched_yield();
  // __asm__ volatile("hlt");
  // sys_sched_yield();
}
DECL_SYSCALL0(fork) {
  idt_disable_hwinterrupts();
  struct task *tsk = create_task_fork(current_task);
  sched_add(tsk);
  reschedule();
}

DECL_SYSCALL1(thcreate, struct sc_thcreate_params *, params) {
  if (!current_task->process)
    return;
	klogf("stack: 0x%x", params->stack);
  params = copy_userspace_ptr(params, sizeof(*params));
  idt_disable_hwinterrupts();
  struct task *thread = thread_create(params->entry, params->stack, current_task->pgd);
	if (!params->join)
		sched_add(thread);
	else {} // TODO: implement 
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
			} else sched_remove(list_item(entry, struct task *));
    }
  }
}

DECL_SYSCALL0(thjoin) {
  if (!current_task->process)
    return;

  idt_disable_hwinterrupts();
}

DECL_SYSCALL1(execve, const char *, file) {
  idt_disable_hwinterrupts();
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