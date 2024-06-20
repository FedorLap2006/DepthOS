#include <depthos/string.h>
#include <depthos/tss.h>
#include <depthos/x86/bootstrap.h>
#include <depthos/x86/gdt.h>

struct tss kernel_tss __align(4096) __section(BOOTSTRAP_DATA);

__section(BOOTSTRAP_CODE) void tss_init() {
  // kernel_tss.cs = 0x1000;
  memset(&kernel_tss, 0, sizeof(struct tss));
}
__section(BOOTSTRAP_CODE) void tss_set_stack(void *stack) {
  kernel_tss.ss0 = GDT_KERNEL_DATA_SEL;
  kernel_tss.esp0 = stack;
}
