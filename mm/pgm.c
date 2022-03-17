#include <depthos/console.h>
#include <depthos/logging.h>
#include <depthos/pgm.h>
#include <depthos/stdbits.h>
#include <depthos/string.h>

extern void *kimalloc(size_t);

extern pagedir_t current_pgd;

uint8_t *pgm_bitmap;
size_t bitmap_size;
size_t pgm_pagecount;
bool pgm_initialised = false;

#define ARRAY_INDEX(idx, c) ((idx) / c)

void pgm_init(size_t memory_size) {
  pgm_pagecount = memory_size / 4096 + (memory_size % 4096 != 0);
  klogf("initialising page manager for %d pages", pgm_pagecount);
  bitmap_size = ARRAY_INDEX(pgm_pagecount - 1, 8) + 1;
  klogf("estimated page manager bitmap size is %d bytes", bitmap_size);
  pgm_bitmap = (uint8_t *)kimalloc(bitmap_size);
  uint32_t upper_addr = kimalloc(0);
  for (int i = 0; i < ADDR_TO_PHYS(upper_addr); i += 4096)
    pgm_set(i >> 12, true);
  pgm_initialised = true;
}

page_t *pgm_alloc(size_t count) {
  if (!count)
    return NULL;

  int current_count = 0, starting_index = 0;
  for (int i = 0; i < pgm_pagecount; i++) {
    if (pgm_get(i)) {
      klogf("page %d is not available, skipping", i);
      current_count = 0;
      continue;
    }
    if (!current_count) {
      klogf("starting_index=%d", i);
      starting_index = i;
    }

    if (++current_count == count) {
      klogf("return condition is met (count=%d index=%d)", current_count,
            starting_index);
      for (int j = starting_index; j <= i; j++) {
        pgm_set(j, true);
      }

      return get_page(current_pgd, starting_index << 12);
    }
  }

  return NULL;
}

void pgm_free(page_t *pg, size_t count) {
  pageinfo_t pgi = parse_page(pg);
  for (int i = pgi.frame >> 12;
       i < pgm_pagecount && i < (pgi.frame >> 12) + count; i++)
    pgm_set(i, false);
}

void pgm_set(uint32_t idx, bool busy) {
  if (idx >= pgm_pagecount)
    return;

  // klogf("value=%d index=%d offset=%d", busy, ARRAY_INDEX(idx, 8), idx % 8);

  if (busy) {
    pgm_bitmap[ARRAY_INDEX(idx, 8)] |= (1 << idx % 8);
  } else {
    pgm_bitmap[ARRAY_INDEX(idx, 8)] &= ~(1 << idx % 8);
  }
}

void pgm_set_addr(uint32_t addr, bool busy) { pgm_set(addr / 4096, busy); }
void pgm_set_desc(page_t *pg, bool busy) {
  pgm_set_addr(parse_page(pg).frame, busy);
}

uint8_t pgm_get(uint32_t idx) {
  return (pgm_bitmap[ARRAY_INDEX(idx, 8)] >> (idx % 8)) & 1;
}
uint8_t pgm_get_addr(uint32_t addr) { return pgm_get(addr / 4096); }
uint8_t pgm_get_desc(page_t *pg) { return pgm_get_addr(parse_page(pg).frame); }

void pgm_dump() {
  size_t i = 0;
  for (i = 0; i < pgm_pagecount; i++) {
    printk("%d: %d\n", i, pgm_get(i));
  }
}
