#pragma once

#include <depthos/stdtypes.h>
#include <depthos/ports.h>

typedef struct __page_t {
	uint32_t pres 		:  1;
	uint32_t rw	  		:  1;
	uint32_t acc_user 	:  1;
	uint32_t is_free	:  1;
	uint32_t is_dirty	:  1;
	uint32_t unused 	:  7;
	uint32_t frame		: 20;
}page_t;

typedef struct __ptable_t {
	page_t pages[1024];
}ptable_t;

typedef struct __pdir_t {
	ptable_t tabs[1024];
	uint32_t tabsPhys[1024];

	uint32_t physAddr;
}pdir_t;


void alloc_frame(page_t *page,int is_kernel,int is_wr);
void free_frame(page_t *page);

void switchPageDir(pdir_t *newpdir);
page_t *getPage(uint32_t addr,int make, pdir_t dir);



void paging_init();

