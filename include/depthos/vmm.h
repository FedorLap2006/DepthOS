#pragma once

#include <depthos/paging.h>
#include <depthos/stdtypes.h>

#define MAP_PRIVATE 0x01
#define MAP_FIXED 0x04
#define MAP_ANON 0x08

struct sc_mmap_params {
  void *addr;
  size_t length;
  int prot;
  int flags;
  int fd;
  off_t offset;
};

void *do_mmap(pagedir_t pgd, void *addr, size_t length, int prot, int flags,
              int fd, off_t offset);