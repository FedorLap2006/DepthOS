#include "i-con.h"

// GDT

extern void gdt_flush(uint32_t);
static void gdtsg(uint32_t, uint32_t, uint32_t, uint8_t, uint8_t);

static void init_gdt();


static void gdtsg(uint32_t num, uint32_t base, uint32_t off, uint8_t acc, uint8_t gr) {
	if ( num >= 5 ) return;
	gdte[num].offlow = ( off & 0xFFFF );
	gdte[num].blow = ( base & 0xFFFF );
	gdte[num].bmid = ( base >> 16 )  & 0xFF;
	gdte[num].acc = acc;
	gdte[num].gran |= gran & 0xF0;
	gdte[num].bhigh = ( base >> 24 ) & 0xFF;
	return;
}

static void init_gdt() {
	gdt_ptr.hoff = ( sizeof(gdte_t) * 5 ) - 1;
	gdt_ptr.limit = (uint32_t)&gdte;

	gdtsg(0, 0, 0, 0, 0);
	gdtsg(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
	gdtsg(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
	gdtsg(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
	gdtsg(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

	gdt_flush((uint32_t)&gdt_ptr);
}

// IDT

#define isr(num) extern void isr##num()

isr(0); isr(1); isr(2); isr(3); isr(4);
isr(5); isr(6); isr(7); isr(8); isr(9);
isr(10); isr(11); isr(12); isr(13);
isr(14); isr(15); isr(16); isr(17);
isr(18); isr(20); isr(21); isr(22);
isr(23); isr(24); isr(25); isr(26);
isr(27); isr(28); isr(29); isr(30);
isr(31);

#undef isr

extern void idt_flush(uint32_t);
static void idtsg(uint8_t,uint32_t,uint16_t,uint8_t,bool);
static void init_idt();

static void idtsg(uint8_t num,uint32_t base,uint16_t sel,uint8_t flags,bool usmode) {
	if ( num >= 256 ) return;
	idte[num].blow = ( base & 0xFFFF );
	idte[num].sel = sel;
	idte[num].zero = 0;
	if ( usmode == true ) {
		idte[num].flags = flags | 0x60;
	} else {
		idte[num].flags = flags;
	}
	idte[num].bhigh = ( ( base >> 16 ) & 0xFFFF );
}

static void init_idt() {
	idt_ptr.limit = ( sizeof(idte_t) * 256 ) - 1;
	idt_ptr.addr = (uint32_t)&idte;
	memset(&idte, 0, sizeof(idte_t) * 256);

#define isg(num) idtsg(num, (uint32_t)isr##num, 0x08, 0x8E);

	isg(0);	isg(1);	isg(2);	isg(3); isg(4); isg(5);
	isg(6); isg(7); isg(8); isg(9); isg(10); isg(11);
	isg(12); isg(13); isg(14); isg(15); isg(16); isg(17);
	isg(18); isg(19); isg(20); isg(21); isg(22); isg(23);
	isg(24); isg(25); isg(26); isg(27); isg(28); isg(29);
	isg(30); isg(31);

#undef isg
	idt_flush((uint32_t)&idt_ptr);
}

// SPECIAL FUNCTIONS AND VARIABLES

void init_intr() { init_gdt(); init_idt(); }
typedef struct intr_regs_t {
	uint32_t ds;        // Селектор сегмента данных
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t intr_it, err_code;
    uint32_t eip,cs,eflags,useresp,ss;
};
void intr_handler(intr_regs_t r) {
	// ...
}
