#pragma once

#include <depthos/ksymbols.h>

struct trace_stackframe {
  struct trace_stackframe *next;
  uintptr_t eip;
};

void trace(unsigned skip, unsigned max);