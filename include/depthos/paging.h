#pragma once

#include <depthos/stdtypes.h>

#define PG_TABLE_INDEX(vaddr) (((uint32_t)vaddr) >> 22)
#define PG_PAGE_INDEX(vaddr) ((((uint32_t)vaddr) >>12) & 0x3ff)
#define PG_PAGE_OFFSET(vaddr) (((uint32_t)vaddr) & 0xfff)

#define LOAD_MEMORY_ADDRESS 0xC0000000

#define fn_t uint16_t

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
};

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
};


typedef struct __pg_dir_t {
	struct __pg_table_t    *tabs[1024];
	struct __pg_tbref_t tabs_ref[1024];
};


#undef f_t

void* virt_to_phys(pg_dir_t *dir,void *v_addr);

page_t* get_page(void* v_addr,int make,pg_dir_t *dir)



