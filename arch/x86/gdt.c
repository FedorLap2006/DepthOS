#include <depthos/logging.h>
#include <depthos/stdtypes.h>
#include <depthos/string.h>
#include <depthos/x86/bootstrap.h>
#include <depthos/x86/gdt.h>

typedef uint64_t gdt_desc_t;

static inline gdt_desc_t make_gdt_desc(uintptr_t base, size_t limit, int flags, int access) {
  uint32_t b1 = GDT_DESC_BASE_LOW(base) | GDT_DESC_LIMIT_LOW(limit);
  uint32_t b2 =
      GDT_DESC_BASE_MIDHIGH(base) | GDT_DESC_LIMIT_HIGH(limit) | GDT_DESC_ACCESS(access) | GDT_DESC_FLAGS(flags);

  return ((gdt_desc_t)b2 << 32) | (gdt_desc_t)b1;
}

gdt_desc_t __section(BOOTSTRAP_DATA) gdt_entries[GDT_SIZE];

// x86_gdt_init is an important part of the early booting process.
// But since lower loader only identity-maps the first 4 megabytes, we can't
// access anything past that. Thus if GCC places x86_gdt_init and any other
// important functions higher up, we will not be able to boot. To prevent GCC
// from doing that, these functions are put in a separate section, which is
// linked right after multiboot header.
__section(BOOTSTRAP_CODE) void x86_gdt_init() {
  for (int i = 0; i < GDT_SIZE; i++)
    gdt_entries[i] = 0;
  x86_gdt_set_entry(0x0, (struct gdt_entry){.base = 0x0, .limit = 0x0, .flags = 0x0, .access = 0x0});
  x86_gdt_set_entry(1, make_gdt_entry_code(KERNEL_CODE_BASE, KERNEL_CODE_LIMIT, 0, false, true));
  x86_gdt_set_entry(2, make_gdt_entry_data(KERNEL_DATA_BASE, KERNEL_DATA_LIMIT, 0, false, true));
  x86_gdt_set_entry(3, make_gdt_entry_code(USER_CODE_BASE, USER_CODE_LIMIT, 0x3, false, true));
  x86_gdt_set_entry(4, make_gdt_entry_data(USER_DATA_BASE, USER_DATA_LIMIT, 0x3, false, true));
  extern struct tss kernel_tss __align(4096);
  x86_gdt_set_entry(5, make_gdt_entry_tss((uintptr_t)&kernel_tss, 0x0));
  x86_gdt_set_entry(6, make_gdt_entry_data(0x0, 0xfffff, 0x3, false, true)); // gs
  x86_gdt_set_entry(7, make_gdt_entry_data(0x0, 0xfffff, 0x3, false, true)); // fs

  x86_gdt_reload();
  // __asm__ volatile("push %eax; pop %eax");
  __asm__ volatile("ltr %w0" : : "r"(GDT_TSS_SEL)); // TODO: clobber?
  // __asm__ volatile goto("lea %l0, %%eax;"
  //                       "ljmp $0x08, %%eax"
  //                       :
  //                       :
  //                       : "eax"
  //                       : protected);
  // __asm__ volatile goto("lea %l[protected], %%eax;"
  //                       "mov $0x08, %%ebx;"
  //                       "ljmp %%ebx, *%%eax" ::
  //                           : "eax", "ebx"
  //                       : protected);
  __asm__ volatile("pushw %0;"
                   "push $1f;"
                   "retf;"
                   "1:;" ::"i"(GDT_KERNEL_CODE_SEL));
}
static inline uint64_t make_gdtr(uintptr_t base, uint16_t limit) { return limit | ((uint64_t)base << 16); }

__section(BOOTSTRAP_CODE) void x86_gdt_reload() {
  uint64_t gdtr = make_gdtr((uintptr_t)gdt_entries, sizeof(gdt_entries) - 1);
  __asm__ volatile("lgdt %0" ::"m"(gdtr));
  __asm__ volatile("mov %0, %%ax;\n"
                   "mov %%ax, %%ds;\n"
                   "mov %%ax, %%es;\n"
                   "mov %%ax, %%ss;\n"
                   "mov %1, %%ax;\n"
                   "mov %%ax, %%fs;\n"
                   "mov %2, %%ax;\n"
                   "mov %%ax, %%gs;"
                   :
                   : "i"(GDT_KERNEL_DATA_SEL), "i"(GDT_FSBASE_SEL(0)), "i"(GDT_GSBASE_SEL(0))
                   : "ax");
  return;
}

__section(BOOTSTRAP_CODE) void x86_gdt_set_entry(off_t idx, struct gdt_entry entry) {
  if (idx >= GDT_SIZE) return;
  gdt_entries[idx] = make_gdt_desc(entry.base, entry.limit, entry.flags, entry.access);
}

__section(BOOTSTRAP_CODE) void x86_gdt_set_base(off_t idx, uintptr_t base) {
  if (idx >= GDT_SIZE) return;
  uint32_t b1 = (gdt_entries[idx] & ~GDT_DESC_BASE_LOW_MASK) | GDT_DESC_BASE_LOW(base);
  uint32_t b2 = (gdt_entries[idx] & ~GDT_DESC_BASE_MIDHIGH_MASK) | GDT_DESC_BASE_MIDHIGH(base);
  gdt_entries[idx] = ((gdt_desc_t)b2 << 32) | b1;
}
