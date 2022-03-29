#pragma once

#include <depthos/stdtypes.h>

#define MULTIBOOT_FLAG_MEMINFO 1 << 0
#define MULTIBOOT_FLAG_BOOTDEV 1 << 1
#define MULTIBOOT_FLAG_CMDLINE 1 << 2
#define MULTIBOOT_FLAG_MODULES 1 << 3
#define MULTIBOOT_HAS_FEATURE(info, feature)                                   \
  (info->flags & MULTIBOOT_FLAG_##feature)

struct multiboot_module {
  uint32_t start;
  uint32_t end;
  const char *string;
  uint32_t zero;
};

struct multiboot_information {
  uint32_t flags;
  uint32_t mem_lower;
  uint32_t mem_upper;
  uint32_t boot_device;
  const char *cmdline;
  uint32_t mods_count;
  struct multiboot_module *mods;
};
