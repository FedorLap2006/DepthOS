#pragma once

#include <depthos/stdtypes.h>

void pop(void *dst) {
	__asm ( "pop %0" : "=r"(*dst) : );
}
void push(void *data) {
   __asm ( "push %0" : : "r" (*data));
}
