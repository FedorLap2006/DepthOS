#include <depthos/idt.h>
#include <depthos/keyboard.h>
#include <depthos/logging.h>
#include <depthos/string.h>

// TODO: home, end and arrow keys

static keycode_t scancode_set_1[] = {
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
    /////////////////////////
    KEY_KP_7,       // 0x47
    KEY_KP_8,       // 0x48
    KEY_KP_9,       // 0x49
    KEY_KP_MINUS,   // 0x4A
    KEY_KP_4,       // 0x4B
    KEY_KP_5,       // 0x4C
    KEY_KP_6,       // 0x4D
    KEY_KP_PLUS,    // 0x4E
    KEY_KP_1,       // 0x4F
    KEY_KP_2,       // 0x50
    KEY_KP_3,       // 0x51
    KEY_KP_0,       // 0x52
    KEY_KP_DECIMAL, // 0x53
#if 0
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
#endif
};

static keycode_t scancode_set_1_shift[] = {
    // key scancode
    KEY_UNKNOWN,          // 0
    KEY_ESCAPE,           // 1
    KEY_EXCLAMATION,      // 2
    KEY_AT,               // 3
    KEY_HASH,             // 4
    KEY_DOLLAR,           // 5
    KEY_PERCENT,          // 6
    KEY_CARRET,           // 7
    KEY_AMPERSAND,        // 8
    KEY_ASTERISK,         // 9
    KEY_LEFTPARENTHESIS,  // 0xa
    KEY_RIGHTPARENTHESIS, // 0xb
    KEY_UNDERSCORE,       // 0xc
    KEY_PLUS,             // 0xd
    KEY_BACKSPACE,        // 0xe
    KEY_TAB,              // 0xf
    KEY_Q,                // 0x10
    KEY_W,                // 0x11
    KEY_E,                // 0x12
    KEY_R,                // 0x13
    KEY_T,                // 0x14
    KEY_Y,                // 0x15
    KEY_U,                // 0x16
    KEY_I,                // 0x17
    KEY_O,                // 0x18
    KEY_P,                // 0x19
    KEY_LEFTCURL,         // 0x1a
    KEY_RIGHTCURL,        // 0x1b
    KEY_RETURN,           // 0x1c
    KEY_LCTRL,            // 0x1d
    KEY_A,                // 0x1e
    KEY_S,                // 0x1f
    KEY_D,                // 0x20
    KEY_F,                // 0x21
    KEY_G,                // 0x22
    KEY_H,                // 0x23
    KEY_J,                // 0x24
    KEY_K,                // 0x25
    KEY_L,                // 0x26
    KEY_COLON,            // 0x27
    KEY_QUOTEDOUBLE,      // 0x28
    KEY_TILDE,            // 0x29
    KEY_LSHIFT,           // 0x2a
    KEY_BAR,              // 0x2b
    KEY_Z,                // 0x2c
    KEY_X,                // 0x2d
    KEY_C,                // 0x2e
    KEY_V,                // 0x2f
    KEY_B,                // 0x30
    KEY_N,                // 0x31
    KEY_M,                // 0x32
    KEY_LESS,             // 0x33
    KEY_GREATER,          // 0x34
    KEY_QUESTION,         // 0x35
    KEY_RSHIFT,           // 0x36
    KEY_KP_ASTERISK,      // 0x37
    KEY_RALT,             // 0x38
    KEY_SPACE,            // 0x39
    KEY_CAPSLOCK,         // 0x3a
    KEY_F1,               // 0x3b
    KEY_F2,               // 0x3c
    KEY_F3,               // 0x3d
    KEY_F4,               // 0x3e
    KEY_F5,               // 0x3f
    KEY_F6,               // 0x40
    KEY_F7,               // 0x41
    KEY_F8,               // 0x42
    KEY_F9,               // 0x43
    KEY_F10,              // 0x44
    KEY_KP_NUMLOCK,       // 0x45
    KEY_SCROLLLOCK,       // 0x46
    KEY_HOME,             // 0x47
    KEY_KP_8,             // 0x48 //keypad up arrow
    KEY_PAGEUP,           // 0x49
    KEY_KP_2,             // 0x50 //keypad down arrow
    KEY_KP_3,             // 0x51 //keypad page down
    KEY_KP_0,             // 0x52 //keypad insert key
    KEY_KP_DECIMAL,       // 0x53 //keypad delete key
    KEY_UNKNOWN,          // 0x54
    KEY_UNKNOWN,          // 0x55
    KEY_UNKNOWN,          // 0x56
    KEY_F11,              // 0x57
    KEY_F12               // 0x58
};

keyboard_event_handler_t keyboard_current_event_handler;
static keyboard_event_modifiers_t modifiers;

void ps2_keyboard_handle_scancode(uint8_t payload) {
  uint32_t scancode = payload & 0x7f;
  bool pressed = !(payload & 0x80);
  keycode_t keycode =
      (modifiers & KEYBOARD_MODIFIER_SHIFT ? scancode_set_1_shift
                                           : scancode_set_1)[scancode];
  switch (keycode) {
#define UPD_MODIFIER(name)                                                     \
  if (pressed)                                                                 \
    modifiers |= KEYBOARD_MODIFIER_##name;                                     \
  else                                                                         \
    modifiers &= ~KEYBOARD_MODIFIER_##name
  case KEY_LALT:
  case KEY_RALT:
    UPD_MODIFIER(ALT);
    break;
  case KEY_LCTRL:
  case KEY_RCTRL:
    UPD_MODIFIER(CTRL);
    break;
  case KEY_LSHIFT:
  case KEY_RSHIFT:
    UPD_MODIFIER(SHIFT);
    break;
  case KEY_CAPSLOCK:
    if (pressed)
      modifiers ^= KEYBOARD_MODIFIER_CAPSLOCK;
    break;
  default:
    keyboard_current_event_handler((struct keyboard_event){
        .keycode = keycode,
        .pressed = pressed,
        .modifiers = modifiers,
    });
  }
}

void ps2_keyboard_irq_handler(regs_t *r) {
  uint8_t status = inb(PS2_KEYBOARD_STATUS_PORT);
  if (status & 0x01)
    ps2_keyboard_handle_scancode(inb(PS2_KEYBOARD_DATA_PORT));

  return;
}

const char *keycode_to_string(keycode_t keycode) {
  switch (keycode) {
#define _DEFINE_KEYCODE(name, repr)                                            \
  case KEY_##name:                                                             \
    return repr;
    KEYCODES
#undef _DEFINE_KEYCODE
  }
}

keycode_t keycode_from_codepoint(uint32_t codepoint) {
  switch (codepoint) {
#define MATCH_CHAR(k, ch)                                                      \
  case ch:                                                                     \
    return KEY_##k;
#define MATCH_LETTER(l, ch)                                                    \
  case ch:                                                                     \
  case ch + 32:                                                                \
    return KEY_##l;
    MATCH_LETTER(A, 'A')
    MATCH_LETTER(B, 'B')
    MATCH_LETTER(C, 'C')
    MATCH_LETTER(D, 'D')
    MATCH_LETTER(E, 'E')
    MATCH_LETTER(F, 'F')
    MATCH_LETTER(G, 'G')
    MATCH_LETTER(H, 'H')
    MATCH_LETTER(J, 'J')
    MATCH_LETTER(K, 'K')
    MATCH_LETTER(L, 'L')
    MATCH_LETTER(M, 'M')
    MATCH_LETTER(N, 'N')
    MATCH_LETTER(O, 'O')
    MATCH_LETTER(P, 'P')
    MATCH_LETTER(Q, 'Q')
    MATCH_LETTER(R, 'R')
    MATCH_LETTER(S, 'S')
    MATCH_LETTER(T, 'T')
    MATCH_LETTER(U, 'U')
    MATCH_LETTER(V, 'V')
    MATCH_LETTER(W, 'W')
    MATCH_LETTER(X, 'X')
    MATCH_LETTER(Y, 'Y')
    MATCH_LETTER(Z, 'Z')

    MATCH_CHAR(SPACE, ' ')
    MATCH_CHAR(DOT, '.')
    MATCH_CHAR(COMMA, ',')
    MATCH_CHAR(COLON, ':')
    MATCH_CHAR(SEMICOLON, ';')
    MATCH_CHAR(SLASH, '/')
    MATCH_CHAR(BACKSLASH, '\b')
    MATCH_CHAR(PLUS, '+')
    MATCH_CHAR(MINUS, '-')
    MATCH_CHAR(ASTERISK, '*')
    MATCH_CHAR(EXCLAMATION, '!')
    MATCH_CHAR(QUESTION, '?')
    MATCH_CHAR(QUOTEDOUBLE, '"')
    MATCH_CHAR(QUOTE, '\'')
    MATCH_CHAR(EQUAL, '=')
    MATCH_CHAR(HASH, '#')
    MATCH_CHAR(PERCENT, '%')
    MATCH_CHAR(AMPERSAND, '&')
    MATCH_CHAR(UNDERSCORE, '_')
    MATCH_CHAR(LEFTPARENTHESIS, '(')
    MATCH_CHAR(RIGHTPARENTHESIS, ')')
    MATCH_CHAR(LEFTBRACKET, '[')
    MATCH_CHAR(RIGHTBRACKET, ']')
    MATCH_CHAR(LEFTCURL, '{')
    MATCH_CHAR(RIGHTCURL, '}')
    MATCH_CHAR(DOLLAR, '$')
    MATCH_CHAR(LESS, '<')
    MATCH_CHAR(GREATER, '>')
    MATCH_CHAR(BAR, '|')
    MATCH_CHAR(GRAVE, '`')
    MATCH_CHAR(TILDE, '~')
    MATCH_CHAR(AT, '@')
    MATCH_CHAR(CARRET, '^')

    MATCH_CHAR(RETURN, '\n')
  }
}

void standard_keycode_handler(struct keyboard_event event) {}

void keyboard_set_handler(keyboard_event_handler_t handler) {
  if (!handler)
    return;
  keyboard_current_event_handler = handler;
}

void ps2_keyboard_init() {
  idt_register_interrupt(0x20 + 0x1, ps2_keyboard_irq_handler);
  keyboard_set_handler(standard_keycode_handler);
}
