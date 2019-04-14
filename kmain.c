void print_str(char* str) {
  unsigned short* videoMemory = (unsigned short*)0xb8000;

  for (int i = 0; i < str[i] != '\0'; i++) {
    videoMemory[i] = (videoMemory[i] & 0xFF00) | str[i];
  }

}

void kmain(int magic,void *boot_ptr) {
	print_str("hello world!");
	while(1) {}
}
