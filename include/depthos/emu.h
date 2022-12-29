#pragma once

#ifdef CONFIG_EMULATOR_QEMU
void qemu_shutdown();
void qemu_debugc(char c);
void qemu_debug(const char *s);

#endif
