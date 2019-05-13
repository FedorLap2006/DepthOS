ENTRY(_loader)
/*OUTPUT_FORMAT(pei-i386)
OUTPUT_ARCH(i386:i386)*/
 
KERNEL_VIRTUAL_BASE = 0xC0000000;
 
SECTIONS {
   /* The multiboot data and code will exist in low memory
      starting at 0x100000 */
 
   . = 0x00100000;
   .lowerhalf : {
       *(.lowerhalf)
   }
 
   /* The kernel will live at 3GB + 1MB in the virtual
      address space, which will be mapped to 1MB in the
      physical address space. */
 
   . += KERNEL_VIRTUAL_BASE;
   .text ALIGN(4096) : AT(ADDR(.text) - KERNEL_VIRTUAL_BASE) {
       *(.text*)
   }
 
   .data ALIGN (4096) : AT(ADDR(.data) - KERNEL_VIRTUAL_BASE) {
       *(.data)
       *(.rdata*)
       *(.rodata*)
   }
 
   .bss ALIGN (4096) : AT(ADDR(.bss) - KERNEL_VIRTUAL_BASE) {
       _sbss = .;
       *(COMMON)
       *(.bss)
       _ebss = .;
   }
   end = .; _end = .; __end = .;
 
   /DISCARD/ : {
       *(.eh_frame);
       *(.comment*);
   }
}
