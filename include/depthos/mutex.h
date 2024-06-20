#pragma once

#include <depthos/stdtypes.h>

#define MUTEX_UNLOCKED 0
#define MUTEX_LOCKED 1
typedef uint8_t mutex_t;
#define MUTEX_INIT(m) m = MUTEX_UNLOCKED
#define MUTEX_DECLARE(m) mutex_t m = MUTEX_UNLOCKED

bool mutex_try_acquire(mutex_t *m);
void mutex_acquire_yield(mutex_t *m);
void mutex_release(mutex_t *m);
