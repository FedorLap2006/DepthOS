#pragma once
#include <types.h>

void* memset(void *dest, uint8_t val, uint32_t len) {
	char* p = (char*)dest;
	for (size_t i = 0; i < len; ++i) {
		p[i] = val;
	}
	return (void*)p;
}

/////////// VIRTUAL MEMORY \\\\\\\\\\\\
\\\\\\\\\\      < / >      ////////////

#define ptr_t void*
typedef uint16_t sptr_t;
typedef uint32_t lptr_t;
typedef uint64_t llptr_t;




// lptr_t malloc();

