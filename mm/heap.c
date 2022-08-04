
#include "depthos/heap.h"
#include <depthos/bitmap.h>
#include <depthos/logging.h>
#include <depthos/paging.h>
#include <depthos/pmm.h>

// page_t *free_pages = NULL;
// size_t total_page_count = 0;

#if CONFIG_HEAP_LOG_ENABLE == 1
#define heap_log(...) klogf(__VA_ARGS__)
#else
#define heap_log(...)
#endif

extern uint32_t _kernel_end;
uintptr_t imalloc_ptr = (uint32_t)&_kernel_end;

void *kimalloc(size_t count) {
  if (count < 0)
    return NULL;
  uint32_t tmp = imalloc_ptr;
  imalloc_ptr += count;

  return (void *)tmp;
}

// struct heap_slab heap_metacache;

bitmap_t kheap_bitmap;
heap_slab_t heap_cache[256];
int heap_cache_size = 0;

static bool slab_has_addr(heap_slab_t *slab, void *addr) {
  return addr >= slab->start &&
         addr < slab->start +
                    (!slab->large ? HEAP_SLAB_PAGES : HEAP_LGSLAB_PAGES) * 4096;
}

struct heap_slab *kheap_cache_grow(uint16_t size, bool large) {
  heap_log("allocating new slab");

  uint32_t slab_size = (!large ? HEAP_SLAB_PAGES : HEAP_LGSLAB_PAGES);
  uint16_t page = bitmap_scan_flip(&kheap_bitmap, 0, slab_size, false);
  void *start = HEAP_BASE + (page << 12);
  heap_log("allocated new slab at 0x%x (size=%d large=%d)", start, size, large);
  // bitmap_dump_compact(&kheap_bitmap, 0, -1, 32);
  heap_slab_freelist_t *first = (heap_slab_freelist_t *)start, *current;

  current = first;
  slab_size *= 4096;
  heap_log("freelist size is %d", slab_size / size);
  for (int i = 1; i < slab_size / size; i++) {
    current->next = start + (i * size);
    // heap_log("mapping freelist[%d] (current=0x%x current->next=0x%x", i,
    // current, current->next);
    current = current->next;
  }
  heap_log("first=0x%x current=0x%x current->next=0x%x", first, current,
           current->next);
  current->next = NULL;
  heap_log("first=0x%x current=0x%x current->next=0x%x", first, current,
           current->next);
  // for (current = first; current != NULL; current = current->next)
  //   heap_log("0x%x 0x%x", current, current->next);
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
      // pmm_dump();
      // heap_log("heap page bitmap before deallocation:");
      // bitmap_dump_compact(&kheap_bitmap, 0, -1, 32);
      bitmap_setmulti(&kheap_bitmap, false,
                      ((uintptr_t)slab->start - HEAP_BASE) / 4096,
                      (!slab->large ? HEAP_SLAB_PAGES : HEAP_LGSLAB_PAGES));
      // heap_log("heap page bitmap after deallocation:");
      // bitmap_dump_compact(&kheap_bitmap, 0, -1, 32);
      // pmm_free_frame(ADDR_TO_PHYS(slab->start),
      //                (!slab->large ? HEAP_SLAB_PAGES : HEAP_LGSLAB_PAGES));
      // pmm_dump();
      heap_cache_size--;
      if (i != heap_cache_size)
        heap_cache[i] = heap_cache[heap_cache_size];
      heap_log("slab[%d] is purged", i);
      return;
    }
  }
}

void kheap_cache_dump() {
  heap_slab_t *slab;
  for (int i = 0; i < heap_cache_size; i++) {
    slab = heap_cache + i;
    printk("slab[%d]: start=0x%x first=0x%x size=%d inuse=%d large=%d\n", i,
           slab->start, slab->first, slab->object_size, slab->inuse,
           slab->large);
  }
}

void kheap_init() {
  // heap_metacache = allocate_slab(sizeof(heap_slab_t), true);
  kheap_bitmap.bits =
      (bitmap_elem_t *)kimalloc(sizeof(bitmap_elem_t) * 2 * 1024);
  heap_log("0x%x", kheap_bitmap.bits);
  kheap_bitmap.size = 2 * 1024;
  bitmap_init(&kheap_bitmap);
  // heap_log("dumping pmm before allocation of heap pages");
  // pmm_dump_compact();
  pmm_set(ADDR_TO_PHYS(HEAP_BASE), 2 * 1024, false);
  // heap_log("dumping pmm after allocation of heap pages");
  // pmm_dump_compact();
  for (uintptr_t ptr = HEAP_BASE; ptr < imalloc_ptr; ptr += 4096) {
    heap_log("marking %p used", ptr * PAGE_SIZE);
    // heap_log("marking 0x%c for heap", ptr);
    bitmap_mark(&kheap_bitmap, ptr >> 12);
    // bitmap_dump_compact(&kheap_bitmap, 0, -1, 32);
  }
  kheap_cache_grow(8, false);
  struct heap_slab *slab = kheap_cache_grow(16, true);
  kheap_cache_grow(32, true);
  kheap_cache_shrink(slab);
  kheap_cache_grow(16, true);
  // kheap_cache_dump();

  bootlog("Kernel heap initialization complete", LOG_STATUS_SUCCESS);
  // kheap_cache_grow(4096, true);
}

void *kmalloc(int size) {
  if (size < sizeof(heap_slab_freelist_t))
    return NULL; // TODO: implement allocation of object smaller than freelist
                 // entry
  // else (size > HEAP_LGSLAB_PAGES*4096)
  //   return NULL;
  int fit = 0, fit_idx = -1;
  heap_slab_t *slab;
  heap_log("searching for slabs (object_size=%d)", size);
  for (int i = 0; i < heap_cache_size; i++) {
    slab = heap_cache + i;

    if (!slab->first || slab->object_size < size) {
      heap_log("slab[%d] does not fit requirements or is empty", i);
      continue;
    }

    if (!fit || slab->object_size < fit) {
      heap_log("slab[%d] at 0x%x fits the requirements (prev=%d current=%d)", i,
               slab, fit, slab->object_size);
      fit_idx = i;
      fit = slab->object_size;
      if (fit == size) {
        heap_log("slab[%d] exactly matches", i);
        break;
      }
    }
  }
  // kheap_cache_dump();
  // bitmap_dump_compact(&kheap_bitmap, 0, -1, 32);
  slab = fit ? heap_cache + fit_idx : kheap_cache_grow(size, size >= 1024);

  heap_log("slab at 0x%x (inuse=%d first={addr=0x%x next=0x%x})", slab,
           slab->inuse, slab->first, slab->first ? slab->first->next : NULL);
  slab->inuse++;
  void *ret = slab->first;
  slab->first = slab->first->next;

  heap_log("updated slab at 0x%x (inuse=%d first={addr=0x%x next=0x%x})", slab,
           slab->inuse, slab->first, slab->first ? slab->first->next : NULL);
  return ret;
}

void kfree(void *addr, size_t size) {
  int i = 0;
  heap_slab_t *slab;
  for (int i = 0; i < heap_cache_size; i++) {
    slab = heap_cache + i;
    if (slab->object_size >= size && slab_has_addr(slab, addr)) {
      heap_log("found original slab (index=%d object_size=%d start=0x%x)", i,
               slab->object_size, slab->start);
      heap_log("slab metadata: inuse=%d first=0x%x first->next=0x%x",
               slab->inuse, slab->first,
               slab->first ? slab->first->next : NULL);
      slab->inuse--;

      if (!slab->first) {
        heap_log("slab has no freelist items");
        slab->first = addr;
        slab->first->next = NULL;
      } else if (!slab->inuse) {
        heap_log("slab is empty");
        // TODO: bitmap
        kheap_cache_shrink(slab);
        // slab->first = slab->start;
        // slab->first->next = NULL;
      } else {
        heap_slab_freelist_t *ptr = addr;
        ptr->next = slab->first;
        slab->first = ptr;
      }

      heap_log("updated slab metadata: inuse=%d first=0x%x first->next=0x%x",
               slab->inuse, slab->first,
               slab->first ? slab->first->next : NULL);

      return;
    }
  }
}
