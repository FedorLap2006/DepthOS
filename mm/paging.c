\#include "depthos/paging.h"
#include <depthos/heap.h>
#include <depthos/string.h>
#include <depthos/console.h>

static int _pg_enable = 0;

extern uint32_t end;
uint8_t * bitmap = (uint8_t*)(&end);

uint8_t  *mem_start;
uint32_t  tot_blocks;
uint32_t bitmap_size;

struct __pg_dir_t kern_dir;

void* get_paddr(pg_dir_t *dir,void *v_addr) {
	if (!_pg_enable) {
		return (void*)(v_addr - LOAD_MEMORY_ADDRESS);
	}	
	
	if(!dir->tabs[PG_TABLE_INDEX(v_addr)]) {
		print_mod("virt mem: page table does not exist",MOD_ERR);
		return NULL;
	}
	pg_table_t *tab = dir->tabs[PG_TABLE_INDEX(v_addr)];
	if (!tab->pages[PG_PAGE_INDEX(v_addr)].pres) {
		print_mod("virt mem: page is not present",MOD_ERR);
		return NULL;
	}
	
	uint32_t t = tab->pages[PG_PAGE_INDEX(v_addr)].frame;
	t = (t << 12) + PG_PAGE_OFFSET(v_addr);
	return (void*)t;
}



pg_page_t* get_page(pg_dir_t *dir,int make,void* v_addr) {
	if(dir->tabs[PG_TABLE_INDEX(v_addr)]) {
		return &dir->tabs[PG_TABLE_INDEX(v_addr)]->pages[PG_PAGE_INDEX(v_addr)];
	}
	else if(make) {
		alloc_page(dir,v_addr,0,0,1);
		return &dir->tabs[PG_TABLE_INDEX(v_addr)]->pages[PG_PAGE_INDEX(v_addr)];
	}
	else {
		return NULL;
	}
}


void alloc_region(page_directory_t * dir, uint32_t start_va, uint32_t end_va, int iden_map, int is_kernel, int is_writable) {
    uint32_t start = start_va & 0xfffff000;
    uint32_t end = end_va & 0xfffff000;
    while(start <= end) {
        if(iden_map)
            allocate_page(dir, start, start / PAGE_SIZE, is_kernel, is_writable);
        else
            allocate_page(dir, start, 0, is_kernel, is_writable);
        start = start + PAGE_SIZE;
    }
}

void alloc_page(pg_dir_t *dir,uint32_t vaddr,uint32_t frame,int is_kern,int is_rw) {
	pg_table_t *tb = NULL;
	if (!dir) {
		return;
	}
	tb = dir->tabs[PG_TABLE_INDEX(vaddr)];
	if(!tb) {
		tb = malloc(sizeof(pg_table_t));
		memset(tb,0,sizeof(pg_table_t));
		uint32_t t = (uint32_t)get_paddr(kern_dir,tb);


		dir->ref_tabs[PG_TABLE_INDEX(vaddr)].frame = t >> 12;
		dir->ref_tabs[PG_TABLE_INDEX(vaddr)].pres = 1;
		dir->ref_tabs[PG_TABLE_INDEX(vaddr)].rw = (is_rw)?1:0;
		dir->ref_tabs[PG_TABLE_INDEX(vaddr)].user = (is_kern)?0:1;
		dir->ref_tabs[PG_TABLE_INDEX(vaddr)].page_size = 0;

		dir->tabs[PG_TABLE_INDEX(vaddr)] = tb;
	}

	if (!tb->pages[PG_PAGE_INDEX(vaddr)].pres) {
		uint32_t t;
		if(frame)
			t = frame;
		else
			t = alloc_block();
		tb->pages[PG_PAGE_INDEX(vaddr)].frame = t;
		tb->pages[PG_PAGE_INDEX(vaddr)].pres = 1;
		tb->pages[PG_PAGE_INDEX(vaddr)].rw = (is_rw)?1:0;
		tb->pages[PG_PAGE_INDEX(vaddr)].user = (is_kern)?0:1;

	}
}


static uint32_t alloc_block() {
	uint32_t fb = first_free_block();
	SETBIT(fb,bitmap);
	return fb;
}

static uint32_t first_free_block() {
    uint32_t i;
    for(i = 0; i < total_blocks; i++) {
        if(!ISSET(i))
            return i;
    }
    return (uint32_t) -1;
}



void free_region(page_directory_t * dir, uint32_t start_va, uint32_t end_va, int free) {
    uint32_t start = start_va & 0xfffff000;
    uint32_t end = end_va & 0xfffff000;
    while(start <= end) {
        free_page(dir, start, 1);
        start = start + PAGE_SIZE;
    }
}

void free_page(pg_dir_t *dir,uint32_t vaddr,int free) {
	pg_table_t *tb = dir->tabs[PG_TABLE_INDEX(vaddr)];

	if (!tb->pages[PG_PAGE_INDEX(vaddr)].pres) {
		
	}

	if (free)
		free_block(tb->pages[PG_PAGE_INDEX(vaddr)]);
	tb->pages[PG_PAGE_INDEX(vaddr)].pres = 0;
	tb->pages[PG_PAGE_INDEX(vaddr)].frame = 0;
}




static void free_block(uint32_t blk_num) {
    CLEARBIT(blk_num);
}


void pg_switch_dir(pg_dir_t *pdir, uint32_t phys) {
	uint32_t t;
	if (!phys) 
		t = (uint32_t)get_paddr(TEMP_PDIR,pdir);
	else
		t = (uint32_t)pdir;
	__asm volatile ("mov %0,%%cr3" :: "r"(t) : );

}


void pmm_init(uint32_t mm_sz) {
	tot_blocks = mm_sz / BL
}

void paging_init() {
	tot_blocks = 
}








