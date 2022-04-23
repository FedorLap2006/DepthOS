#include <depthos/errno.h>
#include <depthos/fs.h>
#include <depthos/ksymbols.h>
#include <depthos/logging.h>
#include <depthos/string.h>

int parse_hex(int sym) {
  if (sym < '0' || sym > 'f')
    return -1;

  return sym >= 'a' ? 10 + sym - 'a' : sym - '0';
}

struct kernel_symbol kernel_symbols[1024 * 3];
int kernel_symbols_length;

int ksymbols_load(const char *path) {
  struct fs_node *map_file = vfs_open(path);
  if (!map_file) {
    klogf("not found");
    return -ENOENT;
  }
  vfs_seek(map_file, 0);
  char *buffer = kmalloc(256), *ptr;
  do {
    kernel_symbols[kernel_symbols_length].address = 0;
    while (vfs_read(map_file, buffer, 1) && *buffer != ' ') {
      int v = parse_hex(*buffer);
      if (v < 0)
        goto fail;

      kernel_symbols[kernel_symbols_length].address <<= 4;
      kernel_symbols[kernel_symbols_length].address += v;
    }

    if (!vfs_read(map_file, &kernel_symbols[kernel_symbols_length].type, 1))
      goto fail;

    map_file->pos++;
    ptr = buffer;
    *ptr = 0;
    while (vfs_read(map_file, ptr, 1) && *ptr != '\n')
      ptr++;

    *ptr = 0;
    kernel_symbols[kernel_symbols_length].name = strdup(buffer);
    kernel_symbols_length++;
  } while (!vfs_eof(map_file));

  return 0;

fail:
  if (kernel_symbols_length)
    kernel_symbols_length--;
  kfree(buffer, 256);
  vfs_close(map_file);
  // symbols_length = 0;
  return -EINVAL;
}

struct kernel_symbol *ksymbols_lookup(uintptr_t addr) {
  for (int i = kernel_symbols_length - 1; i >= 0; i--)
    if (addr >= kernel_symbols[i].address)
      return &kernel_symbols[i];
  return NULL;
}