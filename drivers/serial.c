#include <depthos/console.h>
#include <depthos/idt.h>
#include <depthos/kconfig.h>
#include <depthos/keyboard.h>
#include <depthos/logging.h>
#include <depthos/ports.h>
#include <depthos/string.h>
#include <depthos/tools.h>

struct serial_device {
  unsigned long base;
};

#define TX(dev) ((dev)->base + 0)
#define DLAB_LO(dev) ((dev)->base + 0)
#define DLAB_HI(dev) ((dev)->base + 1)
#define INTR_ENABLE(dev) ((dev)->base + 1)

#define INTR_IDENT(dev) ((dev)->base + 2)
#define INTR_IDENT_NO_PENDING 0x1
#define INTR_IDENT_RECV_LINE_STATUS 0x6
#define INTR_IDENT_FIFO_ENABLE 0xC0

#define MODEM_CONTROL(dev) ((dev)->base + 4)
// #define MODERM_CONTROL_DTR 0x1
#define MODEM_CONTROL_DTR 0x1
#define MODEM_CONTROL_RTS 0x2
#define MODEM_CONTROL_OUT1 0x4
#define MODEM_CONTROL_OUT2 0x8
#define MODEM_CONTROL_LOOP 0x10

#define LINE_CONTROL(dev) ((dev)->base + 3)
#define LINE_CONTROL_8BIT 0x03
#define LINE_CONTROL_DLAB 0x80

#define LINE_STATUS(dev) ((dev)->base + 5)
#define LINE_STATUS_DR 0x1
#define LINE_STATUS_THRE 0x20

static struct serial_device devices[] = {
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

void serial_write(void *context, const char *data, size_t sz) {
  struct serial_device *dev = context;
  const char *end = data + sz;

  while (data != end) {
    while (!(inb(LINE_STATUS(dev)) & LINE_STATUS_THRE))
      ;
    outb(TX(dev), *data);
    ++data;
  }
}

void serial_read(unsigned id, char *buf, size_t n) {
  struct serial_device *dev = devices + id;
  const char *end = buf + n - 1;
  while (buf != end) {
    while (!(inb(LINE_STATUS(dev)) & LINE_STATUS_DR))
      ;
    *buf = inb(TX(dev));
    ++buf;
  }
  *buf = 0;
}

void serial_console_irq_handler(regs_t *r) {
  static char buf[2];
  serial_read(0, buf, 2);
  extern keyboard_event_handler_t keyboard_current_event_handler;
#ifdef CONFIG_SERIAL_DEBUG
  klogf("keycode: %d %d %d", '\n', buf[0], keycode_from_codepoint(buf[0]));
  klogf("serial: 0x%x keycode=0x%x", buf[0], keycode_from_codepoint(buf[0]));
#endif

  keycode_t keycode;
#define REMAP_SERIAL_KEYCODE(original, remap)                                  \
  case KEY_##original:                                                         \
    keycode = KEY_##remap;                                                     \
    break
  switch (buf[0]) {
  case '\r':
    keycode = KEY_RETURN;
    break;
  case 0x7f: // DEL (backspace, the real delete outputs three chars)
    keycode = KEY_BACKSPACE;
    break;
  default:
    keycode = keycode_from_codepoint(buf[0]);
  }
  keyboard_current_event_handler((struct keyboard_event){
      .keycode = keycode, // TODO: ctrl, alt and similar modifiers
      .pressed = true,
      .modifiers = 0,
  });
}

void serial_console_init(unsigned id) {
  if (id < ARRAY_SIZE(devices)) {
    struct serial_device *dev = devices + id;

    outb(INTR_ENABLE(dev), 0x0);
    outb(LINE_CONTROL(dev),
         LINE_CONTROL_8BIT |
             LINE_CONTROL_DLAB); // XXX: do we need LINE_CONTROL_8BIT?
    outb(DLAB_LO(dev), 0x1);
    outb(DLAB_HI(dev), 0x0);
    outb(LINE_CONTROL(dev), LINE_CONTROL_8BIT);

    // Interrupt setup
    outb(INTR_IDENT(dev), INTR_IDENT_RECV_LINE_STATUS | INTR_IDENT_FIFO_ENABLE |
                              INTR_IDENT_NO_PENDING);
    outb(MODEM_CONTROL(dev),
         MODEM_CONTROL_DTR | MODEM_CONTROL_RTS | MODEM_CONTROL_OUT2);

    // Loopback
    outb(MODEM_CONTROL(dev), MODEM_CONTROL_DTR | MODEM_CONTROL_RTS |
                                 MODEM_CONTROL_OUT1 | MODEM_CONTROL_OUT2 |
                                 MODEM_CONTROL_LOOP);

#define TEST_BYTE 0xDE
    outb(TX(dev), TEST_BYTE);
    int tbyte = inb(TX(dev));
    if (tbyte != TEST_BYTE) {
      // TODO: panic
      bootlog("Could not initialise serial: faulty port", LOG_STATUS_ERROR);
      return;
    }
#undef TEST_BYTE
    outb(MODEM_CONTROL(dev), MODEM_CONTROL_DTR | MODEM_CONTROL_RTS |
                                 MODEM_CONTROL_OUT1 | MODEM_CONTROL_OUT2);
    idt_register_interrupt(0x23, serial_console_irq_handler);
    idt_register_interrupt(0x24, serial_console_irq_handler);
    outb(INTR_ENABLE(dev), 0x1);

    register_console(serial_write, dev);
    bootlog("Serial console initialised", LOG_STATUS_SUCCESS);
  }
}
