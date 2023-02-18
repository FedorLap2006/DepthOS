#include "depthos/ata.h"
#include "depthos/pic.h"
#include <depthos/console.h>
#include <depthos/heap.h>
#include <depthos/idt.h>
#include <depthos/kernel.h>
#include <depthos/multiboot.h>
#include <depthos/paging.h>
#include <depthos/pmm.h>
#include <depthos/serial.h>
#include <depthos/string.h>
//#include <depthos/stdio.h>
#include <depthos/assert.h>
#include <depthos/bitmap.h>
#include <depthos/dev.h>
#include <depthos/elf.h>
#include <depthos/fs.h>
#include <depthos/initrd.h>
#include <depthos/kconfig.h>
#include <depthos/keyboard.h>
#include <depthos/ksymbols.h>
#include <depthos/logging.h>
#include <depthos/proc.h>
#include <depthos/stdarg.h>
#include <depthos/stdbits.h>
#include <depthos/syscall.h>
#include <depthos/tests.h>
#include <depthos/tools.h>
#include <depthos/tss.h>

// #include <depthos/gdt.h>

extern unsigned short *videoMemory;
extern page_t kernel_pgt[1024] __align(4096);

uint32_t tick = 0;
bool ticker_sched_enable = false;
void ticker(regs_t *registers) {
  tick++;
  if (ticker_sched_enable) {
    sched_ticker(registers);
  }
  // klogf("%d", tick);
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
  bootlog("PIT initialization complete", LOG_STATUS_SUCCESS);
}

void sleep(size_t ms) {
  uint32_t max_tick = tick + ms;
  while (tick < max_tick)
    ;
}

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

void welcome_message() {
#ifndef CONFIG_WELCOME_FULLSCREEN
  console_putchar('\n');
  console_write_color("Welcome to DepthOS v", -1, WGREEN_COLOR);
  console_write_color(OSVER, -1, WGREEN_COLOR);
  console_putchar('\n');
#else
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
#endif
}

char *resolve_filetype(struct fs_node *file) {
  switch (file->type) {
  case FS_FILE:
    return "file";
  case FS_DIR:
    return "directory";
  case FS_MOUNT:
    return "mount";
  case FS_PIPE:
    return "pipe";
  }
  return "unknown";
}

void device_init() { devfs_register("txtfb", &console_device); }

__noreturn void kmain(int magic, struct multiboot_information *boot_ptr) {
  console_init(25, 80, 0, BGRAY_COLOR);
  multiboot_init_early(magic, boot_ptr);
  idt_init(); // TODO: why this was not required before?
  paging_init();
  init_timer(1000);
  // pmm_init(3 * 1024 * 4096 + 2 * 1024 * 4096);
  pmm_init(0);
  kheap_init();
  // pmm_dump_compact();
  multiboot_init(boot_ptr);
  vfs_init();
  ksymbols_load("/kernel.map");
  devfs_populate();
  device_init();
  ps2_keyboard_init();
  tty_init();
  // device_test();
  idt_register_interrupt(0x64, syscall_handler);
  idt_register_interrupt(0x80, posix_syscall_handler);
  tss_init();
  uint32_t esp;
  __asm__ volatile("movl %%esp, %0" : "=r"(esp));
  tss_set_stack(esp);
  sched_init();
  // pagedir_t pgd = create_pgd();
  // for (int i = 0; i < 10; i++) {
  //   klogf("%d", i);
  //   kmalloc(0x1000);
  // }

  // printk("here we go!\n");

  /*struct task *init_task = (struct task *)kmalloc(sizeof(struct task));
  elf_load(init_task, "/init.bin");
  bootstrap_user_task(init_task, true);
  sched_add(init_task);

  struct task *user_task = kmalloc(sizeof(struct task));
  printk("yolo: %d\n", elf_probe("/autoload2.bin"));
  elf_load(user_task, "/autoload2.bin");
  bootstrap_user_task(user_task, true);
  sched_add(user_task);
        */
  // process_spawn("/autoload2-t.bin", NULL);
  struct process *init_proc = process_spawn("/init.bin", NULL);
  struct fs_node *tty_file = vfs_open("/dev/tty0");
  list_item(init_proc->threads->first, struct task *)->filetable[0] = tty_file;
  list_item(init_proc->threads->first, struct task *)->filetable[1] = tty_file;

#ifdef CONFIG_TESTS_ENABLED
  tests_init();
  struct task *tests_task = create_kernel_task(tests_run, true);
  tests_task->name = "tests";
  tests_task->process = NULL;
  sched_add(tests_task);
#endif

  // extern struct task *current_task;
  reschedule();

  // create_task();
  // elf_load(current_task, "/autoload.bin");
  // elf_exec(current_task);

  // enter_usemode();

  for (;;)
    __asm __volatile("hlt");
  return;
}
