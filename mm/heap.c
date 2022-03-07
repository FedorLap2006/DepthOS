#include <depthos/paging.h>
#include <depthos/pgm.h>
#include "depthos/heap.h"



extern uint32_t end;

//page_t *free_pages = NULL;
//size_t total_page_count = 0;

uint32_t kend = (uint32_t)&end;
uint32_t glob_mem_start;
uint32_t glob_mem_cur;

heap_t globheap = { false, 0, NULL };

heap_t *cur_heap = &globheap;

void* glob_get_mem(size_t count) {
	//printk("Alloc\n");
	uint32_t tmp = glob_mem_cur;
	glob_mem_cur += count;
	return (void*)tmp;
}


hmajblock_t* alloc_majblock(size_t count_pg) { 
	page_t *start_pg = __pgm_alloc(count_pg);
	if(start_pg == NULL) return NULL;

	hmajblock_t* maj = (hmajblock_t*)glob_get_mem(sizeof(hmajblock_t));

	maj->pages = count_pg;

	maj->size = 0;

	maj->usage = 0;
	maj->start_frame = parse_page(start_pg).frame;


	return maj;	
}

hminblock_t alloc_minblock(struct __heap_minblock *prev) {

}


void init_heapsys(size_t mm_size) {
	mm_size /= 4096;	
	//free_pages = (page_t*)glob_get_mem(mm_size);
	//total_page_count = mm_size;

	glob_mem_start = kend;
	glob_mem_cur = glob_mem_start;
	//globheap.root = alloc_majblock(1);
}


void* heap_alloc(size_t bytes) {
	void* ret;

	hmajblock_t *majb = cur_heap->root;
	hminblock_t *minb = NULL;


}

void heap_free(void* addr) {

}

void switch_heap(heap_t* new_heap) { cur_heap = new_heap; }
