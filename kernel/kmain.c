
#define MBOOT_ID 0x2BADB002

void kmain(unsigned int magic,const void* boot) {
	if ( magic != MBOOT_ID ) return;

}
