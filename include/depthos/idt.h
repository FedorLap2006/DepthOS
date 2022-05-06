#pragma once

#include <depthos/console.h>
#include <depthos/ports.h>
#include <depthos/stdtypes.h>
#include <depthos/tools.h>
#include <depthos/trace.h>

typedef struct registers {
  uint32_t fs, gs, es, ds;
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
  uint32_t int_num, err_code;
  struct trace_stackframe *trace_frame;
  uintptr_t eip;
  uint32_t cs, eflags, useresp, ss;
} __packed, regs_t;

typedef void (*intr_handler_t)(regs_t *);

void idt_init();
extern void idt_flush();

void idt_register_interrupt(uint32_t i, intr_handler_t f);
void idt_disable_hwinterrupts();
void idt_enable_hwinterrupts();
void idt_dump();
