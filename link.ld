ENTRY(_loader)
/*OUTPUT_FORMAT(pei-i386)
OUTPUT_ARCH(i386:i386)*/

SECTIONS {
	. = 0xC0100000;

	/*.boot ALIGN(4K) : AT(ADDR(.boot) - 0xC0000000 ) {
		*(.boot)
	}
	*/
    .text : ALIGN(4K), AT(0x12345000) {
		*(.boot)
    	*(.text)
    }
	.data : ALIGN(4K), AT(ADDR(.data) - 0xC0000000 ) {
        *(.data)
    }
    .bss : ALIGN(4K), AT(ADDR(.bss) - 0xC0000000 ) {
        *(.bss)
    } 
	

	end = .; _end = .; __end = .;
}
