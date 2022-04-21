#include <depthos/assert.h>
#include <depthos/bitmap.h>
#include <depthos/heap.h>
#include <depthos/logging.h>
#include <depthos/pmm.h>

void test_kheap() {
#define a(n, s)                                                                \
  void *addr_##n = kmalloc(s);                                                 \
  klogf("[%d] allocated %d bytes at 0x%x", n, s, addr_##n);
#define f(n, s)                                                                \
  kfree(addr_##n, s);                                                          \
  klogf("[%d] freed %d bytes at 0x%x", n, s, addr_##n);
  a(0, 16);
  kheap_cache_dump();
  f(0, 16);
  kheap_cache_dump();
  a(1, 16);
  assert(addr_0 == addr_1); // NOTE: works only when only one slab with object
                            // size 16 or higher is present
  kheap_cache_dump();
  a(20, 32);
  assert(addr_20 != addr_1);
  a(2, 16);
  a(3, 16);
  a(4, 16);
  a(5, 16);
  a(6, 16);
  a(7, 16);
  a(8, 16);
  a(9, 16);

  a(10, 16);
  assert(addr_9 != addr_10);
  f(1, 16);
  a(11, 16);
  assert(addr_1 == addr_11);
  f(10, 16);
}

void test_bitmap() {
  char bits[128];
  bitmap_t bm = (struct bitmap){.bits = bits, .size = 10};
  bitmap_init(&bm);
  bitmap_dump(&bm, 0, -1);
  printk("\n");
  bitmap_mark(&bm, 0);
  bitmap_flip(&bm, 1);
  bitmap_dump(&bm, 0, -1);
  printk("\n");
  bitmap_flip(&bm, 1);
  bitmap_flip(&bm, 1);
  bitmap_reset(&bm, 0);
  bitmap_dump(&bm, 0, -1);
  printk("\n");
  bitmap_set(&bm, true, 2);
  bitmap_setmulti(&bm, true, 3, 2);
  bitmap_dump(&bm, 0, -1);
  printk("\n");
  klogf("scan: %d", bitmap_scan(&bm, 0, 3, true));
  klogf("scan: %d", bitmap_scan(&bm, 0, 4, true));
  klogf("scan: %d", bitmap_scan(&bm, 0, 5, true));
  bitmap_dump(&bm, 0, -1);
  printk("\n");
  // klogf("scan flip: %d", bitmap_scan_flip(&bm, 0, 3, false));
  // klogf("scan flip: %d", bitmap_scan_flip(&bm, 0, 4, false));
  klogf("scan flip: %d", bitmap_scan_flip(&bm, 0, 5, false));
  bitmap_dump(&bm, 0, -1);
  printk("\n");
}

void test_pmm() {
  extern bitmap_t pmm_bitmap;
  bitmap_dump_pretty(&pmm_bitmap, 360, 500);
  uint32_t a1 = pmm_alloc(1);
  klogf("alloc (1, 1): 0x%x", a1);
  bitmap_dump_pretty(&pmm_bitmap, 360, 500);
  uint32_t a2 = pmm_alloc(2);
  klogf("alloc (2, 2): 0x%x", a2);
  bitmap_dump_pretty(&pmm_bitmap, 360, 500);
  klogf("free (2, 2)");
  pmm_free(a2, 2);
  bitmap_dump_pretty(&pmm_bitmap, 360, 500);
  uint32_t a3 = pmm_alloc(3);
  klogf("alloc (3, 3): 0x%x", a3);
  bitmap_dump_pretty(&pmm_bitmap, 360, 500);
  klogf("alloc (4, 2): 0x%x", pmm_alloc(2));
  bitmap_dump_pretty(&pmm_bitmap, 360, 500);
  klogf("free (1, 1)");
  pmm_free(a1, 1);
  klogf("free (3, 3)");
  pmm_free(a3, 3);
  bitmap_dump_pretty(&pmm_bitmap, 360, 500);
  klogf("alloc (5, 2): 0x%x", pmm_alloc(2));
  bitmap_dump_pretty(&pmm_bitmap, 360, 500);
}

void tests_init() {}

void tests_run() {
  // test_bitmap();
  // test_pmm();
  // test_kheap();
}