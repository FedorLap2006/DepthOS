#pragma once

#include <depthos/stdtypes.h>
#include <depthos/ports.h>


#define BLACK_COLOR 0
#define BLUE_COLOR 1
#define GREEN_COLOR 2
#define WWBLUE_COLOR 3
#define RED_COLOR 4
#define PURPLE_COLOR 5
#define BROWN_COLOR 6
#define WGRAY_COLOR 7
#define BGRAY_COLOR 8
#define WBLUE_COLOR 9
#define WGREEN_COLOR 10
#define WWWBLUE_COLOR 11
#define PINK_COLOR 12
#define WPURPLE_COLOR 13
#define WBROWN_COLOR 14
#define WHITE_COLOR 15

void console_init(int s,int l,int b,int f);
void console_movec(int x,int y);
void console_flushc();

void console_clear();
void console_flushs();

void console_putchara(char c,uint8_t a);
void console_putchar(char c);


void console_write(char* buf);
void console_writea(char* buf,uint8_t a);
void console_read();

void console_write_color(char* buf,int8_t b,int8_t f);
void console_putchar_color(char c,int8_t b,int8_t f);
