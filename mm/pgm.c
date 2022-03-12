#include "depthos/pgm.h"
#include <depthos/console.h>
#include <depthos/stdbits.h>
#include <depthos/string.h>

extern void *glob_get_mem(size_t);

extern pagedir_t current_pgd;

uint8_t *pgm_bitmap;
size_t bitmap_sz;
size_t bitmap_pgc;

static size_t get_arr_index(size_t idx, size_t c) {
  size_t res;
  res = idx / c;
  if ((idx % c) != 0) {
    res++;
  }
  if (res > 0 && (idx % c) != 0) {
    res--;
  }
  return res;
}

void __pgm_init(size_t mm_sz) {
  bitmap_pgc = mm_sz / 4096;

  bitmap_sz = get_arr_index(bitmap_pgc, 8) + 1;
  if (bitmap_pgc % 8 == 0)
    bitmap_sz--;
  pgm_bitmap = (uint8_t *)glob_get_mem(bitmap_sz);

  memset(pgm_bitmap, 0, bitmap_sz);

  __pgm_dump();
}
page_t *__pgm_alloc(size_t count) {
  page_t *ret = NULL;

  size_t cidx = 0;
  size_t cursz = 0;
  size_t nidx = 0;
  mod_log(__func__, "has started");
  for (cidx = 0; cidx < bitmap_pgc; cidx++) {
    // printk("state[%d](%d:%d) at 0x%x: %d\n",cidx, get_arr_index(cidx,8), cidx
    // % 8, cidx * 4096, pgm_bitmap[get_arr_index(cidx,8)] & (1 << cidx % 8));
    mod_log(__func__, "state[%d](%d:%d) at %d: %d\n", cidx,
            get_arr_index(cidx, 8), cidx % 8, cidx * 4096, __pgm_get_bme(cidx));
    if (cursz >= count) {
      mod_log(__func__, "finded free page(%d), csz=%d", nidx, cursz);
      size_t i;
      for (i = nidx; i < nidx + cursz; i++) { // 0; 0 < 0 + 1
        mod_log(__func__, "marking free page:[%d]", i);
        __pgm_set_bme(i, true);
        mod_log(__func__, "checking marking free page:[%d] %d", i,
                __pgm_get_bme(i));
      }

      ret = get_page(current_pgd, (4096 * nidx));
      // mod_log(__func__,"checking marking free page:[%d] %d", i -
      // 1,__pgm_get_bme(i - 1));
      break;
    }

    if (__pgm_get_bme(cidx) != 0 && cursz < count) {
      mod_log(__func__, "reset indices, last(%d,%d)", nidx, cursz);
      cursz = 0;
      nidx = 0;
    }
    if (__pgm_get_bme(cidx) == 0) {
      //			ret = get_page(current_pgdir,(4096 * cidx));
      //			last_idx =
      //			break;
      mod_log(__func__, "new alloc[%d]", cidx);
      if (nidx == 0) {
        mod_log(__func__, "setup new indices(%d)", cidx);
        nidx = cidx;
      }
      cursz++;
    }
  }
  mod_log(__func__, "has ended");

  return ret;
}

void __pgm_free(page_t *pg, size_t count) {
  size_t cidx;
  pageinfo_t pgi = parse_page(pg);
  for (cidx = 0; cidx < bitmap_pgc; cidx++) {
    if (cidx * 4096 >= pgi.frame && cidx * 4096 <= count * 4096 + pgi.frame) {
      __pgm_set_bme(cidx, false);
    }
  }
}

void __pgm_set_bme(uint32_t idx, bool busy) {
  if (idx >= bitmap_sz)
    return;
  if (busy) {
    pgm_bitmap[get_arr_index(idx, 8)] =
        pgm_bitmap[get_arr_index(idx, 8)] | (1 << idx % 8);
  } else {
    pgm_bitmap[get_arr_index(idx, 8)] =
        pgm_bitmap[get_arr_index(idx, 8)] & ~(1 << idx % 8);
  }
}

void __pgm_set_rpg_addr(uint32_t addr, bool busy) {
  __pgm_set_bme(addr / 4096, busy);
}
void __pgm_set_rpg_desc(page_t *pg, bool busy) {
  __pgm_set_rpg_addr(parse_page(pg).frame, busy);
}

uint8_t __pgm_get_bme(uint32_t idx) {
  return (pgm_bitmap[get_arr_index(idx, 8)] >> (idx % 8)) & 1;
}
uint8_t __pgm_get_rpg_addr(uint32_t addr) { return __pgm_get_bme(addr / 4096); }
uint8_t __pgm_get_rpg_desc(page_t *pg) {
  return __pgm_get_rpg_addr(parse_page(pg).frame);
}

void __pgm_dump() {
  size_t i = 0;
  for (i = 0; i < bitmap_pgc; i++) {
    printk("%d - %d\n", i, pgm_bitmap[get_arr_index(i, 8)] & (1 << i % 8));
  }
}
