#pragma once

#include <depthos/stdtypes.h>


typedef uint32_t lptr_t;

typedef struct __stdheap {
	lptr_t startAddr;
	lptr_t endAddr;
}heap_t;

typedef struct __stdheap_chunk {
	size_t size;
	short int used;
}heap_chunk_t;


uint32_t sbrk(uint32_t sz);

heap_t* init_heap(size_t sz);

void* __stdkmalloc(size_t sz,heap_t *heap);
void* __stdkfree(void* p,heap_t *heap);

void* __stdmalloc(size_t sz);
void* __stdfree(void* p);
