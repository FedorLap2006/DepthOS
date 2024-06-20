#pragma once

#include <depthos/stddef.h>
#include <depthos/tss.h>
#include <depthos/x86/asm/gdt.h>

#define GDT_SIZE 10

struct gdt_entry {
  uintptr_t base;
  size_t limit;
  int flags;
  int access;
};

static inline int make_gdt_access(uint8_t dpl, bool type, bool dc, bool rw,
                                  bool exec) {
  return 0x0 | GDT_ACCESS_PRESENT | GDT_ACCESS_DPL(dpl) |
         (type ? GDT_ACCESS_S : 0) | (dc ? GDT_ACCESS_DC : 0) |
         (rw ? GDT_ACCESS_TYPE_R | GDT_ACCESS_TYPE_W : 0) |
         (exec ? GDT_ACCESS_TYPE_X : 0);
}

static inline int make_gdt_flags(bool long_mode, bool bit32, bool g) {
  return 0x0 | (long_mode ? GDT_FLAGS_LONG : 0) | (bit32 ? GDT_FLAGS_32 : 0) |
         (g ? GDT_FLAGS_G : 0);
}

static inline int make_gdt_flags16(bool g) {
  return make_gdt_flags(false, false, g);
}
static inline int make_gdt_flags32(bool g) {
  return make_gdt_flags(false, true, g);
}
static inline int make_gdt_flags64(bool g) {
  return make_gdt_flags(true, false, g);
}

#if defined(__i686__) || defined(__i386__)
#define arch_make_gdt_flags(...) make_gdt_flags32(__VA_ARGS__)
#else
#error "Unsupported architecture"
#endif

static inline struct gdt_entry make_gdt_entry_code(uintptr_t base, size_t limit,
                                                   int dpl, bool conform,
                                                   bool g) {
  return (struct gdt_entry){.base = base,
                            .limit = limit,
                            .access =
                                make_gdt_access(dpl, true, conform, true, true),
                            .flags = arch_make_gdt_flags(g)};
}
static inline struct gdt_entry make_gdt_entry_data(uintptr_t base, size_t limit,
                                                   int dpl, bool down, bool g) {
  return (struct gdt_entry){.base = base,
                            .limit = limit,
                            .access =
                                make_gdt_access(dpl, true, down, true, false),
                            .flags = arch_make_gdt_flags(g)};
}

static inline struct gdt_entry make_gdt_entry_tss(uintptr_t addr, int dpl) {
  return (struct gdt_entry){
      .base = addr,
      .limit = sizeof(struct tss),
      .access = GDT_ACCESS_PRESENT | GDT_ACCESS_DPL(dpl) | GDT_ACCESS_TYPE_X |
                GDT_ACCESS_ACCESSED,
      .flags = 0x0,
  };
}

void x86_gdt_init();
__always_inline void x86_gdt_reload();
void x86_gdt_set_base(off_t idx, uintptr_t base);
void x86_gdt_set_entry(off_t idx, struct gdt_entry entry);
