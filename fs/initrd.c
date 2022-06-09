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
  void *start = ADDR_TO_VIRT(module->start);
  initrd_nheaders = *(uint16_t *)start;
  initrd_headers = (initrd_header_t *)(start + sizeof(initrd_nheaders));
  initrd_data = &initrd_headers[initrd_nheaders];
}

int initrdfs_read(struct fs_node *file, char *buffer, size_t nbytes) {
  initrd_header_t header = initrd_headers[INODE(file->impl)];
  int i;
  // klogf("offset=%d", header.offset);
  for (i = 0; i < header.length - file->pos && i < nbytes; i++) {
    // klogf("buffer[%d]: base=0x%x index=%d src=0x%x dst=0x%x", i, initrd_data,
    //       header.offset + i + file->pos,
    //       initrd_data + header.offset + i + file->pos, buffer + i);
    buffer[i] = ((char *)initrd_data)[header.offset + i + file->pos];
  }
  file->pos += i;
  if (!i)
    file->eof = true;
  return i;
}

void initrdfs_close(struct fs_node *file) {
  kfree(file->impl, sizeof(uint32_t));
}

file_ops_t initrdfs_fileops = (file_ops_t){
    .read = initrdfs_read,
    .write = NULL,
    .close = initrdfs_close,
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
