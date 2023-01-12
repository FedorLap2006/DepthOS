#pragma once

#include <depthos/stdtypes.h>

#define MULTIBOOT_FLAG_MEMINFO 1 << 0
#define MULTIBOOT_FLAG_BOOTDEV 1 << 1
#define MULTIBOOT_FLAG_CMDLINE 1 << 2
#define MULTIBOOT_FLAG_MODULES 1 << 3
#define MULTIBOOT_FLAG_MMAP 1 << 6
#define MULTIBOOT_HAS_FEATURE(info, feature)                                   \
  (info->flags & MULTIBOOT_FLAG_##feature)

struct multiboot_mmap_entry {
  uint32_t size;
  uint64_t addr;
  uint64_t length;
#define MULTIBOOT_MMAP_AREA_AVAILABLE 1
#define MULTIBOOT_MMAP_AREA_RESERVED 2
#define MULTIBOOT_MMAP_AREA_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MMAP_AREA_NVS 4
#define MULTIBOOT_MMAP_AREA_BADRAM 5
  uint32_t type;
} __attribute__((packed));

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
  uint32_t bit_4_5_28;
  uint32_t bit_4_5_32;
  uint32_t bit_4_5_36;
  uint32_t bit_4_5_40;
  uint32_t mmap_length;
  struct multiboot_mmap_entry *mmap;
};

void multiboot_init_early(int magic, struct multiboot_information *boot_ptr);
void multiboot_init(struct multiboot_information *boot_ptr);
