ENTRY(_loader)
/*OUTPUT_FORMAT(pei-i386)
OUTPUT_ARCH(i386:i386)*/

SECTIONS {
    . = 1M;

	.boot : ALIGN(4) {
		*(.boot)
	}
    .text : ALIGN(4) {
    	*(.text)
    }
	.data : ALIGN(4) {
        *(.data)
    }
    .bss : ALIGN(4) {
        *(.bss)
    } 

}
