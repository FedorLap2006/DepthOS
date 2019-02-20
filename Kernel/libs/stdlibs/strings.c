#include "strings.h"


void memset(void *dest, uint8 val, uint32 len) {
	char* p = (char*)dest;
	for (size_t i = 0; i < len; ++i) {
		p[i] = val;
	}
	return b;
}