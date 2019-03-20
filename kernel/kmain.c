
#define MBOOT_ID 0x2BADB002

void kmain(unsigned int magic,const void* mboot) {
	if ( magic != MBOOT_ID ) return;

}
