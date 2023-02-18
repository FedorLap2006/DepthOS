#include <depthos/dev.h>
#include <depthos/errno.h>
#include <depthos/fs.h>
#include <depthos/heap.h>
#include <depthos/logging.h>
#include <depthos/string.h>

#define DEVFS_MAX_DEVICES 256
struct fs_node devfs_files[DEVFS_MAX_DEVICES];
static int devfs_file_count = 0;

#define DEV(file) ((struct device *)file->impl)

int devfs_read(struct fs_node *file, char *buffer, size_t count,
               off_t *offset) {
  if (DEV(file)->read)
    return DEV(file)->read(DEV(file), buffer, count, offset);
  return ENIMPL;
}

int devfs_write(struct fs_node *file, char *buffer, size_t count,
                off_t *offset) {
  if (DEV(file)->write)
    return DEV(file)->write(DEV(file), buffer, count, offset);
  return ENIMPL;
}

int devfs_ioctl(struct fs_node *file, int request, void *data) {
  if (DEV(file)->ioctl)
    return DEV(file)->ioctl(DEV(file), request, data);
  return ENIMPL;
}

struct file_operations devfs_fileops = (struct file_operations){
    .read = devfs_read,
    .write = devfs_write,
    .ioctl = devfs_ioctl,
    .close = NULL,
};

struct fs_node *devfs_open(struct filesystem *fs, const char *path) {
  path++;
  for (int i = 0; i < devfs_file_count; i++) {
    if (strcmp(devfs_files[i].name, path) == 0)
      return &devfs_files[i];
  }
  return NULL;
}

struct fs_operations devfs_ops = (struct fs_operations){
    .name = "devfs",
    .open = devfs_open,
    .mount = NULL,
};

void devfs_register(const char *name, struct device *dev) {
  if (devfs_file_count >= DEVFS_MAX_DEVICES)
    return;

  char *path = kmalloc(sizeof("/dev/") + strlen(name));
  memcpy(path, "/dev/", sizeof("/dev/") - 1);
  memcpy(path + sizeof("/dev/") - 1, name, strlen(name) + 1);
  devfs_files[devfs_file_count++] = (struct fs_node){
      .name = name,
      .path = path,
      .eof = false,
      .ops = &devfs_fileops,
      .pos = 0,
      .type = FS_DEV,
      .impl = dev,
  };
}

int null_write(struct device *dev, char *buffer, size_t n) { return 0; }

int null_read(struct device *dev, char *buffer, size_t n) {
  memset(buffer, 0, n);
  return n;
}

static struct device dev_null = {
    .name = "null",
    .write = null_write,
    .read = null_read,
    .ioctl = NULL,
};

void devfs_populate() { devfs_register("null", &dev_null); }

void devfs_init() {
  vfs_register(&devfs_ops);
  memset(devfs_files, 0, sizeof(struct device *) * DEVFS_MAX_DEVICES);
}
