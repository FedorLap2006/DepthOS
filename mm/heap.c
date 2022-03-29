
#include "depthos/heap.h"
#include <depthos/logging.h>
#include <depthos/paging.h>
#include <depthos/pgm.h>

// page_t *free_pages = NULL;
// size_t total_page_count = 0;

extern uint32_t _kernel_end;
uint32_t imalloc_ptr = (uint32_t)&_kernel_end;

void *kimalloc(size_t count) {
  if (count < 0)
    return NULL;
  uint32_t tmp = imalloc_ptr;
  imalloc_ptr += count;
  return (void *)tmp;
}

// struct heap_slab heap_metacache;

heap_slab_t heap_cache[256];
int heap_cache_size = 0;

static bool slab_has_addr(heap_slab_t *slab, void *addr) {
  return addr >= slab->start &&
         addr < slab->start +
                    (!slab->large ? HEAP_SLAB_PAGES : HEAP_LGSLAB_PAGES) * 4096;
}

struct heap_slab *kheap_cache_grow(uint16_t size, bool large) {
  klogf("allocating new slab");

  uint32_t slab_size = (!large ? HEAP_SLAB_PAGES : HEAP_LGSLAB_PAGES) * 4096;
  void *start = ADDR_TO_VIRT(pgm_alloc_frame(slab_size / 4096));
  klogf("allocated new slab at 0x%x (size=%d large=%d)", start, size, large);
  heap_slab_freelist_t *first = (heap_slab_freelist_t *)start, *current;

  current = first;
  klogf("freelist size is %d", slab_size / size);
  for (int i = 1; i < slab_size / size; i++) {
    current->next = start + (i * size);
    // klogf("mapping freelist[%d] (current=0x%x current->next=0x%x", i,
    // current, current->next);
    current = current->next;
  }
  klogf("first=0x%x current=0x%x current->next=0x%x", first, current,
        current->next);
  current->next = NULL;
  klogf("first=0x%x current=0x%x current->next=0x%x", first, current,
        current->next);
  heap_cache[heap_cache_size] = (heap_slab_t){
      .start = first,
      .first = first,
      .object_size = size,
      .inuse = 0,
      .large = large,
  };

  return &heap_cache[heap_cache_size++];
}

void kheap_cache_shrink(struct heap_slab *slab) {
  for (int i = 0; i < heap_cache_size; i++) {
    if (heap_cache + i == slab) {
      // pgm_dump();
      pgm_free_frame(ADDR_TO_PHYS(slab->start),
                     (!slab->large ? HEAP_SLAB_PAGES : HEAP_LGSLAB_PAGES));
      // pgm_dump();
      heap_cache_size--;
      if (i != heap_cache_size)
        heap_cache[i] = heap_cache[heap_cache_size];
      klogf("slab[%d] is purged", i);
      return;
    }
  }
}

void kheap_cache_dump() {
  heap_slab_t *slab;
  for (int i = 0; i < heap_cache_size; i++) {
    slab = heap_cache + i;
    klogf("slab[%d]: start=0x%x first=0x%x size=%d inuse=%d large=%d", i,
          slab->start, slab->first, slab->object_size, slab->inuse,
          slab->large);
  }
}

void kheap_init() {
  // heap_metacache = allocate_slab(sizeof(heap_slab_t), true);
  kheap_cache_grow(8, false);
  kheap_cache_grow(16, true);
  kheap_cache_grow(32, false);
}

void *kmalloc(int size) {
  if (size < sizeof(heap_slab_freelist_t))
    return NULL; // TODO: implement allocation of object smaller than freelist
                 // entry
  int fit = 0, fit_idx = -1;
  heap_slab_t *slab;
  klogf("searching for slabs (object_size=%d)", size);
  for (int i = 0; i < heap_cache_size; i++) {
    slab = heap_cache + i;

    if (!slab->first || slab->object_size < size) {
      klogf("slab[%d] does not fit requirements or is empty", i);
      continue;
    }

    if (!fit || slab->object_size < fit) {
      klogf("slab[%d] at 0x%x fits the requirements (prev=%d current=%d)", i,
            slab, fit, slab->object_size);
      fit_idx = i;
      fit = slab->object_size;
      if (fit == size) {
        klogf("slab[%d] exactly matches", i);
        break;
      }
    }
  }
  slab = fit ? heap_cache + fit_idx : kheap_cache_grow(size, false);

  klogf("slab at 0x%x (inuse=%d first={addr=0x%x next=0x%x})", slab,
        slab->inuse, slab->first, slab->first->next);
  slab->inuse++;
  void *ret = slab->first;
  slab->first = slab->first->next;

  klogf("updated slab at 0x%x (inuse=%d first={addr=0x%x next=0x%x})", slab,
        slab->inuse, slab->first, slab->first->next);
  return ret;
}

void kfree(void *addr, size_t size) {
  int i = 0;
  heap_slab_t *slab;
  for (int i = 0; i < heap_cache_size; i++) {
    slab = heap_cache + i;
    if (slab->object_size >= size && slab_has_addr(slab, addr)) {
      klogf("found original slab (index=%d object_size=%d start=0x%x)", i,
            slab->object_size, slab->start);
      klogf("slab metadata: inuse=%d first=0x%x first->next=0x%x", slab->inuse,
            slab->first, slab->first ? slab->first->next : NULL);
      slab->inuse--;

      if (!slab->first) {
        klogf("slab has no freelist items");
        slab->first = addr;
        slab->first->next = NULL;
      } else if (!slab->inuse) {
        klogf("slab is empty");
        kheap_cache_shrink(slab);
        // slab->first = slab->start;
        // slab->first->next = NULL;
      } else {
        heap_slab_freelist_t *ptr = addr;
        ptr->next = slab->first;
        slab->first = ptr;
      }

      klogf("updated slab metadata: inuse=%d first=0x%x first->next=0x%x",
            slab->inuse, slab->first, slab->first ? slab->first->next : NULL);

      return;
    }
  }
}
