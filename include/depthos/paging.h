#pragma once

#include <depthos/kernel.h>
#include <depthos/stdtypes.h>
#include <depthos/tools.h>
#define PAGE_SIZE 4096

typedef uint32_t page_t;
typedef uint32_t pde_t;

typedef page_t *pagetb_t;
typedef pde_t *pagedir_t;

typedef struct __pageinfo {
  page_t *pg;

  bool pres : 1;
  bool rw : 1;
  bool us : 1;
  bool pwt : 1;
  bool pcd : 1;
  bool accessed : 1;
  bool dirty : 1;
  bool pat : 1;
  bool glob : 1;
  uint32_t frame;

} pageinfo_t;

#define PTE_PRESENT_SHIFT 0
#define PTE_RW_SHIFT 1
#define PTE_USER_SHIFT 2
#define PTE_WRITETHRU_SHIFT 3
#define PTE_CACHE_SHIFT 4
#define PTE_ACCESS_SHIFT 5
#define PTE_DIRTY_SHIFT 6
#define PTE_ZERO_SHIFT 7
#define PTE_ZERO_MASK (~(1 << PTE_ZERO_SHIFT))
#define PTE_GLOB_SHIFT 8
#define PTE_COW_SHIFT 9
#define PTE_AVAIL_MASK (~((1 << 9) | (1 << 10) | (1 << 11)))
#define PTE_ADDR_SHIFT 12

#define PDE_PRESENT_SHIFT 0
#define PDE_RW_SHIFT 1
#define PDE_USER_SHIFT 2
#define PDE_WRITETHRU_SHIFT 3
#define PDE_CACHE_SHIFT 4
#define PDE_ACCESS_SHIFT 5
#define PDE_ZERO_SHIFT 6
#define PDE_ZERO_MASK (~(1 << PTE_ZERO_SHIFT))
#define PDE_SIZE_SHIFT 7
#define PDE_IGNORE_SHIFT 8
#define PDE_COW_SHIFT 9
#define PDE_AVAIL_MASK (~((1 << 9) | (1 << 10) | (1 << 11)))
#define PDE_ADDR_SHIFT 12

#define PG_P_USER 1
#define PG_P_KERN 0

#define PG_R_RO 0
#define PG_R_RW 1

#define PG_RND_DOWN(a) ROUND_DOWN(a, 0x1000)
#define PG_RND_UP(a) ROUND_UP(a, 0x1000)

static inline void invlpg(uintptr_t addr) {
  __asm__ volatile("invlpg (%0)" : : "r"(addr) : "memory");
}
pageinfo_t parse_page(page_t *pg);
void change_page(page_t *pg, int pres, int rw, int us);
void turn_page(page_t *p);

int pde_index(uint32_t addr);
uintptr_t pte_index(uint32_t addr);

void activate_pgd(pagedir_t pgd);
pagedir_t get_current_pgd();

page_t *get_page(pagedir_t dir, uint32_t vaddr);
void *get_paddr(pagedir_t dir, void *vaddr); // get physical addr from virtual
void map_page(pagedir_t pgd, uintptr_t vaddr, uintptr_t phys, bool user);
void map_addr_fixed(pagedir_t pgd, uintptr_t vaddr, uintptr_t pstart,
                    size_t npages, bool user, bool overwrite);
uintptr_t map_addr(pagedir_t pgd, uintptr_t vaddr, size_t npages, bool user,
                   bool overwrite);

page_t make_pte(uint32_t paddr, int user, int rw);
pde_t make_pde(uint32_t paddr, int user, int rw);

extern pde_t kernel_pgd[1024] __align(4096);
pagedir_t create_pgd(void);
pagedir_t dup_pgd(pagedir_t original);
pagedir_t clone_pgd(pagedir_t original);

void paging_init();
