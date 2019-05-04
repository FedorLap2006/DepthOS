#pragma once

#include <depthos/idt.h>
#include <depthos/stdtypes.h>

#define PAGE_SIZE 4096

#define IS_ALIGN(addr) ((((uint32_t)(addr)) | 0xFFFFF000) == 0)
#define PAGE_ALIGN(addr) ((((uint32_t)(addr)) & 0xFFFFF000) + 0x1000)


#define PG_TABLE_INDEX(vaddr) (((uint32_t)vaddr) >> 22)
#define PG_PAGE_INDEX(vaddr) ((((uint32_t)vaddr) >>12) & 0x3ff)
#define PG_PAGE_OFFSET(vaddr) (((uint32_t)vaddr) & 0xfff)

#define LOAD_MEMORY_ADDRESS 0xC0000000

#define fn_t unsigned int

typedef struct __pg_page_t {
	fn_t pres   		: 1;
	fn_t rw 	   		: 1;
	fn_t user   		: 1;
	fn_t reserved		: 2;
	fn_t accessed		: 1;
	fn_t dirty  		: 1;
	fn_t reserved2		: 2;
	fn_t avaible 		: 3;
	fn_t frame			: 20;
}pg_page_t;

typedef struct __pg_table_t {
	struct __pg_page_t pages[1024];
}pg_table_t;

typedef struct __pg_tbref_t {
	fn_t pres	    : 1;
    fn_t rw         : 1;
    fn_t user       : 1;
    fn_t w_through  : 1;
    fn_t cache      : 1;
    fn_t access     : 1;
    fn_t reserved   : 1;
    fn_t page_size  : 1;
    fn_t global     : 1;
    fn_t available  : 3;
    fn_t frame      : 20;
}pg_tbref_t;


typedef struct __pg_dir_t {
	struct __pg_tbref_t tabs_ref[1024];
	struct __pg_table_t    *tabs[1024];
}pg_dir_t;


#undef f_t

void* get_paddr(pg_dir_t *dir,void *v_addr);

pg_page_t* get_page(pg_dir_t *dir,int make,void* v_addr);

void alloc_region(pg_dir_t * dir, uint32_t start_va, uint32_t end_va, int iden_map, int is_kernel, int is_writable);
void alloc_page(pg_dir_t *dir,uint32_t vaddr,uint32_t frame,int is_kern,int is_rw);

void free_region(pg_dir_t * dir, uint32_t start_va, uint32_t end_va, int free);
void free_page(pg_dir_t *dir,uint32_t vaddr,int free);

/*|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||*/


void do_page_fault(regs_t regs);
void pg_switch_dir(pg_dir_t *pdir, uint32_t phys);
void enable_paging();

void paging_init();
