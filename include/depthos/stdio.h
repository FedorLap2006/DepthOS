#pragma once

#include <depthos/stdarg.h>
#include <depthos/stddef.h>

int sprintf(char *str, const char *fmt, ...);
int snprintf(char *str, size_t size, const char *fmt, ...);

int vsprintf(char *str, const char *fmt, va_list ap);
int vsnprintf(char *str, size_t size, const char *fmt, va_list ap);
