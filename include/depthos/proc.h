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
  TASK_DYING
} task_state_t;

struct process {
  pid_t pid;
  char *filepath;
  process_state_t state;
  struct process *parent;
  struct list_entry *parent_entry;

  struct list *children;
  struct list *threads;
};

struct exec_binary_info {
  void (*entry)(void);
};
typedef uint32_t thid_t;
struct task {
  char *name;
  task_state_t state;
  uintptr_t kernel_stack, kernel_esp, stack;
  pagedir_t pgd;
  struct registers *regs;

  uint16_t running_time;
  uint16_t running_time_sched;

  thid_t thid;
  struct exec_binary_info binfo;
  struct task *parent;
  struct process *process;
  struct list_entry *sched_entry;
};

struct process *process_spawn(const char *filepath, struct process *parent);
void process_kill(struct process *);

struct task *create_task(void *entry, pagedir_t pgd, bool do_stack, void *stack);
void bootstrap_user_task(struct task *task, bool do_stack, void *stack);
struct task *create_kernel_task(void *entry, bool do_stack);
struct task *create_task_fork(struct task *original);
void reschedule_to(struct task *next);
void reschedule(void);

void sched_add(struct task *);
void sched_remove(struct task *);
void sched_init(void);

void preempt_enable();
void preempt_disable();

extern struct task *current_task;

#define PROC_CLONE_MEM 0x1
struct sc_clone_params {
  int flags;
};

struct sc_thcreate_params {
  int flags;
	bool join;
	void *stack;
  void *entry;
};

