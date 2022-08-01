#include <depthos/string.h>
#include <depthos/tss.h>
#include <depthos/x86/gdt.h>
struct tss kernel_tss __align(4096);

void tss_init() {
  memset(&kernel_tss, 0, sizeof(struct tss));
}
void tss_set_stack(void *stack) {
  kernel_tss.ss0 = GDT_KERNEL_DATA_SEL;
  kernel_tss.esp0 = stack;
}
