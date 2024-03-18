#pragma once

#include <depthos/errno.h>
#include <depthos/stdtypes.h>
#include <depthos/tools.h>
#include <depthos/file.h>

struct vm_area;

struct fs_operations;
struct filesystem {
  struct fs_operations *ops;
  struct device *dev;
  void *impl;
};


typedef struct fs_operations {
  char *name;
  struct fs_node *(*open)(struct filesystem *fs, const char *path);
  struct fs_node *(*iopen)(struct filesystem *fs, inode_t inode);
  struct filesystem *(*mount)(struct device *dev);

  // void (*unmount)(struct device *dev);
} fs_ops_t;



struct mount {
  char *path;
  struct filesystem *fsi;
};

/**
 * @brief Mount a filesystem on a device to the specified path
 *
 * @param path Path to mount filesystem to
 * @param fs Filesystem to mount
 * @param dev Device to mount filesystem on
 * @return Created mount, NULL if the filesystem cannot be mounted on the device
 */
struct mount *vfs_mount(const char *path, struct fs_operations *fs,
                        struct device *dev);

#if 0
/**
 * @brief Mount first available filesystem for the device to the specified path
 * 
 * @param path Path to mount filesystem to
 * @param dev Device to mount filesystem on
 * @return Created mount, NULL if no filesystem was able to mount on the device
 */
struct mount* vfs_try_mount(const char* path, struct device* dev);
#endif

/**
 * @brief Unmount a filesystem mounted on the specified path
 *
 * @param path Path to unmount filesystem for
 * @return Whether filesystem has been previously mounted on the specified path
 * and was successfully unmounted
 */
bool vfs_unmount(const char *path);

#define SEEK_CUR 1
#define SEEK_END 2
#define SEEK_SET 3

#define O_DIR 0x1

/**
 * @brief Open a file
 *
 * @param path Path of the file
 * @return File at the specified path
 */
struct fs_node *vfs_open(const char *path);
/**
 * @brief Close the file
 *
 * @param file File to close
 */
void vfs_close(struct fs_node *file);

/**
 * @brief Update the position pointer of a file
 *
 * @param pos Desired position
 * @param max Maximum allowed position (usually file size)
 * @return Updated position on success, otherwise -EINVAL.
 */
soff_t vfs_setpos(struct fs_node *file, soff_t pos, soff_t max);

#define vfs_write(file, buffer, count)                                         \
  SAFE_FNPTR_CALL(file->ops->write, -EINVAL, file, buffer, count, &file->pos)
#define vfs_read(file, buffer, count)                                          \
  SAFE_FNPTR_CALL(file->ops->read, -EINVAL, file, buffer, count, &file->pos)
// #define vfs_seek(file, offset) file->pos = offset
#define vfs_seek(file, offset, whence)                                         \
  SAFE_FNPTR_CALL(file->ops->seek, -EINVAL, file, offset, whence)
#define vfs_eof(file) file->eof
#define vfs_iter(file, dst, len) file->ops->iter(file, dst, len, &file->pos)
#define vfs_fimpl(file, t) ((t)(file)->impl)

/**
 * @brief Find filesystem by the name
 *
 * @param name Name of the filesystem
 * @return Filesystem implementation
 */
struct fs_operations *vfs_get_filesystem(const char *name);
/**
 * @brief Register a filesystem
 *
 * @param fs Filesystem
 * @internal
 */
void vfs_register(struct fs_operations *fs);

void vfs_init();
