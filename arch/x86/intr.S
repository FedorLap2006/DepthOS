// .intel_syntax noprefix

#include <depthos/x86/asm/gdt.h>

.text

.global idt_flush

.extern idt_interrupt_handler
.extern idt_hwinterrupt_handler

.macro INTR_NOERRCODE num
	.global intr\num
	intr\num:
		cli
		pushl $0 # XXX: ebp, could be polluted, set to zero to prevent further frame jumps
		pushl $0
		pushl $\num
		jmp intr_cstub
.endm

.macro INTR_ERRCODE num
	.global intr\num
	intr\num:
		cli
		pushl (%esp) # error code
		# movl $0, 4(%esp) # XXX: ebp, could be polluted, set to zero to prevent further frame jumps
		movl %ebp, 4(%esp)
		pushl $\num
		jmp intr_cstub
.endm

.macro IRQ_NOERRCODE num
	.global intr\num
	intr\num:
		// push 0 in place of the error code
		pushl $0 # XXX: ebp, could be polluted, set to zero to prevent further frame jumps
		pushl	$0
		pushl	$\num
		jmp	irq_cstub
.endm


INTR_NOERRCODE 0 # Division error
INTR_NOERRCODE 1 # Debug
INTR_NOERRCODE 2 # Non-maskable interrupt
INTR_NOERRCODE 3 # Breakpoint
INTR_NOERRCODE 4 # Overflow
INTR_NOERRCODE 5 # Bound range exceeded
INTR_NOERRCODE 6 # Invalid opcode
INTR_NOERRCODE 7 # Device not available
INTR_ERRCODE   8 # Double fault
INTR_NOERRCODE 9 # Coprocessor segment overrun
INTR_ERRCODE   10 # Invalid TSS
INTR_ERRCODE   11 # Segment not present
INTR_ERRCODE   12 # Stack-segment fault
INTR_ERRCODE   13 # General protection fault
INTR_ERRCODE   14 # Page fault
INTR_NOERRCODE 15
INTR_NOERRCODE 16 # X87 floating-point exception
INTR_ERRCODE 17 # Alignment check
INTR_NOERRCODE 18 # Machine check
INTR_NOERRCODE 19 # SIMD floating-point exception
INTR_NOERRCODE 20 # Virtualization exception
INTR_NOERRCODE 21 # Control protection exception
INTR_NOERRCODE 22
INTR_NOERRCODE 23
INTR_NOERRCODE 24
INTR_NOERRCODE 25
INTR_NOERRCODE 26
INTR_NOERRCODE 27
INTR_NOERRCODE 28 # Hypervisor injection exception
INTR_ERRCODE 29 # VMM communication exception
INTR_NOERRCODE 30 # Security exception
INTR_NOERRCODE 31
# IRQ HANDLERS
IRQ_NOERRCODE 32
IRQ_NOERRCODE 33
IRQ_NOERRCODE 34
IRQ_NOERRCODE 35
IRQ_NOERRCODE 36
IRQ_NOERRCODE 37
IRQ_NOERRCODE 38
IRQ_NOERRCODE 39
IRQ_NOERRCODE 40
IRQ_NOERRCODE 41
IRQ_NOERRCODE 42
IRQ_NOERRCODE 43
IRQ_NOERRCODE 44
IRQ_NOERRCODE 45
IRQ_NOERRCODE 46
IRQ_NOERRCODE 47
# END IRQ HANDLERS
INTR_NOERRCODE	48
INTR_NOERRCODE	49
INTR_NOERRCODE	50
INTR_NOERRCODE	51
INTR_NOERRCODE	52
INTR_NOERRCODE	53
INTR_NOERRCODE	54
INTR_NOERRCODE	55
INTR_NOERRCODE	56
INTR_NOERRCODE	57
INTR_NOERRCODE	58
INTR_NOERRCODE	59
INTR_NOERRCODE	60
INTR_NOERRCODE	61
INTR_NOERRCODE	62
INTR_NOERRCODE	63
INTR_NOERRCODE	64
INTR_NOERRCODE	65
INTR_NOERRCODE	66
INTR_NOERRCODE	67
INTR_NOERRCODE	68
INTR_NOERRCODE	69
INTR_NOERRCODE	70
INTR_NOERRCODE	71
INTR_NOERRCODE	72
INTR_NOERRCODE	73
INTR_NOERRCODE	74
INTR_NOERRCODE	75
INTR_NOERRCODE	76
INTR_NOERRCODE	77
INTR_NOERRCODE	78
INTR_NOERRCODE	79
INTR_NOERRCODE	80
INTR_NOERRCODE	81
INTR_NOERRCODE	82
INTR_NOERRCODE	83
INTR_NOERRCODE	84
INTR_NOERRCODE	85
INTR_NOERRCODE	86
INTR_NOERRCODE	87
INTR_NOERRCODE	88
INTR_NOERRCODE	89
INTR_NOERRCODE	90
INTR_NOERRCODE	91
INTR_NOERRCODE	92
INTR_NOERRCODE	93
INTR_NOERRCODE	94
INTR_NOERRCODE	95
INTR_NOERRCODE	96
INTR_NOERRCODE	97
INTR_NOERRCODE	98
INTR_NOERRCODE	99
INTR_NOERRCODE	100
INTR_NOERRCODE	101
INTR_NOERRCODE	102
INTR_NOERRCODE	103
INTR_NOERRCODE	104
INTR_NOERRCODE	105
INTR_NOERRCODE	106
INTR_NOERRCODE	107
INTR_NOERRCODE	108
INTR_NOERRCODE	109
INTR_NOERRCODE	110
INTR_NOERRCODE	111
INTR_NOERRCODE	112
INTR_NOERRCODE	113
INTR_NOERRCODE	114
INTR_NOERRCODE	115
INTR_NOERRCODE	116
INTR_NOERRCODE	117
INTR_NOERRCODE	118
INTR_NOERRCODE	119
INTR_NOERRCODE	120
INTR_NOERRCODE	121
INTR_NOERRCODE	122
INTR_NOERRCODE	123
INTR_NOERRCODE	124
INTR_NOERRCODE	125
INTR_NOERRCODE	126
INTR_NOERRCODE	127
INTR_NOERRCODE 128


intr_cstub:
	pushal
	pushl	%ds
	pushl	%es
	pushl	%gs
	pushl	%fs

	cld
	mov	$(GDT_KERNEL_DATA_SEL), %ax
	mov	%ax, %ds
	mov	%ax, %es
	mov $(GDT_GSBASE_SEL(0)), %ax
	mov %ax, %gs
	mov $(GDT_FSBASE_SEL(0)), %ax

	leal 56(%esp), %ebp
	
	pushl %esp
	// Call the kernel IRQ handler
	call	idt_interrupt_handler
	add $4, %esp

	jmp intr_exit


irq_cstub:
	pushal
	pushl	%ds
	pushl	%es
	pushl	%gs
	pushl	%fs

	cld
	mov	$(GDT_KERNEL_DATA_SEL), %ax
	mov	%ax, %ds
	mov	%ax, %es
	mov $(GDT_GSBASE_SEL(0)), %ax
	mov %ax, %gs
	mov $(GDT_FSBASE_SEL(0)), %ax
	mov %ax, %fs
	leal 56(%esp), %ebp
	
	pushl %esp
	// Call the kernel IRQ handler
	call	idt_hwinterrupt_handler
	add $4, %esp

.global intr_exit
intr_exit:
	popl	%fs
	popl	%gs
	popl	%es
	popl	%ds
	popal
	// pop error code, stack frame and IRQ number
	add	$12, %esp
	iret


idt_flush:
	lidt idt_ptr
	ret
