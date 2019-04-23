#pragma once

#include <depthos/stdtypes.h>
#include <depthos/ports.h>
#include <depthos/console.h>

#define gdt_size 5

enum {
	SEL_GDT,
	SEL_LDT
};


enum {
	RPL_KERNEL,
	RPL_DRIVERS,
	RPL_API,
	RPL_UPROG
};
uint16_t makeSelector(uint16_t index, uint8_t tb, uint8_t rpl);

enum {
	GRAN_BYTE,
	GRAN_PAGE
};


typedef struct __gdt_entry{
	uint16_t llow;
	uint16_t blow;
	uint8_t bmid;
	uint8_t acc;
	uint8_t gran;
	uint8_t bhigh;
}apack;


typedef struct __gdt_ptr {
	uint16_t size;
	uint32_t addr;
}apack;

void gdt_init();

uint16_t gdt_set_entry(int num,uint32_t addr,uint32_t size,uint8_t gran,uint8_t rpl);

