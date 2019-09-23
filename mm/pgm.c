#include <depthos/console.h>
#include <depthos/string.h>
#include <depthos/stdbits.h>
#include "depthos/pgm.h"


extern void* glob_get_mem(size_t);
 
// extern pagedir_t cur_pgd;
// extern uint32_t kend;

// extern page_t kernel_pgt[1024] __align(4096); /* 768 */
// extern page_t heap_mem_pgt[1024] __align(4096);
// extern page_t heap_blks_pgt[1024] __align(4096);
// extern page_t proc_dirs_pgt[1024] __align(4096); /* 769 */ // for processes
// extern page_t proc_tbs_pgt[1024] __align(4096); /* 769 */ // for processes

// uint8_t *pgm_bitmap;
// pagetb_t pgm_cur_pgtb;

// size_t pgm_memory_size;

// size_t pgm_bitmap_size;
// size_t pgm_count_pages;


//extern pgm_state_t pgm_,ultitasking_state;
/*
static size_t get_arr_index(size_t idx,size_t c) { 
	size_t res;
	res = idx / c;
	if ( (idx % c) != 0 ) {
		res++;
	}
  if ( res > 0 && (idx % c) != 0) {
    res--;
  }
	return res;
}

uint32_t __pgm_alloca(size_t count) {
  bool isglobal = false;
  if(pgtb == NULL) { isglobal = true; }

  if(isglobal) {
    for(size_t idx = 0; idx < )
  } else {

  }
}


void __pgm_set_bme(uint32_t idx,bool busy) {
	if(idx >= bitmap_sz) return;
  if(busy) {
    pgm_bitmap[get_arr_index(idx,8)] = pgm_bitmap[get_arr_index(idx,8)] | (1 << idx % 8);
  } else {
    pgm_bitmap[get_arr_index(idx,8)] = pgm_bitmap[get_arr_index(idx,8)] & ~(1 << idx % 8);
  }
}

void __pgm_set_rpg_addr(uint32_t addr,bool busy) {
  __pgm_set_bme(addr / 4096,busy);
}
void __pgm_set_rpg_desc(page_t *pg,bool busy) {
  __pgm_set_rpg_addr(parse_page(pg).frame,busy);
}


uint8_t __pgm_get_bme(uint32_t idx) {
  return (pgm_bitmap[get_arr_index(idx,8)] >> (idx % 8)) & 1;
}
uint8_t __pgm_get_rpg_addr(uint32_t addr) {
  return __pgm_get_bme(addr / 4096);
}
uint8_t __pgm_get_rpg_desc(page_t *pg) {
  return __pgm_get_rpg_addr(parse_page(pg).frame);
}

void __pgm_dump() {
  size_t i = 0;
  for(i = 0; i < bitmap_pgc; i++) {
    printk("%d - %d\n",i,pgm_bitmap[get_arr_index(i,8)] & (1 << i % 8));
  }
}*/
