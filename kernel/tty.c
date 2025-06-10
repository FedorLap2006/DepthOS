#include <depthos/bitmap.h>
#include <depthos/console.h>
#include <depthos/dev.h>
#include <depthos/file.h>
#include <depthos/heap.h>
#include <depthos/keyboard.h>
#include <depthos/logging.h>
#include <depthos/proc.h>
#include <depthos/ringbuffer.h>
#include <depthos/string.h>

#define TTY_BUFFER_SIZE 100

// TODO: how do we deal with seeking and offsets
// TODO: modifiers

int tty_write(struct device *dev, void *rbuffer, size_t nbytes, off_t *offset) {
  /*char *tmp = kmalloc(MAX(nbytes + 1, 8));*/
  /*if (!tmp) {*/
  /*panicf("Cannot allocate %d", nbytes+1);*/
  /*}*/
  /*memcpy(tmp, buffer, nbytes);*/
  /*tmp[nbytes] = 0;*/
  char *buffer = rbuffer;
  if (nbytes >= 3 && buffer[0] == 0x1b && buffer[1] == '[' &&
      buffer[2] == ']') {
    // Cannot be yet used, since it uses video memory and we can boot into
    // graphical mode.
    // TODO: backend independent console
    // console_clear(); // TODO: implement escape codes
  } else {
    for (int i = 0; i < nbytes; i++) {
      putk(*buffer++);
    }
  }
  // kfree(tmp, MAX(nbytes + 1, 8));
  return nbytes;
}

static char data[TTY_BUFFER_SIZE];
static struct ringbuffer tty_buffer = (struct ringbuffer){
    .data = data,
    .elem_size = 1,
    .max_size = TTY_BUFFER_SIZE,
};

int tty_read(struct device *dev, void *rbuffer, size_t nbytes, off_t *offset) {
  // klogf("reading...");
  if (!nbytes)
    return 0;
  int r = 0;
  char *buffer = rbuffer;

read_more:;
  char *rbe = ringbuffer_pop(&tty_buffer);
  if (rbe) {
    buffer[0] = *rbe;
    nbytes--;
    buffer++;
    r++;
    // klogf("debug");
    if (nbytes) {
      // klogf("debug 2");
      goto read_more;
    }
    // klogf("debug 3");
    return r;
  } else if (r > 0) { // TODO: r > 1, what's the correct way?
    // klogf("debug 4");
    // klogf("debug");
    return r;
  }

  // klogf("debug");
  while (ringbuffer_empty(&tty_buffer)) { // TODO: open flag for non-blocking
    // klogf("ringbuffer empty");
    // klogf("debug 5");
    sched_yield();
    // __asm__ volatile("int $0x30");
  }
  // TODO: write remaining bytes into the buffer
  return tty_read(dev, buffer, nbytes, offset);
}

long tty_ioctl(struct device *dev, unsigned long request, void *data) {
  klogf("tty ioctl %ld", request);
  return -ENOSYS;
}

struct device tty_device = {
    .name = "tty",
    .class = DEV_C_TTY,
    .write = tty_write,
    .read = tty_read,
    .ioctl = tty_ioctl,
    .seek = dev_impl_seek_zero,
    .impl = NULL,
};

static bool shift_keycode(struct keyboard_event event) {
  if (!(event.modifiers & KEYBOARD_MODIFIER_SHIFT) !=
      !(event.modifiers & KEYBOARD_MODIFIER_CAPSLOCK)) {
    const char *str = keycode_to_string(event.keycode);
    return strlen(str) == 1 && str[0] >= 'a' && str[0] <= 'z';
  }
  return false;
}

static struct device *current_ps2_tty_dev;

// TODO: refactor keyboard handling
int tty_keyboard_handler(struct keyboard_event event) {
  char *str = NULL;
  bool free_str = false;
  if (!event.pressed)
    return;
  switch (event.keycode) {
  case KEY_KP_8:
    str = "[[^";
    break;
  default:
    str = strdup(keycode_to_string(event.keycode));
      free_str = true;
    if (shift_keycode(event)) {
      str[0] -= 32;
    }
    // if (event.modifiers & KEYBOARD_MODIFIER_CTRL != 0) {
    //   char *str2 = kmalloc(strlen(str) + 2);
    //   str2[0] = '^';
    //   strcpy(str2 + 1, str);
    //   kfree(str, strlen(str) + 1);
    //   str = str2;
    // }
  }
  off_t off;
  if (str[0] == '\b') {
    tty_write(current_ps2_tty_dev, "\b \b", 3, &off);
  } else
    tty_write(current_ps2_tty_dev, str, strlen(str), &off);
  while (*str)
    ringbuffer_push(&tty_buffer, str++);
  if (free_str)
    kfree(str, strlen(str));
}

void tty_init() {
  ringbuffer_init(&tty_buffer, true);
  keyboard_set_handler(tty_keyboard_handler);
  // TODO: vcs device and multiple ttys
  devfs_register("console", &tty_device);
  devfs_register("tty", &tty_device);  // Current process TTY
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
