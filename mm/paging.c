#include <depthos/paging.h>

#include <depthos/idt.h>

#include <depthos/stdbits.h>

int paging_enabled = 0;

pde_t kernel_pgd[1024] __align(4096);

pde_t *cur_pgd __align(4096);

page_t kernel_pgt[1024] __align(4096); /* 768 */
//page_t heap_mem_pgt[1024] __align(4096);
page_t heap_blks_pgt1[1024] __align(4096);
// page_t heap_blks_pgt2[1024] __align(4096);
// page_t heap_blks_pgt3[1024] __align(4096);
page_t proc_dirs_pgt[1024] __align(4096); /* 769 */ // for processes
page_t proc_tbs_pgt[1024] __align(4096); /* 769 */ // for processes
pagetb_t end_pgt __align(4096);

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
	cur_pgd = pgd;
  __asm __volatile ("mov %0, %%cr3"::"r"((int)pgd - (int)VIRT_BASE));
}

pagedir_t __save_pgd(void){
  uint32_t ret;
  __asm __volatile ("mov %%cr3, %0":"=r"(ret));
  return (pagedir_t) (ret + VIRT_BASE);
}

pagedir_t activate_pgd_save(pagedir_t pgd){
	cur_pgd = pgd;	
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
// 0000100 << 2 
// int i = 0000000;
// i = 0000100;
// i >>= 2;

page_t make_pte(uint32_t paddr,int user,int rw) {
	//printk("<%d>",paddr);	
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

#define getPfState(r) \
	int present = r.err_code & 0x1;  \
    int rw = r.err_code & 0x2;       \
    int us = r.err_code & 0x4;       \
    int reserved = r.err_code & 0x8; \
    int id = r.err_code & 0x10;      \


void __noreturn __do_fatal_pf(regs_t r) {
	while(1);
}

void __do_soft_pf(regs_t r) {
	__do_fatal_pf(r);
	return;
}

void __do_pf(regs_t r) 
{
	uint32_t cr2;
    __asm __volatile ("mov %%cr2, %0":"=r"(cr2));
	printk("page fault: [0x%x] ",cr2);

#if 1
	getPfState(r);	
	printk("( .pres = %d, .rw = %d, .supervisor = %d, .reserved = %d, .id = %d )",present,rw,us,reserved,id);
#endif

	if(present == 0) {
		__do_soft_pf(r);
	}
	else {
		__do_fatal_pf(r);
	}
}


#undef getPfState

void paging_init() {
	size_t i,j;
	
	__asm __volatile ("cli");
	
	for(i = 0 * 1024 * 1024; i < 1 * 4 * 1024 * 1024; i += 4096){
		page_t pg = make_pte(i,0,1);
		//mod_log(__func__,"alloc pages ( kernel ) - [%d](%d)",pte_index(i),i);
		kernel_pgt[pte_index(i)] = pg;
		//mod_log(__func__,"alloc pages ( kernel space ) - [%d](%d) <%d> { .rw = %d, .us = %d .frame = %d }\n",
      //pte_index(i),j,i,parse_page(&pg).rw,parse_page(&pg).us,parse_page(&pg).frame);
//		turn_page(&pg);
	}
	//printk("<%d>",i);	
	for(j = 0; i < 3 * 4 * 1024 * 1024, j < 1 * 4 * 1024 * 1024; i += 4096, j += 4096){
		//printk("<%d>",i);	
		page_t pg = make_pte(i,0,1);
		//mod_log(__func__,"alloc pages ( heap blks ) - [%d](%d) <%d> { .rw = %d, .us = %d .frame = %d }\n",
//      pte_index(j),j,i,parse_page(&pg).rw,parse_page(&pg).us,parse_page(&pg).frame);
		heap_blks_pgt1[pte_index(j)] = pg;

//		turn_page(&pg);
	}

	


	//change_page(&kernel_pgt[0],0,0,1);

//	printk("%x || %x - %x = %x\n", (int)kernel_pgt,(int)kernel_pgt,(int)VIRT_BASE,(uint32_t)((int)kernel_pgt - (int)VIRT_BASE));
	kernel_pgd[pde_index(VIRT_BASE + 0 * 4 * 1024 * 1024)] = make_pde((uint32_t)(((int)kernel_pgt 		- (int)VIRT_BASE)),0,1);
	kernel_pgd[pde_index(VIRT_BASE + 1 * 4 * 1024 * 1024)] = make_pde((uint32_t)(((int)heap_blks_pgt1	- (int)VIRT_BASE)),0,1);
//	change_page	
//	end_pgt = heap_blks_pgt;
	activate_pgd(kernel_pgd);
	
	reg_intr(14,__do_pf);

	enable_paging();

	print_mod("vmem initialized",MOD_OK);

	__asm __volatile ("sti");
}



pageinfo_t parse_page(page_t* pg) {
	pageinfo_t pgi;

	pgi.pg = pg;

//	printk("0x%x,0x%x\n",pgi.pg,pg);
	
	
	
	//
	// >> 6 = 101000000
	
	pgi.pres	 = getbit(*pg,PTE_PRESENT_SHIFT); // & 0x1;
	pgi.rw		 = getbit(*pg,PTE_RW_SHIFT); //& 0x2;
	pgi.us		 = getbit(*pg,PTE_USER_SHIFT); //& 0x4;
	pgi.pwt		 = getbit(*pg,PTE_WRITETHRU_SHIFT); //& 0x8;
	pgi.pcd		 = getbit(*pg,PTE_CACHE_SHIFT); //& 0x10;
	pgi.accessed = getbit(*pg,PTE_ACCESS_SHIFT); //& 0x20;
	pgi.dirty	 = getbit(*pg,PTE_DIRTY_SHIFT);//& 0x40;
	pgi.pat 	 = getbit(*pg,PTE_ZERO_SHIFT); //& 80;
	pgi.glob	 = getbit(*pg,PTE_GLOB_SHIFT); //& 0x100;
	uint32_t temp = *pg & 0xFFFFF000;
	pgi.frame = *pg & 0xFFFFF000;
	printk(" :: 0x%x(0x%x) => 0x%x, 0x%x :: ", &pgi, ((char*)&pgi + 13), pgi.frame, temp);
	return pgi;	
}

void change_page(page_t* pg,int pres,int rw,int us) {
//	printk("vals - %d,%d,%d\n",pgi.pres,pgi.rw,pgi.us);
//	printk("old - %d - %d - %d\n",getbit(*pgi.pg,PTE_PRESENT_SHIFT),getbit(*pgi.pg,PTE_RW_SHIFT),getbit(*pgi.pg,PTE_USER_SHIFT));
	setbit(pg,PTE_PRESENT_SHIFT,pres);
	setbit(pg,PTE_RW_SHIFT,rw);
	setbit(pg,PTE_USER_SHIFT,us);


//	*pgi.pg = (pgi.pres	 << PTE_PRESENT_SHIFT)	| (*pgi.pg & (~(1 << PTE_PRESENT_SHIFT)));
// 	*pgi.pg = (pgi.rw	 << PTE_RW_SHIFT)		| (*pgi.pg & (~(1 << PTE_RW_SHIFT)));
//	*pgi.pg = (pgi.us	 << PTE_USER_SHIFT)		| (*pgi.pg & (~(1 << PTE_USER_SHIFT)));

//	printk("new - %d - %d - %d\n",getbit(*pgi.pg,PTE_PRESENT_SHIFT),getbit(*pgi.pg,PTE_RW_SHIFT),getbit(*pgi.pg,PTE_USER_SHIFT));
//	printk("%d",pg);
}
