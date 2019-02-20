#include "dtables.h"

void initDTables() {
	init_gdt();
	init_idt();
}

static void init_gdt() {
	gdt_ptr.limit = (sizeof(gdt_entry_t) * 5) - 1;
	gdt_ptr.base = (int32)&gdt_entries;
	gdt_sgate(0, 0, 0, 0, 0);
	gdt_sgate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
	gdt_sgate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
	gdt_sgate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
	gdt_sgate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);
	gdt_flush((uint32)&gdt_ptr);
}
static void gdt_sgate(int32 num, uint32 base, uint32 limit, uint8 access, uint8 gran) {
	gdt_entries[num].lbase = (base & 0xFFFF);
	gdt_entries[num].mbase = (base >> 16) & 0xFF;
	gdt_entries[num].hbase = (base >> 24) & 0xFF;

	gdt_entries[num].limit = (limit & 0xFFFF);
	gdt_entries[num].granul = (limit >> 16) & 0x0F;
	gdt_entries[num].granul |= gran & 0xF0;
	gdt_entries[num].access = access;
}


static void init_idt() {
	idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
	idt_ptr.base = (uint32)&idt_entries;

	memset(&idt_entries,0,sizeof(idt_entry_t)*256);
	
}

static void idt_set_gate(uint8 num, uint32 base, uint16 sel, uint8 flags) {
	idt_entries[num].laddr = base & 0xFFFF;
	idt_entries[num].haddr = (base >> 16) & 0xFFFF;

	idt_entries[num].csel = sel;
	idt_entries[num].a0 = 0;

	// ... | 0x60 - user privilegies
	idt_entries[num].flags = flags /* | 0x60 */;
}