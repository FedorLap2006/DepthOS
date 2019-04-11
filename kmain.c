

void print_str(char* str) {
  unsigned short* videoMemory = (unsigned short*)0xb8000;

  for (int i = 0; i < str[i] != '\0'; i++) {
    videoMemory[i] = (videoMemory[i] & 0xFF00) | str[i];
  }

}

void kmain(int magic,void *boot_ptr) {

	const char str[] = "H\x0F""e\x0Fl\x0Fl\x0Fo\x0F \x0Fw\x0Fo\x0Fr\x0Fl\x0F""d\x0F";
	char* buf = (char*) 0xB8000;
	char c;
	for(unsigned long int i = 0; c = str[i]; i++) {
		buf[i] = str[i];
	}
	while(1);
}
