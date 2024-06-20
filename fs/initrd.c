#include <depthos/errno.h>
#include <depthos/file.h>
#include <depthos/fs.h>
#include <depthos/heap.h>
#include <depthos/initrd.h>
#include <depthos/kernel.h>
#include <depthos/logging.h>
#include <depthos/string.h>

initrd_header_t *initrd_headers = NULL;
uint16_t initrd_nheaders = 0;
void *initrd_data = NULL;

#define INODE(impl) *(uint32_t *)impl

void initrd_init(struct multiboot_module *module) {
  void *start = (void *)ADDR_TO_VIRT(module->start);
  initrd_nheaders = *(uint16_t *)start;
  initrd_headers = (initrd_header_t *)(start + sizeof(initrd_nheaders));
  initrd_data = &initrd_headers[initrd_nheaders];
  klogf("initrd: %d headers\n", initrd_nheaders);
  klogf("initrd: %p..%p\n", initrd_headers, module->end);
}

int initrdfs_read(struct fs_node *file, char *buffer, size_t nbytes,
                  off_t *offset) {
  initrd_header_t header = initrd_headers[INODE(file->impl)];
  int i;
  // klogf("offset=%d", header.offset);
  // klogf("header length: %ld offset: %ld nbytes: %ld i=%ld offset_nbytes=%ld",
  // header.length, *offset, nbytes, header.length - *offset,
  // *offset + nbytes);
  for (i = 0; i < header.length - *offset && i < nbytes; i++) {
    // klogf("buffer[%d]: base=0x%x index=%d src=0x%x dst=0x%x", i, initrd_data,
    //       header.offset + i + file->pos,
    //       initrd_data + header.offset + i + file->pos, buffer + i);
    buffer[i] = ((char *)initrd_data)[header.offset + i + *offset];
  }
  // klogf("i: %ld", i);
  *offset += i;
  if (!i)
    file->eof = true;
  return i;
}

// int initrdfs_seek(struct fs_node *file, off_t offset, int whence) {
//   initrd_header_t header = initrd_headers[INODE(file->impl)];
//   switch (whence) {
//   case SEEK_SET:
//     file->pos = offset;
//     break;
//   case SEEK_CUR:
//     file->pos += offset;
//     break;
//   case SEEK_END:
//     file->pos = header.length + offset;
//     break;
//   default:
//     return -EINVAL;
//   }
//   if (file->pos < 0)
//     file->pos = 0;
//   if (file->pos > header.length)
//     file->pos = header.length;
//   return file->pos;
// }

soff_t initrdfs_seek(struct fs_node *file, soff_t pos, int whence) {
  return generic_file_seek_size(
      file, pos, initrd_headers[INODE(file->impl)].length - 1, whence);
}

void initrdfs_close(struct fs_node *file) {
  kfree(file->impl, sizeof(uint32_t));
}

file_ops_t initrdfs_fileops = (file_ops_t){
    .read = initrdfs_read,
    .write = NULL,
    .close = initrdfs_close,
    .seek = initrdfs_seek,
};

struct fs_node *initrdfs_open(struct filesystem *fs, const char *path) {
  for (int i = 0; i < initrd_nheaders; i++) {
    if (strcmp(initrd_headers[i].name, path) == 0) {
      struct fs_node *file = (fs_node_t *)kmalloc(sizeof(struct fs_node));
      file->type = FS_FILE;
      file->ops = &initrdfs_fileops;
      file->pos = 0;
      file->impl = kmalloc(sizeof(uint32_t));
      INODE(file->impl) = i;
      return file;
    }
  }
  return NULL;
}

fs_ops_t initrdfs = (fs_ops_t){
    .name = "initrd",
    .open = initrdfs_open,
    .mount = NULL,
};

void initrdfs_init() { vfs_register(&initrdfs); }
