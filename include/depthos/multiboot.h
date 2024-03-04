#pragma once

#include <depthos/stdtypes.h>

#define MULTIBOOT_FLAG_MEMINFO 1 << 0
#define MULTIBOOT_FLAG_BOOTDEV 1 << 1
#define MULTIBOOT_FLAG_CMDLINE 1 << 2
#define MULTIBOOT_FLAG_MODULES 1 << 3
#define MULTIBOOT_FLAG_MMAP 1 << 6
#define MULTIBOOT_FLAG_FB 1 << 12
#define MULTIBOOT_HAS_FEATURE(info, feature)                                   \
  (info->flags & MULTIBOOT_FLAG_##feature)

struct multiboot_mmap_entry {
  uint32_t size;
  uint64_t addr;
  uint64_t length;
#define MULTIBOOT_MMAP_AREA_AVAILABLE 1
#define MULTIBOOT_MMAP_AREA_RESERVED 2
#define MULTIBOOT_MMAP_AREA_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MMAP_AREA_NVS 4
#define MULTIBOOT_MMAP_AREA_BADRAM 5
  uint32_t type;
} __attribute__((packed));

struct multiboot_module {
  uint32_t start;
  uint32_t end;
  const char *string;
  uint32_t zero;
};

struct multiboot_palette_color {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

struct multiboot_information {
  uint32_t flags;
  uint32_t mem_lower;
  uint32_t mem_upper;
  uint32_t boot_device;
  const char *cmdline;
  uint32_t mods_count;
  struct multiboot_module *mods;
  uint32_t bit_4_5_28;
  uint32_t bit_4_5_32;
  uint32_t bit_4_5_36;
  uint32_t bit_4_5_40;
  uint32_t mmap_length;
  struct multiboot_mmap_entry *mmap;

  uint32_t drives_length;
  uint32_t drives_addr;

  uint32_t rom_config_table;
  uint32_t bootloader_name;
  uint32_t apm_table;

  uint32_t vbe_control_info;
  uint32_t vbe_mode_info;
  uint16_t vbe_mode;
  uint16_t vbe_interface_seg;
  uint16_t vbe_interface_off;
  uint16_t vbe_interface_len;
  uint64_t framebuffer_addr;
  uint32_t framebuffer_pitch, framebuffer_width, framebuffer_height;
  uint8_t framebuffer_bpp;
#define MULTIBOOT_FRAMEBUFFER_INDEXED 0
#define MULTIBOOT_FRAMEBUFFER_RGB 1
#define MULTIBOOT_FRAMEBUFFER_EGA_TEXT 2
  uint8_t framebuffer_type;
  union {
    struct {
      struct multiboot_palette_color *palette;
      uint16_t num_colors;
    } indexed;
    struct {
      uint8_t red_field_pos;
      uint8_t red_mask_size;
      uint8_t green_field_pos;
      uint8_t green_mask_size;
      uint8_t blue_field_pos;
      uint8_t blue_mask_size;
    } rgb;
  } framebuffer_color_info;
};

void multiboot_init_early(int magic, struct multiboot_information *boot_ptr);
void multiboot_init(struct multiboot_information *boot_ptr);
