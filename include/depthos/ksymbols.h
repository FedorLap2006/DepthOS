#pragma once

#include <depthos/fs.h>
#include <depthos/stdtypes.h>

struct kernel_symbol {
  uintptr_t address;
  char type;
  const char *name;
};

int ksymbols_load_file(const char *path);
struct kernel_symbol *ksymbols_lookup(uintptr_t addr, bool precise);
