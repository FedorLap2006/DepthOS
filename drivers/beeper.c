#include <depthos/beeper.h>
#include <depthos/dev.h>
#include <depthos/errno.h>
#include <depthos/ports.h>

#define PIT2_FREQ_DIV 1193180

void beeper_play(uint32_t nf) {
  uint32_t div;
  uint8_t tmp;

  // Set the PIT to the desired frequency
  div = PIT2_FREQ_DIV / nf;
  outb(0x43, 0xb6);
  outb(0x42, (uint8_t)(div));
  outb(0x42, (uint8_t)(div >> 8));

  // And play the sound using the PC speaker
  tmp = inb(0x61);
  if (tmp != (tmp | 3)) {
    outb(0x61, tmp | 3);
  }
}

void beeper_stop() {
  uint8_t tmp = inb(0x61) & 0xFC;

  outb(0x61, tmp);
}

long beeper_ioctl(struct device *dev, unsigned long request, void *data) {
  switch (request) {
  case BEEPER_IOCTL_NPLAY:
    if ((uint32_t)data == 0) {
      return -EINVAL;
    }
    beeper_play((uint32_t)data);
    break;
  case BEEPER_IOCTL_NSTOP:
    beeper_stop();
    break;
  }
}

struct device beeper_device = {
    .name = "beeper",
    .ioctl = beeper_ioctl,
    .read = NULL,
    .write = NULL,
    .seek = NULL,
    .mmap = NULL,
};

void beeper_init() { register_device(&beeper_device); }
