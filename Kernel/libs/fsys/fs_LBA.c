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
int sys_write(LBA info){
	byte func = FS_READ;
	byte ssize = 0x10;
	
	info.dap.size = ssize;

	gasm(
		"movl $%0,%%ah" : : "M" (func) :
	);
	gasm(
		"movl $%0,%%dl" : : "M" (info.hdrive) :

	);

	// push DAP struct fields to stack ( next commit )
	

	gasm(
		"movl (%0),%%si\n"
			:
			: "o" (info.dap)
			:
	);

	// set addr of buffer

	
	pgasm(
		"push %%es\n"
		"push %%bx"
	); // prepare es,bx ( addr of buffer )

	
	
	// use buf_ptr ptr as segment based ptr and mov 0 offset to bx
	
	gasm(
		"movl (%0),%%es" 
			:
			: "m" (info.dap.buf)
			:
	);
	gasm(
		"movl $0,%%bx"
	);
	///////////////////////////////////////////////////////////
	
	// one or two -- question!
	////////////////////////

	/*gasm("int 0x13");*/
	intr(0x13); // interrput ( LBA call )
}
int sys_read(LBA info){
}
