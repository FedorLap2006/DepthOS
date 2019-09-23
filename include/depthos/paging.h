#pragma once

#include <depthos/tools.h>
#include <depthos/stdtypes.h>
#include <depthos/kernel.h>

typedef uint32_t page_t;
typedef uint32_t pde_t;

typedef page_t *pagetb_t;
typedef pde_t  *pagedir_t;

#define _PAGEINFO_pres(page) ((page >> PTE_PRESENT_SHIFT) & 1U)
#define _PAGEINFO_rw(page) ((page >> PTE_RW_SHIFT) & 1U)
#define _PAGEINFO_us(page) ((page >> PTE_USER_SHIFT) & 1U)
#define _PAGEINFO_pwt(page) ((page >> PTE_WRITETHRU_SHIFT) & 1U)
#define _PAGEINFO_pcd(page) ((page >> PTE_CACHE_SHIFT) & 1U)
#define _PAGEINFO_accessed(page) ((page >> PTE_ACCESS_SHIFT) & 1U)
#define _PAGEINFO_dirty(page) ((page >> PTE_DIRTY_SHIFT) & 1U)
#define _PAGEINFO_pat(page) ((page >> PTE_ZERO_SHIFT) & 1U)
#define _PAGEINFO_pat(page) ((page >> PTE_GLOB_SHIFT) & 1U)
#define _PAGEINFO_frame(page) (page & 0xFFFFF000)
#define PAGEINFO(page,attr) _PAGEINFO_##attr(page)

typedef struct __pageinfo {
	page_t* pg;
	uint32_t pres 		: 1;
	uint32_t rw 		: 1;
	uint32_t us			: 1;
	uint32_t pwt		: 1;
	uint32_t pcd		: 1;
	uint32_t accessed 	: 1;
	uint32_t dirty 		: 1;
	uint32_t pat 		: 1;
	uint32_t glob		: 1;
	uint32_t frame 		: 20;

}pageinfo_t;

#define PTE_PRESENT_SHIFT   0
#define PTE_RW_SHIFT        1
#define PTE_USER_SHIFT      2
#define PTE_WRITETHRU_SHIFT 3
#define PTE_CACHE_SHIFT     4
#define PTE_ACCESS_SHIFT    5
#define PTE_DIRTY_SHIFT     6
#define PTE_ZERO_SHIFT      7
#define PTE_ZERO_MASK       (~(1 << PTE_ZERO_SHIFT))
#define PTE_GLOB_SHIFT      8
#define PTE_COW_SHIFT       9
#define PTE_AVAIL_MASK      (~((1 << 9) | (1 << 10) | (1 << 11)))
#define PTE_ADDR_SHIFT      12

#define PDE_PRESENT_SHIFT   0
#define PDE_RW_SHIFT        1
#define PDE_USER_SHIFT      2
#define PDE_WRITETHRU_SHIFT 3
#define PDE_CACHE_SHIFT     4
#define PDE_ACCESS_SHIFT    5
#define PDE_ZERO_SHIFT      6
#define PDE_ZERO_MASK       (~(1 << PTE_ZERO_SHIFT))
#define PDE_SIZE_SHIFT      7
#define PDE_IGNORE_SHIFT    8
#define PDE_COW_SHIFT       9
#define PDE_AVAIL_MASK      (~((1 << 9) | (1 << 10) | (1 << 11)))
#define PDE_ADDR_SHIFT      12

#define PG_P_USER     1
#define PG_P_KERN     0

#define PG_R_RO       0
#define PG_R_RW       1

#define PG_RND_DOWN(a) ROUND_DOWN(a, 0x1000)
#define PG_RND_UP(a) ROUND_UP(a, 0x1000)

pageinfo_t parse_page(page_t* pg);

void change_page(page_t *pg,int pres,int rw,int us);

int pde_index(uint32_t addr);

uintptr_t pte_index(uint32_t addr);

void activate_pgd(pagedir_t pgd);

pagedir_t __save_pgd(void);

pagedir_t activate_pgd_save(pagedir_t pgd);

void* get_paddr(pagedir_t dir,void *vaddr); // get physical addr from virtual

void turn_page(page_t *p);

page_t* get_page(pagedir_t dir, uint32_t vaddr);

extern pde_t kernel_pgd[1024] __align(4096);

page_t make_pte(uint32_t paddr,int user,int rw);
pde_t  make_pde(uint32_t paddr,int user,int rw);


void paging_init();
