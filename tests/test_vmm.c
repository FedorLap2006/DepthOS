#include <depthos/paging.h>
#include <depthos/stdtypes.h>
#include <depthos/tests.h>

void test_vmm_page_rounding() {
  TEST("VMM page rounding");
#define CASE(addr, size, npages, rounded_size)                                 \
  {                                                                            \
    uintptr_t end = PG_RND_DOWN(addr + size);                                  \
    uintptr_t start = PG_RND_UP(addr);                                         \
    assert((end - start) == rounded_size);                                     \
    assert(((end - start) / PAGE_SIZE) == npages);                             \
  }

  CASE(0x1000 - 2, 4096, 0, 0);
  CASE(0x1000 - 2, 4096 + 2, 1, 4096);
}
