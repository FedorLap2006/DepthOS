#include "heap.h"
#include "paging.h"
#include <std/memory.h>
uint32_t placeAddr;
extern pDir_t *kernel_dir;
extern pDir_t *cur_dir;

uint32_t sbrk_ap(uint32_t size,int align,uint32_t *phys) {
	
	if(align && (placeAddr & 0xFFFFF000) ) {
		placeAddr &= 0xFFFFF000;
		placeAddr += 0x1000;
	}
	if (phys) {
		*phys = placeAddr;
	}
	uint32_t tmp = placeAddr;
	placeAddr += size;
	return tmp;
}
uint32_t sbrk(uint32_t size) {
	uint32_t tmp = placeAddr;
	placeAddr += size;
	return tmp;
}
uint32_t sbrk_a(uint32_t size,int align) {
	if (align && (placeAddr & 0xFFFFF000)) {
		placeAddr &= 0xFFFFF000;
		placeAddr += 0x1000;
	}
	uint32_t tmp = placeAddr;
}

uint32_t sbrk_p(uint32_t size,uint32_t *phys) {
	if ( phys ) {
		*phys = placeAddr;
	}
	uint32_t tmp = placeAddr;
	placeAddr += size;
	return tmp;
}

heap_t* init_heap(size_t size) {
	if ( size <= 0 ) return;
	heap_t *h = (heap_t*)sbrk(sizeof(heap_t));
	
//	ptr_t curAddr = sbrk(0);
//	ptr_t maxAddr = curAddr + PAGE_SIZE;
	
	// alloc_frame(get_page(curAddr, 1, cur_dir), 1, 1);

//	h->chunks = (hchunk_t*)sbrk(KHEAP_SIZE / KHEAP_CHUNK_SIZE);
	
	ptr_t startAddr = sbrk(0);
	ptr_t endAddr = sbrk(size);

	h->startAddr = startAddr;
	h->endAddr = endAddr;

//	ptr_t ptr = NULL;
//	for ( ptr = h->startAddr; ptr != h->endAddr; ptr++ ) {
//		*(ptr) = 0;
//	}
	memset(startAddr, 0, endAddr - startAddr);


	if ( stdheap == 0 ) stdheap = h;

	return h;
}

static bool check_heap(heap_t* h) {
	return (h->startAddr != 0x0) && (h->endAddr != 0x0);
}

static hchunck_t find_chunck(heap_t *h) {
	// if (!check_heap(h)) return NULL;
	ptr_t ptr = NULL;
	hchunk_t cur_chunk = {0,0,0,0}; // ,false};
	for(ptr = h->startAddr; ptr != h->endAddr; ptr++) {
		if ( *(ptr) != 0 ) continue;
		cur_chunk.startAddr = ptr;
		
		if ( (ptr + KHEAP_CHUNK_SIZE) != h->endAddr ) break;
		else cur_chunk.endAddr = ptr + KHEAP_CHUNK_SIZE;
		
		cur_chunk.curAddr = cur_chunk.startAddr;
		cur_chunk.size = KHEAP_CHUNK_SIZE;
		
		memset(cur_chunk.startAddr,1,cur_chunk.startAddr - cur_chunk.endAddr);

//		cur_chunk.is_free = true;
		
	}
	return cur_chunk;
}

void* decl __stdmalloc(size_t size,heap_t *heap) {
	if ( !size || size < 0 ) return NULL;
	bool is_valid = check_heap(heap);
	if (!is_valid) return;
	ptr_t bega = heap->startAddr;
	ptr_t enda = heap->endAddr;

	// hchunk_t chunk = find_chunk(heap);
	
	alloc_frame(get_page(placeAddr, 1, cur_dir), 1, 1);
	
	ptr_t curAddr = heap->startAddr;

	while (curAddr != heap->endAddr) {
		hchunk_t *chunk = (hchunk_t*)curAddr;
		if (chunk->used == 0 ) {
			if ( chunk->size >= size )
				chunk->used=1;
				return curAddr + sizeof(struct __stdheap_chunk);
			else if ( chunk->size == 0 )
				goto nalloc;
		}
		curAddr += chunk->size;
		curAddr += sizeof(struct __stdheap_chunk);

	}
	return NULL;
nalloc:

	hchunk_t chunk = (hchunk_t)curAddr;
	chunk->size = size;
	chunk->used=1;
	

	curAddr += sizeof(hchunk_t);

	return (char*)curAddr;
	

//	if ( size > KHEAP_CHUNK_SIZE ) 
//
//
		

	
}
//void decl *__stdpa_malloc(size_t size,heap_t *heap) {
//
//}
void* decl __stdfree(void* p,heap_t heap) {
 	if ( (p - sizeof(hchunk_t)) == heap->startAddr ) return;
	hchunk_t *chunk = (hchunk_t*) (p - sizeof(hchunk_t));
	
	chunk->used = 0;
}

void* decl malloc(size_t size) {
	return __stdmalloc(size,stdheap);
}
void decl free(void* p) {
	__stdfree(p,stdheap);
}
