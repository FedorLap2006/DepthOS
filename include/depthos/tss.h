#pragma once

#include <depthos/stddef.h>
#include <depthos/stdtypes.h>

struct tss {
  uint16_t backlink, : 16;

  void *esp0;
  uint16_t ss0, : 16;
  void *esp1;
  uint16_t ss1, : 16;
  void *esp2;
  uint16_t ss2, : 16;

  uint32_t cr3;
  void (*eip)();
  uint32_t eflags;
  uint32_t eax, ecx, edx, ebx;
  void *esp;
  uint32_t ebp, esi, edi;

  uint16_t es, : 16;
  uint16_t cs, : 16;
  uint16_t ss, : 16;
  uint16_t ds, : 16;
  uint16_t fs, : 16;
  uint16_t gs, : 16;
  uint16_t ldt, : 16;
  uint16_t debug_trap : 1, : 15, bitmap;
} __pack;

void tss_init();

/**
 * @brief Update TSS stack pointer
 *
 * @param stack New stack pointer
 */
void tss_set_stack(void *stack);