#include <depthos/console.h>
#include <depthos/initrd.h>
#include <depthos/kernel.h>
#include <depthos/logging.h>
#include <depthos/multiboot.h>
#include <depthos/serial.h>

#define mmap_print(ptr)                                                        \
  printk("mmap entry (%x): sz=%x addr=%llx len=%llx type=%x\n", ptr,           \
         ptr->size, ptr->addr, ptr->length, ptr->type);

#define mmap_foreach(VAR)                                                      \
  for (struct multiboot_mmap_entry *ptr = start;                               \
       (uintptr_t)ptr < (uintptr_t)start + length;                             \
       ptr = (struct multiboot_mmap_entry *)((uintptr_t)ptr + ptr->size +      \
                                             sizeof(ptr->size)))

void multiboot_load_mmap(struct multiboot_mmap_entry *start, size_t length) {
  uint64_t mem_length = 0;
  mmap_foreach(ptr) {
    mmap_print(ptr);
    if (ptr->type == MULTIBOOT_MMAP_AREA_AVAILABLE)
      mem_length += ptr->length;
  }
  printk("memory amount: %luMB\n", mem_length / 1024 / 1024);
  extern size_t __total_memory;
  __total_memory = mem_length;
}

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

  if (MULTIBOOT_HAS_FEATURE(boot_ptr, MMAP) && boot_ptr->mmap_length > 0) {
    multiboot_load_mmap(boot_ptr->mmap, boot_ptr->mmap_length);
  }
}

void multiboot_init(struct multiboot_information *boot_ptr) {
  if (MULTIBOOT_HAS_FEATURE(boot_ptr, MODULES) && boot_ptr->mods_count >= 1) {
    initrd_init(&boot_ptr->mods[0]);
  }
}
