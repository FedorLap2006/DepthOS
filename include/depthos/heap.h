#pragma once

#include <depthos/stdtypes.h>

#define HEAP_MAGIC 0x7971D91F // 2037504287




typedef struct __heap_page{
	bool pres;
	bool rw;
	bool us;
	bool used;
	bool dirty;
	uint32_t frame;
}heap_page_t;

typedef struct __heap_ptable {
	bool pres;
	bool rw;
	bool us;
	bool used;
	uint32_t ptaddr;
	size_t ptsize;
}heap_ptable_t;

typedef heap_ptable_t* heap_pdir_t;
typedef heap_pdir_t __heap_pdir_t;


typedef struct __heap {
	heap_pdir_t pdir;
	int total_used;
	heap_page_t* heap_begin;
	heap_page_t* heap_cur;
	heap_page_t* heap_end;
}heap_t;

void init_heapsys();

void* heap_alloc(size_t bytes);
void heap_free(void* addr);
