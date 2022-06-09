#include <depthos/errno.h>
#include <depthos/fs.h>
#include <depthos/syscall.h>

DECL_SYSCALL3(write, int, fd, char *, buf, size_t, n) {
  if (fd >= TASK_FILETABLE_MAX)
    return -EINVAL;
  struct fs_node *file = current_task->filetable[fd];
  if (!file)
    return -EINVAL;

  return vfs_write(file, buf, n);
}
DECL_SYSCALL3(read, int, fd, char *, buf, size_t, n) {
  if (fd >= TASK_FILETABLE_MAX)
    return -EINVAL;
  struct fs_node *file = current_task->filetable[fd];
  if (!file)
    return -EINVAL;

  return vfs_read(file, buf, n);
}
DECL_SYSCALL1(open, const char *, path) {
  if (!current_task->filetable)
    return;
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
  if (fd >= TASK_FILETABLE_MAX)
    return -EINVAL;
  struct fs_node *file = current_task->filetable[fd];
  if (!file)
    return -EINVAL;
  vfs_close(file);
}