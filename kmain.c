
#include <depthos/console.h>
#include <depthos/idt.h>

extern unsigned short *videoMemory;
void print_str(char* str) {

  for (int i = 0; i < str[i] != '\0'; i++) {
    videoMemory[i] = (videoMemory[i] & 0xFF00) | str[i];
  }

}


void syscall_event(regs_t r) {
	switch(r.eax) {
	case 0:
		console_write("hello!");
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

void kb_event(regs_t r) {
	console_write("new!");
	unsigned char status;
	char keycode;

	status = inb(0x64);
	if ( status & 0x01 ) {
		keycode = inb(0x60);
		if ( keycode < 0 ) {
			return;
		}
		console_write_dec(keycode);
	}
	return;
}
static uint32_t tick = 0;
void ticker(regs_t regs) {
	tick++;
	console_write("tick:");
	console_write_dec(tick);
	console_write("\n");
}
void init_timer(uint32_t freq) {
	reg_intr(32,ticker);
	uint32_t divisor = 1193180 / freq;
	outb(0x43,0x36);
	uint8_t l = (uint8_t)(divisor & 0xFF);
	uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);

	outb(0x40,l);
	outb(0x40,h);
}	


void kmain(int magic,void *boot_ptr) {
//	print_str("hello world! ");
	console_init(
		25,
		80,
		0,
		BGRAY_COLOR
	);
//	console_write("[ II ] WWWW ");
//	console_putchar_color('*',BLACK_COLOR,RED_COLOR);
//	console_write_color("[ OK ] kernel loaded",BLACK_COLOR,RED_COLOR);
	print_mod("kernel loaded",MOD_OK);
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
	idt_init();
	reg_intr(33,kb_event);
	reg_intr(0x80,syscall_event);
//	init_timer(50);

	console_putchar('\n');
	char welcome[] = "Welcome to DepthOS v";

	console_write_color(welcome,-1,WGREEN_COLOR);
	
	console_write_color(OSVER,-1,WGREEN_COLOR);
	console_putchar('\n');

/*	*/
	char user[] = "root";
	console_write_color(user,-1,PINK_COLOR);
	console_putchar_color('@',-1,GREEN_COLOR);
	console_write_color("depthos",-1,BROWN_COLOR);
	console_putchar_color('#',-1,GREEN_COLOR);
	console_putchar(' ');
	

	__asm ( "movl $0x1,%%eax" : );
	__asm ( "int $0x80" );
	return;
}
