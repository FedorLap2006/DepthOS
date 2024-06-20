#include <depthos/ports.h>

void outb(uint16_t port, uint8_t value) {
  __asm volatile("outb %1, %0" : : "dN"(port), "a"(value) : "memory");
}

void outw(uint16_t port, uint16_t value) {
  __asm volatile("outw %1, %0" : : "dN"(port), "a"(value) : "memory");
}

void outl(uint16_t port, uint32_t value) {
  __asm volatile("outl %1, %0" : : "dN"(port), "a"(value) : "memory");
}


uint8_t inb(uint16_t port) {
  uint8_t ret;
  __asm volatile("inb %1, %0" : "=a"(ret) : "dN"(port) : "memory");
  return ret;
}

uint16_t inw(uint16_t port) {
  uint16_t ret;
  __asm volatile("inw %1, %0" : "=a"(ret) : "dN"(port) : "memory");
  return ret;
}

uint32_t inl(uint16_t port) {
  uint32_t ret;
  __asm volatile("inl %1, %0" : "=a"(ret) : "dN"(port) : "memory");
  return ret;
}


void iowait(uint32_t microseconds) {
  for (int i = 0; i < microseconds; i++)
    inb(0x80);
}
