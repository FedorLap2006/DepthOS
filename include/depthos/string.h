#pragma once

#include <depthos/stddef.h>

void *memchr(const void *p, int c, size_t n);
int memcmp(const void *a, const void *b, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
void *memset(void *p, int c, size_t n);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
int strcmp(const char *a, const char *b);
size_t strlen(const char *s);
char *strcpy(char *dst, const char *src);
char *strstr(const char *haystack, const char *needle);
int strsplt(const char *src, char *dest, size_t buf_str_size, size_t n,
            char delim);
char *strdup(const char *s);
