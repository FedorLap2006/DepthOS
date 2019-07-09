
#include <depthos/console.h>
#include <depthos/idt.h>
#include <depthos/pmm.h>
#include <depthos/heap.h>
#include <depthos/paging.h>
#include <depthos/serial.h>
#include <depthos/string.h>
//#include <depthos/stdio.h>
#include <depthos/tools.h>
#include <depthos/stdarg.h>
#include <depthos/keyboard.h>
#include <depthos/stdbits.h>
// #include <depthos/gdt.h>

extern unsigned short *videoMemory;
void print_str(char* str) {

  for (int i = 0; str[i] != '\0'; i++) {
    videoMemory[i] = (videoMemory[i] & 0xFF00) | str[i];
  }

}

extern page_t kernel_pgt[1024] __align(4096);

void syscall_event(regs_t r) {
	printk("syscall -- (%d)",r.eax);
	switch(r.eax) {
	case 0:
		console_write("i m syscall");
		break;
	case 1:
		console_write("hello 2!");
		break;
	case 2:
		break;
	default:
		break;
	}	
//	console_write_dec(r.eax);
}


static uint32_t tick = 0;
void ticker(regs_t regs) {
	tick++;
//	if ( (tick % 4) != 0 ) return;
//	console_write("tick:");
//	console_write_dec(tick);
//	console_write("\n");
}
void init_timer(uint32_t freq) {
	reg_intr(0x20 + 0x0,ticker);
	uint32_t divisor = 1193180 / freq;
	outb(0x43,0x36);
	uint8_t l = (uint8_t)(divisor & 0xFF);
	uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);

	outb(0x40,l);
	outb(0x40,h);
}	

struct multiboot_information {
	uint32_t flags;
	uint32_t mem_lower;
	uint32_t mem_upper;
	uint32_t boot_device;
	const char *cmdline;
};

// extern heap_t *kern_heap;
extern uint32_t end;
void kmain(int magic, struct multiboot_information *boot_ptr) {

//	print_str("hello world! ");
	console_init(
		25,
		80,
		0,
		BGRAY_COLOR
	);
	if (strstr(boot_ptr->cmdline, "console=ttyS0")) {
	    serial_console_init(0);
	}

//	console_write("[ II ] WWWW ");
//	console_putchar_color('*',BLACK_COLOR,RED_COLOR);
//	console_write_color("[ OK ] kernel loaded",BLACK_COLOR,RED_COLOR);
	
	/* byte_t m1[] = "kernel loaded \n";
	console_write("[",1);
	print_ok();
	console_write("]",1);
	console_write_color(m1,sizeof(m1),0,WBLUE_COLOR);

//	print_mod_msg(m1,MOD_OK);
	// byte_t m2[] = "[OK] console system initialized\n";
//	console_write_color(m2,sizeof(m2),0,WBLUE_COLOR);
//	print_mod_msg("console system initialized",MOD_OK);
//	print_mod_msg("some error",MOD_ERROR);
*/
//	gdt_init();
	print_mod("GDT initialized",MOD_OK);
	idt_init();
//	reg_intr(0x20 + 0x1,kb_event);
//	pmm_init(1096 * (1024 * 1024));
	


	paging_init();

	reg_intr(0x80,syscall_event);
	
	init_timer(1000);

	__kb_driver_init();
	

	print_mod("kernel loaded",MOD_OK);
	
	console_putchar('\n');
	char welcome[] = "Welcome to DepthOS v";

	console_write_color(welcome,-1,WGREEN_COLOR);
	
	console_write_color(OSVER,-1,WGREEN_COLOR);
	console_putchar('\n');

/*	*/
	/*char user[] = "root";
	console_write_color(user,-1,PINK_COLOR);
	console_putchar_color('@',-1,GREEN_COLOR);
	console_write_color("depthos",-1,BROWN_COLOR);
	console_putchar_color('#',-1,GREEN_COLOR);
	console_putchar(' ');**/
	
/*	struct __mmh heap;

	init_heap(&heap,100);

	char* one = (char*)malloc(sizeof(char) * 5,&heap);
	one[0] = 'h';
	one[1] = 'e';
	one[2] = 'l';
	one[3] = 'l';
	one[4] = 'o';
	
	printk("0x%x - %s\n",one,one);
	
	free(one,&heap);

 	char* two = (char*)malloc(sizeof(char) * 5,&heap);
	two[0] = 'n';
	two[1] = 'e';
	two[2] = 'w';
	two[3] = '!';
	two[4] = '!';
	
	printk("0x%x - %s\n",two,two);
	
	printk("0x%x - %s\n",one,one);

	
	free(two,&heap);
*/
	int *ptr = (int*)(4 * 1024 * 1024 + 4097);
	
	pageinfo_t pgi;

	pgi = parse_page(&kernel_pgt[1]);
	
	printk("%d,%d: %d\n",pgi.pres,pgi.us,pgi.frame);
	printk("%d,%d: %d\n",getbit(*pgi.pg,PTE_PRESENT_SHIFT),getbit(*pgi.pg,PTE_RW_SHIFT),getbit(*pgi.pg,PTE_USER_SHIFT));


	printk("%d",*ptr);


	for (;;)
		__asm __volatile ("hlt");
	return;
}
