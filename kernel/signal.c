#include <depthos/heap.h>
#include <depthos/list.h>
#include <depthos/assert.h>
#include <depthos/kernel.h>
#include <depthos/paging.h>
#include <depthos/proc.h>
#include <depthos/signal.h>
#include <depthos/syscall.h>

DECL_SYSCALL3(sigaction, int, num, struct signal_handler *, new,
              struct signal_handler *, old) {
  if (num < 1 || num > NSIG) {
    return -EINVAL;
  } else if (num == SIGKILL || num == SIGSTOP) {
    return -EINVAL;
  }


  // if (new && !check_userspace_addr((uintptr_t)new, true))
  //   return -EFAULT;
  // if (old && !check_userspace_addr((uintptr_t)old, true))
  //   return -EFAULT;

  klogf("sigaction(old=%p, new=%p)", old, new);
  // TODO: userspace checks


  if (old)
    *old = current_task->signal_handlers[num];

  if (new)
    current_task->signal_handlers[num] = *new;

  klogf("registered %s signal handler, restorer: %p", signal_names[num], (void*)current_task->signal_handlers[num].restorer);
  return 0;
}

DECL_SYSCALL1(sigrestore, struct signal_context *, ctx) {
  // TODO: verify location

  struct registers *regs = current_task->regs;


  klogf("sigrestore");
  klogf("ctx: %p", ctx);
  klogf("(struct signal_task_context) {");

#if defined(__i386__)
  // TODO: eflags header
  ctx->tcontext.eflags &= 0x218DFF;
  // XXX: maybe separate out restorable regs and read only regs?
  ctx->tcontext.cs = regs->cs;
  ctx->tcontext.ds = regs->ds;
  ctx->tcontext.ss = regs->ss;
  ctx->tcontext.es = regs->es;
  ctx->tcontext.fs = regs->fs;
  ctx->tcontext.gs = regs->gs;
#else
#error "Unexpected architecture"
#endif


#define _DEFINE_TCTX_RE(T, R, RSN) regs->RSN = ctx->tcontext.R; klogf("\t."#R" = 0x%lx,", ctx->tcontext.R);
  SIGNAL_TASK_CONTEXT_REGS
#undef _DEFINE_TCTX_RE

  klogf("}");

  current_task->signal_mask_current = ctx->tcontext.old_mask;

  return;
}

void kill_with_signal(struct task *task, int num) {
  // TODO: figure out the termination codes
  if (task->process) {
    klogf("process %s was killed by %s", task->process->filepath,
          signal_names[num]);
    task->process->state = PROCESS_DEAD;
    task->process->signal = num;
    process_kill(task->process); // XXX: if only one thread received the signal,
                                 // should we actually kill the process
  } else {
    // XXX: and if there is no process... shall we panic? Cus this is usually a kernel
    // thread.
    klogf("task %s was killed by %s", task->name, signal_names[num]);
    task->state = TASK_DYING;
    if (task == current_task)
      reschedule();
  }
}

void default_signal_handler(struct task *task, struct signal s) {
  switch (s.number) {
  case SIGCHLD:
  case SIGURG:
  case SIGWINCH:
    break;
  default:
    kill_with_signal(task, s.number);
  }
}

struct signal_frame {
  int number;
  struct signal *info;
  struct signal_context *ctx;
};

void signal_exec(struct task *task, struct signal s) {
  // TODO: kernel tasks support
  if (task->pgd == kernel_pgd)
    return;

  if (s.number == SIGKILL) {
    kill_with_signal(task, s.number);
    return;
  } else if (s.number == SIGSTOP) {
    task->state = TASK_SUSPENDED;
    return;
  }

  klogf("handlers: %p", task->signal_handlers);

  struct signal_handler h = task->signal_handlers[s.number];
  if (h.entry == SIGNAL_DEFAULT_ACTION) {
    default_signal_handler(task, s);
    return;
  } else if (h.entry == SIGNAL_IGNORE_ACTION) {
    klogf("ignoring %d", s.number);
    return;
  }
  // TODO: ensure esp is ok with all of this

  struct signal_context ctx = (struct signal_context){
      .tcontext =
          (struct signal_task_context){
#define _DEFINE_TCTX_RE(T, N, RSN) .N = task->regs->RSN,
              SIGNAL_TASK_CONTEXT_REGS
#undef _DEFINE_TCTX_RE
          },
      .mask = h.mask | (1 << s.number) | task->signal_mask,
  };

  pagedir_t pgd = get_current_pgd();
  uintptr_t esp = task->regs->useresp; // TODO: map a separate area?
  klogf("esp: %p", (void *)esp);
  activate_pgd(task->pgd);

#define PUSH(v, t)                                                             \
  ({                                                                           \
    esp -= sizeof(t);                                                          \
    *(t *)esp = v;                                                             \
    (t *)esp;                                                                  \
  })

  struct signal_frame frame = (struct signal_frame){
      .info = PUSH(s, struct signal),
      .ctx = PUSH(ctx, struct signal_context),
      .number = s.number,
  };

  PUSH(frame, struct signal_frame);
  klogf("restorer: %p", (void*)h.restorer);
  PUSH(h.restorer, uintptr_t);

#undef PUSH

  activate_pgd(pgd);

  task->regs->useresp = esp;
  task->regs->eip = h.entry;
  task->signal_mask_current = h.mask | (1 << s.number);
}

struct signal *signal_fetch(struct task *t) {
  list_foreach(t->signal_queue, item) {
    struct signal *curr = list_item(item, struct signal *);
    signal_mask_t m = 1 << curr->number;

    if (t->signal_mask & m || t->signal_mask_current & m) {
      continue;
    }

    list_remove(t->signal_queue, item);
    return curr;
  }
  return NULL;
}

void signal_send(struct task *task, struct signal sig) {
  if (sig.number > NSIG || sig.number < 1)
    return;

  if (task->signal_mask_current & (1 << sig.number) ||
      task->signal_mask & (1 << sig.number)) {
    assert(task->signal_queue->length !=
           SIGNAL_MAX_QUEUE_SIZE); // TODO: implement

    struct signal *si = (struct signal *)kmalloc(sizeof(struct signal));
    *si = sig;
    list_push(task->signal_queue, (list_value_t)si);
    return;
  }

  signal_exec(task, sig);
}

DECL_SYSCALL1(sigselfsend, struct signal*, s) {
  if (s) signal_send(current_task, *s);
}

// DECL_SYSCALL1()
