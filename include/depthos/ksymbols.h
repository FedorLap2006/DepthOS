#pragma once

#include <depthos/stdtypes.h>

struct kernel_symbol {
  uintptr_t address;
  char type;
  const char *name;
};

int ksymbols_load(const char *path);
struct kernel_symbol *ksymbols_lookup(uintptr_t addr, bool precise);
