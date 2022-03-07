#pragma once

#include <depthos/stdtypes.h>

#define HEAP_MAGIC 0x7971D91F // 2037504287

struct __heap_minblock;

typedef struct __heap_majblock {
	struct __heap_majblock *next;
	struct __heap_majblock *prev;
	
	short int pages;
	short int size;
	uint16_t usage;
	uint32_t start_frame;

	struct __heap_minblock *firstblock;
	struct __heap_minblock *endblock;
}hmajblock_t;

typedef struct __heap_minblock {
	unsigned int magic;
	struct __heap_minblock *next;
	struct __heap_minblock *prev;
	struct __heap_majblock *majblock;
	bool used;

	size_t size;
	uint32_t frame;
}hminblock_t;

typedef struct __heap_t {
	bool us;
	size_t tsize;
	struct __heap_majblock *root;
	struct __heap_majblock *top;
}heap_t;


void init_heapsys();

void* heap_alloc(size_t bytes);
void heap_free(void* addr);
void switch_heap(heap_t* new_heap);
