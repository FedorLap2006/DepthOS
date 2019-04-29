#pragma once

#include <depthos/stdarg.h>

void vformat(void (*output)(void *context, const char *data, size_t sz),
	     void *context, const char *fmt, va_list ap);
