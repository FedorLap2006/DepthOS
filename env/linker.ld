ENTRY(krnl_bootloader)


SECTIONS 
{
  . = 1M;

  __kernel_beg = .;
  .text ALIGN(4K) :
  {
    /**(.bootloader_zerolvl_header)*/
    *(.bl_0l_header)
    *(.text)
  }
  .rodata ALIGN (4K) :
  {
    *(.rodata)
    *(.rodata.*)
  }
  .data ALIGN (4K) :
  {
    *(.data)
  }
  .bss ALIGN (4K) :
  {
    *(.krnl_data)
    *(.krnl_bss)
    *(.bss)
  }
  __kernel_end = .;

  /DISCARD/ : {
     *(.eh_frame);
     *(.comment*);
  }
}


/*SECTIONS 
{
  . = 0xC0100000;

  _kernel_beg = .;
  .text ALIGN(4K) : AT(ADDR(.text) - 0xC0000000) 
  {
    *(.bootloader_zerolvl_header)
    *(.text)
  }
  .rodata ALIGN (4K) : AT (ADDR (.rodata) - 0xC0000000)
  {
    *(.rodata)
    *(.rodata.*)
  }
  .data ALIGN (4K) : AT (ADDR (.data) - 0xC0000000)
  {
    *(.data)
  }
  .bss ALIGN (4K) : AT (ADDR (.bss) - 0xC0000000)
  {
    *(.bss)
  }
}*/