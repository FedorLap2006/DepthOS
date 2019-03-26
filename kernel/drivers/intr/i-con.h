#pragma once

#include <std/types.h>
#include <std/ports.h>
#include <std/memory.h>

// GDT

typedef struct gdte_t {
	uint16_t offlow;
	uint16_t blow;
	uint8_t bmid;
	uint8_t acc;
	uint8_t gran;
	uint8_t bhigh;
}apack;

typedef struct gtdp_t {
	uint16_t limit;
	uint32_t addr;
}apack;



// IDT

typedef struct idte_t {
	uint16_t blow;
	uint16_t sel;
	uint8_t zero;
	uint8_t flags;
	uint16_t bhigh;
}apack;

typedef struct idtp_t {
	uint16_t limit;
	uint32_t addr;
}apack;

// SPECIAL FUNCTIONS AND VARIABLES

gdte_t gdte[5];
gdtp_t gdt_ptr;
idte_t idte[256];
idtp_t idt_ptr;

void init_intr();
void intr_handler();
