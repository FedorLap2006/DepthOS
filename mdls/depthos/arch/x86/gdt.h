#pragma once

#include <depthos/stdtypes.h>

typedef struct gdt_descriptor {
  uint16_t llimit;
  uint16_t lbase;
  uint8_t mbase;
  uint8_t acc;
  uint8_t gran;
  uint8_t hbase;
}__attribute__((packed));
typedef struct gdt_descriptor _gdt_descriptor_t;

void __gdt_init();
void __gdt_set();