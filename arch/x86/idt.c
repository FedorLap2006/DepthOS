#include <depthos/idt.h>
#include <depthos/kernel.h>
#include <depthos/logging.h>
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

struct __idt_entry idt[IDT_SIZE];
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

void idt_register_llhandler(uint8_t i, uint8_t type, uint8_t dpl, uint32_t cb) {
  if (i >= IDT_SIZE)
    return;
  idt[i].addr_low = (cb & 0x0000ffff);

  idt[i].sel = 0x8;
  idt[i].zero = 0x00;
  idt[i].flags = (type & 0xF) | (dpl << 5) | 0x80;
  idt[i].addr_high = ((cb & 0xffff0000) >> 16);
}

void _idt_unregistered_interrupt_llhandler() {}

void idt_init() {
  __init_idt = 1;
  idt_ptr.size = (sizeof(struct __idt_entry) * IDT_SIZE) - 1;
  idt_ptr.addr = (uint32_t)&idt;

  pic_init(default_pic_config);

#include "idt_handlers.h"

  extern void intr128();
  idt_register_llhandler(0x80, 0xE, 3, (uint32_t)intr128);
  idt_register_llhandler(0x30, 0xE, 3, (uint32_t)intr48);
  for (int i = INTERRUPTS_COUNT; i < IDT_SIZE; i++) {
    idt_register_llhandler(i, 0xE, 0,
                           (uint32_t)_idt_unregistered_interrupt_llhandler);
  }
  void gpf_handler(regs_t *);
  idt_register_interrupt(13, gpf_handler);
  idt_flush();
  print_status("IDT initialized", MOD_OK);
}

void idt_hwinterrupt_handler(regs_t *r) {
  // klogf("interrupt received %d", r.int_num);
  idt_interrupt_handler(r);
  pic_eoi(r->int_num);
}

void idt_interrupt_handler(regs_t *r) {
  /*if (r.int_num == 0x20 || r.int_num == 0x21)*/
  /*klogf("interrupt received %d", r.int_num);*/
  // printk("interrupt: %d\n", r.int_num);
  if (intrs[r->int_num] != 0) {
    // klogf("(%x) registers at 0x%x", r.int_num, &r);
    intr_handler_t h = intrs[r->int_num];
    h(r);
  }
}

void gpf_handler(regs_t *r) {
  printk("GP fault: 0x%x\n", r->err_code);
  dump_registers(*r);
  trace(0, -1);
}

void idt_register_interrupt(uint32_t i, intr_handler_t f) {
  // trace(1, 5);
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

void idt_dump() {
  for (int i = 0; i < INTERRUPTS_COUNT; i++) {
    printk("[%d]: 0x%x flags=%x selector=%x\n", i,
           idt[i].addr_low | (idt[i].addr_high << 16), idt[i].flags,
           idt[i].sel);
  }
  printk("\n");
}

void idt_disable_hwinterrupts() { __asm__ volatile("cli"); }
void idt_enable_hwinterrupts() { __asm__ volatile("sti"); }
