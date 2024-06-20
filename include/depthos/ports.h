#pragma once

#include <depthos/stdtypes.h>

void outb(uint16_t port, uint8_t value);

void outw(uint16_t port, uint16_t value);

void outl(uint16_t port, uint32_t value);

uint8_t inb(uint16_t port);

uint16_t inw(uint16_t port);

uint32_t inl(uint16_t port);

void iowait(uint32_t microseconds);
