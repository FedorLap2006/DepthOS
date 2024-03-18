#pragma once

#include <depthos/idt.h>
#include <depthos/list.h>
#include <depthos/paging.h>
#include <depthos/stdtypes.h>

typedef uint32_t pid_t;

typedef enum process_state {
  PROCESS_STARTING,
  PROCESS_RUNNING,
  PROCESS_SUSPENDED,
  PROCESS_EXITING,
} process_state_t;

typedef enum task_state {
  TASK_STARTING,
  TASK_RUNNING,
  TASK_SUSPENDED,
  TASK_SLEEPING,
  TASK_DYING
} task_state_t;

struct process {
  pid_t pid;
  char *filepath;
  char const **argv;
  char const **envp;
  process_state_t state;
  struct process *parent;
  struct list_entry *parent_entry;

  struct list *children;
  struct list *threads;
#define TASK_FILETABLE_MAX 256
  struct fs_node **filetable;
};

struct exec_binary_info {
  void (*entry)(void);
};
typedef uint32_t thid_t;
struct task {
  char *name;
  task_state_t state;
  thid_t thid;

#define KSTACK_SIZE PAGE_SIZE
  uintptr_t kernel_stack;
  uintptr_t kernel_esp;
#define STACK_SIZE PAGE_SIZE * 5
  uintptr_t stack;
  pagedir_t pgd;
  size_t mmap_bump_idx;
  
  struct registers *regs;
  uintptr_t gs_base, fs_base;

  size_t running_time;
	size_t wake_time;
	
	
  // uint16_t running_time_sched;

  struct exec_binary_info binfo;
  struct task *parent;
  struct process *process;
  struct list_entry *sched_entry;
  struct fs_node **filetable;
};


/**
 * @brief Spawn a process from the given parent process.
 *
 * @param filepath Executable file path (ELF).
 * @param parent Parent process to link to.
 * @param argv Arguments passed to the process.
 * @param envp Environment variables passed to the process.
 * @return struct process* Newly created process.
 */
struct process *process_spawn(const char *filepath, struct process *parent,
                              char const **argv, char const **envp);
/**
 * @brief Kill a process with all it's threads and child processes.
 */
void process_kill(struct process *);

/**
 * @brief Create a task
 *
 * @param entry Address of the task entrypoint
 * @param pgd Address space of the task (page directory)
 * @param kernel Whether the task is executed with kernel privileges
 * @param stack Base of the stack
 * @param stack_size Size of task the stack
 * @return struct task* Newly created task
 */
struct task *create_task(void *entry, pagedir_t pgd, bool kernel,
                         uintptr_t stack, size_t stack_size);

/**
 * @brief Create a task with kernel privileges
 *
 * @param entry Address of the task entrypoint
 * @param do_stack Whether to setup kernel stack for the task.
 * @return struct task* Newly created task
 */
struct task *create_kernel_task(void *entry, bool do_stack);

/**
 * @brief Fork the task
 *
 * @param original Task to fork.
 * @return struct task* Forked task.
 */
struct task *fork_task(struct task *original);

/**
 * @brief Initialise kernel stack for a task.
 *
 * @param task Task to initialise stack for.
 * @param esp Stack pointer.
 */
void task_init_kstack(struct task *task, uintptr_t esp);
/*
 * @brief Initialise kernel stack for a task with kernel privileges.
 *
 * @param task Task to initialise stack for.
 * @param esp Stack pointer.
 */
void kernel_task_init_kstack(struct task *task, uintptr_t esp);

/**
 * @brief Initialise specified filetable.
 *
 * @param ft Filetable to initialise.
 */
void task_setup_filetable(struct fs_node **ft);
/**
 * @brief Create a thread in the current process.
 *
 * @param entry Address of the thread entrypoint.
 * @param stack Thread stack base.
 * @param pgd Address space of the thread (page directory)
 * @param filetable Filetable to use for the thread.
 * @return struct task* Newly created thread.
 */
struct task *thread_create(void *entry, void *stack, pagedir_t pgd,
                           struct fs_node **filetable);
/**
 * @brief Destroy the specified thread.
 *
 * @param thread Thread to destroy.
 */
void thread_destroy(struct task *thread);

/**
 * @brief Reschedule to a particular task.
 *
 * @param next Task to reschedule to.
 */
void reschedule_to(struct task *next);

/**
 * @brief Reschedule to the next task in the queue
 */
void reschedule(void);

/**
 * @brief Add task to the scheduler queue.
 *
 */
void sched_add(struct task *);
/**
 * @brief Remove the task from the scheduler queue.
 *
 * @param task Task to remove.
 */
void sched_remove(struct task *);
void sched_init(void);
void sched_yield(void);

void preempt_enable();
void preempt_disable();

/**
 * @brief Task that is currently being executed.
 *
 */
extern struct task *current_task;

#define SC_CLONE_MEM 0x1
struct sc_clone_params {
  int flags;
};

/**
 * @brief Parameters of the sys_thcreate system call.
 *
 */
struct sc_thcreate_params {
  int flags;
  bool join;
  void *stack;
  void *entry;
};
