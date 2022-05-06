#pragma once

#include <depthos/idt.h>
#include <depthos/paging.h>
#include <depthos/stdtypes.h>

typedef uint32_t pid_t;
typedef enum {
  PROCESS_KERNEL = 0,
  PROCESS_DRIVER = 1,
  PROCESS_USERSPACE = 3,
} process_privilege_level_t;

struct process {
  pid_t pid;
  char *filepath;
  process_privilege_level_t privilege_level;
  struct process *parent;
  struct task *task;
};

struct exec_binary_info {
  void (*entry)(void);
};

typedef enum process_state {
  PROCESS_STARTING,
  PROCESS_RUNNING,
  PROCESS_EXITING,
} process_state_t;

struct task {
  char *name;
  uint16_t running;
  process_state_t state;
  struct exec_binary_info binfo;
  uintptr_t kernel_stack, kernel_esp, stack;

  pagedir_t pgd;

  struct registers *regs, sys_regs;

  struct task *parent;
};

struct task *create_task(void *entry, pagedir_t pgd, bool do_stack);
void bootstrap_user_task(struct task *task, bool do_stack);
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