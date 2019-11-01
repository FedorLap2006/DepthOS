#include "ports.h"

void outb(uint16_t port, uint8_t value) {
  __asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}


void outw(uint16_t port, uint16_t value) {
  __asm volatile  ("outw %1, %0" : : "dN" (port), "a" (value));
}

uint8_t inb(uint16_t port) {
  uint8_t ret;
  __asm volatile ("inb %1, %0" : "=a" (ret) : "dN" (port));
  return ret;
}

uint16_t inw(uint16_t port) {
  uint16_t ret;
  __asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
  return ret;
}