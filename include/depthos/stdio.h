#pragma once

#include <depthos/stdarg.h>
#include <depthos/stddef.h>

__printf_fmt(2, 3) int sprintf(char *str, const char *fmt, ...);
__printf_fmt(3, 4) int snprintf(char *str, size_t size, const char *fmt, ...);

int vsprintf(char *str, const char *fmt, va_list ap);
int vsnprintf(char *str, size_t size, const char *fmt, va_list ap);

void putk(char c);
void vprintk(const char *fmt, va_list ap);
__printf_fmt(1, 2) void printk(const char *fmt, ...);
