#include <depthos/bitmap.h>
#include <depthos/console.h>
#include <depthos/logging.h>
#include <depthos/pmm.h>
#include <depthos/stdbits.h>
#include <depthos/string.h>

extern void *kimalloc(size_t);
extern pagedir_t current_pgd;
struct bitmap pmm_bitmap;
bool pmm_initialised = false;

void pmm_init(size_t memory_size) {
  pmm_bitmap.size = memory_size / 4096 + (memory_size % 4096 != 0);
  klogf("initialising page manager for %d pages", pmm_bitmap.size);
  klogf("estimated page manager bitmap size is %d bytes",
        bitmap_byte_size(&pmm_bitmap));
  pmm_bitmap.bits = (bitmap_elem_t *)kimalloc(bitmap_byte_size(&pmm_bitmap));
  bitmap_init(&pmm_bitmap);
  extern uintptr_t imalloc_ptr;
  // for (int i = 0; i < ADDR_TO_PHYS(imalloc_ptr); i += 4096) {
  //   bitmap_flip(&pmm_bitmap, i >> 12);
  //   klogf("marking 0x%x", ADDR_TO_VIRT(i));
  // }
  for (int i = 0; i < 1024 * 4096; i += 4096) {
    bitmap_flip(&pmm_bitmap, i >> 12);
  }

  pmm_initialised = true;
}

uint32_t pmm_alloc(size_t count) {
  size_t idx = bitmap_scan_flip(&pmm_bitmap, 0, count, false);
  if (idx < 0)
    return idx;

  return idx << 12;
}

void pmm_free(uint32_t frame, size_t count) {
  bitmap_setmulti(&pmm_bitmap, false, frame >> 12, count);
}

void pmm_free_desc(page_t *pg, size_t count) {
  return pmm_free(parse_page(pg).frame, count);
}

void pmm_set(uint32_t frame, size_t count, bool avail) {
  bitmap_setmulti(&pmm_bitmap, !avail, frame >> 12, count);
}

void pmm_dump() { bitmap_dump(&pmm_bitmap, 0, -1); }
void pmm_dump_pretty() { bitmap_dump_pretty(&pmm_bitmap, 0, -1); }
void pmm_dump_compact() { bitmap_dump_compact(&pmm_bitmap, 0, -1, 32); }
