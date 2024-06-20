#pragma once

#include <depthos/uid.h>
#include <depthos/kernel.h>
#include <depthos/proc-ids.h>
#include <depthos/stdtypes.h>

#define SIGHUP 1
#define SIGINT 2
#define SIGQUIT 3
#define SIGILL 4
#define SIGTRAP 5
#define SIGABRT 6
#define SIGBUS 7
#define SIGFPE 8
#define SIGKILL 9
#define SIGUSR1 10
#define SIGSEGV 11
#define SIGUSR2 12
#define SIGPIPE 13
#define SIGALRM 14
#define SIGTERM 15
#define SIGSTKFLT 16
#define SIGCHLD 17
#define SIGCONT 18
#define SIGSTOP 19
#define SIGTSTP 20
#define SIGTTIN 21
#define SIGTTOU 22
#define SIGURG 23
#define SIGXCPU 24
#define SIGXFSZ 25
#define SIGVTALRM 26
#define SIGPROF 27
#define SIGWINCH 28
#define SIGIO 29
#define SIGPOLL SIGIO
#define SIGPWR 30
#define SIGSYS 31
#define SIGRTMIN 32
#define SIGRTMAX 33
#define SIGCANCEL 34

#define NSIG 34

static const char *const signal_names[] = {
    [SIGHUP] = "SIGHUP",     [SIGINT] = "SIGINT",
    [SIGQUIT] = "SIGQUIT",   [SIGILL] = "SIGILL",
    [SIGTRAP] = "SIGTRAP",   [SIGABRT] = "SIGABRT",
    [SIGBUS] = "SIGBUS",     [SIGFPE] = "SIGFPE",
    [SIGKILL] = "SIGKILL",   [SIGUSR1] = "SIGUSR1",
    [SIGSEGV] = "SIGSEGV",   [SIGUSR2] = "SIGUSR2",
    [SIGPIPE] = "SIGPIPE",   [SIGALRM] = "SIGALRM",
    [SIGTERM] = "SIGTERM",   [SIGSTKFLT] = "SIGSTKFLT",
    [SIGCHLD] = "SIGCHLD",   [SIGCONT] = "SIGCONT",
    [SIGSTOP] = "SIGSTOP",   [SIGTSTP] = "SIGTSTP",
    [SIGTTIN] = "SIGTTIN",   [SIGTTOU] = "SIGTTOU",
    [SIGURG] = "SIGURG",     [SIGXCPU] = "SIGXCPU",
    [SIGXFSZ] = "SIGXFSZ",   [SIGVTALRM] = "SIGVTALRM",
    [SIGPROF] = "SIGPROF",   [SIGWINCH] = "SIGWINCH",
    [SIGIO] = "SIGIO",       [SIGPWR] = "SIGPWR",
    [SIGSYS] = "SIGSYS",     [SIGRTMIN] = "SIGRTMIN",
    [SIGRTMAX] = "SIGRTMAX", [SIGCANCEL] = "SIGCANCEL",
    // XXX: SIGPOLL?
};

struct signal {
  int number;
  int code;
  int errno;

  pid_t pid;
  uid_t uid;
  uintptr_t fault_addr;
  int status;
  long band;
  union {
    int v_int;
    void *v_ptr;
  } queue_val; // sigqueue
};

typedef uint64_t signal_mask_t;

struct signal_task_context {
  signal_mask_t old_mask;

#define SIGNAL_TASK_CONTEXT_REGS                                               \
  _DEFINE_TCTX_RE(uint32_t, eax, eax)                                          \
  _DEFINE_TCTX_RE(uint32_t, ecx, ecx)                                          \
  _DEFINE_TCTX_RE(uint32_t, ebx, ebx)                                          \
  _DEFINE_TCTX_RE(uint32_t, edx, edx)                                          \
  _DEFINE_TCTX_RE(uint32_t, ebp, ebp)                                          \
  _DEFINE_TCTX_RE(uint32_t, esi, esi)                                          \
  _DEFINE_TCTX_RE(uint32_t, edi, edi)                                          \
  _DEFINE_TCTX_RE(uint32_t, eip, eip)                                          \
  _DEFINE_TCTX_RE(uint32_t, esp, useresp)                                      \
  _DEFINE_TCTX_RE(uint32_t, eflags, eflags)                                    \
  _DEFINE_TCTX_RE(uint32_t, cs, cs) \
  _DEFINE_TCTX_RE(uint32_t, ds, ds) \
  _DEFINE_TCTX_RE(uint32_t, ss, ss) \
  _DEFINE_TCTX_RE(uint32_t, es, es) \
  _DEFINE_TCTX_RE(uint32_t, fs, fs) \
  _DEFINE_TCTX_RE(uint32_t, gs, gs) \

#define _DEFINE_TCTX_RE(T, N, RSN) T N;
  SIGNAL_TASK_CONTEXT_REGS
#undef _DEFINE_TCTX_RE

};

struct signal_context {
  // TODO: uc_link
  signal_mask_t mask;
  struct signal_task_context tcontext;
};

struct signal_handler {
#define SIGNAL_DEFAULT_ACTION 0
#define SIGNAL_IGNORE_ACTION (uintptr_t)(-1)
  uintptr_t entry;
  uintptr_t restorer;
#define SIGF_NO_CHILD_NOTIFY 0x1
#define SIGF_NO_CHILD_WAIT 0x2
  int flags;
  signal_mask_t mask; // All signals here will be blocked when this handler is
                      // being executed.
};

struct task;

void signal_send(struct task *task, struct signal s);
void signal_exec(struct task *task, struct signal s);
struct signal *signal_fetch(struct task *t);
