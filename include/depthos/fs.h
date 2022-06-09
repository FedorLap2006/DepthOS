#pragma once

#include <depthos/stdtypes.h>


struct fs_operations;
struct filesystem {
  struct fs_operations *ops;
  struct device *dev;
  void *impl;
};
typedef struct fs_operations {
  char *name;
  struct fs_node *(*open)(struct filesystem *fs, const char *path);
  struct filesystem *(*mount)(struct device *dev);
	// void (*unmount)(struct device *dev);
} fs_ops_t;

struct fs_node;
typedef struct file_operations {
  int (*read)(struct fs_node *file, char *buffer, size_t nbytes);
  int (*write)(struct fs_node *file, char *buffer, size_t nbytes);
  long (*ioctl)(struct fs_node *file, unsigned long request, void *data);
	int (*stat)(struct fs_node *file, struct stat *buf);
  void (*close)(struct fs_node *file);
} file_ops_t;

typedef struct fs_node {
  char *name;
  char *path;

#define FS_FILE 0x0001
#define FS_DIR 0x0002
#define FS_MOUNT 0x0003
#define FS_PIPE 0x0004
#define FS_DEV 0x0005
  uint8_t type;
  uint32_t pos;
  bool eof;

  struct file_operations *ops;
  void *impl;
} fs_node_t;

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

#define vfs_write(file, buffer, nbytes) file->ops->write(file, buffer, nbytes)
#define vfs_read(file, buffer, nbytes) file->ops->read(file, buffer, nbytes)
#define vfs_eof(file) file->eof
#define vfs_seek(file, newpos) file->pos = newpos

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
