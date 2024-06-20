#pragma once

#include <depthos/logging.h>
#include <depthos/kernel.h>
#include <depthos/stdtypes.h>
#include <depthos/tools.h>

#define PAGE_SIZE 4096
#define PAGE_TABLE_SIZE 1024

typedef uint32_t page_t;
typedef uint32_t pde_t;

typedef page_t *pagetable_t;
typedef pde_t *pagedir_t;

// Page table entry (page descriptor) definitions.

#define PTE_PRESENT_SHIFT 0
#define PTE_RW_SHIFT 1
#define PTE_USER_SHIFT 2
#define PTE_PWT_SHIFT 3
#define PTE_PCD_SHIFT 4
#define PTE_ACCESS_SHIFT 5
#define PTE_DIRTY_SHIFT 6
#define PTE_PAT_SHIFT 7
#define PTE_GLOB_SHIFT 8
#define PTE_AVAIL_SHIFT 9
#define PTE_AVAIL_MASK 0x7
#define PTE_EXTRACT_AVAIL(v) (((v) >> PTE_AVAIL_SHIFT) & PTE_AVAIL_MASK)
#define PTE_ADDR_SHIFT 12
#define PTE_ADDR_MASK 0xFFFFF
#define PTE_EXTRACT_ADDR(v)                                                    \
  ((((v) >> PTE_ADDR_SHIFT) & PTE_ADDR_MASK) * PAGE_SIZE)
#define PAGE_EXTRACT_ADDR PTE_EXTRACT_ADDR

// Custom PTE bitfields.

#define PTE_C_COW_SHIFT 9

// Page directory entry (table descriptor) bitfields.

#define PDE_PRESENT_SHIFT 0
#define PDE_RW_SHIFT 1
#define PDE_USER_SHIFT 2
#define PDE_PWT_SHIFT 3
#define PDE_PCD_SHIFT 4
#define PDE_ACCESS_SHIFT 5
#define PDE_AVAIL_SHIFT 6
#define PDE_SIZE_SHIFT 7
#define PDE_AVAIL2_SHIFT 8
#define PDE_AVAIL2_MASK 0xF
#define PDE_EXTRACT_AVAIL2(v) (((v) >> PDE_AVAIL2_SHIFT) & PDE_AVAIL2_MASK)
#define PDE_ADDR_SHIFT 12
#define PDE_ADDR_MASK 0xFFFFF
#define PDE_EXTRACT_ADDR(v)                                                    \
  ((((v) >> PDE_ADDR_SHIFT) & PDE_ADDR_MASK) * PAGE_SIZE)

// Custom PDE bitfields.

#define PDE_C_COW_SHIFT PDE_AVAIL_SHIFT

// Address utilities.

#define PG_RND_DOWN(a) ROUND_DOWN(a, PAGE_SIZE)
#define PG_RND_UP(a) ROUND_UP(a, PAGE_SIZE)
#define PG_OFFSET(a) (((uint32_t)a) % PAGE_SIZE)

// Constructors for page directory and page table entries.

static inline page_t make_pte(uintptr_t frame, bool write, bool user) {
  page_t pg = 0;

#define BITF_M(V, S) ((V & 0x1) << PTE_##S##_SHIFT)
  pg |= BITF_M(true, PRESENT);
  pg |= BITF_M(true, PWT);
  pg |= BITF_M(write, RW);
  pg |= BITF_M(user, USER);
  pg |= frame & (PTE_ADDR_MASK << PTE_ADDR_SHIFT);
#undef BITF_M

  return pg;
}

static inline pde_t make_pde(uintptr_t frame, bool write, bool user) {
  pde_t pg = 0;
#define BITF_M(V, S) ((V & 0x1) << PDE_##S##_SHIFT)
  pg |= BITF_M(true, PRESENT);
  pg |= BITF_M(true, PWT);
  pg |= BITF_M(write, RW);
  pg |= BITF_M(user, USER);
  pg |= frame & (PDE_ADDR_MASK << PDE_ADDR_SHIFT);
#undef BITF_M

  return pg;
}

static inline void invlpg(uintptr_t addr) {
  __asm__ volatile("invlpg (%0)" : : "r"(addr) : "memory");
}

static inline size_t pde_index(uintptr_t addr) {
  return addr / (PAGE_SIZE * PAGE_TABLE_SIZE);
}
static inline size_t pte_index(uintptr_t addr) {
  return ((addr / PAGE_SIZE) % PAGE_TABLE_SIZE);
}

struct pagefault_state {
  bool present, write, user, reserved, ifetch;
  bool protkey, shadow /* , guard */;
};

#define PAGEFAULT_STATE_STRUCT(r)                                              \
  (struct pagefault_state) {                                                   \
    .present = r->err_code & 0x1, .write = (r->err_code >> 1) & 0x1,           \
    .user = (r->err_code >> 2) & 0x1, .reserved = (r->err_code >> 3) & 0x1,    \
    .ifetch = (r->err_code >> 4) & 0x1,                                        \
  }

#define PAGEFAULT_STATE(r)                                                     \
  int present = (r->err_code & 0x1) != 0;                                      \
  int rw = (r->err_code & 0x2) != 0;                                           \
  int us = (r->err_code & 0x4) != 0;                                           \
  int reserved = (r->err_code & 0x8) != 0;                                     \
  int id = (r->err_code & 0x10) != 0;

struct pte_info parse_page(page_t *pg);
void change_page(page_t *pg, int rw, int us);
void turn_page(page_t *p);

void activate_pgd(pagedir_t pgd);
pagedir_t get_current_pgd();

pagetable_t get_pagetable(pagedir_t dir, uintptr_t vaddr);
page_t *get_page(pagedir_t dir, uintptr_t vaddr);
uintptr_t get_phys_addr(pagedir_t dir,
                        uintptr_t vaddr); // get physical addr from virtual
void map_page(pagedir_t pgd, uintptr_t vaddr, uintptr_t phys, bool user);
void map_addr_fixed(pagedir_t pgd, uintptr_t vaddr, uintptr_t pstart,
                    size_t npages, bool user, bool overwrite);
uintptr_t map_addr(pagedir_t pgd, uintptr_t vaddr, size_t npages, bool user,
                   bool overwrite);
void unmap_page(pagedir_t pgd, uintptr_t vaddr);
void unmap_addr(pagedir_t pgd, uintptr_t vaddr, size_t npages, bool freetbl);
#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define PROT_EXEC 0x4
void mprotect(pagedir_t pgd, uint32_t vaddr, size_t npages, int prot);

// page_t make_pte(uint32_t paddr, int user, int rw);
// pde_t make_pde(uint32_t paddr, int user, int rw);

extern pde_t kernel_pgd[1024] __align(4096);
pagedir_t create_pgd(void);
pagedir_t dup_pgd(pagedir_t original);
pagedir_t clone_pgd(pagedir_t original);

void paging_init();
