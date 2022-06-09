#pragma once

#include <depthos/stdtypes.h>

struct device {
#define DEV_CHAR 1
#define DEV_BLOCK 2
  uint8_t type;
  char *name;

  int (*read)(struct device *dev, void *buffer, size_t count);
  int (*write)(struct device *dev, void *buffer, size_t count);
  long (*ioctl)(struct device *dev, unsigned long request, void *data);

  uint32_t pos;
  void *impl;
};

void devfs_init();
void devfs_register(const char *name, struct device *);
