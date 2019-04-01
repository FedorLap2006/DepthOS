#include "heap.h"

uint32_t placeAddr;

uint32_t kalloc_ap(uint32_t size,int align,uint32_t *phys) {
	
	if(align && (placeAddr & 0xFFFFF000) ) {
		placeAddr &= 0xFFFFF000;
		placeAddr += 0x1000;
	}
	if (phys) {
		*phys = placeAddr;
	}
	uint32_t tmp = placeAddr;
	placeAddr += size;
	return tmp;
}
uint32_t kalloc(uint32_t size) {
	uint32_t tmp = placeAddr;
	placeAddr += size;
	return tmp;
}
uint32_t kalloc_a(uint32_t size,int align) {
	if (align && (placeAddr & 0xFFFFF000)) {
		placeAddr &= 0xFFFFF000;
		placeAddr += 0x1000;
	}
	uint32_t tmp = placeAddr;
}

uint32_t kalloc_p(uint32_t size,uint32_t *phys) {
	if ( phys ) {
		*phys = placeAddr;
	}
	uint32_t tmp = placeAddr;
	placeAddr += size;
	return tmp;
}


