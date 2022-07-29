#include <depthos/logging.h>
#include <depthos/x86/gdt.h>

typedef uint64_t gdt_desc_t;

static inline gdt_desc_t make_gdt_desc(uintptr_t base, size_t limit, int flags,
                                       int access) {
  uint32_t b1 = GDT_DESC_BASE_LOW(base) | GDT_DESC_LIMIT_LOW(limit);
  uint32_t b2 = GDT_DESC_BASE_HIGH(base) | GDT_DESC_LIMIT_HIGH(limit) |
                GDT_DESC_ACCESS(access) | GDT_DESC_FLAGS(flags);

  return ((gdt_desc_t)b2 << 32) | b1;
}

static gdt_desc_t gdt_entries[GDT_SIZE];

void x86_gdt_init() {
  memset(gdt_entries, 0x0);
  x86_gdt_set_entry(0x0,
                    (struct gdt_entry){
                        .base = 0x0, .limit = 0x0, .flags = 0x0, .limit = 0x0});
  x86_gdt_set_entry(1, make_gdt_entry_code(KERNEL_CODE_BASE, KERNEL_CODE_LIMIT,
                                           0, false, true));
  x86_gdt_set_entry(2, make_gdt_entry_data(KERNEL_CODE_BASE, KERNEL_DATA_LIMIT,
                                           0, false, true));
  x86_gdt_set_entry(3, make_gdt_entry_code(USER_CODE_BASE, KERNEL_CODE_BASE,
                                           0x3, false, true));
  x86_gdt_set_entry(4, make_gdt_entry_data(USER_DATA_BASE, USER_DATA_LIMIT, 0x3,
                                           false, true));
  extern struct tss kernel_tss __align(4096);
  x86_gdt_set_entry(5, make_gdt_entry_tss(&kernel_tss, 0x0));
  x86_gdt_set_entry(6, make_gdt_entry_data(0x0, 0xffff, 0x3, false, false));
  x86_gdt_set_entry(7, make_gdt_entry_data(0x0, 0xffff, 0x3, false, false));

  x86_gdt_reload();
  __asm__ volatile("mov %0, %%cx;"
                   "ltr %%cx"
                   :
                   : "i"(GDT_TSS_SEL));
  __asm__ volatile("mov %0, %%ax;"
                   "mov %%ax, %%ds;"
                   "mov %%ax, %%es;"
                   "mov %%ax, %%ss;"
                   "mov %1, %%ax;"
                   "mov %%ax, %%fs;"
                   "mov %2, %%ax;"
                   "mov %%ax, %%gs;"
                   :
                   : "i"(GDT_KERNEL_DATA_SEL), "i"(GDT_FSBASE_SEL(0)),
                     "i"(GDT_GSBASE_SEL(0)));
}
static inline uint64_t make_gdtr(uintptr_t base, uint16_t limit) {
  return limit | ((uint64_t)base << 16);
}

void x86_gdt_reload() {
  uint64_t gdtr = make_gdtr(gdt_entries, sizeof(gdt_entries) - 1);
  __asm__ volatile("lgdt %0" ::"m"(gdtr));
}

void x86_gdt_set_entry(off_t idx, struct gdt_entry entry) {
  if (idx >= GDT_SIZE)
    return;
  gdt_entries[idx] =
      make_gdt_desc(entry.base, entry.limit, entry.flags, entry.access);
}

void x86_gdt_set_base(off_t idx, uintptr_t base) {
  if (idx >= GDT_SIZE)
    return;
  uint32_t b1 =
      (gdt_entries[idx] & ~GDT_DESC_BASE_LOW_MASK) | GDT_DESC_BASE_LOW(base);
  uint32_t b2 =
      (gdt_entries[idx] & ~GDT_DESC_BASE_HIGH_MASK) | GDT_DESC_BASE_HIGH(base);
  gdt_entries[idx] = ((gdt_desc_t)b2 << 32) | b1;
}