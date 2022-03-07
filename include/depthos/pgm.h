#pragma once

#include <depthos/stdtypes.h>
#include <depthos/paging.h>

void __pgm_init(size_t mm_sz);

page_t* __pgm_alloc(size_t count);

void __pgm_free(page_t* start_page,size_t count);

void __pgm_set_bme(uint32_t idx,bool busy);
void __pgm_set_rpg_addr(uint32_t addr,bool busy);
void __pgm_set_rpg_desc(page_t *pg,bool busy);

uint8_t __pgm_get_bme(uint32_t idx);
uint8_t __pgm_get_rpg_addr(uint32_t addr);
uint8_t __pgm_get_rpg_desc(page_t *pg);


void __pgm_dump();
