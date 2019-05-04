#include "depthos/string.h"

void *memchr(const void *p, int c, size_t n)
{
	const char *s = p;
	const char *e = s + n;

	while (s != e && *s != c)
		++s;
	if (s == e)
		return NULL;
	else
		return (void *)s;
}

int memcmp(const void *a, const void *b, size_t sz)
{
	const char *pa = a;
	const char *pb = b;

	while (sz && *pa == *pb) {
		++pa;
		++pb;
		--sz;
	}
	if (sz)
		return *pa - *pb;
	else
		return 0;
}

void *memcpy(void *dst, const void *src, size_t n)
{
	unsigned char *d = dst;
	const unsigned char *s = src;

	while (n --> 0)
		*d++ = *s++;
	return dst;
}

void *memset(void *p, int c, size_t n)
{
	unsigned char *p1 = p;

	while (n --> 0)
		*p1++ = c;
	return p;
}

char *strchr(const char *s, int c)
{
	while (*s && *s != c)
		++s;
	if (!*s)
		return NULL;
	else
		return (char *)s;
}

int strcmp(const char *a, const char *b)
{
	while (*a && *b && *a == *b) {
		++a;
		++b;
	}
	if (*a == *b)
		return 0;
	else
		return *a - *b;
}

size_t strlen(const char *s)
{
	const char *s1 = s;

	while (*s1)
		++s1;
	return s1 - s;
}

char *strstr(const char *haystack, const char *needle)
{
	size_t sz = strlen(needle);

	while (memcmp(haystack, needle, sz))
		if (!*haystack++)
			return NULL;
	return (char *)haystack;
}
