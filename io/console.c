#include <depthos/console.h>
#include <depthos/stdarg.h>
#include <depthos/vformat.h>

unsigned short* videoMemory = (unsigned short*)0xb8000;

int strs_count = 25;
int strs_len = 80;

int dbcolor = 0;
int dfcolor = WHITE_COLOR;

int cursorx = 0, cursory = 0;

void mod_loga(char *file,int line,char *mod,char *msg,...) {
	static char mlog_s[sizeof(msg)];                         
	va_list args;
	va_start(args,msg);
  vsprintf(mlog_s,msg,args); 
  va_end(args);
  printk("(%s:%d) [%s]: %s\n",file,line,mod,mlog_s);
}

void print_mod(char* buf,int m) {
	switch(m) {
		case MOD_OK: {
			console_putchar('[');
			console_write_color("OK",-1,WGREEN_COLOR);
//			console_putchar_color(0xFB,-1,WGREEN_COLOR);
			console_putchar(']');
			console_putchar(' ');
			console_write_color(buf,-1,WBLUE_COLOR);
			console_putchar('\n');
		   break;
		}
		case MOD_ERR: {
			console_putchar('[');
			console_write_color("ERROR",-1,PINK_COLOR);
//			console_putchar_color(0xFB,-1,WGREEN_COLOR);
			console_putchar(']');
			console_putchar(' ');
			console_write_color(buf,-1,WBLUE_COLOR);
			console_putchar('\n');
		  break;
		}
		default:
			break;
	}
}



void console_init(int s,int l,int b, int f) {
	if ( s > 0 )
		strs_count = s;
	if ( l > 0 )
		strs_len = l;
	if ( b > 0 )
		dbcolor = b;
	if ( f > 0 )
		dfcolor = f;
	console_clear();
	print_mod("console initialized",MOD_OK);
}

void console_movec(int x,int y) {
	cursorx += x;
	cursory += y;
	console_flushc();
}

void console_flushc() {
	uint16_t pos = ( cursory * strs_len ) + cursorx;
	outb(0x3D4, 14);
	outb(0x3D5, pos >> 8);
	outb(0x3D4, 15);
	outb(0x3D5, pos);
}

void console_clear() {
	uint8_t ab = (0 << 4) | (15 & 0x0F);
	uint16_t c = 0x20 | (ab << 8);
	for(int i = 0;i < strs_len * strs_count; i++)
		videoMemory[i] = c;
	cursorx = 0;
	cursory = 0;
	console_flushc();
}

void console_flushs() {
	uint8_t at = ( dbcolor << 4 ) | ( dfcolor & 0x0F );
	uint16_t b = 0x20 | ( at << 8 );

	if ( cursory >= strs_count ) {
		for ( int i = 0; i < ( strs_count-1 ) * strs_len; i++ ) {
			videoMemory[i] = videoMemory[i+strs_len];
		}
		for ( int i = ( strs_count - 1 ) * strs_len; i < strs_count * strs_len; i++ ) {
		   videoMemory[i] = b;
		}
 		cursory = strs_count - 1;		
	}
}

void console_putchara(unsigned char c,uint8_t a) {
	uint16_t attr = a << 8;
	uint16_t *loc;


	if ( c == 0x08 && cursorx ) {
		--cursorx;
	}
	else if ( c == 0x09 ) {
		cursorx = (cursorx + 8) & ~(8-1);
	}
	else if ( c == '\r' ) {
		cursorx = 0;
	}
	else if ( c == '\n' ) {
		cursorx = 0;
		++cursory;	
	}
	else if ( c >= ' ') {
		loc = videoMemory + ( cursory*strs_len + cursorx );
		*loc = c | attr;
		++cursorx;
	}
	if ( cursorx >= strs_len || cursorx == strs_len - 1) { // 79 80
		console_putchara('\n',( dbcolor << 4 ) | ( dfcolor & 0x0F));
//		cursorx = 0;
//		++cursory;
	}
	console_flushs();
	console_flushc();
}


void console_putchar(unsigned char c) {
	console_putchara(c, ( dbcolor << 4 ) | ( dfcolor & 0x0F));
}

void console_write(const char* buf) {
	int i=0;
	while(buf[i]) {
		console_putchar(buf[i]);
		i++;
	}
}

void console_writea(const char* buf,uint8_t a) {
	int i = 0;
	while(buf[i]) {
		console_putchara(buf[i],a);
		i++;
	}
}

void console_write_hex(uint32_t v) {
	console_write("0x");
	console_write_int(v,16);
}

void console_write_int(uint32_t v, unsigned base)
{
	int acc = v;
	char c[33], c2[33];
	int i = 0, j = 0;

	if (v == 0 || base > 16)
	{
		console_putchar('0');
		return;
	}

	while (acc > 0)
	{
		c[i] = "0123456789abcdef"[acc % base];
		acc /= base;
		++i;
	}
	c[i] = 0;
	c2[i--] = 0;
	while(i >= 0)
		c2[i--] = c[j++];

	console_write(c2);
}

void console_write_dec(uint32_t v) {
	console_write_int(v, 10);
}

void console_write_color(const char* buf,int8_t b,int8_t f) {
	if ( b < 0 ) {
		b = dbcolor;
	}
	if ( f < 0 ) {
		f = dfcolor;
	}
	console_writea(buf,( b << 4 ) | (f & 0x0F ));
}

void console_putchar_color(unsigned char c,int8_t b,int8_t f) {
	if ( b < 0 ) {
		b = dbcolor;
	}
	if ( f < 0 ) {
		f = dfcolor;
	}
	console_putchara(c, ( b << 4 ) | ( f & 0x0F ));
}

static void console_output(void *context, const char *data, size_t sz)
{
	while (sz --> 0)
		console_putchar(*data++);
}

static void (*output_console)(void *context, const char *data, size_t sz) =
console_output;
static void *console_context;

void register_console(void (*output)(void *context, const char *data, size_t sz),
		      void *context)
{
	output_console = output;
	console_context = context;
}

void vprintk(const char *fmt, va_list ap)
{
	vformat(output_console, console_context, fmt, ap);
}

void printk(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vprintk(fmt, ap);
	va_end(ap);
}
