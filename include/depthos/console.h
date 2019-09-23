#pragma once

#include <depthos/stdarg.h>
#include <depthos/stdio.h>
#include <depthos/serial.h>
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

enum {
	MOD_OK,
	MOD_ERR
};



void console_init(int s,int l,int b,int f);
void console_movec(int x,int y);
void console_flushc();

void console_clear();
void console_flushs();

void console_putchara(unsigned char c,uint8_t a);
void console_putchar(unsigned char c);


void console_write(const char* buf);
void console_writea(const char* buf,uint8_t a);
void console_write_hex(uint32_t v);
void console_write_dec(uint32_t v);
void console_write_int(uint32_t v, unsigned base);
void console_read();

void console_write_color(const char* buf,int8_t b,int8_t f);
void console_putchar_color(unsigned char c,int8_t b,int8_t f);

void print_mod(char* buf,int m);
//char *mlog_s;                                               


void mod_loga(char* file, int line, char *mod,char *msg,...);
#define mod_log(mod,msg,...) mod_loga(__FILE__,__LINE__,mod,msg, ##__VA_ARGS__)
#define mod_loge(mod,msg,...) mod_loga(__FILE__,__LINE__,mod,msg)
#define panic(msg,...)     \
  mod_log(__func__,msg, ##__VA_ARGS__); \
  while(1) {}
#define panice(msg)     \
  mod_loga(__FILE__,__LINE__,__func__,msg);\
  while(1) {}

void register_console(void (*output)(void *context, const char *data, size_t sz),
		      void *context);

void printk(const char *fmt, ...);
void vprintk(const char *fmt, va_list ap);
