#include <depthos/bitmap.h>
#include <depthos/console.h>
#include <depthos/dev.h>
#include <depthos/heap.h>
#include <depthos/keyboard.h>
#include <depthos/proc.h>
#include <depthos/ringbuffer.h>
#include <depthos/string.h>


#define TTY_BUFFER_SIZE 100

int tty_write(struct device *dev, char *buffer, size_t nbytes) {
  for (int i = 0; i < nbytes; i++) {
    console_putchar(*buffer++);
  }
  return nbytes;
}

static ringbuffer_elem_t data[TTY_BUFFER_SIZE];
static struct ringbuffer tty_buffer = (struct ringbuffer){
    .data = data,
    .max_size = TTY_BUFFER_SIZE,
};

int tty_read(struct device *dev, char *buffer, size_t nbytes) {
  ringbuffer_elem_t *rbe = ringbuffer_pop(&tty_buffer);
  if (rbe) {
    buffer[0] = *rbe;
    return 1;
  }
  while (ringbuffer_empty(&tty_buffer))
    __asm__ volatile("int $0x30");

  // TODO: write remaining bytes into the buffer
  return tty_read(dev, buffer, nbytes);
}

long tty_ioctl(struct device *dev, long request, void *data) {}


struct device tty_device = {
    .name = "tty",
    .write = tty_write,
    .read = tty_read,
    .ioctl = tty_ioctl,
    .impl = NULL,
};

// TODO: refactor keyboard handling
void tty_ps2_handler(uint32_t k) {
  char *str = NULL;
  static char str_buf[10];

  switch (k) {
  case KEY_KP_8:
    str = "[[^";
    break;
  default:
    str_buf[0] = (char)k;
    str_buf[1] = 0;
    str = str_buf;
  }

  while (*str) {
    ringbuffer_push(&tty_buffer, (ringbuffer_elem_t)*str++);
  }
}


void tty_init() {
  ringbuffer_init(&tty_buffer, true);
  keyboard_driver_set_handler(tty_ps2_handler);
  // TODO: vcs device and multiple ttys
  devfs_register("console", &tty_device);
  devfs_register("tty", &tty_device); // Current process TTY
  devfs_register("tty0", &tty_device); // Currently opened TTY
  devfs_register("tty1", &tty_device); // First TTY and default TTY
}


#if 0

#define TTY_BUFFER_SIZE 100

static ringbuffer_elem_t tty_buffer_data[TTY_BUFFER_SIZE];
static struct ringbuffer tty_buffer = (struct ringbuffer){
    .data = tty_buffer_data,
    .max_size = TTY_BUFFER_SIZE,
};




struct tty_device {
	int (*getc)();
	void (*putc)(int);
};


/*
struct tty_device ttys[MAX_TTY];
void tty_init() {
	for (int i = 0; i < MAX_TTY; i++) {
		// 1. Make tty a device and register it in the devfs.
		// 2. Wrap the console?
		// TODO: Just have a single tty (tty1) booya!!! Implement multiple tty's (requires /dev/vcs)
	}
}
*/


#define MAX_TTY
struct fs_node tty_files[MAX_TTY];

static struct tty_device* create_tty(int (*getc_impl)(), void (*putc_impl)(int)) {
	struct tty_device *tty = kmalloc(sizeof(tty_device));
	tty->getc = getc_impl;
	tty->putc = putc_impl;
	return tty;
}

static void register_tty(struct tty_device *tty) {

}

void tty_init() {

}

#endif
