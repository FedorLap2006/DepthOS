#define FS_OK 0
#define FS_FAIL 1

int write(unsigned long int dnum,unsigned long int sector,unsigned char *buffer,unsigned long int count_sec){
	asm(
		"movl $0x42,%%ah"
		//...
	);
	return FS_OK;
}

int read(unsigned long int sector,unsigned char *buffer,unsigned long int count_sec){
	return FS_OK;
}
