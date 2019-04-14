ENTRY(_loadkernel)

SECTIONS {
    . = 0x100000;
	.boot : {
		*(.multiboot)

	}
    .text : {
    	*(.text)
    }
    .data : {
        *(.data)
    }
    .bss : {
        *(.bss)
    }
}
