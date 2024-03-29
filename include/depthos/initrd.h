#pragma once

#include <depthos/multiboot.h>
#include <depthos/stdtypes.h>

#define INITRD_PATH_LENGTH 60

typedef struct initrd_header {
  char name[INITRD_PATH_LENGTH];
  uint32_t length;
  uint32_t offset;
} initrd_header_t;

void initrdfs_init();
void initrd_init(struct multiboot_module *module);
