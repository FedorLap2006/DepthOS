#pragma once

#include <depthos/stdtypes.h>

struct device {
#define DEV_CHAR 1
#define DEV_BLOCK 2
  uint8_t type;
  size_t block_size;
  char *name;

  int (*read)(struct device *dev, void *buffer, size_t count, off_t *offset);
  int (*seek)(struct device *dev, off_t offset, int whence, off_t *pos);
  int (*write)(struct device *dev, void *buffer, size_t count, off_t *offset);
  long (*ioctl)(struct device *dev, unsigned long request, void *data);

  off_t pos; // NOTE: when device is accessed through an fs_node, its position
             // is used.
  void *impl;
};

void devfs_init();
void devfs_register(const char *name, struct device *);
void devfs_populate();
