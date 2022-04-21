#include <depthos/console.h>
#include <depthos/initrd.h>
#include <depthos/kernel.h>
#include <depthos/multiboot.h>
#include <depthos/serial.h>

void multiboot_init_early(int magic, struct multiboot_information *boot_ptr) {
  if (MULTIBOOT_HAS_FEATURE(boot_ptr, CMDLINE)) {
#define CMDLINE_FLAG(flag) if (strstr(boot_ptr->cmdline, flag))
    CMDLINE_FLAG("console=ttyS0") serial_console_init(0);
    CMDLINE_FLAG("console_no_color") { console_no_color = true; }
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