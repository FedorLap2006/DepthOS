
#include <depthos/console.h>
#include <depthos/idt.h>
#include <depthos/heap.h>
#include <depthos/paging.h>
#include <depthos/tools.h>
// #include <depthos/gdt.h>

extern unsigned short *videoMemory;
void print_str(char* str) {

  for (int i = 0; str[i] != '\0'; i++) {
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
enum KEYCODE {

    // Alphanumeric keys ////////////////

    KEY_SPACE = ' ',
    KEY_0 = '0',
    KEY_1 = '1',
    KEY_2 = '2',
    KEY_3 = '3',
    KEY_4 = '4',
    KEY_5 = '5',
    KEY_6 = '6',
    KEY_7 = '7',
    KEY_8 = '8',
    KEY_9 = '9',

    KEY_A = 'a',
    KEY_B = 'b',
    KEY_C = 'c',
    KEY_D = 'd',
    KEY_E = 'e',
    KEY_F = 'f',
    KEY_G = 'g',
    KEY_H = 'h',
    KEY_I = 'i',
    KEY_J = 'j',
    KEY_K = 'k',
    KEY_L = 'l',
    KEY_M = 'm',
    KEY_N = 'n',
    KEY_O = 'o',
    KEY_P = 'p',
    KEY_Q = 'q',
    KEY_R = 'r',
    KEY_S = 's',
    KEY_T = 't',
    KEY_U = 'u',
    KEY_V = 'v',
    KEY_W = 'w',
    KEY_X = 'x',
    KEY_Y = 'y',
    KEY_Z = 'z',

    KEY_RETURN = '\n',
    KEY_ESCAPE = 0x1b,
    KEY_BACKSPACE = 0x7f,

    // Arrow keys ////////////////////////

    KEY_UP = 0x1100,
    KEY_DOWN = 0x1101,
    KEY_LEFT = 0x1102,
    KEY_RIGHT = 0x1103,

    // Function keys /////////////////////

    KEY_F1 = 0x1201,
    KEY_F2 = 0x1202,
    KEY_F3 = 0x1203,
    KEY_F4 = 0x1204,
    KEY_F5 = 0x1205,
    KEY_F6 = 0x1206,
    KEY_F7 = 0x1207,
    KEY_F8 = 0x1208,
    KEY_F9 = 0x1209,
    KEY_F10 = 0x120a,
    KEY_F11 = 0x120b,
    KEY_F12 = 0x120b,
    KEY_F13 = 0x120c,
    KEY_F14 = 0x120d,
    KEY_F15 = 0x120e,

    KEY_DOT = '.',
    KEY_COMMA = ',',
    KEY_COLON = ':',
    KEY_SEMICOLON = ';',
    KEY_SLASH = '/',
    KEY_BACKSLASH = '\\',
    KEY_PLUS = '+',
    KEY_MINUS = '-',
    KEY_ASTERISK = '*',
    KEY_EXCLAMATION = '!',
    KEY_QUESTION = '?',
    KEY_QUOTEDOUBLE = '\"',
    KEY_QUOTE = '\'',
    KEY_EQUAL = '=',
    KEY_HASH = '#',
    KEY_PERCENT = '%',
    KEY_AMPERSAND = '&',
    KEY_UNDERSCORE = '_',
    KEY_LEFTPARENTHESIS = '(',
    KEY_RIGHTPARENTHESIS = ')',
    KEY_LEFTBRACKET = '[',
    KEY_RIGHTBRACKET = ']',
    KEY_LEFTCURL = '{',
    KEY_RIGHTCURL = '}',
    KEY_DOLLAR = '$',
    KEY_POUND = '$',
    KEY_EURO = '$',
    KEY_LESS = '<',
    KEY_GREATER = '>',
    KEY_BAR = '|',
    KEY_GRAVE = '`',
    KEY_TILDE = '~',
    KEY_AT = '@',
    KEY_CARRET = '^',

    // Numeric keypad //////////////////////

    KEY_KP_0 = '0',
    KEY_KP_1 = '1',
    KEY_KP_2 = '2',
    KEY_KP_3 = '3',
    KEY_KP_4 = '4',
    KEY_KP_5 = '5',
    KEY_KP_6 = '6',
    KEY_KP_7 = '7',
    KEY_KP_8 = '8',
    KEY_KP_9 = '9',
    KEY_KP_PLUS = '+',
    KEY_KP_MINUS = '-',
    KEY_KP_DECIMAL = '.',
    KEY_KP_DIVIDE = '/',
    KEY_KP_ASTERISK = '*',
    KEY_KP_NUMLOCK = 0x300f,
    KEY_KP_ENTER = 0x3010,

    KEY_TAB = 0x4000,
    KEY_CAPSLOCK = 0x4001,

    // Modify keys ////////////////////////////

    KEY_LSHIFT = 0x4002,
    KEY_LCTRL = 0x4003,
    KEY_LALT = 0x4004,
    KEY_LWIN = 0x4005,
    KEY_RSHIFT = 0x4006,
    KEY_RCTRL = 0x4007,
    KEY_RALT = 0x4008,
    KEY_RWIN = 0x4009,

    KEY_INSERT = 0x400a,
    KEY_DELETE = 0x400b,
    KEY_HOME = 0x400c,
    KEY_END = 0x400d,
    KEY_PAGEUP = 0x400e,
    KEY_PAGEDOWN = 0x400f,
    KEY_SCROLLLOCK = 0x4010,
    KEY_PAUSE = 0x4011,

    KEY_UNKNOWN,
    KEY_NUMKEYCODES
};

static int scancode_std [] = {
    // key scancode
    KEY_UNKNOWN,    //0
    KEY_ESCAPE,    //1
    KEY_1,    //2
    KEY_2,    //3
    KEY_3,    //4
    KEY_4,    //5
    KEY_5,    //6
    KEY_6,    //7
    KEY_7,    //8
    KEY_8,    //9
    KEY_9,    //0xa
    KEY_0,    //0xb
    KEY_MINUS,    //0xc
    KEY_EQUAL,    //0xd
    KEY_BACKSPACE,    //0xe
    KEY_TAB,    //0xf
    KEY_Q,    //0x10
    KEY_W,    //0x11
    KEY_E,    //0x12
    KEY_R,    //0x13
    KEY_T,    //0x14
    KEY_Y,    //0x15
    KEY_U,    //0x16
    KEY_I,    //0x17
    KEY_O,    //0x18
    KEY_P,    //0x19
    KEY_LEFTBRACKET,//0x1a
    KEY_RIGHTBRACKET,//0x1b
    KEY_RETURN,    //0x1c
    KEY_LCTRL,    //0x1d
    KEY_A,    //0x1e
    KEY_S,    //0x1f
    KEY_D,    //0x20
    KEY_F,    //0x21
    KEY_G,    //0x22
    KEY_H,    //0x23
    KEY_J,    //0x24
    KEY_K,    //0x25
    KEY_L,    //0x26
    KEY_SEMICOLON,    //0x27
    KEY_QUOTE,    //0x28
    KEY_GRAVE,    //0x29
    KEY_LSHIFT,    //0x2a
    KEY_BACKSLASH,    //0x2b
    KEY_Z,    //0x2c
    KEY_X,    //0x2d
    KEY_C,    //0x2e
    KEY_V,    //0x2f
    KEY_B,    //0x30
    KEY_N,    //0x31
    KEY_M,    //0x32
    KEY_COMMA,    //0x33
    KEY_DOT,    //0x34
    KEY_SLASH,    //0x35
    KEY_RSHIFT,    //0x36
    KEY_KP_ASTERISK,//0x37
    KEY_RALT,    //0x38
    KEY_SPACE,    //0x39
    KEY_CAPSLOCK,    //0x3a
    KEY_F1,    //0x3b
    KEY_F2,    //0x3c
    KEY_F3,    //0x3d
    KEY_F4,    //0x3e
    KEY_F5,    //0x3f
    KEY_F6,    //0x40
    KEY_F7,    //0x41
    KEY_F8,    //0x42
    KEY_F9,    //0x43
    KEY_F10,    //0x44
    KEY_KP_NUMLOCK,    //0x45
    KEY_SCROLLLOCK,    //0x46
    KEY_HOME,    //0x47
    KEY_KP_8,    //0x48 //keypad up arrow
    KEY_PAGEUP,    //0x49
    KEY_KP_2,    //0x50 //keypad down arrow
    KEY_KP_3,    //0x51 //keypad page down
    KEY_KP_0,    //0x52 //keypad insert key
    KEY_KP_DECIMAL,    //0x53 //keypad delete key
    KEY_UNKNOWN,    //0x54
    KEY_UNKNOWN,    //0x55
    KEY_UNKNOWN,    //0x56
    KEY_F11,    //0x57
    KEY_F12    //0x58
};

void kb_event(regs_t r) {
	unsigned char status;
	unsigned char keycode;

	status = inb(0x64);
	if ( status & 0x01 ) {
		keycode = inb(0x60);
		if (keycode >= ARRAY_SIZE(scancode_std)) {
			return;
		}
		console_putchar(scancode_std[keycode]);
//		console_write_dec(keycode);
	}
	return;
}

void init_kb() {
	reg_intr(0x20 + 0x1,kb_event);
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

//
	reg_intr(0x80,syscall_event);
	init_timer(1000);
	init_kb();
	

	print_mod("kernel loaded",MOD_OK);

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
	
	console_write_hex(0xD0F);

	printk("Hello %s\n", "world");

	for (;;)
		__asm __volatile ("hlt");
	return;
}
