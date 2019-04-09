#pragma once

#include <types.h>
#include <list.h>
#include <macro-tools.h>
#define KHEAP_START    0xC0000000
#define KHEAP_SIZE 8*1000

#define KHEAP_CHUNK_SIZE 8*1024

typedef uint32_t ptr_t;

typedef struct __stdheap {
	size_t startAddr;
	size_t endAddr;
	struct __stdheap_chunk *chunks;
}heap_t;

typedef struct __stdheap_chunk {
	size_t size;
	short int used;
}hchunk_t;

uint32_t sbrk_ap(uint32_t size,int align,uint32_t *phys);
uint32_t sbrk_p(uint32_t size,uint32_t *phys);
uint32_t sbrk_a(uint32_t size,int align);
uint32_t sbrk(uint32_t size);

struct __stdheap *stdheap;

heap_t* init_heap(size_t size);

void* decl __stdmalloc(size_t size, heap_t *heap);
//void decl *__stdpa_malloc(size_t size,heap_t *heap);
void decl __stdfree(void* p,heap_t heap);

void *malloc(size_t size);
void *free(void* p);

