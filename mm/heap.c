#include <depthos/paging.h>
#include "depthos/heap.h"

extern uint32_t end;
extern pg_dir_t *cur_dir;
extern int _pg_enabled;
int _heap_enabled = 0;

heap_t* kern_heap = 0;

size_t kheap_sz = 100;

uint32_t placeAddr = (uint32_t)&end;

uint32_t pl_sbrk(uint32_t sz) {
	return pl_sbrk_uni(sz,0,NULL);
}

uint32_t pl_sbrk_uni(uint32_t sz,int align,uint32_t *phys) {
	if(align && (placeAddr & 0xFFFFF000) ) {
		placeAddr &= 0xFFFFF000;
		placeAddr += 0x1000;
	}
	if (phys) {
		*phys = placeAddr;
	}
	uint32_t tmp = placeAddr;
	placeAddr += sz;
	return tmp;
}
uint32_t pl_sbrk_a(uint32_t sz,int align) {
	return pl_sbrk_uni(sz,align,NULL);
}

uint32_t pl_sbrk_p(uint32_t sz,uint32_t *phys) {
	return pl_sbrk_uni(sz,0,phys);
}



uint32_t kmalloc_uni(uint32_t sz,int align,uint32_t *phys) {
	if (_heap_enabled) {
		if ( align ) sz += 4096;
		void *addr = __stdkmalloc(sz,kern_heap);
		uint32_t aaddr =  ((uint32_t)addr & 0xFFFFF000) + 0x1000;

		if (phys != 0) {
			uint32_t t = (uint32_t)addr;
			if ( align )
				t = aaddr;
			*phys = (uint32_t)get_paddr(cur_dir,(void*)t);
		}

		if ( align )
			return aaddr;
		return (uint32_t)addr;
	}
	else {
		return pl_sbrk_uni(sz,align,phys);
	}
	return 0;
}

uint32_t kmalloc(size_t sz) {
	return kmalloc_uni(sz,0,NULL);
}

uint32_t kmalloc_a(size_t sz,int align) {
	return kmalloc_uni(sz,align,NULL);
}
uint32_t kmalloc_p(size_t sz,uint32_t *phys) {
	return kmalloc_uni(sz,0,phys);
}


void* sbrk(size_t sz) {
	return NULL;
}

void* __stdkmalloc(size_t sz,heap_t *heap) {
	return NULL;
}


void __stdkfree(void* p,heap_t *heap) {
	return;
}
