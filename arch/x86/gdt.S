#define DESC_G		0x00800000
#define DESC_D_B	0x00400000
#define DESC_P		0x00008000
#define DESC_S		0x00001000
#define DESC_A    0x00000100
#define DESC_TYPE_X	0x00000800
#define DESC_TYPE_R	0x00000200
#define DESC_TYPE_W	0x00000200

#define DESC_LIMIT_LOW(v) ((v) & 0x0000ffff)
#define DESC_LIMIT_HIGH(v) ((v) & 0x000f0000)
#define DESC_BASE_LOW(v) (((v) & 0x0000ffff) << 16)
#define DESC_BASE_HIGH(v) (((v) & 0xff000000) | (((v) & 0x00ff0000) >> 16))
#define DESC_DPL(v) ((v & 0x3) << 13)

#define KERNEL_CODE_BASE	0
#define KERNEL_CODE_LIMIT	0xfffff

#define KERNEL_DATA_BASE	0
#define KERNEL_DATA_LIMIT	0xfffff

#define USER_CODE_BASE	0
#define USER_CODE_LIMIT	0xfffff

#define USER_DATA_BASE	0
#define USER_DATA_LIMIT	0xfffff



	.text
	.globl set_up_gdt
set_up_gdt:
	push %edx
	push %ebx
	push %ecx
	
	lea kernel_tss, %ecx
	// subl $0xC0000000, %ecx
	movl %ecx, %edx
	lea .Lgdt, %ebx
	andl $0x0000ffff, %edx
	shl $16, %edx
	orl %edx, (5 * 8)(%ebx)
	movl %ecx, %edx
	and $0x00ff0000, %edx
	shr $16, %edx
	orl %edx, (5 * 8 + 4)(%ebx)
	and $0xff000000, %ecx
	orl %ecx, (5 * 8 + 4)(%ebx)

	pop %ecx
	pop %ebx
	pop %edx

	mov	$0x10, %ax
	lgdt 1f
	mov $((5 * 8) | 0), %cx
	ltr %cx
	# lgdt 1f
	jmp	$0x8, $2f
	# jmp	$0x8, $2f
2:
	mov	%ax, %ds
	mov	%ax, %es
	mov %ax, %gs
	mov %ax, %fs
	mov	%ax, %ss
	ret

	.data
1:
	.short	.Lgdt_end - .Lgdt
	.long	.Lgdt

	.align	16
.Lgdt:
	.long	0
	.long	0
	
	.long	DESC_LIMIT_LOW(KERNEL_CODE_LIMIT) | \
		DESC_BASE_LOW(KERNEL_CODE_BASE)
	.long	DESC_G | DESC_D_B | \
		DESC_P | DESC_S | DESC_TYPE_X | DESC_TYPE_R | \
		DESC_DPL(0) | \
		DESC_LIMIT_HIGH(KERNEL_CODE_LIMIT) | \
		DESC_BASE_HIGH(KERNEL_CODE_BASE)

	.long	DESC_LIMIT_LOW(KERNEL_DATA_LIMIT) | \
		DESC_BASE_LOW(KERNEL_DATA_BASE)
	.long	DESC_G | DESC_D_B | \
		DESC_P | DESC_S | DESC_TYPE_R | DESC_TYPE_W | \
		DESC_DPL(0) | \
		DESC_LIMIT_HIGH(KERNEL_DATA_LIMIT) | \
		DESC_BASE_HIGH(KERNEL_DATA_BASE)

	.long DESC_LIMIT_LOW(USER_CODE_LIMIT) | \
		DESC_BASE_LOW(USER_CODE_BASE)
	.long DESC_G | DESC_D_B | \
		DESC_P | DESC_S | DESC_TYPE_X | DESC_TYPE_R | \
		DESC_DPL(3) | \
		DESC_LIMIT_HIGH(USER_CODE_LIMIT) | \
		DESC_BASE_HIGH(USER_CODE_BASE)

	.long DESC_LIMIT_LOW(USER_DATA_LIMIT) | \
		DESC_BASE_LOW(USER_DATA_BASE)
	.long DESC_G | DESC_D_B | \
		DESC_P | DESC_S | DESC_TYPE_R | DESC_TYPE_W | \
		DESC_DPL(3) | \
		DESC_LIMIT_HIGH(USER_DATA_LIMIT) | \
		DESC_BASE_HIGH(USER_DATA_BASE)
	
	.long DESC_LIMIT_LOW(0x68)
	.long DESC_P | DESC_A | DESC_TYPE_X | \
		DESC_LIMIT_HIGH(0x68)

.Lgdt_end:
