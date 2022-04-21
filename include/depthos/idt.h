#pragma once

#include <depthos/ports.h>
#include <depthos/stdtypes.h>
#include <depthos/tools.h>

#include <depthos/console.h>

typedef struct __regs {
  uint32_t fs, gs, es, ds;
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
  uint32_t int_num, err_code;
  uint32_t eip, cs, eflags, useresp, ss;
} regs_t;

typedef void (*intr_handler_t)(regs_t *);

void idt_init();
extern void idt_flush();

void idt_register_interrupt(uint32_t i, intr_handler_t f);
void idt_disable_hwinterrupts();
void idt_enable_hwinterrupts();
void idt_dump();
