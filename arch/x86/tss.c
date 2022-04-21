#include <depthos/tss.h>
struct tss kernel_tss __align(4096);

void tss_init() {
  memset(&kernel_tss, 0, sizeof(struct tss));
  kernel_tss.cs = 0x0b;
  kernel_tss.ds = 0x13;
  kernel_tss.es = 0x13;
  kernel_tss.fs = 0x13;
  kernel_tss.gs = 0x13;
  kernel_tss.ss = 0x13;
}
void tss_set_stack(void *stack) {
  kernel_tss.ss0 = 0x10;
  kernel_tss.esp0 = stack;
}
