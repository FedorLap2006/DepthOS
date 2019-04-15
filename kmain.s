	.file	"kmain.c"
	.text
	.globl	print_str
	.def	print_str;	.scl	2;	.type	32;	.endef
print_str:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	$753664, -8(%ebp)
	movl	$0, -4(%ebp)
	jmp	L2
L3:
	movl	-4(%ebp), %eax
	leal	(%eax,%eax), %edx
	movl	-8(%ebp), %eax
	addl	%edx, %eax
	movzwl	(%eax), %eax
	movb	$0, %al
	movl	%eax, %ecx
	movl	-4(%ebp), %edx
	movl	8(%ebp), %eax
	addl	%edx, %eax
	movzbl	(%eax), %eax
	cbtw
	orl	%eax, %ecx
	movl	%ecx, %edx
	movl	-4(%ebp), %eax
	leal	(%eax,%eax), %ecx
	movl	-8(%ebp), %eax
	addl	%ecx, %eax
	movw	%dx, (%eax)
	addl	$1, -4(%ebp)
L2:
	movl	-4(%ebp), %edx
	movl	8(%ebp), %eax
	addl	%edx, %eax
	movzbl	(%eax), %eax
	movsbl	%al, %eax
	cmpl	%eax, -4(%ebp)
	jl	L3
	nop
	leave
	ret
	.section .rdata,"dr"
LC0:
	.ascii "hello world!\0"
	.text
	.globl	kmain
	.def	kmain;	.scl	2;	.type	32;	.endef
kmain:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$4, %esp
	movl	$LC0, (%esp)
	call	print_str
L5:
	jmp	L5
	.ident	"GCC: (GNU) 7.4.0"
