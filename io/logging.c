#include <depthos/console.h>
#include <depthos/logging.h>
#include <depthos/stdtypes.h>

void kloga(const char *file, int line, const char *loc, char *msg, ...) {
  static char buffer[1024];
  va_list args;
  va_start(args, msg);
  vsprintf(buffer, msg, args);
  va_end(args);
#if KLOG_ENABLED
  printk("%s (%s:%d): %s\n", loc, file, line, buffer);
#endif
}

void print_status(char *buf, int m) {
  const char *mod;
  uint8_t color;

  switch (m) {
  case MOD_OK:
    mod = "OK";
    color = WGREEN_COLOR;
    break;
  case MOD_ERR:
    mod = "ERROR";
    color = PINK_COLOR;
    break;
  case MOD_WARNING:
    mod = "WARN";
    color = YELLOW_COLOR;
    break;
  default:
    return;
  }

  console_write("[");
  console_write_color(mod, -1, color);
  console_write("] ");
  console_write_color(buf, -1, WBLUE_COLOR);
  console_write("\n");
}