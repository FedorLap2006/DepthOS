#include <depthos/stdtypes.h>
#include <depthos/string.h>
#include "depthos/pmm.h"

uint8_t * bitmap = (uint8_t*)(&end);
uint8_t * mm_start;
uint32_t total_blocks;
uint32_t bitmap_sz;


void pmm_init(uint32_t mm_sz) {
	total_blocks = mm_sz / PMM_BLOCK_SIZE;

	bitmap_sz = total_blocks / PMM_BLOCKS_PER_BUCKET;

	if(bitmap_sz * PMM_BLOCKS_PER_BUCKET < total_blocks)
		bitmap_sz++;

	memset(bitmap,0,bitmap_sz);
	
	mm_start = (uint8_t*)PMM_BLOCK_ALIGN(((uint32_t)(bitmap + bitmap_sz)));
}

uint32_t pmm_alloc() {
	uint32_t fb = pmm_find();

	PMM_SETBIT(fb,bitmap);
	return fb;
}

void pmm_free(uint32_t blk_num) {
	PMM_CLEARBIT(blk_num,bitmap);
}

uint32_t pmm_find() {
	for(uint32_t i = 0; i < total_blocks; i++) {
		if(!PMM_ISSET(i,bitmap))
			return i;
	}
	return (uint32_t) -1;
}

