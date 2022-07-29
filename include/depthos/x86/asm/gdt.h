#pragma once

// Descriptor access byte definitions.

#define GDT_ACCESS_ACCESSED 1
#define GDT_ACCESS_TYPE_R 0x002
#define GDT_ACCESS_TYPE_W 0x002
#define GDT_ACCESS_DC 0x004
#define GDT_ACCESS_TYPE_X 0x008
#define GDT_ACCESS_S 0x010
#define GDT_ACCESS_DPL(v) ((v & 0x3) << 5)
#define GDT_ACCESS_PRESENT 0x080

// Descriptor flags.

#define GDT_FLAGS_LONG 0x01
#define GDT_FLAGS_32 0x02
#define GDT_FLAGS_DB GDT_FLAGS_32
#define GDT_FLAGS_G 0x04

// Full descriptor definitions.

#define GDT_DESC_G 0x00800000
#define GDT_DESC_D_B 0x00400000
#define GDT_DESC_P 0x00008000
#define GDT_DESC_S 0x00001000
#define GDT_DESC_A 0x00000100
#define GDT_DESC_TYPE_X 0x00000800
#define GDT_DESC_TYPE_R 0x00000200
#define GDT_DESC_TYPE_W 0x00000200

#define GDT_DESC_BASE_LOW_MASK 0xffff0000
#define GDT_DESC_BASE_LOW(v) (((v)&0x0000ffff) << 16)
#define GDT_DESC_LIMIT_LOW_MASK 0x0000ffff
#define GDT_DESC_LIMIT_LOW(v) ((v)&0x0000ffff)
#define GDT_DESC_DPL(v) ((v & 0x3) << 13)

#define GDT_DESC_LIMIT_HIGH_MASK 0x000f0000
#define GDT_DESC_LIMIT_HIGH(v) ((v)&GDT_DESC_LIMIT_HIGH_MASK)
#define GDT_DESC_BASE_HIGH_MASK 0xff0000ff
#define GDT_DESC_BASE_HIGH(v) (((v)&0xff000000) | (((v)&0x00ff0000) >> 16))
#define GDT_DESC_ACCESS(v) ((v & 0xFF) << 8)
#define GDT_DESC_FLAGS(v) ((v & 0xF) << 20)
#ifdef __x86_64__
#define GDT_DESC_BASE_EXTRA(v) (v >> 32)
#endif

#ifdef __i686__
#define KERNEL_CODE_BASE 0
#define KERNEL_CODE_LIMIT 0xfffff

#define KERNEL_DATA_BASE 0
#define KERNEL_DATA_LIMIT 0xfffff

#define USER_CODE_BASE 0
#define USER_CODE_LIMIT 0xfffff

#define USER_DATA_BASE 0
#define USER_DATA_LIMIT 0xfffff

#define USER_TCB_LIMIT 0xfffff
#endif

#define GDT_SEL(idx, tb, rpl)                                                  \
  ((rpl & 0x3) | ((tb & 0x1) << 2) | ((idx & 0x1ff) << 3))
#define GDT_KERNEL_CODE_SEL 0x8
#define GDT_KERNEL_DATA_SEL 0x10
#define GDT_USER_CODE_SEL 0x1b
#define GDT_USER_DATA_SEL 0x23
#define GDT_TSS_SEL 0x28
#define GDT_GSBASE_SEL(rpl) GDT_SEL(6, 0, rpl)
#define GDT_FSBASE_SEL(rpl) GDT_SEL(7, 0, rpl)
#define GDT_SEL_IDX(sel) ((sel >> 3) & 0x1ff)

#if 0
#include <depthos/console.h>
#include <depthos/ports.h>
#include <depthos/stdtypes.h>

#define gdt_size 5

enum { SEL_GDT, SEL_LDT };

enum { RPL_KERNEL, RPL_DRIVERS, RPL_API, RPL_UPROG };
uint16_t makeSelector(uint16_t index, uint8_t tb, uint8_t rpl);

enum { GRAN_BYTE, GRAN_PAGE };

typedef struct __gdt_entry {
  uint16_t llow;
  uint16_t blow;
  uint8_t bmid;
  uint8_t acc;
  uint8_t gran;
  uint8_t bhigh;
} apack;

typedef struct __gdt_ptr {
  uint16_t size;
  uint32_t addr;
} apack;

void gdt_init();

uint16_t gdt_set_entry(int num, uint32_t addr, uint32_t size, uint8_t gran,
                       uint8_t rpl);
#endif
