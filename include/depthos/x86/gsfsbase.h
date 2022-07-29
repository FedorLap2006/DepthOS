#pragma once

#include <depthos/logging.h>
#include <depthos/x86/gdt.h>

static inline void x86_set_gsbase(uintptr_t ptr) {
  uint16_t sel;
  __asm__ volatile("mov %%gs, %0" : "=r"(sel));
  // klogf("%d %d %d", sel, sel / 8, GDT_SEL_IDX(sel));
  x86_gdt_set_base(GDT_SEL_IDX(sel), ptr);
  __asm__ volatile("mov %0, %%gs" : : "r"(sel));
}
static inline void x86_set_fsbase(uintptr_t ptr) {
  uint16_t sel;
  __asm__ volatile("mov %%fs, %0" : "=r"(sel));
  // klogf("%d %d %d", sel, sel / 8, GDT_SEL_IDX(sel));
  x86_gdt_set_base(GDT_SEL_IDX(sel), ptr);
  __asm__ volatile("mov %0, %%fs" : : "r"(sel));
}