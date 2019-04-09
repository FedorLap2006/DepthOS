#include <types.h>
#include <heap.h>
#include <task.h>
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
	init_paging();
	init_heap(KHEAP_SIZE);

	addProcess(createProcess("hello!",NULL));
	
	

	exec();

	print_str("hello world!");
	while (true) {}
}
