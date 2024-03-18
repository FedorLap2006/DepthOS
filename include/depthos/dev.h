#pragma once

#include <depthos/stdtypes.h>

#define MAX_DEVICES 1024
#define DEVICE_STATE(D, T) ((T)D->impl)

typedef uint32_t devid_t;
static inline devid_t make_devid(uint16_t c, uint16_t i) {
  return (c << 16) | i;
}
static inline uint16_t devid_class(devid_t id) {
  return id >> 16;
}
static inline uint16_t devid_index(devid_t id) {
  return id & 0xFFFF;
}
struct device {
#define DEV_CHAR 1
#define DEV_BLOCK 2
  uint8_t type;
#define DEV_C_GENERIC 0
#define DEV_C_TTY 1
  uint16_t class;
  uint16_t idx;
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
