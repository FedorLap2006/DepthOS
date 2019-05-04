#pragma once

#include <depthos/stddef.h>

void *memchr(const void *p, int c, size_t n);
int memcmp(const void *a, const void *b, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
void *memset(void *p, int c, size_t n);
char *strchr(const char *s, int c);
int strcmp(const char *a, const char *b);
size_t strlen(const char *s);
char *strstr(const char *haystack, const char *needle);
