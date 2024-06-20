#include <depthos/errno.h>
#include <depthos/fs.h>
#include <depthos/heap.h>
#include <depthos/ksymbols.h>
#include <depthos/logging.h>
#include <depthos/string.h>

int parse_hex(int sym) {
  if (sym < '0' || sym > 'f') {
    errno = EINVAL;
    return 0;
  }

  return sym >= 'a' ? 10 + sym - 'a' : sym - '0';
}

#define MAX_KERNEL_SYMBOLS 1024 * 3
struct kernel_symbol kernel_symbols[MAX_KERNEL_SYMBOLS];
int kernel_symbols_length;

int ksymbols_load_file(const char *path) {
  struct fs_node *map_file = vfs_open(path);
  if (!map_file) {
    return -ENOENT;
  }
  klogf("vfs seek: before");
  vfs_seek(map_file, 0, SEEK_SET);
  klogf("vfs seek: after");
  char *buffer = (char *)kmalloc(256);
  char *ptr;
  kernel_symbols_length = 0;
  do {
    kernel_symbols[kernel_symbols_length].address = 0;
    while (vfs_read(map_file, buffer, 1) && *buffer != ' ') {
      errno = 0;
      int v = parse_hex(*buffer);
      if (errno) {
        // klogf("invalid symbol address");
        goto stop;
      }
      kernel_symbols[kernel_symbols_length].address <<= 4;
      kernel_symbols[kernel_symbols_length].address += v;
    }
    if (vfs_eof(map_file))
      break;
    if (!vfs_read(map_file, &kernel_symbols[kernel_symbols_length].type, 1)) {
      klogf("cannot read symbol type for %p (%d)",
            kernel_symbols[kernel_symbols_length].address,
            kernel_symbols_length);
      goto stop;
    }

    map_file->pos++;
    ptr = buffer;
    *ptr = 0;
    while (vfs_read(map_file, ptr, 1) && *ptr != '\n')
      ptr++;

    *ptr = 0;
    kernel_symbols[kernel_symbols_length].name = strdup(buffer);
    kernel_symbols_length++;
  } while (!vfs_eof(map_file));

stop:;
  kfree(buffer, 256);
  vfs_close(map_file);
  return kernel_symbols_length;
}

struct kernel_symbol *ksymbols_lookup(uintptr_t addr, bool precise) {
  for (int i = kernel_symbols_length - 1; i >= 0; i--) {
    if (addr == kernel_symbols[i].address ||
        (!precise && addr > kernel_symbols[i].address))
      return &kernel_symbols[i];
  }
  return NULL;
}
