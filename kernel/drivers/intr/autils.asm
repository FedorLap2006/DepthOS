global gdt_flush
global idt_flush

;;;;;;;;;;;;;;; GDT ;;;;;;;;;;;;;;;

gdt_flush:
	mov eax, [esp+4]
	lgdt [eax]
	
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	
	jmp 0x08:.flush
.flush:
	ret

 ;;;;;;;;;;;;;;; IDT ;;;;;;;;;;;;;;;
idt_flush:
	mov eax, [esp+4]
	lidt [eax]
	ret

;;;;;;;;;;;;;;; ISR ;;;;;;;;;;;;;;;

%macro isr_nec 1
global isr%1

isr%1:
	cli
	push byte 0
	push byte %1
	jmp intr_prepare
%endmacro

%assign isr_i 0
%rep 31
isr_nec isr_i

%assign isr_i i+1
%endrep

extern intr_handler

intr_prepare:
	pusha
	
	mov ax,ds
	push eax
	
	mov ax,0x10
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax

	call intr_handler
	
	pop eax
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax

	popa
	add esp, 8
	sti
	iret
;;;;;;;;;;;;;;; IRQ ;;;;;;;;;;;;;;;
