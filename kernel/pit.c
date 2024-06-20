#include <depthos/logging.h>
#include <depthos/syscall.h>
#include <depthos/pit.h>

uint32_t pit_ticks = 0;
uint32_t pit_tps = 0;
bool pit_sched_enable = false;

uint16_t pit_get_counter() {
  outb(0x43, 0xC0);

  uint16_t count = inb(0x40) | (inb(0x40) << 8);
  return count;
}

uint16_t pit_prev_counter = 0;

void pit_ticker(regs_t *registers) {
  idt_disable_hwinterrupts();
  // klogf("pit!!! %ld", pit_ticks);
  pit_ticks++;
  // if (pit_ticks % pit_tps == 0) {
  //   klogf("counter: %u %u", pit_get_counter(), pit_prev_counter);
  //   pit_prev_counter = pit_get_counter();
  // }
  if (pit_sched_enable) {
    extern void sched_ticker(regs_t * registers);
    sched_ticker(registers);
  }
}

void pit_init(uint32_t tps) {
  pit_tps = tps;
  idt_register_interrupt(0x20 + 0x0, pit_ticker);
  uint32_t divisor = 1193180 / tps;
  outb(0x43, 0x36);
  uint8_t l = (uint8_t)(divisor & 0xFF);
  uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);

  outb(0x40, l);
  outb(0x40, h);
  bootlog("PIT initialization complete", LOG_STATUS_SUCCESS);
}

void sleep(size_t ms) {
  uint32_t max_tick = pit_ticks + ms;
  while (pit_ticks < max_tick)
    ;
}

struct tms {
  long user;
  long sys;
  long unused[2];
};

DECL_SYSCALL1(times, struct tms*, buf) {
  // klogf("times!");
  *buf = (struct tms) {
    .sys = 0,
    .user = (1000000 / pit_tps) * current_task->running_time,
  };
}
