#include <depthos/emu.h>
#include <depthos/ports.h>

void qemu_shutdown() {
  outw(0xB004, 0x2000);
  outw(0x604, 0x2000);
}

void qemu_debugc(char c) { outb(0xE9, c); }
void qemu_debug(const char *s) {
  while (*s)
    qemu_debugc(*s++);
}
