#pragma once

#define PMM_BLOCK_SIZE 4096
#define PMM_BLOCKS_PER_BUCKET 8
// Define some bit manipulating operations
#define PMM_SETBIT(i,bitmap) bitmap[i / PMM_BLOCKS_PER_BUCKET] = bitmap[i / PMM_BLOCKS_PER_BUCKET] | (1 << (i % PMM_BLOCKS_PER_BUCKET))
#define PMM_CLEARBIT(i,bitmap) bitmap[i / PMM_BLOCKS_PER_BUCKET] = bitmap[i / PMM_BLOCKS_PER_BUCKET] & (~(1 << (i % PMM_BLOCKS_PER_BUCKET)))
#define PMM_ISSET(i,bitmap) ((bitmap[i / PMM_BLOCKS_PER_BUCKET] >> (i % PMM_BLOCKS_PER_BUCKET)) & 0x1)
#define PMM_GET_BUCKET32(i,bitmap) (*((uint32_t*) &bitmap[i / 32]))

#define PMM_BLOCK_ALIGN(addr) (((addr) & 0xFFFFF000) + 0x1000)


extern uint32_t end; // link.ld

void pmm_init(uint32_t mm_sz);

uint32_t pmm_alloc();
void pmm_free(uint32_t blk_num);

uint32_t pmm_find();
