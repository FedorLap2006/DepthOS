#pragma once

#include <depthos/stdtypes.h>

#define BEEPER_IOCTL_NPLAY 0x1
#define BEEPER_IOCTL_NSTOP 0x0

void beeper_play(uint32_t nf);
void beeper_stop();
void beeper_init();
