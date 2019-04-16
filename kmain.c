
unsigned short* videoMemory = (unsigned short*)0xb8000;


void print_str(char* str) {

  for (int i = 0; i < str[i] != '\0'; i++) {
    videoMemory[i] = (videoMemory[i] & 0xFF00) | str[i];
  }

}

void kmain(int magic,void *boot_ptr) {
//	print_str("hello world! ");
	*videoMemory = 0x2f4b2f4f;
//	while(1) {}
}
