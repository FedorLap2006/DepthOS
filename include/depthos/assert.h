#pragma once

#include <depthos/kernel.h>

#define assert(expr)                                                  \
  if (!(expr)) \
    panicf("Assertion failed: (%s)", #expr);
