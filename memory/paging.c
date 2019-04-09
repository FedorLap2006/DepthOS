#include "paging.h"
#include "heap.h"

extern placeAddr;

uint32_t *frames;
uint32_t nframes;

pDir_t *kernel_dir=0;
pDir_t *cur_dir=0;

/**
  Sets up the environment, page directories etc and
  enables paging.
**/
void init_paging() {
	uint32_t mem_end = 0x1000000;
	nframes = mem_end / 0x1000;

	frames = (uint32_t*)sbrk(IBIT(nframes));
	memset(frames, 0, IBIT(nframes));

	kernel_dir=(pDir_t*)sbrk(sizeof(pDir_t),1);
	memset(kernel_dir,0,sizeof(pDir_t));
	cur_dir = kernel_dir;
	
	int i = 0;

	while(i < placeAddr) {
		alloc_frame(get_page(i,1,kernel_dir),0,0);
		i += 0x1000;
	}

	reg_intr_handler(14,page_fault);
	switchPDir(kernel_dir);
}

/**
  Causes the specified page directory to be loaded into the
  CR3 register.
**/
void switchPDir(pDir_t *newp) {
	cur_dir = newp;
	asm volatile ("mov %0, %%cr3" : :"r"(&newp->tablesPhys));
	uint32_t cr0;
	asm volatile ("mov %%cr0, %0":"=r"(cr0));
	cr0 |= 0x80000000;
	asm volatile ("mov %0,%%cr0" : :"r"(cr0));
}

/**
  Retrieves a pointer to the page required.
  If make == 1, if the page-table in which this page should
  reside isn't created, create it!
**/
pEntry_t *get_page(u32int address, int make, pDir_t *dir) {
	address /= 0x1000;

	uint32_t table_idx = address / 1024;

	if ( dir->tables[table_idx] ) {
		return &dir->tables[table_idx]->pages[address%1024];
	}
	else if(make) {
		uint32_t tmp;
		dir->tables[table_idx] = (pTable_t*)sbrk_ap(sizeof(pTable_t),1,&tmp);
		memset(dir->tables[table_idx],0,0x1000);
		dir->tablesPhys[table_idx] = tmp | 0x7;
		return &dir->tables[table_idx]->pages[address%1024];
	}
	else
		return 0;
	
}


void set_frame(uint32_t fa) {
	uint32_t frame = fa / 0x1000;
	uint32_t idx = IBIT(frame);
	uint32_t off = OFFBIT(frame);
	frames[idx] |= (0x1 << off);
}
void clear_frame(uint32_t fa) {
	uint32_t frame = frame_addr/0x1000;
	uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
	frames[idx] &= ~(0x1 << off);
}
uint32_t test_frame(uint32_t fa) {
	uint32_t frame = frame_addr/0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    return (frames[idx] & (0x1 << off));
}

uint32_t firts_frame() {
	uint32_t i, j;
    for (i = 0; i < IBIT(nframes); ++i)
    {
        if (frames[i] != 0xFFFFFFFF) // nothing free, exit early
            for (j = 0; j < 32; ++j)
            {
                uint32_t toTest = 0x1 << j;
                if(!(frames[i] & toTest))
                    return i * 4 * 8 + j;
            }
    }
}
void alloc_frame(pEntry_t,int is_kernel,int is_rw) {
	if (page->frame != 0)
        return; // Кадр уже выделен для данной страницы
    else {
        u32int idx = first_frame(); // index of the first frame
        if(idx == (u32int)-1)
            return;

        set_frame(idx*0x1000); // застолбили кадр
        page->present = 1;
        page->rw = (is_writeable)?1:0;
        page->user = (is_kernel)?0:1;
        page->frame = idx;
    }
}
void free_frame(pEntry_t* page) {
	uint32_t frame;
    if(!(frame = page->frame))
        return; // Кадр для данной страницы не выделен
    else
    {
        clear_frame(frame);
        page->frame = 0x0;
    }
}

/**
  Handler for page faults.
**/
void page_fault(registers_t regs);
