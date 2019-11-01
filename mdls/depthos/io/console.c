#include "console.h"
#include <depthos/io/ports.h>

uint16_t* videoMemory = (unsigned short*)0xB8000;


void __console_minit() {
  _krnl_console.clear();
}

void __krnl_cput(char c) {
  // uint16_t sym;

  // uint8_t fc = BGRAY_COLOR, bc = BLACK_COLOR;

  // sym = c;
  // sym = ( bc << 4 ) | ( fc & 0x0F );

  // videoMemory[_krnl_console.cx + (_krnl_console.cy * _krnl_console.xsize)] = sym;
  // if (_krnl_console.cx < _krnl_console.xsize) _krnl_console.cx++;
  // else { _krnl_console.cy++; _krnl_console.cx = 0; }
  // console_flushc();
  _krnl_console.putcl(c,0,0);
}


void __krnl_cmovec(int x, int y) {
  outb(0x3D4, 14);
  outb(0x3D5, (y * 80 + x) >> 8);
  outb(0x3D4, 15);
  outb(0x3D5, (y * 80 + x));
}

void __krnl_cclear() {
  _krnl_console.movec(0,0);
  for (int i = 0; i < _krnl_console.xsize * _krnl_console.ysize; i++) {
    videoMemory[i] = 0x20 | (((BLACK_COLOR << 4) | (BGRAY_COLOR & 0x0F)) << 8);
  }
  _krnl_console.movec(0,0);
  _krnl_console.flush();
}

void __krnl_cflush() {
  uint8_t at = ( BLACK_COLOR << 4 ) | ( WHITE_COLOR & 0x0F );
  uint16_t sym = 0x20 | ( at << 8 );

  static bool wnls = false;

  // if ( _krnl_console.cy >= 25 ) {
  //   for ( int i = 0; i < 24 * 80; i++ ) {
  //     videoMemory[i] = videoMemory[i+_krnl_console.xsize];
  //   }
  //   for ( int i = 24 * 80; i < 25 * 80; i++ ) {
  //      videoMemory[i] = b;
  //   }
  //   _krnl_console.cy = 24;
  // }

  if(_krnl_console.cx >= _krnl_console.xsize) {
    _krnl_console.cy += _krnl_console.cx / _krnl_console.xsize;
    _krnl_console.cx = _krnl_console.cx - (_krnl_console.cx / _krnl_console.xsize * _krnl_console.xsize);
  }

  if(_krnl_console.cy - _krnl_console.ysize == 1 && !wnls) { wnls = true; return; }
  else if(wnls /* || _krnl_console.cx > 0 && _krnl_console.cy > _krnl_console.ysize */) {
    int i;
    for(i = 0; i < (_krnl_console.ysize - 1) * _krnl_console.xsize; i++) {
      videoMemory[i] = videoMemory[i+_krnl_console.xsize];
    }

    for(i = (_krnl_console.ysize - 1) * _krnl_console.xsize; i < _krnl_console.ysize * _krnl_console.xsize; i++) {
      videoMemory[i] = sym;
    }
    _krnl_console.cy--;
    _krnl_console.cx = 0;
    _krnl_console.movec(_krnl_console.cx, _krnl_console.cy);
    wnls = false;
  }
}

void __krnl_cputcl(char c, uint8_t f, uint8_t b) {
  uint16_t sym;

  uint8_t fc = f ? f : BGRAY_COLOR, bc = b ? b : BLACK_COLOR;

  sym = c;

  if(c == 0x08) {
    if(_krnl_console.cx) --_krnl_console.cx;
    else { --_krnl_console.cy; _krnl_console.cx = _krnl_console.xsize - (1 * 2); }
  } else if(c == 0x09) {
    _krnl_console.cx += 4;
  } else if(c == '\r') {
    _krnl_console.cx = 0;
  } else if(c == '\n') {
    _krnl_console.cx = 0;
    _krnl_console.cy++;
  } else if(c >= ' ') {
    _krnl_console.flush();
    uint16_t *loc = videoMemory + (_krnl_console.cy * _krnl_console.ysize + _krnl_console.cx);
    sym |= (( bc << 4 ) | ( fc & 0x0F )) << 8;
    *loc = sym;
    ++_krnl_console.cx;
  }
  //videoMemory[_krnl_console.cx + (_krnl_console.cy * _krnl_console.xsize)] = sym;

  // if (_krnl_console.cx < _krnl_console.xsize) _krnl_console.cx++;
  // else { _krnl_console.cy++; _krnl_console.cx = 0; }
  _krnl_console.movec(_krnl_console.cx, _krnl_console.cy);
  _krnl_console.flush();
}


void __krnl_cwrite(char* str) {
  int i = 0;
  while(str[i]) { _krnl_console.putc(str[i++]); }
}

console_t _krnl_console = { 
  .putc = __krnl_cput,  
  .putcl = __krnl_cputcl,
  .puts = __krnl_cwrite, 

  .getc = NULL,
  .gets = NULL,
  .clear = __krnl_cclear,
  .movec = __krnl_cmovec,
  .flush = __krnl_cflush,
  .xsize = 80,
  .ysize = 25,
  .cx = 0,
  .cy = 0
};



