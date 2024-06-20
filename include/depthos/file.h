#pragma once

#include <depthos/dev.h>
#include <depthos/stddef.h>
#include <depthos/stdtypes.h>
#include <depthos/uid.h>
#include <depthos/time.h>

struct vm_area;

typedef uint32_t inode_t;

#define FM_OWNER_READ 0400
#define FM_OWNER_WRITE 0200
#define FM_OWNER_EXEC 0100
#define FM_GROUP_READ 040
#define FM_GROUP_WRITE 020
#define FM_GROUP_EXECUTE 010
#define FM_ALL_READ 04
#define FM_ALL_WRITE 02
#define FM_ALL_EXECUTE 01
#define FM_SETUID 04000
#define FM_SETGID 02000
#define FM_STICKY 01000
typedef uint16_t fmode_t;

struct file_operations;
typedef struct fs_node {
  char *name;
  char *path;

#define FS_FILE 0x0001
#define FS_DIR 0x0002
// #define FS_MOUNT 0x0004
#define FS_PIPE 0x0004
#define FS_DEV 0x0008
  uint8_t type;
  fmode_t mode;
  off_t pos;
  bool eof;
  size_t size;
  devid_t dev;
  inode_t inode_num;
  bool rdonly; // TODO: file descriptor

  struct file_operations *ops;
  void *impl;
  struct pipe *pipe;
  struct filesystem *fs;
} fs_node_t;

struct file_descriptor {
  bool present;
  struct fs_node *node;
#define FD_EXEC 1
#define FD_RDONLY 2
#define FD_RDWR 3
#define FD_WRONLY 5
  int flags;
};

static inline struct file_descriptor make_fd(struct fs_node* node, int flags) {
  return (struct file_descriptor) {
    .present = true,
    .node = node,
    .flags = flags,
  };
}

struct dentry {
  uint8_t type;
  inode_t inode;
  off_t offset;
  char *name;
} __pack;

struct stat {
  devid_t storage_dev;
  inode_t inode;
  uint16_t mode;
  int n_hardlinks;
  uid_t uid;
  gid_t gid;
  devid_t dev;
  off_t size;
  struct timespec access_time;
  struct timespec modification_time;
  struct timespec creation_time;
  ssize_t fs_block_size;
  ssize_t n_file_blocks;
};

static inline int stat_make_mode(fmode_t mode, uint8_t ftype) {
  return (mode & 0777) | ((ftype & 0xF) << 12);
}

typedef struct file_operations {
  // XXX: open?
  soff_t (*seek)(struct fs_node *file, soff_t pos,
                 int whence); // TODO: soff_t is a temporary fix, once
                              // off_t supports signed values, remove.
  int (*read)(struct fs_node *file, char *buffer, size_t count, off_t *offset);
  int (*iter)(struct fs_node *file, struct dentry *dest, size_t max_name_len,
              off_t *offset); // NOTE: buffer for name is supplied by the caller
  int (*write)(struct fs_node *file, char *buffer, size_t count, off_t *offset);
  // ssize_t (*seek)(struct fs_node *file, off_t offset, int whence);
  int (*ioctl)(struct fs_node *file, int request, void *data);
  int (*stat)(struct fs_node *file, struct stat *buf);
  void (*close)(struct fs_node *file);
  int (*mmap)(struct fs_node *file, struct vm_area *area);
  // TODO: rename
} file_ops_t;

enum vm_fault_kind generic_file_load_mapped_page(struct vm_area *area,
                                                 uintptr_t vaddr, off_t offset);

soff_t
generic_file_seek_size(struct fs_node *file, soff_t pos, off_t size,
                       int whence); // TODO: soff_t is a temporary fix, once
                                    // off_t supports signed values, remove.
