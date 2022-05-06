#pragma once

#include <depthos/stdtypes.h>

#define ROUND_DOWN(x, s) ((x) & ~((s)-1))
#define ROUND_UP(N, S) ((((N) + (S)-1) / (S)) * (S))

#define VIRT_BASE 0xC0000000
#define ADDR_TO_PHYS(addr) ((uint32_t)addr - VIRT_BASE)
#define ADDR_TO_VIRT(addr) ((uint32_t)addr + VIRT_BASE)

#define KERNEL_VIRT_BASE 0xC0100000
#define KERNEL_PHYS_BASE 0x00100000

void panic(const char *file, int line, const char *loc, const char *format,
           ...);
#define panicf(fmt, ...) panic(__FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)