#include "depthos/paging.h"
#include <depthos/heap.h>
#include <depthos/string.h>
#include <depthos/console.h>
#include <depthos/pmm.h>

#include <depthos/idt.h>

#define SET_PGBIT(cr0) (cr0 = cr0 | 0x80000000)
#define CLEAR_PSEBIT(cr4) (cr4 = cr4 & 0xffffffef)

int _pg_enabled = 0;

extern uint32_t end;
extern int _heap_enabled;
extern uint8_t *bitmap;
extern uint32_t bitmap_sz;


uint8_t  *mem_start;
uint32_t  tot_blocks;
uint32_t bitmap_size;

extern pg_dir_t* TEMP_PG_DIR;

pg_dir_t *kern_dir;
pg_dir_t *cur_dir;

static uint8_t *temp_mem;

static void* dumb_pgmalloc(uint32_t sz,int align) {
	void *ret = temp_mem;
	
	if (align && !IS_ALIGN(ret))
		ret = (void*)PAGE_ALIGN(ret);
	temp_mem += sz;
	return ret;
}

void* get_paddr(pg_dir_t *dir,void *v_addr) {
	if (!_pg_enabled) {
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
		alloc_page(dir,(uint32_t)v_addr,0,0,1);
		return &dir->tabs[PG_TABLE_INDEX(v_addr)]->pages[PG_PAGE_INDEX(v_addr)];
	}
	else {
		return NULL;
	}
}


void alloc_region(pg_dir_t * dir, uint32_t start_va, uint32_t end_va, int iden_map, int is_kernel, int is_writable) {
    uint32_t start = start_va & 0xfffff000;
    uint32_t end = end_va & 0xfffff000;
    while(start <= end) {
        if(iden_map)
            alloc_page(dir, start, start / PAGE_SIZE, is_kernel, is_writable);
        else
            alloc_page(dir, start, 0, is_kernel, is_writable);
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
		if (_heap_enabled)
			tb = (pg_table_t*)kmalloc_uni(sizeof(pg_table_t),1,NULL);
		else
			tb = (pg_table_t*)dumb_pgmalloc(sizeof(pg_table_t),1);
		memset(tb,0,sizeof(pg_table_t));
		uint32_t t = (uint32_t)get_paddr(kern_dir,tb);


		dir->tabs_ref[PG_TABLE_INDEX(vaddr)].frame = t >> 12;
		dir->tabs_ref[PG_TABLE_INDEX(vaddr)].pres = 1;
		dir->tabs_ref[PG_TABLE_INDEX(vaddr)].rw = (is_rw)?1:0;
		dir->tabs_ref[PG_TABLE_INDEX(vaddr)].user = (is_kern)?0:1;
		dir->tabs_ref[PG_TABLE_INDEX(vaddr)].page_size = 0;

		dir->tabs[PG_TABLE_INDEX(vaddr)] = tb;
	}

	if (!tb->pages[PG_PAGE_INDEX(vaddr)].pres) {
		uint32_t t;
		if(frame)
			t = frame;
		else
			t = pmm_alloc();
		tb->pages[PG_PAGE_INDEX(vaddr)].frame = t;
		tb->pages[PG_PAGE_INDEX(vaddr)].pres = 1;
		tb->pages[PG_PAGE_INDEX(vaddr)].rw = (is_rw)?1:0;
		tb->pages[PG_PAGE_INDEX(vaddr)].user = (is_kern)?0:1;

	}
}
void free_region(pg_dir_t * dir, uint32_t start_va, uint32_t end_va, int free) {
    uint32_t start = start_va & 0xfffff000;
    uint32_t end = end_va & 0xfffff000;
    while(start <= end) {
        free_page(dir, start, 1);
        start = start + PAGE_SIZE;
    }
}

void free_page(pg_dir_t *dir,uint32_t vaddr,int free) {
	if(dir == TEMP_PG_DIR) return;
	pg_table_t *tb = dir->tabs[PG_TABLE_INDEX(vaddr)];

	if (!tb->pages[PG_PAGE_INDEX(vaddr)].pres) {
		
	}

	if (free)
		pmm_free(tb->pages[PG_PAGE_INDEX(vaddr)].frame);
	tb->pages[PG_PAGE_INDEX(vaddr)].pres = 0;
	tb->pages[PG_PAGE_INDEX(vaddr)].frame = 0;
}

void pg_switch_dir(pg_dir_t *pdir, uint32_t phys) {
	uint32_t t;
	if (!phys) 
		t = (uint32_t)get_paddr(TEMP_PG_DIR,pdir);
	else
		t = (uint32_t)pdir;
	cur_dir = pdir;
	__asm volatile ("mov %0,%%cr3" :: "r"(t) : );

}


void enable_paging() {
    uint32_t cr0, cr4;

    __asm volatile ("mov %%cr4, %0" : "=r"(cr4));
    CLEAR_PSEBIT(cr4);
    __asm volatile ("mov %0, %%cr4" :: "r"(cr4));

    __asm volatile ("mov %%cr0, %0" : "=r"(cr0));
    SET_PGBIT(cr0);
    __asm volatile ("mov %0, %%cr0" :: "r"(cr0));

    _pg_enabled = 1;
}

extern size_t kheap_sz;


void do_page_fault(regs_t regs) {
}


void paging_init() {
	cur_dir = TEMP_PG_DIR;
	temp_mem = bitmap + bitmap_sz;

	kern_dir = dumb_pgmalloc(sizeof(pg_dir_t),1);
	memset(kern_dir,0,sizeof(pg_dir_t));

	uint32_t i = 0xC0000000;

	while( i < 0xC0000000 + 4 * 1024 * 1024 ) {
		alloc_page(kern_dir,i,0,1,1);
		i += PAGE_SIZE;
	}
	
//	i = 0xC0000000 + 4 * ;

	while( i < 0xC0000000 + 8 * 1024 * 1024 + kheap_sz) {
		alloc_page(kern_dir,i,0,1,1);
		i += PAGE_SIZE;
	}

	reg_intr(14,do_page_fault);
	
	pg_switch_dir(kern_dir,0);

	enable_paging();

	alloc_region(kern_dir,0x0,0x10000,1,1,1);	
	print_mod("paging initialized",MOD_OK);
}























