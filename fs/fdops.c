#include <depthos/paging.h>
#include <depthos/ringbuffer.h>
#include <depthos/dev.h>
#include <depthos/errno.h>
#include <depthos/file.h>
#include <depthos/fs.h>
#include <depthos/heap.h>
#include <depthos/idt.h>
#include <depthos/logging.h>
#include <depthos/proc.h>
#include <depthos/pipe.h>
#include <depthos/string.h>
#include <depthos/syscall.h>

#define CHECK_FD(fd)                                                           \
  if (fd < 0)                                                                  \
    return -EINVAL;

#define CHECK_BUF(buf)                                                         \
  if (buf == NULL || (uintptr_t)buf > VIRT_BASE)                               \
    return -EFAULT;

static inline struct fs_node *lookup_file(int fd) {
  if (fd < 0 || fd >= TASK_FILETABLE_MAX) {
    errno = EBADF;
    return NULL;
  }
#if 0
  klogf("current task: %p filetable: %p", current_task,
        current_task->filetable);
  if (fd > 2) {
    for (int i = 0; i < TASK_FILETABLE_MAX; i++) {
      if (!current_task->filetable[i])
        break;
      klogf("filetable: path=(%p - '%s') name=(%p - '%s')",
            current_task->filetable[i]->path, current_task->filetable[i]->path,
            current_task->filetable[i]->name, current_task->filetable[i]->name);
    }
  }
#endif

  struct fs_node *file = current_task->filetable[fd];
  if (!file)
    // errno = EINVAL;
    errno = EBADF;

  return file;
}

DECL_SYSCALL3(write, int, fd, char *, buf, size_t, n) {
  // klogf("writing");
  CHECK_BUF(buf);
  errno = 0;
  struct fs_node *file = lookup_file(fd);
  if (!file)
    return -errno;

  if (file->pipe) {
    if (!file->rdonly)
      return pipe_file_write(file, buf, n, &file->pos);
    else {
      return -ENOSYS;
    }
  }

  // klogf("fd=%d, buf=%p n=%d", fd, buf, n);
  // for(int i = 0; i < n; i++) klogf("%c", buf[i]);
  return vfs_write(file, buf, n);
}

DECL_SYSCALL3(read, int, fd, char *, buf, size_t, n) {
  CHECK_BUF(buf);
  errno = 0;
  struct fs_node *file = lookup_file(fd);
  if (!file)
    return -errno;

  if (file->pipe) {
    if (file->rdonly)
      return pipe_file_read(file, buf, n, &file->pos);
    else
      return -ENOSYS;
  }

  // klogf("read: %ld bytes from %d (at %d)", n, fd, file->pos);
  // klogf("read %ld", n);
  long ret = vfs_read(file, buf, n);
  // klogf("after read");
  return ret;
}

// TODO: current working directory and handling of "." / ".."

static int find_available_fd(void) {
  for (int i = 0; i < TASK_FILETABLE_MAX; i++) {
    if (current_task->filetable[i] == NULL) {
      return i;
    }
  }
  return -1;
}

// void attach_pipe(struct fs_node* file, struct pipe* p) {
//   file->pipe = pipe;
// }


DECL_SYSCALL2(open, const char *, path, int, flags) {
  // klogf("open: %s", path);
  // if (path[0] != '/')
  //   klogf("path is relative");
  if (!current_task->filetable)
    return -ENIMPL;
  if (flags & O_DIR) {
  }

  char *rpath = vfs_resolve(path, current_task->process ? current_task->process->cwd : "/");
  struct fs_node *file = vfs_open(rpath);
  if (!file) {
    klogf("enoent");
    return -ENOENT;
  }
  kfree(rpath, strlen(rpath) + 1);

  if (file->type == FS_PIPE) {
    klogf("opening pipe: %ld", file->inode_num);
    file->pipe = pipe_open(file->dev, file->inode_num);
    klogf("pipe: %p", file->pipe);
    if (flags & FD_RDONLY) file->pipe->n_readers++;
    else file->pipe->n_writers++;
  }

  file->rdonly = (flags & FD_RDONLY) != 0;


  int fd = find_available_fd();
  if (fd < 0) {
    return -EMFILE;
  }
  current_task->filetable[fd] = file;
  return fd;
}

DECL_SYSCALL3(seek, int, fd, soff_t, offset, int, whence) {
  errno = 0;
  // klogf("seek: %d %ld %d", fd, offset, whence);
  struct fs_node *file = lookup_file(fd);
  if (!file)
    return -errno;
  // klogf("seek: fd %d is good", fd);
  // klogf("seek: file->ops=%p file->ops->seek=%p", file->ops,
  //       (file->ops ? file->ops->seek : 0xDEADBEEF));
  return vfs_seek(file, offset, whence);
}

DECL_SYSCALL1(close, int, fd) {
  errno = 0;
  struct fs_node *file = lookup_file(fd);
  if (!file)
    return -errno;

  if (file->pipe)
    pipe_file_close(file);

  vfs_close(file);

  return 0;
}

DECL_SYSCALL3(dup3, int, oldfd, int, newfd, int, flags) {
  errno = 0;
  struct fs_node *file = lookup_file(oldfd);
  if (!file)
    return -errno;

  if (newfd == -1) {
    for (int i = 0; i < TASK_FILETABLE_MAX; i++) {
      if (current_task->filetable[i] == NULL) {
        current_task->filetable[i] = file;
        return i;
      }
    }
    return -EMFILE;
  } else if (newfd < 0 || newfd >= TASK_FILETABLE_MAX) {
    return -EBADF;
  }

  // sys_close(newfd); // TODO: huh?
  current_task->filetable[newfd] = file;
  return newfd;
}

DECL_SYSCALL3(ioctl, int, fd, int, request, void *, data) {
  errno = 0;
  struct fs_node *file = lookup_file(fd);
  if (!file) {
    klogf("hey!");
    return -errno;
  }

  if (!file->ops->ioctl)
    return -ENOSYS;

  klogf("ioctl on %s (%d): req=%d data=(%x)", file->path, fd, request, data);
  return file->ops->ioctl(file, request, data);
}

struct dentry_user {
  uint8_t type;
  inode_t inode;
  off_t offset;
  char name[];
} __pack;

// XXX: mlibc errors __ensure(dir->__ent_next <=
// dir->__ent_limit) when bash doesnt exist?

DECL_SYSCALL3(readentry, int, fd, struct dentry_user *, buf, size_t, size) {
  errno = 0;
  struct fs_node *file = lookup_file(fd);
  if (!file) {
    return -errno;
  }

  if (file->type != FS_DIR) {
    // klogf("type: %d name: %s ptr: %p", file->type, file->name, &file->type);
    return -ENOTDIR;
  }

  if (!file->ops->iter)
    return -ENOSYS;

  if (size < sizeof(struct dentry_user)) {
    return -EINVAL;
  }

  size_t max_name_len = size - sizeof(struct dentry_user);
  struct dentry *tmp = kmalloc(sizeof(struct dentry));
  tmp->name = kmalloc(max_name_len * sizeof(char));
  if (!vfs_iter(file, tmp, max_name_len)) {
    // klogf("iter returns 0");
    return 0;
  }

  *buf = (struct dentry_user){
      .inode = tmp->inode,
      .offset = tmp->offset,
      .type = tmp->type,
  };

  // klogf("name pointer: %p", buf->name);

  size_t name_len = MIN(max_name_len - 1, strlen(tmp->name));
  // klogf("name len: %d %p %d %d", name_len, tmp->name, strlen(tmp->name),
  //       max_name_len - 1);

  memcpy(buf->name, tmp->name, name_len);
  buf->name[name_len] = 0;
  kfree(tmp->name, max_name_len * sizeof(char));
  kfree(tmp, sizeof(struct dentry));
  // return 0;
  return sizeof(struct dentry_user) + name_len + 1; // One is the null
  // symbol.
}

DECL_SYSCALL1(isatty, int, fd) {
  klogf("isatty %d", fd);
  errno = 0;
  struct fs_node *file = lookup_file(fd);
  klogf("errno: %d", errno);
  if (!file)
    return -errno;

  klogf("file type: %d", file->type);
  if (file->type != FS_DEV)
    return -ENOTTY;

  struct device *dev = vfs_fimpl(file, struct device *);
  klogf("class: %d", dev->class);

  if (dev->class != DEV_C_TTY)
    return -ENOTTY;

  return 0;
}

int do_stat(struct fs_node *file, struct stat *buf) {
  // HACK: check buffer for kernel address ranges & unmapped pages.

  klogf("buf: %p", buf);
  *buf = (struct stat){
      .gid = 0, .uid = 0, .size = file->size,
      .mode = stat_make_mode(0777, file->type), // TODO: file type needs to
      .dev = file->dev,
      .inode = file->inode_num,
      // be translated or updated in mlibc abi.
  };
  klogf("%s: %ld", file->path, buf->size);

  return 0;
}

DECL_SYSCALL2(statfd, int, fd, struct stat *, buf) {
  errno = 0;
  struct fs_node *file = lookup_file(fd);
  if (!file)
    return -errno;

  return do_stat(file, buf);
}

DECL_SYSCALL2(stat, const char *, path, struct stat *, buf) {
  klogf("stat: %s", path);
  char *rpath = vfs_resolve(path, current_task->process ? current_task->process->cwd : "/");
  klogf("stat: %s (%s)", path, rpath);
  struct fs_node *file = vfs_open(rpath);
  if (!file)
    return -ENOENT;

  kfree(rpath, strlen(rpath) + 1);

  return do_stat(file, buf);
}

DECL_SYSCALL2(pipe, int*, fds, int, flags) {
  // TODO: verify fds

  struct pipe* p = (struct pipe*)kmalloc(sizeof(*p));
  p->buf = ringbuffer_create(PAGE_SIZE, 1);

  struct fs_node *rf = create_pipe_file(p, false);
  struct fs_node *wf = create_pipe_file(p, true);
 
  int i;

  for (i = 0; i < TASK_FILETABLE_MAX; i++) {
    if (current_task->filetable[i] == NULL) {
      current_task->filetable[i] = rf;
      fds[0] = i;
      break;
    }
  }

  for (i = 0; i < TASK_FILETABLE_MAX; i++) {
    if (current_task->filetable[i] == NULL) {
      current_task->filetable[i] = wf;
      fds[1] = i;
      break;
    }
  }

  if (i == TASK_FILETABLE_MAX) {
    kfree(p, sizeof(*p));
    kfree(rf, sizeof(*rf));
    kfree(wf, sizeof(*wf));
    return -EMFILE;
  }

  klogf("fds: %d %d", fds[0], fds[1]);

  return 0;
}
