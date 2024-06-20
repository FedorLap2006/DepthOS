#pragma once

#include <depthos/console.h>
#include <depthos/kernel.h>
#include <depthos/ports.h>
#include <depthos/stdtypes.h>
#include <depthos/tools.h>
#include <depthos/trace.h>

typedef void (*intr_handler_t)(regs_t *);

#define IRQ(o) (0x20 + (o))

void idt_init();
extern void idt_flush();

void idt_register_interrupt(uint32_t i, intr_handler_t f);
void idt_disable_hwinterrupts();
void idt_enable_hwinterrupts();
void idt_dump();
