#include <depthos/console.h>

#define CONSOLE_WIDTH 80 // Ширина экрана в символах
#define CONSOLE_HEIGHT 25 // Высота экрана в символах
#define VIDEO_MEMORY_SIZE (CONSOLE_WIDTH * CONSOLE_HEIGHT) // Сколько символов может поместиться на экране
#define CONSOLE_MAX_OUTPUT_SIZE 500 // Для предотвращения бесконечной печати текста

unsigned short* videoMemory = (unsigned short*)0xB8000;
short consoleCursorX = 0, consoleCursorY = 0; // Позиция курсора на экране
short consoleColor = 0x0F00; // Цвет фона и символа при выводе текста на экран

// Прокрутка текста на одну строку
void Console_MoveUp(){
	short i;
	short a = 0;
	short b = CONSOLE_WIDTH;
	i = CONSOLE_WIDTH * (CONSOLE_HEIGHT - 1);
	i++;
	// Сначала текст перемещается на одну строку вверх, удаляя первую строку
	while(i){
		i--;
		videoMemory[a] = videoMemory[b];
		a++;
		b++;
	}
	i = CONSOLE_WIDTH;
	// А затем последняя строка заполняется пробелами
	while(i){
		i--;
		videoMemory[a] = consoleColor | ' ';
		a++;
	}
	// Примечание: "for(; i; i--)", "while(i)i--;", "while(i--)".
}

// Печатает текст "s" на экране
void Console_PrintText(const char *s){
    short i = consoleCursorY * CONSOLE_WIDTH + consoleCursorX; // Текущая позиция курсора
    short lastLine = CONSOLE_WIDTH * CONSOLE_HEIGHT; // Позиция, определяющая необходимость прокрутки текста
    short errStop = CONSOLE_MAX_OUTPUT_SIZE; // На случай, если будет подана строка без нуль-терминатора

    while(errStop){
		errStop--;
        if(*s == '\0'){
		// Напечатав всю строку нужно запомнить новую позицию курсора
			Console_SetCursorPos(i % CONSOLE_WIDTH, i / CONSOLE_WIDTH);
            return;
        }else if(*s == '\n'){
		// Переход на новую строку
            i = i / CONSOLE_WIDTH + 1;
            i = i * CONSOLE_WIDTH;
            s++;
        }else{
		// Печать текста на экран
            videoMemory[i] = consoleColor | *s;
            i++;
            s++;
        }
		// Если следующий символ не поместится на экране, то прокрутить текст на экране
        if(i > lastLine){
            Console_MoveUp();
            i = i - CONSOLE_WIDTH;
        }
    }
}

// Печатает текст на экране, временно изменив цвет
void Console_PrintColoredText(const char *s, char backgroundColor, char textColor){
	short bufferColor = consoleColor;
	if(backgroundColor == COLOR_WITH_CURRENT)
		backgroundColor = consoleColor >> 12;
	if(textColor == COLOR_WITH_CURRENT)
		textColor = consoleColor >> 8;
	Console_SetColor(backgroundColor, textColor);
	Console_PrintText(s);
	consoleColor = bufferColor;
}

// Печатает целое знаковое число на экране
void Console_PrintInteger(int n){
	if(n < 0){
		Console_PrintText("-");
		n = -n;	
	}
	char buffer[11];
	short i = 10;
	buffer[i] = 0;
	do{
		i--;
		buffer[i] = n % 10 + 48;
		n = n / 10;
	}while(n && i);
	Console_PrintText(&buffer[i]);
}

// Устанавливает цвет печатаемых символов и цвет их фона
void Console_SetColor(char backgroundColor, char textColor){
	backgroundColor &= 0x0F;
	consoleColor = backgroundColor;
	consoleColor <<= 4;
	textColor &= 0x0F;
	consoleColor |= textColor;
	consoleColor <<= 8;
}

// Очищает консоль
void Console_Clear(){
	short i = CONSOLE_HEIGHT;
	while(i){
		i--;
		Console_MoveUp();
	}
	Console_SetCursorPos(0, 0);
}

// Устанавливает новую позицию курсора
void Console_SetCursorPos(short X, short Y){
	consoleCursorX = X;
	consoleCursorY = Y;
	uint16_t pos = (consoleCursorY * CONSOLE_WIDTH) + consoleCursorX;
	outb(0x3D4, 14);
	outb(0x3D5, pos >> 8);
	outb(0x3D4, 15);
	outb(0x3D5, pos);
}






/*
void console_flushc() {
	//uint16_t pos = ( cursory * strs_len ) + cursorx;
	uint16_t pos = ( consoleCursorY * CONSOLE_WIDTH ) + consoleCursorX;
	outb(0x3D4, 14);
	outb(0x3D5, pos >> 8);
	outb(0x3D4, 15);
	outb(0x3D5, pos);
}*/

//void console_flushs() {
/*	uint8_t at = ( dbcolor << 4 ) | ( dfcolor & 0x0F );
	uint16_t b = 0x20 | ( at << 8 );

	if ( cursory >= strs_count ) {
		for ( int i = 0; i < ( strs_count-1 ) * strs_len; i++ ) {
			videoMemory[i] = videoMemory[i+strs_len];
		}
		for ( int i = ( strs_count - 1 ) * strs_len; i < strs_count * strs_len; i++ ) {
		   videoMemory[i] = b;
		}
 		cursory = strs_count - 1;		
	}*/
//}

void print_mod(char* buf,int m) {
	switch(m) {
		case MOD_OK: {
			Console_PrintText("[");
			Console_PrintColoredText("OK", -1, WGREEN_COLOR);
//			console_putchar_color(0xFB,-1,WGREEN_COLOR);
			Console_PrintText("]");
			Console_PrintText(" ");
			Console_PrintColoredText(buf, -1, WBLUE_COLOR);
			Console_PrintText("\n");
		   break;
		}
		case MOD_ERR: {
			Console_PrintText("[");
			Console_PrintColoredText("ERROR", -1, PINK_COLOR);
//			console_putchar_color(0xFB,-1,WGREEN_COLOR);
			Console_PrintText("]");
			Console_PrintText(" ");
			Console_PrintColoredText(buf, -1, WBLUE_COLOR);
			Console_PrintText("\n");
		  break;
		}
		default:
			break;
	}
}

/*
int strs_count = 25;
int strs_len = 80;

int dbcolor = 0;
int dfcolor = WHITE_COLOR;

int cursorx = 0, cursory = 0;


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
*/

