#pragma once

#include <std/types.h>

void outb(uint16_t port, uint8_t value) {
  asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}


void outw(uint16_t port, uint16 value) {
  asm volatile ("outw %1, %0" : : "dN" (port), "a" (value));
  return ret;
}

uint8_t inb(uint16_t port) {
  uint8_t ret;
  asm volatile ("inb %1, %0" : "=a" (ret) : "dN" (port));
  return ret;
}

uint16_t inw(uint16 port) {
  uint16_t ret;
  asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
  return ret;
}
