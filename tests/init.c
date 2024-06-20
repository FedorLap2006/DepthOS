#include <depthos/bitmap.h>
#if defined(__i386__) || defined(__i686__)
#include <depthos/x86/gdt.h>
#include <depthos/x86/gsfsbase.h>
#endif
#include <depthos/ata.h>
#include <depthos/fs.h>
#include <depthos/heap.h>
#include <depthos/list.h>
#include <depthos/logging.h>
#include <depthos/pmm.h>
#include <depthos/ringbuffer.h>
#include <depthos/syscall.h>
#include <depthos/tests.h>

#define TEST_KHEAP_ENABLED 0
#define TEST_LIST_ENABLED 1
#define TEST_BITMAP_ENABLED 1
#define TEST_PMM_ENABLED 1
#define TEST_RINGBUFFER_ENABLED 1


#if TEST_KHEAP_ENABLED == 1
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
#endif

#if TEST_BITMAP_ENABLED == 1
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
#endif

#if TEST_PMM_ENABLED == 1
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
#endif

#if TEST_LIST_ENABLED == 1
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
#endif

#if 0
void test_ringbuffer_old() {
  struct ringbuffer *rb = ringbuffer_create(10);
  TEST("ringbuffer_create");
  assert(rb != NULL);
  assert(rb->data != NULL);
  TEST("ringbuffer_init");
  assert(!rb->write_idx);
  assert(!rb->read_idx);

  TEST("ringbuffer_size");
  assert(ringbuffer_size(rb) == 0);
  TEST("ringbuffer_empty");
  assert(ringbuffer_empty(rb));

  TEST("ringbuffer_push");
  ringbuffer_elem_t *rb_p = ringbuffer_push(rb, 0xC0FFEE);
  assert(rb_p == &rb->data[0]);
  assert(*rb_p == 0xC0FFEE);
  assert(rb->write_idx == 1);
  assert(rb->read_idx == 0);
  assert(rb->data[0] == 0xC0FFEE);
  assert(ringbuffer_size(rb) == 1);

  TEST("ringbuffer_pop");
  assert(*ringbuffer_pop(rb) == 0xC0FFEE);
  assert(rb->read_idx == 1);
  assert(rb->read_idx == rb->write_idx);
  assert(ringbuffer_size(rb) == 0);

  TEST("ringbuffer_pop on empty ringbuffer");
  assert(ringbuffer_pop(rb) == NULL);
  assert(ringbuffer_size(rb) == 0);
  assert(rb->read_idx == 1);
  assert(rb->read_idx == rb->write_idx);
  kfree(rb->data, sizeof(*rb->data) * 10);
  kfree(rb, sizeof(*rb));
}
#endif


void test_ringbuffer() {
  struct ringbuffer *rb = ringbuffer_create(10, sizeof(uint32_t));
  TEST("ringbuffer_create");
  assert(rb != NULL);
  assert(rb->data != NULL);
  TEST("ringbuffer_init");
  assert(!rb->write_idx);
  assert(!rb->read_idx);

  TEST("ringbuffer_size");
  assert(ringbuffer_size(rb) == 0);
  TEST("ringbuffer_empty");
  assert(ringbuffer_empty(rb));

  TEST("ringbuffer_push");
  uint32_t val = 0xC0FFEE;
  void *rb_p = ringbuffer_push(rb, &val);
  assert(rb_p == &rb->data[0]);
  assert(*(uint32_t*)rb_p == 0xC0FFEE);
  assert(rb->write_idx == 1);
  assert(rb->read_idx == 0);
  assert(*(uint32_t*)ringbuffer_at(rb, 0) == 0xC0FFEE);
  assert(ringbuffer_size(rb) == 1);

  TEST("ringbuffer_pop");
  assert(*(uint32_t*)ringbuffer_pop(rb) == 0xC0FFEE);
  assert(rb->read_idx == 1);
  assert(rb->read_idx == rb->write_idx);
  assert(ringbuffer_size(rb) == 0);

  TEST("ringbuffer_pop on empty ringbuffer");
  assert(ringbuffer_pop(rb) == NULL);
  assert(ringbuffer_size(rb) == 0);
  assert(rb->read_idx == 1);
  assert(rb->read_idx == rb->write_idx);

  TEST("full buffer");
  for (uint32_t i = 0; i < 10; i++)
    ringbuffer_push(rb, &i);
  
  assert(rb->read_idx == rb->write_idx);
  assert(ringbuffer_size(rb) == 10);

  uint32_t *full_ptr;
  for (uint32_t i = 0; i < 10; i++) {
    full_ptr = ringbuffer_pop(rb);
    assert(*full_ptr == i);
    klogf("%ld", ringbuffer_size(rb));
    assert(ringbuffer_size(rb) == 10-i-1);
  }

  assert(rb->read_idx == rb->write_idx);

  kfree(rb->data, 10 * sizeof(uint32_t));
  kfree(rb, sizeof(*rb));


}

// void ata_test() { TEST("ata identify"); }

#if 0
void print_ata_identify(struct ata_identify *idata) {
  if (!idata) {
    klogf("not exists");
    return;
  }
  klogf("model: %.40s", idata->model_number); // TODO: incorrect
  // for (int i = 0; i < 20; i++) {
  //   klogf("model[%d]: %x", i, idata->model_number[i]);
  // }
  klogf("serial: %.20s", idata->serial_number); // TODO: incorrect
  klogf("num heads: %d", idata->num_heads);
  klogf("num cylinders: %d", idata->num_cylinders);
  klogf("atapi: %d", idata->general_info.atapi);
  klogf("removable: %d", idata->general_info.removable_media);
  klogf("incomplete: %d", idata->general_info.response_incomplete);
  klogf("capabilities: (iordy=%d standby=%d)",
        idata->capabilities.iordy_support,
        idata->capabilities.standy_timer_support);
  // klogf("sectors=%x addressable=%x", idata->current_sector_capacity,
  //       idata->max_addressable_sectors)
  klogf("capacity: %dM", idata->max_addressable_sectors * 512 / 1024 / 1024);
}
// struct ata_port primary_ata =
//     (struct ata_port){.io_base = 0x1F0, .ctl_base = 0x3F6};
// struct ata_port secondary_ata =
//     (struct ata_port){.io_base = 0x170, .ctl_base = 0x376};
void test_ata_old() {

  // klogf("ata disk 0.0: type=%d", ata_identify(&primary, 0));
  // klogf("ata disk 0.1: type=%d", ata_identify(&primary, 1));
  // klogf("ata disk 1.0: type=%d", ata_identify(&secondary, 0));
  // klogf("ata disk 1.1: type=%d", ata_identify(&secondary, 1));
  // klogf("=========================++++++++++++++++++++++++++++++++++++");
  extern struct ata_port *ata_primary_port;
  extern struct ata_port *ata_secondary_port;
  print_ata_identify(ata_identify(ata_primary_port, 0));
  print_ata_identify(ata_identify(ata_primary_port, 1));
  print_ata_identify(ata_identify(ata_secondary_port, 0));
  print_ata_identify(ata_identify(ata_secondary_port, 1));
  struct device *atadrive = create_ata_device(ata_primary_port, 1);
  // ata_drive_select(&primary_ata, 1);

  char *buf = kmalloc(2 * 256 * sizeof(uint16_t));
  memset(buf, 0, 2 * 256 * sizeof(uint16_t));

  strcpy(buf, "hello world! hello ata! hello qemu!!!");
  strcpy(buf + 512, "NOT ELFhello second sector!!!");
  for (int i = 0; i < 2 * 10; i++) {
    klogf("writing: 0x%x", buf[i]);
  }

  // ata_pio_write(&primary_ata, buf, 0, 2);
  atadrive->write(atadrive, buf, 2, &atadrive->pos);
  memset(buf, 0, 2 * 256 * sizeof(uint16_t));
  // ata_pio_read(&primary_ata, buf, 0, 2);
  atadrive->pos = 1;
  atadrive->read(atadrive, buf, 1, &atadrive->pos);

  for (int i = 0; i < 2 * 512; i++) {
    klogf("reading: 0x%x", (unsigned char)buf[i]);
  }
  kfree(buf, 2 * 256 * sizeof(uint16_t));
}

void test_vfs_ata_old() {
  struct file *ata1 = vfs_open("/dev/ata1");
  if (!ata1)
    panicf("What is going on");
  char *buf = kmalloc(2 * 256 * sizeof(uint16_t));
  memset(buf, 0, 2 * 256 * sizeof(uint16_t));
  strcpy(buf, "lol");
  klogf("write: %d", vfs_write(ata1, buf, 2));
  ata1->pos = 0;
  klogf("read: %d", vfs_read(ata1, buf, 2));
  for (int i = 0; i < 2 * 512; i++) {
    klogf("reading from file: 0x%x", (unsigned char)buf[i]);
  }
}

void test_ata() {
  // TEST("identify"); // TODO: pci check
}

#endif

#if defined(__i386__) || defined(__i686__)
void test_gsset() {
  struct gs_test {
    struct gs_test *self;
    int a;
  };
  struct gs_test gstest;
  gstest.self = &gstest;
  // arch_gdt_set_base(6, &gstest);
  x86_set_gsbase((uintptr_t)&gstest);
  unsigned long v;
  __asm__ volatile("movl %0, %%gs" : : "r"(0x30));
  __asm__ volatile("movl %%gs:0, %0" : "=r"(v));
  klogf("%lx %p", v, &gstest);
}
#else
void test_gsset() {}
#endif

void test_vmm_page_rounding();

void tests_init() {}

void tests_run() {
  // preempt_disable();
  // idt_disable_hwinterrupts();
  idt_enable_hwinterrupts();
  test_bitmap();
  // test_pmm();
  // test_kheap();
  test_list();
  test_ringbuffer();
  // test_gsset();
  // test_ata_old();
  // test_vfs_ata_old();
  test_vmm_page_rounding();

  // while (1) {
  //   klogf("lol");
  //   __asm__ volatile("int $0x30");
  // }

  // preempt_enable();
  // idt_enable_hwinterrupts();
  // current_task->state = TASK_DYING;
  // reschedule();
  current_task->state = TASK_DYING;
  // __asm__ volatile("int $0x30");
  sched_yield();
  // sys_exit(0); // TODO: why bash halts if we enable tests
  // sys_exit();
}
