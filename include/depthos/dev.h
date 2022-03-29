#pragma once

#include <depthos/stdtypes.h>

struct filesystem;

typedef struct device {
#define DEV_CHAR 1
#define DEV_BLOCK 2
  uint8_t type;

#define DEV_IFACE_ATA 1
#define DEV_IFACE_TTY 2
  uint8_t iface;

  int (*read)(struct device *dev, void *buffer, size_t count);
  int (*write)(struct device *dev, void *buffer, size_t count);

  uint32_t pos;

  char *name;
  void *impl;

  struct filesystem *fs;
} device_t;