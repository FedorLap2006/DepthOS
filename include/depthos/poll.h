#pragma once

#include <depthos/stddef.h>

#define POLLIN 0x01
#define POLLOUT 0x02
#define POLLPRI 0x04
#define POLLHUP 0x08
#define POLLERR 0x10
#define POLLRDHUP 0x20
#define POLLNVAL 0x40
#define POLLWRNORM 0x80
#define POLLRDNORM 0x100
#define POLLWRBAND 0x200
#define POLLRDBAND 0x400

typedef size_t nfds_t;

struct pollfd {
  int fd;
  short events;
  short revents;
};
