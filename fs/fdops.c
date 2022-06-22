#include <depthos/errno.h>
#include <depthos/fs.h>
#include <depthos/syscall.h>

static inline struct fs_node *lookup_file(int fd) {
  if (fd >= TASK_FILETABLE_MAX) {
    errno = -EINVAL;
    return NULL;
  }
  struct fs_node *file = current_task->filetable[fd];
  if (!file)
    errno = -EINVAL;

  return file;
}

DECL_SYSCALL3(write, int, fd, char *, buf, size_t, n) {
  errno = 0;
  struct fs_node *file = lookup_file(fd);
  if (!file)
    return errno;

  return vfs_write(file, buf, n);
}
DECL_SYSCALL3(read, int, fd, char *, buf, size_t, n) {
  errno = 0;
  struct fs_node *file = lookup_file(fd);
  if (file < 0)
    return (int)file;

  return vfs_read(file, buf, n);
}
DECL_SYSCALL1(open, const char *, path) {
  if (!current_task->filetable)
    return -ENIMPL;
  struct fs_node *file = vfs_open(path);
  for (int i = 0; i < TASK_FILETABLE_MAX; i++) {
    if (current_task->filetable[i] == NULL) {
      current_task->filetable[i] = file;
      return i;
    }
  }
  return -EINVAL;
}

DECL_SYSCALL1(close, int, fd) {
  errno = 0;
  struct fs_node *file = lookup_file(fd);
  if (!file)
    return errno;

  vfs_close(file);
}