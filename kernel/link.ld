SECTIONS {
	. = 0x10;

	.text : {
		*(.multiboot)
		*(.text)
	}
	.data : {
		*(.data)
	}
	.bss : {
		*(.bss)
	}
}
