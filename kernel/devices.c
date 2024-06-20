#include <depthos/dev.h>
#include <depthos/logging.h>
#include <depthos/stdtypes.h>
#include <depthos/string.h>

struct device *devices[MAX_DEVICES];
static int device_count = 0;

void register_device(struct device *dev) {
  if (device_count >= MAX_DEVICES) {
    return; // TODO: return error
  }
  int idx = 0;
  for (int i = 0; i < device_count; i++) {
    if (devices[i]->class == dev->class)
      idx++;
  }
  dev->idx = idx;
  // klogf("name: %p %s", dev->name, dev->name);
  devices[device_count++] = dev;
  devfs_register(dev->name, dev); // TODO: refactor it in here
}

struct device *get_device(const char *name) {
  for (int i = 0; i < device_count; i++)
    if (strcmp(name, devices[i]->name) == 0)
      return devices[i];
  return NULL;
}

soff_t dev_impl_seek_zero(struct device *dev, soff_t offset, int whence,
                          off_t *pos) {
  switch (whence) {
  case SEEK_SET:
  case SEEK_CUR:
  case SEEK_END:
    *pos = 0;
    return 0;
  }
  return -EINVAL;
}
