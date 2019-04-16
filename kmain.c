
#include <depthos/console.h>

extern unsigned short *videoMemory;
void print_str(char* str) {

  for (int i = 0; i < str[i] != '\0'; i++) {
    videoMemory[i] = (videoMemory[i] & 0xFF00) | str[i];
  }

}


enum {
	MOD_INIT,
	MOD_PROC,
	MOD_ERR,
	MOD_END
};

void print_mod(char* buf,int m) {
	switch(m) {
		case MOD_INIT: {
			console_putchar('[');
			console_write_color("OK",WGREEN_COLOR,-1);
			console_putchar(']');
			console_putchar(' ');
			console_write_color(buf,-1,WBLUE_COLOR);
			console_putchar('\n');
		   break;
		}
		case MOD_END: {
		  break;
		}
		default:
			break;
	}
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
	print_mod("kernel loaded",MOD_INIT);
	/* byte_t m1[] = "kernel loaded \n";
	console_write("[",1);
	print_ok();
	console_write("]",1);
	console_write_color(m1,sizeof(m1),0,WBLUE_COLOR);

//	print_mod_msg(m1,MOD_INIT);
	// byte_t m2[] = "[OK] console system initialized\n";
//	console_write_color(m2,sizeof(m2),0,WBLUE_COLOR);
//	print_mod_msg("console system initialized",MOD_INIT);
//	print_mod_msg("some error",MOD_ERROR);

	byte_t welcome[] = "Welcome to DepthOS v";

	size_t wcount = sizeof(welcome)/sizeof(byte_t);
	console_write_color(welcome,wcount,0,WGREEN_COLOR);
	
	byte_t *osver = (byte_t*)OSVER;
	*/
	
//	while(1) {}
}
