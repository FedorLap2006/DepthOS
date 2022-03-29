#include <depthos/console.h>
#include <depthos/idt.h>
#include <depthos/multiboot.h>
//#include <depthos/pgm.h>
#include <depthos/heap.h>
#include <depthos/paging.h>
#include <depthos/serial.h>
#include <depthos/string.h>
//#include <depthos/stdio.h>
#include <depthos/fs.h>
#include <depthos/initrd.h>
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

void multiboot_init_early(int magic, struct multiboot_information *boot_ptr) {
  if (MULTIBOOT_HAS_FEATURE(boot_ptr, CMDLINE) &&
      strstr(boot_ptr->cmdline, "console=ttyS0")) {
    serial_console_init(0);
  }
  if (MULTIBOOT_HAS_FEATURE(boot_ptr, MODULES) && boot_ptr->mods_count > 0) {
    extern uint32_t imalloc_ptr;
    imalloc_ptr = ADDR_TO_VIRT(boot_ptr->mods[boot_ptr->mods_count - 1].end);
  }
}

void multiboot_init(struct multiboot_information *boot_ptr) {
  if (MULTIBOOT_HAS_FEATURE(boot_ptr, MODULES) && boot_ptr->mods_count >= 1) {
    initrd_init(&boot_ptr->mods[0]);
  }
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

void kmain(int magic, struct multiboot_information *boot_ptr) {
  console_init(25, 80, 0, BGRAY_COLOR);
  multiboot_init_early(magic, boot_ptr);

  print_status("GDT initialized", MOD_OK);
  idt_init();
  init_timer(1000);
  paging_init();
  pgm_init(1024 * 4096);
  kheap_init();
  multiboot_init(boot_ptr);
  vfs_init();

  fs_node_t *cfg = vfs_open("/hello.txt");
  if (cfg) {
    char *buffer = kmalloc(1024);
    int b = vfs_read(cfg, buffer, 1024);
    buffer[b] = 0;
    klogf("%s [%s]: %s", cfg->path, resolve_filetype(cfg), buffer);
  } else
    klogf("not found");
  idt_register_interrupt(0x80, syscall_event);
  keyboard_driver_init();

  welcome_message();
  shell_eventloop();

  for (;;)
    __asm __volatile("hlt");
  return;
}
