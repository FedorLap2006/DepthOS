#include <depthos/bitmap.h>
#include <depthos/heap.h>
#include <depthos/list.h>
#include <depthos/logging.h>
#include <depthos/pmm.h>
#include <depthos/syscall.h>

#define TEST(name)                                                             \
  printk("\n\x1B[96;1m%s: Executing %s\x1B[0m\n", __func__, name);
#define assert(expr)                                                           \
  if (!(expr))                                                                 \
    panicf("%s: Assertion failed (%s)", __func__, #expr);                      \
  else                                                                         \
    printk("\x1B[32;1m%s: Assertion passed (%s)\x1B[0m\n", __func__, #expr);

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

void test_list() {
  struct list *list = list_create();
  TEST("insert_value to empty list");
  assert(list != NULL);
  assert(list->first == NULL);
  assert(list->last == NULL);
  assert(list->length == 0);
  struct list_entry *first = list_insert_value(list, NULL, NULL, 0xDEADC0DE);
  assert(first != NULL);
  assert(list->first == first);
  assert(list->first == list->last);
  assert(list->first->next == NULL);
  assert(list->first->prev == NULL);
  assert(list->length == 1);
  assert(list->first->value == 0xDEADC0DE);

  // list_foreach(list, item) {
  //   printk("{next=0x%x prev=0x%x value=0x%llx} at 0x%x\n", item->next,
  //          item->prev, list_item(item, uint64_t), (uint32_t)item);
  // }
  TEST("insert_value to the back");
  struct list_entry *second = list_insert_value(list, first, NULL, 0xC0DE);
  assert(second != NULL);
  assert(list->first == first);
  assert(list->last == second);
  assert(list->first->next == second);
  assert(list->last->prev == first);
  assert(list->last->next == NULL);
  assert(list->first->value == 0xDEADC0DE);
  assert(list->last->value == 0xC0DE);
  assert(list->length == 2);
  // list_foreach(list, item) {
  //   printk("{next=0x%x prev=0x%x value=0x%llx} at 0x%x\n", item->next,
  //          item->prev, list_item(item, uint64_t), (uint32_t)item);
  // }

  TEST("remove first element in 2-element list");
  list_remove(list, first);
  assert(list->first == second);
  assert(list->last == second);
  assert(list->length == 1);
  assert(second->next == NULL);
  assert(second->prev == NULL);

  list_insert(list, NULL, second, first);
  assert(list->first == first);
  assert(list->last == second);
  assert(list->first->next == second);
  assert(list->last->prev == first);
  assert(list->last->next == NULL);
  assert(list->first->value == 0xDEADC0DE);
  assert(list->last->value == 0xC0DE);
  assert(list->length == 2);

  TEST("remove second element in 2-element list");
  list_remove(list, second);
  assert(list->first == first);
  assert(list->last == first);
  assert(list->length == 1);
  assert(first->next == NULL);
  assert(first->prev == NULL);

  list_insert(list, first, NULL, second);
  assert(list->first == first);
  assert(list->last == second);
  assert(list->first->next == second);
  assert(list->last->prev == first);
  assert(list->last->next == NULL);
  assert(list->first->value == 0xDEADC0DE);
  assert(list->last->value == 0xC0DE);
  assert(list->length == 2);

  TEST("remove all elements in 2-element list");
  list_remove(list, first);
  list_remove(list, second);
  assert(list->first == NULL);
  assert(list->last == NULL);
  assert(list->length == 0);

  list_insert(list, NULL, NULL, first);
  list_insert(list, first, NULL, second);
  assert(list->first == first);
  assert(list->last == second);
  assert(list->first->next == second);
  assert(list->last->prev == first);
  assert(list->last->next == NULL);
  assert(list->first->value == 0xDEADC0DE);
  assert(list->last->value == 0xC0DE);
  assert(list->length == 2);

  TEST("insert value in-between");
  struct list_entry *third = list_insert_value(list, first, second, 0x64);
  assert(third != NULL);
  assert(list->first == first);
  assert(list->last == second);
  assert(list->first->next == third);
  assert(list->last->prev == third);
  assert(list->last->next == NULL);
  assert(list->length == 3);

  TEST("remove first element from 3-element list");
  list_remove(list, first);
  assert(list->first == third);
  assert(list->last == second);
  assert(list->first->next == second);
  assert(list->last->prev == third);
  assert(list->length == 2);

  list_insert(list, NULL, third, first);
  assert(list->first == first);
  assert(list->last == second);
  assert(list->first->next == third);
  assert(list->last->prev == third);
  assert(list->last->next == NULL);
  assert(list->length == 3);

  TEST("remove last element from 3-element list");
  list_remove(list, second);
  assert(list->first == first);
  assert(list->last == third);
  assert(list->first->next == third);
  assert(list->last->prev == first);
  assert(list->last->next == NULL);
  assert(list->length == 2);

  list_insert(list, third, NULL, second);
  assert(list->first == first);
  assert(list->last == second);
  assert(list->first->next == third);
  assert(list->last->prev == third);
  assert(list->last->next == NULL);
  assert(list->length == 3);

  TEST("remove middle element of 3-element list");
  list_remove(list, third);
  assert(list->first == first);
  assert(list->last == second);
  assert(list->first->next == second);
  assert(list->last->prev == first);
  assert(list->first->prev == NULL);
  assert(list->last->next == NULL);
  assert(list->length == 2);

  list_insert(list, first, second, third);
  assert(list->first == first);
  assert(list->last == second);
  assert(list->first->next == third);
  assert(list->last->prev == third);
  assert(list->last->next == NULL);
  assert(list->length == 3);

  TEST("remove first and last elements of 3 element list");
  list_remove(list, first);
  list_remove(list, second);
  assert(list->first == third);
  assert(list->last == list->first);
  assert(list->first->prev == NULL);
  assert(list->first->next == NULL);
  assert(list->length == 1);

  TEST("list_push");
  struct list_entry *fourth = list_push(list, 0x1);
  assert(list->last == fourth);
  klogf("0x%x 0x%x", list->last->prev, second);
  assert(list->last->prev == third);
  assert(list->last->prev->next == fourth);
  assert(list->last->next == NULL);

  TEST("list_pop");
  list_pop(list);
  assert(list->last == third);
  assert(list->last->next == NULL);

  TEST("list_push_front");
  struct list_entry *fifth = list_push_front(list, 0x2);
  assert(list->first == fifth);
  assert(list->first->next == third);

  TEST("list_pop_front");
  list_pop_front(list);
  assert(list->first == third);

  TEST("list_pop_front on 1 element list");
  list_pop_front(list);
  assert(list->first == NULL);
  assert(list->last == NULL);
  assert(list->length == 0);

  TEST("list_push on empty list");
  first = list_push_front(list, 0x3);
  assert(list->first == first);
  assert(list->last == first);
  assert(list->length == 1);
}

void tests_init() {}

void tests_run() {
  // preempt_disable();
  idt_disable_hwinterrupts();
  // test_bitmap();
  // test_pmm();
  // test_kheap();
  test_list();
  // preempt_enable();
  idt_enable_hwinterrupts();
  sys_exit();
}