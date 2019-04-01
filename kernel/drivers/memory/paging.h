#pragma once

#include <std/types.h>
#include "heap.h"

#define IBIT(a) (a/(8*4))
#define OFFBIT(a) (a%(8*4))


typedef struct _pDir_t {
	pTable_t* tables[1024];
	uint32_t tablesPhys[1024];
	
	uint32_t physAddr;
}pDir_t;
typedef struct _pTable_t {
	pEntry_t pages[1024];
}pTable_t;

#define PAGE_M_RONLY 0
#define PAGE_M_RW 1
#define PAGE_PHYS 0
#define PAGE_VIRT 1
#define PAGE_ACC_KERN 0
#define PAGE_ACC_USER 1
#define PAGE_USED 1
#define PAGE_PUSE 0

typedef struct _pEntry_t {
	uint32_t pres : PAGE_PHYS;
	uint32_t rw : PAGE_RW; 
	uint32_t user : PAGE_ACC_USER;
	uint32_t is_free : PAGE_PUSE;
	uint32_t is_dirty : PAGE_PUSE;
	uint32_t unused : 7;
	uint32_t frame : 20;
}pEntry_t;


/**
  Sets up the environment, page directories etc and
  enables paging.
**/
void init_paging();

/**
  Causes the specified page directory to be loaded into the
  CR3 register.
**/
void switchPDir(pDir_t *newp);

/**
  Retrieves a pointer to the page required.
  If make == 1, if the page-table in which this page should
  reside isn't created, create it!
**/
pEntry_t *get_page(u32int address, int make, pDir_t *dir);


void set_frame(uint32_t fa);
void clear_frame(uint32_t fa);
uint32_t test_frame(uint32_t fa);

uint32_t firts_frame();
void alloc_frame(pEntry_t,int is_kernel,int is_rw);

void free_frame(pEntry_t* page);

/**
  Handler for page faults.
**/
void page_fault(registers_t regs);
