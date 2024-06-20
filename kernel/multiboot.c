#include <depthos/pmm.h>
#include <depthos/vmm.h>
#include <depthos/assert.h>
#include <depthos/console.h>
#include <depthos/dev.h>
#include <depthos/errno.h>
#include <depthos/framebuffer.h>
#include <depthos/heap.h>
#include <depthos/initrd.h>
#include <depthos/kernel.h>
#include <depthos/logging.h>
#include <depthos/math.h>
#include <depthos/multiboot.h>
#include <depthos/paging.h>
#include <depthos/serial.h>
#include <depthos/string.h>

#define mmap_print(ptr)                                                        \
  klogf("mmap (%p): sz=%x addr=%llx len=%llx type=%x\n", ptr, ptr->size,       \
        ptr->addr, ptr->length, ptr->type);

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
  klogf("amount of available memory: %luMB\n", mem_length / 1024 / 1024);
  extern size_t __total_memory;
  __total_memory = mem_length;
}

struct multiboot_fb_dev_impl {
  struct framebuffer_stinfo static_info;
  struct framebuffer_varinfo variable_info;
  uintptr_t phys_addr;
  struct multiboot_information *mb;
};

long multiboot_fb_ioctl(struct device *dev, unsigned long request, void *data) {
  struct multiboot_fb_dev_impl *impl = dev->impl;
  switch (request) {
  case FRAMEBUFFER_IOCTL_NVARINFO:
    *(struct framebuffer_varinfo *)data = impl->variable_info;
    break;
  case FRAMEBUFFER_IOCTL_NSTINFO:
    *(struct framebuffer_stinfo *)data = impl->static_info;
    break;
  case FRAMEBUFFER_IOCTL_NSETVARINFO: {
    struct framebuffer_varinfo *varinfo = (struct framebuffer_varinfo *)data;
    if (varinfo->xres > impl->static_info.width ||
        varinfo->yres > impl->static_info.height)
      return EINVAL;
    impl->variable_info.xres = varinfo->xres;
    impl->variable_info.yres = varinfo->yres;
    break;
  }
  default:
    return ENOIOCTL;
  }
}

// long multiboot_fb_mmap(struct device *dev, long *address) {}
int multiboot_fb_mmap(struct device *dev, struct vm_area *area) {
  struct multiboot_fb_dev_impl *impl = dev->impl;
  klogf("hi %p %p", impl->phys_addr, area->addr);
  area->phys_addr = impl->phys_addr;
  size_t limit = impl->static_info.pitch * impl->static_info.height;
  area->n_pages = ceil_div(limit, PAGE_SIZE);
  area->limit = limit;
  return 0;
}

struct device multiboot_fb_dev = {
    .name = "fb0",
    .type = DEV_CHAR,
    .class = DEV_C_VIDEO,
    .ioctl = multiboot_fb_ioctl,
    .mmap = multiboot_fb_mmap,
    .read = NULL,
    .write = NULL,
    .seek = NULL,
    .impl = NULL,
};

bool multiboot_graphics_enabled = false;

void multiboot_init_early(int magic, struct multiboot_information *boot_ptr) {
  if (MULTIBOOT_HAS_FEATURE(boot_ptr, CMDLINE)) {
#define CMDLINE_FLAG(flag) if (strstr(boot_ptr->cmdline, flag))
    CMDLINE_FLAG("console=ttyS0") serial_console_init(0);
    CMDLINE_FLAG("console_no_color") { console_no_color = true; }
    CMDLINE_FLAG("graphics") {
      multiboot_graphics_enabled = true;
    } // TODO: properly abstract away framebuffer
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
  // NOTE: all addresses provided by boot_ptr should be used with ADDR_TO_VIRT.
  klogf("boot: %p", boot_ptr);
  if (MULTIBOOT_HAS_FEATURE(boot_ptr, MODULES) && boot_ptr->mods_count >= 1) {
    initrd_init((void *)ADDR_TO_VIRT(&boot_ptr->mods[0]));
  }

  if (MULTIBOOT_HAS_FEATURE(boot_ptr, FB) &&
      boot_ptr->framebuffer_type ==
          MULTIBOOT_FRAMEBUFFER_RGB) { // TODO: palette
    struct multiboot_fb_dev_impl *impl =
        kmalloc(sizeof(struct multiboot_fb_dev_impl));
    *impl = (struct multiboot_fb_dev_impl){
        .mb = boot_ptr,
        .phys_addr = boot_ptr->framebuffer_addr,
        .static_info =
            (struct framebuffer_stinfo){
                .width = boot_ptr->framebuffer_width,
                .height = boot_ptr->framebuffer_height,
                .pitch = boot_ptr->framebuffer_pitch,
                .bpp = boot_ptr->framebuffer_bpp,
            },
        .variable_info =
            (struct framebuffer_varinfo){
                .xres = boot_ptr->framebuffer_width,
                .yres = boot_ptr->framebuffer_height,
                // TODO: RGB positions and bitmasks
            },
    };
    multiboot_fb_dev.impl = impl;
    register_device(&multiboot_fb_dev);
    for (uintptr_t fa = boot_ptr->framebuffer_addr; fa != boot_ptr->framebuffer_pitch * boot_ptr->framebuffer_height; fa += PAGE_SIZE) {
      pmm_set(fa, 1, false);
    } 

    klogf("framebuffer: type %d bpp: %d addr: %llx (%llx)",
          boot_ptr->framebuffer_type, boot_ptr->framebuffer_bpp,
          boot_ptr->framebuffer_addr, ADDR_TO_VIRT(boot_ptr->framebuffer_addr));
#if 0
    uint32_t *fb = (uint32_t *)ADDR_TO_VIRT(boot_ptr->framebuffer_addr);
    extern pde_t kernel_pgd[1024];
    map_addr_fixed( // FIXME: we can't use map_addr here
        kernel_pgd, (uintptr_t)fb, ADDR_TO_VIRT(boot_ptr->framebuffer_addr),
        ceil_div(boot_ptr->framebuffer_pitch * boot_ptr->framebuffer_height *
                     ceil_div(boot_ptr->framebuffer_bpp, 4),
                 PAGE_SIZE),
        false, false);

    for (size_t y = 0; y < boot_ptr->framebuffer_height; y++) {
      for (size_t x = 0; x < boot_ptr->framebuffer_width; x++) {
        if (y == x)
          fb[y * boot_ptr->framebuffer_width + x] = 0xFF00FF;
      }

      // for (int i = 0; i < boot_ptr->framebuffer_width - 3; i += 3) {
      //   fb[y * boot_ptr->framebuffer_width + i] =
      //       (y % 3 == 0) ? 0xFF00FF : 0x0F0F0F;
      //   fb[y * boot_ptr->framebuffer_width + i + 1] =
      //       (y % 3 == 0) ? 0xFFFFFF : 0x000000;
      //   fb[y * boot_ptr->framebuffer_width + i + 2] = 0xFFFFFA;
      // }
    }

#endif
  }
}
