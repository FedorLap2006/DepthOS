#include "depthos/stdbits.h"

#include <depthos/console.h>

void setbit(uint32_t *number,uint32_t off,int bit) {
	bit &= (1 << 0);
	*number ^= (-bit ^ *number) & (1UL << off);
}

int getbit(uint32_t number,uint32_t off) {
	return (number >> off) & 1U;
}

