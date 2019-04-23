#define DESC_G		0x00800000
#define DESC_D_B	0x00400000
#define DESC_P		0x00008000
#define DESC_S		0x00001000
#define DESC_TYPE_X	0x00000800
#define DESC_TYPE_R	0x00000200
#define DESC_TYPE_W	0x00000200

#define DESC_LIMIT_LOW(v) ((v) & 0x0000ffff)
#define DESC_LIMIT_HIGH(v) ((v) & 0x000f0000)
#define DESC_BASE_LOW(v) (((v) & 0x0000ffff) << 16)
#define DESC_BASE_HIGH(v) (((v) & 0xff000000) | (((v) & 0x00ff0000) >> 16))

#define KERNEL_CODE_BASE	0
#define KERNEL_CODE_LIMIT	0xfffff

#define KERNEL_DATA_BASE	0
#define KERNEL_DATA_LIMIT	0xfffff

	.text
	.globl set_up_gdt
set_up_gdt:
	mov	$0x10, %ax
	lgdt	1f
	jmp	$0x8, $2f
2:
	mov	%ax, %ds
	mov	%ax, %es
	mov	%ax, %ss
	ret

	.data
1:
	.2byte	.Lgdt_end - .Lgdt
	.4byte	.Lgdt

	.align	16
.Lgdt:
	.4byte	0
	.4byte	0

	.4byte	DESC_LIMIT_LOW(KERNEL_CODE_LIMIT) | \
		DESC_BASE_LOW(KERNEL_CODE_BASE)
	.4byte	DESC_G | DESC_D_B | \
		DESC_P | DESC_S | DESC_TYPE_X | DESC_TYPE_R | \
		DESC_LIMIT_HIGH(KERNEL_CODE_LIMIT) | \
		DESC_BASE_HIGH(KERNEL_CODE_BASE)

	.4byte	DESC_LIMIT_LOW(KERNEL_DATA_LIMIT) | \
		DESC_BASE_LOW(KERNEL_DATA_BASE)
	.4byte	DESC_G | DESC_D_B | \
		DESC_P | DESC_S | DESC_TYPE_R | DESC_TYPE_W | \
		DESC_LIMIT_HIGH(KERNEL_DATA_LIMIT) | \
		DESC_BASE_HIGH(KERNEL_DATA_BASE)
.Lgdt_end:
