#include <depthos/logging.h>
#include <depthos/paging.h>

#include <depthos/bitmap.h>
#include <depthos/heap.h>
#include <depthos/idt.h>
#include <depthos/kernel.h>
#include <depthos/pmm.h>
#include <depthos/proc.h>
#include <depthos/stdbits.h>
#include <depthos/string.h>

int paging_enabled = 0;

pde_t kernel_pgd[1024] __align(4096);
pde_t *current_pgd __align(4096);

page_t kernel_pgt[1024] __align(4096); /* 768 */
page_t heap_1_pgt[1024] __align(4096);

#define page_offset(a) (((uint32_t)a) & 0xfff)
#define page_index(a) ((((uint32_t)a) >> 12) & 0x3ff)
#define table_index(a) (((uint32_t)a) >> 22)

int pde_index(uint32_t addr) { return addr >> 22; }
uintptr_t pte_index(uint32_t addr) { return (uintptr_t)((addr / 4096) % 1024); }

void activate_pgd(pagedir_t pgd) {
  __asm__ __volatile("mov %0, %%cr3" ::"r"(ADDR_TO_PHYS(pgd)));
  current_pgd = pgd;
}

pagedir_t get_current_pgd(void) {
  uint32_t ret;
  __asm__ __volatile("mov %%cr3, %0" : "=r"(ret));
  return (pagedir_t)ADDR_TO_VIRT(ret);
}

pagedir_t create_pgd() {
  pagedir_t pgd = kmalloc(4096);
  memset(pgd, 0, sizeof(kernel_pgd));
  memcpy(&pgd[pde_index(VIRT_BASE)], &kernel_pgd[pde_index(VIRT_BASE)],
         sizeof(kernel_pgd) - pde_index(VIRT_BASE) * sizeof(pde_t));
  return pgd;
}

pagedir_t dup_pgd(pagedir_t original) {
  pagedir_t pgd = kmalloc(4096);
  memcpy(pgd, original, 4096);
  return pgd;
}

pagedir_t clone_pgd(pagedir_t original) {
  pagedir_t pgd = dup_pgd(original);
  pagedir_t current_pgd = get_current_pgd();
  kheap_cache_dump();
  void *buffer = kmalloc(4096);
  kheap_cache_dump();
  for (int i = 0; i < 1024; i++) {
    if (!original[i] || i * 1024 * 4096 >= VIRT_BASE)
      continue;
    pagetb_t table = kmalloc(4096);
    memset(table, 0, 4096);
    pgd[i] = make_pde(ADDR_TO_PHYS(table), (original[i] >> PDE_USER_SHIFT) != 0,
                      (original[i] >> PDE_RW_SHIFT) != 0);
    for (int j = 0; j < 1024; j++) {
      if (!*get_page(original, i * 1024 * 4096 + j * 4096))
        continue;
      page_t *page = get_page(original, i * 1024 * 4096 + j * 4096);
      table[j] = make_pte(pmm_alloc(1), getbit(*page, PTE_USER_SHIFT),
                          getbit(*page, PTE_RW_SHIFT));
      activate_pgd(original);
      // printk("booya 0x%x\n", i * 1024 * 4096 + j * 4096);
      memcpy(buffer, i * 1024 * 4096 + j * 4096, 4096);
      activate_pgd(pgd);
      // printk("yahoo\n");
      memcpy(i * 1024 * 4096 + j * 4096, buffer, 4096);
      activate_pgd(current_pgd);
      // setbit(&table[j], PTE_COW_SHIFT, true);
      // setbit(page, PTE_RW_SHIFT, 0);
    }
  }
  kfree(buffer, 4096);
  return pgd;
}

void *get_paddr(pagedir_t dir, void *virtual_address) {
  if (!paging_enabled)
    return (void *)ADDR_TO_PHYS(virtual_address);
  pde_t *pdes = dir, pde;
  page_t *ptes;
  page_t page;
  uint32_t addr;
  int pdi = pde_index((uint32_t)virtual_address);
  int pti = pte_index((uint32_t)virtual_address);

  pde = pdes[pdi];
  pde >>= PDE_ADDR_SHIFT;
  pde <<= 12;
  if (pde == 0)
    return NULL;
  pde += VIRT_BASE;
  ptes = (page_t *)pde;
  page = ptes[pti];

  page >>= PTE_ADDR_SHIFT;
  page <<= 12;

  if (page == 0)
    return NULL;
  addr = page;
  addr += page_offset(virtual_address);

  return (void *)addr;
}

#undef page_offset
#undef page_index
#undef table_index

void turn_page(page_t *p) {
  *p &= ((!(*p << PTE_PRESENT_SHIFT)) << PTE_PRESENT_SHIFT);
}

page_t *get_page(pagedir_t pgd, uint32_t vaddr) {
  int ipde, ipte;

  pde_t *pdes = pgd, pde;

  page_t *ptes;

  pde = pgd[pde_index(vaddr)];
  pde >>= PDE_ADDR_SHIFT;
  pde <<= 12;
  if (!pde)
    return NULL;

  pde += VIRT_BASE;

  return (page_t *)pde + pte_index(vaddr);
}

void map_addr(pagedir_t pgd, uint32_t vaddr, size_t npages, bool user) {
  // klogf("=================== test ================");
  map_addr_phys(pgd, vaddr, pmm_alloc(npages), user);
#if 0
  for (int i = 0; i <= npages / 1024; i++) {
    klogf("mapping 0x%x table", vaddr + i * 4096 * 1024);
    if (!(pgd[pde_index(vaddr + i * 4096 * 1024)] >> PDE_ADDR_SHIFT)) {
      klogf("allocating new table");
      pagetb_t tb = kmalloc(4096);
      memset(tb, 0, 4096);
      pgd[pde_index(vaddr + i * 4096 * 1024)] =
          make_pde(ADDR_TO_PHYS(tb), user, rw);
    }
    klogf("a %d %d %d %d", npages, 0 < 4096, 0 < npages,
          0 < 4096 && 0 < npages);
    for (int j = 0; j < 4096 && j < npages; j++) {
      klogf("mapping 0x%x", vaddr + i * 4096 * 1024 + j * 4096);
      *get_page(pgd, vaddr + i * 4096 * 1024 + j * 4096) =
          make_pte(pmm_alloc(1), user, rw);
    }
  }
#endif
}

void map_addr_phys(pagedir_t pgd, uintptr_t vaddr, uintptr_t phys, bool user) {
  if (!(pgd[pde_index(vaddr)] >> PDE_ADDR_SHIFT)) {
    // klogf("hello world");
    pagetb_t tb = kmalloc(0x1000);
    memset(tb, 0, 0x1000);
    // klogf("bye world");
    pgd[pde_index(vaddr)] = make_pde(ADDR_TO_PHYS(tb), user, true);
  }

  *get_page(pgd, vaddr) = make_pte(phys, user, true);
}

// 0000100 << 2
// int i = 0000000;
// i = 0000100;
// i >>= 2;

page_t make_pte(uint32_t paddr, int user, int rw) {
  return 0x0 | (1 << PTE_PRESENT_SHIFT) | (rw << PTE_RW_SHIFT) |
         (user << PTE_USER_SHIFT) | (1 << PTE_WRITETHRU_SHIFT) |
         (0 << PTE_CACHE_SHIFT) | (0 << PTE_ACCESS_SHIFT) |
         (0 << PTE_DIRTY_SHIFT) & (PTE_ZERO_MASK) |
         (0 << PTE_GLOB_SHIFT) & (PTE_AVAIL_MASK) | paddr;
}

pde_t make_pde(uint32_t paddr, int user, int rw) {
  return 0x0 | (1 << PDE_PRESENT_SHIFT) | (rw << PDE_RW_SHIFT) |
         (user << PDE_USER_SHIFT) | (1 << PDE_WRITETHRU_SHIFT) |
         (0 << PDE_CACHE_SHIFT) | (0 << PDE_ACCESS_SHIFT) |
         (0 << PDE_ZERO_SHIFT) | (0 << PDE_SIZE_SHIFT) |
         (0 << PDE_IGNORE_SHIFT) & (PDE_AVAIL_MASK) | paddr;
}

void enable_paging() {
  uint32_t cr0, cr4;
  __asm __volatile("mov %%cr4, %0" : "=r"(cr4) :);

  cr4 = cr4 & 0xffffffef;
  __asm __volatile("mov %0, %%cr4" : : "r"(cr4) :);
  __asm __volatile("mov %%cr0, %0" : "=r"(cr0)::);
  cr0 |= 0x80000000;
  __asm __volatile("mov %0,%%cr0" ::"r"(cr0) :);

  paging_enabled = 1;
}

#define PAGEFAULT_STATE(r)                                                     \
  int present = (r->err_code & 0x1) != 0;                                      \
  int rw = (r->err_code & 0x2) != 0;                                           \
  int us = (r->err_code & 0x4) != 0;                                           \
  int reserved = (r->err_code & 0x8) != 0;                                     \
  int id = (r->err_code & 0x10) != 0;

__noreturn void _do_kernel_pgf(regs_t *r, uintptr_t cr2) {
  panicf("Kernel panic");
}

// void _do_cow(uintptr_t cr2) {
//   void *buffer = kmalloc(4096);
//   memcpy(buffer, cr2, 4096);
//   *get_page(current_task->pgd, cr2) = make_pte(pmm_alloc(1), 1, 1);
//   invlpg(cr2);
//   memcpy(cr2, buffer, 4096);
//   kfree(buffer, 4096);

//   if (current_task->parent)
//     setbit(get_page(current_task->parent->pgd, cr2), 1, 1);
// }

void _do_user_pgf(regs_t *r, uintptr_t cr2) {
  // if (getbit(*get_page(get_current_pgd(), cr2), PTE_COW_SHIFT)) {
  //   _do_cow(cr2);
  //   return;
  // }

  sys_exit();
}

void pagefault_handler(regs_t *r) {
  uint32_t cr2;
  __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
  PAGEFAULT_STATE(r);
  printk("Page fault [0x%x] at 0x%x: ", cr2, r->eip);
  printk("0x%x (pres=%d rw=%d super=%d reserved=%d id=%d)\n", r->err_code,
         present, rw, us, reserved, id);
  dump_registers(*r);
  trace(1, -1);
#if 0
  extern bitmap_t kheap_bitmap;
  bitmap_dump_compact(&kheap_bitmap, 0, -1, 32);
#endif
  if (r->eip >= VIRT_BASE)
    _do_kernel_pgf(r, cr2);
  else
    _do_user_pgf(r, cr2);
}

#undef PAGEFAULT_STATE

void paging_init() {
  size_t i, j;

  idt_disable_hwinterrupts();
  for (i = 0 * 1024 * 1024; i < 4 * 1024 * 1024; i += 4096) {
    page_t pg = make_pte(i, 0, 1);
    kernel_pgt[pte_index(i)] = pg;
  }
  for (i = 0 * 1024 * 1024; i < 4 * 1024 * 1024; i += 4096) {
    page_t pg = make_pte(i + 1 * 1024 * 4096, 0, 1);
    heap_1_pgt[pte_index(i)] = pg;
  }

  // change_page(&kernel_pgt[0],0,0,1);

  //	printk("%x || %x - %x = %x\n",
  //(int)kernel_pgt,(int)kernel_pgt,(int)VIRT_BASE,(uint32_t)((int)kernel_pgt
  //- (int)VIRT_BASE));
  uint32_t kernel_pgd_entry = make_pde(ADDR_TO_PHYS(kernel_pgt), 0, 1);
  kernel_pgd[pde_index(VIRT_BASE)] = kernel_pgd_entry;
  kernel_pgd[pde_index(0)] = kernel_pgd_entry; // TODO: remove when not needed
  kernel_pgd[pde_index(VIRT_BASE + 1 * 1024 * 4096)] =
      make_pde(ADDR_TO_PHYS(heap_1_pgt), 0, 1);

  idt_register_interrupt(14, pagefault_handler);
  activate_pgd(kernel_pgd);
  enable_paging();
  idt_enable_hwinterrupts();
  print_status("vmem initialized", MOD_OK);
}

pageinfo_t parse_page(page_t *pg) {
  pageinfo_t pgi;

  pgi.pg = pg;

  //	printk("0x%x,0x%x\n",pgi.pg,pg);

  //
  // >> 6 = 101000000

  pgi.pres = getbit(*pg, PTE_PRESENT_SHIFT);    // & 0x1;
  pgi.rw = getbit(*pg, PTE_RW_SHIFT);           //& 0x2;
  pgi.us = getbit(*pg, PTE_USER_SHIFT);         //& 0x4;
  pgi.pwt = getbit(*pg, PTE_WRITETHRU_SHIFT);   //& 0x8;
  pgi.pcd = getbit(*pg, PTE_CACHE_SHIFT);       //& 0x10;
  pgi.accessed = getbit(*pg, PTE_ACCESS_SHIFT); //& 0x20;
  pgi.dirty = getbit(*pg, PTE_DIRTY_SHIFT);     //& 0x40;
  pgi.pat = getbit(*pg, PTE_ZERO_SHIFT);        //& 80;
  pgi.glob = getbit(*pg, PTE_GLOB_SHIFT);       //& 0x100;
  pgi.frame = *pg & 0xFFFFF000;

  return pgi;
}

void change_page(page_t *pg, int pres, int rw, int us) {
  //	printk("vals - %d,%d,%d\n",pgi.pres,pgi.rw,pgi.us);
  //	printk("old - %d - %d -
  //%d\n",getbit(*pgi.pg,PTE_PRESENT_SHIFT),getbit(*pgi.pg,PTE_RW_SHIFT),getbit(*pgi.pg,PTE_USER_SHIFT));
  setbit(pg, PTE_PRESENT_SHIFT, pres);
  setbit(pg, PTE_RW_SHIFT, rw);
  setbit(pg, PTE_USER_SHIFT, us);

  //	*pgi.pg = (pgi.pres	 << PTE_PRESENT_SHIFT)	| (*pgi.pg & (~(1 <<
  // PTE_PRESENT_SHIFT)));
  // 	*pgi.pg = (pgi.rw	 << PTE_RW_SHIFT)		| (*pgi.pg &
  // (~(1
  // << PTE_RW_SHIFT)));
  //	*pgi.pg = (pgi.us	 << PTE_USER_SHIFT)		| (*pgi.pg &
  //(~(1
  //<< PTE_USER_SHIFT)));

  //	printk("new - %d - %d -
  //%d\n",getbit(*pgi.pg,PTE_PRESENT_SHIFT),getbit(*pgi.pg,PTE_RW_SHIFT),getbit(*pgi.pg,PTE_USER_SHIFT));
  //	printk("%d",pg);
}
