#pragma once

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
  // process_privelege_level_t privilege_level;
  struct task *task;
};

struct binary_info {
  void (*entry)(void);
};

struct task {
  struct binary_info binfo;
  pagedir_t pgd;
};
