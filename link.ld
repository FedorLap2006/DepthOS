ENTRY(_loader)
/*OUTPUT_FORMAT(pei-i386)
OUTPUT_ARCH(i386:i386)*/
 
 
SECTIONS {
   /* The multiboot data and code will exist in low memory
      starting at 0x100000 */
 
   . = 1M;
 
   /* The kernel will live at 3GB + 1MB in the virtual
      address space, which will be mapped to 1MB in the
      physical address space. */
 
   .text ALIGN(4096) : {
	   *(.boot)
       *(.text*)
   }
 
   .data ALIGN (4096) : {
       *(.data)
       *(.rdata*)
       *(.rodata*)
   }
 
   .bss ALIGN (4096) : {
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
