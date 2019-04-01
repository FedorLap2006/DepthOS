#pragma once

void memset(void *dest, uint8 val, uint32 len) {
	char* p = (char*)dest;
	for (size_t i = 0; i < len; ++i) {
		p[i] = val;
	}
	return b;
}

/////////// VIRTUAL MEMORY \\\\\\\\\\\\
\\\\\\\\\\      < / >      ////////////

typedef void* ptr_t;
typedef uint16_t* sptr_t;
typedef uint32_t* lptr_t;
typedef uint64_t* llptr_t;




lptr_t malloc();

