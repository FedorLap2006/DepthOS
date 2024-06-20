#include <depthos/ps2.h>
#include <depthos/ports.h>

uint8_t ps2_read () {
  return inb(PS2_DATA_PORT);
}

uint8_t ps2_get_status() {
  return inb(PS2_STATUS_PORT);
}
