#include <depthos/stdtypes.h>
#include <depthos/io/ports.h>

#define BLACK_COLOR 0
#define BLUE_COLOR 1
#define GREEN_COLOR 2
#define WWBLUE_COLOR 3
#define RED_COLOR 4
#define PURPLE_COLOR 5
#define BROWN_COLOR 6
#define WGRAY_COLOR 7
#define BGRAY_COLOR 8
#define WBLUE_COLOR 9
#define WGREEN_COLOR 10
#define WWWBLUE_COLOR 11
#define PINK_COLOR 12
#define WPURPLE_COLOR 13
#define WBROWN_COLOR 14
#define WHITE_COLOR 15



typedef struct _console {
  void (*putc)(char);
  void (*putcl)(char,uint8_t,uint8_t);
  char (*getc)();
  void (*puts)(char*);
  char* (*gets)(size_t);
  void (*clear)();
  void (*movec)(int, int);
  void (*flush)();
  int xsize, ysize;
  int cx, cy;
}console_t;

extern console_t _krnl_console;

