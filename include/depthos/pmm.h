#pragma once

#include <depthos/paging.h>
#include <depthos/stdtypes.h>

void pmm_init(size_t mm_sz);
void pmm_dump();
void pmm_dump_pretty();
void pmm_dump_compact();

uint32_t pmm_alloc(size_t count);
void pmm_set(uint32_t frame, size_t count, bool avail);
void pmm_free(uint32_t frame, size_t count);
void pmm_free_desc(page_t *pg, size_t count);
extern bool pmm_initialised;