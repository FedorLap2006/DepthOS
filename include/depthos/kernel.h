#pragma once

#define ROUND_DOWN(x, s) ((x) & ~((s)-1))
#define ROUND_UP(N, S) ((((N) + (S) - 1) / (S)) * (S))

#define VIRT_BASE 0x00000000//0xC0000000

#define KERNEL_VIRT_BASE 0x01000000
#define KERNEL_PHYS_BASE 0x00100000
