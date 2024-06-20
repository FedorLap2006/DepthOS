#pragma once

#include <depthos/dev.h>
#include <depthos/tools.h>

#define FRAMEBUFFER_IOCTL_NVARINFO 1
#define FRAMEBUFFER_IOCTL_NSTINFO 2
#define FRAMEBUFFER_IOCTL_NSETVARINFO 3

struct framebuffer_varinfo {
  uint16_t xres;
  uint16_t yres;
} __pack;

struct framebuffer_stinfo {
  uint16_t width;
  uint16_t height;
  uint16_t pitch;
  uint8_t bpp;
} __pack;
