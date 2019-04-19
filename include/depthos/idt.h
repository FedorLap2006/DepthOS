#pragma once

#include <depthos/stdtypes.h>
#include <depthos/ports.h>

#include <depthos/console.h>

typedef struct __regs {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t int_num, err_code;
    uint32_t eip,cs,eflags,useresp,ss;
} regs_t;

typedef struct __idt_entry{
	uint16_t addr_low;
	uint16_t sel;
	uint8_t zero;
	uint8_t flags;
	uint16_t addr_high;
}apack;

typedef struct __idt_ptr {
	uint16_t size;
	uint32_t addr;
}apack;

typedef void (*intr_ht)(regs_t);

void idt_init();
void reg_intr(uint32_t i,intr_ht f); // register interrupt handler

extern void idt_flush();



// handlers
void intr_handler(regs_t r);
void __idt_default_handler();
