#pragma once

#include <depthos/stdtypes.h>
#include <depthos/tools.h>

#define ROUND_DOWN(x, s) ((x) - ((x) % (s)))
#define ROUND_UP(N, S) ((((N) + (S)-1) / (S)) * (S))

#define VIRT_BASE 0xC0000000
#define ADDR_TO_PHYS(addr) ((uintptr_t)addr - VIRT_BASE)
#define ADDR_TO_VIRT(addr) ((uintptr_t)addr + VIRT_BASE)

#define KERNEL_VIRT_BASE 0xC0100000
#define KERNEL_PHYS_BASE 0x00100000

// #define MAX_DMA_ADDR 

#define INVALID_ADDR ((uintptr_t)(-1))


struct registers {
  uint32_t fs, gs, es, ds;
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
  uint32_t int_num, err_code;
  struct trace_stackframe *trace_frame;
  uintptr_t eip;
  uint32_t cs, eflags, useresp, ss;
} __pack;

typedef struct registers regs_t;

__noreturn void panic(const char *file, int line, const char *loc,
                      const char *format, ...);
#define panicf(fmt, ...) panic(__FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
