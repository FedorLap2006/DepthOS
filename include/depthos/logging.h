#pragma once
#include <depthos/idt.h>
#include <depthos/kconfig.h>

enum { LOG_STATUS_SUCCESS, LOG_STATUS_ERROR, LOG_STATUS_WARNING };
void bootlog(const char *msg, int status);
__printf_fmt(4, 5) void klogv(const char *file, int line, const char *loc,
                              char *msg, ...);
// TODO: somehow disable logging? sinking all arguments but still evaling them?
#define klog(loc, msg, ...) klogv(__FILE__, __LINE__, loc, msg, ##__VA_ARGS__)
#define klogf(...) klog(__func__, ##__VA_ARGS__)

// TODO: anonymous (no line / file) logging
// TODO: driver/module logging

void dump_registers(regs_t r);
