#pragma once

#include <depthos/string.h>
#include <depthos/stdtypes.h>

// mru - memory reuse (algorithm)

struct __heap_cacheblock;
struct __heap_localblock;

typedef struct __heap_resblock {
	size_t pages;
	struct __heap_cacheblock *root_cacheblock;
	struct __heap_cacheblock *top_cacheblock;
	bool busy_mru;
	uint32_t resid;
	uint32_t beginpf;
	uint32_t lastpf;
	//uint32_t lastlba;
}heap_resblock_t;

typedef struct __heap_localblock {
	struct __heap_cacheblock *cacheblock;
}heap_localblock_t;

typedef struct __heap_cacheblock {
	size_t size;
	bool busy;
	bool busy_mru;

	heap_localblock_t *localblock;

	struct __heap_resblock *resource;

	struct __heap_cacheblock *next;
	struct __heap_cacheblock *prev;
}heap_cacheblock_t;
	

heap_resblock_t* __heap_alloc_resblock();
heap_resblock_t* __heap_find_resblock();
void __heap_free_resblock(heap_resblock_t* rb);

void __heap_free(void* ptr);
void* __heap_alloc(heap_resblock_t* resb, size_t size);

void __gheap_init();