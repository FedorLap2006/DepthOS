#pragma once

#include <depthos/stddef.h>
#include <depthos/stdtypes.h>

#define HEAP_SLAB_PAGES 2
#define HEAP_LGSLAB_PAGES 4

#define HEAP_BASE (VIRT_BASE + 1 * 1024 * 4096)

typedef struct heap_slab_freelist {
  struct heap_slab_freelist *next;
} heap_slab_freelist_t;

typedef struct heap_slab {
  void *start;
  heap_slab_freelist_t *first;
  uint16_t object_size;
  uint16_t inuse;
  bool large;
} heap_slab_t;

void *kmalloc(int size);
void kfree(void *addr, size_t size);
void kheap_init();