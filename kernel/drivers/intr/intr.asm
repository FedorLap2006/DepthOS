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
	jmp isr_prepare
%endmacro

%assign isr_i 0
%rep 31
isr_nec isr_i

%assign isr_i isr_i+1
%endrep

extern isr_handler

isr_prepare:
	pusha
	
	mov ax,ds
	push eax
	
	mov ax,0x10
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax

	call isr_handler
	
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


%macro irq 2
global isr%1

irq%1:
	cli
	push byte 0
	push byte %2
	jmp irq_prepare
%endmacro

%assign irq_i1 0
%assign irq_i2 32
%rep 31
irq irq_i1, irq_i2

%assign irq_i1 irq_i1+1
%assign irq_i2 irq_i2+1
%endrep

extern irq_handler

irq_prepare:
	pusha
	
	mov ax,ds
	push eax
	
	mov ax,0x10
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax

	call irq_handler
	
	pop eax
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax

	popa
	add esp, 8
	sti
	iret

extern cintr_handler

cintr78:
	cli
	push byte 0
	push byte 78h
	jmp cintr_prepare

cintr79:
	cli
	push byte 0
	push byte 79h
	jmp cintr_prepare

cintr80:
	cli
	push byte 0
	push byte 80h
	jmp cintr_prepare

cintr_prepare:
	pusha
	
	mov ax,ds
	push eax
	
	mov ax,0x10
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax

	call cintr_handler
	
	pop eax
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax

	popa
	add esp, 8
	sti
	iret

