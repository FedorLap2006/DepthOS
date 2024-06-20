#pragma once

#include <depthos/stdtypes.h>

struct vm_area;
struct stat;

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
#define DEV_C_STORAGE 2
#define DEV_C_VIDEO 3
#define DEV_C_PERIPHERAL 3
  uint16_t class;
  uint16_t idx; // Set when device is registered.
  size_t block_size;
  char *name;

  int (*read)(struct device *dev, void *buffer, size_t count, off_t *offset);
  soff_t (*seek)(struct device *dev, soff_t offset, int whence,
                 off_t *pos); // TODO: soff_t fix
  int (*write)(struct device *dev, void *buffer, size_t count, off_t *offset);
  long (*ioctl)(struct device *dev, unsigned long request, void *data); // TODO: pass along fd, might be useful, for grabbing exclusive access for example
  int (*mmap)(struct device *dev, struct vm_area *area);
  int (*stat)(struct device* dev, struct stat* buf);

  off_t pos; // NOTE: may or may not be used for caching current device position
             // after I/O operations. Regardless of offset pointer. Depends on
             // the implementation.
  void *impl;
};



void devfs_init();
void devfs_register(const char *name, struct device *);
void devfs_populate();

void register_device(struct device *);
struct device *get_device(const char *name);

soff_t dev_impl_seek_zero(struct device *dev, soff_t offset, int whence,
                          off_t *pos);


