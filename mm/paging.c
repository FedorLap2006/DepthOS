#include <depthos/heap.h>
#include "depthos/paging.h"

#include <depthos/console.h>

#define MEM_SIZE 0x1000000

pdir_t *kern_dir = 0;
pdir_t *cur_dir = 0;

uint32_t *frames;
uint32_t nframes;

extern uint32_t placeAddr;

void switchPageDir(pdir_t *newpdir) {
	cur_dir = newpdir;
	__asm volatile ( "mov %0, %%cr3" ::"r"(&newpdir->tabsPhys):);

	uint32_t cr0;
	__asm volatile ( "mov %%cr0, %0" : "=r"(cr0)::);
	cr0 |= 0x80000000;
	__asm volatile ( "mov %0,%%cr0" : :"r"(cr0));
}

page_t *get_page(uint32_t address, int make, page_directory_t *dir) {
   // Turn the address into an index.
   address /= 0x1000;
   // Find the page table containing this address.
   uint32_t table_idx = address / 1024;
   if (dir->tables[table_idx]) // If this table is already assigned
   {
       return &dir->tables[table_idx]->pages[address%1024];
   }
   else if(make)
   {
       uint32_t tmp;
       dir->tables[table_idx] = (page_table_t*)sbrk_ap(sizeof(page_table_t), &tmp);
       memset(dir->tables[table_idx], 0, 0x1000);
       dir->tablesPhysical[table_idx] = tmp | 0x7; // PRESENT, RW, US.
       return &dir->tabs[table_idx]->pages[address%1024];
   }
   else
   {
       return 0;
   }


#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

// Static function to set a bit int the frames bitset
static void set_frame(uint32_t frame_addr) {
	uint32_t frame = frame_addr/0x1000;
	uint32_t idx = INDEX_FROM_BIT(frame);
	uint32_t off = OFFSET_FROM_BIT(frame);
	frames[idx] |= (0x1 << off);
}

static void clear_frame(uint32_t frame_addr) {
	uint32_t frame = frame_addr/0x1000;
	uint32_t idx = INDEX_FROM_BIT(frame);
	uint32_t off = OFFSET_FROM_BIT(frame);
	frames[idx] &= ~(0x1 << off);
}

static uint32_t test_frame(uint32_t frame_addr) {
	uint32_t frame = frame_addr/0x1000;
	uint32_t idx = INDEX_FROM_BIT(frame);
	uint32_t off = OFFSET_FROM_BIT(frame);
	return (frames[idx] & (0x1 << off));
}

static uint32_t first_frame() {
	uint32_t i, j;
	for (i = 0; i < INDEX_FROM_BIT(nframes); ++i)
	{
		if (frames[i] != 0xFFFFFFFF) // nothing free, exit early
			for (j = 0; j < 32; ++j)
			{
				uint32_t toTest = 0x1 << j;
				if(!(frames[i]&toTest))
					return i*4*8+j;
			}
	}
}


void alloc_frame(page_t *page,int is_kernel,int is_wr) {
	if ( page->frame != 0 ) {
		return;
	}
	else {
		uint32_t idx = first_frame();
		if ( idx == (uint32_t) -1 ) {
			print_mod("no free frames",MOD_ERR);
			return;
		}

		set_frame(idx * 0x1000);
		page->pres = 1;
		page->rw = (is_wr)?1:0;
		page->acc_user = (is_kernel)?0:1;
		page->frame = idx;
	}
}

void free_frame(page_t *page) {
	uint32_t frame;
	
	if ( !(frame = page->frame) ) {
		return;
	}
	else {
		clear_frame(frame);
		page->frame = 0x0;
	}
}


void paging_init() {
	uint32_t mend_page = MEM_SIZE;

	nframes = mend_page / 0x1000;
	frames = (uint32_t)sbrk(INDEX_FROM_BIT(nframes));

	kern_dir = (pdir_t*)sbrk_a(sizeof(pdir_t));
	// memset(kern_dir,0,sizeof(pdir_t));
	cur_dir = kern_dir;

	int i = 0;
	while ( i < placeAddr ) {
		alloc_frame(get_page(i,1,kern_dir),0,0);
		i += 0x1000;
	}
	reg_intr(14,page_fault);
	switchPageDir(kern_dir);

}

void page_fault(regs_t regs) {
	
}
