OUTPUT_FORMAT("elf32-i386")
ENTRY(_loader)
/*OUTPUT_FORMAT(pei-i386)
OUTPUT_ARCH(i386:i386)*/
 
 
SECTIONS {
   /* The multiboot data and code will exist in low memory
      starting at 0x100000 */
   . = 0xC0100000;
    
   /* The kernel will live at 3GB + 1MB in the virtual
      address space, which will be mapped to 1MB in the
      physical address space. */
 
   .text ALIGN(4096) : AT(ADDR(.text) - 0xC0000000) {
       *(.multiboot)
       *(.text*)
   }
   .rodata ALIGN(4K) : AT(ADDR(.rodata) - 0xC0000000) {
       *(.rdata*)
        *(.rodata)
   }

   .bss ALIGN (4096) : AT(ADDR(.bss) - 0xC0000000) {
       *(COMMON)
       *(.bss)
   }

   .data ALIGN (4096) : AT(ADDR(.data) - 0xC0000000) {
       *(.data)
   }

   kernel_end = .; _kernel_end = .; __kernel_end = .;
 
   /DISCARD/ : {
       *(.eh_frame);
       *(.comment*);
   }
}
