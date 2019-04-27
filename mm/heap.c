#include "depthos/heap.h"

extern uint32_t end;
uint32_t placeAddr = (uint32_t)&end;

uint32_t sbrk(uint32_t sz) {
	uint32_t tmp = placeAddr;
	placeAddr += sz;
	return tmp;
}

uint32_t sbrk_ap(uint32_t sz,int align,uint32_t *phys) {
	
	if(align && (placeAddr & 0xFFFFF000) ) {
		placeAddr &= 0xFFFFF000;
		placeAddr += 0x1000;
	}
	if (phys) {
		*phys = placeAddr;
	}
	uint32_t tmp = placeAddr;
	placeAddr += sz;
	return tmp;
}
uint32_t sbrk_a(uint32_t sz,int align) {
	if (align && (placeAddr & 0xFFFFF000)) {
		placeAddr &= 0xFFFFF000;
		placeAddr += 0x1000;
	}
	uint32_t tmp = placeAddr;
}

uint32_t sbrk_p(uint32_t sz,uint32_t *phys) {
	if ( phys ) {
		*phys = placeAddr;
	}
	uint32_t tmp = placeAddr;
	placeAddr += sz;
	return tmp;
}
