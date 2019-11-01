#include <depthos/io/console.h>
extern void __console_minit();
//#include <depthos/arch/x86/gdt.h>

// unsigned *videoMemory = 0xb8000;
// void print_str(char* str) {

//   for (int i = 0; str[i] != '\0'; i++) {
//     videoMemory[i] = (videoMemory[i] & 0xFF00) | str[i];
//   }

// }
  /////////////////////////////////////////
  ////// ::: Setup Page Directory ::: /////
  /////////////////////////////////////////

//   movl $boot_pagetb, %edi // current page
//   movl $0x1000, %cx // current idx, now count of repeat
// 1:
//   addb $0x03, (%edi)
//   movl $4096, %esi
//   shll $0x0C, %esi
//   addl %esi, (%edi)
//   addl $0x20, %edi
//   loop 1b
  /////////////////////////////////////////

int _kernel(int magic, void *mptr) {
  // __console_mdl_init(25,80,BLACK_COLOR,WHITE_COLOR);
  // __console_write_dec(50);
  // __console_putchar('C');
  //__console_mdl_init();
  __console_minit();
  // _krnl_console.putc('H');
  // _krnl_console.putc('E');
  // _krnl_console.putc('L');
  // _krnl_console.putc('L');
  // _krnl_console.putc('O');
  // _krnl_console.putc(' ');
  // _krnl_console.putc('W');
  // _krnl_console.putc('O');
  // _krnl_console.putc('R');
  // _krnl_console.putc('L');
  // _krnl_console.putc('D');
  // _krnl_console.putc('!');
  // _krnl_console.putc('\n');
  _krnl_console.puts("The x86 architecture has two methods of memory protection and of providing virtual memory - segmentation and paging.\n\
\n\
With segmentation, every memory access is evaluated with respect to a segment. That is, the memory address is added to the segment's base address, and checked against the segment's length. You can think of a segment as a window into the address space - The process does not know it's a window, all it sees is a linear address space starting at zero and going up to the segment length.\n\
\n\
With paging, the address space is split into (usually 4KB, but this can change) blocks, called pages. Each page can be mapped into physical memory - mapped onto what is called a 'frame'. Or, it can be unmapped. Like this you can create virtual memory spaces.\n\
\n\
Both of these methods have their advantages, but paging is much better. Segmentation is, although still usable, fast becoming obsolete as a method of memory protection and virtual memory. In fact, the x86-64 architecture requires a flat memory model (one segment with a base of 0 and a limit of 0xFFFFFFFF) for some of it's instructions to operate properly.\n\
\n\
Segmentation is, however, totally in-built into the x86 architecture. It's impossible to get around it. So here we're going to show you how to set up your own Global Descriptor Table - a list of segment descriptors.\n\
\n\
As mentioned before, we're going to try and set up a flat memory model. The segment's window should start at 0x00000000 and extend to 0xFFFFFFFF (the end of memory). However, there is one thing that segmentation can do that paging can't, and that's set the ring level.\n\
\n\
A ring is a privilege level - zero being the most privileged, and three being the least. Processes in ring zero are said to be running in kernel-mode, or supervisor-mode, because they can use instructions like sti and cli, something which most processes can't. Normally, rings 1 and 2 are unused. They can, technically, access a greater subset of the supervisor-mode instructions than ring 3 can. Some microkernel architectures use these for running server processes, or drivers.\n\
");

}