#include "fs_LBA.h"

// not used comments ( examples )

// int write(unsigned long int dnum,unsigned long int sector,unsigned char *buffer,unsigned long int count_sec){
// 	gasm(
//
// 	)
// 	return FS_OK;
// }
//
// int read(unsigned long int sector,unsigned char *buffer,unsigned long int count_sec){
// 	return FS_OK;
// }
//{
//asm(
//	"movb $%1,%%ah"
//	:
//: "r" (FS_READ)
//	:
//	);
//asm(
//	"movl $%1,%%dl" // number of disk
//	:
//: "r" (d.hlba)
//	:
//	);
//asm(
//	"push $%1" // large bits sector
//	:
//: "r" (lsec)
//	:
//	);
//asm(
//	"push $%1" // min bits sector
//	:
//: "r" (msec)
//	:
//	);
//// asm(
////     "push %d%s"
//// );
//asm(
//	"push %%fs"
//);
//asm("push %1" : : "r" (&d.dap.buf) : );
//asm(
//	"movl %%sp,%%si\n"
//	// "movl $%1,%%sp"
//	//     :
//	//     : "r" (d.dap_off)
//	"int 0x13\n"
//);
//}
// d.dap_off = __LINE__ - 1;
//	asm(
//		"movl (%0),%%si\n" 
//		:
//		: "o" (info.dap)
//		:
//	);
/* ( set addr of DAP buffer )

		////////////////////////////////////////////////////////////

		// for stack variables

		pgasm("movl %%ss,%%es"); // ss or ds ( segment of buffer ) ???
		gasm(
			"movl %0,%%bx" // set offset of buffer
			:
			: "o" (info.dap.buf) // o or m ???
			:
		);
		////////////////////////////////////////////////////////////
	*/






// fat example ( 13h ) https://habrastorage.org/getpro/habr/post_images/0d6/275/777/0d627577730050dfa24baa9928b2a0d0.jpg

int sys_writeLBA(LBA info){
	byte func = FS_WRITE;
	int32 jsbits = info.dap.sector << (8*4);
	int32 ssbits = info.dap.sector;
	
	asm volatile("pushal"); // push to stack all x32 regs

	///////////////////////////////////////////////////////////////////


	// prepare data to interrput ( LBA call )
	
	asm volatile(
		"push %%es\n"
		"push %%bx"
	); // prepare es,bx ( addr of buffer )

	
	
	// use buf_ptr ptr as segment based ptr and mov 0 offset to bx
	
	asm(
		"movl (%0),%%es" // corrected mov
			:
			: "m" (info.dap.buf)
			:
	);
	asm(
		"movl $0,%%bx"
	);

	///////////////////////////////////////////////////////////////////
	
	// general push to stack

	
	asm(
		"push %0\n"
			:
			: "r" (ssbits)
			:
	); // push senior-bits of sector number

	asm(
		"push %0\n"
			:
			: "r" (jsbits)
			:
	); // push junior-bits of sector number	

	asm("push %%es");
	asm("push %%bx");
	asm("push $%0": : "M" (info.dap.sector_count) :);
	
	asm("push $0x10");
	
	///////////////////////////////////////////////////////////////////	

	// recovery used registers and clear stack

	// ...

	///////////////////////////////////////////////////////////////////
	
	asm("movl $%0,%%ah" : : "M" (func) : ); // prepare number of func 
 	asm("movl $%0,%%dl" : : "M" (info.hdrive) : ); // drive index ( from 0 )
 	asm("movl %%esp,%%esi"); // prepare DAP struct
 	
	//////////////////////////////////////////////////////////////////
	
	asm("int $0x13"); // interrput ( LBA call )
	asm volatile("popal"); // pop from stack all x32 regs
}
int sys_readLBA(LBA info){
	byte func = FS_READ;
	int32 jsbits = info.dap.sector << (8*4);
	int32 ssbits = info.dap.sector;
	
	asm volatile("pushal"); // push to stack all x32 regs

	///////////////////////////////////////////////////////////////////


	// prepare data to interrput ( LBA call )
	
	asm volatile(
		"push %%es\n"
		"push %%bx"
	); // prepare es,bx ( addr of buffer )

	
	
	// use buf_ptr ptr as segment based ptr and mov 0 offset to bx
	
	asm(
		"movl (%0),%%es" // corrected mov
			:
			: "m" (info.dap.buf)
			:
	);
	asm(
		"movl $0,%%bx"
	);

	///////////////////////////////////////////////////////////////////
	
	// general push to stack

	
	asm(
		"push %0\n"
			:
			: "r" (ssbits)
			:
	); // push senior-bits of sector number

	asm(
		"push %0\n"
			:
			: "r" (jsbits)
			:
	); // push junior-bits of sector number	

	asm("push %%es");
	asm("push %%bx");
	asm("push $%0": : "M" (info.dap.sector_count) :);
	
	asm("push $0x10");
	
	///////////////////////////////////////////////////////////////////	

	// recovery used registers and clear stack

	// ...

	///////////////////////////////////////////////////////////////////
	
	asm("movl $%0,%%ah" : : "M" (func) : ); // prepare number of func 
 	asm("movl $%0,%%dl" : : "M" (info.hdrive) : ); // drive index ( from 0 )
 	asm("movl %%esp,%%esi"); // prepare DAP struct
 	
	//////////////////////////////////////////////////////////////////
	
	asm("int $0x13"); // interrput ( LBA call )
	asm volatile("popal"); // pop from stack all x32 regs
}
