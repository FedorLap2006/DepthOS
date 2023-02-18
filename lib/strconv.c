#include <depthos/strconv.h>
#include <depthos/string.h>

void itoa(unsigned int v, unsigned int base, char *buf) {
  int acc = v;
  char c[33];
  int i = 0, j = 0;

  if (v == 0) {
    strcpy(buf, "0");
    return;
  } else if (base > 16) {
    return;
  }

  while (acc > 0) {
    c[i] = "0123456789abcdef"[acc % base];
    acc /= base;
    ++i;
  }
  c[i] = 0;
  buf[i--] = 0;
  while (i >= 0)
    buf[i--] = c[j++];
}
