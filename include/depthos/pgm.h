#pragma once

#include <depthos/paging.h>
#include <depthos/stdtypes.h>

void pgm_init(size_t mm_sz);

page_t *pgm_alloc(size_t count);

void pgm_free(page_t *start_page, size_t count);

void pgm_set(uint32_t idx, bool busy);
void pgm_set_addr(uint32_t addr, bool busy);
void pgm_set_desc(page_t *pg, bool busy);

uint8_t pgm_get(uint32_t idx);
uint8_t pgm_get_addr(uint32_t addr);
uint8_t pgm_get_desc(page_t *pg);

void pgm_dump();
