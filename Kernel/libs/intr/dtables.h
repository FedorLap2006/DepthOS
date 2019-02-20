#pragma once

#include "../stdafx.h"
#include "../stdtypes.h"
#include "../stdlibs/strings.h"

/*

GDT

*/

void initDTables();

struct _gdt_entry_t {
	uint16 limit; // low 16 bits of offset
	uint16 lbase; // low 16 bits of base
	uint8 mbase; // middle bits of base
	uint8 access; // access
	uint8 granul; // granularity
	uint8 hbase; // high bits of base
}packed;

typedef struct _gdt_entry gdt_entry_t;


struct _gdt_ptr_t {
	uint16 limit;
	uint32 base;
}packed;

typedef struct _gdt_ptr_t gdt_ptr_t;


gdt_entry_t gdt_entries[5];
gdt_ptr_t gdt_ptr;


extern void gdt_flush(uint32);
static void init_gdt();
static void gdt_sgate(int32, uint32, uint32, uint8, uint8);

/*

IDT

*/

struct _idt_entry_t {
	uint16 laddr; // 
	uint16 csel; //
	uint8 a0; // not used, always 0
	uint8 flags; //
	uint16 haddr; //
}packed;

typedef _idt_entry_t idt_entry_t;

struct _idt_ptr_t {
	uint16 limit; //
	uint32 base; //
}packed;

typedef _idt_ptr_t idt_ptr_t;

idt_entry_t idt_entries[256];
idt_ptr_t idt_ptr;


// reserved processor interrputs
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
////////////////////////////////

