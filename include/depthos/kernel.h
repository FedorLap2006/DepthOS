#pragma once

#include <depthos/stdtypes.h>

#define ROUND_DOWN(x, s) ((x) & ~((s)-1))
#define ROUND_UP(N, S) ((((N) + (S)-1) / (S)) * (S))

#define VIRT_BASE 0xC0000000
#define ADDR_TO_PHYS(addr) ((uint32_t)addr - VIRT_BASE)
#define ADDR_TO_VIRT(addr) ((uint32_t)addr + VIRT_BASE)

#define KERNEL_VIRT_BASE 0xC0100000
#define KERNEL_PHYS_BASE 0x00100000

typedef struct registers {
  uint32_t fs, gs, es, ds;
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
  uint32_t int_num, err_code;
  struct trace_stackframe *trace_frame;
  uintptr_t eip;
  uint32_t cs, eflags, useresp, ss;
} __packed, regs_t;

void panic(const char *file, int line, const char *loc, const char *format,
           ...);
#define panicf(fmt, ...) panic(__FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)