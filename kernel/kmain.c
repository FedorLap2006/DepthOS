#include <std/types.h>

#define MBOOT_ID 0x2BADB002

void print_str(char* str) {
  unsigned short* videoMemory = (unsigned short*)0xb8000;

  for (int i = 0; i < str[i] != '\0'; i++) {
    videoMemory[i] = (videoMemory[i] & 0xFF00) | str[i];
  }

}
// unsigned int magic
void kmain(void) {
	// if ( magic != MBOOT_ID ) return;
  print_str("hello world!");
  while (true) {}
}
