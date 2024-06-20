#include <depthos/dev.h>
#include <depthos/errno.h>
#include <depthos/fs.h>
#include <depthos/heap.h>
#include <depthos/logging.h>
#include <depthos/string.h>
#include <depthos/vmm.h>

struct fs_node devfs_files[MAX_DEVICES];
static int devfs_file_count = 0;

#define DEV(file) ((struct device *)file->impl)

int devfs_read(struct fs_node *file, char *buffer, size_t count,
               off_t *offset) {
  if (!DEV(file)->read)
    return -ENIMPL;

  return DEV(file)->read(DEV(file), buffer, count, offset);

  // int ret;
  // // TODO: nonblocking flags
  // while ((ret = DEV(file)->read(DEV(file), buffer, count, offset)) == -EAGAIN)
  //   sched_yield();
  // return ret;
}

int devfs_write(struct fs_node *file, char *buffer, size_t count,
                off_t *offset) {
  if (!DEV(file)->write)
    return -ENIMPL;

  return DEV(file)->write(DEV(file), buffer, count, offset);

  // int ret;
  // // TODO: nonblocking flags
  // while ((ret = DEV(file)->write(DEV(file), buffer, count, offset)) == -EAGAIN)
  //   sched_yield();
  // return ret;
}

int devfs_ioctl(struct fs_node *file, int request, void *data) {
  if (!DEV(file)->ioctl)
    return -ENIMPL;
  return DEV(file)->ioctl(DEV(file), request, data);
}

soff_t devfs_seek(struct fs_node *file, soff_t pos, int whence) {
  if (!DEV(file)->seek)
    return -ENIMPL;
  return DEV(file)->seek(DEV(file), pos, whence,
                         &file->pos); // XXX: file->pos or something else?
}

int devfs_mmap(struct fs_node *file, struct vm_area *area) {
  if (DEV(file)->mmap)
    return DEV(file)->mmap(DEV(file), area);
  return ENIMPL;
}

struct file_operations devfs_fileops = (struct file_operations){
    .read = devfs_read,
    .write = devfs_write,
    .ioctl = devfs_ioctl,
    .mmap = devfs_mmap,
    .seek = devfs_seek,
    .close = NULL,
};

struct fs_node *devfs_open(struct filesystem *fs, const char *path) {
  klogf("devfs open: %s", path);
  path++;
  for (int i = 0; i < devfs_file_count; i++) {
    klogf("devfs: open: file: %p %d %p", devfs_files[i].name, i,
          devfs_files[i].path);
    if (strcmp(devfs_files[i].name, path) == 0) {
      klogf("devfs file: %s %s", devfs_files[i].path, devfs_files[i].name);
      return &devfs_files[i];
    }
  }
  klogf("devfs open failed");
  return NULL;
}

struct fs_operations devfs_ops = (struct fs_operations){
    .name = "devfs",
    .open = devfs_open,
    .mount = NULL,
};

void devfs_register(const char *name, struct device *dev) {
  if (devfs_file_count >= MAX_DEVICES)
    return;
  klogf("devfs register: %s %p", name, name);
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
  klogf("devfs file count: %d", devfs_file_count);
}

int null_write(struct device *dev, void *buffer, size_t n, off_t *off) {
  *off += n;
  return 0;
}

int null_read(struct device *dev, void *buffer, size_t n, off_t *off) {
  memset(buffer, 0, n);
  *off += n;
  return n;
}

static struct device dev_null = {
    .name = "null",
    .write = null_write,
    .read = null_read,
    .seek = NULL,
    .ioctl = NULL,
};

void devfs_populate() { devfs_register("null", &dev_null); }

void devfs_init() {
  vfs_register(&devfs_ops);
  // memset(devfs_files, 0, sizeof(struct device *) * MAX_DEVICES);
}
