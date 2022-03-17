
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
