#include <depthos/console.h>
#include <depthos/logging.h>
#include <depthos/stdtypes.h>

void kloga(const char *file, int line, const char *loc, char *msg, ...) {
  static char buffer[1024];
  va_list args;
  va_start(args, msg);
  vsprintf(buffer, msg, args);
  va_end(args);
#if KLOG_ENABLED == 1
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

void dump_registers(regs_t r) {
  printk("EIP=0x%x CS=0x%x EFLAGS=0x%x USERESP=0x%x STACK=0x%x\n", r.eip, r.cs,
         r.eflags, r.useresp, r.ss);
  printk("EDI=0x%x ESI=0x%x EBP=0x%x ESP=0x%x EBX=0x%x EDX=0x%x ECX=0x%x "
         "EAX=0x%x\n",
         r.edi, r.esi, r.ebp, r.esp, r.ebx, r.edx, r.ecx, r.eax);
}