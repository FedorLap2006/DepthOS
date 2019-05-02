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


uint32_t pl_sbrk(uint32_t sz);
uint32_t pl_sbrk_uni(uint32_t sz,int aling,uint32_t *phys);
uint32_t pl_sbrk_a(uint32_t sz,int aling);
uint32_t pl_sbrk_p(uint32_t sz,uint32_t *phys);

uint32_t kmalloc_uni(uint32_t sz,int align,uint32_t *phys);
uint32_t kmalloc(uint32_t sz);
uint32_t kmalloc_a(uint32_t sz,int align);
uint32_t kmalloc_p(uint32_t sz,uint32_t *phys);


void* sbrk(size_t sz);



heap_t* init_heap(size_t sz);

void* __stdkmalloc(size_t sz,heap_t *heap);
void __stdkfree(void* p,heap_t *heap);

void* __stdmalloc(size_t sz);
void* __stdfree(void* p);
