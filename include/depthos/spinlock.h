#pragma once

#include <depthos/proc.h>
#include <depthos/stdtypes.h>
#include <depthos/tools.h>

#define SPINLOCK_UNLOCKED 0
#define SPINLOCK_LOCKED 1
typedef uint8_t spinlock_t;

// XXX: atomic operations?

/* Currently locks are noop, since there is no smp nor kernel preemption. */

static inline void spin_lock_acquire(spinlock_t *lock) {}
static inline void spin_lock_release(spinlock_t *lock) {}
