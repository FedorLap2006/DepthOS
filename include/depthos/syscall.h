#pragma once

#include <depthos/idt.h>
#include <depthos/proc.h>
#include <depthos/tools.h>

#define _SC_PARAM(t, n) long n
#define _SC_CAST(t, n) (t) n
#define _SC_DECL(t, n) t n

#define DECL_SYSCALL(name, nargs, ...)                                         \
  __diag_push(GCC);                                                            \
  __diag_ignore(GCC, "-Wattribute-alias",                                      \
                "Type aliasing is used to sanitize system call arguments");    \
  asmlinkage long sys_##name(__MAP(nargs, _SC_DECL, __VA_ARGS__))              \
      __attribute__((alias(__stringify(__se_sys_##name))));                    \
  static inline long __do_sys_##name(__MAP(nargs, _SC_DECL, __VA_ARGS__));     \
  asmlinkage long __se_sys##name(__MAP(nargs, _SC_PARAM, __VA_ARGS__));        \
  asmlinkage long __se_sys_##name(__MAP(nargs, _SC_PARAM, __VA_ARGS__)) {      \
    long ret = __do_sys_##name(__MAP(nargs, _SC_CAST, __VA_ARGS__));           \
    return ret;                                                                \
  }                                                                            \
  __diag_pop(GCC);                                                             \
  static inline long __do_sys_##name(__MAP(nargs, _SC_DECL, __VA_ARGS__))

#define DECL_SYSCALL0(name, ...) DECL_SYSCALL(name, 0, __VA_ARGS__)
#define DECL_SYSCALL1(name, ...) DECL_SYSCALL(name, 1, __VA_ARGS__)
#define DECL_SYSCALL2(name, ...) DECL_SYSCALL(name, 2, __VA_ARGS__)
#define DECL_SYSCALL3(name, ...) DECL_SYSCALL(name, 3, __VA_ARGS__)
#define DECL_SYSCALL4(name, ...) DECL_SYSCALL(name, 4, __VA_ARGS__)
#define DECL_SYSCALL5(name, ...) DECL_SYSCALL(name, 5, __VA_ARGS__)
#define DECL_SYSCALL6(name, ...) DECL_SYSCALL(name, 6, __VA_ARGS__)
void syscall_handler(regs_t *r);
void posix_syscall_handler(regs_t *r);

void *copy_userspace_ptr(void *ptr, size_t size);

struct sc_thcreate_params;
struct sc_mmap_params;

asmlinkage long sys_write(int fd, char *buf, size_t n);
asmlinkage long sys_read(int fd, char *buf, size_t n);
asmlinkage long sys_ioctl(int fd, int request, void *data);
asmlinkage long sys_close(int fd);
asmlinkage long sys_open(const char *path, int flags);
asmlinkage long sys_dup3(int oldfd, int newfd, int flags);
asmlinkage long sys_exit(int status);
asmlinkage long sys_fork(void);
asmlinkage long sys_execve(const char *file, char const **argv,
                           char const **envp);
asmlinkage long sys_thcreate(struct sc_thcreate_params *params);
asmlinkage long sys_thkill(thid_t thid);
asmlinkage long sys_mmap(struct sc_mmap_params *params);
asmlinkage long sys_prctl(int option, uintptr_t address);
asmlinkage long sys_sleep(unsigned int seconds, unsigned int nanos);

// TODO: others

asmlinkage long sys_isatty(int fd);
