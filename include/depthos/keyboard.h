#pragma once

#include <depthos/stdtypes.h>

typedef enum KEYCODE {

// Alphanumeric keys ////////////////

#define KEYCODES                                                               \
  _DEFINE_KEYCODE(UNKNOWN, "Unknown")                                          \
  _DEFINE_KEYCODE(SPACE, " ")                                                  \
  _DEFINE_KEYCODE(0, "0")                                                      \
  _DEFINE_KEYCODE(1, "1")                                                      \
  _DEFINE_KEYCODE(2, "2")                                                      \
  _DEFINE_KEYCODE(3, "3")                                                      \
  _DEFINE_KEYCODE(4, "4")                                                      \
  _DEFINE_KEYCODE(5, "5")                                                      \
  _DEFINE_KEYCODE(6, "6")                                                      \
  _DEFINE_KEYCODE(7, "7")                                                      \
  _DEFINE_KEYCODE(8, "8")                                                      \
  _DEFINE_KEYCODE(9, "9")                                                      \
  _DEFINE_KEYCODE(A, "a")                                                      \
  _DEFINE_KEYCODE(B, "b")                                                      \
  _DEFINE_KEYCODE(C, "c")                                                      \
  _DEFINE_KEYCODE(D, "d")                                                      \
  _DEFINE_KEYCODE(E, "e")                                                      \
  _DEFINE_KEYCODE(F, "f")                                                      \
  _DEFINE_KEYCODE(G, "g")                                                      \
  _DEFINE_KEYCODE(H, "h")                                                      \
  _DEFINE_KEYCODE(I, "i")                                                      \
  _DEFINE_KEYCODE(J, "j")                                                      \
  _DEFINE_KEYCODE(K, "k")                                                      \
  _DEFINE_KEYCODE(L, "l")                                                      \
  _DEFINE_KEYCODE(M, "m")                                                      \
  _DEFINE_KEYCODE(N, "n")                                                      \
  _DEFINE_KEYCODE(O, "o")                                                      \
  _DEFINE_KEYCODE(P, "p")                                                      \
  _DEFINE_KEYCODE(Q, "q")                                                      \
  _DEFINE_KEYCODE(R, "r")                                                      \
  _DEFINE_KEYCODE(S, "s")                                                      \
  _DEFINE_KEYCODE(T, "t")                                                      \
  _DEFINE_KEYCODE(U, "u")                                                      \
  _DEFINE_KEYCODE(V, "v")                                                      \
  _DEFINE_KEYCODE(W, "w")                                                      \
  _DEFINE_KEYCODE(X, "x")                                                      \
  _DEFINE_KEYCODE(Y, "y")                                                      \
  _DEFINE_KEYCODE(Z, "z")                                                      \
                                                                               \
  _DEFINE_KEYCODE(DOT, ".")                                                    \
  _DEFINE_KEYCODE(COMMA, ",")                                                  \
  _DEFINE_KEYCODE(COLON, ":")                                                  \
  _DEFINE_KEYCODE(SEMICOLON, ";")                                              \
  _DEFINE_KEYCODE(SLASH, "/")                                                  \
  _DEFINE_KEYCODE(BACKSLASH, "\\")                                             \
  _DEFINE_KEYCODE(PLUS, "+")                                                   \
  _DEFINE_KEYCODE(MINUS, "-")                                                  \
  _DEFINE_KEYCODE(ASTERISK, "*")                                               \
  _DEFINE_KEYCODE(EXCLAMATION, "!")                                            \
  _DEFINE_KEYCODE(QUESTION, "?")                                               \
  _DEFINE_KEYCODE(QUOTEDOUBLE, "\"")                                           \
  _DEFINE_KEYCODE(QUOTE, "'")                                                  \
  _DEFINE_KEYCODE(EQUAL, "=")                                                  \
  _DEFINE_KEYCODE(HASH, "#")                                                   \
  _DEFINE_KEYCODE(PERCENT, "%")                                                \
  _DEFINE_KEYCODE(AMPERSAND, "&")                                              \
  _DEFINE_KEYCODE(UNDERSCORE, "_")                                             \
  _DEFINE_KEYCODE(LEFTPARENTHESIS, "(")                                        \
  _DEFINE_KEYCODE(RIGHTPARENTHESIS, ")")                                       \
  _DEFINE_KEYCODE(LEFTBRACKET, "[")                                            \
  _DEFINE_KEYCODE(RIGHTBRACKET, "]")                                           \
  _DEFINE_KEYCODE(LEFTCURL, "{")                                               \
  _DEFINE_KEYCODE(RIGHTCURL, "}")                                              \
  _DEFINE_KEYCODE(DOLLAR, "$")                                                 \
  _DEFINE_KEYCODE(POUND, "$")                                                  \
  _DEFINE_KEYCODE(EURO, "$")                                                   \
  _DEFINE_KEYCODE(LESS, "<")                                                   \
  _DEFINE_KEYCODE(GREATER, ">")                                                \
  _DEFINE_KEYCODE(BAR, "|")                                                    \
  _DEFINE_KEYCODE(GRAVE, "`")                                                  \
  _DEFINE_KEYCODE(TILDE, "~")                                                  \
  _DEFINE_KEYCODE(AT, "@")                                                     \
  _DEFINE_KEYCODE(CARRET, "^")                                                 \
                                                                               \
  _DEFINE_KEYCODE(RETURN, "\n")                                                \
  _DEFINE_KEYCODE(ESCAPE, "\033")                                              \
  _DEFINE_KEYCODE(BACKSPACE, "\b")                                             \
  _DEFINE_KEYCODE(CARRIAGE_RETURN, "\r")                                       \
                                                                               \
  /* ////////////////// Arrow keys //////////////////  */                      \
                                                                               \
  _DEFINE_KEYCODE(UP, "\033[1A")                                               \
  _DEFINE_KEYCODE(DOWN, "\033[1B")                                             \
  _DEFINE_KEYCODE(LEFT, "\033[1C")                                             \
  _DEFINE_KEYCODE(RIGHT, "\033[1D")                                            \
                                                                               \
  /* //////////////////  Function keys //////////////////  */                  \
                                                                               \
  _DEFINE_KEYCODE(F1, "F1")                                                    \
  _DEFINE_KEYCODE(F2, "F2")                                                    \
  _DEFINE_KEYCODE(F3, "F3")                                                    \
  _DEFINE_KEYCODE(F4, "F4")                                                    \
  _DEFINE_KEYCODE(F5, "F5")                                                    \
  _DEFINE_KEYCODE(F6, "F6")                                                    \
  _DEFINE_KEYCODE(F7, "F7")                                                    \
  _DEFINE_KEYCODE(F8, "F8")                                                    \
  _DEFINE_KEYCODE(F9, "F9")                                                    \
  _DEFINE_KEYCODE(F10, "F10")                                                  \
  _DEFINE_KEYCODE(F11, "F11")                                                  \
  _DEFINE_KEYCODE(F12, "F12")                                                  \
  _DEFINE_KEYCODE(F13, "F13")                                                  \
  _DEFINE_KEYCODE(F14, "F14")                                                  \
  _DEFINE_KEYCODE(F15, "F15")                                                  \
                                                                               \
  /* //////////////////  Numpad //////////////////  */                         \
                                                                               \
  _DEFINE_KEYCODE(KP_0, "Numpad0")                                             \
  _DEFINE_KEYCODE(KP_1, "Numpad1")                                             \
  _DEFINE_KEYCODE(KP_2, "Numpad2")                                             \
  _DEFINE_KEYCODE(KP_3, "Numpad3")                                             \
  _DEFINE_KEYCODE(KP_4, "Numpad4")                                             \
  _DEFINE_KEYCODE(KP_5, "Numpad5")                                             \
  _DEFINE_KEYCODE(KP_6, "Numpad6")                                             \
  _DEFINE_KEYCODE(KP_7, "Numpad7")                                             \
  _DEFINE_KEYCODE(KP_8, "Numpad8")                                             \
  _DEFINE_KEYCODE(KP_9, "Numpad9")                                             \
  _DEFINE_KEYCODE(KP_PLUS, "Numpad +")                                         \
  _DEFINE_KEYCODE(KP_MINUS, "Numpad -")                                        \
  _DEFINE_KEYCODE(KP_DECIMAL, "Numpad .")                                      \
  _DEFINE_KEYCODE(KP_DIVIDE, "Numpad /")                                       \
  _DEFINE_KEYCODE(KP_ASTERISK, "Numpad *")                                     \
  _DEFINE_KEYCODE(KP_NUMLOCK, "NumLock")                                       \
  _DEFINE_KEYCODE(KP_ENTER, "Enter")                                           \
                                                                               \
  _DEFINE_KEYCODE(TAB, "\t")                                                   \
  _DEFINE_KEYCODE(CAPSLOCK, "CapsLock")                                        \
                                                                               \
  /* //////////////////  Modification keys //////////////////  */              \
                                                                               \
  _DEFINE_KEYCODE(LSHIFT, "Shift")                                             \
  _DEFINE_KEYCODE(LCTRL, "Ctrl")                                               \
  _DEFINE_KEYCODE(LALT, "Alt")                                                 \
  _DEFINE_KEYCODE(LSYS, "System")                                              \
  _DEFINE_KEYCODE(RSHIFT, "RShift")                                            \
  _DEFINE_KEYCODE(RCTRL, "RCtrl")                                              \
  _DEFINE_KEYCODE(RALT, "RAlt")                                                \
  _DEFINE_KEYCODE(RSYS, "RSystem")                                             \
                                                                               \
  _DEFINE_KEYCODE(INSERT, "Insert")                                            \
  _DEFINE_KEYCODE(DELETE, "Delete")                                            \
  _DEFINE_KEYCODE(HOME, "Home")                                                \
  _DEFINE_KEYCODE(END, "End")                                                  \
  _DEFINE_KEYCODE(PAGEUP, "PageUp")                                            \
  _DEFINE_KEYCODE(PAGEDOWN, "PageDown")                                        \
  _DEFINE_KEYCODE(SCROLLLOCK, "ScrollLock")                                    \
  _DEFINE_KEYCODE(PAUSE, "Pause")                                              \
  // _DEFINE_KEYCODE(KEY_NUMKEYCODES, "Not sure"

#define _DEFINE_KEYCODE(name, repr) KEY_##name,
  KEYCODES
#undef _DEFINE_KEYCODE
} keycode_t;

/**
 * @brief Keyboard event modifier bitmap
 */
typedef uint8_t keyboard_event_modifiers_t;

/**
 * @brief Keyboard event representation
 */
struct keyboard_event {
  keycode_t keycode;
  bool pressed;

#define KEYBOARD_MODIFIER_CTRL 0x1
#define KEYBOARD_MODIFIER_ALT 0x2
#define KEYBOARD_MODIFIER_SHIFT 0x4
#define KEYBOARD_MODIFIER_CAPSLOCK 0x8
  keyboard_event_modifiers_t modifiers;
};
typedef int (*keyboard_event_handler_t)(struct keyboard_event);

/**
 * @brief Convert a keycode to a string representation.
 *
 * @param keycode Keycode to convert.
 * @return const char* String representation of the keycode.
 */
const char *keycode_to_string(keycode_t keycode);

/**
 * @brief Construct keycode from a codepoint
 *
 * @param codepoint Codepoint to construct the keycode from.
 * @return keycode_t Constructed keycode.
 */
keycode_t keycode_from_codepoint(uint32_t codepoint);

/**
 * @brief Set current keyboard event handler
 *
 * @param handler Handler to set
 */
void keyboard_set_handler(keyboard_event_handler_t handler);

/**
 * @brief Initialize keyboard
 */
void ps2_keyboard_init();
