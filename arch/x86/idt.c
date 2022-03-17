#include <depthos/idt.h>
#include <depthos/kernel.h>
#include <depthos/pic.h>

int __init_idt = 0;

#define IDT_SIZE 256
#define INTERRUPTS_COUNT 129

struct __idt_entry {
  uint16_t addr_low;
  uint16_t sel;
  uint8_t zero;
  uint8_t flags;
  uint16_t addr_high;
} __pack;

struct __idt_ptr {
  uint16_t size;
  uint32_t addr;
} __pack;

static struct __idt_entry idt[IDT_SIZE];
struct __idt_ptr idt_ptr;

intr_handler_t intrs[INTERRUPTS_COUNT];

#define IRQC0 (1 << 0)
#define IRQC1 (1 << 1)
#define IRQC2 (1 << 2)
#define IRQC3 (1 << 3)
#define IRQC4 (1 << 4)
#define IRQC5 (1 << 5)
#define IRQC6 (1 << 6)
#define IRQC7 (1 << 7)
#define IRQC8 (1 << 8)
#define IRQC9 (1 << 9)
#define IRQC10 (1 << 10)
#define IRQC11 (1 << 11)
#define IRQC12 (1 << 12)
#define IRQC13 (1 << 13)
#define IRQC14 (1 << 14)
#define IRQC15 (1 << 15)
#define IRQC16 (1 << 16)
#define IRQCALL 0xFFFF
#define IRQCDALL 0x0000

struct pic_config default_pic_config = {
    .dirty_mask = false,
    .iowait_mode = true,
    .sfn_mode = false,
    .bufmode = false,
    .ps_bufmode = false,
    .auto_eoi = false,
    .mp_mode = true,
    .primary_offset = 0x20,
    .secondary_offset = 0x28,
    .sec_irq_line_num = 0x2,
    .sconn_irq_line = 0x4,
    .mask = (uint16_t)IRQCALL, // IRQ0 (PIT)
};

void idt_register_llhandler(uint8_t i, uint32_t cb) {
  if (i >= IDT_SIZE)
    return;
  idt[i].addr_low = (cb & 0x0000ffff);

  idt[i].sel = 0x8;
  idt[i].zero = 0x00;
  idt[i].flags = 0x8E;
  idt[i].addr_high = ((cb & 0xffff0000) >> 16);
}

void _idt_unregistered_interrupt_llhandler() {}

void idt_init() {
  __init_idt = 1;
  idt_ptr.size = (sizeof(struct __idt_entry) * IDT_SIZE) - 1;
  idt_ptr.addr = (uint32_t)&idt - VIRT_BASE;

  pic_init(default_pic_config);

#include "idt_handlers.h"

  for (int i = INTERRUPTS_COUNT; i < IDT_SIZE; i++) {
    idt_register_llhandler(i, (uint32_t)_idt_unregistered_interrupt_llhandler);
  }
  idt_flush();
  print_status("IDT initialized", MOD_OK);
}

void idt_hwinterrupt_handler(regs_t r) {
  idt_interrupt_handler(r);
  pic_eoi(r.int_num);
}

void idt_interrupt_handler(regs_t r) {
  if (intrs[r.int_num] != 0) {
    intr_handler_t h = intrs[r.int_num];
    h(r);
  }
}

void idt_register_interrupt(uint32_t i, intr_handler_t f) {
  if (i >= INTERRUPTS_COUNT) {
    print_status("cannot register interrupt: interrupt vector is out of range",
                 MOD_ERR);
    return;
  }
  if (intrs[i] != 0) {
    print_status("warning: overwriting existing interrupt handler",
                 MOD_WARNING);
  }
  intrs[i] = f;
}

void idt_disable_hwinterrupts() { __asm__ volatile("cli"); }
void idt_enable_hwinterrupts() { __asm__ volatile("sti"); }
