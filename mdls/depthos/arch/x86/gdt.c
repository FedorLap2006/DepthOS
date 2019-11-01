#include "gdt.h"

static _gdt_descriptor_t _knrl_gdt[20];
// void __gdt_init() {

// }

// void __gdt_set(uint32_t idx, _gdt_descriptor_t d) {
//   if(idx >= sizeof(_knrl_gdt) / sizeof(_gdt_descriptor_t)) return
//   else _knrl_gdt[idx] = d;
// }

// void __gdt_upd() {
//   uint64_t gdtr;
//   gdtr = (sizeof(gdt - 1)) | ((uint64_t) (uint32_t)gdt << 16);
//   asm volatile("lgdt %0" : :"m"(gdtr));
// }