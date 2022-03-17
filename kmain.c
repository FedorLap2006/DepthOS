
#include <depthos/console.h>
#include <depthos/idt.h>
//#include <depthos/pgm.h>
#include <depthos/heap.h>
#include <depthos/paging.h>
#include <depthos/serial.h>
#include <depthos/string.h>
//#include <depthos/stdio.h>
#include <depthos/keyboard.h>
#include <depthos/logging.h>
#include <depthos/stdarg.h>
#include <depthos/stdbits.h>
#include <depthos/tools.h>

// #include <depthos/gdt.h>

extern unsigned short *videoMemory;
extern page_t kernel_pgt[1024] __align(4096);

void syscall_event(regs_t r) {
  printk("syscall -- (%d)", r.eax);
  switch (r.eax) {
  case 0:
    console_write("syscall 0");
    break;
  case 1:
    console_write("syscall 1");
    break;
  case 2:
    break;
  default:
    break;
  }
  //	console_write_dec(r.eax);
}

uint32_t tick = 0;
void ticker(regs_t regs) {
  tick++;
  //	if ( (tick % 4) != 0 ) return;
  // console_write("tick:");
  // console_write_dec(tick);
  // console_write("\n");
  // if (tick % 1000)
  // printk("tiker %d\n", tick);
}

void init_timer(uint32_t tps) {
  idt_register_interrupt(0x20 + 0x0, ticker);
  uint32_t divisor = 1193180 / tps;
  outb(0x43, 0x36);
  uint8_t l = (uint8_t)(divisor & 0xFF);
  uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);

  outb(0x40, l);
  outb(0x40, h);
}

void sleep(size_t ms) {
  uint32_t max_tick = tick + ms;
  while (tick < max_tick)
    ;
}

struct multiboot_information {
  uint32_t flags;
  uint32_t mem_lower;
  uint32_t mem_upper;
  uint32_t boot_device;
  const char *cmdline;
};

// extern heap_t *kern_heap;
extern uint32_t kernel_end;

extern void shell_eventloop();

// TODO: \n and \b handling
void console_write_color_centered(const char *str, int bg, int fg) {
  int tmp = cursorx;
  cursorx = strs_len / 2 - strlen(str) / 2;
  console_flushc();
  console_write_color(str, bg, fg);
  cursorx = tmp;
}

void console_write_centered(const char *str) {
  console_write_color_centered(str, -1, -1);
}

bool fullscreen_welcome_message = false;

void welcome_message() {
  if (!fullscreen_welcome_message) {
    console_putchar('\n');
    console_write_color("Welcome to DepthOS v", -1, WGREEN_COLOR);
    console_write_color(OSVER, -1, WGREEN_COLOR);
    console_putchar('\n');
    return;
  }
  const int delay = 700;
  const int duration = 5000;

  console_clear();
  cursory = strs_count / 2 - 1;
  console_write_color_centered("DepthOS", -1, PINK_COLOR);
  console_write("\n\n");
  cursorx = strs_len / 2 - 4;
  uint32_t max_tick = tick + duration;
  uint32_t current = 0;
  while (tick < max_tick) {
    for (int i = 0; i < 5; i++) {
      console_putchar_color('.', -1,
                            i == current % 5 ? WHITE_COLOR : BGRAY_COLOR);
      if (i < 4)
        console_putchar(' ');
    }
    sleep(delay);
    cursorx -= sizeof(". . . . .") - 1;
    current++;
  }
  console_clear();
}

void kmain(int magic, struct multiboot_information *boot_ptr) {
  console_init(25, 80, 0, BGRAY_COLOR);
  if (strstr(boot_ptr->cmdline, "console=ttyS0")) {
    serial_console_init(0);
  }
  print_status("GDT initialized", MOD_OK);
  idt_init();
  paging_init();
  pgm_init(1024 * 4096);
  // pgm_dump();
  klogf("349=%d 348=%d", pgm_get(349), pgm_get(348));
  // page_t *page_alloc1 = pgm_alloc(1);
  // klogf("(pgm_alloc(1) at 0%x).frame = 0x%x", page_alloc1,
  //       parse_page(page_alloc1).frame);
  // page_t *page_alloc2 = pgm_alloc(1);
  // klogf("(pgm_alloc(1) at 0%x).frame = 0x%x", page_alloc2,
  //       parse_page(page_alloc2).frame);
  // pgm_dump();
  // klogf("should fail");
  // pgm_free(0x0c010900f, 1);
  // pgm_dump();
  // pgm_free(page_alloc1, 1);
  // pgm_dump();
  // page_t *page_alloc3 = pgm_alloc(2);
  // klogf("(pgm_alloc(2) at 0%x).frame = 0x%x", page_alloc3,
  //       parse_page(page_alloc3).frame);
  // pgm_dump();
  // pgm_free(page_alloc2, 1);
  // pgm_dump();
  // pgm_free(page_alloc3, 2);
  // pgm_dump();

  // pgm_init(10 * 4096); // | 0 0 0 0 0 0 0 0 |
  // pmm_init(1096 * (1024 * 1024));
  idt_register_interrupt(0x80, syscall_event);
  init_timer(1000);
  __kb_driver_init();

  welcome_message();
  shell_eventloop();

  /*printk("mem size: %d", boot_ptr->mem_upper - boot_ptr->mem_lower);*/

  for (;;)
    __asm __volatile("hlt");
  return;
}
