#include <depthos/assert.h>
#include <depthos/fs.h>
#include <depthos/heap.h>
#include <depthos/initrd.h>
#include <depthos/logging.h>
#include <depthos/string.h>

#define MAX_VFS_MOUNTS 256
static int vfs_mounts_count = 0;
static struct mount *root_mount = NULL;
static struct mount vfs_mounts[MAX_VFS_MOUNTS];
#define MAX_VFS_FILESYSTEMS 10
static int vfs_filesystems_count = 0;
static struct filesystem_operations *vfs_filesystems[MAX_VFS_FILESYSTEMS];

void vfs_init() {
  initrdfs_init();

  vfs_mount("/", vfs_get_filesystem("initrd"), NULL);
}

struct filesystem_operations *vfs_get_filesystem(const char *name) {
  for (int i = 0; i < vfs_filesystems_count; i++)
    if (strcmp(name, vfs_filesystems[i]->name) == 0)
      return vfs_filesystems[i];
  return NULL;
}
void vfs_register_fs(struct filesystem_operations *fs) {
  vfs_filesystems[vfs_filesystems_count++] = fs;
}

struct mount *vfs_mount(const char *path, struct filesystem_operations *fs,
                        struct device *dev) {
  assert(fs != NULL);
  struct filesystem *fsi;
  if (fs->mount) {
    fsi = fs->mount(dev);
    if (!fsi)
      return NULL;
  } else {
    fsi = (struct filesystem *)kmalloc(sizeof(struct filesystem));
    fsi->ops = fs;
    fsi->impl = NULL;
  }
  fsi->dev = dev;
  vfs_mounts[vfs_mounts_count] = (struct mount){
      .path = path,
      .fsi = fsi,
  };
  if (strcmp(path, "/") == 0)
    root_mount = vfs_mounts + vfs_mounts_count;
  return &vfs_mounts[vfs_mounts_count++];
}

#if 0
struct mount* vfs_try_mount(const char *path, struct device *dev) {
  struct mount *ret;
  for (int i = 0; i < vfs_filesystems_count; i++)
    if(ret = vfs_mount(path, vfs_filesystems + i, dev)) return ret;
  return NULL;
}
#endif

bool vfs_unmount(const char *path) {
  if (strcmp(path, "/"))
    root_mount = NULL;
  for (int i = 0; i < vfs_mounts_count; i++) {
    if (strcmp(path, vfs_mounts[i].path) == 0) {
      kfree(vfs_mounts[i].fsi, sizeof(struct filesystem));
      vfs_mounts[i] = vfs_mounts[--vfs_mounts_count];
      return true;
    }
  }
  return false;
}

/**
 * @brief Find filesystem mounted on specified path
 *
 * @param path Path filesystem was mounted on
 * @return The mount, NULL if no filesystem was mounted at the specified path
 * @internal
 */
struct mount *find_mount_exact(const char *path) {
  for (int i = 0; i < vfs_mounts_count; i++)
    if (strcmp(path, vfs_mounts[i].path) == 0) {
      return &vfs_mounts[i];
    }
  return NULL;
}

#define PATH_SEPARATOR '/'

/**
 * @brief Find a mountpoint for the specified path or it's parents
 *
 * @param path Path to find mountpoint for
 * @return The mountpoint
 * @internal
 */
struct mount *find_mount(const char *path) {
  struct mount *ret = root_mount;
  if (strcmp(path, "/") == 0)
    return ret;

  char *str = kmalloc(strlen(path)), *substr;
  memcpy(str, path, strlen(path));
  while ((substr = strrchr(str, PATH_SEPARATOR))) {
    ret = find_mount_exact(str);
    if (ret)
      goto out;
    *substr = 0;
  }
  ret = root_mount;
out:
  kfree(str, strlen(path));
  return ret;
}

struct fs_node *vfs_open(const char *path) {
  struct mount *mountpoint = find_mount(path);
  if (!mountpoint)
    return NULL;
  char *relpath = path;
  if (strlen(path) == mountpoint->path)
    relpath = "/";
  else if (strcmp(mountpoint->path, "/"))
    relpath = path + strlen(mountpoint->path);
  struct fs_node *node = mountpoint->fsi->ops->open(mountpoint->fsi, relpath);
  if (!node)
    return NULL;
  node->path = strdup(path);
  node->name = strdup(strrchr(path, PATH_SEPARATOR) + 1);
  return node;
}

void vfs_close(struct fs_node *file) {
  if (file->ops->close)
    file->ops->close(file);
  kfree(file->path, strlen(file->path));
  kfree(file->name, strlen(file->name));
  kfree(file, sizeof(struct fs_node));
}
