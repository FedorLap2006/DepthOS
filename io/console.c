#include <depthos/console.h>
#include <depthos/dev.h>
#include <depthos/kernel.h>
#include <depthos/stdarg.h>
#include <depthos/vformat.h>

unsigned short *videoMemory = (unsigned short *)ADDR_TO_VIRT(0xb8000);

int strs_count = 25;
int strs_len = 80;

int dbcolor = BLACK_COLOR;
int dfcolor = WHITE_COLOR;

int cursorx = 0, cursory = 0;

bool console_no_color = false;

void console_init(int s, int l, int b, int f) {
  if (s > 0)
    strs_count = s;
  if (l > 0)
    strs_len = l;
  if (b > 0)
    dbcolor = b;
  if (f > 0)
    dfcolor = f;
  console_getcursor(&cursorx, &cursory);
  console_flushc();
}

void console_movec(int x, int y) {
  if (cursorx + x >= 0)
    cursorx += x;
  if (cursory + y >= 0)
    cursory += y;
  console_flushc();
}
void console_getcursor(int *x, int *y) {
  uint16_t pos = 0;
  outb(0x3D4, 0x0F);
  pos |= inb(0x3D5);
  outb(0x3D4, 0x0E);
  pos |= ((uint16_t)inb(0x3D5)) << 8;

  *y = pos / strs_len;
  *x = pos % strs_len;
}

void console_flushc() {
  uint16_t pos = (cursory * strs_len) + cursorx;
  outb(0x3D4, 14);
  outb(0x3D5, pos >> 8);
  outb(0x3D4, 15);
  outb(0x3D5, pos);
}

void console_clear() {
  uint8_t ab = (dbcolor << 4) | (dfcolor & 0x0F);
  uint16_t c = 0x20 | (ab << 8);
  for (int i = 0; i < strs_len * strs_count; i++)
    videoMemory[i] = c;
  cursorx = 0;
  cursory = 0;
  console_flushc();
}

void console_flushs() {
  uint8_t at = (dbcolor << 4) | (dfcolor & 0x0F);
  uint16_t b = 0x20 | (at << 8);

  if (cursory >= strs_count) {
    for (int i = 0; i < (strs_count - 1) * strs_len; i++) {
      videoMemory[i] = videoMemory[i + strs_len];
    }
    for (int i = (strs_count - 1) * strs_len; i < strs_count * strs_len; i++) {
      videoMemory[i] = b;
    }
    cursory = strs_count - 1;
  }
}

void console_putchara(unsigned char c, uint8_t a) {
  uint16_t attr = a << 8;
  uint16_t *loc;

  if (c == 0x08 && cursorx) {
    --cursorx;
  } else if (c == 0x09) {
    cursorx = (cursorx + 8) & ~(8 - 1);
  } else if (c == '\r') {
    cursorx = 0;
  } else if (c == '\n') {
    cursorx = 0;
    ++cursory;
  }
  if (c == '\033') { // TODO: properly implement escape codes
    console_putchara('^', a);
    console_putchara('[', a);
    return;
  } else if (c >= ' ') {
    loc = videoMemory + (cursory * strs_len + cursorx);
    *loc = c | attr;
    ++cursorx;
  }
  if (cursorx >= strs_len || cursorx == strs_len - 1) { // 79 80
    console_putchara('\n', (dbcolor << 4) | (dfcolor & 0x0F));
    //		cursorx = 0;
    //		++cursory;
  }
  console_flushs();
  console_flushc();
}

void console_putchar(unsigned char c) {
  console_putchara(c, (dbcolor << 4) | (dfcolor & 0x0F));
}

void console_write(const char *buf) {
  int i = 0;
  while (buf[i]) {
    console_putchar(buf[i]);
    i++;
  }
}

void console_writea(const char *buf, uint8_t a) {
  int i = 0;
  while (buf[i]) {
    console_putchara(buf[i], a);
    i++;
  }
}

void console_write_hex(uint32_t v) {
  console_write("0x");
  console_write_int(v, 16);
}

void console_write_int(uint32_t v, unsigned base) {
  int acc = v;
  char c[33], c2[33];
  int i = 0, j = 0;

  if (v == 0 || base > 16) {
    console_putchar('0');
    return;
  }

  while (acc > 0) {
    c[i] = "0123456789abcdef"[acc % base];
    acc /= base;
    ++i;
  }
  c[i] = 0;
  c2[i--] = 0;
  while (i >= 0)
    c2[i--] = c[j++];

  console_write(c2);
}

void console_write_dec(uint32_t v) { console_write_int(v, 10); }

void console_write_color(const char *buf, int8_t b, int8_t f) {
  if (b < 0) {
    b = dbcolor;
  }
  if (f < 0) {
    f = dfcolor;
  }
  console_writea(buf, (b << 4) | (f & 0x0F));
}

void console_putchar_color(unsigned char c, int8_t b, int8_t f) {
  if (b < 0) {
    b = dbcolor;
  }
  if (f < 0) {
    f = dfcolor;
  }
  console_putchara(c, (b << 4) | (f & 0x0F));
}

void console_output(void *context, const char *data, size_t sz) {
  while (sz-- > 0)
    console_putchar(*data++);
}

static void (*output_console)(void *context, const char *data,
                              size_t sz) = console_output;
static void *console_context;

void switch_console_provider(void (*output)(void *context, const char *data,
                                            size_t sz),
                             void *context) {
  output_console = output;
  console_context = context;
}

void putk(char c) { output_console(console_context, &c, 1); }

void vprintk(const char *fmt, va_list ap) {
  vformat(output_console, console_context, fmt, ap);
}

void printk(const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  vprintk(fmt, ap);
  va_end(ap);
}

int consoledev_write(struct device *dev, void *buffer, size_t nbytes,
                     off_t *off) {
  // TODO: offsets
  for (int i = 0; i < nbytes; i++)
    console_putchar(((unsigned char *)buffer)[i]);
  *off += nbytes;
  return nbytes;
}

long consoledev_ioctl(struct device *dev, unsigned long request, void *data) {
  switch (request) {
  case CONSOLE_IOCTL_CLEAR:
    console_clear();
    break;
  case CONSOLE_IOCTL_MOVECURSOR: {
    struct console_ioctl_movecursor *d =
        (struct console_ioctl_movecursor *)data;
    console_movec(d->dx, d->dy);
    break;
  }
  }
}

struct device console_device = (struct device){
    .name = "textvga",
    .pos = 0,
    .write = consoledev_write,
    .read = NULL, // TODO: ps/2
    .ioctl = consoledev_ioctl,
};
