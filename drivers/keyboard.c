#include <depthos/idt.h>
#include <depthos/keyboard.h>
#include <depthos/string.h>

// TODO: release scancodes
// TODO: ctrl, shift and alt handling
static uint32_t scancode_set_1[] = {
    // key scancode
    KEY_UNKNOWN,      // 0
    KEY_ESCAPE,       // 1
    KEY_1,            // 2
    KEY_2,            // 3
    KEY_3,            // 4
    KEY_4,            // 5
    KEY_5,            // 6
    KEY_6,            // 7
    KEY_7,            // 8
    KEY_8,            // 9
    KEY_9,            // 0xa
    KEY_0,            // 0xb
    KEY_MINUS,        // 0xc
    KEY_EQUAL,        // 0xd
    KEY_BACKSPACE,    // 0xe
    KEY_TAB,          // 0xf
    KEY_Q,            // 0x10
    KEY_W,            // 0x11
    KEY_E,            // 0x12
    KEY_R,            // 0x13
    KEY_T,            // 0x14
    KEY_Y,            // 0x15
    KEY_U,            // 0x16
    KEY_I,            // 0x17
    KEY_O,            // 0x18
    KEY_P,            // 0x19
    KEY_LEFTBRACKET,  // 0x1a
    KEY_RIGHTBRACKET, // 0x1b
    KEY_RETURN,       // 0x1c
    KEY_LCTRL,        // 0x1d
    KEY_A,            // 0x1e
    KEY_S,            // 0x1f
    KEY_D,            // 0x20
    KEY_F,            // 0x21
    KEY_G,            // 0x22
    KEY_H,            // 0x23
    KEY_J,            // 0x24
    KEY_K,            // 0x25
    KEY_L,            // 0x26
    KEY_SEMICOLON,    // 0x27
    KEY_QUOTE,        // 0x28
    KEY_GRAVE,        // 0x29
    KEY_LSHIFT,       // 0x2a
    KEY_BACKSLASH,    // 0x2b
    KEY_Z,            // 0x2c
    KEY_X,            // 0x2d
    KEY_C,            // 0x2e
    KEY_V,            // 0x2f
    KEY_B,            // 0x30
    KEY_N,            // 0x31
    KEY_M,            // 0x32
    KEY_COMMA,        // 0x33
    KEY_DOT,          // 0x34
    KEY_SLASH,        // 0x35
    KEY_RSHIFT,       // 0x36
    KEY_KP_ASTERISK,  // 0x37
    KEY_RALT,         // 0x38
    KEY_SPACE,        // 0x39
    KEY_CAPSLOCK,     // 0x3a
    KEY_F1,           // 0x3b
    KEY_F2,           // 0x3c
    KEY_F3,           // 0x3d
    KEY_F4,           // 0x3e
    KEY_F5,           // 0x3f
    KEY_F6,           // 0x40
    KEY_F7,           // 0x41
    KEY_F8,           // 0x42
    KEY_F9,           // 0x43
    KEY_F10,          // 0x44
    KEY_KP_NUMLOCK,   // 0x45
    KEY_SCROLLLOCK,   // 0x46
    KEY_HOME,         // 0x47
    KEY_KP_8,         // 0x48 //keypad up arrow
    KEY_PAGEUP,       // 0x49
    KEY_KP_2,         // 0x50 //keypad down arrow
    KEY_KP_3,         // 0x51 //keypad page down
    KEY_KP_0,         // 0x52 //keypad insert key
    KEY_KP_DECIMAL,   // 0x53 //keypad delete key
    KEY_UNKNOWN,      // 0x54
    KEY_UNKNOWN,      // 0x55
    KEY_UNKNOWN,      // 0x56
    KEY_F11,          // 0x57
    KEY_F12           // 0x58

    //
};

unsigned char keycodes_buf[1024 * 5];
void (*keyboard_event_handler)(uint32_t keycode);

static int keycodes_pos = 0;

void keyboard_driver_intr_handler(regs_t r) {
  uint8_t status;
  uint16_t scancode;

  /*	if (keycodes_pos == ARRAY_SIZE(keycodes_buf) - 1) {
                  keycodes_pos = 0;
          }
  */
  status = inb(0x64);
  if (status & 0x01) {
    scancode = inb(0x60);
    if (scancode >= ARRAY_SIZE(scancode_set_1))
      return;
    keyboard_event_handler(scancode_set_1[scancode]);
    //		keycodes_buf[keycodes_pos++] = keycode;
    //		console_write_dec(keycode);
  }
  return;
}

void standard_keycode_handler(uint32_t keycode) {
  switch (keycode) {
  case KEY_RALT:
  case KEY_LCTRL:
  case KEY_RCTRL:
  case KEY_LSHIFT:
  case KEY_RSHIFT:
    break;
  case KEY_BACKSPACE:
    console_putchar(0x08);
    console_putchar_color(' ', -1, WHITE_COLOR);
    console_movec(-1, 0);
    break;
  default:
    console_putchar_color(keycode, -1, WHITE_COLOR);
  }
}

char keyboard_driver_readchar() {
  if (keycodes_pos - 1 < 0)
    return -1;
  else
    return keycodes_buf[keycodes_pos--];
}

int keyboard_driver_readkc() {
  if (keycodes_pos - 1 < 0)
    return -1;
  else
    return scancode_set_1[keycodes_buf[keycodes_pos--]];
}

void keyboard_driver_init() {
  idt_register_interrupt(0x20 + 0x1, keyboard_driver_intr_handler);
  keyboard_driver_set_handler(standard_keycode_handler);
}
void keyboard_driver_set_handler(void (*handler)(int)) {
  if (!handler)
    return;
  keyboard_event_handler = handler;
}