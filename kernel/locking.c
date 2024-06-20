#include <depthos/mutex.h>
#include <depthos/proc.h>
#include <depthos/spinlock.h>

bool mutex_try_acquire(mutex_t *m) {
  if (m && *m == MUTEX_UNLOCKED) {
    *m = MUTEX_LOCKED;
    return true;
  }
  return false;
}

void mutex_acquire_yield(mutex_t *m) {
  while (!mutex_try_acquire(m))
    sched_yield();
}
void mutex_release(mutex_t *m) { *m = MUTEX_UNLOCKED; }
