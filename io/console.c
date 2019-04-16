#include <depthos/console.h>

unsigned short* videoMemory = (unsigned short*)0xb8000;

int strs_count = 25;
int strs_len = 80;

int dbcolor = 0;
int dfcolor = WHITE_COLOR;

int cursorx = 0, cursory = 0;

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


void console_putchara(char c,uint8_t a) {
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
	if ( cursorx >= strs_len ) {
		cursorx = 0;
		++cursory;
	}
	console_flushc();

}


void console_putchar(char c) {
	console_putchara(c, ( dbcolor << 4 ) | ( dfcolor & 0x0F));
}

void console_write(char* buf) {
	int i=0;
	while(buf[i]) {
		console_putchar(buf[i]);
		i++;
	}
}

void console_writea(char* buf,uint8_t a) {
	int i = 0;
	while(buf[i]) {
		console_putchara(buf[i],a);
		i++;
	}
}

void console_write_color(char* buf,int8_t b,int8_t f) {
	if ( b < 0 ) {
		b = dbcolor;
	}
	if ( f < 0 ) {
		f = dfcolor;
	}
	console_writea(buf,( b << 4 ) | (f & 0x0F ));
}

void console_putchar_color(char c,int8_t b,int8_t f) {
	if ( b < 0 ) {
		b = dbcolor;
	}
	if ( f < 0 ) {
		f = dfcolor;
	}
	console_putchara(c, ( b << 4 ) | ( f & 0x0F ));
}
