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


void init_intr();
void isr_handler(intr_regs_t);
void irq_handler(intr_regs_t);
void cintr_handler(intr_regs_t);

typedef struct intr_regs_t {
	uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t intr_it, err_code;
    uint32_t eip,cs,eflags,useresp,ss;
};

typedef void (*intrh_t)(intr_regs_t);

#define irq0 32
#define irq1 33
#define irq2 34
#define irq3 35
#define irq4 36
#define irq5 37
#define irq6 38
#define irq7 39
#define irq8 40
#define irq9 41
#define irq10 42
#define irq11 43
#define irq12 44
#define irq13 45
#define irq14 46
#define irq15 47

void reg_intr_handler(uint8_t,intrh_t);


