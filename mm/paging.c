#include <depthos/paging.h>

#include <depthos/idt.h>

int paging_enabled = 0;

pde_t kernel_pgd[1024] __align(4096);

pde_t *cur_pgd[1024] __align(4096);

static page_t kernel_pgt[1024] __align(4096); /* 768 */
static page_t heap_1_pgt[1024] __align(4096); /* 769 */

#define page_offset(a) (((uint32_t)a) & 0xfff)
#define page_index(a) ((((uint32_t)a) >>12) & 0x3ff)
#define table_index(a) (((uint32_t)a) >> 22)
int pde_index(uint32_t addr){
	return addr >> 22;
}

uintptr_t pte_index(uint32_t addr){
	return (uintptr_t) ((addr / 4096) % 1024);
}

void activate_pgd(pagedir_t pgd){
//	cur_pgd = &pgd;
    __asm __volatile ("mov %0, %%cr3"::"r"((int)pgd - (int)VIRT_BASE));
}

pagedir_t __save_pgd(void){
    uint32_t ret;
    __asm __volatile ("mov %%cr3, %0":"=r"(ret));
    return (pagedir_t) (ret + VIRT_BASE);
}

pagedir_t activate_pgd_save(pagedir_t pgd){
//	cur_pgd = &pgd;	
    pagedir_t ret = __save_pgd();
    __asm __volatile ("mov %0, %%cr3"::"r"((int)pgd - (int)VIRT_BASE));
    return ret;
}


void* get_paddr(pagedir_t dir,void *vaddr) {
	if (!paging_enabled) 
		return (void*)(vaddr - VIRT_BASE);
	pde_t *pdes = dir, pde;
	page_t *ptes;
	page_t page;
	uint32_t addr;
	int pdi = pde_index((uint32_t)vaddr);
	int pti = pte_index((uint32_t)vaddr);

	pde = pdes[pdi];
	pde >>= PDE_ADDR_SHIFT;
	pde <<= 12;
	if(pde == 0)
		return NULL;
	pde += VIRT_BASE;
	ptes = (page_t*)pde;
	page = ptes[pti];
	

	page >>= PTE_ADDR_SHIFT;
	page <<=12;

	if (page == 0)
		return NULL;
	addr = page;
	addr += page_offset(vaddr);


	return (void*)addr;
}

#undef page_offset
#undef page_index
#undef table_index


void turn_page(page_t* p) {
	*p &= ((!(*p << PTE_PRESENT_SHIFT)) << PTE_PRESENT_SHIFT);
}

page_t* get_page(pagedir_t pgd,uint32_t vaddr) {
	int ipde, ipte;

	pde_t *pdes = pgd, pde;

	page_t *ptes;

	ipde = pde_index(vaddr);
	ipte = pte_index(vaddr);

	pde = pdes[ipde];
	pde >>= PDE_ADDR_SHIFT;
	pde <<= 12;
	if (pde == 0)
		return NULL;
	pde += VIRT_BASE;

	ptes = (page_t*)pde;
	return &ptes[ipte];
}


page_t make_pte(uint32_t paddr,int user,int rw) {
	paddr = paddr >> 12;

		return (0x0                      |
		   (1 << PTE_PRESENT_SHIFT)      |
		   (rw << PTE_RW_SHIFT)          |
           (user << PTE_USER_SHIFT)      |
		   (1 << PTE_WRITETHRU_SHIFT)    |
		   (0 << PTE_CACHE_SHIFT)        |
           (0 << PTE_ACCESS_SHIFT)       |
           (0 << PTE_DIRTY_SHIFT)        &
           (PTE_ZERO_MASK)               |
           (0 << PTE_GLOB_SHIFT)         &
           (PTE_AVAIL_MASK)              |
           (paddr << PTE_ADDR_SHIFT)); 
	
}

pde_t make_pde(uint32_t paddr,int user,int rw) {
	
	paddr = paddr >> 12;

	return 0x0                           |
		   (1    << PDE_PRESENT_SHIFT)   |
           (rw   << PDE_RW_SHIFT)        |
           (user << PDE_USER_SHIFT)      |
		   (1    << PDE_WRITETHRU_SHIFT) |
           (0    << PDE_CACHE_SHIFT)     |
           (0    << PDE_ACCESS_SHIFT)    |
		   (0    << PDE_ZERO_SHIFT)      |
           (0    << PDE_SIZE_SHIFT)      |
		   (0    << PDE_IGNORE_SHIFT)    &
		   (PDE_AVAIL_MASK)              |
		   (paddr << PDE_ADDR_SHIFT);
}

void enable_paging() {
#define SET_PGBIT(cr0) (cr0 = cr0 | 0x80000000)
#define CLEAR_PSEBIT(cr4) (cr4 = cr4 & 0xffffffef)

	uint32_t cr0, cr4;

	__asm __volatile ( "mov %%cr4, %0" : "=r"(cr4) :);
	CLEAR_PSEBIT(cr4);
	__asm __volatile ( "mov %0, %%cr4" : :"r"(cr4) :);
	__asm __volatile ( "mov %%cr0, %0" :"=r"(cr0) ::);
	SET_PGBIT(cr0);
	__asm __volatile ( "mov %0,%%cr0" ::"r"(cr0) :);


	paging_enabled = 1;

#undef SET_PGBIT
#undef CLEAR_PSEBIT
}

void __noreturn __do_pf(regs_t r) 
{
	printk("page fault: [%x]");
	while(1);
}

void paging_init() {
	size_t i,j;
	
	__asm __volatile ("cli");
	
	for(i = 0 * 1024 * 1024; i < 4 * 1024 * 1024; i += 4096){
		page_t pg = make_pte(i,0,1);
		kernel_pgt[pte_index(i)] = pg;
//		turn_page(&pg);
	}
//	printk("%x || %x - %x = %x\n", (int)kernel_pgt,(int)kernel_pgt,(int)VIRT_BASE,(uint32_t)((int)kernel_pgt - (int)VIRT_BASE));
	kernel_pgd[pde_index(VIRT_BASE)] = make_pde((uint32_t)(((int)kernel_pgt - (int)VIRT_BASE)),0,1);
	
	

	activate_pgd(kernel_pgd);
	
	reg_intr(14,__do_pf);

	enable_paging();

	print_mod("vmem initialized",MOD_OK);

	__asm __volatile ("sti");
}






