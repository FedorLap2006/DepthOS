#include <depthos/string.h>

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

size_t strlen(const char *s)
{
	const char *s1 = s;

	while (*s1)
		++s1;
	return s1 - s;
}
