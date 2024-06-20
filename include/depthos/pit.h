#pragma once

#include <depthos/stdtypes.h>

extern uint32_t pit_ticks;
extern bool pit_sched_enable;

void pit_init(uint32_t tps);
void sleep(uint32_t ticks);
