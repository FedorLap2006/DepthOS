#pragma once
#include "../stdafx.h"
#include "../stdtypes.h"


typedef struct _regs_t {
	uint32 ds;
	uint32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32 int_n, err_code;
	uint32 eip, cs, eflags, uesp, ss;
}regs_t;