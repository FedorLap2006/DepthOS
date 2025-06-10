#include "depthos/signal.h"
#include "depthos/tools.h"
#include <depthos/bitmap.h>
#include <depthos/heap.h>
#include <depthos/idt.h>
#include <depthos/kernel.h>
#include <depthos/logging.h>
#include <depthos/paging.h>
#include <depthos/pmm.h>
#include <depthos/proc.h>
#include <depthos/stdbits.h>
#include <depthos/string.h>
#include <depthos/syscall.h>
#include <depthos/vmm.h>

int paging_enabled = 0;

pde_t kernel_pgd[1024] __align(4096);
pde_t *current_pgd __align(4096);

page_t kernel_pgt[1024] __align(4096); /* 768 */
page_t heap_1_pgt[1024] __align(4096);
page_t heap_2_pgt[1024] __align(4096);
page_t heap_3_pgt[1024] __align(4096);

pagetable_t get_pagetable(pagedir_t pgd, uintptr_t vaddr) {
  pde_t pde = pgd[pde_index(vaddr)];
  uintptr_t addr = PDE_EXTRACT_ADDR(pde);
  if (!addr)
    return NULL;
  return (pagetable_t)ADDR_TO_VIRT(addr);
}

page_t *get_page(pagedir_t pgd, uintptr_t vaddr) {
  if (pde_index(vaddr) > 1024 || pte_index(vaddr) > 1024)
    return NULL;

  pagetable_t pde = get_pagetable(pgd, vaddr);
  if (!pde)
    return NULL;

  return &pde[pte_index(vaddr)];
}

uintptr_t get_phys_addr(pagedir_t pgd, uintptr_t vaddr) {
  if (!paging_enabled)
    return ADDR_TO_PHYS(vaddr);
  page_t *pte = get_page(pgd, vaddr);
  if (!pte)
    return INVALID_ADDR;
  uintptr_t addr = PTE_EXTRACT_ADDR(*pte);
  addr += PG_OFFSET(vaddr);

  return addr;
}

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
  klogf("create_pgd: %p", pgd);
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
  // kheap_cache_dump();
  void *buffer = kmalloc(4096);
  // kheap_cache_dump();
  for (int i = 0; i < 1024; i++) {
    if (!original[i] || i * 1024 * 4096 >= VIRT_BASE)
      continue;
    klogf("copying %d pagetable", i);
    pagetable_t table = kmalloc(4096);
    memset(table, 0, 4096);
    pgd[i] =
        make_pde(ADDR_TO_PHYS(table), (original[i] & (1 << PDE_RW_SHIFT)) != 0,
                 (original[i] & (1 << PDE_USER_SHIFT)) != 0);
    for (int j = 0; j < 1024; j++) {
      if (!*get_page(original, i * 1024 * 4096 + j * 4096))
        continue;
      page_t *page = get_page(original, i * 1024 * 4096 + j * 4096);
      table[j] = make_pte(pmm_alloc(1), getbit(*page, PTE_RW_SHIFT),
                          getbit(*page, PTE_USER_SHIFT));
      activate_pgd(original);
      // printk("booya 0x%x\n", i * 1024 * 4096 + j * 4096);
      // klogf("one %p..%p", (void *)(i * 1024 * 4096 + j * 4096),
      //       (void *)(i * 1024 * 4096 + j * 4096) + 4096);
      memcpy(buffer, (void *)(i * 1024 * 4096 + j * 4096), 4096);
      activate_pgd(pgd);
      // printk("yahoo\n");
      // klogf("two %p..%p", (void *)(i * 1024 * 4096 + j * 4096),
      //       (void *)(i * 1024 * 4096 + j * 4096) + 4096);
      memcpy((void *)(i * 1024 * 4096 + j * 4096), buffer, 4096);
      activate_pgd(current_pgd);
      // setbit(&table[j], PTE_COW_SHIFT, true);
      // setbit(page, PTE_RW_SHIFT, 0);
    }
  }
  kfree(buffer, 4096);
  return pgd;
}

void turn_page(page_t *p) {
  *p &= ((!(*p << PTE_PRESENT_SHIFT)) << PTE_PRESENT_SHIFT);
}

void map_addr_fixed(pagedir_t pgd, uintptr_t vaddr, uintptr_t pstart,
                    size_t npages, bool user, bool overwrite) {
  // klogf("=================== test ================");
  vaddr = PG_RND_DOWN(vaddr); // TODO: proper alignment check

  for (int i = 0; i < npages; i++) {
    page_t *page = get_page(pgd, vaddr + i * PAGE_SIZE);
    if (page && *page && !overwrite) // TODO: free the page
      continue;
    map_page(pgd, vaddr + i * PAGE_SIZE, pstart + i * PAGE_SIZE, user);
  }
#if 0
  for (int i = 0; i <= npages / 1024; i++) {
    klogf("mapping 0x%x table", vaddr + i * 4096 * 1024);
    if (!(pgd[pde_index(vaddr + i * 4096 * 1024)] >> PDE_ADDR_SHIFT)) {
      klogf("allocating new table");
      pagetb_t tb = kmalloc(4096);
      memset(tb, 0, 4096);
      pgd[pde_index(vaddr + i * 4096 * 1024)] =
          make_pde(ADDR_TO_PHYS(tb), rw, user);
    }
    klogf("a %d %d %d %d", npages, 0 < 4096, 0 < npages,
          0 < 4096 && 0 < npages);
    for (int j = 0; j < 4096 && j < npages; j++) {
      klogf("mapping 0x%x", vaddr + i * 4096 * 1024 + j * 4096);
      *get_page(pgd, vaddr + i * 4096 * 1024 + j * 4096) =
          make_pte(pmm_alloc(1), rw, user);
    }
  }
#endif
}
uintptr_t map_addr(pagedir_t pgd, uintptr_t vaddr, size_t npages, bool user,
                   bool overwrite) {
  // klogf("mmap: vaddr=%p npages=%d user=%d ow=%d", vaddr, npages, user,
  //       overwrite);
  vaddr = PG_RND_DOWN(vaddr); // TODO: proper alignment check
  uintptr_t phys_start = pmm_alloc(npages);
  if (phys_start < 0) {
    klogf("cannot allocate %ld continuous pages", npages);
    return NULL;
  }

  if (vaddr == 0x8000000) {
    klogf("phys start: %x", phys_start);
  }
  map_addr_fixed(pgd, vaddr, phys_start, npages, user, overwrite);
  return phys_start;
}

void unmap_table(pagedir_t pgd, uintptr_t vaddr) {
  if (pde_index(vaddr) >= 1024)
    return;
  pagetable_t taddr = (pagetable_t)((pgd[pde_index(vaddr)] >> PDE_ADDR_SHIFT)
                                    << PDE_ADDR_SHIFT);
  if (!taddr)
    return;
  taddr = (pagetable_t)ADDR_TO_VIRT(taddr);
  pgd[pde_index(vaddr)] = 0;
  kfree(taddr, PAGE_SIZE);
}

void unmap_addr(pagedir_t pgd, uintptr_t vaddr, size_t npages,
                bool freetbl) { // TODO: test
  // TODO: pmm_free
  uintptr_t cur = vaddr, end = vaddr + npages * PAGE_SIZE;
  if (freetbl) {
    for (; cur < ROUND_UP(vaddr, PAGE_TABLE_SIZE * PAGE_SIZE) && cur < end;
         cur += PAGE_SIZE) {
      unmap_page(pgd, cur);
    }
    for (; cur < ROUND_DOWN(end, PAGE_TABLE_SIZE * PAGE_SIZE) && cur < end;
         cur += PAGE_SIZE * PAGE_TABLE_SIZE) {
      unmap_table(pgd, cur);
    }
  }
  for (; cur < end; cur += PAGE_SIZE) {
    unmap_page(pgd, cur);
  }
}

void unmap_page(pagedir_t pgd, uintptr_t vaddr) {
  if (!get_page(pgd, vaddr))
    return;
  *get_page(pgd, vaddr) = 0;
}

void map_page(pagedir_t pgd, uintptr_t vaddr, uintptr_t phys, bool user) {
  int ti = pde_index(vaddr);
  if (ti != 0 && PDE_EXTRACT_ADDR(pgd[ti]) == 0) {
    // klogf("hello world");
    pagetable_t tb = kmalloc(PAGE_SIZE);
    if (!tb)
      panicf("out of memory");
    memset(tb, 0, PAGE_SIZE);
    // klogf("bye world");
    pgd[pde_index(vaddr)] = make_pde(ADDR_TO_PHYS(tb), true, user);
  }
  page_t *pg = get_page(pgd, vaddr);
  *pg = make_pte(phys, true, user);
}

// 0000100 << 2
// int i = 0000000;
// i = 0000100;
// i >>= 2;

// page_t make_pte(uint32_t paddr, int user, int rw) {
//   return 0x0 | (1 << PTE_PRESENT_SHIFT) | (rw << PTE_RW_SHIFT) |
//          (user << PTE_USER_SHIFT) | (1 << PTE_WRITETHRU_SHIFT) |
//          (0 << PTE_CACHE_SHIFT) | (0 << PTE_ACCESS_SHIFT) |
//          (0 << PTE_DIRTY_SHIFT) & (PTE_ZERO_MASK) |
//          (0 << PTE_GLOB_SHIFT) & (PTE_AVAIL_MASK) | paddr;
// }

// pde_t make_pde(uint32_t paddr, int user, int rw) {
//   return 0x0 | (1 << PDE_PRESENT_SHIFT) | (rw << PDE_RW_SHIFT) |
//          (user << PDE_USER_SHIFT) | (1 << PDE_WRITETHRU_SHIFT) |
//          (0 << PDE_CACHE_SHIFT) | (0 << PDE_ACCESS_SHIFT) |
//          (0 << PDE_ZERO_SHIFT) | (0 << PDE_SIZE_SHIFT) |
//          (0 << PDE_IGNORE_SHIFT) & (PDE_AVAIL_MASK) | paddr;
// }

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
void _do_kernel_pgf(regs_t *r, uintptr_t cr2) { panicf("kernel pagefault"); }

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

int vma_pagefault_handler(regs_t *r, uintptr_t cr2);

void _do_user_pgf(regs_t *r, uintptr_t cr2) {
  // if (getbit(*get_page(get_current_pgd(), cr2), PTE_COW_SHIFT)) {
  //   _do_cow(cr2);
  //   return;
  // }
  signal_send(current_task, (struct signal) {
    .pid = current_task->thid, // TODO: thid
    .number = SIGSEGV,
    .errno = 0xDEAD,
    .fault_addr = cr2,
  });
  // sys_exit(1); // TODO: proper status
}

void pagefault_handler(regs_t *r) {
  uint32_t cr2;
  __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));

  int status = VM_FAULT_CRASH; // TODO: handle buserror
  if (current_task && current_task->vm_areas) {
    int res = vma_pagefault_handler(r, cr2);
    if (res == VM_FAULT_IGNORE)
      return;
  }

  PAGEFAULT_STATE(r);
  if (!console_no_color)
    printk("\x1B[31;1m");

  const char *kind = "";
  switch (status) {
  case VM_FAULT_BUSERROR:
    kind = "bus error";
    break;
  }

  if (strlen(kind))
    printk("page fault (%s) at %p for accessing %p: ", kind, r->eip, cr2);
  else
    printk("page fault at %p for accessing %p: ", r->eip, cr2);
  printk("0x%x (pres=%d rw=%d super=%d reserved=%d id=%d)\n", r->err_code,
         present, rw, us, reserved, id);
  if (!console_no_color)
    printk("\x1B[0m");

  dump_registers(*r);
  trace(3, -1);
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

  // TODO: protect null page

  idt_disable_hwinterrupts();

  extern char _kernel_start;
  uintptr_t kernel_start_addr = ADDR_TO_PHYS((uintptr_t)&_kernel_start);
  extern char _kernel_end;
  uintptr_t kernel_end_addr = ADDR_TO_PHYS((uintptr_t)&_kernel_end);

  klogf("kernel address range is: %p..%p", (void *)kernel_start_addr,
        (void *)kernel_end_addr);
  klogf("mapping kernel pgt: %p..%p", (void *)0,
        (void *)(PAGE_SIZE * PAGE_TABLE_SIZE));

  for (i = 0; i < PAGE_SIZE * PAGE_TABLE_SIZE; i += PAGE_SIZE) {
    page_t pg = make_pte(i, false, false);
    kernel_pgt[pte_index(i)] = pg;
  }

  klogf("mapping heap pgt[0]: %p..%p", (void *)ADDR_TO_PHYS(HEAP_BASE),
        (void *)ADDR_TO_PHYS(HEAP_BASE + PAGE_SIZE * PAGE_TABLE_SIZE));
  klogf("mapping heap pgt[1]: %p..%p",
        (void *)ADDR_TO_PHYS(HEAP_BASE + PAGE_SIZE * PAGE_TABLE_SIZE),
        (void *)ADDR_TO_PHYS(HEAP_BASE + PAGE_SIZE * PAGE_TABLE_SIZE * 2));

  for (i = 0; i < PAGE_TABLE_SIZE; i++) {
    uintptr_t offset = i * PAGE_SIZE;
    page_t pg = make_pte(ADDR_TO_PHYS(HEAP_BASE) + offset, true, false);
    heap_1_pgt[pte_index(HEAP_BASE + offset)] = pg;
  }
  for (i = 0; i < PAGE_TABLE_SIZE; i++) {
    uintptr_t offset = (PAGE_TABLE_SIZE + i) * PAGE_SIZE;
    page_t pg = make_pte(i + ADDR_TO_PHYS(HEAP_BASE) + offset, true, false);
    heap_2_pgt[pte_index(HEAP_BASE + offset)] = pg;
  }
  for (i = 0; i < PAGE_TABLE_SIZE; i++) {
    uintptr_t offset = (PAGE_TABLE_SIZE * 2 + i) * PAGE_SIZE;
    page_t pg = make_pte(i + ADDR_TO_PHYS(HEAP_BASE) + offset, true, false);
    heap_3_pgt[pte_index(HEAP_BASE + offset)] = pg;
  }


  kernel_pgt[pte_index(0)] = make_pte(0, false, false);

  // change_page(&kernel_pgt[0],0,0,1);

  //	printk("%x || %x - %x = %x\n",
  //(int)kernel_pgt,(int)kernel_pgt,(int)VIRT_BASE,(uint32_t)((int)kernel_pgt
  //- (int)VIRT_BASE));
  uint32_t kernel_pgd_entry = make_pde(ADDR_TO_PHYS(kernel_pgt), true, false);
  kernel_pgd[pde_index(VIRT_BASE)] = kernel_pgd_entry;
  // kernel_pgd[pde_index(0)] = kernel_pgd_entry; // TODO: remove when not needed
  kernel_pgd[pde_index(VIRT_BASE + 1 * 1024 * 4096)] =
      make_pde(ADDR_TO_PHYS(heap_1_pgt), true, false);
  kernel_pgd[pde_index(VIRT_BASE + 2 * 1024 * 4096)] =
      make_pde(ADDR_TO_PHYS(heap_2_pgt), true, false);
  kernel_pgd[pde_index(VIRT_BASE + 3 * 1024 * 4096)] =
      make_pde(ADDR_TO_PHYS(heap_3_pgt), true, false);

  idt_register_interrupt(14, pagefault_handler);
  activate_pgd(kernel_pgd);
  enable_paging();
  // idt_enable_hwinterrupts();
}
