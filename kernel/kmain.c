#include <depthos/ata.h>
#include <depthos/beeper.h>
#include <depthos/console.h>
#include <depthos/heap.h>
#include <depthos/idt.h>
#include <depthos/kernel.h>
#include <depthos/math.h>
#include <depthos/multiboot.h>
#include <depthos/paging.h>
#include <depthos/partition.h>
#include <depthos/pic.h>
#include <depthos/pmm.h>
#include <depthos/sb16.h>
#include <depthos/serial.h>
#include <depthos/string.h>
// #include <depthos/stdio.h>
#include <depthos/assert.h>
#include <depthos/ata.h>
#include <depthos/bitmap.h>
#include <depthos/dev.h>
#include <depthos/elf.h>
#include <depthos/ext2.h>
#include <depthos/fs.h>
#include <depthos/initrd.h>
#include <depthos/kconfig.h>
#include <depthos/keyboard.h>
#include <depthos/ksymbols.h>
#include <depthos/logging.h>
#include <depthos/mbr.h>
#include <depthos/pit.h>
#include <depthos/proc.h>
#include <depthos/stdarg.h>
#include <depthos/stdbits.h>
#include <depthos/syscall.h>
#include <depthos/tests.h>
#include <depthos/tools.h>
#include <depthos/tss.h>
#include <depthos/tty.h>
#include <depthos/pci.h>

// #include <depthos/gdt.h>

// extern heap_t *kern_heap;
extern uint32_t _kernel_end;

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
  extern uint32_t pit_ticks;
  uint32_t max_tick = pit_ticks + duration;
  uint32_t current = 0;
  while (pit_ticks < max_tick) {
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
  // case FS_MOUNT:
  //   return "mount";
  case FS_PIPE:
    return "pipe";
  }
  return "unknown";
}

// XXX: should we use Haiku-like on-demand device initialization?
void device_init() {
  devfs_populate();
  pci_init();
  klogf("registerdevice");
  register_device(&console_device);
  klogf("after");
  ps2_keyboard_init();
  klogf("after 2");
  ata_init();
  klogf("after 3");
  tty_init();
  klogf("after 4");
  beeper_init();
  sb16_init();

  pci_enum();
}

void mbr_test() {
  // struct device *drive = vfs_open("/dev/ata0")->impl;
  struct device *drive = get_device("ata0");

  struct mbr *mbr = mbr_parse(drive);

  klogf("mbr: bootsec=%x partition type=%x lba=%x sec=%d attr=%x",
        mbr->valid_bootsector_signature, mbr->partitions[0].type,
        mbr->partitions[0].lba, mbr->partitions[0].sectors,
        mbr->partitions[0].attributes);
  klogf("mbr: bootsec=%x partition type=%x lba=%x sec=%d attr=%x",
        mbr->valid_bootsector_signature, mbr->partitions[1].type,
        mbr->partitions[1].lba, mbr->partitions[1].sectors,
        mbr->partitions[1].attributes);

  struct generic_partition *part =
      create_mbr_partition(drive, 0, mbr->partitions[0]);
  struct device *pdev = create_partition_device("ata0-0", part);
  // FIXME: why the data is empty
  // pdev->pos = 20;
  char *buf = kmalloc(512);
  memset(buf, 0, 512);
  pdev->pos = 64 - part->lba;
  klogf("reading from partition 0: %d", pdev->read(pdev, buf, 1, &pdev->pos));
  // for (int i = 0; i < 512; i++)
  //   klogf("reading [%d]: %x", i, buf[i]);
}

void kmain(int magic, struct multiboot_information *boot_ptr) {
  boot_ptr = (struct multiboot_information*)ADDR_TO_VIRT(boot_ptr);
  console_init(25, 80, 0, BGRAY_COLOR);
  multiboot_init_early(magic, boot_ptr);
  idt_init(); // TODO: why this was not required before. What has changed? --
              // version differ 4.2 here and 6.2 on old pc
  paging_init();
  klogf("paging");
  pit_init(1000);
  idt_enable_hwinterrupts();
  klogf("pit");
  // pmm_init(3 * 1024 * 4096 + 2 * 1024 * 4096);
  pmm_init(0);
  klogf("pmm");
  kheap_init();
  klogf("kheap");
  multiboot_init(boot_ptr);
  klogf("multiboot");
  vfs_init();
  klogf("vfs initialised");
  // TODO: optional ata disk
  bootlog("Mounting initial ramdisk",
          vfs_mount("/initrd", vfs_get_filesystem("initrd"), NULL)
              ? LOG_STATUS_SUCCESS
              : LOG_STATUS_ERROR);
  klogf("memory(%x..%x) %lx", boot_ptr->mem_lower * 1024,
        boot_ptr->mem_upper * 1024, boot_ptr->mem_upper - boot_ptr->mem_lower);

  ksymbols_load_file("/initrd/kernel.map");
  // *(char *)0xffffffff = 1;
  device_init();
  idt_register_interrupt(0x64, syscall_handler);
  idt_register_interrupt(0x80, posix_syscall_handler);

  klogf("mbr");
  mbr_test();
  klogf("ext2");
  ext2_init();

  struct device *fs_root_dev = get_device("ata1");
  klogf("ata1: %p", fs_root_dev);
  bootlog("Mounting root filesystem",
          fs_root_dev && vfs_mount("/", vfs_get_filesystem("ext2"),
                                   fs_root_dev)
              ? LOG_STATUS_SUCCESS // TODO: rootdev=ata1 grub parameter
              : LOG_STATUS_ERROR);
  if (!fs_root_dev)
    return;
  // ext2_test();
  tss_init();
  void *esp;
  __asm__ volatile("movl %%esp, %0" : "=r"(esp));
  tss_set_stack(esp);
  // pit_sched_enable = true;
  // for (;;)
  //   ;

  sched_init();
  // klogf("scheduling init process");
#ifdef CONFIG_TEST_MODE
  tests_init();
  struct task *tests_task = create_kernel_task(tests_run, true);
  tests_task->name = "tests";
  tests_task->process = NULL;
  sched_add(tests_task);
#else
  klogf("spawning init process");
  struct process *init_proc = process_spawn(
      "/sbin/init", NULL, (char const *[]){"/sbin/init", NULL},
      (char const *[]){NULL});
  if (!init_proc)
    panicf("Could not spawn init process. Halting.");

#endif

  klogf("scheduling init system");
  reschedule();
  __builtin_unreachable();
}
