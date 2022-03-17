#include <depthos/console.h>
#include <depthos/ports.h>
#include <depthos/tools.h>

struct serial_device {
  unsigned long base;
};

#define TX(dev) ((dev)->base + 0)
#define DLAB_LO(dev) ((dev)->base + 0)
#define DLAB_HI(dev) ((dev)->base + 1)

#define LINE_CONTROL(dev) ((dev)->base + 3)
#define LINE_CONTROL_8BIT 0x03
#define LINE_CONTROL_DLAB 0x80

#define LINE_STATUS(dev) ((dev)->base + 5)
#define LINE_STATUS_THRE 0x20

void serial_console_output(void *context, const char *data, size_t sz) {
  struct serial_device *dev = context;
  const char *end = data + sz;

  while (data != end) {
    while (!(inb(LINE_STATUS(dev)) & LINE_STATUS_THRE))
      ;
    outb(TX(dev), *data);
    ++data;
  }
}

void serial_console_init(unsigned id) {
  static struct serial_device device[] = {
      {
          .base = 0x3f8,
      },
      {
          .base = 0x2f8,
      },
      {
          .base = 0x3e8,
      },
      {
          .base = 0x2e8,
      },
  };

  if (id < ARRAY_SIZE(device)) {
    struct serial_device *dev = device + id;

    outb(LINE_CONTROL(dev), LINE_CONTROL_8BIT | LINE_CONTROL_DLAB);
    outb(DLAB_LO(dev), 1);
    outb(DLAB_HI(dev), 1);
    outb(LINE_CONTROL(dev), LINE_CONTROL_8BIT);

    register_console(serial_console_output, dev);
  }
}
